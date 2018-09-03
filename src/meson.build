gnome = import('gnome')
pkgconfig = import('pkgconfig')

marshal = gnome.genmarshal('feed-marshal',
  prefix: 'feed_marshal',
  sources: ['feed-marshal.list']
)

libgrss_private_headers = [
  'feed-atom-handler.h',
  'feed-handler.h',
  'feed-rss-handler.h',
  'feed-pie-handler.h',
  'feeds-group-handler.h',
  'feeds-opml-group-handler.h',
  'feeds-pubsubhubbub-subscriber.h',
  'feeds-rsscloud-subscriber.h',
  'feeds-subscriber-handler.h',
  'feeds-subscriber-private.h',
  'feeds-xbel-group-handler.h',
  'feeds-xoxo-group-handler.h',
  'ns-handler.h',
  'utils-h',
]

libgrss_public_headers = [
  'libgrss.h',
  'person.h',
  'feed-atom-formatter.h',
  'feed-channel.h',
  'feed-enclosure.h',
  'feed-formatter.h',
  'feed-item.h',
  'feed-parser.h',
  'feed-rss-formatter.h',
  'feeds-group.h',
  'feeds-pool.h',
  'feeds-publisher.h',
  'feeds-store.h',
  'feeds-subscriber.h',
]

install_headers(libgrss_public_headers)

libgrss_public_sources = [
  'person.c',
  'feed-atom-formatter.c',
  'feed-channel.c',
  'feed-enclosure.c',
  'feed-formatter.c',
  'feed-item.c',
  'feed-parser.c',
  'feed-rss-formatter.c',
  'feeds-group.c',
  'feeds-pool.c',
  'feeds-publisher.c',
  'feeds-store.c',
  'feeds-subscriber.c',
]

libgrss_private_sources = [
  marshal,
  'feed-atom-handler.c',
  'feed-handler.c',
  'feed-rss-handler.c',
  'feed-pie-handler.c',
  'feeds-group-handler.c',
  'feeds-opml-group-handler.c',
  'feeds-pubsubhubbub-subscriber.c',
  'feeds-rsscloud-subscriber.c',
  'feeds-subscriber-handler.c',
  'feeds-xbel-group-handler.c',
  'feeds-xoxo-group-handler.c',
  'ns-handler.c',
  'utils.c',
]

libgrss_inc = include_directories('.')

libgrss_deps = [
  dependency('libsoup-2.4', version: '>=2.48.0'),
  dependency('glib-2.0', version: '>=2.42.1'),
  dependency('libxml-2.0', version: '>=2.9.2'),
]

libgrss = library('grss-@0@.@1@'.format(grss_version_major, grss_version_minor),
  libgrss_public_sources + libgrss_private_sources,
  dependencies: libgrss_deps,
  install: true,
)

libgrss_dep = declare_dependency(
  dependencies: libgrss_deps,
  link_with: libgrss,
)

pkgconfig.generate(
  description: 'GObject RSS handling library',
  libraries: [libgrss],
  version: meson.project_version(),
  name: 'Grss',
  requires: ['libsoup-2.4', 'glib-2.0', 'libxml-2.0'],
  filebase: 'libgrss',
)

if get_option('with_introspection')
  libgrss_gir = gnome.generate_gir(
    libgrss,
    sources: libgrss_public_sources + libgrss_public_headers,
    nsversion: '@0@.@1@'.format(grss_version_major, grss_version_minor),
    namespace: 'Grss',
    symbol_prefix: 'grss',
    includes: ['GLib-2.0', 'libxml2-2.0', 'Soup-2.4'],
    install: true,
  )

  gnome.generate_vapi(
    '@0@-@1@.@2@'.format(meson.project_name(), grss_version_major, grss_version_minor),
    sources: libgrss_gir[0],
    packages: ['glib-2.0', 'libsoup-2.4', 'libxml2-2.0'],
    install: true,
    metadata_dirs: [meson.current_source_dir()],
  )
endif

subdir('tests')