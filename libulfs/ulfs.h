#ifndef _ULFS_H_
#define _ULFS_H_

#include "ulv_types.h"

int ulfs_open(const char *path, int flags, int mode);
void ulfs_close(int fd);
ssize_t ulfs_read(int fd, void *buf, size_t count);
ssize_t ulfs_write(int fd, const void *buf, size_t count);

void ulfs_init(void);

#endif
