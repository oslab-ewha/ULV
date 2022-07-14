#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <lkl_host.h>
#include "virtio.h"
#include "../../../../solo5/include/solo5/solo5.h"

typedef struct {
	int	fd;
	char	mpoint[32];
} fs_t;

void
start_lkl_kernel(void *mem_start, unsigned long mem_size)
{
	lkl_start_kernel(&lkl_host_ops, mem_start, mem_size);
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

void
setup_lkl_network(int idx, int addr_my, int netmask, int addr_gw)
{
	int	ifidx;

	ifidx = lkl_netdev_get_ifindex(idx);
	lkl_if_up(ifidx);
	lkl_if_set_mtu(ifidx, 1500);
	lkl_if_set_ipv4(ifidx, addr_my, netmask);
	lkl_set_ipv4_gateway(addr_gw);
#if 0
	//TODO: There's a serious delay if ARP cache does not exist
	lkl_add_neighbor(ifidx, 2, ipaddr, macaddr);
#endif
}
