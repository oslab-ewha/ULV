#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <lkl_host.h>

#include "liblkl.h"

void
init_liblkl(void)
{
	lkl_start_kernel(&lkl_host_ops, "mem=800M");
}

char *
mount_fs(const char *path_host)
{
	struct lkl_disk	disk;
	char	mpoint[32];
	int	ret;

	disk.fd = open(path_host, O_RDWR);
	if (disk.fd < 0)
		return NULL;
	disk.ops = NULL;

	ret = lkl_disk_add(&disk);
	if (ret < 0) {
		close(disk.fd);
		return NULL;
	}

	ret = lkl_mount_dev(ret, 0, "ext4", 0, NULL, mpoint, 32);
	if (ret) {
		close(disk.fd);
		return NULL;
	}
	return strdup(mpoint);
}
