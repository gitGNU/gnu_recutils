#!/usr/bin/python

# -*- mode: Python -*-
#
#       File:         recutilstest.py
#       Date:         Thu Sep 26 16:31:12 2013
#
#       GNU recutils - Test file for Python bindings 
#
#

# Copyright (C) 2013 Maninya M.

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


import sys	
import recutils
import pyrec

print "CREATING DATABASE!"
string1 = "movies.rec"
string2 = "books_account.rec"
db1 = pyrec.Recdb()
db2 = pyrec.Recdb()

print "\nLOADING FILE INTO DB"
db1.loadfile(string1);
db2.loadfile(string2);

size1 = db1.size()
print "Size of db1 = ",size1
size2 = db2.size()
print "Size of db2 = ",size2

print "\nGETTING THE RECORD SET AT A CERTAIN POSITION"

recset = db2.get_rset(2)
print "Got the rset"

print "\nGETTING THE NUMBER OF RECORDS IN RSET"
num_rec = recset.num_records()
print "Number of records = ", num_rec

print "\nGETTING THE TYPE OF A RECORD SET"
str_type = recset.type()
print "Type is",str_type 

print "\nGETTING THE RECORD DESCRIPTOR OF AN RSET"
desc = recset.descriptor()
print "Got the record descriptor"

print "\nGETTING NUMBER OF FIELDS IN THE DESCRIPTOR (RECORD)"
num_fields = desc.num_fields()
print "Number of fields is",num_fields

print "\nCHECKING FOR A FIELD VALUE IN A RECORD"
fname = desc.contains_value("Login",1)
print fname
if fname:
	print "Value exists"
else:
	print "Value doesn't exist"

print "\nCHECKING IF A TYPE EXISTS IN THE DB"
ty = db1.type("movies");
print "ty = ",ty
if ty:
	print "Type exists"
else:
	print "Type doesn't exist"

print "\nGETTING THE RSET BY TYPE"
rsettype = db2.get_rset_by_type("Account")
print "Got rset by type"








