#!/usr/bin/python
import sys	
import recutils
import pyrec

db = pyrec.Recdb()
print "Created db"
db.appendfile("books.rec")
# db1 = pyrec.Recdb()
db2 = pyrec.Recdb()
print "Created db2"
db.appendfile("account.rec")
db.appendfile("account.rec") #Should get duplicate rset error
db2.loadfile("books.rec")
db.pywritefile("books_account.rec")


print "CREATE TWO FIELDS"
fl1 = recutils.field("Author", "Richard M. Stallman")
fl2 = recutils.field("Skater", "Terry Pratchett")

ch = recutils.field_equal_p(fl1,fl2)
print ch
if ch:
	print "Fields equal"
else:
	print "Fields not equal"

print "\nPRINT NAME AND VALUE OF FIELDS"
name1 = fl1.name()
print "name1 = ",name1

fvalue2 = fl2.value()
print "fvalue2 = ",fvalue2

print "\nSET NAME OF FIELD1"
s = fl1.set_name("Mike Wazowski")

name1 = fl1.name()
print "New name1 = ",name1

print "\nSET VALUE OF FIELD2"
s = fl2.set_value("The Friendly Monster")

value2 = fl2.value()
print "New value2 = ",value2

print "\nGET SOURCE OF FIELD2"
source = fl2.source()
print "source = ", source

print "\nGETTING THE RECORD SET AT A CERTAIN POSITION"
recset = db.get_rset(2)
print "Got the rset"

print "\nGETTING THE RECORD DESCRIPTOR OF AN RSET"
desc = recset.descriptor()
print "Got the record descriptor"

print "\nCHECKING IF FIELD '%confidential: Password' EXISTS"
fname = desc.contains_field("%confidential", "Password")
if fname:
	print "Field exists"
else:
	print "Field doesn't exist"

print "\nINSERT AN RSET INTO DB"
num = db2.size()
print "Size before = ",num
db2.insert_rset(recset,0);
num = db2.size()
print "Size after = ",num
print "Writing to file"
flag4 = db2.pywritefile("account_books.rec")
print "flag4 = ", flag4

print "\nREMOVE AN RSET FROM DB"
db2.remove_rset(2)
print "Writing to file"
flag4 = db2.pywritefile("account1.rec")
print "flag4 = ", flag4


print "\nCREATE THE SEXES & FEXES"
sex1 = pyrec.RecSex(1)
b = sex1.pycompile("Location = 'home'")
print "Sex compiled success = ",b
fexe1 = pyrec.Fexenum.REC_FEX_SIMPLE
fex1 = recutils.fex("Author",fexe1)

print "\nCALLING QUERY FUNCTION"
print "Query for a record set consisting of the list of Authors of books at home"
queryrset = db.query("Book", None, None, sex1, None, 10, fex1, None, None, None, 0)
num_rec = queryrset.num_records()
print "Number of queried records = ",num_rec

print "\nINSERTING QUERIED RSET"
db2.insert_rset(queryrset,2);

print "\nSIZE AFTER INSERTING"
num = db2.size()
print "Num = ",num

flag4 = db2.writefile("account1_query.rec")





