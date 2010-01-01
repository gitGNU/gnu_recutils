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

bin_PROGRAMS = recinf recsel

recinf_SOURCES = recinf.h \
                 recinf.c

recinf_LDFLAGS =
recinf_CPPFLAGS = -I$(top_srcdir)/lib
recinf_LDADD = $(top_builddir)/lib/libgnu.la -lrec

recsel_SOURCES = recsel.h \
                 recsel.c

recsel_LDFLAGS =
recsel_CPPFLAGS = -I$(top_srcdir)/lib
recsel_LDADD = $(top_builddir)/lib/libgnu.la -lrec

# End of Makefile.am
