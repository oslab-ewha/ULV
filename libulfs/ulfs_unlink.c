#include "ulv_libc.h"

#include "ulfs_p.h"

int
ulfs_unlink(const char *pathname)
{
	path_t	path;
	inode_t	*inode_file;

	ulfs_path_init(&path, pathname);

	ulfs_path_dirname(&path);
	inode_file = ulfs_lookup_path(&path, NULL);
	if (inode_file == NULL)
		return -ENOENT;
	ulfs_path_init(&path, pathname);
	ulfs_path_basename(&path);

	if (!ulfs_dir_del_inode(inode_file, &path))
		return -ENOENT;

	return 0;
}
