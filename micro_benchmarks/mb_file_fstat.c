#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "libmb.h"

#define TEST_FILE	"/testfile"

#define BLKSIZE_MAX	16384

static int	count = 1024;

static void
usage(void)
{
	printf(
"mb_file_fstat [<loop count>]\n"
"  loop count default:1024\n"
		);
}

int
main(int argc, char *argv[])
{
	int	fd;
	int	i;
	int	dummy = 0;

	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0) {
			usage();
			return 0;
		}
		count = atoi(argv[1]);
	}

	fd = open(TEST_FILE, O_RDWR | O_CREAT, 0660);
	if (fd < 0) {
		printf("failed to create\n");
		return 1;
	}

	init_tickcount();

	for (i = 0; i < count; i++) {
		struct stat	statbuf;

		if (fstat(fd, &statbuf) < 0) {
			printf("failed to fstat\n");
			return 2;
		}
		dummy += statbuf.st_ino;
	}
	
	printf("elapsed: %d, %d(dummy)\n", get_tickcount(), dummy);

	close(fd);
	unlink(TEST_FILE);

	return 0;
}
