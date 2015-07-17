include $(top_srcdir)/glib-tap.mk

AM_CFLAGS = -g $(LIBGRSS_CFLAGS)
LDADD = $(top_builddir)/src/libgrss-@LIBGRSS_API_VERSION@.la $(LIBGRSS_LIBS)
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	$(NULL)

dist_test_data = \
	test.atom.xml \
	test.rss.xml \
	$(NULL)

test_programs = \
	channel \
	formatter \
	$(NULL)

-include $(top_srcdir)/git.mk
