#ifndef _ULV_THREAD_H_
#define _ULV_THREAD_H_

typedef unsigned long	ulv_tid_t;

ulv_tid_t ulv_thread_clone(char *stack);
void ulv_thread_exit(ulv_tid_t tid);
ulv_tid_t ulv_thread_self(void);
void ulv_thread_set_blocked(ulv_tid_t tid, int blocked);
void ulv_thread_reschedule(void);

#endif
