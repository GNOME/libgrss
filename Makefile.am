ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src doc
EXTRA_DIST = autogen.sh
CLEANFILES = libgrss-$(LIBGRSS_API_VERSION).pc

pcfiledir = $(libdir)/pkgconfig
pcfile_DATA = libgrss-$(LIBGRSS_API_VERSION).pc

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

libgrss-$(LIBGRSS_API_VERSION).pc: libgrss.pc
	@cp -f $< $@
