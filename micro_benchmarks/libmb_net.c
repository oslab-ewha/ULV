#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_addr.h>
#include <string.h>
#include <errno.h>

static int
netlink_sock(unsigned int groups)
{
	struct sockaddr_nl	la;
	int	fd, err;

	fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);
	if (fd < 0)
		return fd;

	memset(&la, 0, sizeof(la));
	la.nl_family = AF_NETLINK;
	la.nl_groups = groups;
	err = bind(fd, (struct sockaddr *)&la, sizeof(la));
	if (err < 0)
		return err;

	return fd;
}

static int
rtnl_listen(int fd, int (*handler)(struct sockaddr_nl *nladdr,
				   struct nlmsghdr *, void *),
	    void *arg)
{
	struct nlmsghdr	*h;
	struct sockaddr_nl	nladdr = { .nl_family = AF_NETLINK };
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char   buf[16384];
	int	status;

	iov.iov_base = buf;
	while (1) {
		iov.iov_len = sizeof(buf);
		status = recvmsg(fd, &msg, 0);

		if (status < 0) {
			if (status == -EINTR || status == -EAGAIN)
				continue;
			if (status == -ENOBUFS)
				continue;
			return status;
		}
		if (status == 0)
			return -1;
		if (msg.msg_namelen != sizeof(nladdr))
			return -1;

		for (h = (struct nlmsghdr *)buf; (unsigned int)status >= sizeof(*h);) {
			int	len = h->nlmsg_len;
			int	l = len - sizeof(*h);
			int	err;

			if (l < 0 || len > status) {
				if (msg.msg_flags & MSG_TRUNC) {
					return -1;
				}
				return -1;
			}

			err = handler(&nladdr, h, arg);
			if (err <= 0)
				return err;

			status -= NLMSG_ALIGN(len);
			h = (struct nlmsghdr *)((char *)h + NLMSG_ALIGN(len));
		}
		if (msg.msg_flags & MSG_TRUNC) {
			continue;
		}
		if (status) {
			return -1;
		}
	}
}

static int
check_error(struct sockaddr_nl *nladdr, struct nlmsghdr *n, void *arg)
{
	unsigned int	s = *(unsigned int *)arg;

	if (nladdr->nl_pid != 0 || n->nlmsg_seq != s) {
		/* Don't forget to skip that message. */
		return 1;
	}

	if (n->nlmsg_type == NLMSG_ERROR) {
		struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(n);
		int	l = n->nlmsg_len - sizeof(*n);

		if (l >= (int)sizeof(struct nlmsgerr) && !err->error)
			return 0;

		return err->error;
	}
	return -1;
}

static unsigned int	seq;

static int
rtnl_talk(int fd, struct nlmsghdr *n)
{
	int	status;
	struct sockaddr_nl	nladdr = { .nl_family = AF_NETLINK };
	struct iovec	iov = {.iov_base = (void *)n, .iov_len = n->nlmsg_len};
	struct msghdr	msg = {
			.msg_name = &nladdr,
			.msg_namelen = sizeof(nladdr),
			.msg_iov = &iov,
			.msg_iovlen = 1,
	};

	n->nlmsg_seq = seq;
	n->nlmsg_flags |= NLM_F_ACK;

	status = sendmsg(fd, &msg, 0);
	if (status < 0) {
		return status;
	}

	status = rtnl_listen(fd, check_error, (void *)&seq);
	seq++;
	return status;
}

static int
addattr_l(struct nlmsghdr *n, unsigned int maxlen, int type, const void *data, int alen)
{
	struct rtattr	*rta;
	int	len = RTA_LENGTH(alen);

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		return -1;
	}
	rta = ((struct rtattr *)(((void *) (n)) + NLMSG_ALIGN(n->nlmsg_len)));
	rta->rta_type = type;
	rta->rta_len = len;
	memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	return 0;
}

static int
set_ipaddr(int ifindex, int addr, unsigned int netmask_len)
{
	struct {
		struct nlmsghdr		n;
		struct ifaddrmsg	ifa;
		char buf[256];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifaddrmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL,
		.n.nlmsg_type = RTM_NEWADDR,
		.ifa.ifa_family = AF_INET,
		.ifa.ifa_prefixlen = netmask_len,
		.ifa.ifa_index = ifindex,
	};
	int	fd;
	int	err, addr_sz;

	fd = netlink_sock(0);
	if (fd < 0)
		return -1;

	// create the IP attribute
	addattr_l(&req.n, sizeof(req), IFA_LOCAL, &addr, 4);

	err = rtnl_talk(fd, &req.n);

	close(fd);
	return err;
}

static int
add_route(int ifindex, int addr_gw)
{
	struct {
		struct nlmsghdr	n;
		struct rtmsg	r;
		char		buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
		.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE | NLM_F_EXCL,
		.n.nlmsg_type = RTM_NEWROUTE,
		.r.rtm_family = AF_INET,
		.r.rtm_table = RT_TABLE_MAIN,
		.r.rtm_scope = RT_SCOPE_UNIVERSE,
		.r.rtm_protocol = RTPROT_BOOT,
		.r.rtm_scope = RT_SCOPE_UNIVERSE,
		.r.rtm_type = RTN_UNICAST,
	};
	int	err, addr_sz;
	int	i, fd;

	fd = netlink_sock(0);
	if (fd < 0)
		return -1;

	addattr_l(&req.n, sizeof(req), RTA_GATEWAY, &addr_gw, 4);

	req.r.rtm_table = ifindex * 2;
	addattr_l(&req.n, sizeof(req), RTA_OIF, &ifindex, 4);

	err = rtnl_talk(fd, &req.n);
	close(fd);
	return err;
}

static int
get_addr_netmask(const char *c_straddr, int *paddr, int *pnetmask)
{
	char	*straddr;
	struct in_addr	inaddr;
	char	*p;
	int	ret;

	straddr = strdup(c_straddr);
	if ((p = strchr(straddr, '/'))) {
		*p = '\0';
		if (sscanf(p + 1, "%u", pnetmask) != 1 && *pnetmask > 32) {
			free(straddr);
			return -1;
		}
	}
	else if (pnetmask)
		*pnetmask = 32;

	ret = inet_aton(straddr, &inaddr);
	free(straddr);
	if (ret == 0)
		return -1;

	*paddr = inaddr.s_addr;
	return 0;
}

int
setup_network(const char *str_addr_my, const char *str_addr_gw)
{
	int	sock;
	struct ifreq	ifr;
	int	ifindex;
	int	addr, netmask;
	int	ret;

	if (get_addr_netmask(str_addr_my, &addr, &netmask) < 0)
		return -1;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return -1;
	}

	snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "eth0");
	ret = ioctl(sock, SIOCGIFINDEX, (long)&ifr);
	if (ret < 0) {
		close(sock);
		return -1;
	}

	ifindex = ifr.ifr_ifindex;
	ifr.ifr_flags |= IFF_UP;
	ioctl(sock, SIOCSIFFLAGS, (long)&ifr);
	ifr.ifr_mtu = 1500;
	ioctl(sock, SIOCSIFMTU, (long)&ifr);

	if (set_ipaddr(ifindex, addr, netmask) < 0)
		return -1;
	if (str_addr_gw) {
		int	addr_gw;

		if (get_addr_netmask(str_addr_gw, &addr_gw, NULL) == 0)
			return add_route(ifindex, addr_gw);
	}
	return 0;
}
