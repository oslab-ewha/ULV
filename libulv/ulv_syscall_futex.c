#include <errno.h>
#include <limits.h>

#include "ulv_types.h"
#include "ulv_list.h"
#include "ulv_malloc.h"
#include "ulv_thread.h"
#include "ulv_assert.h"

#define FUTEX_WAIT 0
#define FUTEX_WAKE 1

#define FUTEX_PRIVATE_FLAG	128
#define FUTEX_CLOCK_REALTIME	256
#define FUTEX_CMD_MASK		~(FUTEX_PRIVATE_FLAG | FUTEX_CLOCK_REALTIME)

#define MAX_WAITERS	10

#define WAITER_DONE(futex, idx)	((futex)->waiters[idx] != ulv_thread_self())

typedef struct _futex {
	int	*addr;
	ulv_tid_t	waiters[MAX_WAITERS];
	struct list_head	list;
} futex_t;

static LIST_HEAD(futexes);

static inline futex_t *
find_futex(int *addr)
{
	struct list_head	*lp;

	list_for_each (lp, &futexes) {
		futex_t	*futex = list_entry(lp, futex_t, list);

		if (futex->addr == addr)
			return futex;
	}
	return NULL;
}

static inline unsigned
reg_waiter(futex_t *futex)
{
	int	i;

	for (i = 0; i < MAX_WAITERS; i++) {
		if (futex->waiters[i] == 0) {
			futex->waiters[i] = ulv_thread_self();
			return i;
		}
	}

	ULV_PANIC("futex waiter full!!");
	return 0;
}

static inline ulv_tid_t
pick_waiter(futex_t *futex)
{
	int	i;

	for (i = 0; i < MAX_WAITERS; i++) {
		if (futex->waiters[i] != 0) {
			ulv_tid_t	waiter;

			waiter = futex->waiters[i];
			futex->waiters[i] = 0;
			return waiter;
		}
	}

	return 0;
}

inline static int
futex_wait(int *addr, int val)
{
	futex_t	*futex;
	int	f_val = *addr;
	unsigned	idx;

	if (f_val != val)
		return -EAGAIN;

	futex = find_futex(addr);
	if (futex == NULL) {
		futex = (futex_t *)ulv_malloc(sizeof(futex_t));
		futex->addr = addr;
	}
	else
		list_del(&futex->list);

	list_add(&futex->list, &futexes);

	idx = reg_waiter(futex);

	while (!WAITER_DONE(futex, idx)) {
		ulv_thread_set_blocked(ulv_thread_self(), 1);
		ulv_thread_reschedule();
	}

	return 0;
}

inline static int
futex_wake(int *addr, int nwaiters)
{
	futex_t	*futex;
	unsigned	n_waked = 0;

	futex = find_futex(addr);
	if (futex) {
		ulv_tid_t	waiter;

		while (nwaiters > 0) {
			waiter = pick_waiter(futex);
			if (waiter == 0)
				break;
			ulv_thread_set_blocked(waiter, 0);
			n_waked++;
			nwaiters--;
		}
	}

	return n_waked;
}

int
ulv_syscall_futex(int *uaddr, int futex_op, int val, long timeout, int *uaddr2, int val3)
{
	if (unlikely(uaddr == NULL))
		return -EINVAL;

	switch (futex_op & FUTEX_CMD_MASK) {
	case FUTEX_WAIT:
		return futex_wait(uaddr, val);
	case FUTEX_WAKE:
		if (val != 1 && val != INT_MAX)
			return -ENOSYS;
		return futex_wake(uaddr, val);
	default:
		return -ENOSYS;
	}
	return -ENOSYS;
}
