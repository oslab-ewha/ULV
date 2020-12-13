#define __SYSCALL_LL_E(x) (x)
#define __SYSCALL_LL_O(x) (x)

long lkl_syscall(long, long *);

static __inline long __syscall0(long n)
{
	long	lkl_params[6];
	return lkl_syscall(n, lkl_params);
}

static __inline long
__syscall1(long n, long a1)
{
	long	lkl_params[6] = { a1 };
	return lkl_syscall(n, lkl_params);
}

static __inline long
__syscall2(long n, long a1, long a2)
{
	long	lkl_params[6] = { a1, a2 };
	return lkl_syscall(n, lkl_params);
}

static __inline long
__syscall3(long n, long a1, long a2, long a3)
{
	long	lkl_params[6] = { a1, a2, a3 };
	return lkl_syscall(n, lkl_params);
}

static __inline long
__syscall4(long n, long a1, long a2, long a3, long a4)
{
	long	lkl_params[6] = { a1, a2, a3, a4 };
	return lkl_syscall(n, lkl_params);
}

static __inline long
__syscall5(long n, long a1, long a2, long a3, long a4, long a5)
{
	long	lkl_params[6] = { a1, a2, a3, a4, a5 };
	return lkl_syscall(n, lkl_params);
}

static __inline long
__syscall6(long n, long a1, long a2, long a3, long a4, long a5, long a6)
{
	long	lkl_params[6] = { a1, a2, a3, a4, a5, a6 };
	return lkl_syscall(n, lkl_params);
}

#define VDSO_USEFUL
#define VDSO_CGT_SYM "__vdso_clock_gettime"
#define VDSO_CGT_VER "LINUX_2.6"
#define VDSO_GETCPU_SYM "__vdso_getcpu"
#define VDSO_GETCPU_VER "LINUX_2.6"

#define IPC_64 0
