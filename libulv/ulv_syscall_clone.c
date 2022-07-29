#include "ulv_assert.h"
#include "ulv_thread.h"

#define CLONE_VM		0x00000100
#define CLONE_CHILD_CLEARTID	0x00200000
#define CLONE_CHILD_SETTID	0x01000000

int
ulv_syscall_clone(unsigned long flags, void *stack, int *ptid, int *ctid, void *tls)
{
	if (!(flags & CLONE_VM))
		ULV_PANIC("clone: CLONE_VM flag required");

	return ulv_thread_clone(stack);
}
