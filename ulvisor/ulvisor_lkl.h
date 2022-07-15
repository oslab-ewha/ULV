#ifndef _ULVISOR_LKL_H_
#define _ULVISOR_LKL_H_

void lkl_trigger_irq(int irq);
int lkl_get_free_irq(const char *user);
void lkl_put_irq(int i, const char *user);

#endif
