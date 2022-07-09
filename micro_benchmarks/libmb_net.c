#include <stdio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <errno.h>

int
setup_network(void)
{
	int	sock;
	struct ifreq	ifr;
	int	ret;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth0");
	ret = ioctl(sock, SIOCGIFINDEX, (long)&ifr);

	printf("ret: %d, errno: %d\n", ret, errno);
}

