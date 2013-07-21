#!/usr/bin/python
import sys	
import recutils

print "Creating Databases!"
stringin1 = "books.rec"
stringin2 = "account.rec"
stringin3 = "book_account.rec"
stringout1 = "books_write.rec"

db1 = recutils.recdb()
db2 = recutils.recdb()
db3 = recutils.recdb()

print "Loading files in DBs"
flag1 = db1.loadfile(stringin1)
flag2 = db2.loadfile(stringin2)
num1 = recutils.recdb.size(db1)
print "Size of db1: ",num1
flag3 = db3.loadfile(stringin3)
num3 = recutils.recdb.size(db3)
print "Size of db3: ",num3

print "Writing to file"
flag4 = db3.writefile(stringout1)
print "flag4 = ", flag4

print "Get a record set"
recset = db2.get_rset(1)
num_rec = recset.num_records()
print "Number of records in rset = ", num_rec
desc = recset.descriptor()
print "Got the descriptor"
num_fields = desc.num_fields()
print "Number of fields in the rec descriptor = ", num_fields


