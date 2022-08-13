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

typedef enum {
	INODE_TYPE_NONE = 0,
	INODE_TYPE_FILE,
	INODE_TYPE_DIR,
} inode_type_t;

/* inode */
typedef struct {
	inode_type_t	type;
	uint64_t	size;
	bid_t	bids_data[2];		/* data block */
} inode_t;

#define N_INODES_PER_IB	((BSIZE - sizeof(bid_t) + sizeof(uint16_t) * 2) / sizeof(bid_t))

typedef struct {
	bid_t	next;
	uint16_t	n_used;
	uint16_t	dummy;
	inode_t	inodes[N_INODES_PER_IB];
} inode_block_t;

#define ULFS_NAME_MAX	26

#define N_DIRENT_PER_DB	(BSIZE / sizeof(dirent_t))

typedef struct {
	char	name[ULFS_NAME_MAX];
	bid_t	bid_ib;
	uint16_t	idx_ib;
} dirent_t;

typedef struct {
	inode_t	*inode;
	dirent_t	*head;
	dirent_t	*ent, *ent_last;
	bidblock_t	*bb;
	uint16_t	idx_bb;
	uint64_t	size_remain;
} dirlist_t;

void *ulfs_block_get(bid_t bid);
void ulfs_block_sync(bid_t bid);

bid_t ulfs_block_alloc(void);

void ulfs_block_init(void);

inode_t *ulfs_alloc_inode(inode_type_t type, bid_t *pbid_ib, uint16_t *pidx_ib);
inode_t *ulfs_get_inode(bid_t bid_ib, uint16_t idx_ib);
bid_t ulfs_alloc_data_block(inode_t *inode, bid_t bid);

inode_t *ulfs_lookup_path(const char *path);

int ulfs_dir_open(dirlist_t *dlist, const char *path);
dirent_t *ulfs_dir_get(dirlist_t *dlist);

inode_t *ulfs_dir_add_inode(inode_t *inode_dir, const char *name, inode_type_t type);
			  
#endif
