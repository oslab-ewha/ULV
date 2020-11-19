#include <time.h>
#include <sys/utsname.h>
#include <stdio.h>

void
main(void)
{
	struct utsname	name;
	struct timespec	ts_start, ts_end;
	unsigned long long elapsed;
	int i;

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	for (i = 0; i < 10000; i++) {
		uname(&name);
	}
	clock_gettime(CLOCK_MONOTONIC, &ts_end);

	elapsed = (ts_end.tv_sec - ts_start.tv_sec) * 1000000000 + (ts_end.tv_nsec - ts_start.tv_nsec);
	printf("elapsed: %llu\n", elapsed);
}
