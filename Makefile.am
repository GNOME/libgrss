ACLOCAL_AMFLAGS = -I m4
SUBDIRS = src doc
EXTRA_DIST = autogen.sh

pcfiledir = $(libdir)/pkgconfig
pcfile_DATA = libgrss-0.pc

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

libgrss-0.pc: libgrss.pc
	@cp -f $< $@
