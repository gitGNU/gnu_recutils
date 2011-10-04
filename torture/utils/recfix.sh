#!/bin/sh
#
# recfix.sh - System tests for recfix.
#
# Copyright (C) 2010, 2011 Jose E. Marchesi.
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
test_init "recfix"

#
# Create input files.
#

test_declare_input_file type-int-valid \
'%rec: Types
%type: Integer int

Integer: 10
' 

test_declare_input_file type-int-invalid \
'%rec: Types
%type: Integer int

Integer: aaa
'

test_declare_input_file type-real-valid \
'%rec: Types
%type: Real real

Real: 3.14
'

test_declare_input_file type-real-invalid \
'%rec: Types
%type: Real real

Real: 3..14
'

test_declare_input_file duplicated-keys \
'%rec: Keys
%key: Id

Id: 0

Id: 1

Id: 2

Id: 2

Id: 3
'

test_declare_input_file missing-mandatory \
'%rec: Mandatory
%mandatory: ma

foo: bar

bar: baz
ma: foo
'

test_declare_input_file several-unique \
'%rec: Unique
%unique: Id

Id: 0

Id: 1
Id: 2

Id: 3
'

test_declare_input_file referred-type \
'%rec: One
%type: foo int

foo: 10

foo: 20

%rec: Two

One:foo: 20
'

test_declare_input_file hidden-type \
'%rec: One
%type: foo int

foo: 10

foo: 20

%rec: Two
%type: foo line

One:foo: foobar
'

test_declare_input_file ranges-ok \
'%rec: Foo
%type: bar range -10 10
%type: baz range 10

bar: -10
baz: 0

bar: 10
baz: 10

bar: 2
baz: 5
'

test_declare_input_file ranges-xfail-1 \
'%rec: Foo
%type: bar range -10 10
%type: baz range 10

bar: -25
baz: 0
'

test_declare_input_file ranges-xfail-2 \
'%rec: Foo
%type: bar range -10 10
%type: baz range 10

bar: 2
baz: 11
'

test_declare_input_file multiple-rec \
'%rec: foo
%type: bar int
%rec: bar

bar: 10
'

test_declare_input_file enum-valid \
'%rec: foo
%type: bar enum
+ KEY1 (This is key 1)
+ KEY2 (This is key 2)
+ KEY3 (This is key 3)

bar: KEY1

bar: KEY2

bar: KEY3
'

test_declare_input_file enum-invalid-1 \
'%rec: foo
%type: bar enum
+ KEY1 (This is key 1)
+ KEY2 ((This is key 2)
+ KEY3 (This is key 3)

bar: KEY1

bar: KEY2

bar: KEY3
'

test_declare_input_file enum-invalid-2 \
'%rec: foo
%type: bar enum
+ KEY1 (This is key 1)
+ KEY2 (This is key 2))
+ KEY3 (This is key 3)

bar: KEY1

bar: KEY2

bar: KEY3
'

test_declare_input_file prohibited-fields-ok \
'%rec: foo
%prohibit: banned

foo: bar

bar: baz
'

test_declare_input_file prohibited-fields \
'%rec: foo
%prohibit: banned1 banned2

foo: bar
banned1: foo

bar: baz
banned2: bar

bar: foo
'

test_declare_input_file auto-int \
'%rec: foo
%type: myint int
%auto: myint
'

test_declare_input_file auto-range \
'%rec: foo
%type: myrange range 0 10
%auto: myrange
'

test_declare_input_file auto-date \
'%rec: foo
%type: mydate date
%auto: mydate
'

test_declare_input_file auto-notype \
'%rec: foo
%auto: myint
'

test_declare_input_file auto-nofex \
'%rec: foo
%auto: this%is#not%a^ fex
'

test_declare_input_file size-invalid-1 \
'%rec: foo
%size: >
'

test_declare_input_file size-invalid-2 \
'%rec: foo
%size: foo
'

test_declare_input_file size-exact-zero \
'%rec: foo
%size: 0
'

test_declare_input_file size-exact-zero-invalid \
'%rec: foo
%size: 0

foo: bar
'

test_declare_input_file size-exact \
'%rec: foo
%size: 2

foo: bar

bar: baz
'

test_declare_input_file size-exact-invalid \
'%rec: foo
%size: 2

