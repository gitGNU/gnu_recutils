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

test_declare_input_file repeated-fields \
'field1: value11
field2: value121
field2: value122
field3: value13

field1: value21
field2: value221
field2: value222
field3: value23
'

test_declare_input_file multiple-types \
'%rec: type1

field1: value11
field2: value12
field3: value13

%rec: type2

field1: value21
field2: value22
field3: value23

%rec: type3

field1: value31
field2: value32
field3: value33
'

test_declare_input_file integer-fields \
'field1: 314

field1: 10

field1: -10

field1: 0
'

test_declare_input_file real-fields \
'field1: 3.14

field1: 10.0

field1: -10.0

field1: 0
'

test_declare_input_file compound-names \
'%rec: Hacker

Name: John Smith
Email: john@smith.org

Name: Jose E. Marchesi
Email: jemarch@gnu.org

%rec: Task

Id: 1
Summary: This is task 1
Hacker:OpenedBy: John Smith

Id: 2
Summary: This is task 2
Hacker:OpenedBy: Jose E. Marchesi
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

# Print a count of all the records.
test_tool recsel-count \
          recsel \
          '-c' \
          multiple-records \
'3
'

# Subscripts.
test_tool recsel-subs \
          recsel \
          '-p field2[0]' \
          repeated-fields \
'field2: value121

field2: value221
'

test_tool recsel-subs-2 \
          recsel \
          '-p field2[1]' \
          repeated-fields \
'field2: value122

field2: value222
'

# Print records identified by its position into the record set.

test_tool recsel-index \
          recsel \
          '-n 0' \
          multiple-records \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-index \
          recsel \
          '-n 1' \
          multiple-records \
'field1: value21
field2: value22
field3: value23
'

# Print records of several types.

test_tool recsel-type \
          recsel \
          '-t type1' \
          multiple-types \
'field1: value11
field2: value12
field3: value13
'

test_tool recsel-type-2 \
          recsel \
          '-t type2' \
          multiple-types \
'field1: value21
field2: value22
field3: value23
'

test_tool recsel-type-3 \
          recsel \
          '-t type2' \
          multiple-types \
'field1: value21
field2: value22
field3: value23
'

test_tool recsel-type-4 \
          recsel \
          '-t type3' \
          multiple-types \
'field1: value31
field2: value32
field3: value33
'

# Selection expressions.

test_tool recsel-sex-field-names \
          recsel \
          '-t Task -e "OpenedBy = '\''John Smith'\''"' \
          compound-names \
'Id: 1
Summary: This is task 1
Hacker:OpenedBy: John Smith
'

test_tool recsel-sex-field-names-2 \
          recsel \
          '-t Task -e "Hacker:OpenedBy: = '\''Jose E. Marchesi'\''"' \
          compound-names \
'Id: 2
Summary: This is task 2
Hacker:OpenedBy: Jose E. Marchesi
'

test_tool recsel-sex-integer-equal \
          recsel \
          '-e "field1 = 0"' \
          integer-fields \
'field1: 0
'

test_tool recsel-sex-integer-nonequal \
          recsel \
          '-e "field1 != 314"' \
          integer-fields \
'field1: 10

field1: -10

field1: 0
'

test_tool recsel-sex-integer-lessthan \
          recsel \
          '-e "field1 < -5"' \
          integer-fields \
'field1: -10
'

test_tool recsel-sex-integer-biggerthan \
          recsel \
          '-e "field1 > 10"' \
          integer-fields \
'field1: 314
'

test_tool recsel-sex-integer-plus \
          recsel \
          '-e "field1 + 2 = 316"' \
          integer-fields \
'field1: 314
'

test_tool recsel-sex-integer-minus \
          recsel \
          '-e "field1 - 2 = -12"' \
          integer-fields \
'field1: -10
'

test_tool recsel-sex-integer-mul \
          recsel \
          '-e "field1 * 20 = 200"' \
          integer-fields \
'field1: 10
'

test_tool recsel-sex-integer-div \
          recsel \
          '-e "field1 / 2 = 5"' \
          integer-fields \
'field1: 10
'

test_tool recsel-sex-integer-mod \
          recsel \
          '-e "field1 % 313 = 1"' \
          integer-fields \
'field1: 314
'

test_tool recsel-sex-integer-not \
          recsel \
          '-e "!field1"' \
          integer-fields \
'field1: 0
'

test_tool recsel-sex-integer-and \
          recsel \
          '-e "field1 && field1"' \
          integer-fields \
'field1: 314

field1: 10

field1: -10
'

test_tool recsel-sex-integer-or \
          recsel \
          '-e "field1 || 1"' \
          integer-fields \
'field1: 314

field1: 10

field1: -10

field1: 0
'

test_tool recsel-sex-real-equal \
          recsel \
          '-e "field1 = 3.14"' \
          real-fields \
'field1: 3.14
'

test_tool recsel-sex-real-nonequal \
          recsel \
          '-e "field1 != 3.14"' \
          real-fields \
'field1: 10.0

field1: -10.0

field1: 0
'

test_tool recsel-sex-real-lessthan \
          recsel \
          '-e "field1 < -5.2"' \
          real-fields \
'field1: -10.0
'

test_tool recsel-sex-real-biggerthan \
          recsel \
          '-e "field1 > 3.14"' \
          real-fields \
'field1: 10.0
'

test_tool recsel-sex-real-plus \
          recsel \
          '-e "((field1 + 2) > 5.14) && ((field + 2) < 5.15)"' \
          real-fields \
'field1: 3.14
'

test_tool recsel-sex-real-minus \
          recsel \
          '-e "((field1 - 2.0) > -12.0)"' \
          real-fields \
'field1: -10.0
'

test_tool recsel-sex-real-mul \
          recsel \
          '-e "field1 * 20.0 = 200.0"' \
          real-fields \
'field1: 10.0
'

#
# Cleanup
#

test_cleanup
exit $?

# End of recsel.sh
