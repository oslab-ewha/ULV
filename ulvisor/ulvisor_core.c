/*
 * Copyright (c) 2015-2019 Contributors as noted in the AUTHORS file
 *
 * This file is part of Solo5, a sandboxed execution environment.
 *
 * Permission to use, copy, modify, and/or distribute this software
 * for any purpose with or without fee is hereby granted, provided
 * that the above copyright notice and this permission notice appear
 * in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT,
 * NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define _GNU_SOURCE
#include <assert.h>
#include <err.h>
#include <libgen.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <seccomp.h>
#include <sys/personality.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <limits.h>

#include <linux/filter.h>
#include <linux/seccomp.h>

#if defined(__x86_64__)
#include <asm/prctl.h>
#endif

#include "ulvisor.h"

/*
 * TODO: Split up the functions in this module better, and introduce something
 * similar to hvt_gpa_t for clarity.
 */

/*
 * Defined by standard GNU ld linker scripts to the lowest address of the text
 * segment.
 */
extern long __executable_start;

static bool use_exec_heap = false;

ulvisor_t *
ulvisor_init(size_t mem_size, int argc, char **argv)
{
	ulvisor_t	*ulvisor = malloc(sizeof(ulvisor_t));

	if (ulvisor == NULL)
		err(1, "malloc");
	memset(ulvisor, 0, sizeof (struct ulvisor));

#if defined(__PIE__)
	/*
	 * On systems where we are built as a PIE executable:
	 *
	 * The kernel will apply ASLR and map the tender at a high virtual address
	 * (see ELF_ET_DYN_BASE in the kernel source for the arch-specific value,
	 * as we only support 64-bit architectures for now where this should always
	 * be >= 4 GB).
	 *
	 * Therefore, rather than mislead the user with an incorrect error message,
	 * assert that a) the tender has been loaded with a base address of at
	 * least 4GB and b) tender address space does not overlap with guest
	 * address space. We can re-visit this if it turns out that users run on
	 * systems where this does not hold (e.g. kernel ASLR is disabled).
	 */
	assert((uint64_t)&__executable_start >= (1ULL << 32));
	assert((uint64_t)(mem_size - 1) < (uint64_t)&__executable_start);
#else
	/*
	 * On systems where we are NOT built as a PIE executable, first assert that
	 * -Ttext-segment has been correctly passed at the link step (see
	 * configure.sh), and then check that guest memory size is within limits.
	 */
	assert((uint64_t)&__executable_start >= (1ULL << 30));
	if ((uint64_t)(mem_size - 1) >= (uint64_t)&__executable_start) {
		uint64_t max_mem_size_mb = (uint64_t)&__executable_start >> 20;
		warnx("Maximum guest memory size (%lu MB) exceeded.",
		      max_mem_size_mb);
		errx(1, "Either decrease --mem-size, or recompile ulvisor"
		     " as a PIE executable.");
	}
#endif

	/*
	 * Sooo... it turns out that at least on some distributions, the Linux
	 * "personality" flag READ_IMPLIES_EXEC is the default unless linked with
	 * -z noexecstack. This is bad, as it results in mmap() with PROT_READ
	 *  implying PROT_EXEC. Cowardly refuse to run on such systems.
	 */
	int persona = -1;
	persona = personality(0xffffffff);
	assert(persona >= 0);
	if (persona & READ_IMPLIES_EXEC)
		errx(1, "Cowardly refusing to run with a sys_personality of "
		     "READ_IMPLIES_EXEC. Please report a bug, with details of your "
		     "Linux distribution and GCC version");

	/*
	 * ulvisor->mem is addressed starting at 0, however we cannot actually map it
	 * at 0 due to restrictions on mapping low memory addresses present in
	 * modern Linux kernels (vm.mmap_min_addr sysctl). Therefore, we map
	 * ulvisor_mem at SPT_HOST_MEM_BASE, adjusting the returned pointer and region
	 * size appropriately.
	 */
	int prot = PROT_READ | PROT_WRITE | (use_exec_heap ? PROT_EXEC : 0);
	ulvisor->mem = mmap((void *)ULVISOR_HOST_MEM_BASE, mem_size - ULVISOR_HOST_MEM_BASE,
			    prot, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);
	if (ulvisor->mem == MAP_FAILED)
		err(1, "Error allocating guest memory");
	assert(ulvisor->mem == (void *)ULVISOR_HOST_MEM_BASE);
	ulvisor->mem -= ULVISOR_HOST_MEM_BASE;
	ulvisor->mem_size = mem_size;

	ulvisor->epollfd = epoll_create1(0);
	if (ulvisor->epollfd == -1)
		err(1, "epoll_create1() failed");
	ulvisor->timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
	if (ulvisor->timerfd == -1)
		err(1, "timerfd_create() failed");
	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.u64 = ULVISOR_INTERNAL_TIMERFD;
	if (epoll_ctl(ulvisor->epollfd, EPOLL_CTL_ADD, ulvisor->timerfd, &ev) == -1)
		err(1, "epoll_ctl(EPOLL_CTL_ADD) failed");

	ulvisor->sc_ctx = seccomp_init(SCMP_ACT_NOTIFY);
	assert(ulvisor->sc_ctx != NULL);

	ulvisor->argc = argc;
	ulvisor->argv = argv;

	return ulvisor;
}

