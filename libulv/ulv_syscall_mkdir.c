#include "ulv_types.h"
#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_fd_table.h"
#include "ulfs.h"

int
ulv_syscall_mkdir(const char *pathname, int mode)
{
	ulfs_mkdir(pathname);
	return 0;
}
