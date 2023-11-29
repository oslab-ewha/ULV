SUBDIRS = gdbm_lite libulv libulfs micro_benchmarks macro_benchmarks tools/ulfs tests

prep: musl/lib/libc.a lwip-build/liblwip.a

musl/lib/libc.a: musl/config.mak
	cd musl && make

musl/config.mak:
	cd musl && ./configure --disable-shared --target=x86_64-ulv

lwip-build/liblwip.a: lwip-build/CMakeFiles/lwip.dir
	cd lwip-build && make

lwip-build/CMakeFiles/lwip.dir: lwip-build/CMakeLists.txt
	cd lwip-build && cmake .
