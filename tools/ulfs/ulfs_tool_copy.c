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
do_copy_data(inode_t *inode, int fd)
{
	uint64_t	size_copy = inode->size;
	unsigned	i;

	for (i = 0; size_copy > 0; i++) {
		bid_t	bid = ulfs_alloc_data_block(inode, (bid_t)i);
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
do_copyto(int fd, const char *_path)
{
	char	*path, *path_dir;
	inode_t	*inode, *inode_dir;
	struct stat	statb;

	if (fstat(fd, &statb) < 0) {
		error("failed to get file size");
		return 3;
	}

	path = strdup(_path);
	path_dir = dirname(path);
	if (strcmp(path_dir, ".") == 0) {
		inode_dir = ulfs_lookup_path("/");
	}
	else {
		inode_dir = ulfs_lookup_path(path_dir);
	}

	if (inode_dir == NULL) {
		error("path does not exist");
		return 4;
	}
	if (inode_dir->type != INODE_TYPE_DIR) {
		error("not directory");
		return 4;
	}

	inode = ulfs_dir_add_inode(inode_dir, basename(path), INODE_TYPE_FILE);
	if (inode == NULL) {
		error("duplicate entry or unknown error");
		return 4;
	}

	inode->size = statb.st_size;

	do_copy_data(inode, fd);

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
		path = basename(argv[0]);
	}

	ret = do_copyto(fd, path);
	close(fd);

	return ret;
}
