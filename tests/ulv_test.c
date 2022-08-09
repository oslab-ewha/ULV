#include "ulv_test.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

static int	passed;
static int	failed;

void
PASS(const char *fmt, ...)
{
	va_list ap;
	char	buf[1024];

	passed++;
	va_start(ap, fmt);
	vsnprintf (buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printf("[PASSED]: %s\n", buf);
}

void
FAIL(const char *fmt, ...)
{
	va_list ap;
	char	buf[1024];

	failed++;
	va_start(ap, fmt);
	vsnprintf (buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printf("\033[31m[FAILED]\033[0m: %s\n", buf);
	exit(1);
}

void
verbose(const char *fmt, ...)
{
	va_list ap;
	char	buf[1024];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);
	printf("%s", buf);
}
