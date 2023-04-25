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
static int	whence = SEEK_SET;

static void
usage(void)
{
	printf(
"mb_file_lseek [<type>] [<loop count>]\n"
"  type: set(default), end, cur\n"
"  loop count default:1024\n"
		);
}

static void
parse_seek_type(const char *typestr)
{
	if (strcmp(typestr, "set") == 0)
		whence = SEEK_SET;
	else if (strcmp(typestr, "end") == 0)
		whence = SEEK_END;
	else if (strcmp(typestr, "cur") == 0)
		whence = SEEK_CUR;
	else
		printf("warning: no such seek type: %s\n", typestr);
}

int
main(int argc, char *argv[])
{
	int	fd;
	int	i;

	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0) {
			usage();
			return 0;
		}
		parse_seek_type(argv[1]);
	}
	if (argc > 2)
		count = atoi(argv[2]);

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
