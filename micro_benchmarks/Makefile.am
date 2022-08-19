CLEANFILES = *~
DISTCLEANFILES = *~

noinst_LIBRARIES = libmb.a
noinst_PROGRAMS = mb_thread mb_file mb_syscall mb_network

CFLAGS = -g
whole_libulv = -Wl,--whole-archive ../libulv/libulv.a -Wl,--no-whole-archive
common_LINK = musl-gcc -static -nolibc -o $@ $(whole_libulv)

mb_thread_SOURCES = thread_test.c
mb_thread_LDADD = ../libulv/libulv_dummy.a ../musl/lib/libc.a
mb_thread_LINK = $(common_LINK)

mb_file_SOURCES = file_test.c
mb_file_LDADD = libmb.a ../libulfs/libulfs.a ../libulv/libulv_dummy.a ../musl/lib/libc.a
mb_file_LINK = $(common_LINK)

mb_syscall_SOURCES = syscall_test.c
mb_syscall_LDADD = libmb.a ../libulv/libulv_dummy.a ../musl/lib/libc.a
mb_syscall_LINK = $(common_LINK)

mb_network_SOURCES = network_test.c
mb_network_LDADD = libmb.a ../lwip-build/liblwip.a ../libulv/libulv_dummy.a ../musl/lib/libc.a
mb_network_LINK = $(common_LINK)

libmb_a_SOURCES = libmb_stdio.c libmb_time.c libmb_net.c

$(noinst_PROGRAMS): ../libulv/libulv.a
