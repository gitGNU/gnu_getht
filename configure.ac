#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([getht], [0.1])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE()

# Checks for programs.
AC_PROG_CC
AC_PROG_AWK
AC_PROG_MAKE_SET

# Checks for libraries.
PKG_CHECK_MODULES([LIBXML2], [libxml-2.0 >= 2.0.0])
PKG_CHECK_MODULES([LIBCURL], [libcurl >= 7.7.2])
LIBS="$LIBXML2_LIBS $LIBCURL_LIBS $LIBS"
CFLAGS="$LIBXML2_CFLAGS $LIBCURL_CFLAGS $CFLAGS"

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([mkdir])

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 src/Makefile])
AC_OUTPUT
