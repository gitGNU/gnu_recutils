# torture/ Makefile.am
# GNU rec library

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

if CHECK

TESTS = runtests
noinst_PROGRAMS = runtests

REC_MSET_TSUITE = rec-mset/rec-mset-new.c \
                  rec-mset/rec-mset-dup.c \
                  rec-mset/rec-mset-type-p.c \
                  rec-mset/rec-mset-register-type.c \
                  rec-mset/tsuite-rec-mset.c

REC_FIELD_TSUITE = rec-field/rec-field-name.c \
                  rec-field/rec-field-set-name.c \
                  rec-field/rec-field-value.c \
                  rec-field/rec-field-set-value.c \
                  rec-field/rec-field-dup.c \
                  rec-field/tsuite-rec-field.c

REC_RECORD_TSUITE = rec-record/tsuite-rec-record.c

runtests_SOURCES = runtests.c \
                   $(REC_MSET_TSUITE) \
                   $(REC_FIELD_TSUITE) \
                   $(REC_RECORD_TSUITE)

AM_CPPFLAGS = -I$(top_srcdir)/src
runtests_LDADD = $(CHECK_LIBS) ../src/librec.la

clean-local:
	-rm -f tests.log

endif # CHECK

# End of Makefile.am
