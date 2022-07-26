SUBDIRS = libulv micro_benchmarks

prep: musl/lib/libc.a

musl/lib/libc.a: musl/config.mak
	cd musl && make

musl/config.mak:
	cd musl && ./configure --disable-shared --target=x86_64-ulv

