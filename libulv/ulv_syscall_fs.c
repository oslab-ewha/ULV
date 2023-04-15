#include "ulfs.h"

int
ulv_syscall_unlink(const char *pathname)
{
	return ulfs_unlink(pathname);
}
