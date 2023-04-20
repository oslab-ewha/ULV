#include "ulv_types.h"
#include "ulv_test.h"
#include "ulv_syscall_flags.h"

#include "ulv_libc.h"

#define ULFS_NO_WEAK

#include "ulfs.h"

#define ULFS_BACKING_HOSTFILE	".ulfs.backing"

int unlink(const char *pathname);

int ulfs_tool_mkfs(int argc, char *argv[]);
int ulfs_tool_get_n_used_inodes(void);

static bool_t
mk_ulfs(char *fpath_backing, char *sizestr)
{
	char	*argv[2];

	argv[0] = fpath_backing;
	argv[1] = sizestr;
	if (ulfs_tool_mkfs(2, argv) != 0)
		return FALSE;
	return TRUE;
}

int
main(int argc, char *argv[])
{
	int	n_used_inodes;
	int	i;

	if (!mk_ulfs(ULFS_BACKING_HOSTFILE, "128m"))
		FAIL("cannot create ULFS");

	setenv("ULV_BLOCK", ".ulfs.backing", 1);
	ulfs_init();

	for (i = 0; i < 256; i++) {
		char	fpath[128];
		int	fd;

		snprintf(fpath, sizeof(fpath), "/testfile.%04d", i);
		fd = ulfs_open(fpath, O_CREAT, 0);
		if (fd < 0) {
			FAIL("cannot create file: %s", fpath);
		}
		ulfs_close(fd);
	}
	for (i = 0; i < 256; i++) {
		char	fpath[128];
		int	err;

		snprintf(fpath, sizeof(fpath), "/testfile.%04d", i);
		err = ulfs_unlink(fpath);
		if (err != 0) {
			FAIL("cannot remove file: %s", fpath);
		}
	}

	if ((n_used_inodes = ulfs_tool_get_n_used_inodes()) != 1)
		FAIL("invalid used inodes: %d(expected: 1)", n_used_inodes);

	unlink(ULFS_BACKING_HOSTFILE);

	PASS("ulfs inode OK");
}
