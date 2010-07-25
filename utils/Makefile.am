# utils/ Makefile.am
# GNU rec

# Copyright (C) 2009 Jose E. Marchesi

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

bin_PROGRAMS = recinf recsel recins recdel recset recfix

COMMONSRC = recutl.h recutl.c

recinf_SOURCES = $(COMMONSRC) \
                 recinf.c

recinf_LDFLAGS = -L$(top_builddir)/src
recinf_CPPFLAGS = -I$(top_srcdir)/lib
recinf_LDADD = $(top_builddir)/lib/libgnu.la -lrec

recsel_SOURCES = $(COMMONSRC) \
                 recsel.c

recsel_LDFLAGS = -L$(top_builddir)/src
recsel_CPPFLAGS = -I$(top_srcdir)/lib
recsel_LDADD = $(top_builddir)/lib/libgnu.la -lrec

recins_SOURCES = $(COMMONSRC) \
                 recins.c

recins_LDFLAGS = -L$(top_builddir)/src
recins_CPPFLAGS = -I$(top_srcdir)/lib
recins_LDADD = $(top_builddir)/lib/libgnu.la -lrec

recdel_SOURCES = $(COMMONSRC) \
                 recdel.c

recdel_LDFLAGS = -L$(top_builddir)/src
recdel_CPPFLAGS = -I$(top_srcdir)/lib
recdel_LDADD = $(top_builddir)/lib/libgnu.la -lrec

recset_SOURCES = $(COMMONSRC) \
                 recset.c

recset_LDFLAGS = -L$(top_builddir)/src
recset_CPPFLAGS = -I$(top_srcdir)/lib
recset_LDADD = $(top_builddir)/lib/libgnu.la -lrec

recfix_SOURCES = $(COMMONSRC) \
                 recfix.c

recfix_LDFLAGS = -L$(top_builddir)/src
recfix_CPPFLAGS = -I$(top_srcdir)/lib
recfix_LDADD = $(top_builddir)/lib/libgnu.la -lrec

# End of Makefile.am
