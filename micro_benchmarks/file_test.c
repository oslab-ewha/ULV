#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "libmb.h"

static void
usage(void)
{
	printf("mb_file <file path>\n");
}

static void
write_file(int fd)
{
	char	buf[1024];
	int	i;

	for (i = 0; i < 1024; i++) {
		buf[i] = (char)i;
	}
	for (i = 0; i < 1024; i++) {
		int	ret;

		ret = write(fd, buf, 1024);
		if (ret != 1024) {
			printf("write error\n");
			exit(1);
		}
	}
}

static void
read_file(int fd)
{
	char	buf[1024];
	int	i;

	for (i = 0; i < 1024; i++) {
		int	ret;

		ret = read(fd, buf, 1024);
		if (ret != 1024) {
			printf("read error\n");
			exit(1);
		}
	}
	for (i = 0; i < 1024; i++) {
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

	return 0;
}
