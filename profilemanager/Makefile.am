
localedir=$(datadir)/locale

AM_CXXFLAGS = -DLOCALEDIR=\"$(localedir)\" -Wall -I$(top_srcdir)/ -I$(top_srcdir)/imagesource/ -I$(top_srcdir)/support/ $(LCMS_CFLAGS) $(GTK2_CFLAGS)

noinst_LTLIBRARIES = libprofilemanager.la

libprofilemanager_la_SOURCES =	\
	lcmswrapper.cpp	\
	lcmswrapper.h	\
	profilemanager.cpp	\
	profilemanager.h	\
	profileselector.cpp	\
	profileselector.h	\
	intentselector.cpp	\
	intentselector.h	\
	ciesupport.cpp \
	ciesupport.h

libprofilemanager_la_LDFLAGS = -static

