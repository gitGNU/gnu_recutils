dnl configure.ac for GNU rec
dnl
dnl Please process this file with autoconf to get a 'configure'
dnl script.

dnl Copyright (C) 2009, 2010 Jose E. Marchesi

dnl This program is free software: you can redistribute it and/or modify
dnl it under the terms of the GNU General Public License as published by
dnl the Free Software Foundation, either version 3 of the License, or
dnl (at your option) any later version.
dnl
dnl This program is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU General Public License for more details.
dnl
dnl You should have received a copy of the GNU General Public License
dnl along with this program.  If not, see <http://www.gnu.org/licenses/>.

AC_INIT([GNU recutils], [1.1.90], [bug-recutils@gnu.org])

dnl Must come before AM_INIT_AUTOMAKE
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_MACRO_DIR([m4])

dnl Autobuild
AB_INIT

gl_EARLY

AC_PROG_LIBTOOL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_YACC

dnl System
AC_CANONICAL_HOST
canonical=$host

gl_INIT

# GNU help2man creates man pages from --help output; in many cases, this
# is sufficient, and obviates the need to maintain man pages separately.
# However, this means invoking executables, which we generally cannot do
# when cross-compiling, so we test to avoid that (the variable
# "cross_compiling" is set by AC_PROG_CC).
if test $cross_compiling = no; then
  AM_MISSING_PROG(HELP2MAN, help2man)
else
  HELP2MAN=:
fi

dnl Seach for headers
AC_CHECK_HEADERS([malloc.h string.h])

dnl Search for data types
AC_CHECK_TYPE(size_t, unsigned)
AC_CHECK_TYPE(off_t, long)

dnl Search for functions
AC_FUNC_FSEEKO

dnl Search for required libraries

have_check=yes
AC_CHECK_LIB([check],[suite_create],,[have_check=no])
AM_CONDITIONAL([CHECK], [test "x$have_check" != "xno"])

have_curl=yes
AC_CHECK_LIB([curl],[curl_global_init],,[have_curl=no])
if test "x$have_curl" = "xyes"; then
   CURLLIBS=-lcurl
fi
AC_SUBST([CURLLIBS])

have_mdb=no
AC_CHECK_LIB([mdb],[mdb_init],[have_mdb=yes],)
if test "x$have_mdb" = "xyes"; then
   MDBLIBS=-lmdb
fi
AC_SUBST([MDBLIBS])

have_glib=no
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.0.0],
                  [have_glib=yes], [have_glib=no])

AM_CONDITIONAL([COMPILE_MDB2REC],
               [test "x$have_glib" = "xyes" && test "x$have_mdb" = "xyes"])
AM_CONDITIONAL([REMOTE_DESCRIPTORS], [test "x$have_curl" = "xyes"])

dnl Search for required programs

AM_PROG_LEX

dnl Platform-based compilation options
compile_w32_system=no
case "${host}" in
     *-mingw32*)
          compile_w32_system=yes
          ;;
     *)
          ;;
esac

dnl i18n with gettext
AM_GNU_GETTEXT_VERSION([0.18.1])
AM_GNU_GETTEXT([external])

dnl gcov compilation
AC_ARG_ENABLE([coverage],
              AS_HELP_STRING([--enable-coverage],
                             [Compile the library with code coverage support (default is NO)]),
              [use_gcov=$enableval], [use_gcov=no])
AM_CONDITIONAL([USE_COVERAGE], [test "x$use_gcov" = "xyes"])

dnl Generate output files
AC_OUTPUT(Makefile
          lib/Makefile
          libcsv/Makefile
          src/Makefile
          utils/Makefile
          doc/Makefile
          po/Makefile.in
          torture/Makefile
          torture/utils/Makefile
          man/Makefile
          etc/Makefile)

dnl Report warnings

if test "x$have_mdb" = "xno"; then
   echo "warning: libmdb was not found in the system."
   echo "warning: the mdb2rec utility won't get built."
fi

if test "x$have_glib" = "xno"; then
   echo "warning: glib was not found in the system."
   echo "warning: the mdb2rec utility won't get built."
fi

dnl End of configure.ac
