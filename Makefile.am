DEF_SUBDIRS = lib applet wizard sendto docs po help

SUBDIRS = lib applet wizard sendto docs po

##FIXME: what happened to the icons subdir?

if ENABLE_DOCUMENTATION
SUBDIRS += help
endif

ACLOCAL_AMFLAGS = -I m4 --install ${ACLOCAL_FLAGS}

EXTRA_DIST = intltool-extract.in intltool-update.in intltool-merge.in ChangeLog.pre-2.27 gtk-doc.make

DISTCHECK_CONFIGURE_FLAGS = --disable-schemas-install \
	--disable-desktop-update --disable-mime-update --disable-icon-update \
	--enable-gtk-doc --disable-scrollkeeper --enable-introspection

DISTCLEANFILES = intltool-extract intltool-update intltool-merge dllar.sh \
	po/*.lock ./po/.intltool-merge-cache* po/missing *~

MAINTAINERCLEANFILES = Makefile.in \
	aclocal.m4 configure config.h.in config.sub config.guess \
	ltmain.sh depcomp missing install-sh mkinstalldirs \
	intltool-extract.in intltool-update.in intltool-merge.in \
	gnome-bluetooth-1.0.pc

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = gnome-bluetooth-1.0.pc

install-data-hook:
if DESKTOP_UPDATE
	$(UPDATE_DESKTOP_DATABASE) $(DESTDIR)$(datadir)/applications
endif

uninstall-hook:
if DESKTOP_UPDATE
	$(UPDATE_DESKTOP_DATABASE) $(DESTDIR)$(datadir)/applications
endif

# Build ChangeLog from GIT history
ChangeLog:
	@if test -f $(top_srcdir)/.git/HEAD; then \
		git log --stat --after="Fri Feb 20 00:08:02 2009" > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog

distclean-local:
	-rm -rf autom4te.cache || rmdir autom4te.cache

