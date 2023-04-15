#include "ulfs.h"

int
ulv_syscall_mkdir(const char *pathname, int mode)
{
	return ulfs_mkdir(pathname);
}

int
ulv_syscall_rmdir(const char *pathname)
{
	return ulfs_rmdir(pathname);
}
