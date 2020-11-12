#include "solo5.h"
#include "../../bindings/lib.c"
#include <lkl.h>

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
	int	i;

	UNUSED(si);

	puts("start uname() call 100 times\n");
	for (i = 0; i < 100; i++) {
		lkl_sys_uname(&uname);
	}
	puts("done\n");

	return 0;
}
