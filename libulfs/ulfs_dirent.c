#include "ulfs_p.h"
#include "ulv_assert.h"

#include "ulfs.h"

static void
init_dirlist_with_ulfd(dirlist_t *dlist, ulfd_t *ulfd)
{
	void	*data;

	dlist->inode = ulfd->inode;
	dlist->off = ulfd->off;

	data = ulfs_first_dblock(ulfd->inode, ulfd->off / BSIZE, FALSE, &dlist->walk);
	if (data == NULL)
		dlist->ent = NULL;
	else {
		dlist->ent = (dirent_t *)(data + ulfd->off % BSIZE);
		dlist->idx_in_block = (ulfd->off % BSIZE) / sizeof(dirent_t);
	}
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

	ulfd = ulfs_get_ulfd(fd);
	if (ulfd == NULL)
		return -EBADF;

	init_dirlist_with_ulfd(&dirlist, ulfd);

	while (1) {
		dirent_t	*dirent;
		inode_t		*inode;
		uint32_t	ino;
		int	namelen, reclen;

		dirent = ulfs_dir_get(&dirlist);
		if (dirent == NULL)
			break;
		namelen = copy_name_len(dirp->d_name, dirent->name);
		reclen = namelen + sizeof(ulfs_dirent_t);
		if (count < nfilled + reclen)
			break;
		dirp->d_off = dirlist.off;
		dirp->d_reclen = reclen;
		inode = ulfs_get_inode(dirent->bid_ib, dirent->idx_ib, &ino);
		dirp->d_ino = (uint64_t)ino;
		dirp->d_type = inode->type;

		dirp = (ulfs_dirent_t *)(((uint8_t *)dirp) + reclen);
		nfilled += reclen;
	}

	ulfd->off = dirlist.off;
	return nfilled;
}
