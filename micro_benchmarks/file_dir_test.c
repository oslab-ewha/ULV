#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#include "libmb.h"

static int	count = 1024;

static void
usage(void)
{
	printf(
"mb_file_dir <file path> [<count>]\n"
"  default count: 1024\n"
		);
}

static void
read_dirents(DIR *dir)
{
	int	i;

	for (i = 0; i < count; i++) {
		struct dirent	*dirent;
		while ((dirent = readdir(dir)))
			printf("ent: %ld %s\n", dirent->d_ino, dirent->d_name);
	}
}

int
main(int argc, char *argv[])
{
	DIR	*dir;

	if (argc < 2) {
		usage();
		return 1;
	}

	if (argc > 2)
		count = atoi(argv[2]);

	init_tickcount();

	dir = opendir(argv[1]);
	if (dir == NULL) {
		printf("failed to open: %s\n", argv[1]);
		return 2;
	}

	read_dirents(dir);

	closedir(dir);

	printf("elapsed: %d\n", get_tickcount());

	return 0;
}
