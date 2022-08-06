#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"

struct iovec {
	void	*iov_base;
	size_t	iov_len;
};

ssize_t
ulv_syscall_write(int fd, const void *buf, size_t count)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_HOST))
		return -1;
	return __syscall3(__NR_write, (long)fd, (long)buf, (long)count);
}

ssize_t
ulv_syscall_writev(int fd, const struct iovec *iov, int iovcnt)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_HOST))
		return -1;
	return __syscall3(__NR_writev, (long)fd, (long)iov, (long)iovcnt);
}
