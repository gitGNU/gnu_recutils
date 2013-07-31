#!/usr/bin/python
import sys	
import recutils

print "CREATING DATABASE!"
string1 = "movies.rec"
string2 = "books_account.rec"
db1 = recutils.recdb()
db2 = recutils.recdb()

print "LOADING FILE INTO DB"
db1.loadfile(string1);
db2.loadfile(string2);

size1 = db1.size()
print "Size of db1 = ",size1
size2 = db2.size()
print "Size of db2 = ",size2

print "GETTING THE RECORD SET AT A CERTAIN POSITION"

recset = db2.get_rset(2)
print "Got the rset"

print "GETTING THE NUMBER OF RECORDS IN RSET"
num_rec = recset.num_records()
print "Number of records = ", num_rec

print "GETTING THE TYPE OF A RECORD SET"
str_type = recset.type()
print "Type is",str_type 

print "GETTING THE RECORD DESCRIPTOR OF AN RSET"
desc = recset.descriptor()
print "Got the record descriptor"

print "GETTING NUMBER OF FIELDS IN THE DESCRIPTOR (RECORD)"
num_fields = desc.num_fields()
print "Number of fields is",num_fields

print "CHECKING FOR A FIELD VALUE IN A RECORD"
fname = desc.contains_value("Login",1)
print fname
if fname:
	print "Value exists"
else:
	print "Value doesn't exist"

print "CHECKING IF A TYPE EXISTS IN THE DB"
ty = db1.type("movies");
print "ty = ",ty
if ty:
	print "Type exists"
else:
	print "Type doesn't exist"

print "GETTING THE RSET BY TYPE"
rsettype = db2.get_rset_by_type("Account")
print "Got rset by type"








