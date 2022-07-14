#include "ulvisor_virtio.h"

#include <unistd.h>
#include <linux/virtio_blk.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

#define DEV_BLK_STATUS_OK	0
#define DEV_BLK_STATUS_IOERR	1
#define DEV_BLK_STATUS_UNSUP	2

#define DEV_BLK_TYPE_READ	0
#define DEV_BLK_TYPE_WRITE	1
#define DEV_BLK_TYPE_FLUSH	4
#define DEV_BLK_TYPE_FLUSH_OUT	5

struct virtio_blk_dev {
	struct virtio_dev	dev;
	struct virtio_blk_config	config;
	int	fd;
};

struct virtio_blk_req_trailer {
	uint8_t status;
};

struct blk_req {
        unsigned int	type;
        unsigned int	prio;
        unsigned long long	sector;
        struct iovec	*buf;
        int count;
};

static int
blk_check_features(struct virtio_dev *dev)
{
	if (dev->driver_features == dev->device_features)
		return 0;

	return -EINVAL;
}

static int
do_rw(ssize_t (*fn)(int, void *, size_t, off_t), int fd, struct blk_req *req)
{
	off_t	off = req->sector * 512;
	void	*addr;
	int	len, ret = 0;
	int	i;

	for (i = 0; i < req->count; i++) {
		addr = req->buf[i].iov_base;
		len = req->buf[i].iov_len;

		do {
			ret = fn(fd, addr, len, off);
			if (ret <= 0) {
				ret = -1;
				goto out;
			}

			addr += ret;
			len -= ret;
			off += ret;

		} while (len > 0);
	}

out:
	return ret;
}

static int
blk_request(int fd, struct blk_req *req)
{
	int err = 0;

	switch (req->type) {
	case DEV_BLK_TYPE_READ:
		err = do_rw(pread, fd, req);
		break;
	case DEV_BLK_TYPE_WRITE:
		err = do_rw((ssize_t (*)(int, void *, size_t, off_t))pwrite, fd, req);
		break;
	case DEV_BLK_TYPE_FLUSH:
	case DEV_BLK_TYPE_FLUSH_OUT:
		err = fdatasync(fd);
		break;
	default:
		return DEV_BLK_STATUS_UNSUP;
	}

	if (err < 0)
		return DEV_BLK_STATUS_IOERR;

	return DEV_BLK_STATUS_OK;
}

static int
blk_enqueue(struct virtio_dev *dev, int q, struct virtio_req *vreq)
{
	struct virtio_blk_dev	*blk_dev;
	struct virtio_blk_outhdr	*h;
	struct virtio_blk_req_trailer *t;
	struct blk_req	req;

	if (vreq->buf_count < 3)
		goto out;

	h = vreq->buf[0].iov_base;
	t = vreq->buf[vreq->buf_count - 1].iov_base;
	blk_dev = container_of(dev, struct virtio_blk_dev, dev);

	t->status = DEV_BLK_STATUS_IOERR;

	if (vreq->buf[0].iov_len != sizeof(*h))
		goto out;

	if (vreq->buf[vreq->buf_count - 1].iov_len != sizeof(*t))
		goto out;

	req.type = le32toh(h->type);
	req.prio = le32toh(h->ioprio);
	req.sector = le64toh(h->sector);
	req.buf = &vreq->buf[1];
	req.count = vreq->buf_count - 2;

	t->status = blk_request(blk_dev->fd, &req);

out:
	virtio_req_complete(vreq, 0);

	return 0;
}

static struct virtio_dev_ops	blk_ops = {
	.check_features = blk_check_features,
	.enqueue = blk_enqueue,
};

static unsigned long long
get_capacity(int fd)
{
	struct stat	statb;

	if (fstat(fd, &statb) < 0)
		return 0;
	return htole64(statb.st_size / 512);
}

int
ulvisor_add_block(int fd)
{
	struct virtio_blk_dev	*dev;
	int	ret;

	dev = malloc(sizeof(struct virtio_blk_dev));
	if (!dev)
		return -ENOMEM;

	dev->dev.device_id = VIRTIO_ID_BLOCK;
	dev->dev.vendor_id = 0;
	dev->dev.device_features = 0;
	dev->dev.config_gen = 0;
	dev->dev.config_data = &dev->config;
	dev->dev.config_len = sizeof(dev->config);
	dev->dev.ops = &blk_ops;
	dev->fd = fd;

	dev->config.capacity = get_capacity(fd);
	ret = virtio_dev_setup(&dev->dev, 1, 32);
	if (ret)
		goto out_free;

	return dev->dev.virtio_mmio_id;

out_free:
	free(dev);

	return ret;
}
