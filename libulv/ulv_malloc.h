#ifndef _ULV_MALLOC_H_
#define _ULV_MALLOC_H_

#include "ulv_types.h"

extern void *malloc(size_t);
extern void *realloc(void *, size_t);
extern void free(void *);

#define ulv_malloc	malloc
#define ulv_realloc	realloc
#define ulv_free	free

#endif
