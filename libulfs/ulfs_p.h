#ifndef _ULFS_P_H_
#define _ULFS_P_H_

#ifndef ULFS_USE_GLIBC
#include "ulv_types.h"
#endif

#define BSIZE		4096
#define ULFS_SB_MAGIC	0x11089160

typedef unsigned	bid_t;

typedef struct {
	unsigned	magic;
} sb_t;

typedef struct {
	bid_t	next;
	char	bitmap[BSIZE - sizeof(bid_t)];
} mapblock_t;

#define N_BIDS_PER_BB	((BSIZE - sizeof(bid_t)) / sizeof(bid_t))

typedef struct {
	bid_t	next;
	bid_t	bids[N_BIDS_PER_BB];
} bidblock_t;

/* file entry */
typedef struct {
	uint16_t	path_off;	/* 0 means empty or link to next */
	bid_t	bid_path;		/* path name block or next file block */
	uint64_t	size;
	bid_t	bids_data[2];		/* data block */
} fe_t;

typedef struct {
	bid_t	bid_next;
	unsigned	used;
	char	data[1];
} path_block_t;

void *ulfs_block_get(bid_t bid);
void ulfs_block_sync(bid_t bid);

bid_t ulfs_block_alloc(void);

void ulfs_block_init(void);

fe_t *ulfs_alloc_fe(const char *path);
bid_t ulfs_alloc_data_block(fe_t *fe, bid_t bid);

#endif
