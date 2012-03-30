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
bin_PROGRAMS = \
        openicc-device

lib_LTLIBRARIES = libOpenICC.la


libOpenICC_la_SOURCES = \
	src/openicc_config.c \
	src/aglob.c \
	src/xdg_bds.c

libOpenICC_la_LIBADD = @YAJL_LIBS@
libOpenICC_la_LDFLAGS = -version-info @LIBTOOL_VERSION@
libOpenICC_la_CPPFLAGS = @X_CFLAGS@ @YAJL_CFLAGS@

libOpenICCincludedir = $(includedir)/OpenICC

libOpenICCinclude_HEADERS = \
	OpenICC/openicc_config.h \
	OpenICC/openicc_version.h

INCLUDES = -I$(top_builddir) -I$(top_builddir)/OpenICC \
	-I$(top_srcdir)/OpenICC -I$(top_srcdir)/src @X_CFLAGS@ @YAJL_CFLAGS@


openicc_config_read_LDADD = -L$(top_builddir) -lOpenICC
openicc_config_read_SOURCES = \
	examples/openicc_config_read.c

openicc_device_LDADD = -L$(top_builddir) -lOpenICC
openicc_device_SOURCES = \
	utils/openicc_device.c


EXTRA_DIST = \
	src/openicc_config_internal.h \
	src/xdg_bds.h \
	src/aglob.h \
	utils/openicc_macros.h \
	OpenICC_device_config_DB.json \
	makefile.simple \
	bootstrap.sh

