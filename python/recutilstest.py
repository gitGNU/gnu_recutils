#!/usr/bin/python
import sys
import recutils

print "Creating Database!"
string = "books.rec"
string2 = "books_write.rec"
a = recutils.RecDb()
print "I made an object!"
flag = a.loadfile(string)
print "flag = ", flag
num = recutils.RecDb.size(a)
print "Size of db: ",num
print "Writing to file"
flag2 = a.writefile(string2)
print flag2


