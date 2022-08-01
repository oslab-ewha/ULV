#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

ssize_t
ulv_syscall_read(int fd, void *buf, size_t count)
{
	return __syscall3(__NR_read, (long)fd, (long)buf, (long)count);
}
