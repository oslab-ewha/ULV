#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#include "libmb.h"

#define TEST_DIR	"/dirtest"

static int	count_entries = 10;
static int	count_loop = 10;

static void
usage(void)
{
	printf(
"mb_file_dir [<dir entry count>] [<loop count>]\n"
"  default: entry count: 10, loop count: 10\n"
		);
}

static void
fillup_dir_entries(void)
{
	char	buf[128];
	int	i;

	for (i = 0; i < count_entries; i++) {
		snprintf(buf, sizeof(buf), "%s/%04d", TEST_DIR, i);
		if (mkdir(buf, 0644) != 0) {
			printf("cannot fillup entries\n");
			return;
		}
	}
}

static void
remove_dir_entries(void)
{
	char	buf[128];
	int	i;

	for (i = 0; i < count_entries; i++) {
		snprintf(buf, sizeof(buf), "%s/%04d", TEST_DIR, i);
		rmdir(buf);
	}
}

static void
read_dirents(const char *path)
{
	DIR	*dir;
	struct dirent	*dirent;

	dir = opendir(path);
	while ((dirent = readdir(dir))) {
#if 0
		printf("ent: %ld %s\n", dirent->d_ino, dirent->d_name);
#endif
	}
	closedir(dir);
}

int
main(int argc, char *argv[])
{
	int	i;

	if (argc > 1)
		count_entries = atoi(argv[1]);
	if (argc > 2)
		count_loop = atoi(argv[2]);

	if (mkdir(TEST_DIR, 0644) != 0) {
		printf("failed to create test directory: %s\n", TEST_DIR);
		return 2;
	}

	fillup_dir_entries();

	init_tickcount();

	for (i = 0; i < count_loop; i++) {
		read_dirents(TEST_DIR);
	}

	printf("elapsed: %d\n", get_tickcount());

	remove_dir_entries();
	rmdir(TEST_DIR);

	return 0;
}
