#!/usr/bin/env python

# Copyright (C) 2012 Jose E. Marchesi

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

from distutils.core import setup, Extension

setup(
    name = 'rec',
    version = '1.5',
    author = 'Jose E. Marchesi',
    author_email = 'jemarch@gnu.org',
    url = 'http://www.gnu.org/software/recutils',
    description = 'text based databases called recfiles',
    license = 'GPLv3',
    long_description = open('README').read(),
    classifiers = [
        'Intended Audience :: Developers',
        'Environment :: Console',
        'Operating System :: POSIX',
        'Operating System :: Unix',
        'Operating System :: Microsoft :: Windows',
        'Topic :: Software Development :: Libraries'
        ],
    ext_modules = [
        Extension('rec', ['recmodule.c'],
                  libraries = [ 'rec' ],
                  ),
        ],
    )

# End of setup.py
