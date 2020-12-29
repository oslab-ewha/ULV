BENCHMARKS := micro_benchmarks/syscall_test.spt macro_benchmarks/gdbm_test.spt macro_benchmarks/wget_test.spt

all: solo5/tenders/lkl/solo5-lkl $(BENCHMARKS)

musl/lib/libc.a: musl/config.mak
	cd musl && make

musl/config.mak:
	cd musl && ./configure

lkl/tools/lkl/liblkl.a:
	cd lkl/tools/lkl && make

solo5/tenders/lkl/solo5-lkl: solo5/GNUmakefile lkl/tools/lkl/liblkl.a
	cd solo5 && make

solo5/GNUmakefile:
	cd solo5 && ./configure.sh

micro_benchmarks/syscall_test.spt: musl/lib/libc.a
	cd micro_benchmarks && make

macro_benchmarks/gdbm_test.spt: musl/lib/libc.a
	cd macro_benchmarks && make

macro_benchmarks/wget_test.spt: musl/lib/libc.a macro_benchmarks/wget/src/libwget.a macro_benchmarks/wget/lib/libgnu.a
	cd macro_benchmarks && make

macro_benchmarks/wget/src/libwget.a macro_benchmarks/wget/lib/libgnu.a: macro_benchmarks/wget/Makefile
	cd macro_benchmarks/wget && make

macro_benchmarks/wget/Makefile:
	cd macro_benchmarks/wget && ./configure --without-ssl --without-zlib --disable-pcre2
