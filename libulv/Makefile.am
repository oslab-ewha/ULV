CLEANFILES = *~
DISTCLEANFILES = *~

noinst_LIBRARIES = libulv.a libulv_dummy.a

libulv_a_SOURCES = ulv_init.c ulv_thread.c ulv_thread.h ulv_longjmp.S ulv_clone.S ulv_types.h ulv_assert.h \
		ulv_epoller.c ulv_worker.c ulv_worker.h ulv_atomic.h ulv_fd_table.c ulv_fd_table.h \
		ulv_syscall.c ulv_syscall.h ulv_syscall_no.h ulv_host_syscall.h ulv_syscall_flags.h \
		ulv_syscall_open.c ulv_syscall_read.c ulv_syscall_write.c ulv_syscall_uname.c \
		ulv_syscall_exit.c ulv_syscall_ioctl.c ulv_syscall_socket.c \
		ulv_syscall_mmap.c ulv_syscall_set_tid_address.c \
		ulv_syscall_futex.c

libulv_a_CFLAGS = -I../lwip/src/include -I../lwip-build -I../lwip/contrib/ports/unix/port/include

libulv_dummy_a_SOURCES = ulv_dummy.c
