#define _GNU_SOURCE
#include <sched.h>
#include <seccomp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>

#include "lkl_thread.h"

void notify_sc_listen_fd(int fd);

static int
func_user_thread(void *arg)
{
	scmp_filter_ctx	ctx_filter;
	int	fd_notify;

	ctx_filter = seccomp_init(SCMP_ACT_NOTIFY);
	if (ctx_filter == NULL) {
		err(1, "failed: seccomp_init");
	}

	/* One dummy rule should be added. Because zero filter count incurs invalid error. */
	seccomp_rule_add(ctx_filter, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);

	if (seccomp_load(ctx_filter) < 0) {
		err(1, "failed: seccomp_load");
	}

	fd_notify = seccomp_notify_fd(ctx_filter);
	if (fd_notify < 0) {
		err(1, "failed: seccomp_notify_fd");
	}

	notify_sc_listen_fd(fd_notify);

	((void (*)(void *))arg)(NULL);

	/* notify shutdown to lkl kernel */
	syscall(1000);
	seccomp_release(ctx_filter);

	return 0;
}

void
run_user_thread(void (*start_fn)(void *))
{
	pid_t	pid;

	pid = start_thread(func_user_thread, 4096, start_fn);
	waitpid(pid, NULL, 0);
}
