PREFIX ?= $(HOME)/slashem9install
HACKDIR ?= $(PREFIX)/slashem9dir
BINDIR ?= $(PREFIX)/bin
DATADIR ?= $(HACKDIR)/dat

CFLAGS += -Iinclude -Isys/share/libtre -DWIZARD=\"$(shell whoami)\" -DSLASHEM_GIT_COMMIT_REV=\"$(shell git rev-parse --short HEAD)\"
CFLAGS += -g -ggdb -O0 -pipe
CFLAGS += -std=c11 -D_POSIX_C_SOURCE=200809L -Wall -Wimplicit-fallthrough -U_FORTIFY_SOURCE
# -U_FORTIFY_SOURCE - some systems enable source fortification by default, but we can't handle that yet

LDFLAGS += -lgc

DLBMODE ?= lib

ifeq ($(DLBMODE),embed)
	CFLAGS += -DDLBEMBED
else ifeq ($(DLBMODE),none)
else #lib
	CFLAGS += -DDLBLIB 
endif


ifneq ($(FORTIFY),)
	CFLAGS += -D_FORTIFY_SOURCE -O2 -Wno-format-overflow -Wno-maybe-uninitialized -Wno-format-truncation
endif

ifneq ($(CC),tcc)
ifneq ($(CC),pcc)
	CFLAGS += -Werror
endif
endif
ifeq ($(CC),pcc)
	CFLAGS += -D__float128="long double"
endif

CFLAGS := $(CFLAGS) $(shell pkg-config --cflags ncursesw)
ifeq ($(shell uname -s),Darwin)
	SLDFLAGS += -lncurses
else
	SLDFLAGS += -lncursesw
endif

CC ?= cc
CCLD ?= $(CC)


OBJ_BASE := src/allmain.o src/alloc.o src/apply.o src/artifact.o src/attrib.o src/ball.o src/bones.o src/botl.o src/cmd.o src/dbridge.o src/decl.o src/detect.o src/dig.o src/display.o src/dlb.o src/do.o src/do_name.o src/do_wear.o src/dog.o src/dogmove.o src/dokick.o src/dothrow.o src/drawing.o src/dungeon.o src/eat.o src/end.o src/engrave.o src/exper.o src/explode.o src/extralev.o src/files.o src/fountain.o src/hack.o src/hacklib.o src/invent.o src/light.o src/lock.o src/mail.o src/makemon.o src/mapglyph.o src/mcastu.o src/mhitm.o src/mhitu.o src/minion.o src/mklev.o src/mkmap.o src/mkmaze.o src/mkobj.o src/mkroom.o src/mon.o src/mondata.o src/monmove.o src/monst.o src/mplayer.o src/mthrowu.o src/monstr.o src/muse.o src/music.o src/nhstr.o src/o_init.o src/objects.o src/objnam.o src/options.o src/pager.o src/pickup.o src/pline.o src/polyself.o src/potion.o src/pray.o src/priest.o src/quest.o src/questpgr.o src/read.o src/rect.o src/region.o src/restore.o src/rip.o src/rnd.o src/role.o src/rumors.o src/save.o src/shk.o src/shknam.o src/sit.o src/sounds.o src/sp_lev.o src/spell.o src/steal.o src/steed.o src/teleport.o src/timeout.o src/topten.o src/track.o src/trap.o src/u_init.o src/uhitm.o src/vault.o src/version.o src/vision.o src/weapon.o src/were.o src/wield.o src/windows.o src/wizard.o src/worm.o src/worn.o src/write.o src/zap.o src/gypsy.o src/tech.o src/unicode.o
OBJ_SYS := sys/share/ioctl.o sys/share/unixtty.o sys/unix/unixmain.o sys/unix/unixunix.o sys/unix/unixres.o sys/share/libtre/regcomp.o sys/share/libtre/regerror.o sys/share/libtre/regexec.o sys/share/libtre/tre-ast.o sys/share/libtre/tre-compile.o sys/share/libtre/tre-match-backtrack.o sys/share/libtre/tre-match-parallel.o sys/share/libtre/tre-mem.o sys/share/libtre/tre-parse.o sys/share/libtre/tre-stack.o
OBJ_TTY := win/tty/getline.o win/tty/termcap.o win/tty/topl.o win/tty/wintty.o
OBJ_CURSES := win/curses/cursdial.o win/curses/cursinit.o win/curses/cursinvt.o win/curses/cursmain.o win/curses/cursmesg.o win/curses/cursmisc.o win/curses/cursstat.o win/curses/curswins.o
OBJ_PROXY := win/proxy/callback.o win/proxy/dlbh.o win/proxy/getopt.o win/proxy/glyphmap.o win/proxy/mapid.o win/proxy/riputil.o win/proxy/winproxy.o
OBJ_DGNCOMP := util/dgn_yacc.o util/dgn_lex.o util/dgn_main.o src/alloc.o util/panic.o
OBJ_LEVCOMP := util/lev_yacc.o util/lev_lex.o util/lev_main.o src/alloc.o util/panic.o src/drawing.o src/decl.o src/objects.o src/monst.o
OBJ_DLB := util/dlb_main.o src/alloc.o util/panic.o src/dlb.o
OBJ_DATA2C := util/data2c.o
OBJ_RECOVER := util/recover.o

