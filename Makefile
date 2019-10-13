PREFIX ?= $(HOME)/slashem-nextinstall
HACKDIR ?= $(PREFIX)/slashemdir
BINDIR ?= $(PREFIX)/bin

.PHONY: unix linux mingw win32

default: linux

all: unix linux mingw win32

unix:
	touch Tupfile.ini
	cp sys/tup/Tupunix.tup Tupfile
	mkdir -p build-unix
	touch build-unix/tup.config
	tup build-unix
	rm -f Tupfile Tupfile.ini

linux:
	touch Tupfile.ini
	cp sys/tup/Tuplinux.tup Tupfile
	mkdir -p build-linux
	touch build-linux/tup.config
	tup build-linux
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

install: linux
	mkdir -p $(HACKDIR)/save $(BINDIR)
	touch $(HACKDIR)/perm $(HACKDIR)/xlogfile

	cp build-linux/slashem build-linux/nhdat $(HACKDIR)

	sed -e 's;@HACKDIR@;$(HACKDIR);' < sys/unix/slashem.sh > $(BINDIR)/slashem
	chmod 755 $(BINDIR)/slashem
	cp build-linux/slashem-recover $(BINDIR)/
