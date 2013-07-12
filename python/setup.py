  #!/usr/bin/env python

from distutils.core import setup, Extension
setup(
    name = 'recutils', 
    version = '1.5',
    ext_modules = [
        Extension('recutils', ['recutils.c'],
                  libraries = [ 'rec' ],
                  ),
      ],
)  
