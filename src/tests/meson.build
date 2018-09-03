grss_tests = [
  'channel',
  'formatter',
]

test_env = [
  'G_TEST_SRCDIR=@0@'.format(meson.current_source_dir()),
  'G_TEST_BUILDDIR=@0@'.format(meson.current_build_dir()),
  'G_DEBUG=gc-friendly',
  'GSETTINGS_BACKEND=memory',
  'PYTHONDONTWRITEBYTECODE=yes',
  'MALLOC_CHECK_=2',
#  'MALLOC_PERTURB_=$((${RANDOM:-256} % 256))',
]

foreach grss_test : grss_tests
  exe = executable(grss_test,
    ['@0@.c'.format(grss_test)],
    dependencies: libgrss_dep,
    include_directories: libgrss_inc,
  )

  test(grss_test, exe, env: test_env)
endforeach