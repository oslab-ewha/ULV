#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "libmb.h"

static int	count = 1;

static void
usage(void)
{
	printf(
"mb_loop [<loop count>]\n"
"  loop count default:1\n"
		);
}

int
main(int argc, char *argv[])
{
	float	dummy = 1.32943;
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
		dummy *= 1.3321293;
		if (dummy > 100000)
			dummy /= 100000;
	}

	printf("elapsed: %d\n", get_tickcount());
	printf("dummy: %f\n", dummy);

	return 0;
}
