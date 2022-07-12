#include "ulvisor.h"

#include <time.h>

#include "ulvisor_thread.h"

static int	pid_trigger;
static int	trigger_done;
static int	futex_clockevent_on;
static int	futex_trigger_exit;

static unsigned long	ns_after;

extern int lkl_trigger_irq(int irq);

static void
trigger_sleep(unsigned long ns)
{
	struct timespec	ts_req, ts_rem;

	ts_req.tv_sec = ns / 1000000000;
	ts_req.tv_nsec = ns % 1000000000;
	nanosleep(&ts_req, &ts_rem);
}

static int
irq_trigger(void *arg)
{
	int	irq = (int)(long)arg;

	while (!trigger_done) {
		if (ns_after == 0)
			futex_wait(&futex_clockevent_on);
		else {
			trigger_sleep(ns_after);
			lkl_trigger_irq(irq);
			ns_after = 0;
		}
	}

	futex_wakeup(&futex_trigger_exit);
}

void
ulvisor_clockevent_setnext(unsigned long ns)
{
	ns_after = ns;
	futex_wakeup(&futex_clockevent_on);
}

void
ulvisor_clockevent_alloc(int irq)
{
	pid_trigger = start_thread(irq_trigger, 256, (void *)(long)irq);
}

void
ulvisor_clockevent_free(void)
{
	if (pid_trigger == 0)
		return;
	trigger_done = 1;
	futex_wakeup(&futex_clockevent_on);
	futex_wait(&futex_trigger_exit);
	pid_trigger = 0;
}
