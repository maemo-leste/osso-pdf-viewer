# Makefile.am for libFoFi for XPDF engine

INCLUDES = -I$(top_srcdir)	\
	   -I$(top_srcdir)/goo	\
	   $(MAEMO_LAUNCHER_CFLAGS)

noinst_LIBRARIES = libfofi.a

libfofi_includedir = $(includedir)/fofi
libfofi_include_HEADERS = \
	FoFiBase.h		\
	FoFiEncodings.h		\
	FoFiTrueType.h		\
	FoFiType1.h		\
	FoFiType1C.h

libfofi_a_SOURCES = \
	FoFiBase.cc		\
	FoFiEncodings.cc 	\
	FoFiTrueType.cc 	\
	FoFiType1.cc 		\
	FoFiType1C.cc 
