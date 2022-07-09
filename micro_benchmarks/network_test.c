#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#include "libmb.h"

void
main(void)
{
	int	sock;
	int	addr_my = 0xb40011ac;
	int	addr_gw = 0x010011ac;
	struct sockaddr_in	addr;
	int	ret;

	setup_stdout();
	setup_network();

	ret = socket(AF_INET, SOCK_STREAM, 0);
	addr.sin_family = 2;
	addr.sin_port = 0x3075;	/* 30000 */
	addr.sin_addr.s_addr = 0x010011ac;

	ret = connect(ret, (struct sockaddr *)&addr, sizeof(addr));
	printf("ret: %d, errno: %d\n", ret, errno);
}
