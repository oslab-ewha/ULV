#include "ulfs_p.h"
#include "ulv_assert.h"

#include "ulfs.h"

static void
init_dirlist_with_ulfd(dirlist_t *dlist, ulfd_t *ulfd)
{
	dlist->inode = ulfd->inode;
	dlist->size_remain = ulfd->inode->size - ulfd->off;
	dlist->bb = ulfd->bb;
	dlist->idx_bb = ulfd->idx_bb;
	dlist->head = dlist->ent = (dirent_t *)(ulfd->data + ulfd->off % BSIZE);
}

static int
copy_name_len(char *dst, const char *src)
{
	char	*d = dst;

	for (; *src != '\0'; d++, src++)
		*d = *src;

	*d = '\0';
	return (int)(d - dst) + 1;
}

int
ulfs_getdents(int fd, ulfs_dirent_t *dirp, unsigned int count)
{
	dirlist_t	dirlist;
	ulfd_t	*ulfd;
	int	nfilled = 0;
	off_t	off;

	ulfd = ulfs_get_ulfd_data(fd);
	if (ulfd == NULL)
		return -EBADF;

	init_dirlist_with_ulfd(&dirlist, ulfd);
	off = ulfd->off;

	while (1) {
		dirent_t	*dirent;
		inode_t		*inode;
		int	namelen, reclen;

		dirent = ulfs_dir_get(&dirlist);
		if (dirent == NULL)
			break;
		namelen = copy_name_len(dirp->d_name, dirent->name);
		reclen = namelen + sizeof(ulfs_dirent_t);
		if (count < nfilled + reclen)
			break;
		dirp->d_ino = 1; //TODO
		dirp->d_off = off;
		dirp->d_reclen = reclen;
		inode = ulfs_get_inode(dirent->bid_ib, dirent->idx_ib);
		dirp->d_type = inode->type;

		dirp = (ulfs_dirent_t *)(((uint8_t *)dirp) + reclen);
		off += sizeof(dirent_t);
		nfilled += reclen;
	}

	ulfd->off = off;
	return nfilled;
}
