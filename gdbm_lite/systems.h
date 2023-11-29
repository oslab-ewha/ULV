#include <sys/types.h>

#include <unistd.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <limits.h>
#include <sys/file.h>

#ifndef CHAR_BIT
#define CHAR_BIT	8
#endif

#if HAVE_STRUCT_STAT_ST_BLKSIZE
# define STATBLKSIZE(st) (st).st_blksize
#else
# define STATBLKSIZE(st) 1024
#endif
