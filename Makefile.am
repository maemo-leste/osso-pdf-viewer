# Options
#AUTOMAKE_OPTIONS = no-dependencies

# Non-source files to be included in "make dist"
EXTRA_DIST = \
		autogen.sh \
		debian/changelog \
		debian/compat \
		debian/control \
		debian/copyright \
		debian/rules \
		debian/docs \
		debian/osso-pdf-viewer.links \
		debian/osso-pdf-viewer.install \
		debian/osso-pdf-viewer.postinst \
		debian/osso-pdf-viewer.postrm \
		debian/README.Debian \
		data/osso_pdfviewer.desktop \
		data/com.nokia.osso_pdfviewer.service \
		data/osso-pdf-viewer.xml \
		doc/pdffonts.1 \
		doc/pdffonts.cat \
		doc/pdffonts.hlp \
		doc/pdfimages.1 \
		doc/pdfimages.cat \
		doc/pdfimages.hlp \
		doc/pdfinfo.1 \
		doc/pdfinfo.cat \
		doc/pdfinfo.hlp \
		doc/pdftoppm.1 \
		doc/pdftoppm.cat \
		doc/pdftoppm.hlp \
		doc/pdftops.1 \
		doc/pdftops.cat \
		doc/pdftops.hlp \
		doc/pdftotext.1 \
		doc/pdftotext.cat \
		doc/pdftotext.hlp \
		doc/sample-xpdfrc \
		doc/xpdf.1 \
		doc/xpdf.cat \
		doc/xpdf.hlp \
		doc/xpdfrc.5 \
		doc/xpdfrc.cat \
		doc/xpdfrc.hlp

# Order of subdirs is crucial
SUBDIRS = data splash fofi goo xpdf src

INCLUDES = $(DEPS_CFLAGS)

deb:	dist
	-mkdir -p $(top_builddir)/debian-build
	cd $(top_builddir)/debian-build && tar zxf ../$(top_builddir)/$(PACKAGE)-$(VERSION).tar.gz
	cd $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION) && \
	  DEB_BUILD_OPTIONS=maemo-launcher dpkg-buildpackage -rfakeroot -uc -us -sa -D
	-rm -rf $(top_builddir)/debian-build/$(PACKAGE)-$(VERSION)
	ls -al --color $(top_builddir)/debian-build
	
