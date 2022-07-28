#include "ulv_host_syscall.h"
#include "ulv_syscall_no.h"
#include "ulv_types.h"

int
ulv_syscall_brk(void *addr)
{
	return (int)__syscall1(__NR_brk, (long)addr);
}

void *
ulv_syscall_mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
{
	return (void *)__syscall6(__NR_mmap, (long)addr, (long)length, (long)prot, (long)flags, (long)fd, (long)offset);
}

int
ulv_syscall_munmap(void *addr, size_t length)
{
	return (int)__syscall2(__NR_munmap, (long)addr, (long)length);
}

int
ulv_syscall_mprotect(void *addr, size_t len, int prot)
{
	return (int)__syscall3(__NR_mprotect, (long)addr, (long)len, (long)prot);
}
