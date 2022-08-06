#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"

ssize_t
ulv_syscall_read(int fd, void *buf, size_t count)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_HOST))
		return -1;
	return __syscall3(__NR_read, (long)fd, (long)buf, (long)count);
}
