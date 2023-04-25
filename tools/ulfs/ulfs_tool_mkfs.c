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
do_mk_sb(long size)
{
	sb_t	*sb;

	sb = (sb_t *)ulfs_block_get(0);
	sb->magic = ULFS_SB_MAGIC;
	sb->max_blocks = size / BSIZE;
	ulfs_block_sync(0);
}

static void
do_init_mapb(void)
{
	mapblock_t	*mapb;

	mapb = (mapblock_t *)ulfs_block_get(BID_MAPB_START);
	mapb->n_frees = ULFS_ENDOFMAPB;
	memset(mapb->bitmap, 0, sizeof(mapb->bitmap));
	ulfs_block_sync(BID_MAPB_START);
}

static void
do_init_indb(void)
{
	inode_block_t	*indb;

	/* This bid of first inode block willl be BID_IND_START */
	indb = (inode_block_t *)ulfs_block_get(ulfs_block_alloc());
	indb->next = 0;
	indb->prev = 0;
	indb->ino_start = 1;
	indb->n_used = 0;
	ulfs_block_sync(BID_INDB_START);
}

static void
do_mk_root(void)
{
	inode_t	*inode;
	bid_t	bid_ib;
	uint16_t	idx_ib;
	dirent_t	*ent;

	inode = ulfs_alloc_inode(INODE_TYPE_DIR, &bid_ib, &idx_ib);
	ent = (dirent_t *)ulfs_alloc_dblock(inode, 0);

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
do_mkfs(long size)
{
	do_mk_sb(size);
	do_init_mapb();
	do_init_indb();
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

	do_mkfs(size);

	return 0;
}
