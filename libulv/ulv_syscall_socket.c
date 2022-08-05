#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

extern int
epoller_select_events(int nfds, long readfds, long writefds, long exceptfds, long timeout);

long lwip_socket(int domain, int type, int protocol);
long lwip_connect(long fd, long name, long len);
long lwip_bind(long fd, long addr, long len);
long lwip_listen(long fd, long backlog);
long lwip_sendto(long fd, long buf, long len, long flags, long dest_addr, long addrlen);
long lwip_recvfrom(long fd, long buf, long len, long flags, long src_addr, long addrlen);

int
ulv_syscall_socket(int domain, int type, int protocol)
{
	return lwip_socket(domain, type, protocol);
}

int
ulv_syscall_connect(long fd, long name, long len)
{
	return lwip_connect(fd, name, len);
}

int
ulv_syscall_bind(long fd, long addr, long len)
{
	return lwip_bind(fd, addr, len);
}

int
ulv_syscall_listen(long fd, long backlog)
{
	return lwip_listen(fd, backlog);
}

int
ulv_syscall_sendto(long fd, long buf, long len, long flags, long dest_addr, long addrlen)
{
	return lwip_sendto(fd, buf, len, flags, dest_addr, addrlen);
}

int
ulv_syscall_recvfrom(long fd, long buf, long len, long flags, long src_addr, long addrlen)
{
	return lwip_recvfrom(fd, buf, len, flags, src_addr, addrlen);
}

int
ulv_syscall_select(int nfds, long readfds, long writefds, long exceptfds, long timeout)
{
	return epoller_select_events(nfds, readfds, writefds, exceptfds, timeout);
}
