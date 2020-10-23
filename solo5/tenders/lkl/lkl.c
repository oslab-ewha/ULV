#include <lkl.h>
#include <lkl_host.h>

void
init_lkl(void)
{
	lkl_start_kernel(&lkl_host_ops, "mem=100M");
}
