if get_option('enable_gtk_doc')
  gnome = import('gnome')

  version_in = configuration_data()
  version_in.set('GRSS_VERSION', meson.project_version())
  version_xml = configure_file(
    input: 'version.xml.in',
    output: 'version.xml',
    configuration: version_in,
  )

  glib_prefix = dependency('glib-2.0').get_pkgconfig_variable('prefix')
  glib_docpath = join_paths(glib_prefix, 'share', 'gtk-doc', 'html')
  docpath = join_paths(get_option('datadir'), 'gtk-doc', 'html')

  gnome.gtkdoc(
    meson.project_name(),
    main_xml: meson.project_name() + '-docs.sgml',
    dependencies: libgrss_dep,
    scan_args: [
      '--rebuild-types',
      '--rebuild-sections',
    ],
    gobject_typesfile: 'libgrss.types',
    src_dir: [
      join_paths(meson.source_root(), 'src'),
      join_paths(meson.build_root(), 'src'),
    ],
    fixxref_args: [
      '--html-dir=@0@'.format(docpath),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'glib')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gobject')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gio')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gi')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'gtk3')),
      '--extra-dir=@0@'.format(join_paths(glib_docpath, 'libxml2')),
    ],
    content_files: version_xml,
    install: true,
  )
endif