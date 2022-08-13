#include "ulfs_p.h"

#define INODE_OFFSET(ib, inode)	((char *)(inode) - (char *)(ib)->inodes)
#define IDX_IN_IB(ib, inode)	(INODE_OFFSET(ib, inode) / sizeof(inode_t))

static inline bid_t
get_next_inode_block_bid(inode_block_t *ib)
{
	if (ib->next == 0)
		ib->next = ulfs_block_alloc();
	return ib->next;
}

static inline inode_t *
get_empty_inode(inode_block_t *ib)
{
	inode_t	*inode;
	int	i;

	for (i = 0, inode = ib->inodes; i < N_INODES_PER_IB; i++, inode++) {
		if (inode->type == INODE_TYPE_NONE)
			return inode;
	}
	return NULL;
}

#if 0 ////TODO
static void
copy_path(const char *path, char *path_in_pb)
{
	const char	*p;
	char	*q;

	for (p = path, q = path_in_pb; *p; p++, q++)
		*q = *p;
	*q = '\0';
}

static uint16_t
get_pathlen(const char *path)
{
	const char	*p;

	for (p = path; *p; p++);
	return (uint16_t)(p - path + 1);
}

static bid_t
get_path_bid_off(const char *path, uint16_t *poff)
{
	uint16_t	pathlen;
	bid_t	bid_pb = 3;

	pathlen = get_pathlen(path);

	while (1) {
		path_block_t	*pb;

		pb = (path_block_t *)ulfs_block_get(bid_pb);
		if (AVAIL_PER_PATH_BLOCK(pb) >= pathlen) {
			*poff = 8 + pb->used;
			copy_path(path, pb->data + pb->used);
			pb->used += pathlen;
			return bid_pb;
		}
		if (pb->bid_next == 0)
			pb->bid_next = ulfs_block_alloc();
		bid_pb = pb->bid_next;
	}
}
#endif

inode_t *
ulfs_alloc_inode(inode_type_t type, bid_t *pbid_ib, uint16_t *pidx_ib)
{
	bid_t	bid_ib = 2;

	while (1) {
		inode_block_t	*ib;

		ib = (inode_block_t *)ulfs_block_get(bid_ib);
		if (ib->n_used < N_INODES_PER_IB) {
			inode_t	*inode;

			inode = get_empty_inode(ib);
			ib->n_used++;
			inode->type = type;

			*pbid_ib = bid_ib;
			*pidx_ib = IDX_IN_IB(ib, inode);
			return inode;
		}
		bid_ib = get_next_inode_block_bid(ib);
	}

	/* never reach */
	return NULL;
}

inode_t *
ulfs_get_inode(bid_t bid_ib, uint16_t idx_ib)
{
	inode_block_t	*ib;

	ib = (inode_block_t *)ulfs_block_get(bid_ib);
	return ib->inodes + idx_ib;
}
