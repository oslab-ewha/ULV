/*
 * Copyright (c) 2015-2019 Contributors as noted in the AUTHORS file
 *
 * This file is part of Solo5, a sandboxed execution environment.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <err.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <seccomp.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

#include <sys/uio.h>

#include "tap_attach.h"
#include "ulvisor.h"

int	fd_tap = -1;

static int	epollfd;

extern void ulvisor_add_netdev(void);

static int
handle_cmdarg(char *cmdarg)
{
	char	iface[20];
	int	rc;

	if (strncmp("--net=", cmdarg, 6) != 0)
		return -1;

	rc = sscanf(cmdarg, "--net=%19s", iface);
        if (rc != 1)
		return -1;

        fd_tap = tap_attach(iface);
        if (fd_tap < 0) {
		warnx("Could not attach interface: %s", iface);
		return -1;
        }

	return 0;
}

static int
setup(ulvisor_t *ulvisor)
{
	struct epoll_event	epev;

	if (fd_tap < 0)
		return 0;

	epollfd = ulvisor->epollfd;

	epev.events = EPOLLIN | EPOLLOUT;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_tap, &epev) < 0) {
		err(1, "epoll_ctl(EPOLL_CTL_ADD, fd=%d) failed", fd_tap);
	}

	ulvisor_add_netdev();

	return 0;
}

static char *
usage(void)
{
	return "--net=IFACE (attach tap at IFACE)\n";
}

static ssize_t
net_read(uint8_t *buf, size_t size)
{
	size_t	nread_total = 0;
	ssize_t	nread;

again:
	nread = read(fd_tap, buf, size);
	if (nread < 0) {
		if (errno != EAGAIN)
			return -EIO;
		return nread_total;
	}

	nread_total += nread;
	if (nread < size) {
		buf += nread;
		size -= nread;
		goto again;
	}

	return nread_total;
}

static int
net_write(const uint8_t *buf, size_t size)
{
	size_t	nwrite_total = 0;
	ssize_t	nwrite;

again:
	nwrite = write(fd_tap, (const void *)buf, size);
	if (nwrite < 0)
		return -EIO;

	nwrite_total += nwrite;
	if (nwrite < size) {
		buf += nwrite;
		size -= nwrite;
		goto again;
	}

	return nwrite_total;
}

int
ulvisor_net_read(struct iovec *iov, int cnt)
{
	int	nread_total = 0;
	int	i;

	for (i = 0; i < cnt; i++, iov++) {
		ssize_t	nread;

		if ((nread = net_read(iov->iov_base, iov->iov_len)) < 0)
			return -1;

		nread_total += nread;
		if (nread < iov->iov_len)
			break;
	}
	return nread_total;
}

int
ulvisor_net_write(struct iovec *iov, int cnt)
{
	int	nwrite_total = 0;
	int	i;

	for (i = 0; i < cnt; i++, iov++) {
		int	nwrite;
		int	res;

		if ((nwrite = net_write(iov->iov_base, iov->iov_len)) < 0)
			return -1;

		nwrite_total += nwrite;
		if (nwrite < iov->iov_len)
			break;
	}
	return nwrite_total;
}

int
ulvisor_net_poll(void)
{
	while (1) {
		struct epoll_event	epev;
		int	ret;

		ret = epoll_wait(epollfd, &epev, 1, -1);
		if (ret > 0) {
			int	evset = 0;
			if (epev.events & EPOLLIN)
				evset += 1;
			if (epev.events & EPOLLOUT)
				evset += 2;
			return evset;
		}
	}

	return 0;
}

DECLARE_MODULE(net,
	       .setup = setup,
	       .handle_cmdarg = handle_cmdarg,
	       .usage = usage)
