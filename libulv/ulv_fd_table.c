#include "ulv_malloc.h"
#include "ulv_assert.h"
#include "ulv_fd_table.h"

#define MAX_FDS		100
#define FDINFO_ALLOC_CHUNK	32

typedef struct {
	fdtype_t	type;
	int	fd_real;
} fdinfo_t;

static int	fd_tables[MAX_FDS];

static fdinfo_t	*fdinfos;

static int	fd_avail;
static int	fdinfos_n_alloced;
static int	fdinfo_avail;

static int
find_next_fd_avail(void)
{
	int	i;

	for (i = fd_avail + 1; i < MAX_FDS; i++) {
		if (fd_tables[i] == 0)
			return i;
	}

	return i;
}

static int
find_next_fdinfo_avail(void)
{
	int	i;

	for (i = fdinfo_avail + 1; i < fdinfos_n_alloced; i++) {
		if (fdinfos[i].type == 0)
			return i;
	}

	return i;
}

int
ulv_assign_fd(fdtype_t type, int fd_real)
{
	fdinfo_t	*fdinfo;
	int	fd;

	if (fd_avail >= MAX_FDS)
		ULV_PANIC("exceed max fd");
	fd = fd_avail;
	fd_avail = find_next_fd_avail();
	if (fdinfo_avail == fdinfos_n_alloced - 1) {
		int	i;

		fdinfos_n_alloced += FDINFO_ALLOC_CHUNK;
		fdinfos = (fdinfo_t *)ulv_realloc(fdinfos, sizeof(fdinfo_t) * fdinfos_n_alloced);
		for (i = fdinfos_n_alloced - FDINFO_ALLOC_CHUNK; i < fdinfos_n_alloced; i++)
			fdinfos[i].type = 0;
	}

	fd_tables[fd] = fdinfo_avail + 1;
	fdinfo = fdinfos + fdinfo_avail;
	fdinfo->type = type;
	fdinfo->fd_real = fd_real;
	fdinfo_avail = find_next_fdinfo_avail();

	return fd;
}

void
ulv_release_fd(int fd)
{
	int	fdinfo_idx;

	fdinfo_idx = fd_tables[fd];
	fdinfos[fdinfo_idx - 1].type = 0;
	if (fdinfo_idx - 1 < fdinfo_avail)
		fdinfo_avail = fdinfo_idx - 1;
	fd_tables[fd] = 0;
	if (fd < fd_avail)
		fd_avail = fd;
}

int
ulv_lookup_fd_real(int fd, fdtype_t *ptype)
{
	fdinfo_t	*fdinfo;
	int	fdinfo_idx;

	if (fd < 0 || fd >= MAX_FDS)
		return -1;
	fdinfo_idx = fd_tables[fd];
	if (fdinfo_idx == 0)
		return -1;

	fdinfo = &fdinfos[fdinfo_idx - 1];

	*ptype = fdinfo->type;
	return fdinfo->fd_real;
}

bool_t
ulv_convert_fd_real(int *pfd, fdtype_t type)
{
	int	fd_real;
	fdtype_t	type_lookup;

	fd_real = ulv_lookup_fd_real(*pfd, &type_lookup);
	if (fd_real < 0)
		return FALSE;
	if (type != type_lookup)
		return FALSE;
	*pfd = fd_real;
	return TRUE;
}

void
ulv_fd_table_init(void)
{
	int	i;

	fdinfos_n_alloced = FDINFO_ALLOC_CHUNK;
	fdinfos = (fdinfo_t *)ulv_malloc(sizeof(fdinfo_t) * fdinfos_n_alloced);
	fdinfo_avail = 3;

	/* For standard I/O's */
	for (i = 0; i < 3; i++) {
		fdinfos[i].type = FDTYPE_HOST;
		fdinfos[i].fd_real = i;
		fd_tables[i] = i + 1;
	}
	for (i = 3; i < FDINFO_ALLOC_CHUNK; i++)
		fdinfos[i].type = 0;
	fd_avail = 3;
	fdinfo_avail = 3;
	
}
