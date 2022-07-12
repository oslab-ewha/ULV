#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <poll.h>
#include <lkl_host.h>
#include "iomem.h"
#include "jmp_buf.h"
#include "pure_thread.h"

/* Let's see if the host has semaphore.h */
#include <unistd.h>

#ifdef _POSIX_SEMAPHORES
#include <semaphore.h>
/* TODO(pscollins): We don't support fork() for now, but maybe one day
 * we will? */
#define SHARE_SEM 0
#endif /* _POSIX_SEMAPHORES */

static void print(const char *str, int len)
{
	write(1, str, len);
}

struct lkl_sem {
#ifdef _POSIX_SEMAPHORES
	sem_t sem;
#else
	pthread_mutex_t lock;
	int count;
	pthread_cond_t cond;
#endif /* _POSIX_SEMAPHORES */
};

struct lkl_tls_key {
	pthread_key_t key;
};

#define WARN_UNLESS(exp) do {						\
		if (exp < 0)						\
			lkl_printf("%s: %s\n", #exp, strerror(errno));	\
	} while (0)

static int _warn_pthread(int ret, char *str_exp)
{
	if (ret > 0)
		lkl_printf("%s: %s\n", str_exp, strerror(ret));

	return ret;
}


/* pthread_* functions use the reverse convention */
#define WARN_PTHREAD(exp) _warn_pthread(exp, #exp)

static struct lkl_sem *sem_alloc(int count)
{
	struct lkl_sem *sem;

	sem = malloc(sizeof(*sem));
	if (!sem)
		return NULL;

#ifdef _POSIX_SEMAPHORES
	if (sem_init(&sem->sem, SHARE_SEM, count) < 0) {
		lkl_printf("sem_init: %s\n", strerror(errno));
		free(sem);
		return NULL;
	}
#else
	pthread_mutex_init(&sem->lock, NULL);
	sem->count = count;
	WARN_PTHREAD(pthread_cond_init(&sem->cond, NULL));
#endif /* _POSIX_SEMAPHORES */

	return sem;
}

static void sem_free(struct lkl_sem *sem)
{
#ifdef _POSIX_SEMAPHORES
	WARN_UNLESS(sem_destroy(&sem->sem));
#else
	WARN_PTHREAD(pthread_cond_destroy(&sem->cond));
	WARN_PTHREAD(pthread_mutex_destroy(&sem->lock));
#endif /* _POSIX_SEMAPHORES */
	free(sem);
}

static void sem_up(struct lkl_sem *sem)
{
#ifdef _POSIX_SEMAPHORES
	WARN_UNLESS(sem_post(&sem->sem));
#else
	WARN_PTHREAD(pthread_mutex_lock(&sem->lock));
	sem->count++;
	if (sem->count > 0)
		WARN_PTHREAD(pthread_cond_signal(&sem->cond));
	WARN_PTHREAD(pthread_mutex_unlock(&sem->lock));
#endif /* _POSIX_SEMAPHORES */

}

static void sem_down(struct lkl_sem *sem)
{
#ifdef _POSIX_SEMAPHORES
	int err;

	do {
		err = sem_wait(&sem->sem);
	} while (err < 0 && errno == EINTR);
	if (err < 0 && errno != EINTR)
		lkl_printf("sem_wait: %s\n", strerror(errno));
#else
	WARN_PTHREAD(pthread_mutex_lock(&sem->lock));
	while (sem->count <= 0)
		WARN_PTHREAD(pthread_cond_wait(&sem->cond, &sem->lock));
	sem->count--;
	WARN_PTHREAD(pthread_mutex_unlock(&sem->lock));
#endif /* _POSIX_SEMAPHORES */
}

static void thread_detach(void)
{
	WARN_PTHREAD(pthread_detach(pthread_self()));
}

static int thread_join(lkl_thread_t tid)
{
	if (WARN_PTHREAD(pthread_join((pthread_t)tid, NULL)))
		return -1;
	else
		return 0;
}

static int thread_equal(lkl_thread_t a, lkl_thread_t b)
{
	return pthread_equal((pthread_t)a, (pthread_t)b);
}

static struct lkl_tls_key *tls_alloc(void (*destructor)(void *))
{
	struct lkl_tls_key *ret = malloc(sizeof(struct lkl_tls_key));

	if (WARN_PTHREAD(pthread_key_create(&ret->key, destructor))) {
		free(ret);
		return NULL;
	}
	return ret;
}

static void tls_free(struct lkl_tls_key *key)
{
	WARN_PTHREAD(pthread_key_delete(key->key));
	free(key);
}

static int tls_set(struct lkl_tls_key *key, void *data)
{
	if (WARN_PTHREAD(pthread_setspecific(key->key, data)))
		return -1;
	return 0;
}

static void *tls_get(struct lkl_tls_key *key)
{
	return pthread_getspecific(key->key);
}

