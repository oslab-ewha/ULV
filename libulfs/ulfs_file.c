#include "ulfs_p.h"

#include "ulv_syscall_flags.h"
#include "ulv_assert.h"
#include "ulv_dyntab.h"

static ulv_dyntab_t	ulfds;

int
ulfs_open(const char *pathname, int flags, int mode)
{
	path_t	path;
	inode_t	*inode_dir, *inode;
	ulfd_t	*ulfd;

	ulfs_path_init(&path, pathname);

	ulfs_path_dirname(&path);
	inode_dir = ulfs_lookup_path(&path, NULL);

	ulfs_path_init(&path, pathname);
	if (ulfs_path_is_root(&path)) {
		inode = inode_dir;
	}
	else {
		ulfs_path_init(&path, pathname);
		ulfs_path_basename(&path);

		if (flags & O_CREAT) {
			inode = ulfs_dir_add_inode(inode_dir, &path, INODE_TYPE_FILE, NULL, TRUE);
		} else {
			inode = ulfs_lookup_name(inode_dir, &path, NULL);
			if (inode == NULL)
				return -1;
		}
	}

	ulfd = (ulfd_t *)ulv_dyntab_assign(&ulfds);
	ulfd->off = 0;
	ulfd->inode = inode;
	ulfd->data = NULL;
	ulfd->bb = NULL;
	ulfd->idx_bb = 0;

	return ULV_DYNTAB_ENTRY_IDX(ulfd);
}

void
ulfs_close(int fd)
{
	ulfd_t	*ulfd;

	ulfd = ulv_dyntab_get(&ulfds, fd);
	if (ulfd) {
		ulv_dyntab_release(&ulfds, ulfd);
	}
}

ulfd_t *
ulfs_get_ulfd(int fd)
{
	return (ulfd_t *)ulv_dyntab_get(&ulfds, fd);
}

ulfd_t *
ulfs_get_ulfd_data(int fd)
{
	ulfd_t	*ulfd;

	ulfd = (ulfd_t *)ulv_dyntab_get(&ulfds, fd);
	if (ulfd == NULL)
		return NULL;
	if (ulfd->data == NULL) {
		ulfd->data = ulfs_block_get(ulfs_alloc_data_block(ulfd->inode, ulfd->off / BSIZE, &ulfd->bb, &ulfd->idx_bb));
	}
	return ulfd;
}

void
ulfs_file_init(void)
{
	ulv_dyntab_init(&ulfds, sizeof(ulfd_t), 16);
}
