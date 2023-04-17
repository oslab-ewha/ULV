#include "ulv_syscall.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_assert.h"

#define CASE_ULV_SYSCALL_1(name)	case __NR_##name: return ulv_syscall_##name(args[0])
#define CASE_ULV_SYSCALL_2(name)	case __NR_##name: return ulv_syscall_##name(args[0], args[1])
#define CASE_ULV_SYSCALL_3(name)	case __NR_##name: return ulv_syscall_##name(args[0], args[1], args[2])
#define CASE_ULV_SYSCALL_4(name)	case __NR_##name: return ulv_syscall_##name(args[0], args[1], args[2], args[3])
#define CASE_ULV_SYSCALL_5(name)	case __NR_##name: return ulv_syscall_##name(args[0], args[1], args[2], args[3], args[4])
#define CASE_ULV_SYSCALL_6(name)	case __NR_##name: return ulv_syscall_##name(args[0], args[1], args[2], args[3], args[4], args[5])

#define CASE_ULV_SYSCALL(name, N) CASE_ULV_SYSCALL_## N(name)

long
ulv_syscall(long n, long *args)
{
	switch (n) {
		CASE_ULV_SYSCALL(read, 3);
		CASE_ULV_SYSCALL(write, 3);
		CASE_ULV_SYSCALL(open, 3);
		CASE_ULV_SYSCALL(close, 1);
		CASE_ULV_SYSCALL(mkdir, 2);
		CASE_ULV_SYSCALL(rmdir, 1);
		CASE_ULV_SYSCALL(unlink, 1);
		CASE_ULV_SYSCALL(fcntl, 3);
		CASE_ULV_SYSCALL(getdents64, 3);
		CASE_ULV_SYSCALL(mmap, 6);
		CASE_ULV_SYSCALL(mprotect, 3);
		CASE_ULV_SYSCALL(munmap, 2);
		CASE_ULV_SYSCALL(brk, 1);
	case __NR_rt_sigprocmask:
		return 0;
		CASE_ULV_SYSCALL(ioctl, 3);
		CASE_ULV_SYSCALL(writev, 3);
		CASE_ULV_SYSCALL(select, 5);
		CASE_ULV_SYSCALL(socket, 3);
		CASE_ULV_SYSCALL(connect, 3);
		CASE_ULV_SYSCALL(sendto, 6);
		CASE_ULV_SYSCALL(recvfrom, 6);
		CASE_ULV_SYSCALL(bind, 3);
		CASE_ULV_SYSCALL(listen, 2);
		CASE_ULV_SYSCALL(uname, 1);
		CASE_ULV_SYSCALL(exit, 1);
		CASE_ULV_SYSCALL(futex, 6);
		CASE_ULV_SYSCALL(set_tid_address, 1);
		CASE_ULV_SYSCALL(exit_group, 1);
	default:
		ULV_PANIC("undefined system call: %ld", n);
		break;
	}

	return 0;
}