static unsigned long long time_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return 1e9*ts.tv_sec + ts.tv_nsec;
}

static void *timer_alloc(void *fn, void *arg)
{
	int err;
	timer_t timer;
	struct sigevent se =  {
		.sigev_notify = SIGEV_THREAD,
		.sigev_value = {
			.sival_ptr = arg,
		},
		.sigev_notify_function = (void (*)(union sigval))fn
	};

	err = timer_create(CLOCK_REALTIME, &se, &timer);
	if (err)
		return NULL;

	return (void *)(long)timer;
}

static int timer_set_oneshot(void *_timer, unsigned long ns)
{
	timer_t timer = (timer_t)(long)_timer;
	struct itimerspec ts = {
		.it_value = {
			.tv_sec = ns / 1000000000,
			.tv_nsec = ns % 1000000000,
		},
	};

	return timer_settime(timer, 0, &ts, NULL);
}

static void timer_free(void *_timer)
{
	timer_t timer = (timer_t)(long)_timer;

	timer_delete(timer);
}

static void panic(void)
{
	assert(0);
}

static long _gettid(void)
{
#ifdef	__FreeBSD__
	return (long)pthread_self();
#else
	return syscall(SYS_gettid);
#endif
}

static void *page_alloc(unsigned long size)
{
	void *addr;

	addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
		     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (addr == MAP_FAILED)
		return NULL;

	return addr;
}

static void page_free(void *addr, unsigned long size)
{
	munmap((void *)addr, size);
}

#ifdef LKL_HOST_CONFIG_VFIO_PCI
extern struct lkl_dev_pci_ops vfio_pci_ops;
#endif

struct lkl_host_operations lkl_host_ops = {
	.panic = panic,
	.thread_switch = pure_thread_switch,
	.thread_create = pure_thread_create,
	.thread_detach = thread_detach,
	.thread_exit = pure_thread_exit,
	.thread_join = thread_join,
	.thread_self = pure_thread_self,
	.thread_equal = thread_equal,
	.sem_alloc = sem_alloc,
	.sem_free = sem_free,
	.sem_up = sem_up,
	.sem_down = sem_down,
	.tls_alloc = tls_alloc,
	.tls_free = tls_free,
	.tls_set = tls_set,
	.tls_get = tls_get,
	.time = time_ns,
	.timer_alloc = timer_alloc,
	.timer_set_oneshot = timer_set_oneshot,
	.timer_free = timer_free,
	.print = print,
	.mem_alloc = malloc,
	.mem_free = free,
	.page_alloc = page_alloc,
	.page_free = page_free,
	.ioremap = lkl_ioremap,
	.iomem_access = lkl_iomem_access,
	.virtio_devices = lkl_virtio_devs,
	.gettid = _gettid,
	.jmp_buf_set = jmp_buf_set,
	.jmp_buf_longjmp = jmp_buf_longjmp,
	.memcpy = memcpy,
#ifdef LKL_HOST_CONFIG_VFIO_PCI
	.pci_ops = &vfio_pci_ops,
#endif
};

static int fd_get_capacity(struct lkl_disk disk, unsigned long long *res)
{
	off_t off;

	off = lseek(disk.fd, 0, SEEK_END);
	if (off < 0)
		return -1;

	*res = off;
	return 0;
}

static int do_rw(ssize_t (*fn)(), struct lkl_disk disk, struct lkl_blk_req *req)
{
	off_t off = req->sector * 512;
	void *addr;
	int len;
	int i;
	int ret = 0;

	for (i = 0; i < req->count; i++) {

		addr = req->buf[i].iov_base;
		len = req->buf[i].iov_len;

		do {
			ret = fn(disk.fd, addr, len, off);

			if (ret <= 0) {
				ret = -1;
				goto out;
			}

			addr += ret;
			len -= ret;
			off += ret;

		} while (len);
	}

out:
	return ret;
}

static int blk_request(struct lkl_disk disk, struct lkl_blk_req *req)
{
	int err = 0;

	switch (req->type) {
	case LKL_DEV_BLK_TYPE_READ:
		err = do_rw(pread, disk, req);
		break;
	case LKL_DEV_BLK_TYPE_WRITE:
		err = do_rw(pwrite, disk, req);
		break;
	case LKL_DEV_BLK_TYPE_FLUSH:
	case LKL_DEV_BLK_TYPE_FLUSH_OUT:
#ifdef __linux__
		err = fdatasync(disk.fd);
#else
		err = fsync(disk.fd);
#endif
		break;
	default:
		return LKL_DEV_BLK_STATUS_UNSUP;
	}

	if (err < 0)
		return LKL_DEV_BLK_STATUS_IOERR;

	return LKL_DEV_BLK_STATUS_OK;
}

struct lkl_dev_blk_ops lkl_dev_blk_ops = {
	.get_capacity = fd_get_capacity,
	.request = blk_request,
};

