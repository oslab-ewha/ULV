#ifndef _ULV_TYPES_H_
#define _ULV_TYPES_H_

#include <sys/types.h>

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define NULL	((void *)0)

#endif
