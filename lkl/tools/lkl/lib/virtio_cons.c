#include <lkl_host.h>
#include "virtio.h"
#include "endian.h"

struct virtio_console_config {
	uint16_t	cols;
	uint16_t	rows;
	uint32_t	max_nr_ports;
	uint32_t	emerg_wr;
};
	
struct virtio_console_dev {
	struct virtio_dev dev;
	struct virtio_console_config config;
};

static int
console_check_features(struct virtio_dev *dev)
{
	if (dev->driver_features == dev->device_features)
		return 0;

	return -LKL_EINVAL;
}

static int
console_enqueue(struct virtio_dev *dev, int q, struct virtio_req *req)
{
	if (q == 1) {
		/* TODO: */
		write(1, (char *)req->buf[0].iov_base, req->buf[0].iov_len);
		virtio_req_complete(req, req->total_len);
	}
	else {
		virtio_req_complete(req, 0);
	}

	return 0;
}

static struct virtio_dev_ops	console_ops = {
	.check_features = console_check_features,
	.enqueue = console_enqueue,
};

int
lkl_console_add(void)
{
	struct virtio_console_dev	*dev;
	int ret;

	dev = lkl_host_ops.mem_alloc(sizeof(*dev));
	if (!dev)
		return -LKL_ENOMEM;

	dev->dev.device_id = LKL_VIRTIO_ID_CONSOLE;
	dev->dev.vendor_id = 0;
	dev->dev.device_features = 0;
	dev->dev.config_gen = 0;
	dev->dev.config_data = &dev->config;
	dev->dev.config_len = sizeof(dev->config);
	dev->dev.ops = &console_ops;

	ret = virtio_dev_setup(&dev->dev, 2, 32);
	if (ret)
		goto out_free;

	return dev->dev.virtio_mmio_id;

out_free:
	lkl_host_ops.mem_free(dev);

	return ret;
}
