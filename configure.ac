# bump micro_version to the next even number for each point release
# bump micro_version to the next odd number after each release
m4_define([grss_major_version], [0])
m4_define([grss_minor_version], [7])
m4_define([grss_micro_version], [1])
m4_define([grss_version],
          [grss_major_version.grss_minor_version.grss_micro_version])

# bump up by 1 for every micro release with no API changes, otherwise
# set to 0. after release, bump up by 1
m4_define([grss_interface_age], [1])
m4_define([grss_binary_age], [m4_eval(100 * grss_minor_version + grss_micro_version)])

m4_define([lt_current], [m4_eval(100 * grss_minor_version + grss_micro_version - grss_interface_age)])
m4_define([lt_revision], [grss_interface_age])
m4_define([lt_age], [m4_eval(grss_binary_age - grss_interface_age)])

AC_PREREQ(2.62)

AC_INIT([libgrss],
        [grss_version],
        [https://bugzilla.gnome.org/enter_bug.cgi?product=libgrss],
        [libgrss],
        [https://wiki.gnome.org/Projects/Libgrss])

AC_CONFIG_HEADERS(config.h)

AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz tar-ustar])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_REQUIRE_AUX_FILE([tap-driver.sh])

LT_PREREQ([2.2.6])
LT_INIT([disable-static])

AC_PROG_CC

dnl === Versioning ============================================================

GRSS_MAJOR_VERSION=grss_major_version
GRSS_MINOR_VERSION=grss_minor_version
GRSS_MICRO_VERSION=grss_micro_version
GRSS_VERSION=grss_version
AC_SUBST(GRSS_MAJOR_VERSION)
AC_SUBST(GRSS_MICRO_VERSION)
AC_SUBST(GRSS_MINOR_VERSION)
AC_SUBST(GRSS_VERSION)

GRSS_LT_CURRENT=lt_current
GRSS_LT_REVISION=lt_revision
GRSS_LT_AGE=lt_age
GRSS_LT_VERSION="$GRSS_LT_CURRENT:$GRSS_LT_REVISION:$GRSS_LT_AGE"
GRSS_LT_LDFLAGS="-version-info $GRSS_LT_VERSION"

AC_SUBST(GRSS_LT_LDFLAGS)

# FIXME: enable in the future
#AX_COMPILER_FLAGS([WARN_CFLAGS],[WARN_LDFLAGS])

AC_ARG_ENABLE(deprecation_flags,
              [AC_HELP_STRING([--enable-deprecation-flags],
                              [use *_DISABLE_DEPRECATED flags @<:@default=no@:>@])],,
              [enable_deprecation_flags=no])

if test "x$enable_deprecation_flags" = "xyes"; then
   DISABLE_DEPRECATED_CFLAGS=$DISABLE_DEPRECATED
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
fi

AC_PATH_PROG(PKG_CONFIG, pkg-config)
AC_PATH_PROG([GLIB_GENMARSHAL], [glib-genmarshal])
AC_PATH_PROG([GLIB_MKENUMS], [glib-mkenums])
AC_CHECK_FUNCS([strptime localtime_r])

AC_CHECK_LIBM
AC_SUBST(LIBM)

PKG_CHECK_MODULES([LIBGRSS],[
	glib-2.0 >= 2.42.1
	libsoup-2.4 >= 2.48.0
	libxml-2.0 >= 2.9.2
])

##################################################
# Checks for gtk-doc and docbook-tools
##################################################
GTK_DOC_CHECK([1.9])

GOBJECT_INTROSPECTION_CHECK([1.30.0])

GLIB_TESTS

AC_OUTPUT([
	Makefile
	src/Makefile
	src/tests/Makefile
	doc/Makefile
	doc/reference/Makefile
	doc/reference/version.xml
	libgrss.pc
])

dnl ---------------------------------------------------------------------------
dnl - Show summary
dnl ---------------------------------------------------------------------------

echo "
              libgrss $VERSION
              `echo libgrss $VERSION | sed "s/./=/g"`

        prefix:                       ${prefix}
        source code location:         ${srcdir}
        compiler:                     ${CC}
        cflags:                       ${CFLAGS}
        Maintainer mode:              ${USE_MAINTAINER_MODE}
        Use *_DISABLE_DEPRECATED:     ${enable_deprecation_flags}

        Build introspection support:  ${found_introspection}
        Build gtk-doc documentation:  ${enable_gtk_doc}

"