SLASHEMOBJ := $(OBJ_BASE) $(OBJ_SYS) $(OBJ_TTY) $(OBJ_CURSES)

ALLOBJ := $(SLASHEMOBJ) $(OBJ_DGNCOMP) $(OBJ_LEVCOMP) $(OBJ_RECOVER) $(OBJ_DLB) $(OBJ_DATA2C)


default: all

install: all
	mkdir -p $(HACKDIR)/save $(BINDIR)
	touch $(HACKDIR)/perm $(HACKDIR)/xlogfile

	install util/slashem9-recover $(BINDIR)
	install src/slashem9 $(HACKDIR)
ifeq ($(DLBMODE),lib)
	cp dat/nhdat $(HACKDIR)
else ifeq ($(DLBMODE),none)
	mkdir -p $(DATADIR)
	cp dat/* $(DATADIR)
endif

	sed -e 's;@HACKDIR@;$(HACKDIR);' < sys/unix/slashem9.sh > $(BINDIR)/slashem9
	chmod 755 $(BINDIR)/slashem9

src/slashem9: $(SLASHEMOBJ)
	$(CCLD) -o src/slashem9 $(SLASHEMOBJ) $(LDFLAGS) $(SLDFLAGS)

util/slashem9-recover: $(OBJ_RECOVER)
	$(CCLD) -o util/slashem9-recover $(OBJ_RECOVER) $(LDFLAGS)

all: src/slashem9 util/slashem9-recover
ifeq ($(DLBMODE),lib)
all: dat/nhdat
endif
ifeq ($(DLBMODE),embed)
all: include/dlb_archive.h
src/dlb.o: include/dlb_archive.h
endif

util/dgn_lex.c: util/dgn_yacc.c # actually _yacc.h, but make doesn't know about that

util/dgn_yacc.c: util/dgn_yacc.y
	yacc -d util/dgn_yacc.y
	mv -f y.tab.c util/dgn_yacc.c
	mv -f y.tab.h util/dgn_yacc.h

util/dgn_comp: $(OBJ_DGNCOMP)
	$(CCLD) -o util/dgn_comp $(OBJ_DGNCOMP) $(LDFLAGS)

util/lev_lex.c: util/lev_yacc.c # ditto

# artificial dependency on dgn_yacc.c to force one to be built completely
# before the other; otherwise, there can be a race where one y.tab.[ch]
# overwrites another
util/lev_yacc.c: util/lev_yacc.y util/dgn_yacc.c
	yacc -d util/lev_yacc.y
	mv -f y.tab.c util/lev_yacc.c
	mv -f y.tab.h util/lev_yacc.h

util/lev_comp: $(OBJ_LEVCOMP)
	$(CCLD) -o util/lev_comp $(OBJ_LEVCOMP) $(LDFLAGS)

util/dlb: $(OBJ_DLB)
	$(CCLD) -o util/dlb $(OBJ_DLB) $(LDFLAGS)

util/data2c: $(OBJ_DATA2C)
	$(CCLD) -o util/data2c $(OBJ_DATA2C)

dat/nhdat: util/dlb util/dgn_comp util/lev_comp dat/*.des dat/dungeon.def dat/help dat/hh dat/cmdhelp dat/history dat/license dat/opthelp dat/wizhelp dat/gypsy.txt dat/data.base dat/oracles.txt dat/quest.txt dat/rumors.fal dat/rumors.tru
	./util/dgn_comp dat/dungeon.def
	for i in dat/*.des; do ./util/lev_comp $$i; done
	mv *.lev dat/
	cd dat; ../util/dlb cf nhdat dungeon *.lev help hh cmdhelp history license opthelp wizhelp gypsy.txt data.base oracles.txt quest.txt rumors.fal rumors.tru

include/dlb_archive.h: util/data2c util/dgn_comp util/lev_comp dat/*.des dat/dungeon.def dat/help dat/hh dat/cmdhelp dat/history dat/license dat/opthelp dat/wizhelp dat/gypsy.txt dat/data.base dat/oracles.txt dat/quest.txt dat/rumors.fal dat/rumors.tru
	./util/dgn_comp dat/dungeon.def
	for i in dat/*.des; do ./util/lev_comp $$i; done
	mv *.lev dat/
	cd dat; ../util/data2c ../include/dlb_archive.h dungeon *.lev help hh cmdhelp history license opthelp wizhelp gypsy.txt data.base oracles.txt quest.txt rumors.fal rumors.tru

clean:
	rm -f $(ALLOBJ) src/slashem9 util/dlb util/data2c util/lev_comp util/dgn_comp dat/nhdat dat/*.lev util/*_yacc.c util/*_lex.c util/*_yacc.h dat/dungeon include/dlb_archive.h
