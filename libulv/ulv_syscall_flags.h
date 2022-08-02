#ifndef _ULV_SYSCALL_FLAGS_H_
#define _ULV_SYSCALL_FLAGS_H_

#define PROT_READ	1
#define PROT_WRITE	2

#define MAP_FAILED	((void *)-1)

#define MAP_PRIVATE	0x02
#define MAP_ANONYMOUS	0x20
#define MAP_STACK	0x20000

#define CLONE_VM		0x00000100
#define CLONE_FS		0x00000200
#define CLONE_FILES		0x00000400
#define CLONE_SIGHAND		0x00000800
#define CLONE_THREAD		0x00010000
#define CLONE_CHILD_CLEARTID	0x00200000
#define CLONE_CHILD_SETTID	0x01000000

#define EPOLLIN		0x001
#define EPOLLOUT	0x004
#define EPOLLERR	0x008

#define EPOLL_CTL_ADD	1
#define EPOLL_CTL_DEL	2
#define EPOLL_CTL_MOD	3

#endif
