#include "ulfs_p.h"
#include "ulv_assert.h"

void
ulfs_sb_init(void)
{
	sb_t	*sb;

	sb = (sb_t *)ulfs_block_get(0);
	if (sb->magic != ULFS_SB_MAGIC)
		ULV_PANIC("ULFS: invalid super block");
}
