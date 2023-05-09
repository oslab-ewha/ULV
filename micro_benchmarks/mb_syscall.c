#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/utsname.h>

#include "libmb.h"

static int	count = 1;

static void
usage(void)
{
	printf(
"mb_syscall [<loop count>]\n"
"  loop count default:1\n"
		);
}

int
main(int argc, char *argv[])
{
	struct utsname	utsname;
	struct timeval	tv;
	unsigned	elapsed;
	int	i;

	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0) {
			usage();
			return 0;
		}
		count = atoi(argv[1]);
	}

	init_tickcount();

	for (i = 0; i < count; i++) {
		uname(&utsname);
	}

	elapsed = get_tickcount();
	printf("uname elapsed: %d\n", elapsed);

	init_tickcount();

	for (i = 0; i < count; i++) {
		gettimeofday(&tv, NULL);
	}
	
	elapsed = get_tickcount();
	printf("gettimeofday elapsed: %d\n", elapsed);
}
