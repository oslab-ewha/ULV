#include <setjmp.h>
#include <stdlib.h>

#include "ulvisor_pure_thread.h"

#ifdef DEBUG
#include <stdio.h>
#define DBG(fmt, ...) printf("PTHDBG: " fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...) do {} while (0)
#endif

#define STACK_SIZE	16384

#define THSTACK(thinfo)	((thinfo)->stack + STACK_SIZE - 8)

typedef struct {
	int	started:1;
	void *(*fn)(void *);
	void	*arg;
	char	stack[STACK_SIZE];
	jmp_buf	jmpbuf;
} thinfo_t;

static thinfo_t	*cur_thinfo;

static thinfo_t *
create_thinfo(void *(*fn)(void *), void *arg)
{
	thinfo_t	*thinfo;

	thinfo = (thinfo_t *)malloc(sizeof(thinfo_t));
	thinfo->started = 0;
	thinfo->fn = fn;
	thinfo->arg = arg;
	return thinfo;
}

static void
start_new_thread(thinfo_t *thinfo)
{
	DBG("new thread scheduled: %p\n", thinfo);

	asm("movq %0, %%rax;"
	    "movq %1, %%rbp;"
	    "movq %%rbp, %%rsp;"
	    "pushq %%rax;"
	    :: "l"(thinfo), "l"(THSTACK(thinfo)): "rax", "rbp");

	{
		thinfo_t	*thinfo_new;

		asm("popq %0" : "=l"(thinfo_new));

		thinfo_new->started = 1;
		cur_thinfo = thinfo_new;
		thinfo_new->fn(thinfo_new->arg);
	}
}

void
pure_thread_switch(lkl_thread_t prev, lkl_thread_t next)
{
	DBG("SWITCH: prev:%lx -> next: %lx\n", prev, next);

	if (prev) {
		thinfo_t	*thinfo_prev = (thinfo_t *)prev;

		if (setjmp(thinfo_prev->jmpbuf)) {
			DBG("thread scheduled again: %p\n", thinfo_prev);
			cur_thinfo = thinfo_prev;
			return;
		}
	}

	if (next) {
		thinfo_t	*thinfo_next = (thinfo_t *)next;

		if (thinfo_next->started) {
			longjmp(thinfo_next->jmpbuf, 1);
		}
		else {
			start_new_thread(thinfo_next);
		}
		DBG("never reached!!!");
	}
	else {
		thinfo_t	*thinfo = create_thinfo(NULL, NULL);

		thinfo->started = 1;
		cur_thinfo = thinfo;

		DBG("created by switch: %p\n", thinfo);
	}
}

lkl_thread_t
pure_thread_create(void *(*fn)(void *), void *arg)
{
	thinfo_t	*thinfo;

	thinfo = create_thinfo(fn, arg);

	DBG("created: %p\n", thinfo);

	return (lkl_thread_t)thinfo;
}

void
pure_thread_exit(lkl_thread_t lthrd)
{
	thinfo_t	*thinfo = (thinfo_t *)lthrd;

	DBG("EXIT: %p\n", thinfo);

	free(thinfo);
}

lkl_thread_t
pure_thread_self(void)
{
	return (lkl_thread_t)cur_thinfo;
}
