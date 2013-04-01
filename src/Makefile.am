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

sources_c = \
	$(marshal_source)               \
	feed-atom-handler.c             \
	feed-channel.c                  \
	feed-enclosure.c                \
	feed-handler.c                  \
	feed-item.c                     \
	feed-parser.c                   \
	feed-rss-handler.c              \
	feed-pie-handler.c              \
	feeds-group.c                   \
	feeds-group-handler.c           \
	feeds-opml-group-handler.c      \
	feeds-pool.c                    \
	feeds-publisher.c               \
	feeds-store.c                   \
	feeds-pubsubhubbub-subscriber.c \
	feeds-rsscloud-subscriber.c     \
	feeds-subscriber.c              \
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
	$(sources_c) \
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
