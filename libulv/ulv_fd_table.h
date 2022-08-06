#ifndef _ULV_FD_TABLE_H_
#define _ULV_FD_TABLE_H_

#include "ulv_types.h"

typedef enum {
	FDTYPE_HOST = 1,
	FDTYPE_LWIP,
} fdtype_t;

int ulv_assign_fd(fdtype_t type, int fd_real);
void ulv_release_fd(int fd);
int ulv_lookup_fd_real(int fd, fdtype_t *ptype);
bool_t ulv_convert_fd_real(int *pfd, fdtype_t type);

#endif
