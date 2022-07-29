extern void ulv_thread_init(void);
extern void tcpip_init(void);

void
init_ulv(void)
{
	ulv_thread_init();
	tcpip_init();
}