foo: bar
'

test_declare_input_file size-less \
'%rec: foo
%size: < 2

foo: bar
'

test_declare_input_file size-less-invalid \
'%rec: foo
%size: < 2

foo: bar

bar: baz
'

test_declare_input_file size-less-equal \
'%rec: foo
%size: <= 2

foo: bar

bar: baz
'

test_declare_input_file size-less-equal-invalid \
'%rec: foo
%size: <= 1

foo: bar

bar: baz
'

test_declare_input_file size-bigger \
'%rec: foo
%size: > 1

foo: bar

bar: baz
'

test_declare_input_file size-bigger-invalid \
'%rec: foo
%size: > 2

foo: bar
'

test_declare_input_file size-bigger-equal \
'%rec: foo
%size: >= 2

foo: bar

bar: baz
'

test_declare_input_file size-bigger-equal-invalid \
'%rec: foo
%size: >= 2

foo: bar
'

test_declare_input_file size-several \
'%rec: foo
%size: 2
%size: < 10
'

test_declare_input_file typedef-valid \
'%rec: foo
%typedef: Id_t int
%type: Id Id_t

Id: 10
'

test_declare_input_file typedef-valid-xfail \
'%rec: foo
%typedef: Id_t int
%type: Id Id_t

Id: xx
'

test_declare_input_file typedef-valid-with-blanks \
'%rec: foo
%typedef:     
+ Id_t int
%type: Id Id_t    
+    

Id: 10
'

test_declare_input_file typedef-valid-with-blanks-xfail \
'%rec: foo
%typedef:     
+ Id_t int
%type: Id Id_t    
+    

Id: xx
'

test_declare_input_file typedef-valid-order \
'%rec: foo
%type: Id Id_t
%typedef: Id_t int

Id: 10
'

test_declare_input_file typedef-valid-order-xfail \
'%rec: foo
%type: Id Id_t
%typedef: Id_t int

Id: xx
'

test_declare_input_file typedef-valid-chain \
'%rec: foo
%typedef: Foo_t Bar_t
%typedef: Bar_t Baz_t
%typedef: Baz_t int
%type: Foo Foo_t

Foo: 10
'

test_declare_input_file typedef-valid-chain-xfail \
'%rec: foo
%typedef: Foo_t Bar_t
%typedef: Bar_t Baz_t
%typedef: Baz_t int
%type: Foo Foo_t

Foo: xx
'

test_declare_input_file typedef-valid-multiple \
'%rec: foo
%typedef: Foo_t int
%typedef: Foo_t email
%type: Foo Foo_t

Foo: foo@bar.baz
'

test_declare_input_file typedef-valid-multiple-xfail \
'%rec: foo
%typedef: Foo_t int
%typedef: Foo_t email
%type: Foo Foo_t

Foo: 10
'

test_declare_input_file typedef-invalid-bad-type \
'%rec: foo
%typedef: Id_t int invalid
%type: Id Id_t

Id: 10
'

test_declare_input_file typedef-invalid-chain-undefined \
'%rec: foo
%typedef: Id_t Undefined_t
%type: Id Id_t

Id: 10
'

test_declare_input_file typedef-invalid-chain-loop \
'%rec: foo
%typedef: Foo_t Bar_t
%typedef: Bar_t Baz_t
%typedef: Baz_t Foo_t
%type: Id Foo_t

Id: 10
'

test_declare_input_file sortcheck-valid \
'%rec: foo
%sort: AField
'

test_declare_input_file sortcheck-with-blanks \
'%rec: foo
%sort:       
+       AField \
   
'

test_declare_input_file sortcheck-invalid-empty \
'%rec: foo
%sort:
'

test_declare_input_file sortcheck-invalid-field-name \
'%rec: foo
%sort: A/Field
'

test_declare_input_file confidential \
'%rec: foo
%confidential: Foo Bar Baz

Foo: encrypted-foo
Bar: encrypted-bar
Baz: encrypted-baz

%rec: bar

Foo: jorl
Bar: jarl
Baz: jerl
'

test_declare_input_file confidential-several \
'%rec: foo
%confidential: Foo Bar Baz
%confidential: Jorl
'

test_declare_input_file confidential-with-unencrypted-fields \
'%rec: foo
%confidential: Foo

