#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

int
ulv_syscall_ioctl(int fd, unsigned long request, long val)
{
	if (fd == 1)
		return __syscall3(__NR_ioctl, (long)fd, (long)request, (long)val);
	return -1;
}
