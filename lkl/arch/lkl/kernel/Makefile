extra-y := vmlinux.lds

ccflags-$(CONFIG_LKL_DEBUG) := -DLKL_DEBUG

obj-y = setup.o threads.o irq.o time.o syscalls.o misc.o console.o \
	syscalls_32.o cpu.o

obj-$(CONFIG_LKL_DEBUG) += dbg.o
