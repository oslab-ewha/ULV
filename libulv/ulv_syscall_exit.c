#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_thread.h"

int
ulv_syscall_exit(int status)
{
	if (ulv_is_last_thread() || ulv_is_main_thread())
		return __syscall1(__NR_exit, status);
	ulv_thread_exit();
	return 0;
}

int
ulv_syscall_exit_group(int status)
{
	return 0;
}
