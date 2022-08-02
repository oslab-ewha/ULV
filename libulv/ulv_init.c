extern void ulv_thread_init(void);
extern void tcpip_init(void *, void *);

#include "lwip/ip_addr.h"
#include "lwip/inet.h"

void lwip_add_netif_tap(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip_addr_t *gw);

void
init_ulv(void)
{
	ulv_thread_init();
	tcpip_init(0, 0);

	if (getenv("IPADDR") && getenv("NETMASK") && getenv("GWADDR")) {
		ip4_addr_t	ipaddr, netmask, gwaddr;

		ip4addr_aton(getenv("IPADDR"), &ipaddr);
		ip4addr_aton(getenv("NETMASK"), &netmask);
		ip4addr_aton(getenv("GWADDR"), &gwaddr);
		lwip_add_netif_tap(&ipaddr, &netmask, &gwaddr);
	}
}
