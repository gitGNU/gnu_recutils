# Makefile.am for recutils/man

# Copyright (C) 2010 Jose E. Marchesi

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

man_MANS = recdel.1 \
           recfix.1 \
           recinf.1 \
           recins.1 \
           recsel.1 \
           recset.1 \
           recfmt.1 \
           csv2rec.1 \
           rec2csv.1

if COMPILE_MDB2REC
man_MANS += mdb2rec.1
endif

EXTRA_DIST = $(man_MANS)
MAINTAINERCLEANFILES = $(man_MANS)

# The man pages depend on the --help strings and the version number.
common_mandeps = $(top_srcdir)/configure.ac \
                 $(top_srcdir)/utils/recutl.c

# Depend on the source, not the binary; we don't need to regenerate
# the binary when any source file changes, only the main one.
# Use -o so that the `missing' program can infer the output file.

rec2csv.1: $(top_srcdir)/utils/rec2csv.c $(common_mandeps)
	$(HELP2MAN) --name="rec to csv converter" ../utils/rec2csv -o $@

csv2rec.1: $(top_srcdir)/utils/csv2rec.c $(common_mandeps)
	$(HELP2MAN) --name="csv to rec converter" ../utils/csv2rec -o $@

mdb2rec.1: $(top_srcdir)/utils/mdb2rec.c $(common_mandeps)
	$(HELP2MAN) --name="mdb to rec converter" ../utils/mdb2rec -o $@

recdel.1: $(top_srcdir)/utils/recdel.c $(common_mandeps)
	$(HELP2MAN) --name="remove records from a recfile" ../utils/recdel -o $@

recfix.1: $(top_srcdir)/utils/recfix.c $(common_mandeps)
	$(HELP2MAN) --name="check a recfile for errors" ../utils/recfix -o $@

recinf.1: $(top_srcdir)/utils/recinf.c $(common_mandeps)
	$(HELP2MAN) --name="print information about a recfile" ../utils/recinf -o $@

recins.1: $(top_srcdir)/utils/recins.c $(common_mandeps)
	$(HELP2MAN) --name="insert records in a recfile" ../utils/recins -o $@

recsel.1: $(top_srcdir)/utils/recsel.c $(common_mandeps)
	$(HELP2MAN) --name="print records from a recfile" ../utils/recsel -o $@

recset.1: $(top_srcdir)/utils/recset.c $(common_mandeps)
	$(HELP2MAN) --name="manipulate fields in a recfile" ../utils/recset -o $@

recfmt.1: $(top_srcdir)/utils/recfmt.c $(common_mandeps)
	$(HELP2MAN) --name="apply a template to records" ../utils/recfmt -o $@

# End of Makefile.am
