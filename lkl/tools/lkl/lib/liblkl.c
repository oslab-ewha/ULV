#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lkl_host.h>

void
start_lkl_kernel(void *mem_start, unsigned long mem_size)
{
	lkl_start_kernel(&lkl_host_ops, mem_start, mem_size);
}
