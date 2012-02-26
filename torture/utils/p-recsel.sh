#!/bin/sh
#
# p-recsel.sh - Performance tests for recsel.
#
# Copyright (C) 2012 Jose E. Marchesi.
#
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

#
# Initialization
#

: ${srcdir=.}
: ${crypt_support=yes}

. $srcdir/testutils.sh
test_init "p-recsel"

#
# Create input files.
#

test_gen_input_file 200-records 200 10
test_gen_input_file 500-records 500 10
test_gen_input_file 1000-records 1000 10
test_gen_input_file 10000-records 10000 10
test_gen_input_file 20000-records 20000 10

#
# Tests.
#

test_tool recsel-200-records perf \
          recsel \
          "" \
          200-records

test_tool recsel-500-records perf \
          recsel \
          "" \
          500-records

test_tool recsel-1000-records perf \
          recsel \
          "" \
          1000-records

test_tool recsel-10000-records perf \
          recsel \
          "" \
          10000-records

test_tool recsel-20000-records perf \
          recsel \
          "" \
          20000-records

test_tool recsel-200-records-sex-regexp perf \
          recsel \
          "-e 'field4 ~ \".*04.*\"'" \
          200-records

test_tool recsel-500-records-sex-regexp perf \
          recsel \
          "-e 'field4 ~ \".*04.*\"'" \
          500-records

test_tool recsel-1000-records-sex-regexp perf \
          recsel \
          "-e 'field4 ~ \".*04.*\"'" \
          1000-records

test_tool recsel-10000-records-sex-regexp perf \
          recsel \
          "-e 'field4 ~ \".*04.*\"'" \
          10000-records

test_tool recsel-20000-records-sex-regexp perf \
          recsel \
          "-e 'field4 ~ \".*04.*\"'" \
          20000-records

#
# Cleanup
#

test_cleanup
exit $?

# End of p-recsel.sh
