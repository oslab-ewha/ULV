#ifndef _ULFS_H_
#define _ULFS_H_

#include "ulv_build.h"
#include "ulv_types.h"

#define ERR_ULFS_NOTEXIST	-1

#if defined(ULFS_USE_GLIBC) && defined(_BUILD_WEAK)
#undef _BUILD_WEAK
#define _BUILD_WEAK
#endif

#define ULFS_DT_UNKNOWN	0
#define ULFS_DT_DIR	4
#define ULFS_DT_REG	8

typedef struct ulfs_dirent {
	uint64_t	d_ino;
	uint64_t	d_off;
	uint16_t	d_reclen;
	uint8_t		d_type;
	char		d_name[];
} ulfs_dirent_t;

int _BUILD_WEAK ulfs_open(const char *path, int flags, int mode);
void _BUILD_WEAK ulfs_close(int fd);
ssize_t _BUILD_WEAK ulfs_read(int fd, void *buf, size_t count);
ssize_t _BUILD_WEAK ulfs_write(int fd, const void *buf, size_t count);

int _BUILD_WEAK ulfs_mkdir(const char *path);
int _BUILD_WEAK ulfs_rmdir(const char *path);
int _BUILD_WEAK ulfs_unlink(const char *path);

int _BUILD_WEAK ulfs_getdents(int fd, ulfs_dirent_t *dirp, unsigned int count);

void _BUILD_WEAK ulfs_init(void);

#endif
