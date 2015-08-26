# autoconf macros
ACLOCAL_AMFLAGS = -I m4


dist_doc_DATA = \
            COPYING \
            README \
            README.md \
            AUTHORS

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = openicc.pc

AM_CFLAGS = -I$(top_builddir) -I$(top_builddir)/src/include -I$(top_builddir)/src/libopenicc \
	-I$(top_srcdir)/src/include -I$(top_srcdir)/src -I$(top_srcdir)/src/libopenicc @X_CFLAGS@ @YAJL_CFLAGS@

# targets
lib_LTLIBRARIES = libOpenICC.la


libOpenICC_la_SOURCES = \
	src/libopenicc/openicc_config.c \
	src/libopenicc/aglob.c \
	src/libopenicc/xdg_bds.c

libOpenICC_la_LIBADD = @YAJL_LIBS@
libOpenICC_la_LDFLAGS = -version-info @LIBTOOL_VERSION@
libOpenICC_la_CPPFLAGS = @X_CFLAGS@ @YAJL_CFLAGS@

libOpenICCincludedir = $(includedir)/OpenICC

libOpenICCinclude_HEADERS = \
	src/include/openicc_config.h \
	src/include/openicc_version.h


check_PROGRAMS = \
        openicc-config-read \
	test-xdg
bin_PROGRAMS = \
        openicc-device

openicc_config_read_LDADD = -L$(top_builddir) libOpenICC.la
openicc_config_read_SOURCES = \
	src/examples/openicc_config_read.c

openicc_device_LDADD = -L$(top_builddir) libOpenICC.la
openicc_device_SOURCES = \
	src/tools/openicc_device.c

test_xdg_SOURCES = \
	src/libopenicc/aglob.c \
	src/libopenicc/xdg_bds.c
test_xdg_CPPFLAGS = -DSTANDALONE_TEST $(AM_CFLAGS)

EXTRA_DIST = \
	src/libopenicc/openicc_config_internal.h \
	src/libopenicc/xdg_bds.h \
	src/libopenicc/aglob.h \
	src/tools/openicc_macros.h \
	OpenICC_device_config_DB.json \
	makefile.simple \
	bootstrap.sh

test:	test-xdg
	./test-xdg

.PHONY:	test
