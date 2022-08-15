#include "ulfs_p.h"

void *memcpy(void *, const void *, size_t);

#define MIN(a, b)	((a) > (b) ? (b): (a))

ssize_t
ulfs_read(int fd, void *buf, size_t count)
{
	ulfd_t	*ulfd;
	ssize_t	nread = 0;
	size_t	remain;

	ulfd = ulfs_get_ulfd(fd);
	if (ulfd == NULL)
		return -1;
	if (ulfd->data == NULL)
		ulfd->data = ulfs_block_get(ulfs_alloc_data_block(ulfd->inode, ulfd->off / BSIZE, &ulfd->bb, &ulfd->idx_bb));
	remain = ulfd->inode->size - ulfd->off;
	while (count > 0 && remain > 0) {
		uint16_t	remain_db;
		off_t	off_in_db = ulfd->off % BSIZE;
		size_t	cpysize;

		remain_db = BSIZE - off_in_db;

		cpysize = MIN(count, remain_db);
		cpysize = MIN(cpysize, remain);
		memcpy(buf, ulfd->data + off_in_db, cpysize);
		ulfd->off += cpysize;
		count -= cpysize;
		nread += cpysize;
		buf += cpysize;
		remain -= cpysize;
		if (count > 0) {
			ulfd->data = ulfs_block_get(ulfs_alloc_data_block_next(ulfd->inode, &ulfd->bb, &ulfd->idx_bb));
		}
	}

	return nread;
}

ssize_t
ulfs_write(int fd, const void *buf, size_t count)
{
	ulfd_t	*ulfd;
	ssize_t	nwrite = 0;

	ulfd = ulfs_get_ulfd(fd);
	if (ulfd == NULL)
		return -1;
	if (ulfd->data == NULL)
		ulfd->data = ulfs_block_get(ulfs_alloc_data_block(ulfd->inode, ulfd->off / BSIZE, &ulfd->bb, &ulfd->idx_bb));

	while (count > 0) {
		uint16_t	remain_db;
		off_t	off_in_db = ulfd->off % BSIZE;
		size_t	cpysize;

		remain_db = BSIZE - off_in_db;

		cpysize = MIN(count, remain_db);
		memcpy(ulfd->data + off_in_db, buf, cpysize);
		ulfd->off += cpysize;
		count -= cpysize;
		nwrite += cpysize;
		buf += cpysize;
		if (count > 0) {
			ulfd->data = ulfs_block_get(ulfs_alloc_data_block_next(ulfd->inode, &ulfd->bb, &ulfd->idx_bb));
		}
	}

	if (ulfd->off > ulfd->inode->size)
		ulfd->inode->size = ulfd->off;
	return nwrite;
}
