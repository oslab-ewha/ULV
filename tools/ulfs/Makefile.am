CLEANFILES = *~
DISTCLEANFILES = *~

noinst_PROGRAMS = ulfs

AM_CPPFLAGS = -I../../libulv -I../../libulfs

ulfs_SOURCES = ulfs_tool_main.c ulfs_tool_mkfs.c ulfs_tool_ls.c ulfs_tool_copy.c
ulfs_LDADD = ../../libulfs/libulfs.a
