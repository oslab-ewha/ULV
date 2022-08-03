#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

extern int
epoller_select_events(int nfds, long readfds, long writefds, long exceptfds, long timeout);

__attribute__((__weak__)) int
lwip_socket(int domain, int type, int protocol)
{
	return -1;
}

__attribute__((__weak__)) int
lwip_connect(long fd, long name, long len)
{
	return -1;
}

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
ulv_syscall_select(int nfds, long readfds, long writefds, long exceptfds, long timeout)
{
	return epoller_select_events(nfds, readfds, writefds, exceptfds, timeout);
}
