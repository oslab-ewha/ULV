#include <string.h>
#include <stdio.h>

#define ULFS_USE_GLIBC
#include "ulfs_p.h"

#include "ulfs_tool.h"

static char	typechars[] = { 'f', 'd' };

static int
do_ls(const char *_path)
{
	path_t		path;
	dirlist_t	dir;
	dirent_t	*ent;

	ulfs_path_init(&path, _path);
	if (ulfs_dir_open(&dir, &path) < 0) {
		error("%s: not exist or not directory", _path);
		return 2;
	}

	while ((ent = ulfs_dir_get(&dir))) {
		inode_t	*inode;

		inode = ulfs_get_inode(ent->bid_ib, ent->idx_ib);
		printf("%c %s %lu\n", typechars[inode->type - 1], ent->name, inode->size);
	}

	return 0;
}

int
ulfs_tool_ls(int argc, char *argv[])
{
	ulfs_block_init();

	if (argc < 1)
		return do_ls("/");
	
	return do_ls(argv[0]);
}
