#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <lkl_host.h>

static int lkl_vprintf(const char *fmt, va_list args)
{
	int n;
	char *buffer;
	va_list copy;

	if (!lkl_host_ops.print)
		return 0;

	va_copy(copy, args);
	n = vsnprintf(NULL, 0, fmt, copy);
	va_end(copy);

	buffer = lkl_host_ops.mem_alloc(n + 1);
	if (!buffer)
		return -1;

	vsnprintf(buffer, n + 1, fmt, args);

	lkl_host_ops.print(buffer, n);
	lkl_host_ops.mem_free(buffer);

	return n;
}

int lkl_printf(const char *fmt, ...)
{
	int n;
	va_list args;

	va_start(args, fmt);
	n = lkl_vprintf(fmt, args);
	va_end(args);

	return n;
}
