#include "ulv_malloc.h"
#include "ulv_assert.h"
#include "ulv_dyntab.h"
#include "ulv_fd_table.h"

#define FDINFO_ALLOC_CHUNK	32

typedef struct {
	fdtype_t	type;
	int	fd_real;
} fdinfo_t;

static ulv_dyntab_t	fd_table;

int
ulv_assign_fd(fdtype_t type, int fd_real)
{
	fdinfo_t	*fdinfo;

	fdinfo = ulv_dyntab_assign(&fd_table);
	fdinfo->type = type;
	fdinfo->fd_real = fd_real;

	return ULV_DYNTAB_ENTRY_IDX(fdinfo);
}

void
ulv_release_fd(int fd)
{
	fdinfo_t	*fdinfo;

	fdinfo = (fdinfo_t *)ulv_dyntab_get(&fd_table, fd);
	if (fdinfo)
		ulv_dyntab_release(&fd_table, fdinfo);
}

int
ulv_lookup_fd_real(int fd, fdtype_t *ptype)
{
	fdinfo_t	*fdinfo;

	fdinfo = (fdinfo_t *)ulv_dyntab_get(&fd_table, fd);
	if (fdinfo == NULL)
		return -1;
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

	ulv_dyntab_init(&fd_table, sizeof(fdinfo_t), FDINFO_ALLOC_CHUNK);

	/* For standard I/O's */
	for (i = 0; i < 3; i++) {
		fdinfo_t	*fdinfo = (fdinfo_t *)ulv_dyntab_assign(&fd_table);

		fdinfo->type = FDTYPE_HOST;
		fdinfo->fd_real = i;
	}
}
