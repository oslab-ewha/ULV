#ifndef _LKL_LIB_PURE_THREAD_H
#define _LKL_LIB_PURE_THREAD_H

#include <lkl_host.h>

void pure_thread_switch(lkl_thread_t prev, lkl_thread_t next);
lkl_thread_t pure_thread_create(void *(*fn)(void *), void *arg);
void pure_thread_exit(lkl_thread_t lthrd);
lkl_thread_t pure_thread_self(void);

#endif
