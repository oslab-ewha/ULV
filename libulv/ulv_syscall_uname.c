#include <string.h>

struct utsname {
	char	sysname[65];
	char	nodename[65];
	char	release[65];
	char	version[65];
	char	machine[65];
	char	domainname[65];
};

int
ulv_syscall_uname(struct utsname *buf)
{
	strcpy(buf->sysname, "ULV");
	strcpy(buf->nodename, "node");
	strcpy(buf->release, "0.0.1");
	strcpy(buf->version, "0");
	strcpy(buf->machine, "N/A");
	strcpy(buf->domainname, "node");

	return 0;
}
