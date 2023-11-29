CLEANFILES = *~
DISTCLEANFILES = *~

MACRO_BENCHMARKS = Mb_gdbm
noinst_LIBRARIES = libMb.a
noinst_PROGRAMS = $(MACRO_BENCHMARKS)

GDBM := ../gdbm_lite

LIBGDBM := $(GDBM)/libgdbm.a

CFLAGS = -g
libulv = -Wl,--whole-archive ../libulv/libulv.a -Wl,--no-whole-archive
libulfs = -Wl,--whole-archive ../libulfs/libulfs.a ../libulv/libulv.a -Wl,--no-whole-archive
liblwip = -Wl,--whole-archive ../libulv/libulv_lwip.a -Wl,--no-whole-archive ../lwip-build/liblwip.a -Wl,--whole-archive ../libulv/libulv.a -Wl,--no-whole-archive
common_LINK = musl-gcc -static -nolibc -o $@

Mb_gdbm_SOURCES = Mb_gdbm.c
Mb_gdbm_LDADD = libMb.a $(LIBGDBM) ../musl/lib/libc.a
Mb_gdbm_LINK = $(common_LINK) $(libulfs)

libMb_a_SOURCES = libMb_time.c
