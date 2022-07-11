CLEANFILES = *~
DISTCLEANFILES = *~

noinst_LIBRARIES = libmb.a
noinst_PROGRAMS = mb_syscall mb_network

CFLAGS = -g
common_LINK = musl-gcc -static -nolibc -o $@

mb_syscall_SOURCES = syscall_test.c
mb_syscall_LDADD = libmb.a ../musl/lib/libc.a
mb_syscall_LINK = $(common_LINK)

mb_network_SOURCES = network_test.c
mb_network_LDADD = libmb.a ../musl/lib/libc.a
mb_network_LINK = $(common_LINK)

libmb_a_SOURCES = libmb_stdio.c libmb_time.c libmb_net.c
