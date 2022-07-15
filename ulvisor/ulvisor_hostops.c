#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/mman.h>
#include <string.h>

#include "ulvisor_pure_thread.h"
#include "../lkl/arch/lkl/include/uapi/asm/host_ops.h"

void ulvisor_clockevent_setnext(unsigned long ns);
void ulvisor_clockevent_alloc(int irq);
void ulvisor_clockevent_free(void);

void *ulvisor_ioremap(long addr, int size);
int ulvisor_iomem_access(const volatile void *addr, void *res, int size, int write);

extern char virtio_devs[];

static void
print(const char *str, int len)
{
	int	n = write(1, str, len);
}

static unsigned long long
time_ns(void)
{
	struct timespec	ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return 1e9*ts.tv_sec + ts.tv_nsec;
}

static void
panic(void)
{
	assert(0);
}

static void *
page_alloc(unsigned long size)
{
	void *addr;

	addr = mmap(NULL, size, PROT_READ | PROT_WRITE,
		     MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

	if (addr == MAP_FAILED)
		return NULL;

	return addr;
}

static void
page_free(void *addr, unsigned long size)
{
	munmap((void *)addr, size);
}

struct lkl_host_operations lkl_host_ops = {
	.panic = panic,
	.thread_switch = pure_thread_switch,
	.thread_create = pure_thread_create,
	.thread_exit = pure_thread_exit,
	.thread_self = pure_thread_self,
	.time = time_ns,
	.clockevent_alloc = ulvisor_clockevent_alloc,
	.clockevent_set_next = ulvisor_clockevent_setnext,
	.clockevent_free = ulvisor_clockevent_free,
	.print = print,
	.mem_alloc = malloc,
	.mem_free = free,
	.page_alloc = page_alloc,
	.page_free = page_free,
	.ioremap = ulvisor_ioremap,
	.iomem_access = ulvisor_iomem_access,
	.virtio_devices = virtio_devs,
	.memcpy = memcpy,
};
