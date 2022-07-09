#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

void
setup_stdout(void)
{
	int	fd;

	fd = open("/dev/console", O_RDWR);
	if (fd >= 0) {
		if (fd != 1) {
			dup2(fd, 1);
			close(fd);
		}
	}
}
