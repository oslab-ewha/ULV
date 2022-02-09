#include <asm/host_ops.h>
#include <asm/lkl_dbg.h>

void
dbg_output(const char *fmt, ...)
{
	va_list	args;
	char	buf[256];
	int	len;

	va_start(args, fmt);
	len = vsnprintf(buf, 256, fmt, args);
	va_end(args);

	lkl_ops->print(buf, len);
}

const char *
dbg_thread(struct thread_info *ti)
{
	if (ti == NULL)
		return "<none>";
	if (ti->task->comm)
		return ti->task->comm;
	return "<unknown>";
}
