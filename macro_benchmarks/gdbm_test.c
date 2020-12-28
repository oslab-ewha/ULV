#include "solo5.h"
#include "liblkl.h"

#define UNUSED(x) (void)(x)

# define GDBM_READER    0       /* A reader. */
# define GDBM_WRITER    1       /* A writer. */
# define GDBM_WRCREAT   2       /* A writer.  Create the db if needed. */

typedef struct
{
	char	*dptr;
	int	dsize;
} datum;

typedef int gdbm_error;

void *gdbm_open(const char *name, int block_size, int flags, int mode, void (*fatal_func)(const char *));
int gdbm_store(void *dbf, datum key, datum content, int flag);
datum gdbm_fetch(void *dbf, datum key);
void gdbm_close(void *dbf);

extern int *gdbm_errno_location (void);
#define gdbm_errno (*gdbm_errno_location ())

gdbm_error gdbm_last_errno(void *dbf);
const char *gdbm_strerror(gdbm_error);

static void
puts(const char *s)
{
	solo5_console_write(s, strlen(s));
}

static char	*path_dbf;

static void
fatal(const char *msg)
{
	puts(msg);
}

static char	*envp[1024] = { 0, 0 };

static void
gdbm_test(void)
{
	void	*dbf;
	int	i;

	dbf = gdbm_open(path_dbf, 0, GDBM_WRCREAT, 0600, fatal);
	if (dbf == NULL) {
		puts(gdbm_strerror(gdbm_errno));
		puts("\nfailed to open for create\n");
		return;
	}

	for (i = 0; i < 1000; i++) {
		datum	key, content;
		key.dptr = (char *)&i;
		key.dsize = sizeof(int);
		content.dptr = (char *)&i;
		content.dsize = sizeof(int);
		gdbm_store(dbf, key, content, 0);
	}
	gdbm_close(dbf);

	dbf = gdbm_open(path_dbf, 0, GDBM_READER, 0400, NULL);
	if (dbf == NULL) {
		puts("failed to open to fetch\n");
		return;
	}

	for (i = 0; i < 1000; i++) {
		datum	key;
		key.dptr = (char *)&i;
		key.dsize = sizeof(int);
		gdbm_fetch(dbf, key);
	}

	gdbm_close(dbf);

	lkl_sys_unlink(path_dbf);
}

int
solo5_app_main(const struct solo5_start_info *si)
{
        solo5_time_t    start;
	uint64_t	elapsed;
        void    *fs;
	char	buf[256];

	UNUSED(si);

	{
		extern void __init_libc(char *envp[], const char *);
		__init_libc(envp, "lkl");
	}

	puts("gdbm test");

        if (mount_fs("/tmp/testfile", &fs) < 0) {
                puts("failed to mount\n");
                return 1;
        }

	path_dbf = get_path(fs, "/test.db");

	start = solo5_clock_monotonic();

	gdbm_test();

	elapsed = solo5_clock_monotonic() - start;

	free_path(path_dbf);
	umount_fs(fs);

        snprintf(buf, 256, "Elapsed time: %lluus\n", elapsed / 1000);
        puts(buf);

	return 0;
}
