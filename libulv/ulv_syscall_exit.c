#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

int
ulv_syscall_exit(int status)
{
	return __syscall1(__NR_exit, status);
}

int
ulv_syscall_exit_group(int status)
{
	return 0;
}
