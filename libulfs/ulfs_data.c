#include "ulfs_p.h"

bid_t
ulfs_alloc_data_block(inode_t *inode, bid_t bid)
{
	bid_t	bid_bb;

	if (bid == 0) {
		if (inode->bids_data[0] == 0)
			inode->bids_data[0] = ulfs_block_alloc();
		return inode->bids_data[0];
	}
	if (inode->bids_data[1] == 0)
		inode->bids_data[1] = ulfs_block_alloc();

	bid_bb = inode->bids_data[1];

	while (1) {
		bidblock_t	*bb = ulfs_block_get(bid_bb);

		if (bid <= N_BIDS_PER_BB) {
			if (bb->bids[bid - 1] == 0)
				bb->bids[bid - 1] = ulfs_block_alloc();
			return bb->bids[bid - 1];
		}
		bid -= N_BIDS_PER_BB;
		if (bb->next == 0)
			bb->next = ulfs_block_alloc();
		bid_bb = bb->next;
	}

	/* never reach */

	return 0;
}
