#ifndef _ULV_TYPES_H_
#define _ULV_TYPES_H_

typedef unsigned int	uint32_t;
typedef unsigned long	uint64_t;

typedef unsigned long	off_t;

typedef unsigned long	size_t;
typedef long		ssize_t;

#define likely(x)	__builtin_expect(!!(x), 1)
#define unlikely(x)	__builtin_expect(!!(x), 0)

#define NULL	((void *)0)

#endif
