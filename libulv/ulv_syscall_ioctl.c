#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"

int
ulv_syscall_ioctl(int fd, unsigned long request, long val)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_HOST))
		return -1;
	return __syscall3(__NR_ioctl, (long)fd, (long)request, (long)val);
}
