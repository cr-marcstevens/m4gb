# ax_givaro.m4: An m4 macro to detect and configure Givaro
#
# Copyright  2017 Rusydi Makarim <makarim@cwi.nl>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#
# As a special exception to the GNU General Public License, if you
# distribute this file as part of a program that contains a
# configuration script generated by Autoconf, you may include it under
# the same distribution terms that you use for the rest of that program.
#

#
# SYNOPSIS
#       AX_GIVARO()
#
# DESCRIPTION
#       Checks the existence of Givaro headers and libraries.
#       Options:
#       --with-givaro=(path|yes|no)
#               Indicates whether to use Givaro or not, and the path of a non-standard
#               installation location of Givaro if necessary.
#
#       This macro calls:
#               AC_SUBST(GIVARO_CPPFLAGS)
#               AC_SUBST(GIVARO_LDFLAGS)
#               AC_SUBST(GIVARO_LIB)
#
AC_DEFUN([AX_GIVARO],
[
GIVARODIR=${GIVARODIR:-$(pwd)/givaro}
AC_ARG_WITH([givaro], AS_HELP_STRING([--with-givaro@<:@=yes|no|DIR@:>@],[prefix where givaro is installed (default=yes)]),
[
	if test "$withval" = "no"; then
		want_givaro="no"
	elif test "$withval" = "yes"; then
		want_givaro="yes"
	else
		want_givaro="yes"
		GIVARODIR=$withval
	fi
],[
	want_givaro="maybe"
]
)

have_givaro=no
if test "x$want_givaro" != "xno"; then
	CPPFLAGS_SAVED="$CPPFLAGS"
	LDFLAGS_SAVED="$LDFLAGS"
	LIBS_SAVED="$LIBS"

	if test "x$GIVARODIR" != "x" && test -d $GIVARODIR/include; then
		GIVARO_CPPFLAGS="-I$GIVARODIR/include"
		GIVARO_LDFLAGS="-L$GIVARODIR/lib"
	else
		GIVARO_CPPFLAGS=""
		GIVARO_LDFLAGS=""
	fi
	GMP_LIB=-lgmp
	GMPXX_LIB=-lgmpxx
	GIVARO_LIB=-lgivaro
	CPPFLAGS="$GIVARO_CPPFLAGS $CPPFLAGS"
	LDFLAGS="$GIVARO_LDFLAGS $LDFLAGS"
	LIBS="$GIVARO_LIB $GMP_LIB $GMPXX_LIB $LIBS"

	have_givaro=yes
	AC_DEFINE(GIVARO_GLOBAL_H,1,[Force exclusion of givaro/include/givaro-config.h])
	AC_CHECK_HEADER([givaro/givinit.h],[],[have_givaro=no])
	AC_MSG_CHECKING(for usability of Givaro)
	AC_LINK_IFELSE([AC_LANG_SOURCE([
		#include <givaro/givinit.h>
		int main() {
			Givaro::GivaroMain::Init();
		}
		])],
		[AC_MSG_RESULT(yes)],
		[AC_MSG_RESULT(no)
		have_givaro=no])

	AS_IF([test "x$have_givaro" = "xyes"],
		[AC_DEFINE(HAVE_GIVARO,1,[Define if Givaro is installed])],
		[AS_IF([test "x$want_givaro" = "xyes"],[AC_MSG_ERROR(error: see log)],[])])
	CPPFLAGS="$CPPFLAGS_SAVED"
	LDFLAGS="$LDFLAGS_SAVED"
	LIBS="$LIBS_SAVED"
fi

AM_CONDITIONAL(HAVE_GIVARO, test "x${have_givaro}" = "xyes")

AC_SUBST(GIVARO_CPPFLAGS)
AC_SUBST(GIVARO_LDFLAGS)
AC_SUBST(GIVARO_LIB)
AC_SUBST(GMP_LIB)
AC_SUBST(GMPXX_LIB)

])
