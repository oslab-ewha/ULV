#include <linux/kernel.h>
#include <linux/sched/stat.h>
#include <asm/host_ops.h>
#include <asm/cpu.h>
#include <asm/thread_info.h>
#include <asm/unistd.h>
#include <asm/sched.h>
#include <asm/syscalls.h>


/*
 * This structure is used to get access to the "LKL CPU" that allows us to run
 * Linux code. Because we have to deal with various synchronization requirements
 * between idle thread, system calls, interrupts, "reentrancy", CPU shutdown,
 * imbalance wake up (i.e. acquire the CPU from one thread and release it from
 * another), we can't use a simple synchronization mechanism such as (recursive)
 * mutex or semaphore. Instead, we use a mutex and a bunch of status data plus a
 * semaphore.
 */
struct lkl_cpu {
	/*
	 * Since we must free the cpu lock during shutdown we need a
	 * synchronization algorithm between lkl_cpu_shutdown() and the CPU
	 * access functions since lkl_cpu_get() gets called from thread
	 * destructor callback functions which may be scheduled after
	 * lkl_cpu_shutdown() has freed the cpu lock.
	 *
	 * An atomic counter is used to keep track of the number of running
	 * CPU access functions and allow the shutdown function to wait for
	 * them.
	 *
	 * The shutdown functions adds MAX_THREADS to this counter which allows
	 * the CPU access functions to check if the shutdown process has
	 * started.
	 *
	 * This algorithm assumes that we never have more the MAX_THREADS
	 * requesting CPU access.
	 */
	#define MAX_THREADS 1000000
	unsigned int shutdown_gate;
	bool irqs_pending;
	/* no of threads waiting the CPU */
	unsigned int sleepers;
	/* no of times the current thread got the CPU */
	unsigned int count;
	/* current thread that owns the CPU */
	lkl_thread_t owner;
	/* semaphore for threads waiting the CPU */
	struct lkl_sem *sem;
	/* semaphore used for shutdown */
	struct lkl_sem *shutdown_sem;
} cpu;

void lkl_cpu_shutdown(void)
{
	__sync_fetch_and_add(&cpu.shutdown_gate, MAX_THREADS);
}

void lkl_cpu_wait_shutdown(void)
{
	lkl_ops->sem_down(cpu.shutdown_sem);
	lkl_ops->sem_free(cpu.shutdown_sem);
}

static void lkl_cpu_cleanup(bool shutdown)
{
	while (__sync_fetch_and_add(&cpu.shutdown_gate, 0) > MAX_THREADS)
		;

	if (shutdown)
		lkl_ops->sem_up(cpu.shutdown_sem);
	else if (cpu.shutdown_sem)
		lkl_ops->sem_free(cpu.shutdown_sem);
	if (cpu.sem)
		lkl_ops->sem_free(cpu.sem);
}

int lkl_cpu_init(void)
{
	cpu.sem = lkl_ops->sem_alloc(0);
	cpu.shutdown_sem = lkl_ops->sem_alloc(0);

	if (!cpu.sem || !cpu.shutdown_sem) {
		lkl_cpu_cleanup(false);
		return -ENOMEM;
	}

	return 0;
}
