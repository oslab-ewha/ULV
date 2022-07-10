#ifndef _LIBMB_H_
#define _LIBMB_H_

void setup_stdout(void);

void init_tickcount(void);
unsigned get_tickcount(void);

int setup_network(const char *addr_my, const char *addr_gw);

#endif
