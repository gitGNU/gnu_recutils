# guile/ Makefile.am
# GNU recutils

# Copyright (C) 2012 Jose E. Marchesi

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

# TODO: s/2.0/guile_effective
guileextensiondir=$(libdir)/guile/2.0
guilemoduledir = $(GUILE_SITE)

nobase_nodist_guilemodule_DATA = recutils.scm

guileextension_LTLIBRARIES = guile-rec.la
guile_rec_la_SOURCES =  guile-rec.c
guile_rec_la_CPPFLAGS = $(GUILE_CFLAGS)
guile_rec_la_LDFLAGS = $(GUILE_LDFLAGS) -L$(top_builddir)/src -lrec -module

SNARFCPPOPTS = $(DEFS) $(INCLUDES) $(AM_CPPFLAGS) $(CPPFLAGS) \
               $(CFLAGS) $(AM_CFLAGS) $(GUILE_CFLAGS) -I$(top_builddir)/src

.c.x:
	$(GUILE_SNARF) -o $@ $< $(SNARFCPPOPTS)

.in.scm:
	cat "$^" | $(SED) -e's|[@]guileexteniondir[@]|$(guileextensiondir)|g' \
		> "$@.tmp"
	mv "$@.tmp" "$@"

# End of Makefile.am
