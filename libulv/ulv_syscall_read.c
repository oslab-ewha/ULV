#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"
#include "ulfs.h"

ssize_t
ulv_syscall_read(int fd, void *buf, size_t count)
{
	int	fd_real;
	fdtype_t	type;

	fd_real = ulv_lookup_fd_real(fd, &type);
	if (fd_real < 0)
		return -1;
	if (type == FDTYPE_ULFS)
		return ulfs_read(fd_real, buf, count);
	return __syscall3(__NR_read, (long)fd_real, (long)buf, (long)count);
}
