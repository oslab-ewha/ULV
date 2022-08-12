#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define ULFS_USE_GLIBC
#include <inttypes.h>
#include "ulfs_p.h"

#include "ulfs_tool.h"

static long
parse_sizestr(const char *arg)
{
	long	size;
	char	unit;

	if (sscanf(arg, "%lu%c", &size, &unit) == 2) {
		if (unit == 'k')
			size *= 1024;
		else if (unit == 'm')
			size *= 1024 * 1024;
		else if (unit == 'g')
			size *= 1024 * 1024 * 1024;
		else {
			error("invalid size unit");
			return -1;
		}
	}
	return size;
}

static void
do_mk_sb(void)
{
	sb_t	*sb;

	sb = (sb_t *)ulfs_block_get(0);
	sb->magic = ULFS_SB_MAGIC;
	ulfs_block_sync(0);
}

static void
do_mkfs(void)
{
	do_mk_sb();
}

int
ulfs_tool_mkfs(int argc, char *argv[])
{
	long	size;
	int	fd;

	if (argc < 2) {
		error("insufficient arguments");
		return 2;
	}

	size = parse_sizestr(argv[1]);
	if (size < 0)
		return 3;
	if (size < BSIZE * 3) {
		error("too small fs size");
		return 3;
	}
	fd = creat(argv[0], 0660);
	if (fd < 0) {
		error("failed to create fs image");
		return 3;
	}

	if (ftruncate(fd, size) < 0) {
		error("failed to truncate");
		close(fd);
		return 3;
	}
	close(fd);

	setenv("ULV_BLOCK", argv[0], 1);
	ulfs_block_init();

	do_mkfs();

	return 0;
}
