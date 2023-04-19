#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

#include "libmb.h"

static int	count = 10;

static void
usage(void)
{
	printf(
"mb_file_dir <file path> [<count>]\n"
"  default count: 1024\n"
		);
}

static void
read_dirents(const char *path)
{
	DIR	*dir;
	struct dirent	*dirent;

	dir = opendir(path);
	while ((dirent = readdir(dir)))
		printf("ent: %ld %s\n", dirent->d_ino, dirent->d_name);
	closedir(dir);
}

int
main(int argc, char *argv[])
{
	DIR	*dir;
	int	i;

	if (argc < 2) {
		usage();
		return 1;
	}

	if (argc > 2)
		count = atoi(argv[2]);

	dir = opendir(argv[1]);
	if (dir == NULL) {
		printf("failed to open: %s\n", argv[1]);
		return 2;
	}
	closedir(dir);

	init_tickcount();

	for (i = 0; i < count; i++) {
		read_dirents(argv[1]);
	}

	printf("elapsed: %d\n", get_tickcount());

	return 0;
}
