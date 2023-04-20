noinst_PROGRAMS = test_dyntab test_ulfs_inode

test_dyntab_SOURCES = test_dyntab.c ulv_test.c ulv_test.h
test_dyntab_CFLAGS = -I../libulv
test_dyntab_LDADD = ../libulv/libulv.a

test_ulfs_inode_SOURCES = test_ulfs_inode.c ../tools/ulfs/ulfs_tool_mkfs.c ../tools/ulfs/ulfs_tool_info.c \
			ulv_test.c ulv_test.h
test_ulfs_inode_CFLAGS = -I../libulv -I../libulfs
test_ulfs_inode_LDADD = ../libulfs/libulfs.a ../libulv/libulv.a
