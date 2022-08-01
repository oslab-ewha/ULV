extern void ulv_thread_init(void);
extern void tcpip_init(void);

#include "lwip/ip_addr.h"

void lwip_add_netif_tap(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip_addr_t *gw);

void
init_ulv(void)
{
	ip_addr_t	ipaddr, netmask, gwaddr;

	ulv_thread_init();
	tcpip_init();

	IP4_ADDR(&ipaddr, 192, 168, 1, 200);
	IP4_ADDR(&netmask, 255, 255, 255, 0);
	IP4_ADDR(&gwaddr, 192, 168, 1, 1);

	lwip_add_netif_tap(&ipaddr, &netmask, &gwaddr);
}
