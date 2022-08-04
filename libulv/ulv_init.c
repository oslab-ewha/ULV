extern void ulv_thread_init(void);
extern void ulv_net_init(void);
extern void cleanup_epoller(void);

void
ulv_init(void)
{
	ulv_thread_init();
	ulv_net_init();
}

void
ulv_fini(void)
{
	cleanup_epoller();
}
