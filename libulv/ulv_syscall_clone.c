#include "ulv_assert.h"
#include "ulv_thread.h"
#include "ulv_syscall_flags.h"

int
ulv_syscall_clone(unsigned long flags, void *stack, int *ptid, int *ctid, void *tls)
{
	if (!(flags & CLONE_VM))
		ULV_PANIC("clone: CLONE_VM flag required");

	return ulv_thread_clone(stack, tls);
}
