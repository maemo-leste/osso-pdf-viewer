# Makefile.am for xpdf

if HAVE_BLUETOOTH
HAVE_BLUETOOTH_FLAGS=-DVERSION_3
endif

INCLUDES = -I$(top_srcdir)		\
	   -I$(top_srcdir)/fofi		\
	   -I$(top_srcdir)/goo		\
	   -I$(top_srcdir)/splash	\
	   $(FREETYPE_CFLAGS)           \
           $(HAVE_BLUETOOTH_FLAGS)	\
	   $(MAEMO_LAUNCHER_CFLAGS)

# Fictituous library name, to generate required object files
noinst_LIBRARIES = libxpdf.a

libxpdf_includedir = $(includedir)/xpdf
libxpdf_include_HEADERS = \
	Annot.h				\
	Array.h				\
	BuiltinFont.h			\
	BuiltinFontTables.h		\
	CMap.h				\
	Catalog.h			\
	CharCodeToUnicode.h		\
	CharTypes.h			\
	CompactFontTables.h		\
	Decrypt.h			\
	Dict.h				\
	Error.h				\
	ErrorCodes.h			\
	FontEncodingTables.h		\
	Function.h			\
	Gfx.h				\
	GfxFont.h			\
	GfxState.h			\
	GlobalParams.h			\
	ImageOutputDev.h		\
	JArithmeticDecoder.h		\
	JBIG2Stream.h			\
	JPXStream.h			\
	Lexer.h				\
	Link.h				\
	NameToCharCode.h		\
	NameToUnicodeTable.h		\
	Object.h			\
	Outline.h			\
	OutputDev.h			\
	PDFDoc.h			\
	PDFDocEncoding.h		\
	PSOutputDev.h			\
	PSTokenizer.h			\
	Page.h				\
	Parser.h			\
	SecurityHandler.h		\
	SplashOutputDev.h		\
	Stream-CCITT.h			\
	Stream.h			\
	UTF8.h				\
	UnicodeMap.h			\
	UnicodeMapTables.h		\
	UnicodeTypeTable.h		\
	XRef.h				\
	XpdfPluginAPI.h			\
	config.h

# OSSO PDF Viewer specific classes in xpdf/ are built in src/
libxpdf_a_SOURCES = \
	Annot.cc		\
	Array.cc		\
	BuiltinFont.cc		\
	BuiltinFontTables.cc	\
	Catalog.cc		\
	CharCodeToUnicode.cc	\
	CMap.cc			\
	Decrypt.cc		\
	Dict.cc			\
	Error.cc		\
	FontEncodingTables.cc	\
	Function.cc		\
	Gfx.cc			\
	GfxFont.cc		\
	GfxState.cc		\
	GlobalParams.cc		\
	JArithmeticDecoder.cc	\
	JBIG2Stream.cc		\
	JPXStream.cc		\
	Lexer.cc		\
	Link.cc			\
	NameToCharCode.cc	\
	Object.cc		\
	Outline.cc		\
	OutputDev.cc		\
	Page.cc			\
	Parser.cc		\
	PDFDoc.cc		\
	PDFDocEncoding.cc	\
	PSOutputDev.cc		\
	PSTokenizer.cc		\
	SecurityHandler.cc	\
	SplashOutputDev.cc	\
	Stream.cc		\
	UnicodeMap.cc		\
	UnicodeTypeTable.cc	\
	XRef.cc			\
	XpdfPluginAPI.cc
