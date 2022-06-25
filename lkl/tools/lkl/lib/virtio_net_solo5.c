#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <net/if.h>
#include <errno.h>

#include "virtio.h"

struct lkl_netdev_solo5 {
	struct lkl_netdev	dev;
	int	fd;
};

int solo5_net_read(int fd_tap, uint8_t *buf, size_t size, size_t *read_size);
int solo5_net_write(int fd_tap, const uint8_t *buf, size_t size);
int solo5_yield(void);

static int
net_tx(struct lkl_netdev *nd, struct iovec *iov, int cnt)
{
	struct lkl_netdev_solo5	*nd_solo5 = container_of(nd, struct lkl_netdev_solo5, dev);
	int	i, ret = 0;

	for (i = 0; i < cnt; i++) {
		int	res;

		if ((res = solo5_net_write(nd_solo5->fd, iov[i].iov_base, iov[i].iov_len)) != 0) {
			if (res != -EAGAIN) {
				char	buf[4096];
				snprintf(buf, 4096, "failed to write:%d\n", res);
				perror(buf);
			}
			return 0;
		}
		ret += iov[i].iov_len;
	}
	return ret;
}

static int
net_rx(struct lkl_netdev *nd, struct iovec *iov, int cnt)
{
	struct lkl_netdev_solo5	*nd_solo5 = container_of(nd, struct lkl_netdev_solo5, dev);
	int	i, ret = 0;
	int	offset = 0;

	for (i = 0; i < cnt;) {
		int	res;
		size_t	size, nread;

		size = iov[i].iov_len - offset;

		if ((res = solo5_net_read(nd_solo5->fd, iov[i].iov_base + offset, size, &nread)) != 0) {
			if (res != -EAGAIN) {
				perror("failed to read from net\n");
				return -1;
			}
			return ret;
		}
		ret += nread;
		if (size == nread) {
			i++;
			offset = 0;
		}
		else
			offset += nread;
	}
	return ret;
}

static int
net_poll(struct lkl_netdev *nd)
{
	return solo5_yield();
}

static void
net_poll_hup(struct lkl_netdev *nd)
{
	//TODO:
	perror("HUP not implemented\n");
}

static void
net_free(struct lkl_netdev *nd)
{
	struct lkl_netdev_solo5	*nd_solo5 = container_of(nd, struct lkl_netdev_solo5, dev);

	free(nd_solo5);
}

struct lkl_dev_net_ops	solo5_net_ops =  {
	.tx = net_tx,
	.rx = net_rx,
	.poll = net_poll,
	.poll_hup = net_poll_hup,
	.free = net_free,
};

struct lkl_netdev *
lkl_netdev_solo5_create(int fd)
{
	struct lkl_netdev_solo5	*nd;

	nd = (struct lkl_netdev_solo5 *)malloc(sizeof(struct lkl_netdev_solo5));
	if (!nd) {
		fprintf(stderr, "solo5 netdev: failed to allocate memory\n");
		return NULL;
	}
	nd->dev.ops = &solo5_net_ops;
	nd->dev.has_vnet_hdr = 0;
	nd->fd = fd;
	return &nd->dev;
}
