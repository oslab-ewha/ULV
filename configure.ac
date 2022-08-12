#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([ulv], [0.1.0], [cezanne@codemayo.com])
AC_CONFIG_SRCDIR([libulv/ulv_syscall.c])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign])

AM_SILENT_RULES([yes])

AM_MAINTAINER_MODE([disable])

# Checks for programs.
AC_PROG_CC
AM_PROG_AS
AC_PROG_RANLIB

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T
AC_TYPE_UINT64_T

AC_DEFINE(_GNU_SOURCE, [], [GNU Supported Code])

# Checks for library functions.

AC_CONFIG_FILES([Makefile
                 libulv/Makefile
                 libulfs/Makefile
		 micro_benchmarks/Makefile
		 tools/ulfs/Makefile
		 tests/Makefile
		 ])

AC_OUTPUT
