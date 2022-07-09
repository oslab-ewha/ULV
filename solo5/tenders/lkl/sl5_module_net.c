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
#include <sys/timerfd.h>
#include <unistd.h>
#include <errno.h>

#include "../common/tap_attach.h"
#include "sl5.h"

int	fd_tap = -1;

static int	epollfd;
static int	timerfd;

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
setup(struct sl5 *sl5)
{
	struct epoll_event	epev;

	if (fd_tap < 0)
		return 0;

	epollfd = sl5->epollfd;
	timerfd = sl5->timerfd;

	epev.events = EPOLLIN | EPOLLOUT;
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd_tap, &epev) < 0) {
		err(1, "epoll_ctl(EPOLL_CTL_ADD, fd=%d) failed", fd_tap);
	}

	return 0;
}

static char *
usage(void)
{
	return "--net=IFACE (attach tap at IFACE)\n";
}

int
solo5_net_read(int fd_tap, uint8_t *buf, size_t size, size_t *read_size)
{
	long	nbytes;

	nbytes = read(fd_tap, (char *)buf, size);
	if (nbytes < 0) {
		if (errno == EAGAIN)
			return -EAGAIN;
		else
			return -EIO;
	}

	*read_size = (size_t)nbytes;
	return 0;
}

int
solo5_net_write(int fd_tap, const uint8_t *buf, size_t size)
{
	long	nbytes;

	nbytes = write(fd_tap, (const char *)buf, size);
	return (nbytes == (int)size) ? 0: -EIO;
}

int
solo5_yield(void)
{
	/*
	 * We can always safely restart this call on EINTR, since the internal
	 * timerfd is independent of its invocation.
	 */
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
