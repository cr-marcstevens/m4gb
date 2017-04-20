AC_PREREQ([2.68])
AC_INIT([M4GB], [1.0.0], [marc@marc-stevens.nl],[m4gb],[https://github.com/cr-marcstevens/m4gb])
AC_CONFIG_SRCDIR([README.md])
AC_CONFIG_HEADERS([config.h])
AX_PREFIX_CONFIG_H([lib/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([foreign subdir-objects])

DEFAULT_CXXFLAGS="-march=native -O3 -Wall -Werror"

CXXFLAGS=${CXXFLAGS:-$DEFAULT_CXXFLAGS}

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

AC_ARG_WITH([openf4], AS_HELP_STRING([--without-openf4],[Disable OpenF4]))
if test "x$with_openf4" != "xno"; then
	if test -d "${with_openf4}/include" ; then
		CPPFLAGS="-I${with_openf4}/include ${CPPFLAGS}"
		LDFLAGS="-L${with_openf4}/lib ${LDFLAGS}"
	elif test -d ${OPENF4DIR}/include/openf4 ; then
		CPPFLAGS="-I${OPENF4DIR}/include ${CPPFLAGS}"
		LDFLAGS="-L${OPENF4DIR}/lib ${LDFLAGS}"
	fi
	AC_DEFINE(OPENF4_GLOBAL_H,1,[Force exclusion of openf4/include/config.h])
	AC_CHECK_HEADER([libopenf4.h],[have_openf4=yes],[have_openf4=no])
else
	have_openf4=no
fi
if test "x$have_openf4" = "xyes"; then
	AC_MSG_CHECKING(for usability of OpenF4)
	LIBS="-lopenf4 ${LIBS}"
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
