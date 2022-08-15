/* inspired from hermitux minifs */

char *strdup(const char *);

#include "ulv_malloc.h"
#include "ulv_syscall_flags.h"
#include "ulv_assert.h"
#include "ulfs_p.h"

extern void ulfs_sb_init(void);
extern void ulfs_file_init(void);

void
ulfs_init(void)
{
	ulfs_block_init();
	ulfs_sb_init();
	ulfs_file_init();
}
