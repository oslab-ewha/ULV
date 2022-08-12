#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_syscall_flags.h"
#include "ulv_assert.h"
#include "ulfs_p.h"

#define BID_TO_BLOCK(bid)	((char *)mapped + (bid) * BSIZE)
#define N_BIDS_PER_MB	((BSIZE - 4) * 8)
#define N_DEF_BIDS	4

static void	*mapped;

struct stat {
	char	dummy1[44];
	off_t	st_size;
	char	dummy2[84];
};

const char *getenv(const char *);

void *
ulfs_block_get(bid_t bid)
{
	return BID_TO_BLOCK(bid);
}

void
ulfs_block_sync(bid_t bid)
{
	if (__syscall3(__NR_msync, (long)BID_TO_BLOCK(bid), (long)BSIZE, (long)MS_SYNC) < 0)
		ULV_PANIC("msync failed");
}

static unsigned
alloc_bit_index(char *bitmap)
{
	unsigned	i;

	for (i = 0; i < BSIZE - sizeof(unsigned); i++) {
		if (bitmap[i] != 0xff) {
			int	j;
			char	mask = 0x80;
			for (j = 0; j < 8; j++) {
				if (!(bitmap[i] & mask)) {
					bitmap[i] |= mask;
					return i * 8 + j;
				}
				mask >>= 1;
			}
		}
	}
	return N_BIDS_PER_MB;
}

static void
alloc_new_block(mapblock_t *mapb, bid_t bid)
{
	mapblock_t	*mapb_new;

	mapb->next = bid + N_BIDS_PER_MB;
	mapb_new = (mapblock_t *)ulfs_block_get(mapb->next);
	mapb_new->bitmap[0] |= 0x80;
}

bid_t
ulfs_block_alloc(void)
{
	mapblock_t	*mapb;
	bid_t	bid_mapb = 1;
	bid_t	bid = N_DEF_BIDS;

	while (1) {
		mapb = (mapblock_t *)ulfs_block_get(bid_mapb);
		if (mapb->next == 0) {
			unsigned	bit_idx;

			bit_idx = alloc_bit_index(mapb->bitmap);
			if (bit_idx != N_BIDS_PER_MB)
				return bid + bit_idx;
			alloc_new_block(mapb, bid);
		}
		bid += N_BIDS_PER_MB;
		bid_mapb = mapb->next;
	}

	/* never reach */
	return 0;
}

void
ulfs_block_init(void)
{
	const char	*blkpath;
	int	fd;
	struct stat	statb;

	blkpath = getenv("ULV_BLOCK");
	if (blkpath == NULL)
		return;

	fd = __syscall2(__NR_open, (long)blkpath, (long)O_RDWR);
	if (fd < 0)
		ULV_PANIC("non-existent block path: %s", blkpath);

	if (__syscall2(__NR_fstat, (long)fd, (long)&statb) < 0)
		ULV_PANIC("failed to fstat");
	mapped = (void *)__syscall6(__NR_mmap, (long)0, (long)statb.st_size, (long)(PROT_READ | PROT_WRITE), (long)MAP_SHARED, (long)fd, (long)0);
	if (mapped == MAP_FAILED)
		ULV_PANIC("failed to mmap");
	__syscall1(__NR_close, (long)fd);
}
