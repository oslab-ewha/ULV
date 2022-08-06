#ifndef _ULV_THREAD_H_
#define _ULV_THREAD_H_

#include "ulv_types.h"

typedef unsigned long	ulv_tid_t;

void ulv_thread_exit(void);
ulv_tid_t ulv_thread_self(void);
void ulv_thread_set_blocked(ulv_tid_t tid, int blocked);
void ulv_thread_reschedule(void);
bool_t ulv_is_last_thread(void);
bool_t ulv_is_main_thread(void);

#endif
