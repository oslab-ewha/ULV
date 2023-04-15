#include <string.h>
#include <stdio.h>

#define ULFS_USE_GLIBC
#include "ulfs_p.h"
#include "ulfs.h"

#include "ulfs_tool.h"

int
ulfs_tool_rm(int argc, char *argv[])
{
	int	ret;

	ulfs_block_init();

	if (argc < 1) {
		error("insufficient arguments");
		return 2;
	}

	ret = ulfs_unlink(argv[0]);
	switch (ret) {
	case 0:
		return 0;
	case -ENOENT:
		error("no such file");
		return 3;
	default:
		error("unknown error: %d", ret);
		return 4;
	}
}

int
ulfs_tool_mkdir(int argc, char *argv[])
{
	ulfs_block_init();

	if (argc < 1) {
		error("insufficient arguments");
		return 2;
	}

	return ulfs_mkdir(argv[0]);
}

int
ulfs_tool_rmdir(int argc, char *argv[])
{
	ulfs_block_init();

	if (argc < 1) {
		error("insufficient arguments");
		return 2;
	}

	return ulfs_rmdir(argv[0]);
}
