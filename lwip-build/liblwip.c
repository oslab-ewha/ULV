#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "netif/tapif.h"

static struct netif	netif;

void
lwip_add_netif_tap(const ip4_addr_t *ipaddr, const ip4_addr_t *netmask, const ip_addr_t *gw)
{
	netif_add(&netif, ipaddr, netmask, gw, NULL, tapif_init, netif_input);
	netif_set_up(&netif);
}
