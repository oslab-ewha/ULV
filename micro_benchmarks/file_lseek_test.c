#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include "libmb.h"

#define TEST_FILE	"/testfile"

#define BLKSIZE_MAX	16384

static int	count = 1024;

static void
usage(void)
{
	printf(
"mb_file_lseek [<loop count>]\n"
"  default loop count: 1024\n"
		);
}

int
main(int argc, char *argv[])
{
	int	fd;
	int	i;

	if (argc > 1)
		count = atoi(argv[1]);

	fd = open(TEST_FILE, O_RDWR | O_CREAT, 0660);
	if (fd < 0) {
		printf("failed to create\n");
		return 1;
	}

	init_tickcount();

	for (i = 0; i < count; i++)
		if (lseek(fd, 0, SEEK_SET) < 0) {
			printf("failed to lseek\n");
			return 2;
		}
	
	printf("elapsed: %d\n", get_tickcount());

	close(fd);
	unlink(TEST_FILE);

	return 0;
}
