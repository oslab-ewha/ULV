/* inspired from hermitux minifs */

char *strdup(const char *);

#include "ulv_malloc.h"
#include "ulv_syscall_flags.h"
#include "ulv_dyntab.h"
#include "ulv_assert.h"
#include "ulfs_p.h"

typedef struct {
        char	*name;
        uint64_t	size;
	int	refcnt;
	unsigned	*blocks;
} lwfile_t;

typedef struct {
        off_t	off;
	lwfile_t	*file;
} lwfd_t;

static ulv_dyntab_t	lwfiles;
static ulv_dyntab_t	lwfds;

extern void ulfs_sb_init(void);

static lwfile_t *
create_internal(const char *pathname, int mode)
{
	lwfile_t	*lwfile;

	lwfile = (lwfile_t *)ulv_dyntab_assign(&lwfiles);
	lwfile->name = strdup(pathname);
	lwfile->size = 0;
	lwfile->refcnt = 0;

	/* TODO: initialize blocks */

	return lwfile;
}

static lwfile_t *
find_file(const char *pathname)
{
	return NULL;
}

int
ulfs_open(const char *pathname, int flags, int mode)
{
	lwfile_t	*lwfile;
	lwfd_t		*lwfd;

	if (flags & O_CREAT) {
		lwfile = create_internal(pathname, mode);
                ULV_ASSERT(lwfile != NULL);
	} else {
		lwfile = find_file(pathname);
		if (lwfile == NULL)
			return -1;
	}

	lwfile->refcnt++;

	lwfd = (lwfd_t *)ulv_dyntab_assign(&lwfds);
	lwfd->off = 0;
	lwfd->file = lwfile;

	return ULV_DYNTAB_ENTRY_IDX(lwfd);
}

void
ulfs_close(int fd)
{
	lwfd_t	*lwfd;

	lwfd = ulv_dyntab_get(&lwfds, fd);
	if (lwfd) {
		lwfd->file->refcnt--;
		ulv_dyntab_release(&lwfds, lwfd);
	}
}

void
ulfs_init(void)
{
	ulfs_block_init();
	ulfs_sb_init();
	ulv_dyntab_init(&lwfiles, sizeof(lwfile_t), 8);
	ulv_dyntab_init(&lwfds, sizeof(lwfd_t), 16);
}
