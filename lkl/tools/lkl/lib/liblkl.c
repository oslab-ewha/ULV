#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lkl_host.h>

typedef struct {
	int	fd;
	char	mpoint[32];
} fs_t;

void
init_liblkl(void *mem_start, unsigned long mem_size)
{
	lkl_start_kernel(&lkl_host_ops, mem_start, mem_size);
}

int
mount_fs(const char *path_host, void **pfs)
{
	struct lkl_disk	disk;
	fs_t	*fs;
	int	ret;

	disk.fd = open(path_host, O_RDWR);
	if (disk.fd < 0)
		return -1;
	disk.ops = NULL;

	ret = lkl_disk_add(&disk);
	if (ret < 0) {
		close(disk.fd);
		return -2;
	}

	fs = (fs_t *)malloc(sizeof(fs_t));
	ret = lkl_mount_dev(ret, 0, "ext4", 0, NULL, fs->mpoint, 32);
	if (ret) {
		close(disk.fd);
		free(fs);
		return -3;
	}

	fs->fd = disk.fd;

	*pfs = fs;
	return 0;
}

int
umount_fs(void *_fs)
{
	fs_t	*fs = (fs_t *)_fs;

	if (lkl_umount_timeout(fs->mpoint, 0, 1000) < 0)
		return -1;
	close(fs->fd);
	free(fs);
	return 0;
}

char *
get_path(void *_fs, const char *path)
{
	fs_t	*fs = (fs_t *)_fs;
	char	buf[1024];

	snprintf(buf, 1024, "%s%s", fs->mpoint, path);
	return strdup(buf);
}

void
free_path(char *path)
{
	if (path)
		free(path);
}
