PREFIX ?= $(HOME)/slashem-nextinstall
HACKDIR ?= $(PREFIX)/slashemdir
BINDIR ?= $(PREFIX)/bin

.PHONY: unix mingw win32

default: unix

all: unix mingw win32

unix:
	touch Tupfile.ini
	cp sys/tup/Tupunix.tup Tupfile
	mkdir -p build-unix
	touch build-unix/tup.config
	tup build-unix
	rm -f Tupfile Tupfile.ini

mingw:
	touch Tupfile.ini
	cp sys/tup/Tupmingw.tup Tupfile
	mkdir -p build-mingw
	touch build-mingw/tup.config
	tup build-mingw
	rm -f Tupfile Tupfile.ini

win32:
	touch Tupfile.ini
	cp sys/tup/Tupwin32.tup Tupfile
	mkdir -p build-win32
	touch build-win32/tup.config
	tup build-win32
	rm -f Tupfile Tupfile.ini

clean:
	rm -rf build-*

install: unix
	mkdir -p $(HACKDIR)/save $(BINDIR)
	touch $(HACKDIR)/perm $(HACKDIR)/logfile $(HACKDIR)/xlogfile

	cp build-unix/slashem build-unix/nhdat $(HACKDIR)

	sed -e 's;@HACKDIR@;$(HACKDIR);' < sys/unix/slashem.sh > $(BINDIR)/slashem
	chmod 755 $(BINDIR)/slashem
	cp build-unix/slashem-recover $(BINDIR)/
