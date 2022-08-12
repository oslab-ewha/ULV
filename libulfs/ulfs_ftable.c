#include "ulfs_p.h"

#define N_FES_PER_BLOCK	(BSIZE / sizeof(fe_t))
#define AVAIL_PER_PATH_BLOCK(pb)	(BSIZE - 8 - (pb)->used)

#define GET_LINK_FE(fe)	((fe_t *)(fe) + N_FES_PER_BLOCK - 1)
#define IS_FE_EMPTY(fe)	((fe)->path_off == 0 && (fe)->bid_path == 0)

static fe_t *
get_next_fe(fe_t *fe)
{
	fe_t	*fe_link;

	fe_link = GET_LINK_FE(fe);
	if (fe_link->bid_path == 0) {
		fe_link->bid_path = ulfs_block_alloc();
	}
	return (fe_t *)ulfs_block_get(fe_link->bid_path);
}

static inline fe_t *
get_empty_fe(fe_t *fe)
{
	int	i;

	for (i = 0; i < N_FES_PER_BLOCK - 1; i++, fe++) {
		if (IS_FE_EMPTY(fe))
			return fe;
	}
	return NULL;
}

static fe_t *
alloc_fe(void)
{
	fe_t	*fe_first;
	fe_t	*fe;

	fe_first = (fe_t *)ulfs_block_get(2);
	while (1) {
		fe = get_empty_fe(fe_first);
		if (fe != NULL)
			break;
		fe_first = get_next_fe(fe_first);
	}

	return fe;
}

static void
copy_path(const char *path, char *path_in_pb)
{
	const char	*p;
	char	*q;

	for (p = path, q = path_in_pb; *p; p++, q++)
		*q = *p;
	*q = '\0';
}

static uint16_t
get_pathlen(const char *path)
{
	const char	*p;

	for (p = path; *p; p++);
	return (uint16_t)(p - path + 1);
}

static bid_t
get_path_bid_off(const char *path, uint16_t *poff)
{
	uint16_t	pathlen;
	bid_t	bid_pb = 3;

	pathlen = get_pathlen(path);

	while (1) {
		path_block_t	*pb;

		pb = (path_block_t *)ulfs_block_get(bid_pb);
		if (AVAIL_PER_PATH_BLOCK(pb) >= pathlen) {
			*poff = 8 + pb->used;
			copy_path(path, pb->data + pb->used);
			pb->used += pathlen;
			return bid_pb;
		}
		if (pb->bid_next == 0)
			pb->bid_next = ulfs_block_alloc();
		bid_pb = pb->bid_next;
	}
}

fe_t *
ulfs_alloc_fe(const char *path)
{
	fe_t	*fe;

	fe = alloc_fe();
	fe->bid_path = get_path_bid_off(path, &fe->path_off);

	return fe;
}
