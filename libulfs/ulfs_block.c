#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"
#include "ulv_syscall_flags.h"
#include "ulv_assert.h"
#include "ulv_libc.h"

#include "ulfs_p.h"

#define BID_TO_BLOCK(bid)	((char *)mapped + (bid) * BSIZE)
#define N_BIDS_PER_MAPB	((BSIZE - 4) * 8)

#define BID_MAPB_NEXT(bid_mapb)	 ((bid_mapb) + N_BIDS_PER_MAPB + 1)
#define BID_MAPB_FROM_BID(bid)	 ((((bid) - BID_MAPB_START) / (N_BIDS_PER_MAPB + 1)) * (N_BIDS_PER_MAPB + 1) + 1)

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
			unsigned char	mask = 0x80;
			for (j = 0; j < 8; j++) {
				if (!(bitmap[i] & mask)) {
					bitmap[i] |= mask;
					return i * 8 + j;
				}
				mask >>= 1;
			}
		}
	}
	return N_BIDS_PER_MAPB;
}

static void
free_bit_index(char *bitmap, int ord)
{
	bitmap[ord / 8] &= ~(((unsigned char)0x80) >> (ord % 8));
}

static void
setup_new_mapb(mapblock_t *mapb_new, bid_t bid_mapb)
{
	mapblock_t	*mapb_next;

	mapb_next = (mapblock_t *)ulfs_block_get(BID_MAPB_NEXT(bid_mapb));
	mapb_next->n_frees = ULFS_ENDOFMAPB;
	mapb_new->n_frees = N_BIDS_PER_MAPB;
	memset(mapb_new->bitmap, 0, sizeof(mapb_new->bitmap));
}

bid_t
ulfs_block_alloc(void)
{
	mapblock_t	*mapb;
	bid_t	bid_mapb = BID_MAPB_START;

	while (1) {
		mapb = (mapblock_t *)ulfs_block_get(bid_mapb);
		if (mapb->n_frees == ULFS_ENDOFMAPB)
			setup_new_mapb(mapb, bid_mapb);
		if (mapb->n_frees > 0) {
			unsigned	bit_idx;

			bit_idx = alloc_bit_index(mapb->bitmap);
			ULV_ASSERT(bit_idx != N_BIDS_PER_MAPB);
			mapb->n_frees--;
			return bid_mapb + 1 + bit_idx;
		}
		bid_mapb = BID_MAPB_NEXT(bid_mapb);
	}

	/* never reach */
	return 0;
}

void
ulfs_block_free(bid_t bid)
{
	mapblock_t	*mapb;
	bid_t	bid_mapb = BID_MAPB_FROM_BID(bid);

	mapb = (mapblock_t *)ulfs_block_get(bid_mapb);
	ULV_ASSERT(mapb->n_frees != ULFS_ENDOFMAPB);
	mapb->n_frees++;
	free_bit_index(mapb->bitmap, bid - bid_mapb - 1);
}

void
ulfs_block_init(void)
{
	const char	*blkpath;
	int	fd;
	struct stat	statb;

	blkpath = getenv("ULV_BLOCK");
	if (blkpath == NULL)
		ULV_PANIC("ULFS backend host file is missing. Please set ULV_BLOCK!");

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
