#define _GNU_SOURCE

#include <pthread.h>
#include <seccomp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "spt_abi.h"

static pthread_t	syscall_listener;

void init_liblkl(void *mem_start, unsigned long mem_size, void *handle);
long lkl_syscall(long, long *);

static int	fd_sc_listen = -1;

#include <errno.h>

static void
handle_syscall(struct seccomp_notif *req, struct seccomp_notif_resp *res)
{
	int	ret;

	ret = lkl_syscall(req->data.nr, (long int *)req->data.args);

	res->id = req->id;
	if (ret < 0) {
		res->val = -1;
		res->error = ret;
	}
	else {
		res->val = ret;
		res->error = 0;
	}
	res->flags = 0;
	if ((ret = seccomp_notify_respond(fd_sc_listen, res)) < 0) {
		/* TODO: ?? */
	}
}

static void *
handle_syscalls(void)
{
	while (1) {
		struct seccomp_notif	*req;
		struct seccomp_notif_resp	*res;

		seccomp_notify_alloc(&req, &res);
		if (seccomp_notify_receive(fd_sc_listen, req) == 0) {
			handle_syscall(req, res);
		}
		seccomp_notify_free(req, res);
	}
	return NULL;
}

static void
wait_listener_fd_ready(void)
{
	while (1) {
		if (fd_sc_listen >= 0)
			break;
		usleep(10);
	}
}

typedef struct {
	void	*heap_start;
	unsigned long	mem_size;
} listener_ctx_t;

static void *
lkl_kernel_func(void *arg)
{
	listener_ctx_t	*ctx = (listener_ctx_t *)arg;

	init_liblkl(ctx->heap_start, ctx->mem_size, NULL);
	wait_listener_fd_ready();
	handle_syscalls();
	free(ctx);

	return NULL;
}

void
bootup_lkl(struct spt_boot_info *bi)
{
	listener_ctx_t	*ctx;

	ctx = (listener_ctx_t *)malloc(sizeof(listener_ctx_t));
	ctx->heap_start = (void *)((bi->kernel_end + 4096 - 1) & ~4096);
	ctx->mem_size = bi->mem_size - bi->kernel_end;

	pthread_create(&syscall_listener, NULL, lkl_kernel_func, ctx);
}

void
notify_sc_listen_fd(int fd)
{
	fd_sc_listen = fd;
}
