#include "solo5.h"
#include "liblkl.h"

#define UNUSED(x) (void)(x)

static void
puts(const char *s)
{
	solo5_console_write(s, strlen(s));
}

int
solo5_app_main(const struct solo5_start_info *si)
{
	struct utsname	uname;
	struct timeval	tv;
	void	*fs;
	char	*path_open;
	uint64_t	elapsed;
	char	buf[256];
	solo5_time_t	start;
	int	i;

	UNUSED(si);

	puts("start uname() call 10000 times\n");

	start = solo5_clock_monotonic();
	for (i = 0; i < 10000; i++) {
		lkl_sys_uname(&uname);
	}

	elapsed = solo5_clock_monotonic() - start;

	snprintf(buf, 256, "Elapsed time: %lluus\n", elapsed / 1000);
	puts(buf);

	puts("start gettimeofday() call 10000 times\n");

	start = solo5_clock_monotonic();
	for (i = 0; i < 10000; i++) {
		lkl_sys_gettimeofday(&tv);
	}

	elapsed = solo5_clock_monotonic() - start;

	snprintf(buf, 256, "Elapsed time: %lluus\n", elapsed / 1000);
	puts(buf);

	puts("start open()/close() call 10000 times\n");

	if (mount_fs("/tmp/testfile", &fs) < 0) {
		puts("failed to mount\n");
		return 1;
	}

	path_open = get_path(fs, "/test.txt");

	start = solo5_clock_monotonic();
	for (i = 0; i < 10000; i++) {
		int fd = lkl_sys_open(path_open, 0);
		if (fd >= 0) {
			lkl_sys_close(fd);
		}
	}

	elapsed = solo5_clock_monotonic() - start;

	free_path(path_open);
	umount_fs(fs);

	snprintf(buf, 256, "Elapsed time: %lluus\n", elapsed / 1000);
	puts(buf);

	return 0;
}
