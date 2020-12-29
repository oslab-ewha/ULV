#include "solo5.h"
#include "liblkl.h"

#define UNUSED(x) (void)(x)

int call_wget(int argc, char **argv);

void group_member(void) {}
void error(void) {}

static void
puts(const char *s)
{
	solo5_console_write(s, strlen(s));
}

static char	*envp[1024] = { 0, 0 };

static void
wget_test(void)
{
	char *argvs[2] = { "wget", "http://google.com" };
	call_wget(2, argvs);
}

int
solo5_app_main(const struct solo5_start_info *si)
{
        solo5_time_t    start;
	uint64_t	elapsed;
	char	buf[256];

	UNUSED(si);

	{
		extern void __init_libc(char *envp[], const char *);
		__init_libc(envp, "lkl");
	}

	puts("wget test");

	start = solo5_clock_monotonic();

	wget_test();

	elapsed = solo5_clock_monotonic() - start;

        snprintf(buf, 256, "Elapsed time: %lluus\n", elapsed / 1000);
        puts(buf);

	return 0;
}
