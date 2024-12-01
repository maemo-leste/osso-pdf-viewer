# Makefile.am for OSSO PDF Viewer

if HAVE_BLUETOOTH
HAVE_BLUETOOTH_FLAGS=-DVERSION_3
endif

# These come from configure-script's module dependency checks
INCLUDES = $(CPPFLAGS)				\
	$(MAEMO_LAUNCHER_CFLAGS)	\
	$(HILDON_CFLAGS)		\
	$(FREETYPE_CFLAGS)		\
	$(GDEPS_CFLAGS) 		\
	-I./../xpdf			\
	-I./../splash			\
	-I./../goo			\
	-I./../fofi			\
	-DBONOBO_DISABLE_DEPRECATED	\
	-DBONOBO_UI_DISABLE_DEPRECATED 	\
	-DEEL_DISABLE_DEPRECATED	\
	-DGCONF_DISABLE_DEPRECATED 	\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_DISABLE_DEPRECATED	\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGNOME_DISABLE_DEPRECATED 	\
	-DGTK_DISABLE_DEPRECATED 	\
	-DLIBGLADE_DISABLE_DEPRECATED	\
	-g

bin_PROGRAMS = osso_pdfviewer

osso_pdfviewer_CFLAGS= 			\
	$(INCLUDES)			\
	$(HAVE_BLUETOOTH_FLAGS) \
	-DLOCALEDIR=\"$(localedir)\" 	\
	-DPIXMAPSDIR=\"$(pixmapsdir)\" 	\
	-DPREFIX=\"$(prefix)\"		\
	-I$(top_srcdir) 

# Note only OSSO PDF Viewer specific classes in xpdf/ are built here
osso_pdfviewer_SOURCES = 		\
	../xpdf/OssoOutputDev.cc 	\
	../xpdf/OssoOutputDev.h 	\
	../xpdf/OssoStream.cc 		\
	../xpdf/OssoStream.h 		\
	ui/callbacks.c ui/callbacks.h \
	ui/interface.c ui/interface.h ui/ui.h\
	pdfviewer.cc			\
	pdfviewer.h			\
	gtk-switch.h 			\
	appdata.h constant.h rpc.c rpc.h i18n.h main.c main.h \
	utility.c utility.h settings.c settings.h \
	configuration.c configuration.h \
	state_save.c state_save.h 	\
	thread_debug.h


XPDF_OBJS = \
	../xpdf/Annot.o 		\
	../xpdf/Array.o 		\
	../xpdf/BuiltinFont.o 		\
	../xpdf/BuiltinFontTables.o 	\
	../xpdf/Catalog.o 		\
	../xpdf/CharCodeToUnicode.o 	\
	../xpdf/CMap.o 			\
	../xpdf/Decrypt.o 		\
	../xpdf/Dict.o 			\
	../xpdf/Error.o 		\
	../xpdf/FontEncodingTables.o 	\
	../xpdf/Function.o 		\
	../xpdf/Gfx.o 			\
	../xpdf/GfxFont.o 		\
	../xpdf/GfxState.o 		\
	../xpdf/GlobalParams.o 		\
	../xpdf/JArithmeticDecoder.o 	\
	../xpdf/JBIG2Stream.o 		\
	../xpdf/JPXStream.o 		\
	../xpdf/Lexer.o 		\
	../xpdf/Link.o 			\
	../xpdf/NameToCharCode.o 	\
	../xpdf/Object.o 		\
	../xpdf/Outline.o 		\
	../xpdf/OutputDev.o 		\
	../xpdf/Page.o 			\
	../xpdf/Parser.o 		\
	../xpdf/PDFDoc.o 		\
	../xpdf/PDFDocEncoding.o 	\
	../xpdf/PSOutputDev.o 		\
	../xpdf/PSTokenizer.o 		\
	../xpdf/SecurityHandler.o 	\
	../xpdf/SplashOutputDev.o 	\
	../xpdf/Stream.o 		\
	../xpdf/UnicodeMap.o 		\
	../xpdf/UnicodeTypeTable.o 	\
	../xpdf/XRef.o

AM_LDFLAGS = -Wl,--version-script=export.map

osso_pdfviewer_LDADD = 				\
	$(MAEMO_LAUNCHER_LIBS) 			\
	$(HILDON_LIBS)				\
	$(GDEPS_LIBS) 				\
	$(FREETYPE_LIBS) 			\
	$(XPDF_OBJS)				\
	-L../goo -lGoo 				\
	-L../splash -lsplash			\
	-L../fofi -lfofi
