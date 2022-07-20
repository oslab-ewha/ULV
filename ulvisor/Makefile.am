CLEANFILES = *~
DISTCLEANFILES = *~

bin_PROGRAMS = ulvisor

ulvisor_SOURCES = ulvisor.c ulvisor_core.c ulvisor_net.c ulvisor_block.c ulvisor_elf.c \
		ulvisor_launch.S ulvisor_hostops.c ulvisor_pure_thread.c ulvisor_pure_thread.h \
		ulvisor_clockevent.c ulvisor_iomem.c uivisor_iomem.h \
		ulvisor_virtio.c ulvisor_virtio_net.c ulvisor_virtio_blk.c ulvisor_virtio_cons.c \
		tap_attach.c block_attach.c \
		ulvisor_kernel.c ulvisor_app.c ulvisor_thread.c ulvisor_longjmp.S \
		ulvisor.h block_attach.h tap_attach.h ulvisor_cc.h ulvisor_elf.h ulvisor_thread.h

ulvisor_LDADD = ../lkl/tools/lkl/liblkl.a -lseccomp -lpthread -lrt
