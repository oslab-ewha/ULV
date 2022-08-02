#ifndef _ULV_WORKER_H_
#define _ULV_WORKER_H_

typedef long	ulv_worker_t;
typedef void (*ulv_worker_func_t)(void);

ulv_worker_t ulv_start_worker(ulv_worker_func_t func);

#endif
