#define _GNU_SOURCE
#include <sched.h>
#include <seccomp.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <err.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>

#include "sl5.h"
#include "sl5_thread.h"

void notify_sc_listen_fd(int fd);

void sl5_launch(void *stack, void *start_fn);
static int	futex_killed;

static void *
setup_stacks(void *stack[], int argc, char *argv[])
{
	int	i;

	stack--;
	*stack = NULL;
	stack--;
	*stack = NULL;

	stack -= argc;
	for (i = 0; i < argc; i++)
		stack[i] = argv[i];
	stack--;
	*stack = (void *)(long long)argc;
	return stack;
}

static int
func_user_thread(void *arg)
{
	struct sl5	*sl5 = (struct sl5 *)arg;
	scmp_filter_ctx	ctx_filter;
	void	**stack;
	int	fd_notify;

	stack = mmap(NULL, 1024 * sizeof(void *), PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	if (stack == MAP_FAILED)
		err(1, "failed to mmap for create_thread()");

	ctx_filter = seccomp_init(SCMP_ACT_NOTIFY);
	if (ctx_filter == NULL) {
		err(1, "failed: seccomp_init");
	}

	/* One dummy rule should be added. Because zero filter count incurs invalid error. */
	seccomp_rule_add(ctx_filter, SCMP_ACT_ALLOW, 1000, 0);

	if (seccomp_load(ctx_filter) < 0) {
		err(1, "failed: seccomp_load");
	}

	fd_notify = seccomp_notify_fd(ctx_filter);
	if (fd_notify < 0) {
		err(1, "failed: seccomp_notify_fd");
	}

	notify_sc_listen_fd(fd_notify);

	/* TODO: stack + 1024 causes seg fault */
	stack = setup_stacks(stack + 1023, sl5->argc, sl5->argv);
	sl5_launch(stack, sl5->start_fn);

	/* Not reach */
	seccomp_release(ctx_filter);

	return 0;
}

void
run_user_thread(struct sl5 *sl5)
{
	start_thread(func_user_thread, 4096, sl5);
	futex_wait(&futex_killed);
}

void
kill_user_thread(void)
{
	futex_wakeup(&futex_killed);
}
