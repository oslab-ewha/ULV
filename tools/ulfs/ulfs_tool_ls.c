#include <string.h>
#include <stdio.h>

#define ULFS_USE_GLIBC
#include "ulfs_p.h"

#include "ulfs_tool.h"

static char	typechars[] = { 'f', 'd' };
static bool_t	ls_size = FALSE;
static bool_t	ls_inode = FALSE;

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
		uint32_t	ino;

		inode = ulfs_get_inode(ent->bid_ib, ent->idx_ib, &ino);
		printf("%c %s", typechars[inode->type - 1], ent->name);
		if (ls_size)
			printf(" %lu", inode->size);
		if (ls_inode)
			printf(" %u", ino);
		printf("\n");
	}

	return 0;
}

int
ulfs_tool_ls(int argc, char *argv[])
{
	ulfs_block_init();

	if (argc < 1)
		return do_ls("/");
	if (*argv[0] == '-') {
		int	i;

		for (i = 1; argv[0][i]; i++) {
			switch (argv[0][i]) {
			case 's':
				ls_size = TRUE;
				break;
			case 'i':
				ls_inode = TRUE;
				break;
			default:
				error("%c: unknown option", argv[0][i]);
				break;
			}
		}

		if (argc < 2)
			return do_ls("/");
		return do_ls(argv[1]);
	}
	return do_ls(argv[0]);
}
