#include "lwip/sockets.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

extern int
epoller_select_events(int nfds, long readfds, long writefds, long exceptfds, long timeout);

extern int
lwip_socket(int domain, int type, int protocol);

int
ulv_syscall_socket(int domain, int type, int protocol)
{
	return lwip_socket(domain, type, protocol);
}

int
ulv_syscall_connect(int fd, struct sockaddr *name, socklen_t len)
{
	return lwip_connect(fd, name, len);
}

int
ulv_syscall_select(int nfds, long readfds, long writefds, long exceptfds, long timeout)
{
	return epoller_select_events(nfds, readfds, writefds, exceptfds, timeout);
}
