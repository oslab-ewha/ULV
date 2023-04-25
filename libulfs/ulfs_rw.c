#include "ulfs_p.h"

#include "ulv_assert.h"

void *memcpy(void *, const void *, size_t);

#define MIN(a, b)	((a) > (b) ? (b): (a))

ssize_t
ulfs_read(int fd, void *buf, size_t count)
{
	ulfd_t	*ulfd;
	lbid_t	lbid;
	off_t	off_in_db;
	ssize_t	nread = 0;
	size_t	remain;

	ulfd = ulfs_get_ulfd(fd);
	if (ulfd == NULL)
		return -EBADF;

	lbid = ulfd->off / BSIZE;
	off_in_db = ulfd->off % BSIZE;
	remain = ulfd->inode->size - ulfd->off;

	while (count > 0 && remain > 0) {
		void	*data;
		uint16_t	remain_db;
		size_t	cpysize;

		data = ulfs_get_dblock(ulfd->inode, lbid, FALSE, &ulfd->bb, &ulfd->idx_bb);
		ULV_ASSERT(data != NULL);

		remain_db = BSIZE - off_in_db;

		cpysize = MIN(count, remain_db);
		cpysize = MIN(cpysize, remain);
		memcpy(buf, data + off_in_db, cpysize);
		ulfd->off += cpysize;
		count -= cpysize;
		nread += cpysize;
		buf += cpysize;
		remain -= cpysize;
		if (remain_db == cpysize) {
			lbid++;
			off_in_db = 0;
		}
	}

	return nread;
}

ssize_t
ulfs_write(int fd, const void *buf, size_t count)
{
	ulfd_t	*ulfd;
	lbid_t	lbid;
	off_t	off_in_db;
	ssize_t	nwrite = 0;

	ulfd = ulfs_get_ulfd(fd);
	if (ulfd == NULL)
		return -EBADF;

	lbid = ulfd->off / BSIZE;
	off_in_db = ulfd->off % BSIZE;

	while (count > 0) {
		void	*data;
		uint16_t	remain_db;
		size_t	cpysize;

		data = ulfs_get_dblock(ulfd->inode, lbid, TRUE, &ulfd->bb, &ulfd->idx_bb);
		remain_db = BSIZE - off_in_db;

		cpysize = MIN(count, remain_db);
		memcpy(data + off_in_db, buf, cpysize);
		ulfd->off += cpysize;
		count -= cpysize;
		nwrite += cpysize;
		buf += cpysize;
		if (remain_db == cpysize) {
			lbid++;
			off_in_db = 0;
		}
	}

	if (ulfd->off > ulfd->inode->size)
		ulfd->inode->size = ulfd->off;
	return nwrite;
}
