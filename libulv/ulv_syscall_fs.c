#include "ulfs.h"

#include "ulv_fd_table.h"

#define S_IFDIR	0040000

struct timespec {
	uint64_t	tv_sec;
	uint64_t	tv_nsec;
};

struct stat {
	uint64_t	st_dev;
        uint64_t	st_ino;
        uint64_t	st_nlink;

        uint32_t	st_mode;
        uint32_t	st_uid;
        uint32_t	st_gid;
        unsigned int    __pad0;
        uint64_t	st_rdev;

        uint64_t	st_size;
        uint64_t	st_blksize;
        uint64_t	st_blocks;

        struct timespec	st_atim;
        struct timespec	st_mtim;
        struct timespec	st_ctim;
        long __unused[3];
};

int
ulv_syscall_fstat(int fd, struct stat *pstat)
{
	int	fd_real;
	fdtype_t	type;

	fd_real = ulv_lookup_fd_real(fd, &type);
	if (fd_real < 0 || type != FDTYPE_ULFS)
		return -1;

	ulfs_stat_t	statbuf;
	int	ret;

	ret = ulfs_fstat(fd_real, &statbuf);
	if (ret != 0)
		return ret;

	pstat->st_ino = statbuf.st_ino;
	pstat->st_mode = 0777;
	if (statbuf.st_is_dir)
		pstat->st_mode |= S_IFDIR;

	pstat->st_size = statbuf.st_size;
	pstat->st_blksize = 4096;
	pstat->st_blocks = (statbuf.st_size + 4095) / 4096;

	return 0;
}

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
