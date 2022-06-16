#ifndef _ASM_LKL_SYSCALLS_H
#define _ASM_LKL_SYSCALLS_H

long lkl_syscall(long no, long *params);
void wakeup_idle_host_task(void);

#define sys_clone sys_ni_syscall
#define sys_vfork sys_ni_syscall
#define sys_rt_sigreturn sys_ni_syscall

#define sys_mmap
#include <asm-generic/syscalls.h>
#undef sys_mmap

asmlinkage long sys_mmap(unsigned long addr, unsigned long len,
			unsigned long prot, unsigned long flags,
			unsigned long fd, unsigned long pgoff);

#endif /* _ASM_LKL_SYSCALLS_H */
