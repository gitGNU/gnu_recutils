# src/ Makefile.am
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

lib_LTLIBRARIES = librec.la

include_HEADERS = rec.h

librec_la_SOURCES = rec-field.c \
                    rec-record.c \
                    rec-rset.c

librec_la_LDFLAGS =
librec_la_CPPFLAGS = -I$(top_srcdir)/lib
librec_la_LIBADD = $(top_builddir)/lib/libgnu.la

# End of Makefile.am
