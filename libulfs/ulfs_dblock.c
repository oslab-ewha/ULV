#include "ulfs_p.h"

#include "ulv_libc.h"
#include "ulv_assert.h"

/* data block means file data which can be traversed by inode bids_data[2] */

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
next_walk_dblock(dblock_walk_t *pwalk)
{
	bidblock_t	*bb;

	if (pwalk->bid_bb == 0) {
		if (pwalk->lbid != 0)
			return 0;
		pwalk->bid_bb = pwalk->inode->bids_data[1];
		if (pwalk->bid_bb == 0) {
			if (!pwalk->alloc_ok)
				return 0;
			pwalk->inode->bids_data[1] = pwalk->bid_bb = alloc_new_bblock();
		}
		pwalk->idx_bb = 0;
	}
	else {
		if (pwalk->idx_bb < N_BIDS_PER_BB - 1)
			pwalk->idx_bb++;
		else {
			bb = (bidblock_t *)ulfs_block_get(pwalk->bid_bb);
			if (bb->next == 0) {
				if (!pwalk->alloc_ok)
					return 0;
				bb->next = alloc_new_bblock();
				pwalk->bid_bb = bb->next;
			}
			pwalk->idx_bb = 0;
		}
	}

	bb = (bidblock_t *)ulfs_block_get(pwalk->bid_bb);
	if (bb->bids[pwalk->idx_bb] == 0) {
		if (!pwalk->alloc_ok)
			return 0;
		bb->bids[pwalk->idx_bb] = ulfs_block_alloc();
	}

	pwalk->lbid++;
	return bb->bids[pwalk->idx_bb];
}

static bid_t
get_dblock_bid(dblock_walk_t *pwalk)
{
	inode_t	*inode = pwalk->inode;
	lbid_t	lbid;
	bid_t	bid_bb;

	if (pwalk->lbid == 0) {
		if (inode->bids_data[0] == 0) {
			if (!pwalk->alloc_ok)
				return 0;
			inode->bids_data[0] = ulfs_block_alloc();
		}

		return inode->bids_data[0];
	}

	if (pwalk->bid_bb != 0) {
		bidblock_t	*bb;

		bb = ulfs_block_get(pwalk->bid_bb);
		return bb->bids[pwalk->idx_bb];
	}

	if (inode->bids_data[1] == 0) {
		if (!pwalk->alloc_ok)
			return 0;
		inode->bids_data[1] = alloc_new_bblock();
	}

	lbid = pwalk->lbid - 1;
	bid_bb = inode->bids_data[1];

	while (1) {
		bidblock_t	*bb = ulfs_block_get(bid_bb);

		if (lbid < N_BIDS_PER_BB) {
			if (bb->bids[lbid] == 0) {
				if (!pwalk->alloc_ok)
					return 0;
				bb->bids[lbid] = ulfs_block_alloc();
			}
			pwalk->bid_bb = bid_bb;
			pwalk->idx_bb = lbid;
			return bb->bids[lbid];
		}
		lbid -= N_BIDS_PER_BB;
		if (bb->next == 0) {
			if (!pwalk->alloc_ok)
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
	dblock_walk_t	walk;

	walk.inode = inode;
	walk.lbid = lbid;
	walk.alloc_ok = TRUE;
	walk.bid_bb = 0;

	return ulfs_get_dblock(&walk);
}

void *
ulfs_get_dblock(dblock_walk_t *pwalk)
{
	return ulfs_block_get(get_dblock_bid(pwalk));
}

void *
ulfs_first_dblock(inode_t *inode, lbid_t lbid, bool_t alloc_ok, dblock_walk_t *pwalk)
{
	pwalk->inode = inode;
	pwalk->lbid = lbid;
	pwalk->alloc_ok = alloc_ok;
	pwalk->bid_bb = 0;

	return ulfs_get_dblock(pwalk);
}

void *
ulfs_next_dblock(dblock_walk_t *pwalk)
{
	bid_t	bid;

	bid = next_walk_dblock(pwalk);
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
