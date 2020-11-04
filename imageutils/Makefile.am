AM_CXXFLAGS = -Wall -I$(top_srcdir)/imagesource/ -I$(top_srcdir)/pixbufthumbnail -I$(top_srcdir)/profilemanager -I$(top_srcdir)/support $(LCMS_CFLAGS) $(GTK2_CFLAGS) ${PNM_CFLAGS}

noinst_LTLIBRARIES = libimageutils.la

libimageutils_la_SOURCES =	\
	maskpixbuf.cpp	\
	maskpixbuf.h	\
	rotatepixbuf.cpp	\
	rotatepixbuf.h	\
	\
	cachedimage.cpp	\
	cachedimage.h	\
	\
	imagesaver.h	\
	jpegsave.cpp	\
	jpegsave.h	\
	tiffsave.cpp	\
	tiffsave.h

libimageutils_la_LDFLAGS = -static

