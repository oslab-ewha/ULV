#include <linux/clocksource.h>
#include <linux/clockchips.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <asm/host_ops.h>

static unsigned long long boot_time;

void __ndelay(unsigned long nsecs)
{
	unsigned long long start = lkl_ops->time();

	while (lkl_ops->time() < start + nsecs)
		;
}

void __udelay(unsigned long usecs)
{
	__ndelay(usecs * NSEC_PER_USEC);
}

void __const_udelay(unsigned long xloops)
{
	__udelay(xloops / 0x10c7ul);
}

void calibrate_delay(void)
{
}

void read_persistent_clock(struct timespec *ts)
{
	*ts = ns_to_timespec(lkl_ops->time());
}

/*
 * Scheduler clock - returns current time in nanosec units.
 *
 */
unsigned long long sched_clock(void)
{
	if (!boot_time)
		return 0;

	return lkl_ops->time() - boot_time;
}

static u64 clock_read(struct clocksource *cs)
{
	return lkl_ops->time();
}

static struct clocksource clocksource = {
	.name	= "lkl",
	.rating = 499,
	.read	= clock_read,
	.flags	= CLOCK_SOURCE_IS_CONTINUOUS,
	.mask	= CLOCKSOURCE_MASK(64),
};

static int timer_irq;

static irqreturn_t timer_irq_handler(int irq, void *dev_id)
{
	struct clock_event_device *dev = (struct clock_event_device *)dev_id;

	dev->event_handler(dev);

	return IRQ_HANDLED;
}

static int
clockevent_alloc(struct clock_event_device *evt)
{
	lkl_ops->clockevent_alloc(timer_irq);
	return 0;
}

static int
clockevent_free(struct clock_event_device *evt)
{
	lkl_ops->clockevent_free();
	return 0;
}

static int
clockevent_set_next(unsigned long ns, struct clock_event_device *evt)
{
	lkl_ops->clockevent_set_next(ns);
	return 0;
}

static struct clock_event_device clockevent = {
	.name			= "lkl",
	.features		= CLOCK_EVT_FEAT_ONESHOT,
	.set_state_oneshot	= clockevent_alloc,
	.set_next_event		= clockevent_set_next,
	.set_state_shutdown	= clockevent_free,
};

static struct irqaction irq0  = {
	.handler = timer_irq_handler,
	.flags = IRQF_NOBALANCING | IRQF_TIMER,
	.dev_id = &clockevent,
	.name = "timer"
};

void __init time_init(void)
{
	int	ret;

	timer_irq = lkl_get_free_irq("timer");
	setup_irq(timer_irq, &irq0);

	ret = clocksource_register_khz(&clocksource, 1000000);
	if (ret)
		pr_err("lkl: unable to register clocksource\n");

	clockevents_config_and_register(&clockevent, NSEC_PER_SEC, 1, ULONG_MAX);

	boot_time = lkl_ops->time();
	pr_info("lkl: time and timers initialized (irq%d)\n", timer_irq);
}
