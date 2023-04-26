#include "ulfs_p.h"

#include "ulv_libc.h"
#include "ulv_assert.h"

#define MIN(a, b)	((a) > (b) ? (b): (a))

ssize_t
ulfs_read(int fd, void *buf, size_t count)
{
	ulfd_t	*ulfd;
	off_t	off_in_db;
	void	*data;
	ssize_t	nread = 0;
	size_t	remain;

	ulfd = ulfs_get_ulfd(fd);
	if (ulfd == NULL)
		return -EBADF;

	if (!ulfd->walked) {
		data = ulfs_first_dblock(ulfd->inode, ulfd->off / BSIZE, FALSE, &ulfd->walk);
		ulfd->walked = TRUE;
	}
	else
		data = ulfs_get_dblock(&ulfd->walk);

	off_in_db = ulfd->off % BSIZE;
	remain = ulfd->inode->size - ulfd->off;

	while (count > 0 && remain > 0) {
		uint16_t	remain_db;
		size_t	cpysize;

		remain_db = BSIZE - off_in_db;

		cpysize = MIN(count, remain_db);
		cpysize = MIN(cpysize, remain);

		if (data)
			memcpy(buf, data + off_in_db, cpysize);
		else
			memset(buf, 0, cpysize);

		ulfd->off += cpysize;
		count -= cpysize;
		nread += cpysize;
		buf += cpysize;
		remain -= cpysize;
		if (remain_db == cpysize) {
			data = ulfs_next_dblock(&ulfd->walk);
			off_in_db = 0;
		}
	}

	return nread;
}

ssize_t
ulfs_write(int fd, const void *buf, size_t count)
{
	ulfd_t	*ulfd;
	off_t	off_in_db;
	void	*data;
	ssize_t	nwrite = 0;

	ulfd = ulfs_get_ulfd(fd);
	if (ulfd == NULL)
		return -EBADF;

	if (!ulfd->walked) {
		data = ulfs_first_dblock(ulfd->inode, ulfd->off / BSIZE, TRUE, &ulfd->walk);
		ulfd->walked = TRUE;
	}
	else
		data = ulfs_get_dblock(&ulfd->walk);

	if (data == NULL)
		return -EIO;

	off_in_db = ulfd->off % BSIZE;

	while (count > 0) {
		uint16_t	remain_db;
		size_t	cpysize;

		remain_db = BSIZE - off_in_db;

		cpysize = MIN(count, remain_db);
		memcpy(data + off_in_db, buf, cpysize);
		ulfd->off += cpysize;
		count -= cpysize;
		nwrite += cpysize;
		buf += cpysize;
		if (remain_db == cpysize) {
			data = ulfs_next_dblock(&ulfd->walk);
			if (data == NULL)
				return -EIO;
			off_in_db = 0;
		}
	}

	if (ulfd->off > ulfd->inode->size)
		ulfd->inode->size = ulfd->off;
	return nwrite;
}
