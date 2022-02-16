#include "solo5.h"
#include "liblkl.h"
#include <stdarg.h>

#define UNUSED(x) (void)(x)

#define console_output(fmt, ...)	do {	\
	char	buf[256];			\
	int	len;				\
	len = snprintf(buf, 256, fmt, ##__VA_ARGS__);	\
	solo5_console_write(buf, len); } while (0)

int socket(int a, int b, int c);

typedef struct {
	unsigned short sin_family;
	unsigned short sin_port;
	unsigned int   sin_addr;
	unsigned int   pad[2];
} sockaddr_in;

int connect(int fd, sockaddr_in *s, int c);

int
solo5_app_main(const struct solo5_start_info *si)
{
	int	addr_my = 0xb40011ac;
	int	addr_gw = 0x010011ac;
	sockaddr_in	addr;
	int	ret;

	UNUSED(si);

	console_output("network test\n");

	setup_network(addr_my, addr_gw);

	ret = socket(2, 1, 0);
	addr.sin_family = 2;
	addr.sin_port = 0x3075;	/* 30000 */
	addr.sin_addr = 0x010011ac;

	console_output("socket: %d\n", ret);

	ret = connect(ret, &addr, sizeof(addr));

	console_output("connect: %d\n", ret);

	return 0;
}
