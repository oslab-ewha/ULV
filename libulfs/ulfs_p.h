#ifndef _ULFS_P_H_
#define _ULFS_P_H_

/* ulfs private header */

#include "ulv_types.h"
#include "errno.h"

#define BSIZE		4096
#define ULFS_SB_MAGIC	0x11089160
#define ULFS_ENDOFMAPB	((unsigned)((int)(-1)))

/* map block starts at bid 1 */
#define BID_MAPB_START	1

/* inode block starts at bid 2 */
#define BID_INDB_START	2

/* root data block starts at bid 3 */
#define BID_ROOTB_START	3

/* block id */
typedef unsigned	bid_t;
/* logical block id for linear file */
typedef unsigned	lbid_t;

typedef struct {
	unsigned	magic;
	unsigned	max_blocks;
} sb_t;

typedef struct {
	unsigned	n_frees;
	char	bitmap[BSIZE - sizeof(bid_t)];
} mapblock_t;

#define N_BIDS_PER_BB	((BSIZE - sizeof(int)) / sizeof(bid_t))

typedef struct {
	bid_t	next;
	bid_t	bids[N_BIDS_PER_BB];
} bidblock_t;

typedef enum {
	INODE_TYPE_NONE = 0,
	INODE_TYPE_DIR = 4,
	INODE_TYPE_FILE = 8
} inode_type_t;

/* inode */
typedef struct {
	inode_type_t	type;
	uint64_t	size;
	/* the first is just a data block and the second is bid block */
	bid_t	bids_data[2];
} inode_t;

#define N_INODES_PER_IB	((BSIZE - offsetof(inode_block_t, inodes)) / sizeof(inode_t))

typedef struct {
	bid_t	next, prev;
	uint32_t	ino_start;
	uint16_t	n_used;
	uint16_t	dummy;
	inode_t	inodes[];
} inode_block_t;

typedef struct {
	inode_t		*inode;
	bool_t	alloc_ok;
	lbid_t		lbid;
	bid_t		bid_bb;
	uint16_t	idx_bb;
} dblock_walk_t;

typedef struct {
        off_t	off;
	inode_t	*inode;
	bool_t		walked;
	dblock_walk_t	walk;
} ulfd_t;

#define ULFS_NAME_MAX	26

#define N_DIRENT_PER_DB	(BSIZE / sizeof(dirent_t))

typedef struct {
	char	name[ULFS_NAME_MAX];
	bid_t	bid_ib;
	uint16_t	idx_ib;
} dirent_t;

typedef struct {
	inode_t		*inode;
	/* entry index in dir data block */
	unsigned	idx_in_block;
	dirent_t	*ent;
	/* current listing entry offset in dir data */
	off_t		off;
	dblock_walk_t	walk;
} dirlist_t;

typedef struct {
	const char	*start, *start_name;
	const char	*end, *end_name;
} path_t;

void *ulfs_block_get(bid_t bid);
void ulfs_block_sync(bid_t bid);

bid_t ulfs_block_alloc(void);
void ulfs_block_free(bid_t bid);

void ulfs_block_init(void);
void ulfs_block_set_max(long max_blocks);

inode_t *ulfs_alloc_inode(inode_type_t type, bid_t *pbid_ib, uint16_t *pidx_ib);
void ulfs_free_inode(bid_t bid_ib, inode_t *inode);

inode_t *ulfs_get_inode(bid_t bid_ib, uint16_t idx_ib, uint32_t *pino);
inode_t *ulfs_get_inode_root(void);
inode_t *ulfs_get_inode_cwd(void);
void *ulfs_alloc_dblock(inode_t *inode, lbid_t lbid);
void *ulfs_first_dblock(inode_t *inode, lbid_t lbid, bool_t alloc_ok, dblock_walk_t *pwalk);
void *ulfs_get_dblock(dblock_walk_t *pwalk);
void *ulfs_next_dblock(dblock_walk_t *pwalk);
void ulfs_free_data_blocks(inode_t *inode);

inode_t *ulfs_lookup_name(inode_t *inode_dir, path_t *ppath, dirent_t **pent);
inode_t *ulfs_lookup_path(path_t *ppath, dirent_t **pent);

int ulfs_dir_open(dirlist_t *dlist, path_t *ppath);
dirent_t *ulfs_dir_get(dirlist_t *dlist);

inode_t *ulfs_dir_add_inode(inode_t *inode_dir, path_t *ppath, inode_type_t type, dirent_t **pent, bool_t exist_ok);
bool_t ulfs_dir_del_inode(inode_t *inode_dir, path_t *ppath);

dirent_t *ulfs_dir_get_ent_root(void);
dirent_t *ulfs_dir_get_ent_cwd(void);

ulfd_t *ulfs_get_ulfd(int fd);

void ulfs_path_init(path_t *ppath, const char *path);
void ulfs_path_dirname(path_t *ppath);
void ulfs_path_basename(path_t *ppath);
void ulfs_path_first_name(path_t *ppath);
bool_t ulfs_path_is_empty(path_t *ppath);
bool_t ulfs_path_is_root(path_t *ppath);
bool_t ulfs_path_is_abs(path_t *ppath);
bool_t ulfs_path_next_name(path_t *ppath);
bool_t ulfs_path_matched(path_t *ppath, const char *name);

#endif
