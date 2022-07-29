#ifndef _ULV_ASSERT_H_
#define _ULV_ASSERT_H_

#define ULV_ASSERT(expr)	do { if (!(expr)) _ulv_crash(); } while (0)
#define ULV_ABORT()	do { _ulv_crash(); } while (0)
#define ULV_PANIC(msg)	do { _ulv_crash(); } while (0)

static inline void _ulv_crash(void)
{
	__asm__ __volatile__( "hlt" : : : "memory" );
}

#endif
