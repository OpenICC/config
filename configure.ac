define([MAJOR],[0])
define([MINOR],[1])
define([MICRO],[0])

AC_INIT(OpenICC, [MAJOR.MINOR.MICRO], http://www.sf.net/p/openicc/bugs, openicc, http://www.sf.net/p/openicc)

LIBTOOL_CURRENT=1
LIBTOOL_REVISION=0
LIBTOOL_AGE=0
LIBTOOL_VERSION=$LIBTOOL_CURRENT:$LIBTOOL_REVISION:$LIBTOOL_AGE

OPENICC_MAJOR_VERSION=MAJOR
OPENICC_MINOR_VERSION=MINOR
OPENICC_MICRO_VERSION=MICRO
OPENICC_VERSION=$OPENICC_MAJOR_VERSION.$OPENICC_MINOR_VERSION.$OPENICC_MICRO_VERSION
OPENICC_VERSION_NAME="$OPENICC_MAJOR_VERSION.$OPENICC_MINOR_VERSION.$OPENICC_MICRO_VERSION"

AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR([src/include/openicc_version.h.in])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])

AC_SUBST(LIBTOOL_VERSION)

AM_SANITY_CHECK

AC_PROG_CC
AM_PROG_CC_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LN_S
AC_PROG_LIBTOOL
AM_SILENT_RULES

AM_MAINTAINER_MODE
AC_C_BIGENDIAN

AC_HEADER_STDC

openicc_save_PKG_CONFIG_PATH="$PKG_CONFIG_PATH"
echo ""
echo PKG_CONFIG_PATH = $PKG_CONFIG_PATH
echo ""
if test -z "$PKG_CONFIG_PATH"; then 
  PKG_CONFIG_PATH="$libdir/pkgconfig"
else
  PKG_CONFIG_PATH="$PKG_CONFIG_PATH:$libdir/pkgconfig"
fi


dnl check for pkg-config
export PKG_CONFIG_PATH
AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
if test "x$PKG_CONFIG" == "xno"; then
  AC_MSG_WARN([
*** Check for pkg-config failed. Many packages requires it.
*** You can download it from http://pkgconfig.sourceforge.net/  .])
  ERROR=1
fi


AC_ARG_ENABLE(debug, [  --enable-debug          turn on debugging [[default=no]]],
if eval "test x$enable_debug = xyes"; then
  DEBUGFLAG="-g -Wall --pedantic"
fi)


if eval "test x$GCC = xyes"; then
  if echo "$CFLAGS" | grep "\-Wall" > /dev/null 2> /dev/null; then
    CFLAGS="$CFLAGS"
  else
    CFLAGS="$CFLAGS -Wall"
  fi

  if eval "test x$enable_ansi = xyes"; then
    if echo "$CFLAGS" | grep "\-ansi" > /dev/null 2> /dev/null; then
      CFLAGS="$CFLAGS"
    else
      CFLAGS="$CFLAGS -ansi"
    fi

    if echo "$CFLAGS" | grep "\-pedantic" > /dev/null 2> /dev/null; then
      CFLAGS="$CFLAGS"
    else
      CFLAGS="$CFLAGS -pedantic"
    fi
  fi
fi

openicc_save_CFLAGS="$CFLAGS"
openicc_save_LDFLAGS="$LDFLAGS"
openicc_save_LIBS="$LIBS"

CFLAGS="$CFLAGS -I$includedir"
LDFLAGS="$LDFLAGS"
LIBS="$LIBS -L$libdir"

dnl Test for libyajl
  export PKG_CONFIG_PATH
    if test "x$PKG_CONFIG" != "xno"; then
      YAJL_CONFIG="$PKG_CONFIG yajl"
      yajl_version=`$YAJL_CONFIG --modversion`
      if expr $yajl_version \>= 1.0.0 >/dev/null; then
        AC_MSG_RESULT([checking for Yajl... (version $yajl_version) yes])
        YAJL_LIBS=`$YAJL_CONFIG --libs | sed  s/-O.\//`
        YAJL_CFLAGS=`$YAJL_CONFIG --cflags | sed  s/-O.\//`
      else
        dnl second Test for libpng
        if test -z "$YAJL_LIBS"; then
          AC_CHECK_LIB(yajl, yajl_tree_parse,
          [AC_CHECK_HEADER(yajl/yajl_tree.h,
       	   YAJL_LIBS='-lyajl',
          [AC_MSG_WARN(*** OpenICC-config can not compile (YAJL header files not found) ***)])],
          [AC_MSG_WARN(*** OpenICC-config can not link (YAJL library not found) ***)])
        fi
      fi
    fi



X_LIBS="$LDFLAGS $X_LIBS"
X_CFLAGS="$CFLAGS $X_CFLAGS"

CFLAGS="$openicc_save_CFLAGS"
LDFLAGS="$openicc_save_LDFLAGS"
LIBS="$openicc_save_LIBS"
PKG_CONFIG_PATH="$openicc_save_PKG_CONFIG_PATH"

AC_PATH_PROGS(RPMBUILD, rpm, :)

AC_SUBST(OPENICC_MAJOR_VERSION)
AC_SUBST(OPENICC_MINOR_VERSION)
AC_SUBST(OPENICC_MICRO_VERSION)
AC_SUBST(OPENICC_VERSION)
AC_SUBST(OPENICC_VERSION_NAME)
AC_SUBST(VERSION)
AC_SUBST(prefix)
AC_SUBST(programincludedir)
AC_SUBST(programsysconfdir)
AC_SUBST(X_LIBS)
AC_SUBST(X_CFLAGS)
AC_SUBST(YAJL_LIBS)
AC_SUBST(YAJL_CFLAGS)

dnl Output the Makefiles
AC_OUTPUT(
Makefile
openicc.pc
src/include/openicc_version.h
)

echo ""
echo "================================================================="
echo "              Configuration Results"
echo ""
echo "OpenICC Version $PROGRAM_VERSION"
echo ""
if test "$YAJL_LIBS"; then
echo "   YAJL                         yes    yajl $yajl_version"
else
echo "   YAJL                         no     !! OpenICC will not build without !!"
fi

if test "$DEBUGFLAG"; then
echo ""
echo "Paths:"
echo "   prefix:              $prefix"
echo "   libdir:              $libdir"
echo ""
echo "Compiler options:"
echo "   X_CFLAGS:            $X_CFLAGS"
echo "   X_LIBS:              $X_LIBS"
fi
echo "================================================================="
echo ""

if test "$ERROR"; then
  AC_MSG_ERROR(
!!! An ERROR occured !!!
    Please check the above messages to see why.
    For bug reports please include the complete above output.
)
else
echo "Now type 'make' and 'make install' to build the package and "
echo "install."
fi
