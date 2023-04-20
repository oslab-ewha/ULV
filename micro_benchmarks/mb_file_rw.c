#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "libmb.h"

#define BLKSIZE_MAX	16384

static int	count = 1024;
static int	blksize = 1024;

static void
usage(void)
{
	printf(
"mb_file_rw <file path> [<count>] [<block size>]\n"
"  default count: 1024\n"
"  default block size: 1024, max: 16384\n"
		);
}

static void
write_file(int fd)
{
	char	buf[BLKSIZE_MAX];
	int	i;

	for (i = 0; i < blksize; i++) {
		buf[i] = (char)i;
	}
	for (i = 0; i < count; i++) {
		int	ret;

		ret = write(fd, buf, blksize);
		if (ret != blksize) {
			printf("write error\n");
			exit(1);
		}
	}
}

static void
read_file(int fd)
{
	char	buf[BLKSIZE_MAX];
	int	i;

	for (i = 0; i < count; i++) {
		int	ret;

		ret = read(fd, buf, blksize);
		if (ret != blksize) {
			printf("read error\n");
			exit(1);
		}
	}
	for (i = 0; i < blksize; i++) {
		if (buf[i] != (char)i) {
			printf("read check failed\n");
			exit(1);
		}
	}
}

int
main(int argc, char *argv[])
{
	int	nread;
	int	fd;

	if (argc < 2) {
		usage();
		return 1;
	}

	if (argc > 2)
		count = atoi(argv[2]);
	if (argc > 3) {
		blksize = atoi(argv[3]);
		if (blksize <= 0 || blksize > 16384) {
			printf("invalid blksize\n");
			return 1;
		}
	}

	init_tickcount();

	fd = open(argv[1], O_RDWR | O_CREAT, 0660);
	if (fd < 0) {
		printf("failed to create\n");
		return 1;
	}

	write_file(fd);
	close(fd);

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		printf("failed to open\n");
		return 1;
	}
	read_file(fd);
	close(fd);

	printf("elapsed: %d\n", get_tickcount());

	unlink(argv[1]);

	return 0;
}
