#include "ulv_types.h"
#include "ulv_thread.h"
#include "ulv_malloc.h"
#include "ulv_list.h"
#include "ulv_atomic.h"
#include "ulv_assert.h"

typedef char	ulv_jmpbuf[72];
extern int ulv_setjmp(ulv_jmpbuf buf);
extern int ulv_setjmp_clone(ulv_jmpbuf buf, char *stack, void *tls, void *thinfo, void *fn, void *arg);
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
	unsigned	blocked:1;
	char	*stack;
	struct list_head	list;
	struct list_head	list_ready;
	ulv_jmpbuf	jmpbuf;
} thinfo_t;

static thinfo_t	*thinfo_cur;
static thinfo_t	*thinfo_main;

static ulv_tid_t	tid_alloc;

static unsigned	n_threads;

static int	locked_ready;

static LIST_HEAD(threads);
static LIST_HEAD(threads_ready);

thinfo_t *
create_thinfo(char *stack)
{
	thinfo_t	*thinfo;

	n_threads++;
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

static void
free_thinfo(thinfo_t *thinfo)
{
	list_del_init(&thinfo->list);
	ULV_ASSERT(n_threads > 0);
	n_threads--;
	ulv_free(thinfo);
}

static thinfo_t *
find_thinfo(ulv_tid_t tid)
{
	struct list_head	*lp;

	list_for_each (lp, &threads) {
		thinfo_t	*thinfo = list_entry(lp, thinfo_t, list);

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
		thinfo_cur = prev;
		return;
	}
	ulv_longjmp(next->jmpbuf, 1);
	DBG("never reached!!!");
}

int
__clone(int (*fn)(void *), void *stack, int flags, void *arg, pid_t *parent_tid, void *tls, pid_t *child_tid)
{
	thinfo_t	*thinfo;

	thinfo = create_thinfo(stack);

	if (ulv_setjmp_clone(thinfo->jmpbuf, stack, tls, thinfo, fn, arg)) {
		asm("popq %0\n\tpopq %1\n\tpopq %2" : "=l"(thinfo), "=l"(arg), "=l"(fn));
		thinfo_cur = thinfo;

		fn(arg);
		ULV_PANIC("never reach");
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

	ulv_spin_lock(&locked_ready);

	if (!thinfo->blocked)
		list_del_init(&thinfo->list_ready);
	else
		list_add(&thinfo->list_ready, &threads_ready);

	ulv_spin_unlock(&locked_ready);

	thinfo->blocked = blocked;
}

static thinfo_t *
get_ready_thread(void)
{
	/* NOTE: Later, consider multi-thread */
	if (list_empty(&threads_ready))
		return NULL;

	return list_entry(threads_ready.next, thinfo_t, list_ready);
}

static thinfo_t *
get_ready_thread_safe(void)
{
	thinfo_t	*thinfo;

	ulv_spin_lock(&locked_ready);
	while ((thinfo = get_ready_thread()) == NULL) {
		ulv_spin_unlock(&locked_ready);
		/* do what ? */
		ulv_spin_lock(&locked_ready);
	}
	ulv_spin_unlock(&locked_ready);

	return thinfo;
}

void
ulv_thread_exit(void)
{
	thinfo_t	*thinfo_ready;

	DBG("EXIT: %p\n", thinfo_cur);

	list_del_init(&thinfo_cur->list_ready);
	free_thinfo(thinfo_cur);

	thinfo_cur = thinfo_ready = get_ready_thread_safe();
	ulv_longjmp(thinfo_ready->jmpbuf, 1);
}

ulv_tid_t
ulv_thread_self(void)
{
	return thinfo_cur->tid;
}

void
ulv_thread_reschedule(void)
{
	thinfo_t	*thinfo_ready;

	thinfo_ready = get_ready_thread_safe();
	thread_switch(thinfo_cur, thinfo_ready);
}

bool_t
ulv_is_last_thread(void)
{
	if (n_threads == 1)
		return TRUE;
	return FALSE;
}

bool_t
ulv_is_main_thread(void)
{
	if (thinfo_main == thinfo_cur)
		return TRUE;
	return FALSE;
}

void
ulv_thread_init(void)
{
	thinfo_t	*thinfo;

	thinfo = create_thinfo(NULL);
	thinfo_main = thinfo_cur = thinfo;
}
