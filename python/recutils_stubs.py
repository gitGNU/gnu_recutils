""" DATABASES"""
    
class RecException(Exception):
    """Base exception class for recutils. May need to add some parameters."""
    pass

class RecParseException(RecException):
    """Exception raised when a file could not be parsed for example"""
    pass

class RecWriteException(RecException):
    """Exception raised when a file could not be written for example"""
    pass


class RecDB:
    def __init_(self):

    def destroy(self):
        pass
        
    def loadfile(self, filename):
        pass

    def writefile(self, filename):
        pass

    def size(self):
        """Return the number of record sets contained in the db"""
        pass

    def add_data_from_file(self, filename):
        """Add data from the given file into the in-memory recdb object."""
        pass

    def query(self, *args, **kwargs):
        """Function to query the database"""
        """Need to create a SEX class"""
        pass

    def insert(self, *args, **kwargs):
        """Insert a recordset or similar into db"""
        pass

    def remove(self, *args, **kwargs):
        """Remove a recordset from the db"""
        pass


    def check_int(self, *args, **kwargs, **errors):
        """Check the integrity of all the record sets stored in a given
           database.  This function returns the number of errors found.
           Descriptive messages about the errors are appended to ERRORS."""
        pass
        
    def type_p(self,*args,**kwargs, *Type):
        """Determine whether an rset named TYPE exists in the db."""
        pass
        
""" RECORDSETS"""
        
class RecSet:
    def __init_(self):

    def destroy(self):
        pass
           
    def num_records(self):
        """Return the number of records stored in the record set"""
        pass
        
    def dup(self):
        """Create a copy of a record set and return a reference to it.  This
           operation performs a deep copy of the contained records and
           comments.  NULL is returned if there is no enough memory to perform
           the operation."""
        pass
        
        
    def descriptor(self):
        """Return the record descriptor of a given record set.  NULL is
           returned if the record set does not feature a record
           descriptor."""
        pass
        
    def 


class Record:
    def __init_(self):

    def destroy(self):
        pass
        

"""Add more classes E.g.: For manipulating record sets or records, SEXes, FEXes, etc"""
