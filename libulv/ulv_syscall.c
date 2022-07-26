#include "ulv_syscall.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

long
ulv_syscall(long n, long *args)
{
	switch (n) {
	case __NR_write:
		return ulv_syscall_write(args[0], args[1], args[2]);
	case __NR_writev:
		return ulv_syscall_writev(args[0], args[1], args[2]);
	case __NR_uname:
		return ulv_syscall_uname(args[0]);
	case __NR_exit:
		__syscall1(n, args[0]);
	default:
		break;
	}

	return 0;
}
