#include "ulv_types.h"
#include "ulv_worker.h"
#include "ulv_syscall_no.h"
#include "ulv_syscall_flags.h"
#include "ulv_host_syscall.h"
#include "ulv_assert.h"

#define STACK_SIZE	16384

extern pid_t ulv_clone_host(long flags, char *stack, ulv_worker_func_t func);

ulv_worker_t
ulv_start_worker(ulv_worker_func_t func)
{
	char	*stack;
	long	prot = PROT_READ | PROT_WRITE;
	long	map_flags = MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK;
	long	clone_flags = CLONE_VM | CLONE_FILES | CLONE_THREAD | CLONE_SIGHAND;
	pid_t	pid;

	stack = (void *)__syscall6(__NR_mmap, 0, (long)STACK_SIZE, (long)prot, (long)map_flags, (long)-1, (long)0);
	if (stack == MAP_FAILED)
		ULV_PANIC("cannot start worker. mmap failed.");

	pid = ulv_clone_host(clone_flags, stack + STACK_SIZE - 8, func);
	if (pid < 0)
		ULV_PANIC("cannot start worker. clone failed.");
	return (ulv_worker_t)pid;
}
