extern int
lwip_socket(int domain, int type, int protocol);

int
ulv_syscall_socket(int domain, int type, int protocol)
{
	return lwip_socket(domain, type, protocol);
}
