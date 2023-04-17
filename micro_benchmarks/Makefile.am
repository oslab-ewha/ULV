CLEANFILES = *~
DISTCLEANFILES = *~

MICRO_BENCHMARKS = mb_thread mb_file_rw mb_file_dir mb_syscall mb_network
noinst_LIBRARIES = libmb.a
noinst_PROGRAMS = $(MICRO_BENCHMARKS)

CFLAGS = -g
libulv = -Wl,--whole-archive ../libulv/libulv.a -Wl,--no-whole-archive
libulfs = -Wl,--whole-archive ../libulfs/libulfs.a ../libulv/libulv.a -Wl,--no-whole-archive
liblwip = -Wl,--whole-archive ../libulv/libulv_lwip.a -Wl,--no-whole-archive ../lwip-build/liblwip.a -Wl,--whole-archive ../libulv/libulv.a -Wl,--no-whole-archive
common_LINK = musl-gcc -static -nolibc -o $@

mb_thread_SOURCES = thread_test.c
mb_thread_LDADD = ../musl/lib/libc.a
mb_thread_LINK = $(common_LINK) $(libulv)

mb_file_rw_SOURCES = file_rw_test.c
mb_file_rw_LDADD = libmb.a ../musl/lib/libc.a
mb_file_rw_LINK = $(common_LINK) $(libulfs)

mb_file_dir_SOURCES = file_dir_test.c
mb_file_dir_LDADD = libmb.a ../musl/lib/libc.a
mb_file_dir_LINK = $(common_LINK) $(libulfs)

mb_syscall_SOURCES = syscall_test.c
mb_syscall_LDADD = libmb.a ../musl/lib/libc.a
mb_syscall_LINK = $(common_LINK) $(libulv)

mb_network_SOURCES = network_test.c
mb_network_LDADD = libmb.a ../musl/lib/libc.a
mb_network_LINK = $(common_LINK) $(liblwip)

libmb_a_SOURCES = libmb_stdio.c libmb_time.c libmb_net.c

$(MICRO_BENCHMARKS): ../libulv/libulv.a ../libulfs/libulfs.a
