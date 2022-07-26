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
	printf("Usage: mb_network <my IP> <gateway IP> <connect IP>\n");
	printf("   my IP: {ipaddr}/{netmask prefix}\n");
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
	int	sock;
	struct sockaddr_in	addr;
	int	ret;

	if (argc < 4) {
		usage();
		return 1;
	}

	setup_network(argv[1], argv[2]);

	ret = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = AF_INET;
	if (get_connect_addr(argv[3], &addr) < 0)
		return 1;

	ret = connect(ret, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == 0)
		printf("connected\n");
	else
		printf("failed! errno: %d\n", errno);

	return 0;
}
