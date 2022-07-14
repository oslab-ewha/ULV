#include "ulvisor_virtio.h"

#include <linux/virtio_net.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define netdev_of(x)	(container_of(x, struct virtio_net_dev, dev))
#define BIT(x)	(1ULL << x)

#define RX_QUEUE_IDX 0
#define TX_QUEUE_IDX 1

#define NUM_QUEUES (TX_QUEUE_IDX + 1)
#define QUEUE_DEPTH 128

struct virtio_net_dev {
	struct virtio_dev	dev;
	struct virtio_net_config	config;
	pthread_mutex_t	**queue_locks;
	pthread_t	thread_poll;
};

extern int ulvisor_net_poll(void);
extern int ulvisor_net_read(struct iovec *iov, int count);
extern int ulvisor_net_write(struct iovec *iov, int count);

static int
net_check_features(struct virtio_dev *dev)
{
	if (dev->driver_features == dev->device_features)
		return 0;

	return -EINVAL;
}

static void
net_acquire_queue(struct virtio_dev *dev, int queue_idx)
{
	pthread_mutex_lock(netdev_of(dev)->queue_locks[queue_idx]);
}

static void
net_release_queue(struct virtio_dev *dev, int queue_idx)
{
	pthread_mutex_unlock(netdev_of(dev)->queue_locks[queue_idx]);
}

static int
net_enqueue(struct virtio_dev *dev, int q, struct virtio_req *req)
{
	struct virtio_net_hdr_v1	*header;
	struct virtio_net_dev	*net_dev;
	struct iovec	*iov;
	int	ret;

	header = req->buf[0].iov_base;
	net_dev = netdev_of(dev);

	/* skip vnet_header & revert */
	req->buf[0].iov_base += sizeof(struct virtio_net_hdr_v1);
	req->buf[0].iov_len -= sizeof(struct virtio_net_hdr_v1);
	iov = req->buf;

	/* Pick which virtqueue to send the buffer(s) to */
	if (q == TX_QUEUE_IDX) {
		ret = ulvisor_net_write(iov, req->buf_count);
		if (ret < 0)
			return -1;
	} else if (q == RX_QUEUE_IDX) {
		int i, len;

		ret = ulvisor_net_read(iov, req->buf_count);
		if (ret < 0)
			return -1;

		header->flags = 0;
		header->gso_type = VIRTIO_NET_HDR_GSO_NONE;

		/*
		 * Have to compute how many descriptors we've consumed (really
		 * only matters to the the mergeable RX mode) and return it
		 * through "num_buffers".
		 */
		for (i = 0, len = ret; len > 0; i++)
			len -= req->buf[i].iov_len;
		header->num_buffers = i;

		if (dev->device_features & BIT(VIRTIO_NET_F_GUEST_CSUM))
			header->flags |= VIRTIO_NET_HDR_F_DATA_VALID;
	} else {
		return -1;
	}

	/* Undo the adjustment */
	req->buf[0].iov_base -= sizeof(struct virtio_net_hdr_v1);
	req->buf[0].iov_len += sizeof(struct virtio_net_hdr_v1);
	ret += sizeof(struct virtio_net_hdr_v1);

	virtio_req_complete(req, ret);

	return 0;
}

static struct virtio_dev_ops	net_ops = {
	.check_features = net_check_features,
	.enqueue = net_enqueue,
	.acquire_queue = net_acquire_queue,
	.release_queue = net_release_queue,
};

static void *
poll_thread(void *arg)
{
	struct virtio_net_dev	*dev = arg;

	/* Synchronization is handled in virtio_process_queue */
	do {
		int	ret = ulvisor_net_poll();

		if (ret & 1)
			virtio_process_queue(&dev->dev, 0);
		if (ret & 2)
			virtio_process_queue(&dev->dev, 1);
	} while (1);

	return NULL;
}

static pthread_mutex_t *
mutex_alloc(int recursive)
{
	pthread_mutex_t	*mutex;
	pthread_mutexattr_t	attr;
	int	kind;

	mutex = malloc(sizeof(pthread_mutex_t));
	if (mutex == NULL)
		return NULL;
	pthread_mutexattr_init(&attr);
	kind = recursive ? PTHREAD_MUTEX_RECURSIVE: PTHREAD_MUTEX_ERRORCHECK;
	pthread_mutexattr_settype(&attr, kind);

	pthread_mutex_init(mutex, &attr);

	return mutex;
}

static void
mutex_free(pthread_mutex_t *mutex)
{
	pthread_mutex_destroy(mutex);
	free(mutex);
}

static void
free_queue_locks(pthread_mutex_t **queues, int num_queues)
{
	int i = 0;
	if (!queues)
		return;

	for (i = 0; i < num_queues; i++)
		mutex_free(queues[i]);

	free(queues);
}

static pthread_mutex_t **
init_queue_locks(int num_queues)
{
	pthread_mutex_t	**ret = malloc(sizeof(pthread_mutex_t *) * num_queues);
	int	i;

	if (!ret)
		return NULL;

	memset(ret, 0, sizeof(pthread_mutex_t *) * num_queues);
	for (i = 0; i < num_queues; i++) {
		ret[i] = mutex_alloc(1);
		if (!ret[i]) {
			free_queue_locks(ret, i);
			return NULL;
		}
	}

	return ret;
}

int
ulvisor_add_netdev(void)
{
	struct virtio_net_dev	*dev;
	int	ret = -ENOMEM;

	dev = malloc(sizeof(*dev));
	if (!dev)
		return -ENOMEM;

	memset(dev, 0, sizeof(*dev));

	dev->dev.device_id = VIRTIO_ID_NET;
	dev->dev.config_data = &dev->config;
	dev->dev.config_len = sizeof(dev->config);
	dev->dev.ops = &net_ops;
	dev->queue_locks = init_queue_locks(NUM_QUEUES);

	if (!dev->queue_locks)
		goto out_free;

	/*
	 * MUST match the number of queue locks we initialized. We could init
	 * the queues in virtio_dev_setup to help enforce this, but netdevs are
	 * the only flavor that need these locks, so it's better to do it
	 * here.
	 */
	ret = virtio_dev_setup(&dev->dev, NUM_QUEUES, QUEUE_DEPTH);

	if (ret)
		goto out_free;

	/*
	 * We may receive upto 64KB TSO packet so collect as many descriptors as
	 * there are available up to 64KB in total len.
	 */
	if (dev->dev.device_features & BIT(VIRTIO_NET_F_MRG_RXBUF))
		virtio_set_queue_max_merge_len(&dev->dev, RX_QUEUE_IDX, 65536);

	pthread_create(&dev->thread_poll, NULL, poll_thread, dev);

	return 0;

out_cleanup_dev:
	virtio_dev_cleanup(&dev->dev);

out_free:
	if (dev->queue_locks)
		free_queue_locks(dev->queue_locks, NUM_QUEUES);
	free(dev);

	return ret;
}
