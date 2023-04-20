#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int ulfs_tool_mkfs(int argc, char *argv[]);
int ulfs_tool_ls(int argc, char *argv[]);
int ulfs_tool_rm(int argc, char *argv[]);
int ulfs_tool_mkdir(int argc, char *argv[]);
int ulfs_tool_rmdir(int argc, char *argv[]);
int ulfs_tool_copyto(int argc, char *argv[]);
int ulfs_tool_info(int argc, char *argv[]);

static void
usage(void)
{
	printf(
"ulfs <command> [<command options>]\n"
" command:\n"
"   mkfs <path> <size>\n"
"   ls [-si] [<path>]\n"
"     -s: list with size, -i: list with inode number\n"
"   rm <path>\n"
"   mkdir/rmdir <path>\n"
"   copyto <host path> [<ulfs path>]\n"
"   info\n"
		);
}

void
error(const char *fmt, ...)
{
	char	buf[256];
	va_list	ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	fprintf(stderr, "Error: %s\n", buf);
}

long long
get_fd_size(int fd)
{
	struct stat	statb;

	if (fstat(fd, &statb) < 0)
		return -1;
	return statb.st_size;
}

int
openr(const char *path)
{
	return open(path, O_RDONLY);
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		usage();
		return 0;
	}
	
	if (strcmp(argv[1], "mkfs") == 0) {
		return ulfs_tool_mkfs(argc - 2, argv + 2);
	}
	else if (strcmp(argv[1], "ls") == 0) {
		return ulfs_tool_ls(argc - 2, argv + 2);
	}
	else if (strcmp(argv[1], "rm") == 0) {
		return ulfs_tool_rm(argc - 2, argv + 2);
	}
	else if (strcmp(argv[1], "mkdir") == 0) {
		return ulfs_tool_mkdir(argc - 2, argv + 2);
	}
	else if (strcmp(argv[1], "rmdir") == 0) {
		return ulfs_tool_rmdir(argc - 2, argv + 2);
	}
	else if (strcmp(argv[1], "copyto") == 0) {
		return ulfs_tool_copyto(argc - 2, argv + 2);
	}
	else if (strcmp(argv[1], "info") == 0) {
		return ulfs_tool_info(argc - 2, argv + 2);
	}
	usage();
	return 1;
}
