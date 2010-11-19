#!/bin/sh 
#
# recsel.sh - System tests for recsel.
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

. testutils.sh
test_init "recsel"

#
# Create input files.
#

test_declare_input_file empty-file ''

test_declare_input_file one-record \
'field1: value1
field2: value2
field3: value3
'

test_declare_input_file multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

test_declare_input_file multiline \
'field1: foo bar \
baz
field2: foo
+ bar
+ baz

field1: jo ja \
ju
field2:
+ foo
+ bar
'

#
# Declare tests
#

# Select the whole record set.
test_tool recsel-all-fields \
          recsel \
          '' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

field1: value21
field2: value22
field3: value23

field1: value31
field2: value32
field3: value33
'

# Print just one field. 
test_tool recsel-one-field \
          recsel \
          '-p field2' \
          multiple-records \
'field2: value12

field2: value22

field2: value32
'

# Print two fields.
test_tool recsel-two-fields \
          recsel \
          '-p field2,field3' \
          multiple-records \
'field2: value12
field3: value13

field2: value22
field3: value23

field2: value32
field3: value33
'

# Print values
test_tool recsel-values \
          recsel \
          '-P field2' \
          multiple-records \
'value12

value22

value32
'

# Print multiple values
test_tool recsel-multiple-values \
          recsel \
          '-P field1,field3' \
          multiple-records \
'value11
value13

value21
value23

value31
value33
'

# Print collapsed
test_tool recsel-collapsed \
          recsel \
          '-C -p field1' \
          multiple-records \
'field1: value11
field1: value21
field1: value31
'

# Print values collapsed
test_tool recsel-values-collapsed \
          recsel \
          '-C -P field2,field3' \
          multiple-records \
'value12
value13
value22
value23
value32
value33
'

# Print in a row
test_tool recsel-row \
          recsel \
          '-R field1,field2' \
          multiple-records \
'value11 value12

value21 value22

value31 value32
'

# Print collapsed in a row
test_tool recsel-collapsed-row \
          recsel \
          '-C -R field1,field2,field3' \
          multiple-records \
'value11 value12 value13
value21 value22 value23
value31 value32 value33
'

# Print all fields (multiline)
test_tool recsel-multi-all-fields \
          recsel \
          '' \
          multiline \
'field1: foo bar baz
field2: foo
+ bar
+ baz

field1: jo ja ju
field2: 
+ foo
+ bar
'

# Print values (multiline)
test_tool recsel-multi-values \
          recsel \
          '-P field1,field2' \
          multiline \
'foo bar baz
foo
bar
baz

jo ja ju

foo
bar
'
# Print collapsed values (multiline)
test_tool recsel-multi-collapsed-values \
          recsel \
          '-C -P field1,field2' \
          multiline \
'foo bar baz
foo
bar
baz
jo ja ju

foo
bar
'

#
# Cleanup
#

test_cleanup
exit $?

# End of recsel.sh
