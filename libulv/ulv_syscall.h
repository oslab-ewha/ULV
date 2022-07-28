#ifndef _ULV_SYSCALL_H_
#define _ULV_SYSCALL_H_

long ulv_syscall_write(long a1, long a2, long a3);
long ulv_syscall_mmap(long a1, long a2, long a3, long a4, long a5, long a6);
long ulv_syscall_munmap(long a1, long a2);
long ulv_syscall_brk(long a1);
long ulv_syscall_ioctl(long a1, long a2, long a3);
long ulv_syscall_writev(long a1, long a2, long a3);
long ulv_syscall_socket(long, long, long);
long ulv_syscall_uname(long a1);
long ulv_syscall_exit(long);
long ulv_syscall_set_tid_address(long a1);
long ulv_syscall_exit_group(long);

#endif
