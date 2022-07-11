SUBDIRS = ulvisor micro_benchmarks

prep: musl/lib/libc.a lkl/tools/lkl/liblkl.a

musl/lib/libc.a: musl/config.mak
	cd musl && make

musl/config.mak:
	cd musl && ./configure

lkl/tools/lkl/liblkl.a:
	cd lkl/tools/lkl && make
