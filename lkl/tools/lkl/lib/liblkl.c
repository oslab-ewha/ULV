#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../../../arch/lkl/include/uapi/asm/host_ops.h"

extern struct lkl_host_operations	lkl_host_ops;

void
start_lkl_kernel(void *mem_start, unsigned long mem_size)
{
	lkl_start_kernel(&lkl_host_ops, mem_start, mem_size);
}
