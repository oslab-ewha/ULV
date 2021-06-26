# This file is part of GDBM.                                   -*- Makefile -*-
# Copyright (C) 2007, 2011, 2013, 2016-2020 Free Software Foundation,
# Inc.
#
# GDBM is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GDBM is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GDBM. If not, see <http://www.gnu.org/licenses/>. */

EXTRA_DIST = build-aux/config.rpath NOTE-WARNING git2chg.awk

if COMPAT_OPT
  MAYBE_COMPAT = compat
endif
SUBDIRS = po src doc $(MAYBE_COMPAT) tests

AM_DISTCHECK_CONFIGURE_FLAGS = --enable-libgdbm-compat

.PHONY: set-dist-date
set-dist-date:
	rm -f $(distdir)/src/version.c; \
	d=`date '+%d/%m/%Y'`; \
	sed 's|/\*@DIST_DATE@\*/|"'"$$d"'"|' $(srcdir)/src/version.c > \
		$(distdir)/src/version.c

dist-hook: ChangeLog set-dist-date
	@if test -f ChangeLog && test -f NEWS; then			\
	  PATCHLEV=`echo "$(PACKAGE_VERSION)" | sed -r "s/[0-9]+\.[0-9]+\.?//"`;\
	  if test $${PATCHLEV:-0} -lt 50; then				     \
	    if grep -q FIXME NEWS; then					     \
	      echo >&2 "*** NEWS file contains FIXMEs";			     \
	      echo >&2 "*** Aborting";					     \
	      exit 1;							     \
	    fi;								     \
	  fi;                                                                \
	fi


gen_start_date = 2016-07-08
prev_change_log = ChangeLog.cvs

.PHONY: ChangeLog
ChangeLog:
	$(AM_V_GEN)if test -d .git; then                                    \
                git log --pretty='format:%ct  %an  <%ae>%n%n%s%n%n%b%n'     \
                        --since=$(gen_start_date) |                         \
                awk -f $(top_srcdir)/git2chg.awk                            \
                    -v append=$(top_srcdir)/$(prev_change_log) > ChangeLog; \
        fi
