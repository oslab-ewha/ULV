#include "ulfs_tool.h"

#include "ulv_assert.h"

static void
check_sanity_ib_n_used(inode_block_t *ib)
{
	int	n_used = 0;
	int	i;

	for (i = 0; i < N_INODES_PER_IB; i++) {
		inode_t	*inode = ib->inodes + i;
		if (inode->type != INODE_TYPE_NONE)
			n_used++;
	}
	ULV_ASSERT(ib->n_used == n_used);
}

int
ulfs_tool_get_n_used_inodes(void)
{
	int	n_used_inodes = 0;
	bid_t	bid_ib = BID_INDB_START;

	while (1) {
		inode_block_t	*ib;

		ib = (inode_block_t *)ulfs_block_get(bid_ib);
		check_sanity_ib_n_used(ib);
		n_used_inodes += ib->n_used;
		if (ib->next == 0)
			break;
		bid_ib = ib->next;
	}

	return n_used_inodes;
}

int
ulfs_tool_info(int argc, char *argv[])
{
	int	n_used_inodes;

	ulfs_block_init();

	n_used_inodes = ulfs_tool_get_n_used_inodes();
	printf("# of used inodes: %d\n", n_used_inodes);

	return 0;
}
