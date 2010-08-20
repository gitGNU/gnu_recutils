#!/bin/sh
# Created 2005-03-27, Karl Berry.  Public domain.
# Modified 2010-08-05, Jose E. Marchesi.  Public domain.

if test "x$1" = x-n; then
  chicken=true
  echo "Ok, playing chicken; not actually running any commands."
else
  chicken=
fi

echo "Preparing git recutils infrastructure:"

# This overwrites lots of files with older versions, so don't use it.
# I keep the newest versions of files common between distributions up to
# date in CVS, using gnulib-tool, because it's not trivial for every
# developer to do this.
#cmd="autoreconf --verbose --force --install --include=gnulib/m4"

: ${ACLOCAL=aclocal}
: ${AUTOHEADER=autoheader}
: ${LIBTOOLIZE=libtoolize}
: ${AUTOMAKE=automake --add-missing}
: ${AUTOCONF=autoconf}

# So instead:
cmd="$ACLOCAL -I m4 && $AUTOCONF && $AUTOHEADER && $LIBTOOLIZE && $AUTOMAKE"
echo "  $cmd"
$chicken eval $cmd || exit 1

echo
echo "Now run configure with your desired options, for instance:"
echo "  ./configure CFLAGS=-g"
