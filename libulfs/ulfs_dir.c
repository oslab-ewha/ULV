#include "ulfs_p.h"
#include "ulv_assert.h"

static inode_t	*inode_cwd;
static inode_t	*inode_root;

static inline void
check_inode_root(void)
{
	inode_block_t	*ib;

	if (inode_root != NULL)
		return;
	ib = ulfs_block_get(2);
	inode_root = ib->inodes;
}

static inline void
check_inode_cwd(void)
{
	if (inode_cwd != NULL)
		return;
	check_inode_root();
	inode_cwd = inode_root;
}

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

static bool_t
is_name_matched(const char *name1, const char *name2, unsigned namelen)
{
	unsigned	i;

	for (i = 0; i < namelen; i++) {
		if (name1[i] != name2[i])
			return FALSE;
	}
	if (name1[namelen] != '\0')
		return FALSE;
	return TRUE;
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

static inode_t *
lookup_name_with_len(inode_t *inode_dir, const char *name, unsigned namelen)
{
	dirlist_t	dirlist;
	dirent_t	*ent;

	init_dirlist(&dirlist, inode_dir);

	while ((ent = ulfs_dir_get(&dirlist))) {
		if (is_name_matched(ent->name, name, namelen)) {
			inode_block_t	*ib = (inode_block_t *)ulfs_block_get(ent->bid_ib);
			return ib->inodes + ent->idx_ib;
		}
	}

	return NULL;
}

static unsigned
get_name_len(const char *name)
{
	const char	*p;

	for (p = name; *p; p++);
	return p - name;
}

static inode_t *
lookup_name(inode_t *inode_dir, const char *name)
{
	return lookup_name_with_len(inode_dir, name, get_name_len(name));
}

static const char *
get_name_end(const char *path)
{
	const char	*p;

	for (p = path; *p && *p != '/'; p++);
	return p;
}

static inode_t *
do_lookup(inode_t *inode_dir, const char *path)
{
	inode_t	*found;
	const char	*p;
	unsigned	namelen;

again:
	p = get_name_end(path);
	namelen = p - path;

	if ((found = lookup_name_with_len(inode_dir, path, namelen))) {
		if (*p == '\0')
			return found;
		if (found->type != INODE_TYPE_DIR)
			return NULL;
		path = p + 1;
		inode_dir = found;
		goto again;
	}
	return NULL;
}

inode_t *
ulfs_lookup_path(const char *path)
{
	if (*path == '/') {
		check_inode_root();
		if (path[1] == '\0')
			return inode_root;
		return do_lookup(inode_root, path + 1);
	}
	else {
		check_inode_cwd();
		return do_lookup(inode_cwd, path);
	}
}

int
ulfs_dir_open(dirlist_t *dlist, const char *path)
{
	inode_t	*inode;

	inode = ulfs_lookup_path(path);
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
set_entry_name(dirent_t *ent, const char *name)
{
	char	*p;
	const char	*q;

	for (p = ent->name, q = name; *q; p++, q++)
		*p = *q;
	*p = '\0';
}

inode_t *
ulfs_dir_add_inode(inode_t *inode_dir, const char *name, inode_type_t type)
{
	dirent_t	*ent;
	bid_t		bid_ib;
	uint16_t	idx_ib;
	inode_t		*inode;

	if (lookup_name(inode_dir, name) != NULL)
		return NULL;

	ent = find_empty_entry(inode_dir);
	inode = ulfs_alloc_inode(type, &bid_ib, &idx_ib);
	set_entry_name(ent, name);
	ent->bid_ib = bid_ib;
	ent->idx_ib = idx_ib;

	return inode;
}
