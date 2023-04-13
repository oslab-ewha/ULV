#include <stdarg.h>

#include "ulv_syscall_no.h"
#include "ulv_host_syscall.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"

int vsnprintf(char *str, int size, const char *fmt, va_list ap);

#pragma GCC diagnostic pop

void
ulv_crash(void)
{
	__asm__ __volatile__( "hlt" : : : "memory" );
}

void
ulv_verbose(const char *fmt, ...)
{
	va_list	ap;
	char	buf[256];
	int	size;

	va_start(ap, fmt);
	size = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	__syscall3(__NR_write, (long)0, (long)buf, (long)size);
}
