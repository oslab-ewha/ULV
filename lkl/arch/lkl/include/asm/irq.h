#ifndef _ASM_LKL_IRQ_H
#define _ASM_LKL_IRQ_H

#define NR_IRQS			((int)sizeof(long) * 8)

void run_irqs(void);
void set_irq_pending(int irq);

#include <uapi/asm/irq.h>

#endif
