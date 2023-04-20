#ifndef _ULFS_H_
#define _ULFS_H_

#include "ulv_build.h"
#include "ulv_types.h"

#define ERR_ULFS_NOTEXIST	-1

#if defined(ULFS_NO_WEAK)
#define _ULFS_WEAKABLE
#else
#define _ULFS_WEAKABLE	_BUILD_WEAK
#endif

#define ULFS_DT_UNKNOWN	0
#define ULFS_DT_DIR	4
#define ULFS_DT_REG	8

#define ULFS_SEEK_SET	0
#define ULFS_SEEK_CUR	1
#define ULFS_SEEK_END	2

typedef struct ulfs_dirent {
	uint64_t	d_ino;
	uint64_t	d_off;
	uint16_t	d_reclen;
	uint8_t		d_type;
	char		d_name[];
} ulfs_dirent_t;

int _ULFS_WEAKABLE ulfs_open(const char *path, int flags, int mode);
void _ULFS_WEAKABLE ulfs_close(int fd);
ssize_t _ULFS_WEAKABLE ulfs_read(int fd, void *buf, size_t count);
ssize_t _ULFS_WEAKABLE ulfs_write(int fd, const void *buf, size_t count);
off_t _ULFS_WEAKABLE ulfs_lseek(int fd, off_t offset, int whence);

int _ULFS_WEAKABLE ulfs_mkdir(const char *path);
int _ULFS_WEAKABLE ulfs_rmdir(const char *path);
int _ULFS_WEAKABLE ulfs_unlink(const char *path);

int _ULFS_WEAKABLE ulfs_getdents(int fd, ulfs_dirent_t *dirp, unsigned int count);

void _ULFS_WEAKABLE ulfs_init(void);

#endif
