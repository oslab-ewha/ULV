#include "ulfs_p.h"
#include "ulv_assert.h"

dirent_t *
ulfs_dir_get(dirlist_t *dlist)
{
	dirent_t	*ent;

again:
	ent = dlist->ent;
	if (ent == NULL)
		return NULL;

	dlist->size_remain -= sizeof(dirent_t);

	if (dlist->size_remain == 0) {
		dlist->ent_last = dlist->ent;
		dlist->ent = NULL;
	}
	else {
		dlist->ent++;
		if (dlist->ent - dlist->head > N_DIRENT_PER_DB) {
			if (dlist->bb == NULL) {
				ULV_ASSERT(dlist->inode->bids_data[1] != 0);
				dlist->bb = (bidblock_t *)ulfs_block_get(dlist->inode->bids_data[1]);
				dlist->idx_bb = 0;
			}
			else {
				dlist->idx_bb++;
				if (dlist->idx_bb == N_BIDS_PER_BB) {
					dlist->bb = (bidblock_t *)ulfs_block_get(dlist->bb->next);
					dlist->idx_bb = 0;
				}
			}
			dlist->ent = dlist->head = (dirent_t *)ulfs_block_get(dlist->bb->bids[dlist->idx_bb]);
		}
	}

	if (*ent->name == '\0')
		goto again;
	return ent;
}

static void
init_dirlist(dirlist_t *dlist, inode_t *inode_dir)
{
	dlist->inode = inode_dir;
	dlist->size_remain = inode_dir->size;
	dlist->bb = NULL;
	dlist->idx_bb = 0;
	dlist->head = dlist->ent = ulfs_block_get(dlist->inode->bids_data[0]);
}

inode_t *
ulfs_lookup_name(inode_t *inode_dir, path_t *ppath)
{
	dirlist_t	dirlist;
	dirent_t	*ent;

	init_dirlist(&dirlist, inode_dir);

	while ((ent = ulfs_dir_get(&dirlist))) {
		if (ulfs_path_matched(ppath, ent->name)) {
			inode_block_t	*ib = (inode_block_t *)ulfs_block_get(ent->bid_ib);
			return ib->inodes + ent->idx_ib;
		}
	}

	return NULL;
}

static inode_t *
do_lookup(inode_t *inode_dir, path_t *ppath)
{
	inode_t	*found;

	ulfs_path_first_name(ppath);

	while ((found = ulfs_lookup_name(inode_dir, ppath))) {
		if (!ulfs_path_next_name(ppath))
			return found;
		if (found->type != INODE_TYPE_DIR)
			return NULL;
		inode_dir = found;
	}
	return NULL;
}

inode_t *
ulfs_lookup_path(path_t *ppath)
{
	if (ulfs_path_is_empty(ppath))
		return ulfs_get_inode_cwd();
	if (ulfs_path_is_root(ppath))
		return ulfs_get_inode_root();
	if (ulfs_path_is_abs(ppath)) {
		path_t	path_from_root;

		path_from_root = *ppath;
		path_from_root.start++;
		return do_lookup(ulfs_get_inode_root(), &path_from_root);
	}
	else
		return do_lookup(ulfs_get_inode_cwd(), ppath);
}

int
ulfs_dir_open(dirlist_t *dlist, path_t *ppath)
{
	inode_t	*inode;

	inode = ulfs_lookup_path(ppath);
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
		if (dirlist.ent_last - dirlist.head < N_DIRENT_PER_DB) {
			ent = dirlist.ent_last + 1;
		}
		else {
			if (dirlist.bb == NULL) {
				dirlist.inode->bids_data[1] = ulfs_block_alloc();
				dirlist.bb = (bidblock_t *)ulfs_block_get(dirlist.inode->bids_data[1]);
			}
			else
				dirlist.idx_bb++;

			if (dirlist.idx_bb == N_BIDS_PER_BB - 1) {
				if (dirlist.bb->next == 0)
					dirlist.bb->next = ulfs_block_alloc();
				dirlist.bb = (bidblock_t *)ulfs_block_get(dirlist.bb->next);
				dirlist.idx_bb = 0;
			}
			ent = (dirent_t *)ulfs_block_get(dirlist.bb->bids[dirlist.idx_bb]);
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
ulfs_dir_add_inode(inode_t *inode_dir, path_t *ppath, inode_type_t type)
{
	dirent_t	*ent;
	bid_t		bid_ib;
	uint16_t	idx_ib;
	inode_t		*inode;

	if (ulfs_lookup_name(inode_dir, ppath) != NULL)
		return NULL;

	ent = find_empty_entry(inode_dir);
	inode = ulfs_alloc_inode(type, &bid_ib, &idx_ib);
	set_entry_name(ent, ppath);
	ent->bid_ib = bid_ib;
	ent->idx_ib = idx_ib;

	return inode;
}
