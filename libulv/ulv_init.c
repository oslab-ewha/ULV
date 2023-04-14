#include "ulv_build.h"

extern void ulv_fd_table_init(void);
extern void ulv_thread_init(void);
extern _BUILD_WEAK void ulfs_init(void);
extern _BUILD_WEAK void ulv_net_init(void);
extern void cleanup_epoller(void);

void
ulv_init(void)
{
	ulv_fd_table_init();
	ulv_thread_init();
	if (ulfs_init)
		ulfs_init();
	if (ulv_net_init)
		ulv_net_init();
}

void
ulv_fini(void)
{
	cleanup_epoller();
}
