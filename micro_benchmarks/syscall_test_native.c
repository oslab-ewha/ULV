#include <time.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

void
main(void)
{
	struct utsname	name;
	struct timespec	ts_start, ts_end;
	struct timeval	tv;
	unsigned long long elapsed;
	int	fd;
	int i;

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	for (i = 0; i < 10000; i++) {
		uname(&name);
	}
	clock_gettime(CLOCK_MONOTONIC, &ts_end);

	elapsed = (ts_end.tv_sec - ts_start.tv_sec) * 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec) / 1000;
	printf("uname elapsed: %lluus\n", elapsed);

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	for (i = 0; i < 10000; i++) {
		gettimeofday(&tv, NULL);
	}
	clock_gettime(CLOCK_MONOTONIC, &ts_end);

	elapsed = (ts_end.tv_sec - ts_start.tv_sec) * 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec) / 1000;
	printf("gettimeofday elapsed: %lluus\n", elapsed);

	clock_gettime(CLOCK_MONOTONIC, &ts_start);
	for (i = 0; i < 10000; i++) {
		int	fd = open("/", O_RDONLY);
		if (fd >= 0)
			close(fd);
	}
	clock_gettime(CLOCK_MONOTONIC, &ts_end);

	elapsed = (ts_end.tv_sec - ts_start.tv_sec) * 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec) / 1000;
	printf("open/close elapsed: %lluus\n", elapsed);
}
