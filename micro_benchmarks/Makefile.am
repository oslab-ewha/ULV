CLEANFILES = *~
DISTCLEANFILES = *~

noinst_LIBRARIES = libmb.a
noinst_PROGRAMS = mb_thread mb_file mb_syscall mb_network

CFLAGS = -g
libulv = -Wl,--whole-archive ../libulv/libulv.a -Wl,--no-whole-archive
libulfs = -Wl,--whole-archive ../libulfs/libulfs.a ../libulv/libulv.a -Wl,--no-whole-archive
liblwip = -Wl,--whole-archive ../libulv/libulv_lwip.a -Wl,--no-whole-archive ../lwip-build/liblwip.a -Wl,--whole-archive ../libulv/libulv.a -Wl,--no-whole-archive
common_LINK = musl-gcc -static -nolibc -o $@

mb_thread_SOURCES = thread_test.c
mb_thread_LDADD = ../musl/lib/libc.a
mb_thread_LINK = $(common_LINK) $(libulv)

mb_file_SOURCES = file_test.c
mb_file_LDADD = libmb.a ../musl/lib/libc.a
mb_file_LINK = $(common_LINK) $(libulfs)

mb_syscall_SOURCES = syscall_test.c
mb_syscall_LDADD = libmb.a ../musl/lib/libc.a
mb_syscall_LINK = $(common_LINK) $(libulv)

mb_network_SOURCES = network_test.c
mb_network_LDADD = libmb.a ../musl/lib/libc.a
mb_network_LINK = $(common_LINK) $(liblwip)

libmb_a_SOURCES = libmb_stdio.c libmb_time.c libmb_net.c

$(noinst_PROGRAMS): ../libulv/libulv.a
