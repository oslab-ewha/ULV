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
#include "pure_thread.h"

/* Let's see if the host has semaphore.h */
#include <unistd.h>

#ifdef _POSIX_SEMAPHORES
#include <semaphore.h>
/* TODO(pscollins): We don't support fork() for now, but maybe one day
 * we will? */
#define SHARE_SEM 0
#endif /* _POSIX_SEMAPHORES */

void ulvisor_clockevent_setnext(unsigned long ns);
void ulvisor_clockevent_alloc(int irq);
void ulvisor_clockevent_free(void);

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

static unsigned long long time_ns(void)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return 1e9*ts.tv_sec + ts.tv_nsec;
}

static void panic(void)
{
	assert(0);
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
	.time = time_ns,
	.clockevent_alloc = ulvisor_clockevent_alloc,
	.clockevent_set_next = ulvisor_clockevent_setnext,
	.clockevent_free = ulvisor_clockevent_free,
	.print = print,
	.mem_alloc = malloc,
	.mem_free = free,
	.page_alloc = page_alloc,
	.page_free = page_free,
	.ioremap = lkl_ioremap,
	.iomem_access = lkl_iomem_access,
	.virtio_devices = lkl_virtio_devs,
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

