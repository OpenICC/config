# autoconf macros
ACLOCAL_AMFLAGS = -I m4


dist_doc_DATA = \
            COPYING \
            README \
            README.md \
            AUTHORS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = openicc.pc

AM_CFLAGS = -I$(top_builddir) -I$(top_builddir)/OpenICC \
	-I$(top_srcdir)/OpenICC -I$(top_srcdir)/src @X_CFLAGS@ @YAJL_CFLAGS@

# targets
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


check_PROGRAMS = \
        openicc-config-read \
	test-xdg
bin_PROGRAMS = \
        openicc-device

openicc_config_read_LDADD = -L$(top_builddir) libOpenICC.la
openicc_config_read_SOURCES = \
	examples/openicc_config_read.c

openicc_device_LDADD = -L$(top_builddir) libOpenICC.la
openicc_device_SOURCES = \
	utils/openicc_device.c

test_xdg_SOURCES = \
	src/aglob.c \
	src/xdg_bds.c
test_xdg_CPPFLAGS = -DSTANDALONE_TEST $(AM_CFLAGS)

EXTRA_DIST = \
	src/openicc_config_internal.h \
	src/xdg_bds.h \
	src/aglob.h \
	utils/openicc_macros.h \
	OpenICC_device_config_DB.json \
	makefile.simple \
	bootstrap.sh

test:	test-xdg
	./test-xdg

.PHONY:	test