Foo: encrypted-foo

Foo: Not encrypted
'

test_declare_input_file confidential-fields \
'%rec: foo
%confidential: Password WebPassword

User: user1
Password: secret1
Password: secret2
WebPassword: websecret1

User: user2
Password: secret2
Password: secret22
WebPassword: websecret2
'

test_declare_input_file encrypt \
'%rec: Account
%confidential: Secret

Secret: foo

Secret: bar

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: je
fo: fu

joo: ji
fo: ja
'

test_declare_input_file decrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

#
# Declare tests.
#

test_tool recfix-with-operation ok \
          recfix \
          '--check' \
          type-int-valid \
''

test_tool recfix-type-int-valid ok \
          recfix \
          '' \
          type-int-valid \
''

test_tool recfix-type-int-invalid xfail \
          recfix \
          '' \
          type-int-invalid

test_tool recfix-type-real-valid ok \
          recfix \
          '' \
          type-real-valid \
''

test_tool recfix-type-real-invalid xfail \
          recfix \
          '' \
          type-real-invalid

test_tool recfix-duplicated-keys xfail \
          recfix \
          '' \
          duplicated-keys

test_tool recfix-missing-mandatory xfail \
          recfix \
          '' \
          missing-mandatory

test_tool recfix-several-unique xfail \
          recfix \
          '' \
          several-unique

test_tool recfix-referred-type ok \
          recfix \
          '' \
          referred-type \
''

test_tool recfix-hidden-type ok \
          recfix \
          '' \
          hidden-type \
''

test_tool recfix-ranges-ok ok \
          recfix \
          '' \
          ranges-ok \
''

test_tool recfix-ranges-xfail-1 xfail \
          recfix \
          '' \
          ranges-xfail-1

test_tool recfix-ranges-xfail-2 xfail \
          recfix \
          '' \
          ranges-xfail-2

test_tool recfix-one-rec ok \
          recfix \
          '' \
          type-int-valid \
          ''

test_tool recfix-multiple-rec-in-descriptor xfail \
          recfix \
          '' \
          multiple-rec

test_tool recfix-enum-valid ok \
          recfix \
          '' \
          enum-valid \
          ''

test_tool recfix-enum-invalid-1 xfail \
          recfix \
          '' \
          enum-invalid-1

test_tool recfix-enum-invalid-2 xfail \
          recfix \
          '' \
          enum-invalid-2

test_tool recfix-prohibited-fields-ok ok \
          recfix \
          '' \
          prohibited-fields-ok \
''

test_tool recfix-prohibited-fields-fail xfail \
          recfix \
          '' \
          prohibited-fields

test_tool recfix-auto-int ok \
          recfix \
          '' \
          auto-int \
''

test_tool recfix-auto-range ok \
          recfix \
          '' \
          auto-range \
''

test_tool recfix-auto-date ok \
          recfix \
          '' \
          auto-date \
''

test_tool recfix-auto-notype xfail \
          recfix \
          '' \
          auto-notype

test_tool recfix-auto-nofex xfail \
          recfix \
          '' \
          auto-nofex

test_tool recfix-size-invalid-1 xfail \
          recfix \
          '' \
          size-invalid-1

test_tool recfix-size-invalid-2 xfail \
          recfix \
          '' \
          size-invalid-2

test_tool recfix-size-exact-zero ok \
          recfix \
          '' \
          size-exact-zero \
''

test_tool recfix-size-exact-zero-invalid xfail \
          recfix \
          '' \
          size-exact-zero-invalid

test_tool recfix-size-exact ok \
          recfix \
          '' \
          size-exact \
''

test_tool recfix-size-exact-invalid xfail \
          recfix \
          '' \
          size-exact-invalid

test_tool recfix-size-less ok \
          recfix \
          '' \
          size-less \
''

test_tool recfix-size-less-invalid xfail \
          recfix \
          '' \
          size-less-invalid

test_tool recfix-size-less-equal ok \
          recfix \
          '' \
          size-less-equal \
''

test_tool recfix-size-less-equal-invalid xfail \
          recfix \
          '' \
          size-less-equal-invalid

test_tool recfix-size-bigger ok \
          recfix \
          '' \
          size-bigger \
''

