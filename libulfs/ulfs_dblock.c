#include "ulfs_p.h"

#include "ulv_libc.h"

static bid_t
alloc_new_bblock(void)
{
	bidblock_t	*bb;
	bid_t	bid;

	bid = ulfs_block_alloc();
	if (bid == 0)
		return 0;
	bb = ulfs_block_get(bid);
	bb->next = 0;
	memset(bb->bids, 0, sizeof(bb->bids));
	return bid;
}

static bid_t
get_dblock_bid(inode_t *inode, lbid_t lbid, bool_t alloc_ok, bidblock_t **pbb, uint16_t *pidx_bb)
{
	bid_t	bid_bb;

	if (lbid == 0) {
		if (inode->bids_data[0] == 0) {
			if (!alloc_ok)
				return 0;
			inode->bids_data[0] = ulfs_block_alloc();
		}
		*pbb = NULL;
		return inode->bids_data[0];
	}
	if (inode->bids_data[1] == 0) {
		if (!alloc_ok)
			return 0;
		inode->bids_data[1] = ulfs_block_alloc();
	}

	bid_bb = inode->bids_data[1];

	while (1) {
		bidblock_t	*bb = ulfs_block_get(bid_bb);

		if (lbid <= N_BIDS_PER_BB) {
			if (bb->bids[lbid - 1] == 0) {
				if (!alloc_ok)
					return 0;
				bb->bids[lbid - 1] = alloc_new_bblock();
			}
			*pbb = bb;
			*pidx_bb = lbid - 1;
			return bb->bids[lbid - 1];
		}
		lbid -= N_BIDS_PER_BB;
		if (bb->next == 0) {
			if (!alloc_ok)
				return 0;
			bb->next = alloc_new_bblock();
		}
		bid_bb = bb->next;
	}

	/* never reach */

	return 0;
}

void *
ulfs_alloc_dblock(inode_t *inode, lbid_t lbid)
{
	bidblock_t	*bb;
	uint16_t	idx_bb;
	bid_t	bid;

	bid = get_dblock_bid(inode, lbid, TRUE, &bb, &idx_bb);
	return ulfs_block_get(bid);
}

void *
ulfs_get_dblock(inode_t *inode, lbid_t lbid, bool_t alloc_ok, bidblock_t **pbb, uint16_t *pidx_bb)
{
	bidblock_t	*bb;
	uint16_t	idx_bb;
	bid_t	bid;

	bid = get_dblock_bid(inode, lbid, alloc_ok, &bb, &idx_bb);
	if (bid == 0)
		return NULL;
	return ulfs_block_get(bid);
}

void
ulfs_free_data_blocks(inode_t *inode)
{
	bid_t	bid_bb;

	if (inode->bids_data[0] != 0) {
		ulfs_block_free(inode->bids_data[0]);
		inode->bids_data[0] = 0;
	}

	bid_bb = inode->bids_data[1];
	while (bid_bb != 0) {
		bidblock_t	*bb = ulfs_block_get(bid_bb);
		int	i;

		for (i = 0; i < N_BIDS_PER_BB; i++) {
			if (bb->bids[i] != 0)
				ulfs_block_free(bb->bids[i]);
		}
		ulfs_block_free(bid_bb);
		bid_bb = bb->next;
	}
}
