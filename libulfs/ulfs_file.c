#include "ulfs_p.h"
#include "ulfs.h"

#include "ulv_syscall_flags.h"
#include "ulv_assert.h"
#include "ulv_dyntab.h"

static ulv_dyntab_t	ulfds;

int
ulfs_open(const char *pathname, int flags, int mode)
{
	path_t	path;
	inode_t	*inode_dir, *inode;
	dirent_t	*ent;
	ulfd_t	*ulfd;

	ulfs_path_init(&path, pathname);

	ulfs_path_dirname(&path);
	inode_dir = ulfs_lookup_path(&path, &ent);

	ulfs_path_init(&path, pathname);
	if (ulfs_path_is_root(&path)) {
		inode = inode_dir;
	}
	else {
		ulfs_path_basename(&path);

		if (flags & O_CREAT) {
			inode = ulfs_dir_add_inode(inode_dir, &path, INODE_TYPE_FILE, &ent, TRUE);
		} else {
			inode = ulfs_lookup_name(inode_dir, &path, &ent);
			if (inode == NULL)
				return -1;
		}
	}

	ulfd = (ulfd_t *)ulv_dyntab_assign(&ulfds);
	ulfd->off = 0;
	ulfd->inode = inode;
	ulfd->ino = ulfs_get_ino_from_dirent(ent);
	ulfd->walked = FALSE;

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

off_t
ulfs_lseek(int fd, off_t off, int whence)
{
	ulfd_t	*ulfd;

	ulfd = (ulfd_t *)ulv_dyntab_get(&ulfds, fd);
	if (ulfd == NULL)
		return -1;
	switch (whence) {
	case ULFS_SEEK_SET:
		ulfd->off = off;
		break;
	case ULFS_SEEK_CUR:
		ulfd->off += off;
		break;
	case ULFS_SEEK_END:
		if (off > 0)
			ulfd->off = ulfd->inode->size;
		else
			ulfd->off = ulfd->inode->size + off;
		break;
	default:
		return -1;
	}
	ulfd->walked = FALSE;
	return ulfd->off;
}

ulfd_t *
ulfs_get_ulfd(int fd)
{
	return (ulfd_t *)ulv_dyntab_get(&ulfds, fd);
}

int
ulfs_fstat(int fd, ulfs_stat_t *statbuf)
{
	ulfd_t	*ulfd;

	ulfd = (ulfd_t *)ulv_dyntab_get(&ulfds, fd);
	if (ulfd == NULL)
		return -1;

	statbuf->st_is_dir = (ulfd->inode->type == INODE_TYPE_DIR) ? 1: 0;
	statbuf->st_ino = ulfd->ino;
	statbuf->st_size = ulfd->inode->size;

	return 0;
}

void
ulfs_file_init(void)
{
	ulv_dyntab_init(&ulfds, sizeof(ulfd_t), 16);
}
