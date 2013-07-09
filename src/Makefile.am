NULL =

INCLUDES = \
	-I$(top_srcdir)/src         \
	-DPREFIX=\""$(prefix)"\"    \
	-DLIBDIR=\""$(libdir)"\"    \
	-DG_DISABLE_DEPRECATED      \
	$(LIBGRSS_CFLAGS)           \
	$(NULL)

LDADD = $(LIBGRSS_LT_LDFLAGS) -export-dynamic -rpath $(libdir)

sources_private_h = \
	feed-atom-handler.h             \
	feed-handler.h                  \
	feed-rss-handler.h              \
	feed-pie-handler.h              \
	feeds-group-handler.h           \
	feeds-opml-group-handler.h      \
	feeds-pubsubhubbub-subscriber.h \
	feeds-rsscloud-subscriber.h     \
	feeds-subscriber-handler.h      \
	feeds-subscriber-private.h	\
	feeds-xbel-group-handler.h      \
	feeds-xoxo-group-handler.h      \
	ns-handler.h                    \
	utils.h

sources_public_h = \
	libgrss.h                       \
	feed-channel.h                  \
	feed-enclosure.h                \
	feed-item.h                     \
	feed-parser.h                   \
	feeds-group.h                   \
	feeds-pool.h                    \
	feeds-publisher.h               \
	feeds-store.h                   \
	feeds-subscriber.h

sources_public_c = \
	feed-channel.c                  \
	feed-enclosure.c                \
	feed-item.c                     \
	feed-parser.c                   \
	feeds-group.c                   \
	feeds-pool.c                    \
	feeds-publisher.c               \
	feeds-store.c                   \
	feeds-subscriber.c

sources_private_c = \
	$(marshal_source)               \
	feed-atom-handler.c             \
	feed-handler.c                  \
	feed-rss-handler.c              \
	feed-pie-handler.c              \
	feeds-group-handler.c           \
	feeds-opml-group-handler.c      \
	feeds-pubsubhubbub-subscriber.c \
	feeds-rsscloud-subscriber.c     \
	feeds-subscriber-handler.c      \
	feeds-xbel-group-handler.c      \
	feeds-xoxo-group-handler.c      \
	ns-handler.c                    \
	utils.c

marshal_source = \
	feed-marshal.h      \
	feed-marshal.c

feed-marshal.h: feed-marshal.list
	$(GLIB_GENMARSHAL) $< --prefix=feed_marshal --header > $@

feed-marshal.c: feed-marshal.list
	echo "#include \"feed-marshal.h\"" > $@ &&    \
		$(GLIB_GENMARSHAL) $< --prefix=feed_marshal --body >> $@

lib_LTLIBRARIES = libgrss-0.5.la

libgrss_0_5_la_LIBADD = $(LIBGRSS_LIBS)
libgrss_0_5_la_SOURCES = \
	$(sources_public_h) \
	$(sources_private_h) \
	$(sources_public_c) \
	$(sources_private_c) \
	$(NULL)
libgrss_0_5_la_LDFLAGS = $(LDADD)

libgrss_0_5_ladir = $(includedir)/libgrss-@LIBGRSS_API_VERSION@
libgrss_0_5_la_HEADERS = $(sources_public_h)

CLEANFILES = $(pcfile_DATA)

BUILT_SOURCES = \
	$(marshal_source)   \
	$(NULL)

EXTRA_DIST = \
	feed-marshal.list   \
	$(NULL)

-include $(INTROSPECTION_MAKEFILE)
INTROSPECTION_GIRS =
INTROSPECTION_SCANNER_ARGS = --add-include-path=$(srcdir) $(GIFLAGS)
INTROSPECTION_COMPILER_ARGS = --includedir=$(srcdir)

if HAVE_INTROSPECTION
introspection_sources = \
	$(sources_public_h) \
	$(sources_public_c) \
	$(NULL)

Grss-0.5.gir: libgrss-0.5.la
Grss_0_5_gir_INCLUDES = GObject-2.0 Gio-2.0 Soup-2.4 libxml2-2.0
Grss_0_5_gir_CFLAGS = $(INCLUDES)
Grss_0_5_gir_LIBS = libgrss-0.5.la
Grss_0_5_gir_FILES = $(introspection_sources)
INTROSPECTION_GIRS += Grss-0.5.gir

girdir = $(datadir)/gir-0.5
gir_DATA = $(INTROSPECTION_GIRS)

typelibdir = $(libdir)/girepository-0.5
typelib_DATA = $(INTROSPECTION_GIRS:.gir=.typelib)

CLEANFILES += $(gir_DATA) $(typelib_DATA)
endif
