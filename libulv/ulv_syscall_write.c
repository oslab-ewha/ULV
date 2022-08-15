#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"
#include "ulfs.h"

struct iovec {
	void	*iov_base;
	size_t	iov_len;
};

ssize_t
ulv_syscall_write(int fd, const void *buf, size_t count)
{
	int	fd_real;
	fdtype_t	type;

	fd_real = ulv_lookup_fd_real(fd, &type);
	if (fd_real < 0)
		return -1;
	if (type == FDTYPE_ULFS)
		return ulfs_write(fd_real, buf, count);
	else
		return __syscall3(__NR_write, (long)fd, (long)buf, (long)count);
}

ssize_t
ulv_syscall_writev(int fd, const struct iovec *iov, int iovcnt)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_HOST))
		return -1;
	return __syscall3(__NR_writev, (long)fd, (long)iov, (long)iovcnt);
}
