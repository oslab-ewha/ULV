#ifndef _ULFS_TOOL_H_
#define _ULFS_TOOL_H_

#include "ulv_libc.h"
#include "ulfs_p.h"
#include "ulfs.h"

void error(const char *fmt, ...);
long long get_fd_size(int fd);
int openr(const char *path);

int read(int fd, void *buf, size_t count);
int creat(const char *pathname, int mode);
int close(int fd);
int ftruncate(int fd, off_t length);

#endif
