# configure.ac for osso-pdf-viewer

# Mandatory, inits autoconf
# NOTE Update version number to reflect debian/changelog (DON*T USE -x; unnecessary)
#
AC_INIT(osso-pdf-viewer, 1.2.26)

# Tests that source dir exists
AC_CONFIG_SRCDIR([src/main.c])

# For automake: generate rules to reproduce config.h, if needed
AM_CONFIG_HEADER(aconf.h)

#Initialize automake, takes package name and version from AC_INIT
AM_INIT_AUTOMAKE([foreign])

AM_PROG_CC_C_O
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LIBTOOL
AC_HEADER_STDC

dnl Enable these unconditionally.
AC_DEFINE([A4_PAPER], [1], [Default to A4 paper.])
#AC_DEFINE([OPI_SUPPORT], [1], [Generate OPI comments in PS output.])
AC_DEFINE([MULTITHREADED], [1], [Enable multithreading support.])
AC_DEFINE([TEXTOUT_WORD_LIST], [1], [Enable word list support.])
AC_DEFINE([USE_FIXEDPOINT], [0], [Enable fixed point arithmetic support.])

CFLAGS="$CFLAGS -Wall -Wmissing-prototypes -Wmissing-declarations"

CPPFLAGS ="-Wall -Wmissing-declarations"
CXXFLAGS += $(CPPFLAGS)

AC_MSG_CHECKING([whether to use maemo launcher])
AC_ARG_ENABLE([maemo-launcher],
          [AS_HELP_STRING([--enable-maemo-launcher],
          [default=yes])],
          [
           case "${enableval}" in
           yes) maemo_launcher=true
                AC_MSG_RESULT([yes])
            PKG_CHECK_MODULES(MAEMO_LAUNCHER, [maemo-launcher-app])
            AC_SUBST(MAEMO_LAUNCHER_CFLAGS)
            MAEMO_LAUNCHER_LIBS="-Xcompiler \"$MAEMO_LAUNCHER_LIBS\""
            AC_SUBST(MAEMO_LAUNCHER_LIBS)
            ;;
           no)  maemo_launcher=false
                    AC_MSG_RESULT([no])
            ;;
           *) AC_MSG_ERROR([
            "Bad value ${enableval} for --enable-maemo-launcher,\n"
            "give exactly 'yes' or 'no' value."]) ;;
              esac
         ], [maemo_launcher=false])


# Check module dependencies, will automatically inlude all needed
# compiler options to DEPS-variables, see src/Makefile.am for details
PKG_CHECK_MODULES(GDEPS, [
	gtk+-2.0 >= 2.2  
	glib-2.0 >= 2.2 
	gio-2.0,
	gconf-2.0 >= 2.6.4 
	comapp >= 0.1.1,
	x11]
)
AC_SUBST(GDEPS_CFLAGS)
AC_SUBST(GDEPS_LIBS)

# 'libossohelp' dependency
PKG_CHECK_MODULES(HILDON, [
	hildon-1 >= 0.8.1
	hildon-fm-2 >= 0.8.1 
	libosso >= 0.8.1 
	libhildonmime]
)
AC_SUBST(HILDON_CFLAGS)
AC_SUBST(HILDON_LIBS)

AC_MSG_CHECKING([for Bluetooth support])
conbtui=`pkg-config --cflags conbtdialogs 2> /dev/null`
AM_CONDITIONAL(HAVE_BLUETOOTH, test "x$conbtui" != "x")
if test "x$conbtui" != "x" ; then
   AC_MSG_RESULT(yes)
else
   AC_MSG_RESULT(no)
fi


# Localization-related
#AC_PROG_INTLTOOL([0.23])

GETTEXT_PACKAGE=osso-pdf-viewer
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Name of gettext package])
#ALL_LINGUAS="de_DE en_GB en_US es_ES es_MX fr_FR it_IT"
#ALL_LINGUAS="en_GB"
AM_GLIB_GNU_GETTEXT

localedir=${datadir}/locale
AC_SUBST(localedir)

pixmapsdir=${datadir}/pixmaps
AC_SUBST(pixmapsdir)

# Mime
dnl MIME-type registration check (2.6 / 2.8 version)
AC_PATH_PROG(UPDATE_MIME_DATABASE, update-mime-database, no)
AM_CONDITIONAL(HAVE_26_MIME, test x$UPDATE_MIME_DATABASE != xno)

AC_PATH_PROG(UPDATE_DESKTOP_DATABASE, update-desktop-database, no)
AM_CONDITIONAL(HAVE_28_MIME, test x$UPDATE_DESKTOP_DATABASE != xno)

# xpdf related checks
dnl #### Default values for Unix
LIBPREFIX="lib"
AR="ar"
UP_DIR=""

dnl #### Do substitutions
AC_SUBST(LIBPREFIX)
AC_SUBST(AR)
AC_SUBST(UP_DIR)

dnl #### check for freetype2
PKG_CHECK_MODULES(FREETYPE, freetype2)
AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

dnl #### Check for headers
AC_PATH_XTRA
AC_HEADER_DIRENT

dnl #### Check for library functions
AC_CHECK_FUNCS(popen mkstemp mkstemps)

dnl ##### Check for fseeko/ftello or fseek64/ftell64
dnl The LARGEFILE and FSEEKO macros have to be called in C, not C++, mode.
AC_SYS_LARGEFILE
AC_FUNC_FSEEKO
AC_CHECK_FUNCS(fseek64, xpdf_cv_func_fseek64=yes, xpdf_cv_func_fseek64=no)
AC_CHECK_FUNCS(ftell64, xpdf_cv_func_ftell64=yes, xpdf_cv_func_ftell64=no)
if test "$xpdf_cv_func_fseek64" = yes -a "$xpdf_cv_func_ftell64" = yes; then
	  AC_DEFINE(HAVE_FSEEK64)
fi

dbuslibpathdir=`pkg-config --variable=libdir dbus-1`
dbusconfdir=/etc/dbus-1/system.d
dbusservdir=/usr/share/dbus-1/services
AC_SUBST(dbuslibpathdir)
AC_SUBST(dbusconfdir)
AC_SUBST(dbusservdir)

# Define ouput files
AC_CONFIG_FILES([
  Makefile
  data/Makefile
  data/osso_pdfviewer.desktop
  data/com.nokia.osso_pdfviewer.service
  fofi/Makefile
  goo/Makefile
  splash/Makefile
  xpdf/Makefile
  src/Makefile
])
#  po/Makefile.in
#  po/porules.mk

# Mandatory, produce ouput
AC_OUTPUT
