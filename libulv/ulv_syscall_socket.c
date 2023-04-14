#include "ulv_build.h"

#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"

extern int
epoller_select_events(int nfds, long readfds, long writefds, long exceptfds, long timeout);

long _BUILD_WEAK lwip_socket(int domain, int type, int protocol);
long _BUILD_WEAK lwip_connect(long fd, long name, long len);
long _BUILD_WEAK lwip_bind(long fd, long addr, long len);
long _BUILD_WEAK lwip_listen(long fd, long backlog);
long _BUILD_WEAK lwip_sendto(long fd, long buf, long len, long flags, long dest_addr, long addrlen);
long _BUILD_WEAK lwip_recvfrom(long fd, long buf, long len, long flags, long src_addr, long addrlen);

int
ulv_syscall_socket(int domain, int type, int protocol)
{
	int	fd = lwip_socket(domain, type, protocol);

	if (fd < 0)
		return fd;

	return ulv_assign_fd(FDTYPE_LWIP, fd);
}

int
ulv_syscall_connect(int fd, long name, long len)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_LWIP))
		return -1;
	return lwip_connect(fd, name, len);
}

int
ulv_syscall_bind(int fd, long addr, long len)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_LWIP))
		return -1;
	return lwip_bind(fd, addr, len);
}

int
ulv_syscall_listen(int fd, long backlog)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_LWIP))
		return -1;
	return lwip_listen(fd, backlog);
}

int
ulv_syscall_sendto(int fd, long buf, long len, long flags, long dest_addr, long addrlen)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_LWIP))
		return -1;
	return lwip_sendto(fd, buf, len, flags, dest_addr, addrlen);
}

int
ulv_syscall_recvfrom(int fd, long buf, long len, long flags, long src_addr, long addrlen)
{
	if (!ulv_convert_fd_real(&fd, FDTYPE_LWIP))
		return -1;
	return lwip_recvfrom(fd, buf, len, flags, src_addr, addrlen);
}

int
ulv_syscall_select(int nfds, long readfds, long writefds, long exceptfds, long timeout)
{
	return epoller_select_events(nfds, readfds, writefds, exceptfds, timeout);
}
