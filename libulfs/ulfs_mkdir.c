#include "ulfs_p.h"

int
ulfs_mkdir(const char *pathname)
{
	path_t	path;
	inode_t	*inode_dir, *inode;

	ulfs_path_init(&path, pathname);

	ulfs_path_dirname(&path);
	inode_dir = ulfs_lookup_path(&path);
	if (inode_dir == NULL)
		return -ENOENT;
	ulfs_path_init(&path, pathname);
	ulfs_path_basename(&path);

	inode = ulfs_dir_add_inode(inode_dir, &path, INODE_TYPE_DIR, FALSE);
	if (inode == NULL)
		return -EEXIST;

	return 0;
}
