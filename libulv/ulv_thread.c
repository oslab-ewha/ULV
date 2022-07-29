#include "ulv_thread.h"
#include "ulv_malloc.h"
#include "ulv_list.h"

typedef char	ulv_jmpbuf[64];
extern int ulv_setjmp(ulv_jmpbuf buf);
extern void ulv_longjmp(ulv_jmpbuf buf, int val);

#ifdef DEBUG
#include <stdio.h>
#define DBG(fmt, ...) printf("PTHDBG: " fmt, ##__VA_ARGS__)
#else
#define DBG(fmt, ...) do {} while (0)
#endif

#define TID_MAX		10000
#define STACK_SIZE	16384

/* align to 16bytes */
#define THSTACK(thinfo)	((char *)((unsigned long)((thinfo)->stack + STACK_SIZE) & 0xfffffffffffffff0))

typedef struct {
	ulv_tid_t	tid;
	int	blocked:1;
	char	*stack;
	struct list_head	list;
	struct list_head	list_ready;
	ulv_jmpbuf	jmpbuf;
} thinfo_t;

static thinfo_t	*cur_thinfo;

static ulv_tid_t	tid_alloc;

static LIST_HEAD(threads);
static LIST_HEAD(threads_ready);

static thinfo_t *
create_thinfo(char *stack)
{
	thinfo_t	*thinfo;

	tid_alloc++;
	if (tid_alloc > TID_MAX)
		tid_alloc = 1;

	thinfo = (thinfo_t *)ulv_malloc(sizeof(thinfo_t));
	thinfo->tid = tid_alloc;
	thinfo->blocked = 0;
	thinfo->stack = stack;
	list_add(&thinfo->list, &threads);
	list_add(&thinfo->list_ready, &threads_ready);
	return thinfo;
}

static thinfo_t *
find_thinfo(ulv_tid_t tid)
{
	struct list_head	*lp;

	list_for_each (lp, &threads) {
		thinfo_t	*thinfo = list_entry(lp, thinfo_t, list_ready);

		if (thinfo->tid == tid)
			return thinfo;
	}

	return NULL;
}

static void
thread_switch(thinfo_t *prev, thinfo_t *next)
{
	DBG("SWITCH: prev:%d -> next: %d\n", prev->tid, next->tid);

	if (ulv_setjmp(prev->jmpbuf)) {
		DBG("thread scheduled again: %d\n", prev->tid);
		cur_thinfo = prev;
		return;
	}
	ulv_longjmp(next->jmpbuf, 1);
	DBG("never reached!!!");
}

static void
setup_thread_stack(thinfo_t *thinfo)
{
	thinfo_t	*thinfo_new;

	DBG("new thread scheduled: %p\n", thinfo);

	asm("movq %0, %%rax;"
	    "movq %1, %%rcx;"
	    "movq %%rcx, %%rsp;"
	    "pushq %%rax;"
	    :: "l"(thinfo), "l"(THSTACK(thinfo)): "rax", "rcx");

	asm("popq %0" : "=l"(thinfo_new));

	cur_thinfo = thinfo_new;
}

ulv_tid_t
ulv_thread_clone(char *stack)
{
	thinfo_t	*thinfo;

	thinfo = create_thinfo(stack);
	if (ulv_setjmp(thinfo->jmpbuf)) {
		setup_thread_stack(thinfo);
		cur_thinfo = thinfo;
		return 0;
	}
	return thinfo->tid;
}

void
ulv_thread_set_blocked(ulv_tid_t tid, int blocked)
{
	thinfo_t	*thinfo = find_thinfo(tid);

	DBG("set blocked: %p, %s\n", thinfo, blocked ? "blocked": "unblocked");

	if (thinfo->blocked == blocked)
		return;

	if (!thinfo->blocked)
		list_del_init(&thinfo->list_ready);
	else
		list_add(&thinfo->list_ready, &threads_ready);
	thinfo->blocked = blocked;
}

void
ulv_thread_exit(ulv_tid_t tid)
{
	thinfo_t	*thinfo = find_thinfo(tid);

	DBG("EXIT: %p\n", thinfo);

	ulv_free(thinfo);
}

ulv_tid_t
ulv_thread_self(void)
{
	return cur_thinfo->tid;
}

void
ulv_thread_reschedule(void)
{
	struct list_head	*lp;

	/* NOTE: Later, consider multi-thread */
	list_for_each (lp, &threads_ready) {
		thinfo_t	*thinfo = list_entry(lp, thinfo_t, list_ready);

		thread_switch(cur_thinfo, thinfo);
		return;
	}
}

void
ulv_thread_init(void)
{
	thinfo_t	*thinfo;

	thinfo = create_thinfo(NULL);
	cur_thinfo = thinfo;
}
