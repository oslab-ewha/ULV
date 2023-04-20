#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <errno.h>

#include "libmb.h"

int
main(int argc, char *argv[], char *envp[])
{
	struct utsname	utsname;
	struct timeval	tv;
	int	ret;
	unsigned	elapsed;
	int	i;

	init_tickcount();

	for (i = 0; i < 10000; i++) {
		uname(&utsname);
	}

	elapsed = get_tickcount();
	printf("uname elapsed: %d\n", elapsed);

	init_tickcount();

	for (i = 0; i < 10000; i++) {
		gettimeofday(&tv, NULL);
	}
	
	elapsed = get_tickcount();
	printf("gettimeofday elapsed: %d\n", elapsed);
}
