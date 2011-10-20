# autoconf macros
ACLOCAL_AMFLAGS = -I m4


doc_DATA =  COPYING \
            README \
            AUTHORS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = openicc.pc

# targets
check_PROGRAMS = \
        openicc-config-read

lib_LTLIBRARIES = libOpenICC.la


libOpenICC_la_SOURCES = \
        src/openicc_config.c

libOpenICC_la_LIBADD = @YAJL_LIBS@
libOpenICC_la_LDFLAGS = -version-info @LIBTOOL_VERSION@
libOpenICC_la_CPPFLAGS = @X_CFLAGS@ @YAJL_CFLAGS@

libOpenICCincludedir = $(includedir)/OpenICC

libOpenICCinclude_HEADERS = \
	OpenICC/openicc_config.h

INCLUDES = -I$(top_builddir) -I$(top_srcdir)/OpenICC


openicc_config_read_LDADD = -L$(top_builddir) -lOpenICC
openicc_config_read_SOURCES = \
	examples/openicc_config_read.c


EXTRA_DIST = OpenICC_device_config_DB.json \
             makefile.simple \
             bootstrap.sh

