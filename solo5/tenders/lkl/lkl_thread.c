#define _GNU_SOURCE
#include <sched.h>
#include <linux/futex.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdatomic.h>
#include <stdio.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <err.h>

#include "lkl_thread.h"

static int
futex(int *uaddr, int futex_op, int val, const struct timespec *timeout, int *uaddr2, int val3)
{
	return syscall(SYS_futex, uaddr, futex_op, val, timeout, uaddr, val3);
}

void
futex_wait(int *futexp)
{
	while (1) {
		const int	one = 1;

		if (atomic_compare_exchange_strong(futexp, &one, 0))
			break;

		futex(futexp, FUTEX_WAIT, 0, NULL, NULL, 0);
	}
}

void
futex_wakeup(int *futexp)
{
	/* atomic_compare_exchange_strong() was described in comments above */

	const int	zero = 0;
	if (atomic_compare_exchange_strong(futexp, &zero, 1)) {
		futex(futexp, FUTEX_WAKE, 1, NULL, NULL, 0);
	}
}

pid_t
start_thread(thread_func_t func, int stack_size, void *ctx)
{
	char	*stack;
	pid_t	pid;

	stack = mmap(NULL, stack_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (stack == MAP_FAILED) {
		err(1, "failed to mmap for create_thread()");
	}
	pid = clone(func, stack + stack_size, CLONE_VM | CLONE_FILES | SIGCHLD, ctx);
	if (pid == -1)
		err(1, "failed to clone");

	return pid;
}
