#ifndef _UAPI_ASM_LKL_UNISTD_H_
#define _UAPI_ASM_LKL_UNISTD_H_

#define __ARCH_WANT_SYSCALL_NO_AT
#define __ARCH_WANT_SYSCALL_DEPRECATED
#define __ARCH_WANT_SYSCALL_NO_FLAGS
#define __ARCH_WANT_RENAMEAT
#define __ARCH_WANT_NEW_STAT
#define __ARCH_WANT_SET_GET_RLIMIT
#define __ARCH_WANT_TIME32_SYSCALLS

#include <asm/bitsperlong.h>

#if __BITS_PER_LONG == 64
#define __ARCH_WANT_SYS_NEWFSTATAT
#endif

#undef __NR_syscalls
#define __NR_syscalls 437

#ifndef __KERNEL__
#include <asm/unistd_64.h>
#endif

#define __NR_virtio_mmio_device_add		436

#endif
