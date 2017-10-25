AC_PREREQ([2.68])
AC_INIT([M4GB], [1.0.0], [marc@marc-stevens.nl],[m4gb],[https://github.com/cr-marcstevens/m4gb])
AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([config.h])
AX_PREFIX_CONFIG_H([lib/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects])

DEFAULT_CXXFLAGS="-O3 -Wall -Werror"
AS_IF([test "x$CXXFLAGS" = "x"],
	[CXXFLAGS="$DEFAULT_CXXFLAGS"]
	[usedefaultcxxflags=yes],
	[usedefaultcxxflags=no])

LT_INIT

AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_MKDIR_P
AC_PROG_CC
AC_PROG_CPP

AC_LANG([C++])
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext],[mandatory])

AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

AS_IF([test "x$cross_compiling" != "xyes" && test "x$usedefaultcxxflags" = "xyes" ],
      [AX_CHECK_COMPILE_FLAG([-march=native], [CXXFLAGS="$CXXFLAGS -march=native"], [])])

AX_PTHREAD()
LIBS="$PTHREAD_LIBS $LIBS"
CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"





OPENF4DIR=${OPENF4DIR:-$(pwd)/local}
AC_ARG_WITH([openf4], AS_HELP_STRING([--with-openf4@<:@=yes|no|DIR@:>@],[prefix where openf4 is installed (default=yes)]),
[
	if test "$withval" = "no"; then
		want_openf4="no"
	elif test "$withval" = "yes"; then
		want_openf4="yes"
	else
		want_openf4="yes"
		OPENF4DIR=$withval
	fi
],[
	want_openf4="yes"
]
)
if test "x$want_openf4" != "xno"; then
	if test "x$OPENF4DIR" != "x" && test -d $OPENF4DIR/include; then
		CPPFLAGS="-I$OPENF4DIR/include $CPPFLAGS"
		LDFLAGS="-L$OPENF4DIR/lib"
	fi
	AC_DEFINE(OPENF4_GLOBAL_H,1,[Force exclusion of openf4/include/config.h])
	AC_CHECK_HEADER([libopenf4.h],[],[AC_MSG_ERROR(error: libopenf4.h not found)])
	AC_MSG_CHECKING(for usability of OpenF4)
	LIBS="-lopenf4 $LIBS"
	AC_LINK_IFELSE([AC_LANG_SOURCE([
		#include <libopenf4.h>
		int main() {
			std::vector<std::string> tmp;
			groebnerBasisF4 (0,0,tmp,tmp,0,0);
		}
		])],
		[AC_MSG_RESULT(yes)
		AC_DEFINE(HAVE_OPENF4,1,[Define if OpenF4 is installed])
		],[AC_MSG_ERROR(error: see log)
		])
fi



AC_CONFIG_FILES([
 Makefile
 tests/Makefile
])

AC_OUTPUT
