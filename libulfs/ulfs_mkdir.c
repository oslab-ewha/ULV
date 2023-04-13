#include "ulv_libc.h"

#include "ulfs_p.h"

int
ulfs_mkdir(const char *pathname)
{
	path_t	path;
	inode_t	*inode_dir, *inode;
	dirent_t	*ent_parent, *ent_new, *ent_dir;

	ulfs_path_init(&path, pathname);

	ulfs_path_dirname(&path);
	inode_dir = ulfs_lookup_path(&path, &ent_parent);
	if (inode_dir == NULL)
		return -ENOENT;
	ulfs_path_init(&path, pathname);
	ulfs_path_basename(&path);

	inode = ulfs_dir_add_inode(inode_dir, &path, INODE_TYPE_DIR, &ent_new, FALSE);
	if (inode == NULL)
		return -EEXIST;
	ent_dir = (dirent_t *)ulfs_get_data_block(inode, 0);
	strcpy(ent_dir->name, ".");
	ent_dir->bid_ib = ent_new->bid_ib;
	ent_dir->idx_ib = ent_new->idx_ib;

	ent_dir++;
	strcpy(ent_dir->name, "..");
	ent_dir->bid_ib = ent_parent->bid_ib;
	ent_dir->idx_ib = ent_parent->idx_ib;

	inode->size = sizeof(dirent_t) * 2;

	return 0;
}

int
ulfs_rmdir(const char *pathname)
{
	path_t	path;
	inode_t	*inode_dir;

	ulfs_path_init(&path, pathname);

	ulfs_path_dirname(&path);
	inode_dir = ulfs_lookup_path(&path, NULL);
	if (inode_dir == NULL)
		return -ENOENT;
	ulfs_path_init(&path, pathname);
	ulfs_path_basename(&path);

	if (!ulfs_dir_del_inode(inode_dir, &path))
		return -ENOENT;

	return 0;
}
