#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <libgen.h>
#include <unistd.h>

#include "ulfs_tool.h"

#define ULFS_USE_GLIBC
#include <inttypes.h>
#include "ulfs_p.h"

static void
do_copy_data(fe_t *fe, int fd)
{
	uint64_t	size_copy = fe->size;
	unsigned	i;

	for (i = 0; size_copy > 0; i++) {
		bid_t	bid = ulfs_alloc_data_block(fe, (bid_t)i);
		char	*data;
		ssize_t	nread;

		data = ulfs_block_get(bid);
		if (size_copy >= BSIZE) {
			nread = read(fd, data, BSIZE);
			size_copy -= BSIZE;
		}
		else {
			nread = read(fd, data, size_copy);
			size_copy = 0;
		}
		if (nread < 0)
			error("failed to read");
	}
}

static int
do_copyto(int fd, const char *path)
{
	fe_t	*fe;
	struct stat	statb;

	if (fstat(fd, &statb) < 0) {
		error("failed to get file size");
		return 3;
	}

	fe = ulfs_alloc_fe(path);
	fe->size = statb.st_size;

	do_copy_data(fe, fd);

	return 0;
}

int
ulfs_tool_copyto(int argc, char *argv[])
{
	const char	*path;
	int	fd, ret;

	if (argc < 1) {
		error("insufficient argument");
		return 2;
	}

	fd = open(argv[0], O_RDONLY);
	if (fd < 0) {
		error("cannot open: %s", argv[0]);
		return 2;
	}

	ulfs_block_init();

	if (argc > 1) {
		path = argv[1];
	}
	else {
		path = basename(argv[1]);
	}

	ret = do_copyto(fd, path);
	close(fd);

	return ret;
}
