#ifndef _LKL_DBG_H_
#define _LKL_DBG_H_

#ifdef LKL_DEBUG

#define DBG(fmt, ...) dbg_output("LKLDBG: " fmt, ##__VA_ARGS__)

#include <asm/sched.h>

void dbg_output(const char *fmt, ...);
const char *dbg_thread(struct thread_info *ti);

#else
#define DBG(fmt, ...) do {} while (0)
#endif
#endif
