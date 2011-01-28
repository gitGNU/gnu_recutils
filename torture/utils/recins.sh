#!/bin/sh
#
# recins.sh - System tests for recins.
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
test_init "recins"

#
# Create input files.
#

test_declare_input_file empty-file \
''

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

test_declare_input_file multiple-named \
'%rec: Type1

field1: value11
field2: value12
field3: value13

%rec: Type2

field1: value21
field2: value22
field3: value23

%rec: Type3

field1: value31
field2: value32
field3: value33
'

test_declare_input_file integrity \
'%rec: Integrity
%type: Id int

Id: 0
'

test_declare_input_file only-descriptor \
'%rec: foo
'

test_declare_input_file comments-and-descriptor \
'# comment 1

# comment 2

%rec: foo
'

test_declare_input_file external-descriptor \
'%rec: Patata external-descriptor-types

foo: 10
'

test_declare_input_file external-descriptor-types \
'%rec: Patata
%type: foo int
' 

#
# Declare tests.
#

test_tool recins-empty ok \
          recins \
          '-f field1 -v "value1"' \
          empty-file \
'field1: value1
'

test_tool recins-empty-with-type ok \
          recins \
          '-t Type1 -f field1 -v "value1"' \
          empty-file \
'%rec: Type1

field1: value1
'

test_tool recins-several-fields ok \
          recins \
          '-f field1 -v "value1" -f field2 -v "value2"' \
          empty-file \
'field1: value1
field2: value2
'

test_tool recins-append ok \
          recins \
          '-f afield1 -v "appended1"' \
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

afield1: appended1
'

test_tool recins-append-in-type ok \
          recins \
          '-t Type2 -f afield1 -v "appended1"' \
          multiple-named \
'%rec: Type1

field1: value11
field2: value12
field3: value13

%rec: Type2

field1: value21
field2: value22
field3: value23

afield1: appended1

%rec: Type3

field1: value31
field2: value32
field3: value33
'

test_tool recins-append-new-type ok \
          recins \
          '-t NewType -f afield1 -v "appended1"' \
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

%rec: NewType

afield1: appended1
'

test_tool recins-replace ok \
          recins \
          '-n 1 -f afield1 -v "replaced"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

afield1: replaced

field1: value31
field2: value32
field3: value33
'

test_tool recins-replace-sex ok \
          recins \
          '-e "field2 = '\''value22'\''" -f afield1 -v "replaced"' \
          multiple-records \
'field1: value11
field2: value12
field3: value13

afield1: replaced

field1: value31
field2: value32
field3: value33
'

test_tool recins-violate-restrictions xfail \
          recins \
          '-t Integrity -f Id -v "not a number"' \
          integrity

test_tool recins-force-restrictions ok \
          recins \
          '--force -t Integrity -f Id -v "not a number"' \
          integrity \
'%rec: Integrity
%type: Id int

Id: 0

Id: not a number
'

test_tool recins-only-descriptor ok \
          recins \
          '' \
          only-descriptor \
'%rec: foo
'

test_tool recins-comments-and-descriptor ok \
          recins \
          '' \
          comments-and-descriptor \
'# comment 1

# comment 2

%rec: foo
'

test_tool recins-external-descriptor ok \
          recins \
          '-t Patata -f foo -v 20' \
          external-descriptor \
'%rec: Patata external-descriptor-types

foo: 10

foo: 20
'          

#
# Cleanup.
#

test_cleanup
exit $?

# End of recins.sh