int
ulvisor_guest_mprotect(void *t_arg, uint64_t addr_start, uint64_t addr_end, int prot)
{
	ulvisor_t	*ulvisor = t_arg;

	assert(addr_start <= ulvisor->mem_size);
	assert(addr_end <= ulvisor->mem_size);
	assert(addr_start < addr_end);

	uint8_t *vaddr_start = ulvisor->mem + addr_start;
	assert(vaddr_start >= ulvisor->mem);
	size_t size = addr_end - addr_start;
	assert(size > 0 && size <= ulvisor->mem_size);

	/*
	 * On ulvisor, there is no distinction between host-side and guest-side memory
	 * protection, so just pass through to mprotect() directly, which will do
	 * the right thing.
	 */
	return mprotect(vaddr_start, size, prot);
}

static void
setup_cmdline(uint8_t *cmdline, int argc, char **argv)
{
	size_t cmdline_free = ULVISOR_CMDLINE_SIZE;

	cmdline[0] = 0;

	for (; *argv; argc--, argv++) {
		size_t alen = snprintf((char *)cmdline, cmdline_free, "%s%s", *argv,
				       (argc > 1) ? " " : "");
		if (alen >= cmdline_free) {
			errx(1, "Guest command line too long (max=%d characters)",
			     ULVISOR_CMDLINE_SIZE - 1);
			break;
		}
		cmdline_free -= alen;
		cmdline += alen;
	}
}

void
ulvisor_boot_info_init(ulvisor_t *ulvisor, uint64_t p_end, int cmdline_argc, char **cmdline_argv)
{
	uint64_t lowmem_pos = ULVISOR_BOOT_INFO_BASE;

	struct ulvisor_boot_info *bi = (struct ulvisor_boot_info *)(ulvisor->mem + lowmem_pos);
	lowmem_pos += sizeof (struct ulvisor_boot_info);
	bi->mem_size = ulvisor->mem_size;
	bi->kernel_end = p_end;
	bi->epollfd = ulvisor->epollfd;
	bi->timerfd = ulvisor->timerfd;

	bi->cmdline = (void *)lowmem_pos;
	setup_cmdline(ulvisor->mem + lowmem_pos, cmdline_argc, cmdline_argv);
	lowmem_pos += ULVISOR_CMDLINE_SIZE;
}

/*
 * Not all glibc versions still in common use provide a wrapper for
 * memfd_create(), so we define our own here.
 */
static inline int
_memfd_create(const char *name, unsigned int flags)
{
	return syscall(__NR_memfd_create, name, flags);
}

void bootup_lkl(struct ulvisor_boot_info *bi);

void run_app_thread(ulvisor_t *ulvisor);
void lkl_console_add(void);

void
ulvisor_run(ulvisor_t *ulvisor)
{
	struct ulvisor_boot_info *bi = (struct ulvisor_boot_info *)(ulvisor->mem + ULVISOR_BOOT_INFO_BASE);
	bootup_lkl(bi);
	lkl_console_add();

	run_app_thread(ulvisor);
}

static int
handle_cmdarg(char *cmdarg)
{
	if (!strncmp("--x-exec-heap", cmdarg, 13)) {
		warnx("WARNING: The use of --x-exec-heap is dangerous and not"
		      " recommended as it makes the heap and stack executable.");
		use_exec_heap = true;
		return 0;
	}
	return -1;
}

static char *
usage(void)
{
	return "--x-exec-heap (make the heap executable)."
		" WARNING: This option is dangerous and not recommended as it"
		" makes the heap and stack executable.";
}

DECLARE_MODULE(core,
	       .setup = NULL,
	       .handle_cmdarg = handle_cmdarg,
	       .usage = usage
)
