#ifndef _UC_ATOMIC_H_
#define _UC_ATOMIC_H_

static inline int uc_atomic_cas(volatile int *p, int t, int s)
{
	__asm__ __volatile__ (
		"lock ; cmpxchg %3, %1"
		: "=a"(t), "=m"(*p) : "a"(t), "r"(s) : "memory" );
        return t;
}

static inline int uc_atomic_swap(volatile int *p, int v)
{
        __asm__ __volatile__(
		"xchg %0, %1"
		: "=r"(v), "=m"(*p) : "0"(v) : "memory" );
	return v;
}

static inline void uc_atomic_store(volatile int *p, int x)
{
	__asm__ __volatile__(
		"mov %1, %0 ; lock ; orl $0,(%%rsp)"
		: "=m"(*p) : "r"(x) : "memory" );
}

static inline void uc_atomic_spin(void)
{
	__asm__ __volatile__( "pause" : : : "memory" );
}


static inline void uc_spin_lock(int *lock)
{
	while (*(volatile int *)lock || uc_atomic_cas(lock, 0, 1)) uc_atomic_spin();
}

static inline void uc_spin_unlock(int *lock)
{
	uc_atomic_store(lock, 0);
}

#endif
