#include "ulfs_p.h"

bid_t
ulfs_alloc_data_block(inode_t *inode, lbid_t lbid, bidblock_t **pbb, uint16_t *pidx_bb)
{
	bid_t	bid_bb;

	if (lbid == 0) {
		if (inode->bids_data[0] == 0)
			inode->bids_data[0] = ulfs_block_alloc();
		*pbb = NULL;
		return inode->bids_data[0];
	}
	if (inode->bids_data[1] == 0)
		inode->bids_data[1] = ulfs_block_alloc();

	bid_bb = inode->bids_data[1];

	while (1) {
		bidblock_t	*bb = ulfs_block_get(bid_bb);

		if (lbid <= N_BIDS_PER_BB) {
			if (bb->bids[lbid - 1] == 0)
				bb->bids[lbid - 1] = ulfs_block_alloc();
			*pbb = bb;
			*pidx_bb = lbid - 1;
			return bb->bids[lbid - 1];
		}
		lbid -= N_BIDS_PER_BB;
		if (bb->next == 0)
			bb->next = ulfs_block_alloc();
		bid_bb = bb->next;
	}

	/* never reach */

	return 0;
}

bid_t
ulfs_alloc_data_block_next(inode_t *inode, bidblock_t **pbb, uint16_t *pidx_bb)
{
	bidblock_t	*bb = *pbb;
	uint16_t	idx_bb = *pidx_bb;

	if (bb == NULL) {
		inode->bids_data[1] = ulfs_block_alloc();
		bb = ulfs_block_get(inode->bids_data[1]);
		idx_bb = 0;
	}
	else {
		idx_bb++;
		if (idx_bb == N_BIDS_PER_BB) {
			bb->next = ulfs_block_alloc();
			bb = ulfs_block_get(bb->next);
			idx_bb = 0;
		}
	}
	if (bb->bids[idx_bb] == 0)
		bb->bids[idx_bb] = ulfs_block_alloc();
	*pbb = bb;
	*pidx_bb = idx_bb;

	return bb->bids[idx_bb];
}

void *
ulfs_get_data_block(inode_t *inode, lbid_t lbid)
{
	bidblock_t	*bb;
	uint16_t	idx_bb;
	bid_t	bid;

	bid = ulfs_alloc_data_block(inode, lbid, &bb, &idx_bb);
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
