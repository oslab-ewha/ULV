#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

extern int strcmp(const char *s1, const char *s2);

int
ulv_syscall_open(const char *pathname, int flags, int mode)
{
	if (strcmp(pathname, "/dev/net/tun") == 0)
		return __syscall2(__NR_open, (long)pathname, (long)flags);
	return -1;
}
