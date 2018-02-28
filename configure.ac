dnl#                                               -*- Autoconf -*-
dnl# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])dnl

m4_define([gnomebt_version_major],[3])dnl
m4_define([gnomebt_version_minor],[10])dnl
m4_define([gnomebt_version_micro],[0])dnl

AC_INIT([gnome-bluetooth],
        [gnomebt_version_major.gnomebt_version_minor.gnomebt_version_micro],
        [http://bugzilla.gnome.org/enter_bug.cgi?product=gnome-bluetooth])dnl

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([applet/bluetooth-applet.c])
AC_CONFIG_HEADERS([config.h])dnl

AC_CANONICAL_TARGET
AC_OBJEXT
AC_EXEEXT
AC_USE_SYSTEM_EXTENSIONS
AC_SYS_INTERPRETER
AX_SYS_LARGEFILE_SENSITIVE

AM_INIT_AUTOMAKE([1.8 dist-xz no-dist-gzip check-news tar-ustar])dnl

# Before making a release, the GNOMEBT_LT_VERSION string should be modified.
# The string is of the form C:R:A.
# - If interfaces have been changed or added, but binary compatibility has
#   been preserved, change to C+1:0:A+1
# - If binary compatibility has been broken (e.g. removed or changed
#   interfaces), change to C+1:0:0
# - If the interface is the same as the previous version, change to C:R+1:A
GNOMEBT_LT_VERSION=12:0:0
AC_SUBST([GNOMEBT_LT_VERSION])dnl

GNOME_MAINTAINER_MODE_DEFINES dnl# calls AM_MAINTAINER_MODE

# Support silent build rules, requires at least automake-1.11. Disable
# by either passing --disable-silent-rules to configure or passing V=1
# to make
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])dnl

# Checks for programs.
AC_LANG([C])
AM_PROG_AR
AC_BAKEFILE_CREATE_FILE_DLLAR_SH
if test "x${CC}" = "x"; then
	test -z "${CC}"
	AC_PROG_CC
else
	test ! -z "${CC}" && export CC
	AC_SUBST([CC])
fi
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S

dnl# Initialize libtool
AC_DISABLE_STATIC([])
LT_INIT([disable-static dlopen win32-dll])dnl

# i18n stuff
IT_PROG_INTLTOOL([0.40.0])dnl

GETTEXT_PACKAGE=gnome-bluetooth2
AC_SUBST([GETTEXT_PACKAGE])dnl
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE],["$GETTEXT_PACKAGE"],
                   [The name of the gettext domain])dnl

GLIB_GSETTINGS

if test "x${XMLLINT}" = "x"; then
	test -z "${XMLLINT}"
	AC_PATH_PROG([XMLLINT],[xmllint])
else
	test ! -z "${XMLLINT}"
fi

dnl# gtk-doc checks
GTK_DOC_CHECK([1.9])dnl

AC_DEFINE_UNQUOTED([LOCALEDIR],["${prefix}/share/locale"],
                   [Directory for the localization files])dnl

AC_ARG_ENABLE([desktop-update],
              [AS_HELP_STRING([--disable-desktop-update],
                              [Disable desktop database update])])
if (test "$enable_desktop_update" != no); then
	AC_PATH_PROG([UPDATE_DESKTOP_DATABASE],[update-desktop-database])
fi
AM_CONDITIONAL([DESKTOP_UPDATE],[test -n "$UPDATE_DESKTOP_DATABASE"])dnl

AC_ARG_ENABLE([icon-update],
              [AS_HELP_STRING([--disable-icon-update],
                              [Disable icon cache update])])
if (test "$enable_icon_update" != no); then
	AC_PATH_PROG([UPDATE_ICON_CACHE],[gtk-update-icon-cache])
fi
AM_CONDITIONAL([ICON_UPDATE],[test -n "$UPDATE_ICON_CACHE"])dnl

GTK_REQUIRED=2.91.3
GLIB_REQUIRED=2.36.0
NOTIFY_REQUIRED=0.7.0
NAUTILUS_SENDTO_REQUIRED=2.29.0
NAUTILUS_SENDTO_MAX_REQUIRED=3.7.0

