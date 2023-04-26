#ifndef _ULV_SYSCALL_NO_H_
#define _ULV_SYSCALL_NO_H_

#define __NR_read	0
#define __NR_write	1
#define __NR_open	2
#define __NR_close	3
#define __NR_fstat	5
#define __NR_lseek	8
#define __NR_mmap	9
#define __NR_mprotect	10
#define __NR_munmap	11
#define __NR_brk	12
#define __NR_rt_sigprocmask	14
#define __NR_ioctl	16
#define __NR_pread64	17
#define __NR_pwrite64	18
#define __NR_writev	20
#define __NR_pipe	22
#define __NR_select	23
#define __NR_msync	26
#define __NR_madvise	28
#define __NR_socket	41
#define __NR_connect	42
#define __NR_sendto	44
#define __NR_recvfrom	45
#define __NR_bind	49
#define __NR_listen	50
#define __NR_clone	56
#define __NR_exit	60
#define __NR_uname	63
#define __NR_fcntl	72
#define __NR_mkdir	83
#define __NR_rmdir	84
#define __NR_unlink	87
#define __NR_futex	202
#define __NR_getdents64		217
#define __NR_set_tid_address	218
#define __NR_exit_group	231
#define __NR_epoll_wait		232
#define __NR_epoll_ctl		233
#define __NR_epoll_create1	291

#endif
