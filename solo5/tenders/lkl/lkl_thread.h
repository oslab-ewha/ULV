#ifndef _LKL_THREAD_H_
#define _LKL_THREAD_H_

#include <sched.h>

typedef int (*thread_func_t)(void *);

pid_t start_thread(thread_func_t func, int stack_size, void *ctx);

void futex_wait(int *futexp);
void futex_wakeup(int *futexp);

#endif
