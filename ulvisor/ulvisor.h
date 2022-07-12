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

#ifndef _LKL_H_
#define _LKL_H_

#include <sys/types.h>
#include <inttypes.h>
#include <err.h>

#define ULVISOR_ABI_VERSION		1
#define ULVISOR_GUEST_MIN_BASE	0x100000
#define ULVISOR_HOST_MEM_BASE	0x10000
#define ULVISOR_INTERNAL_TIMERFD	(~1U)
#define ULVISOR_BOOT_INFO_BASE	(ULVISOR_HOST_MEM_BASE + 0x1000)
#define ULVISOR_CMDLINE_SIZE	8192

struct ulvisor_boot_info {
	uint64_t mem_size;                  /* Memory size in bytes */
	uint64_t kernel_end;                /* Address of end of kernel */
	const char *cmdline;                /* Address of command line (C string) */
	const void *mft;                    /* Address of application manifest */
	int epollfd;                        /* epoll() set for yield() */
	int timerfd;                        /* internal timerfd for yield() */
};

typedef struct ulvisor {
	uint8_t	*mem;
	size_t	mem_size;
	struct	ulvisor_boot_info *bi;
	int	epollfd;
	int	timerfd;
	int	argc;
	char	**argv;
	void	*start_fn;
	void	*sc_ctx;
} ulvisor_t;

ulvisor_t *ulvisor_init(size_t mem_size, int argc, char **argv);

int ulvisor_guest_mprotect(void *t_arg, uint64_t addr_start, uint64_t addr_end, int prot);

void ulvisor_boot_info_init(ulvisor_t *ulvisor, uint64_t p_end, int cmdline_argc, char **cmdline_argv);

void ulvisor_run(ulvisor_t *ulvisor);

/*
 * Operations provided by a module. (setup) is required, all other functions
 * are optional.
 */
struct ulvisor_module_ops {
	int (*setup)(ulvisor_t *ulvisor);
	int (*handle_cmdarg)(char *cmdarg);
	char *(*usage)(void);
};

struct ulvisor_module {
	const char	name[32];
	struct ulvisor_module_ops	ops;
};

/*
 * Declare the module (module_name).
 *
 * Usage:
 *
 * DECLARE_MODULE(module_name, <initializer of struct ulvisor_module_ops>);
 *
 * Note that alignment of the struct is explicitly set, otherwise the linker
 * will pick a default that does not match the compiler's alignment.
 */
#define DECLARE_MODULE(module_name, ...) \
    static struct ulvisor_module __module_ ##module_name \
    __attribute((section("modules"), aligned(8))) \
    __attribute((used)) = { \
	.name = #module_name, \
	.ops = { __VA_ARGS__ } \
    };

#endif
