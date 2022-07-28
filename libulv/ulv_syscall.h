#ifndef _ULV_SYSCALL_H_
#define _ULV_SYSCALL_H_

#define DEF_ULV_SYSCALL_1(name)	long ulv_syscall_## name(long a1)
#define DEF_ULV_SYSCALL_2(name)	long ulv_syscall_## name(long a1, long a2)
#define DEF_ULV_SYSCALL_3(name)	long ulv_syscall_## name(long a1, long a2, long a3)
#define DEF_ULV_SYSCALL_4(name)	long ulv_syscall_## name(long a1, long a2, long a3, long a4)
#define DEF_ULV_SYSCALL_5(name)	long ulv_syscall_## name(long a1, long a2, long a3, long a4, long a5)
#define DEF_ULV_SYSCALL_6(name)	long ulv_syscall_## name(long a1, long a2, long a3, long a4, long a5, long a6)

#define DEF_ULV_SYSCALL(name, N) DEF_ULV_SYSCALL_## N(name)

DEF_ULV_SYSCALL(write, 3);
DEF_ULV_SYSCALL(mmap, 6);
DEF_ULV_SYSCALL(mprotect, 3);
DEF_ULV_SYSCALL(munmap, 2);
DEF_ULV_SYSCALL(brk, 1);
DEF_ULV_SYSCALL(ioctl, 3);
DEF_ULV_SYSCALL(writev, 3);
DEF_ULV_SYSCALL(socket, 3);
DEF_ULV_SYSCALL(uname, 1);
DEF_ULV_SYSCALL(exit, 1);
DEF_ULV_SYSCALL(set_tid_address, 1);
DEF_ULV_SYSCALL(exit_group, 1);

#endif
