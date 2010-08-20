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

AC_INIT(src/rec.h)
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE(recutils, git)
AM_CONFIG_HEADER(src/config.h)
AC_CONFIG_MACRO_DIR([m4])

gl_EARLY

AC_PROG_LIBTOOL
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_YACC

dnl System
AC_CANONICAL_HOST
canonical=$host

gl_INIT

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

have_csv=no
AC_CHECK_LIB([csv],[csv_init],[have_csv=yes],)
AM_CONDITIONAL([LIBCSV], [test "x$have_csv" = "xyes"])
if test "x$have_csv" = "xyes"; then
   CSVLIBS=-lcsv
fi
AC_SUBST([CSVLIBS])

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

dnl Generate output files
AC_OUTPUT(Makefile lib/Makefile src/Makefile utils/Makefile doc/Makefile torture/Makefile)

dnl End of configure.ac
