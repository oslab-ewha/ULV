#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <net/if.h>

#include "virtio.h"
#include "../../../../solo5/include/solo5/solo5.h"

struct lkl_netdev_solo5 {
	struct lkl_netdev	dev;
	solo5_handle_t	handle;
};

static int
solo5_net_tx(struct lkl_netdev *nd, struct iovec *iov, int cnt)
{
	struct lkl_netdev_solo5	*nd_solo5 = container_of(nd, struct lkl_netdev_solo5, dev);
	int	i, ret = 0;

	for (i = 0; i < cnt; i++) {
		solo5_result_t	res;

		if ((res = solo5_net_write(nd_solo5->handle, iov[i].iov_base, iov[i].iov_len)) != SOLO5_R_OK) {
			if (res != SOLO5_R_AGAIN) {
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
solo5_net_rx(struct lkl_netdev *nd, struct iovec *iov, int cnt)
{
	struct lkl_netdev_solo5	*nd_solo5 = container_of(nd, struct lkl_netdev_solo5, dev);
	int	i, ret = 0;
	int	offset = 0;

	for (i = 0; i < cnt;) {
		solo5_result_t	res;
		size_t	size, nread;

		size = iov[i].iov_len - offset;

		if ((res = solo5_net_read(nd_solo5->handle, iov[i].iov_base + offset, size, &nread)) != SOLO5_R_OK) {
			if (res != SOLO5_R_AGAIN) {
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
solo5_net_poll(struct lkl_netdev *nd)
{
	struct lkl_netdev_solo5	*nd_solo5 = container_of(nd, struct lkl_netdev_solo5, dev);
	solo5_handle_set_t	ready_set = 0;

	solo5_yield(solo5_clock_monotonic() + 5000000000, &ready_set);
	if (ready_set & 1U << nd_solo5->handle) {
		return (LKL_DEV_NET_POLL_RX | LKL_DEV_NET_POLL_TX);
	}
	return 0;
}

static void
solo5_net_poll_hup(struct lkl_netdev *nd)
{
	//TODO:
	perror("HUP not implemented\n");
}

static void
solo5_net_free(struct lkl_netdev *nd)
{
	struct lkl_netdev_solo5	*nd_solo5 = container_of(nd, struct lkl_netdev_solo5, dev);

	free(nd_solo5);
}

struct lkl_dev_net_ops	solo5_net_ops =  {
	.tx = solo5_net_tx,
	.rx = solo5_net_rx,
	.poll = solo5_net_poll,
	.poll_hup = solo5_net_poll_hup,
	.free = solo5_net_free,
};

struct lkl_netdev *
lkl_netdev_solo5_create(solo5_handle_t handle)
{
	struct lkl_netdev_solo5	*nd;

	nd = (struct lkl_netdev_solo5 *)malloc(sizeof(struct lkl_netdev_solo5));
	if (!nd) {
		fprintf(stderr, "solo5 netdev: failed to allocate memory\n");
		return NULL;
	}
	nd->dev.ops = &solo5_net_ops;
	nd->handle = handle;
	return &nd->dev;
}
