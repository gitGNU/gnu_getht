#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT(getht, 0.0.1)
AC_CONFIG_SRCDIR([src/])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(AC_PACKAGE_NAME,AC_PACKAGE_VERSION)
AC_LANG([C])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.

dnl check for zlib
CHECK_ZLIB()

dnl check for libxml2
AM_PATH_XML2([2.0.0])
CFLAGS="$CFLAGS $XML_CPPFLAGS"
LIBS="$LIBS $XML_LIBS"
#PKG_CHECK_MODULES(libxml-2.0)
#AC_SUBST(XML_CPPFLAGS)
#AC_SUBST(XML_LIBS)

dnl check for libcurl
LIBCURL_CHECK_CONFIG([yes], [7.7.2])
CFLAGS="$CFLAGS $LIBCURL_CPPFLAGS"
LIBS="$LIBS $LIBCURL"

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([mkdir strdup])

AC_CONFIG_FILES([Makefile
                 doc/Makefile
                 src/Makefile])
AC_OUTPUT
