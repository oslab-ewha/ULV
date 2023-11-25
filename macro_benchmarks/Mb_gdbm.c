#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "libMb.h"

#define PATH_DB	"test.db"

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
fatal(const char *msg)
{
	printf("%s", msg);
}

static void
gdbm_test(int count)
{
	void	*dbf;
	int	i;

	dbf = gdbm_open(PATH_DB, 0, GDBM_WRCREAT, 0600, fatal);
	if (dbf == NULL) {
		printf("%s", gdbm_strerror(gdbm_errno));
		printf("\nfailed to open for create\n");
		return;
	}

	for (i = 0; i < count; i++) {
		datum	key, content;
		key.dptr = (char *)&i;
		key.dsize = sizeof(int);
		content.dptr = (char *)&i;
		content.dsize = sizeof(int);
		gdbm_store(dbf, key, content, 0);
	}
	gdbm_close(dbf);

	dbf = gdbm_open(PATH_DB, 0, GDBM_READER, 0400, NULL);
	if (dbf == NULL) {
		printf("failed to open to fetch\n");
		return;
	}

	for (i = 0; i < count; i++) {
		datum	key;
		key.dptr = (char *)&i;
		key.dsize = sizeof(int);
		gdbm_fetch(dbf, key);
	}

	gdbm_close(dbf);

	unlink(PATH_DB);
}

int
main(int argc, char *argv[])
{
	int	count = 1000;
	unsigned	elapsed;

	if (argc > 1)
		count = atoi(argv[1]);

	init_tickcount();

	gdbm_test(count);

	elapsed = get_tickcount();

	printf("elapsed: %d\n", elapsed);

	return 0;
}
