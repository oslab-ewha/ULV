#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "netif/tapif.h"

static struct netif	netif;

void tcpip_init(void *, void *);
signed char tcpip_input(struct pbuf *, struct netif *);

void
lwip_add_netif_tap(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip_addr_t *gw)
{
	netif_add(&netif, ipaddr, netmask, gw, NULL, tapif_init, tcpip_input);
	netif_set_default(&netif);
	netif_set_up(&netif);
}

void
ulv_net_init(void)
{
	tcpip_init(0, 0);

	if (getenv("IPADDR") && getenv("NETMASK") && getenv("GWADDR")) {
		ip4_addr_t	ipaddr, netmask, gwaddr;

		ip4addr_aton(getenv("IPADDR"), &ipaddr);
		ip4addr_aton(getenv("NETMASK"), &netmask);
		ip4addr_aton(getenv("GWADDR"), &gwaddr);
		lwip_add_netif_tap(&ipaddr, &netmask, &gwaddr);
	}
}
