extern void ulv_thread_init(void);
extern void ulv_net_init(void);

void
init_ulv(void)
{
	ulv_thread_init();
	ulv_net_init();
}
