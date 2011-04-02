# Makefile.am for libsplash for XPDF engine

INCLUDES = -I$(top_srcdir)			\
	   -I$(top_srcdir)/fofi			\
	   -I$(top_srcdir)/goo			\
	   $(FREETYPE_CFLAGS)			\
	   $(MAEMO_LAUNCHER_CFLAGS)

noinst_LIBRARIES = libsplash.a

#libsplash_la_CFLAGS = $(INCLUDES)
libsplash_includedir = $(includedir)/splash
libsplash_include_HEADERS = \
        Splash.h                                \
        SplashBitmap.h                          \
        SplashClip.h                            \
        SplashErrorCodes.h                      \
        SplashFTFont.h                          \
        SplashFTFontEngine.h                    \
        SplashFTFontFile.h                      \
        SplashFont.h                            \
        SplashFontEngine.h                      \
        SplashFontFile.h                        \
        SplashFontFileID.h                      \
        SplashGlyphBitmap.h                     \
        SplashMath.h                            \
        SplashPath.h                            \
        SplashPattern.h                         \
        SplashScreen.h                          \
        SplashState.h                           \
        SplashT1Font.h                          \
        SplashT1FontEngine.h                    \
        SplashT1FontFile.h                      \
        SplashTypes.h                           \
        SplashXPath.h                           \
        SplashXPathScanner.h			

libsplash_a_SOURCES = \
	Splash.cc                               \
        SplashBitmap.cc                         \
        SplashClip.cc                           \
        SplashFTFont.cc                         \
        SplashFTFontEngine.cc                   \
        SplashFTFontFile.cc                     \
        SplashFont.cc                           \
        SplashFontEngine.cc                     \
        SplashFontFile.cc                       \
        SplashFontFileID.cc                     \
        SplashPath.cc                           \
        SplashPattern.cc                        \
        SplashScreen.cc                         \
        SplashState.cc                          \
        SplashT1Font.cc                         \
        SplashT1FontEngine.cc                   \
        SplashT1FontFile.cc                     \
        SplashXPath.cc                          \
        SplashXPathScanner.cc			\
	FixedPoint.cc
