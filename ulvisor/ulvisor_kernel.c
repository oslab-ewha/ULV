#define _GNU_SOURCE

#include <pthread.h>
#include <seccomp.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <linux/futex.h>
#include <sys/time.h>

#include "ulvisor.h"
#include "ulvisor_thread.h"

void start_lkl_kernel(void *mem_start, unsigned long mem_size);
int add_lkl_network(int fd);

long lkl_syscall(long, long *);

static int	fd_sc_listen = -1;
static int	lkl_started;

#include <errno.h>

static int	going_to_shutdown;

extern void kill_app_thread(void);

static void
handle_syscall(struct seccomp_notif *req, struct seccomp_notif_resp *res)
{
	int	ret;

	if (req->data.nr == 231) { /* exit_group */
		kill_app_thread();
		return;
	}
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

static void
handle_syscalls(void)
{
	while (!going_to_shutdown) {
		struct seccomp_notif	*req;
		struct seccomp_notif_resp	*res;

		seccomp_notify_alloc(&req, &res);

		if (seccomp_notify_receive(fd_sc_listen, req) == 0) {
			handle_syscall(req, res);
		}

		seccomp_notify_free(req, res);
	}
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

static int
lkl_kernel_func(void *arg)
{
	listener_ctx_t	*ctx = (listener_ctx_t *)arg;
	extern int	fd_tap;

	if (fd_tap >= 0)
		add_lkl_network(fd_tap);

	start_lkl_kernel(ctx->heap_start, ctx->mem_size);

	free(ctx);

	futex_wakeup(&lkl_started);

	wait_listener_fd_ready();
	handle_syscalls();

	/* TODO: call exit to clean up LKL-driven threads */
	exit(0);

	return 0;
}

void
bootup_lkl(struct ulvisor_boot_info *bi)
{
	listener_ctx_t	*ctx;

	ctx = (listener_ctx_t *)malloc(sizeof(listener_ctx_t));
	ctx->heap_start = (void *)((bi->kernel_end + 4096 - 1) & ~4095);
	ctx->mem_size = bi->mem_size - bi->kernel_end;

	start_thread(lkl_kernel_func, 8192, ctx);
	futex_wait(&lkl_started);
}

void
notify_sc_listen_fd(int fd)
{
	fd_sc_listen = fd;
}
