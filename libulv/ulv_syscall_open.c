#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"

extern int strcmp(const char *s1, const char *s2);

#define O_RDWR	02

int
ulv_open_devtap(void)
{
	int	fd_tap;

	fd_tap = __syscall2(__NR_open, (long)"/dev/net/tun", (long)O_RDWR);
	if (fd_tap < 0)
		return fd_tap;

	return ulv_assign_fd(FDTYPE_HOST, fd_tap);
}

int
ulv_syscall_open(const char *pathname, int flags, int mode)
{
	return -1;
}
