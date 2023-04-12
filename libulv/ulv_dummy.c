__attribute__((__weak__)) void
ulfs_init(void)
{
}

__attribute__((__weak__)) void
ulfs_open(void)
{
}

__attribute__((__weak__)) void
ulfs_close(void)
{
}

__attribute__((__weak__)) void
ulfs_read(void)
{
}

__attribute__((__weak__)) void
ulfs_write(void)
{
}

__attribute__((__weak__)) void
ulfs_mkdir(void)
{
}

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

__attribute__((__weak__)) int
lwip_bind(long fd, long addr, long len)
{
	return -1;
}

__attribute__((__weak__)) int
lwip_listen(long fd, long backlog)
{
	return -1;
}

__attribute__((__weak__)) int
lwip_sendto(long fd, long buf, long len, long flags, long dest_addr, long addrlen)
{
	return -1;
}

__attribute__((__weak__)) int
lwip_recvfrom(long fd, long buf, long len, long flags, long src_addr, long addrlen)
{
	return -1;
}
