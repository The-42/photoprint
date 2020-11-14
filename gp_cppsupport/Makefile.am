AM_CFLAGS = -Wall
AM_CXXFLAGS = -Wall -I$(top_srcdir) $(GP_CFLAGS) $(GTK2_CFLAGS) -DGLIB_VERSION_MIN_REQUIRED=GLIB_VERSION_2_30

noinst_LTLIBRARIES = libgp_cppsupport.la

libgp_cppsupport_la_SOURCES = 		\
	gprintersettings.cpp	\
	gprintersettings.h	\
	gpdevicensupport.cpp \
	gpdevicensupport.h \
	printoutput.cpp	\
	printoutput.h	\
	printerqueueswrapper.cpp	\
	printerqueueswrapper.h	\
	gprinter.cpp	\
	gprinter.h	\
	printoutputselector.cpp \
	printoutputselector.h


libgp_cppsupport_la_LDFLAGS = -static

EXTRA_DIST = 
