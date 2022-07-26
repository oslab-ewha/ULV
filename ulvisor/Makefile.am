CLEANFILES = *~
DISTCLEANFILES = *~

SUBDIRS = libuc

noinst_LIBRARIES = libulvisor.a

libulvisor_a_SOURCES = ulvisor_net.c ulvisor_block.c \
		ulvisor_hostops.c ulvisor_pure_thread.c ulvisor_pure_thread.h \
		ulvisor_clockevent.c ulvisor_iomem.c uivisor_iomem.h \
		ulvisor_virtio.c ulvisor_virtio_net.c ulvisor_virtio_blk.c ulvisor_virtio_cons.c \
		tap_attach.c block_attach.c \
		ulvisor_syscall.c ulvisor_thread.c ulvisor_longjmp.S \
		ulvisor.h block_attach.h tap_attach.h ulvisor_cc.h ulvisor_elf.h ulvisor_thread.h
