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
do_mk_root(void)
{
	inode_t	*inode;
	bid_t	bid_data, bid_ib;
	uint16_t	idx_ib;
	dirent_t	*ent;

	inode = ulfs_alloc_inode(INODE_TYPE_DIR, &bid_ib, &idx_ib);
	bid_data = ulfs_alloc_data_block(inode, 0);
	ent = (dirent_t *)ulfs_block_get(bid_data);

	strcpy(ent->name, ".");
	ent->bid_ib = bid_ib;
	ent->idx_ib = idx_ib;

	ent++;
	strcpy(ent->name, "..");
	ent->bid_ib = bid_ib;
	ent->idx_ib = idx_ib;

	inode->size = sizeof(dirent_t) * 2;
}

static void
do_mkfs(void)
{
	do_mk_sb();
	do_mk_root();
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
