#include "ulfs_p.h"

#define INODE_OFFSET(ib, inode)	((char *)(inode) - (char *)(ib)->inodes)
#define IDX_IN_IB(ib, inode)	(INODE_OFFSET(ib, inode) / sizeof(inode_t))

static inode_t	*inode_root;
static inode_t	*inode_cwd;

static inline bid_t
get_next_inode_block_bid(inode_block_t *ib)
{
	if (ib->next == 0) {
		inode_block_t	*ib_new;

		ib->next = ulfs_block_alloc();
		/* init inode block */
		ib_new = ulfs_block_get(ib->next);
		ib_new->next = 0;
		ib_new->ino_start = ib->ino_start + N_INODES_PER_IB;
		ib_new->n_used = 0;
	}
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

inode_t *
ulfs_alloc_inode(inode_type_t type, bid_t *pbid_ib, uint16_t *pidx_ib)
{
	bid_t	bid_ib = BID_INDB_START;

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

void
ulfs_free_inode(inode_t *inode)
{
	inode->type = INODE_TYPE_NONE;
}

inode_t *
ulfs_get_inode(bid_t bid_ib, uint16_t idx_ib, uint32_t *pino)
{
	inode_block_t	*ib;

	ib = (inode_block_t *)ulfs_block_get(bid_ib);
	if (pino)
		*pino = ib->ino_start + idx_ib;
	return ib->inodes + idx_ib;
}

inline inode_t *
ulfs_get_inode_root(void)
{
	if (inode_root == NULL) {
		inode_block_t	*ib;

		ib = ulfs_block_get(BID_INDB_START);
		inode_root = ib->inodes;
	}
	return inode_root;
}

inline inode_t *
ulfs_get_inode_cwd(void)
{
	if (inode_cwd == NULL)
		inode_cwd = ulfs_get_inode_root();
	return inode_cwd;
}
