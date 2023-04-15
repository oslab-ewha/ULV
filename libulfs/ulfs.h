#ifndef _ULFS_H_
#define _ULFS_H_

#include "ulv_build.h"
#include "ulv_types.h"

#define ERR_ULFS_NOTEXIST	-1

int _BUILD_WEAK ulfs_open(const char *path, int flags, int mode);
void _BUILD_WEAK ulfs_close(int fd);
ssize_t _BUILD_WEAK ulfs_read(int fd, void *buf, size_t count);
ssize_t _BUILD_WEAK ulfs_write(int fd, const void *buf, size_t count);

int _BUILD_WEAK ulfs_mkdir(const char *path);
int _BUILD_WEAK ulfs_rmdir(const char *path);
int _BUILD_WEAK ulfs_unlink(const char *path);

void _BUILD_WEAK ulfs_init(void);

#endif
