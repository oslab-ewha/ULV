SUBDIRS = libulv micro_benchmarks

prep: musl/lib/libc.a lwip-build/liblwip.a

musl/lib/libc.a: musl/config.mak
	cd musl && make

musl/config.mak:
	cd musl && ./configure --disable-shared --target=x86_64-ulv

lwip-build/liblwip.a: lwip-build/CMakeFiles
	cd lwip-build && make

lwip-build/CMakeFiles: lwip-build/CMakeLists.txt
	cd lwip-build && cmake
