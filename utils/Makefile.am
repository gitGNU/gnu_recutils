# utils/ Makefile.am
# GNU rec

# Copyright (C) 2009, 2010 Jose E. Marchesi

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
COMMONLIBS = $(top_builddir)/lib/libgnu.la $(top_builddir)/src/librec.la

AM_CPPFLAGS = -I$(top_srcdir)/lib -I$(top_srcdir)/src

recinf_SOURCES = $(COMMONSRC) \
                 recinf.c
recinf_LDADD = $(COMMONLIBS)

recsel_SOURCES = $(COMMONSRC) \
                 recsel.c
recsel_LDADD = $(COMMONLIBS)

recins_SOURCES = $(COMMONSRC) \
                 recins.c
recins_LDADD = $(COMMONLIBS)

recdel_SOURCES = $(COMMONSRC) \
                 recdel.c
recdel_LDADD = $(COMMONLIBS)

recset_SOURCES = $(COMMONSRC) \
                 recset.c
recset_LDADD = $(COMMONLIBS)

recfix_SOURCES = $(COMMONSRC) \
                 recfix.c
recfix_LDADD = $(COMMONLIBS)

# End of Makefile.am
