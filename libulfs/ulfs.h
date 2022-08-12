#ifndef _ULFS_H_
#define _ULFS_H_

int ulfs_open(const char *pathname, int flags, int mode);
void ulfs_close(int fd);
void ulfs_init(void);

#endif
