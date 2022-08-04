__attribute__((__weak__)) void
ulv_net_init(void)
{
}

__attribute__((__weak__)) int
lwip_socket(long domain, long type, long protocol)
{
	return -1;
}

__attribute__((__weak__)) int
lwip_connect(long fd, long name, long len)
{
	return -1;
}
