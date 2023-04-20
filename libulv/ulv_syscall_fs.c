#include "ulfs.h"

#include "ulv_fd_table.h"

int
ulv_syscall_unlink(const char *pathname)
{
	return ulfs_unlink(pathname);
}

int
ulv_syscall_fcntl(int fd, unsigned long cmd, long arg)
{
	/* TODO: currently, just return OK */
	return 0;
}

int
ulv_syscall_getdents64(int fd, ulfs_dirent_t *dirp, unsigned int count)
{
	int	fd_real;
	fdtype_t	type;

	fd_real = ulv_lookup_fd_real(fd, &type);
	if (fd_real < 0 || type != FDTYPE_ULFS)
		return -1;
	return ulfs_getdents(fd_real, (ulfs_dirent_t *)dirp, count);
}

int
ulv_syscall_lseek(int fd, off_t offset, int whence)
{
	int	fd_real;
	fdtype_t	type;

	fd_real = ulv_lookup_fd_real(fd, &type);
	if (fd_real < 0 || type != FDTYPE_ULFS)
		return -1;
	return ulfs_lseek(fd_real, offset, whence);
}
