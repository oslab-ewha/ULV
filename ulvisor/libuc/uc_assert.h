#ifndef _UC_ASSERT_H_
#define _UC_ASSERT_H_

#define UC_ASSERT(expr)	do { if (!(expr)) _uc_crash(); } while (0)
#define UC_ABORT()	do { _uc_crash(); } while (0)

static inline void _uc_crash(void)
{
	__asm__ __volatile__( "hlt" : : : "memory" );
}

#endif
