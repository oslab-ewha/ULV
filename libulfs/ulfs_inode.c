#include "ulfs_p.h"

#include "ulv_assert.h"

#define INODE_OFFSET(ib, inode)	((char *)(inode) - (char *)(ib)->inodes)
#define IDX_IN_IB(ib, inode)	(INODE_OFFSET(ib, inode) / sizeof(inode_t))

static inode_t	*inode_root;
static inode_t	*inode_cwd;

static bid_t
alloc_new_iblock(void)
{
	inode_block_t	*ib;
	bid_t	bid;
	int	i;

	bid = ulfs_block_alloc();
	if (bid == 0)
		return 0;
	ib = ulfs_block_get(bid);
	ib->next = 0;
	ib->prev = 0;

	for (i = 0; i < N_INODES_PER_IB; i++)
		ib->inodes[i].type = INODE_TYPE_NONE;

	return bid;
}

static inline bid_t
get_next_inode_block_bid(inode_block_t *ib)
{
	if (ib->next == 0) {
		inode_block_t	*ib_new;
		inode_block_t	*ib_prev = NULL;

		if (ib->prev != 0)
			ib_prev = ulfs_block_get(ib->prev);
		ib->next = alloc_new_iblock();
		ib_new = ulfs_block_get(ib->next);
		ib_new->next = 0;
		if (ib_prev)
			ib_new->prev = ib_prev->next;
		else
			ib_new->prev = BID_INDB_START;
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
			inode->bids_data[0] = 0;
			inode->bids_data[1] = 0;

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
ulfs_free_inode(bid_t bid_ib, inode_t *inode)
{
	inode_block_t	*ib;

	inode->type = INODE_TYPE_NONE;

	ib = ulfs_block_get(bid_ib);
	ULV_ASSERT(ib->n_used > 0);
	ib->n_used--;
	if (ib->n_used == 0) {
		inode_block_t	*ib_prev = NULL;

		if (ib->next != 0) {
			inode_block_t	*ib_next;

			ib_next = ulfs_block_get(ib->next);
			if (ib->prev == 0) {
				/* ASSUME: root inode block is never freed */
				ib_next->prev = 0;
			}
			else {
				ib_prev = ulfs_block_get(ib->prev);
				ib_prev->next = ib->next;
				ib_next->prev = ib->prev;
			}
		}
		else {
			/* ASSUME: root inode block is never freed */
			ULV_ASSERT(ib->prev != 0);

			ib_prev = ulfs_block_get(ib->prev);
			ib_prev->next = 0;
		}
		ulfs_block_free(bid_ib);
	}
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
