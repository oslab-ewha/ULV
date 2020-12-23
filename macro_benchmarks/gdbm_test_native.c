#include <time.h>
#include <stdio.h>
#include <gdbm.h>
#include <unistd.h>

static void
gdbm_test(void)
{
	GDBM_FILE	dbf;
	int		i;

	dbf = gdbm_open("test.db", 0, GDBM_WRCREAT, 0600, NULL);
	if (dbf == NULL) {
		printf("error\n");
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

	dbf = gdbm_open("test.db", 0, GDBM_READER, 0400, NULL);
	if (dbf == NULL) {
		printf("failed to open\n");
		return;
	}

	for (i = 0; i < 1000; i++) {
		datum	key;
		key.dptr = (char *)&i;
		key.dsize = sizeof(int);
		gdbm_fetch(dbf, key);
	}

	gdbm_close(dbf);

	unlink("test.db");
}

void
main(void)
{
	struct timespec ts_start, ts_end;
	unsigned long long	elapsed;

        clock_gettime(CLOCK_MONOTONIC, &ts_start);

	gdbm_test();

	clock_gettime(CLOCK_MONOTONIC, &ts_end);
        elapsed = (ts_end.tv_sec - ts_start.tv_sec) * 1000000 + (ts_end.tv_nsec - ts_start.tv_nsec) / 1000;
        printf("gdbm elapsed: %lluus\n", elapsed);
}
