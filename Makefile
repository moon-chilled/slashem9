PREFIX ?= ~/slashem-nextinstall
HACKDIR ?= $(PREFIX)/slashemdir
BINDIR ?= $(PREFIX)/bin

.PHONY: build-mingw build-unix

default: build-unix

all: build-unix build-mingw

build-unix:
	cp sys/tup/Tupunix.tup Tupfile
	mkdir -p build-unix
	touch build-unix/tup.config
	tup build-unix
	rm -f Tupfile

build-mingw:
	cp sys/tup/Tupmingw.tup Tupfile
	mkdir -p build-mingw
	touch build-mingw/tup.config
	tup build-mingw
	rm -f Tupfile

clean:
	rm -rf build-*

install: build-unix
	mkdir -p $(HACKDIR)/save $(BINDIR)
	touch $(HACKDIR)/perm $(HACKDIR)/logfile $(HACKDIR)/xlogfile

	cp build-unix/slashem build-unix/nhdat $(HACKDIR)

	sed -e 's;@HACKDIR@;$(HACKDIR);' < sys/unix/slashem.sh > $(BINDIR)/slashem
	chmod 755 $(BINDIR)/slashem
	cp build-unix/slashem-recover $(BINDIR)/
