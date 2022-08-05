#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>

#include "libmb.h"

static void
usage(void)
{
	printf("Usage: mb_network <connect IP> [<bind IP>]\n");
	printf("   connect IP: {ipaddr}:{port}\n");
}

static int
get_connect_addr(const char *c_straddr, struct sockaddr_in *pinaddr)
{
	char	*straddr;
	char	*p;
	unsigned short	port = 30000;
	int	ret;

	straddr = strdup(c_straddr);
	if ((p = strchr(straddr, ':'))) {
		*p = '\0';
		if (sscanf(p + 1, "%hu", &port) != 1) {
			free(straddr);
			return -1;
		}
	}

	ret = inet_aton(straddr, &pinaddr->sin_addr);
	free(straddr);
	if (ret == 0)
		return -1;
	pinaddr->sin_port = htons(port);

	return 0;
}

int
main(int argc, char *argv[])
{
	int	fd;
	struct sockaddr_in	addr;
	char	buf[128];
	int	ret;

	if (argc < 2) {
		usage();
		return 1;
	}

	init_tickcount();
	fd = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	if (get_connect_addr(argv[1], &addr) < 0)
		return 1;
	if (argc >= 3) {
		struct sockaddr_in	addr_bind;

		inet_aton(argv[2], &addr_bind.sin_addr);
		addr_bind.sin_family = AF_INET;
		addr_bind.sin_port = 0;
		ret = bind(fd, (struct sockaddr *)&addr_bind, sizeof(struct sockaddr_in));
		if (ret != 0) {
			printf("bind error: %d\n", errno);
			return 1;
		}
	}

	ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
		printf("failed to connect! errno: %d\n", errno);
		return 1;
	}
	ret = send(fd, "this is test\n", 13, 0);
	if (ret < 0) {
		printf("failed to send! errno: %d\n", errno);
		return 1;
	}
	ret = recv(fd, buf, sizeof(buf) - 1, 0);
	if (ret < 0) {
		printf("failed to recv! errno: %d\n", errno);
		return 1;
	}
	buf[ret] = '\0';
	printf("elapsed: %d\n", get_tickcount());

	printf("recv: %s\n", buf);

	return 0;
}