test_tool recfix-size-bigger-invalid xfail \
          recfix \
          '' \
          size-bigger-invalid

test_tool recfix-size-bigger-equal ok \
          recfix \
          '' \
          size-bigger-equal \
''

test_tool recfix-size-bigger-equal-invalid xfail \
          recfix \
          '' \
          size-bigger-equal-invalid

test_tool recfix-size-several xfail \
          recfix \
          '' \
          size-several

test_tool recfix-typedef-valid ok \
          recfix \
          '' \
          typedef-valid \
''

test_tool recfix-typedef-valid-xfail xfail \
          recfix \
          '' \
          typedef-valid-xfail

test_tool recfix-typedef-valid-with-blanks ok \
          recfix \
          '' \
          typedef-valid-with-blanks \
''

test_tool recfix-typedef-valid-with-blanks-xfail xfail \
          recfix \
          '' \
          typedef-valid-with-blanks-xfail

test_tool recfix-typedef-valid-order ok \
          recfix \
          '' \
          typedef-valid-order \
''

test_tool recfix-typedef-valid-order-xfail xfail \
          recfix \
          '' \
          typedef-valid-order-xfail

test_tool recfix-typedef-valid-chain ok \
          recfix \
          '' \
          typedef-valid-chain \
''

test_tool recfix-typedef-valid-chain-xfail xfail \
          recfix \
          '' \
          typedef-valid-chain-xfail

test_tool recfix-typedef-valid-multiple ok \
          recfix \
          '' \
          typedef-valid-multiple \
''

test_tool recfix-typedef-valid-multiple-xfail xfail \
          recfix \
          '' \
          typedef-valid-multiple-xfail

test_tool recfix-typedef-invalid-bad-type xfail \
          recfix \
          '' \
          typedef-invalid-bad-type

test_tool recfix-typedef-invalid-chain-undefined xfail \
          recfix \
          '' \
          typedef-invalid-chain-undefined

test_tool recfix-typedef-invalid-chain-loop xfail \
          recfix \
          '' \
          typedef-invalid-chain-loop

test_tool recfix-sortcheck-valid ok \
          recfix \
          '' \
          sortcheck-valid \
''

test_tool recfix-sortcheck-with-blanks ok \
          recfix \
          '' \
          sortcheck-with-blanks \
''

test_tool recfix-sortcheck-invalid-empty xfail \
          recfix \
          '' \
          sortcheck-invalid-empty

test_tool recfix-sortcheck-invalid-field-name xfail \
          recfix \
          '' \
          sortcheck-invalid-field-name

test_tool recfix-confidential ok \
          recfix \
          '--check' \
          confidential \
''

test_tool recfix-confidential-several ok \
          recfix \
          '--check' \
          confidential-several \
''

test_tool recfix-confidential-with-unencrypted-fields xfail \
          recfix \
          '--check' \
          confidential-with-unencrypted-fields

test_tool recfix-encrypt ok \
          recfix \
          '--encrypt -s foo' \
          encrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

test_tool recfix-encrypt-password-long ok \
          recfix \
          '--encrypt --password=foo' \
          encrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

test_tool recfix-encrypt-without-password xfail \
          recfix \
          '--encrypt' \
          encrypt

test_tool recfix-encrypt-password-before-operation xfail \
          recfix \
          '-s foo --encrypt' \
          encrypt

test_tool recfix-decrypt ok \
          recfix \
          '--decrypt -s foo' \
          decrypt \
'%rec: Account
%confidential: Secret

Secret: foo

Secret: bar

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: je
fo: fu

joo: ji
fo: ja
'

test_tool recfix-decrypt-invalid-password ok \
          recfix \
          '--decrypt -s bar' \
          decrypt \
'%rec: Account
%confidential: Secret

Secret: encrypted-xsU/pJwqJBZv3+6tn2AzTA==

Secret: encrypted-V1xOls6u5Zw/D5AOtZ9gfQ==

%rec: Jorl

Secret: jojo

%rec: jojo
%confidential: joo

joo: encrypted-MhsqXvDjqU9vOXG8QoHxKg==
fo: fu

joo: encrypted-By/F2HBy1wiim1fUWMVKRg==
fo: ja
'

#
# Cleanup.
#

test_cleanup
exit $?

# End of recfix.sh
