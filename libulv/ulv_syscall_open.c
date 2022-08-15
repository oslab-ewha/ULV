#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_syscall_flags.h"
#include "ulv_fd_table.h"
#include "ulfs.h"

extern int strcmp(const char *s1, const char *s2);

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
	int	fd;

	fd = ulfs_open(pathname, flags, mode);
	if (fd < 0)
		return -1;
	return ulv_assign_fd(FDTYPE_ULFS, fd);
}

void
ulv_syscall_close(int fd)
{
	fdtype_t	type;
	int	fd_real;

	fd_real = ulv_lookup_fd_real(fd, &type);
	if (fd_real < 0)
		return;
	if (type == FDTYPE_ULFS)
		return ulfs_close(fd_real);
}
