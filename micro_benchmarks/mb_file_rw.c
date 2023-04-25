#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "libmb.h"

#define TEST_RWFILE	"/testfile"
#define BLKSIZE_MAX	16384

static int	count_rw = 1024;
static int	count_loop = 1;
static int	blksize = 1024;

static void
usage(void)
{
	printf(
"mb_file_rw [<block size>] [<count rw>] [<count loop>]\n"
"  default block size: 1024, max: 16384\n"
"  default count rw: 1024\n"
"  default count loop: 1\n"
		);
}

static void
write_file(int fd)
{
	char	buf[BLKSIZE_MAX];
	int	i, j;

	for (i = 0; i < blksize; i++) {
		buf[i] = (char)i;
	}

	for (j = 0; j < count_loop; j++) {
		for (i = 0; i < count_rw; i++) {
			int	ret;

			ret = write(fd, buf, blksize);
			if (ret != blksize) {
				printf("write error\n");
				return;
			}
		}
		if (j < count_loop - 1)
			lseek(fd, 0, SEEK_SET);
	}
}

static void
read_file(int fd)
{
	char	buf[BLKSIZE_MAX];
	int	i, j;

	for (j = 0; j < count_loop; j++) {
		for (i = 0; i < count_rw; i++) {
			int	ret;

			ret = read(fd, buf, blksize);
			if (ret != blksize) {
				printf("read error\n");
				return;
			}
		}
		if (j < count_loop - 1)
			lseek(fd, 0, SEEK_SET);
	}
	for (i = 0; i < blksize; i++) {
		if (buf[i] != (char)i) {
			printf("read check failed\n");
			return;
		}
	}
}

int
main(int argc, char *argv[])
{
	int	fd;

	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0) {
			usage();
			return 0;
		}
		blksize = atoi(argv[1]);
		if (blksize <= 0 || blksize > 16384) {
			printf("invalid blksize\n");
			return 1;
		}
	}
	if (argc > 2)
		count_rw = atoi(argv[2]);
	if (argc > 3)
		count_loop = atoi(argv[3]);

	init_tickcount();

	fd = open(TEST_RWFILE, O_RDWR | O_CREAT, 0660);
	if (fd < 0) {
		printf("failed to create\n");
		return 1;
	}

	write_file(fd);
	close(fd);

	fd = open(TEST_RWFILE, O_RDONLY);
	if (fd < 0) {
		printf("failed to open\n");
		return 1;
	}
	read_file(fd);
	close(fd);

	printf("elapsed: %d\n", get_tickcount());

	unlink(TEST_RWFILE);

	return 0;
}
