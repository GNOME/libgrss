project('libgrss', 'c',
        version: '0.7.1',
        license: 'LGPLv3')

version_arr = meson.project_version().split('.')
grss_version_major = version_arr[0].to_int()
grss_version_minor = version_arr[1].to_int()
grss_version_micro = version_arr[2].to_int()

cc = meson.get_compiler('c')
cc.has_function('strptime')
cc.has_function('localtime_r')

subdir('src')
subdir('doc')

message('\n' +
  '     libgrss ' + meson.project_version() + '\n' +
  '       prefix:                   ' + get_option('prefix') + '\n' +
  '       source code location:     ' + meson.source_root()
)