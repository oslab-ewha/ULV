#ifndef _ULV_LIBC_H_
#define _ULV_LIBC_H_

#include "ulv_types.h"

int printf(const char *format, ...);

int sscanf(const char *str, const char *format, ...);

void *memset(void *s, int c, size_t n);
char *strcpy(char *dest, const char *src);

int setenv(const char *name, const char *value, int overwrite);

char *basename(char *path);

#endif
