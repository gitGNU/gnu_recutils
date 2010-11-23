#!/bin/sh
#
# recset.sh - System tests for recset.
#
# Copyright (C) 2010 Jose E. Marchesi.
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
. $srcdir/testutils.sh
test_init "recset"

#
# Create input files.
#

test_declare_input_file one-record \
'field1: value1
field2: value2
field3: value3
'

test_declare_input_file repeated-fields \
'field1: value1
field2: value21
field2: value22
field3: value3
'

test_declare_input_file integrity \
'%rec: Integrity
%key: Id

Id: 10
other: field
'

#
# Declare tests.
#

test_tool recset-append-field ok \
          recset \
          '-n 0 -f foo -a bar' \
          one-record \
'field1: value1
field2: value2
field3: value3
foo: bar
'

test_tool recset-set-field ok \
          recset \
          '-n 0 -f field2 -s bar' \
          one-record \
'field1: value1
field2: bar
field3: value3
'

test_tool recset-delete-field ok \
          recset \
          '-n 0 -f field2 -d' \
          one-record \
'field1: value1
field3: value3
'

test_tool recset-comment-out-field ok \
          recset \
          '-n 0 -f field2 -c' \
          one-record \
'field1: value1
#field2: value2
field3: value3
'

test_tool recset-delete-non-existant ok \
          recset \
          '-n 0 -f nonexistant -d' \
          one-record \
'field1: value1
field2: value2
field3: value3
'

test_tool recset-comment-out-fex-first ok \
          recset \
          '-n 0 -f field2[0] -c' \
          repeated-fields \
'field1: value1
#field2: value21
field2: value22
field3: value3
'

test_tool recset-comment-out-fex-last ok \
          recset \
          '-n 0 -f field2[1] -c' \
          repeated-fields \
'field1: value1
field2: value21
#field2: value22
field3: value3
'

test_tool recset-violate-integrity xfail \
          recset \
          '-n 0 -f Id -d' \
          integrity

test_tool recset-force-integrity ok \
          recset \
          '--force -n 0 -f Id -d' \
          integrity \
'%rec: Integrity
%key: Id

other: field
'

#
# Cleanup.
#

test_cleanup
exit $?

# End of recset.sh
