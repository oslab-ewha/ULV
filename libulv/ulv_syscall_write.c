#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

struct iovec {
	void	*iov_base;
	size_t	iov_len;
};

ssize_t
ulv_syscall_write(int fd, const void *buf, size_t count)
{
	if (fd == 1) {
		return __syscall3(__NR_write, (long)fd, (long)buf, (long)count);
	}
	return 0;
}

ssize_t
ulv_syscall_writev(int fd, const struct iovec *iov, int iovcnt)
{
	if (fd == 1) {
		return __syscall3(__NR_writev, (long)fd, (long)iov, (long)iovcnt);
	}
	return 0;
}
