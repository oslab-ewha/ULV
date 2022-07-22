#ifndef _LIBUC_H_
#define _LIBUC_H_

#define NULL	((void *)0)

void *uc_malloc(unsigned size);
void uc_free(void *ptr);

#endif
