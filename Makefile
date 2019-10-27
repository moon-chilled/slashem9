MAKE ?= make

PREFIX ?= $(HOME)/slashem-nextinstall
HACKDIR ?= $(PREFIX)/slashemdir
BINDIR ?= $(PREFIX)/bin

CFLAGS += -Iinclude -Ibuild/include -DDLB -DWIZARD=\"$(shell whoami)\"
CFLAGS += -g -O0
CFLAGS += -std=c99 -D_XOPEN_SOURCE=500 -D_DEFAULT_SOURCE -Werror -Wpedantic -pedantic

CFLAGS += $(shell pkg-config --cflags ncursesw)
LDFLAGS += $(shell pkg-config --libs ncursesw) -lncursesw

CC ?= cc
CCLD ?= $(CC)


OBJ_BASE := src/allmain.o src/alloc.o src/apply.o src/artifact.o src/attrib.o src/ball.o src/bones.o src/borg.o src/botl.o src/cmd.o src/dbridge.o src/decl.o src/detect.o src/dig.o src/display.o src/dlb.o src/do.o src/do_name.o src/do_wear.o src/dog.o src/dogmove.o src/dokick.o src/dothrow.o src/drawing.o src/dungeon.o src/eat.o src/end.o src/engrave.o src/exper.o src/explode.o src/extralev.o src/files.o src/fountain.o src/hack.o src/hacklib.o src/invent.o src/light.o src/lock.o src/mail.o src/makemon.o src/mapglyph.o src/mcastu.o src/mhitm.o src/mhitu.o src/minion.o src/mklev.o src/mkmap.o src/mkmaze.o src/mkobj.o src/mkroom.o src/mon.o src/mondata.o src/monmove.o src/monst.o src/mplayer.o src/mthrowu.o src/monstr.o src/muse.o src/music.o src/o_init.o src/objects.o src/objnam.o src/options.o src/pager.o src/pickup.o src/pline.o src/polyself.o src/potion.o src/pray.o src/priest.o src/quest.o src/questpgr.o src/read.o src/rect.o src/region.o src/restore.o src/rip.o src/rnd.o src/role.o src/rumors.o src/save.o src/shk.o src/shknam.o src/sit.o src/sounds.o src/sp_lev.o src/spell.o src/steal.o src/steed.o src/teleport.o src/timeout.o src/topten.o src/track.o src/trap.o src/u_init.o src/uhitm.o src/vault.o src/version.o src/vision.o src/weapon.o src/were.o src/wield.o src/windows.o src/wizard.o src/worm.o src/worn.o src/write.o src/zap.o src/gypsy.o src/tech.o src/unicode.o
OBJ_SYS := sys/share/ioctl.o sys/share/unixtty.o sys/unix/unixmain.o sys/unix/unixunix.o sys/unix/unixres.o
OBJ_TTY := win/tty/getline.o win/tty/termcap.o win/tty/topl.o win/tty/wintty.o
OBJ_CURSES := win/curses/cursdial.o win/curses/cursinit.o win/curses/cursinvt.o win/curses/cursmain.o win/curses/cursmesg.o win/curses/cursmisc.o win/curses/cursstat.o win/curses/curswins.o
OBJ_PROXY := win/proxy/callback.o win/proxy/dlbh.o win/proxy/getopt.o win/proxy/glyphmap.o win/proxy/mapid.o win/proxy/riputil.o win/proxy/winproxy.o
OBJ_DGNCOMP := util/dgn_yacc.o util/dgn_lex.o util/dgn_main.o src/alloc.o util/panic.o
OBJ_LEVCOMP := util/lev_yacc.o util/lev_lex.o util/lev_main.o src/alloc.o util/panic.o src/drawing.o src/decl.o src/objects.o src/monst.o
OBJ_DLB := util/dlb_main.o src/alloc.o util/panic.o src/dlb.o

SLASHEMOBJ := $(OBJ_BASE) $(OBJ_SYS) $(OBJ_TTY) $(OBJ_CURSES)

ALLOBJ := $(SLASHEMOBJ) $(OBJ_DGNCOMP) $(OBJ_LEVCOMP) $(OBJ_DLB)


default: all

install: all
	mkdir -p $(HACKDIR)/save $(BINDIR)
	touch $(HACKDIR)/perm $(HACKDIR)/xlogfile

	cp src/slashem dat/nhdat $(HACKDIR)

	sed -e 's;@HACKDIR@;$(HACKDIR);' < sys/unix/slashem.sh > $(BINDIR)/slashem
	chmod 755 $(BINDIR)/slashem
	#cp util/slashem-recover $(BINDIR)/

src/slashem: $(SLASHEMOBJ)
	$(CCLD) $(LDFLAGS) -o src/slashem $(SLASHEMOBJ)

all: src/slashem dat/nhdat

util/dgn_yacc.c: util/dgn_yacc.y
	yacc -d util/dgn_yacc.y
	mv -f y.tab.c util/dgn_yacc.c
	mv -f y.tab.h util/dgn_yacc.h

util/dgn_comp: $(OBJ_DGNCOMP)
	$(CCLD) -o util/dgn_comp $(OBJ_DGNCOMP)

util/lev_yacc.c: util/lev_yacc.y
	yacc -d util/lev_yacc.y
	mv -f y.tab.c util/lev_yacc.c
	mv -f y.tab.h util/lev_yacc.h

util/lev_comp: $(OBJ_LEVCOMP)
	$(CCLD) -o util/lev_comp $(OBJ_LEVCOMP)

util/dlb: $(OBJ_DLB)
	$(CCLD) -o util/dlb $(OBJ_DLB)

dat/nhdat: util/dlb util/dgn_comp util/lev_comp dat/*.des
	./util/dgn_comp dat/dungeon.def
	for i in dat/*.des; do ./util/lev_comp $$i; done
	mv *.lev dat/
	cd dat; ../util/dlb cf nhdat dungeon *.lev help hh cmdhelp history license opthelp wizhelp gypsy.txt data.base oracles.txt quest.txt rumors.fal rumors.tru

clean:
	rm -f $(ALLOBJ) src/slashem util/dlb util/lev_comp util/dgn_comp dat/nhdat dat/*.lev util/*_yacc.c util/*_lex.c util/*_yacc.h dat/dungeon
