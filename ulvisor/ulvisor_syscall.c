#include "ulvisor.h"
#include "ulvisor_syscall.h"
#include "libuc/uc_syscalls.h"

long
ulv_syscall(long n, long *args)
{
	switch (n) {
	case __NR_exit:
		__syscall1(n, args[0]);
	default:
		break;
	}

	return 0;
}
