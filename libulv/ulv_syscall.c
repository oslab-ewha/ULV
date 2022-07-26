#include "ulv_syscall.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_assert.h"

long
ulv_syscall(long n, long *args)
{
	switch (n) {
	case __NR_write:
		return ulv_syscall_write(args[0], args[1], args[2]);
	case __NR_mmap:
		return ulv_syscall_mmap(args[0], args[1], args[2], args[3], args[4], args[5]);
	case __NR_munmap:
		return ulv_syscall_munmap(args[0], args[1]);
	case __NR_brk:
		return ulv_syscall_brk(args[0]);
	case __NR_writev:
		return ulv_syscall_writev(args[0], args[1], args[2]);
	case __NR_uname:
		return ulv_syscall_uname(args[0]);
	case __NR_exit:
		__syscall1(n, args[0]);
	case __NR_set_tid_address:
		return ulv_syscall_set_tid_address(args[0]);
	default:
		ULV_ABORT();
		break;
	}

	return 0;
}
