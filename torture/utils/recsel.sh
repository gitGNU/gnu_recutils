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

#
# Cleanup
#

test_cleanup
exit $?

# End of recsel.sh