dnl# Requires for the sendto app
PKG_CHECK_MODULES([SENDTO],
                  [gtk+-3.0 gio-2.0])dnl

dnl# Requires for the wizard dialogue
PKG_CHECK_MODULES([WIZARD],
                  [gmodule-export-2.0 gtk+-3.0])dnl

# Checks for libraries.
AC_CHECK_LIB([c],[printf])
AC_CHECK_LIB([m],[sqrt])
LT_LIB_M
AC_SUBST([LIBM])dnl

AC_DEFUN([AC_POSSIBLY_DUPLICATED_MACROS],[
  AC_REQUIRE([AC_PROG_SED])dnl
  AC_REQUIRE([AC_PATH_XTRA])dnl
  AC_REQUIRE([AC_HEADER_STDC])dnl
  AC_REQUIRE([gl_THREADLIB])dnl
])dnl
AC_POSSIBLY_DUPLICATED_MACROS

dnl# Requires for the input helper
PKG_CHECK_MODULES([INPUT],
                  [xi x11 gtk+-3.0])dnl

dnl# Requires for the public library
PKG_CHECK_MODULES([LIBGNOMEBT],
                  [gmodule-2.0 gio-unix-2.0 gtk+-3.0])dnl

dnl# Requires for the geoclue plugin
PKG_CHECK_MODULES([GEOCLUE],
                  [gio-2.0 gtk+-3.0])dnl

dnl# Requires for the plugins
PKG_CHECK_MODULES([PLUGINS],
                  [gtk+-3.0])dnl

GDBUS_CODEGEN="gdbus-codegen"
AC_SUBST([GDBUS_CODEGEN])dnl

GOBJECT_INTROSPECTION_CHECK([0.9.5])dnl

GNOME_DEBUG_CHECK
GNOME_COMPILE_WARNINGS([maximum])
GNOME_CXX_WARNINGS
GNOME_MAINTAINER_MODE_DEFINES

AC_ARG_ENABLE([documentation],
	[AS_HELP_STRING([--enable-documentation],[enable man pages and HTML])],
	[],[enable_documentation=yes])
AM_CONDITIONAL([ENABLE_DOCUMENTATION],[test x$enable_documentation = xyes])dnl

YELP_HELP_INIT

# Checks for header files.
AC_CHECK_HEADERS([asm/types.h fcntl.h linux/types.h stdio.h \
                  sys/ioctl.h sys/time.h])dnl

# Checks for typedefs, structures, and compiler characteristics.
gl_VISIBILITY
AC_C_BIGENDIAN
AC_C_INLINE
AC_C_PROTOTYPES
AC_TYPE_SSIZE_T
AC_TYPE_UINT8_T
AC_TYPE_UINT32_T
AC_CHECK_TYPES([__u8])
AC_CHECK_TYPES([__u32])dnl

# Checks for library functions.
AC_CHECK_FUNCS([fprintf gettimeofday memset pow strstr strtoul])dnl

# Output.
AC_CONFIG_FILES([Makefile
	  gnome-bluetooth-1.0.pc
	  lib/Makefile
	  lib/plugins/Makefile
	  applet/Makefile
	  wizard/Makefile
	  wizard/bluetooth-wizard.desktop.in
	  sendto/Makefile
	  sendto/bluetooth-sendto.desktop.in
	  docs/Makefile
	  docs/reference/Makefile
	  docs/reference/libgnome-bluetooth/Makefile
	  docs/reference/libgnome-bluetooth/version.xml
	  po/Makefile.in
	  help/Makefile
])
AC_OUTPUT
dnl#FIXME: what happened to the icons subdir?!

echo "
Configure summary:

	Compiler....................:  ${CC}
	Compiler Flags..............:  ${CFLAGS}
	Prefix......................:  ${prefix}
	Documentation...............:  ${enable_gtk_doc}
	GObject-Introspection.......:  ${found_introspection}
"

