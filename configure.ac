#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([photoprint],[0.4.2-pre2],[blackfive@fakenhamweb.co.uk])
AC_CONFIG_AUX_DIR([scripts])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_SRCDIR([photoprint.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE([1.9 foreign])

AM_GNU_GETTEXT_VERSION([0.20])
AM_GNU_GETTEXT([external])

# Checks for programs.
LT_INIT
AC_PROG_CXX
AC_PROG_CC
AC_PROG_MAKE_SET
AC_PROG_CPP

# Are we building for Win32?
case $host_os in
	*mingw32* | *msys* |  *-cygwin*)
		echo "Configuring for mingw, msys or cygwin"
		LIBS="-lwinspool $LIBS -mwindows"
		;;
	*)
		echo "Assuming Linux"
		PKG_CHECK_MODULES(X11, [x11])
		LIBS="$LIBS -lX11"
		;;
esac

# Checks for libraries.
PKG_CHECK_MODULES([GLIB2], [glib-2.0])
PKG_CHECK_MODULES([GTHREAD], [gthread-2.0])
PKG_CHECK_MODULES([GTK2], [gtk+-2.0])
PKG_CHECK_MODULES([GDKPIXBUF2], [gdk-pixbuf-2.0])
PKG_CHECK_MODULES([GP], [gutenprint])
PKG_CHECK_MODULES([LCMS], [lcms])
AC_CHECK_LIB(m,pow, LIBM_LIBS="-lm")
AC_SUBST([LIBM_LIBS])
AC_CHECK_LIB([pnm], [pnm_readpaminit],,,[-lm])
AC_CHECK_LIB([netpbm], [pnm_readpaminit],,,[-lm])

# Some distributions have the pam.h header in a netpbm subfolder
AC_CHECK_HEADERS([netpbm/pam.h])

AC_CHECK_LIB([tiff], [TIFFOpen], [TIFF_LIBS="-ltiff"],
	[unset ac_cv_lib_tiff_TIFFOpen
		AC_CHECK_LIB([tiff], [TIFFOpen],[TIFF_LIBS="-ltiff"],
			AC_CHECK_LIB([tiff3], [TIFFOpen],[TIFF_LIBS="-ltiff3"],
				AC_MSG_FAILURE([libtiff or devel files not found])), [-lm])
])
AC_SUBST([TIFF_LIBS])
AC_CHECK_LIB([jpeg], [jpeg_read_header],,
	AC_CHECK_LIB([jpeg62], [jpeg_read_header],JPEG_LIBS="-ljpeg62",AC_MSG_FAILURE([libjpeg or devel files not found])))
AC_SUBST([JPEG_LIBS])

AC_CHECK_LIB([pthread], [pthread_attr_init])
AC_CHECK_LIB([pthreadGC2], [pthread_attr_init],,)

AC_CHECK_LIB([gnugetopt], [getopt_long], [GETOPT_LIBS=-lgnugetopt])
AC_SUBST([GETOPT_LIBS])

AC_CHECK_LIB(socket,socket,
if test "$uname" != "IRIX"; then
	LIBS="-lsocket $LIBS"
else
	echo "Not using -lsocket since you are running IRIX."
fi)
AC_CHECK_LIB(nsl,gethostbyaddr,
if test "$uname" != "IRIX"; then
	LIBS="-lnsl $LIBS"
else
	echo "Not using -lnsl since you are running IRIX."
fi)

AC_CHECK_LIB(cups,httpConnect)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h stdlib.h string.h unistd.h])


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_STAT
AC_CHECK_FUNCS([memset mkdir strcasecmp strdup strncasecmp])

# Build less verbose
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES(yes)])
if make --help 2>&1 | grep -q no-print-directory; then
	AM_MAKEFLAGS="$AM_MAKEFLAGS --no-print-directory";
fi
AC_SUBST(AM_MAKEFLAGS)

AC_CONFIG_FILES([Makefile po/Makefile.in
                 imagesource/Makefile
                 imageutils/Makefile
                 miscwidgets/Makefile
                 support/Makefile
                 profilemanager/Makefile
                 pixbufthumbnail/Makefile
                 effects/Makefile
                 stpui_widgets/Makefile
                 stp_support/Makefile
                 gp_cppsupport/Makefile
                 splashscreen/Makefile
                 shortcuts/Makefile
                 borders/Makefile])
AC_OUTPUT
