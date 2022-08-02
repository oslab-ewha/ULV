#include "ulv_types.h"
#include "ulv_worker.h"
#include "ulv_list.h"
#include "ulv_syscall_no.h"
#include "ulv_syscall_flags.h"
#include "ulv_host_syscall.h"
#include "ulv_thread.h"
#include "ulv_malloc.h"
#include "ulv_atomic.h"

#define FD_SETSIZE 1024

typedef unsigned long fd_mask;

typedef struct {
        unsigned long fds_bits[FD_SETSIZE / 8 / sizeof(long)];
} fd_set;

#define FD_ZERO(s) do { int __i; unsigned long *__b=(s)->fds_bits; for(__i=sizeof (fd_set)/sizeof (long); __i; __i--) *__b++=0; } while(0)
#define FD_SET(d, s)   ((s)->fds_bits[(d)/(8*sizeof(long))] |= (1UL<<((d)%(8*sizeof(long)))))
#define FD_CLR(d, s)   ((s)->fds_bits[(d)/(8*sizeof(long))] &= ~(1UL<<((d)%(8*sizeof(long)))))
#define FD_ISSET(d, s) !!((s)->fds_bits[(d)/(8*sizeof(long))] & (1UL<<((d)%(8*sizeof(long)))))

struct timeval {
	long	tv_sec;
	long	tv_usec;
};

typedef struct epoll_event {
	uint32_t	events;
	void	*data;
} __attribute__((packed)) epoll_event_t;

struct epoller_req;

typedef struct {
	int	fd;
	struct epoller_req	*preq;
} epolled_fdinfo_t;

typedef struct epoller_req {
	ulv_tid_t	tid;
	int	done;
	int	nfds;
	fd_set	*rdset, *wrset, *exset;
	struct timeval *timeout;
	epolled_fdinfo_t	*fdinfos;
	struct list_head	list;
} epoller_req_t;

static ulv_worker_t	worker_epoller;
static int	epfd;
static int	pipefds[2];

static int	locked;

static LIST_HEAD(reqs_new);
static LIST_HEAD(reqs_done);

static void
add_epoller_req(epoller_req_t *preq)
{
	char	dummy = '\0';

	ulv_spin_lock(&locked);
	list_add_tail(&preq->list, &reqs_new);
	preq->tid = ulv_thread_self();
	ulv_spin_unlock(&locked);

	__syscall3(__NR_write, pipefds[1], (long)&dummy, 1);
}

static void
accept_req(epoller_req_t *preq)
{
	epoll_event_t	event;
	epolled_fdinfo_t	*fdinfo;
	int	i;

	preq->fdinfos = (epolled_fdinfo_t *)ulv_malloc(sizeof(epolled_fdinfo_t) * preq->nfds);

	for (i = 0, fdinfo = preq->fdinfos; i < preq->nfds; i++, fdinfo++) {
		fdinfo->fd = i;
		fdinfo->preq = preq;
		event.events = 0;
		event.data = fdinfo;
		if (preq->rdset && FD_ISSET(i, preq->rdset))
			event.events |= EPOLLIN;
		if (preq->wrset && FD_ISSET(i, preq->wrset))
			event.events |= EPOLLOUT;
		if (preq->exset && FD_ISSET(i, preq->exset))
			event.events |= EPOLLERR;
		if (event.events != 0)
			__syscall4(__NR_epoll_ctl, (long)epfd, (long)EPOLL_CTL_ADD, (long)i, (long)&event);
	}
}

static void
accept_new_epoller_req(void)
{
	ulv_spin_lock(&locked);
	while (!list_empty(&reqs_new)) {
		epoller_req_t	*preq = list_entry(reqs_new.next, epoller_req_t, list);

		list_del_init(&preq->list);
		ulv_spin_unlock(&locked);

		accept_req(preq);

		ulv_spin_lock(&locked);
	}
	ulv_spin_unlock(&locked);
}

static void
clear_added_epolls(epoller_req_t *preq)
{
	int	i;

	for (i = 0; i < preq->nfds; i++) {
		if ((preq->rdset && FD_ISSET(i, preq->rdset)) ||
		    (preq->wrset && FD_ISSET(i, preq->wrset)) ||
		    (preq->exset && FD_ISSET(i, preq->exset))) {
			__syscall4(__NR_epoll_ctl, (long)epfd, (long)EPOLL_CTL_DEL, (long)i, 0);
		}
	}
}

static void
check_epolled_req(epoll_event_t *pevent)
{
	epolled_fdinfo_t	*fdinfo = (epolled_fdinfo_t *)pevent->data;
	epoller_req_t	*preq = fdinfo->preq;

	if (list_empty(&preq->list)) {
		clear_added_epolls(preq);
		if (preq->rdset)
			FD_ZERO(preq->rdset);
		if (preq->wrset)
			FD_ZERO(preq->wrset);
		if (preq->exset)
			FD_ZERO(preq->exset);
		preq->nfds = 0;
		list_add_tail(&preq->list, &reqs_done);
	}
	if (pevent->events & EPOLLIN)
		FD_SET(fdinfo->fd, preq->rdset);
	if (pevent->events & EPOLLOUT)
		FD_SET(fdinfo->fd, preq->wrset);
	if (pevent->events & EPOLLERR)
		FD_SET(fdinfo->fd, preq->exset);
	if (pevent->events & (EPOLLIN | EPOLLOUT | EPOLLERR))
		preq->nfds++;
}

static void
done_epolled_req(void)
{
	struct list_head	*lp, *next;

	list_for_each_n (lp, &reqs_done, next) {
		epoller_req_t	*preq = list_entry(lp, epoller_req_t, list);

		ulv_free(preq->fdinfos);
		list_del_init(&preq->list);

		ulv_thread_set_blocked(preq->tid, 0);
		preq->done = 1;
	}
}

static void
epoller_func(void)
{
	while (1) {
		epoll_event_t	event;
		int	ret;

		ret = (int)__syscall4(__NR_epoll_wait, (long)epfd, (long)&event, 1, 100000);
		if (ret > 0) {
			int	i;

			for (i = 0; i < ret; i++) {
				if (event.data != 0)
					check_epolled_req(&event);
			}
		}
		done_epolled_req();
		accept_new_epoller_req();
	}

	__syscall1(__NR_exit, 0);
}

inline static void
check_epoller(void)
{
	epoll_event_t	event;

	if (worker_epoller)
		return;

	epfd = __syscall1(__NR_epoll_create1, 0);
	__syscall1(__NR_pipe, (long)pipefds);

	event.data = 0;
	event.events = EPOLLIN;

	__syscall4(__NR_epoll_ctl, (long)epfd, (long)EPOLL_CTL_ADD, (long)pipefds[0], (long)&event);

	worker_epoller = ulv_start_worker(epoller_func);
}

int
epoller_select_events(int nfds, fd_set *rdset, fd_set *wrset, fd_set *exset, struct timeval *timeout)
{
	epoller_req_t	req;

	check_epoller();

	req.done = 0;
	req.nfds = nfds;
	req.rdset = rdset;
	req.wrset = wrset;
	req.exset = exset;
	req.timeout = timeout;

	add_epoller_req(&req);

	ulv_thread_set_blocked(ulv_thread_self(), 1);
	while (!req.done)
		ulv_thread_reschedule();

	return req.nfds;
}
