PREFIX ?= ~/slashem-nextinstall
HACKDIR ?= $(PREFIX)/slashemdir
BINDIR ?= $(PREFIX)/bin

default: builder
builder:
	tup

clean:
	rm -rf build/*
	touch build/tup.config

install: builder
	mkdir -p $(HACKDIR)/save $(BINDIR)
	touch $(HACKDIR)/perm $(HACKDIR)/logfile $(HACKDIR)/xlogfile

	cp build/slashem build/nhdat $(HACKDIR)
	sed -e 's;@HACKDIR@;$(HACKDIR);' < sys/unix/slashem.sh > $(BINDIR)/slashem
	chmod 755 $(BINDIR)/slashem

