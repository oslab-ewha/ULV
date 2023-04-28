#include "ulfs_p.h"
#include "ulv_assert.h"

static dirent_t	*ent_root;
static dirent_t	*ent_cwd;

dirent_t *
ulfs_dir_get(dirlist_t *dlist)
{
	dirent_t	*ent;

again:
	ent = dlist->ent;
	if (ent == NULL)
		return NULL;

	dlist->idx_in_block++;
	dlist->off += sizeof(dirent_t);

	if (dlist->off == dlist->inode->size)
		dlist->ent = NULL;
	else {
		dlist->ent++;
		if (dlist->idx_in_block == N_DIRENT_PER_DB) {
			dlist->ent = (dirent_t *)ulfs_next_dblock(&dlist->walk);
			dlist->idx_in_block = 0;
			dlist->off += BSIZE - N_DIRENT_PER_DB * sizeof(dirent_t);
		}
	}

	/* It's empty dirent */
	if (*ent->name == '\0')
		goto again;
	return ent;
}

static void
init_dirlist(dirlist_t *dlist, inode_t *inode_dir)
{
	dlist->inode = inode_dir;
	dlist->idx_in_block = 0;
	dlist->off = 0;
	dlist->ent = (dirent_t *)ulfs_first_dblock(inode_dir, 0, FALSE, &dlist->walk);
}

inode_t *
ulfs_lookup_name(inode_t *inode_dir, path_t *ppath, dirent_t **pent)
{
	dirlist_t	dirlist;
	dirent_t	*ent;

	init_dirlist(&dirlist, inode_dir);

	while ((ent = ulfs_dir_get(&dirlist))) {
		if (ulfs_path_matched(ppath, ent->name)) {
			inode_block_t	*ib = (inode_block_t *)ulfs_block_get(ent->bid_ib);
			if (pent)
				*pent = ent;
			return ib->inodes + ent->idx_ib;
		}
	}

	return NULL;
}

static inode_t *
do_lookup(inode_t *inode_dir, path_t *ppath, dirent_t **pent)
{
	inode_t	*found;

	ulfs_path_first_name(ppath);

	while ((found = ulfs_lookup_name(inode_dir, ppath, pent))) {
		if (!ulfs_path_next_name(ppath))
			return found;
		if (found->type != INODE_TYPE_DIR)
			return NULL;
		inode_dir = found;
	}
	return NULL;
}

inode_t *
ulfs_lookup_path(path_t *ppath, dirent_t **pent)
{
	if (ulfs_path_is_empty(ppath)) {
		if (pent)
			*pent = ulfs_dir_get_ent_cwd();
		return ulfs_get_inode_cwd();
	}
	if (ulfs_path_is_root(ppath)) {
		if (pent)
			*pent = ulfs_dir_get_ent_root();
		return ulfs_get_inode_root();
	}
	if (ulfs_path_is_abs(ppath)) {
		path_t	path_from_root;

		path_from_root = *ppath;
		path_from_root.start++;
		return do_lookup(ulfs_get_inode_root(), &path_from_root, pent);
	}
	else
		return do_lookup(ulfs_get_inode_cwd(), ppath, pent);
}

int
ulfs_dir_open(dirlist_t *dlist, path_t *ppath)
{
	inode_t	*inode;

	inode = ulfs_lookup_path(ppath, NULL);
	if (inode == NULL)
		return -1;
	init_dirlist(dlist, inode);
	return 0;
}

static dirent_t *
find_empty_entry(inode_t *inode_dir)
{
	dirlist_t	dirlist;
	dirent_t	*ent;

	init_dirlist(&dirlist, inode_dir);

	while ((ent = ulfs_dir_get(&dirlist))) {
		if (*ent->name == '\0')
			break;
	}
	if (ent == NULL) {
		if (dirlist.idx_in_block < N_DIRENT_PER_DB) {
			ent = (dirent_t *)ulfs_get_dblock(&dirlist.walk) + dirlist.idx_in_block;
		}
		else {
			dirlist.walk.alloc_ok = TRUE;
			ent = (dirent_t *)ulfs_next_dblock(&dirlist.walk);
			inode_dir->size += (BSIZE - N_DIRENT_PER_DB * sizeof(dirent_t));
		}

		inode_dir->size += sizeof(dirent_t);
	}

	return ent;
}

static void
set_entry_name(dirent_t *ent, path_t *ppath)
{
	char	*p;
	const char	*q;

	for (p = ent->name, q = ppath->start; q < ppath->end; p++, q++)
		*p = *q;
	*p = '\0';
}

inode_t *
ulfs_dir_add_inode(inode_t *inode_dir, path_t *ppath, inode_type_t type, dirent_t **pent, bool_t exist_ok)
{
	dirent_t	*ent;
	bid_t		bid_ib;
	uint16_t	idx_ib;
	inode_t		*inode;

	if ((inode = ulfs_lookup_name(inode_dir, ppath, NULL)) != NULL) {
		if (exist_ok)
			return inode;
		return NULL;
	}

	ent = find_empty_entry(inode_dir);
	inode = ulfs_alloc_inode(type, &bid_ib, &idx_ib);
	set_entry_name(ent, ppath);
	ent->bid_ib = bid_ib;
	ent->idx_ib = idx_ib;
	if (pent)
		*pent = ent;

	return inode;
}

bool_t
ulfs_dir_del_inode(inode_t *inode_dir, path_t *ppath)
{
	dirent_t	*ent;
	inode_t		*inode;

	if ((inode = ulfs_lookup_name(inode_dir, ppath, &ent)) == NULL)
		return FALSE;

	*ent->name = '\0';
	ulfs_free_data_blocks(inode);

	ulfs_free_inode(ent->bid_ib, inode);

	return TRUE;
}

inline dirent_t *
ulfs_dir_get_ent_root(void)
{
	if (ent_root == NULL)
		ent_root = (dirent_t *)ulfs_block_get(BID_ROOTB_START);
	return ent_root;
}

inline dirent_t *
ulfs_dir_get_ent_cwd(void)
{
	if (ent_cwd == NULL)
		ent_cwd = ulfs_dir_get_ent_root();
	return ent_cwd;
}
