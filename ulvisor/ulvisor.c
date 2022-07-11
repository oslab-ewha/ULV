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
#include <fcntl.h>
#include <libgen.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "ulvisor_elf.h"

#include "ulvisor.h"

extern struct ulvisor_module __start_modules;
extern struct ulvisor_module __stop_modules;

static const char	*prog;
static const char	*elf_filename;
static size_t	mem_size = 0x20000000;

static void
setup_modules(ulvisor_t *ulvisor)
{
	for (struct ulvisor_module *m = &__start_modules; m < &__stop_modules; m++) {
		if (m->ops.setup == NULL)
			continue;
		if (m->ops.setup(ulvisor)) {
			warnx("Module `%s' setup failed", m->name);
			if (m->ops.usage) {
				warnx("Please check you have correctly specified:\n    %s",
				      m->ops.usage());
			}
			exit(1);
		}
	}
}

static int
handle_cmdarg(char *cmdarg)
{
	for (struct ulvisor_module *m = &__start_modules; m < &__stop_modules; m++) {
		if (m->ops.handle_cmdarg) {
			if (m->ops.handle_cmdarg(cmdarg) == 0) {
				return 0;
			}
		}
	}
	return -1;
}

static void
handle_mem(char *cmdarg, size_t *mem_size)
{
	size_t	mem;
	int	rc;

	rc = sscanf(cmdarg, "--mem=%zd", &mem);
	mem = mem << 20;
	if (rc != 1 || mem <= 0) {
		errx(1, "Malformed argument to --mem");
	}
	*mem_size = mem;
}

static void
usage(const char *prog)
{
	fprintf(stderr, "usage: %s [ CORE OPTIONS ] [ -- ] "
		"KERNEL [ ARGS ]\n", prog);
	fprintf(stderr, "KERNEL is the filename of the unikernel to run.\n");
	fprintf(stderr, "ARGS are optional arguments passed to the unikernel.\n");
	fprintf(stderr, "Core options:\n");
	fprintf(stderr, "  [ --mem=512 ] (guest memory in MB)\n");
	fprintf(stderr, "    --help (display this help)\n");
	fprintf(stderr, "Compiled-in modules: ");
	for (struct ulvisor_module *m = &__start_modules; m < &__stop_modules; m++) {
		assert(m->name);
		fprintf(stderr, "%s ", m->name);
	}
	fprintf(stderr, "\n");
	fprintf(stderr, "Compiled-in module options:\n");

	int	nm = 0;
	for (struct ulvisor_module *m = &__start_modules; m < &__stop_modules; m++) {
		if (m->ops.usage) {
			fprintf(stderr, "    %s\n", m->ops.usage());
			nm++;
		}
	}
	if (nm == 0)
		fprintf(stderr, "    (none)\n");
}

static int
parse_args(int argc, char *argv[])
{
	int	argc_org = argc;

	/*
	 * Scan command line arguments, looking for the first non-option argument
	 * which will be the ELF file to load. Stop if a "terminal" option such as
	 * --help is encountered.
	 */
	while (*argv && *argv[0] == '-') {
		if (strcmp("--", *argv) == 0) {
			/* Consume and stop option processing */
			argc--;
			argv++;
			break;
		}

		if (strcmp("--help", *argv) == 0) {
			usage(prog);
			exit(0);
		}
		else if (strncmp("--mem=", *argv, 6) == 0)
			handle_mem(*argv, &mem_size);
		else if (handle_cmdarg(*argv) < 0) {
			warnx("Invalid option: `%s'", *argv);
			usage(prog);
			exit(1);
		}
		
		argc--;
		argv++;
	}
	if (*argv == NULL) {
		warnx("Missing KERNEL operand");
		usage(prog);
		exit(1);
	}
	elf_filename = *argv;

	return (argc_org - argc);
}

int
main(int argc, char *argv[])
{
	uint64_t	p_entry, p_end;
	int	n_parsed;
	int	elf_fd = -1;

	prog = basename(*argv);
	argc--;
	argv++;

	n_parsed = parse_args(argc, argv);

	argc -= n_parsed;
	argv += n_parsed;

	/*
	 * Now that we have the ELF file name, verify that is binary is
	 * ABI-compatible and load the MFT1 NOTE from it, as subsequent parsing of
	 * the command line in the 2nd pass depends on the application-supplied
	 * manifest.
	 */
	elf_fd = open(elf_filename, O_RDONLY);
	if (elf_fd == -1)
		err(1, "%s: Could not open", elf_filename);

	assert(elf_filename == *argv);

	ulvisor_t	*ulvisor = ulvisor_init(mem_size, argc, argv);

	elf_load(elf_fd, elf_filename, ulvisor->mem, ulvisor->mem_size, ULVISOR_GUEST_MIN_BASE, ulvisor_guest_mprotect, ulvisor, &p_entry, &p_end);
	close(elf_fd);

	setup_modules(ulvisor);

	ulvisor_boot_info_init(ulvisor, p_end, argc, argv);

	ulvisor->start_fn = ulvisor->mem + p_entry;
	ulvisor_run(ulvisor);
}
