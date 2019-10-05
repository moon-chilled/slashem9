CFLAGS="-I../include -Iinclude -DDLB"
mkdir -p dat include src
cc -o makedefs ../util/makedefs.c ../util/panic.c ../src/alloc.c ../src/objects.c ../src/monst.c $CFLAGS
cc -o dlb ../util/dlb_main.c ../src/alloc.c ../util/panic.c ../src/dlb.c $CFLAGS
./makedefs

TTY_SRC="../win/tty/getline.c ../win/tty/termcap.c ../win/tty/topl.c ../win/tty/wintty.c"
CURSES_SRC="../win/curses/cursmain.c ../win/curses/curswins.c ../win/curses/cursmisc.c ../win/curses/cursdial.c ../win/curses/cursstat.c ../win/curses/cursinit.c ../win/curses/cursmesg.c ../win/curses/cursinvt.c"
PROXY_SRC="../win/proxy/winproxy.c ../win/proxy/callback.c ../win/proxy/dlbh.c ../win/proxy/mapid.c ../win/proxy/riputil.c ../win/proxy/getopt.c ../win/proxy/glyphmap.c"

SYSSRC="../sys/share/ioctl.c ../sys/share/unixtty.c ../sys/unix/unixmain.c ../sys/unix/unixunix.c ../sys/unix/unixres.c"
GENSRC="src/monstr.c src/vis_tab.c"
SRC="../src/allmain.c ../src/alloc.c ../src/apply.c ../src/artifact.c ../src/attrib.c ../src/ball.c ../src/bones.c ../src/borg.c ../src/botl.c ../src/cmd.c ../src/dbridge.c ../src/decl.c ../src/detect.c ../src/dig.c ../src/display.c ../src/dlb.c ../src/do.c ../src/do_name.c ../src/do_wear.c ../src/dog.c ../src/dogmove.c ../src/dokick.c ../src/dothrow.c ../src/drawing.c ../src/dungeon.c ../src/eat.c ../src/end.c ../src/engrave.c ../src/exper.c ../src/explode.c ../src/extralev.c ../src/files.c ../src/fountain.c ../src/hack.c ../src/hacklib.c ../src/invent.c ../src/light.c ../src/lock.c ../src/mail.c ../src/makemon.c ../src/mapglyph.c ../src/mcastu.c ../src/mhitm.c ../src/mhitu.c ../src/minion.c ../src/mklev.c ../src/mkmap.c ../src/mkmaze.c ../src/mkobj.c ../src/mkroom.c ../src/mon.c ../src/mondata.c ../src/monmove.c ../src/monst.c ../src/mplayer.c ../src/mthrowu.c ../src/muse.c ../src/music.c ../src/o_init.c ../src/objects.c ../src/objnam.c ../src/options.c ../src/pager.c ../src/pickup.c ../src/pline.c ../src/polyself.c ../src/potion.c ../src/pray.c ../src/priest.c ../src/quest.c ../src/questpgr.c ../src/read.c ../src/rect.c ../src/region.c ../src/restore.c ../src/rip.c ../src/rnd.c ../src/role.c ../src/rumors.c ../src/save.c ../src/shk.c ../src/shknam.c ../src/sit.c ../src/sounds.c ../src/sp_lev.c ../src/spell.c ../src/steal.c ../src/steed.c ../src/teleport.c ../src/timeout.c ../src/topten.c ../src/track.c ../src/trap.c ../src/u_init.c ../src/uhitm.c ../src/vault.c ../src/version.c ../src/vision.c ../src/weapon.c ../src/were.c ../src/wield.c ../src/windows.c ../src/wizard.c ../src/worm.c ../src/worn.c ../src/write.c ../src/zap.c ../src/gypsy.c ../src/tech.c ../src/unicode.c"

OBJ=

for i in $SRC $GENSRC $SYSSRC $TTY_SRC $CURSES_SRC; do
	cc -c $CFLAGS $i&
	OBJ="$OBJ $(echo $i | perl -pe 's/.*\/(.*)/\1/g' | perl -pe 's/\.c/.o/g')"
done

wait

cc -o slashem *.o -lncursesw

spec_levs() {
	bison -d ../util/lev_comp.y
	mv lev_comp.tab.c lev_yacc.c
	mv lev_comp.tab.h include/lev_comp.h

	flex ../util/lev_comp.l
	mv lex.yy.c lev_lex.c

	cc lev_yacc.c lev_lex.c ../util/lev_main.c ../src/alloc.c ../util/panic.c ../src/drawing.c ../src/decl.c ../src/objects.c ../src/monst.c -o lev_comp -I../include -Iinclude


	bison -d ../util/dgn_comp.y
	mv dgn_comp.tab.c dgn_yacc.c
	mv dgn_comp.tab.h include/dgn_comp.h

	flex ../util/dgn_comp.l
	mv lex.yy.c dgn_lex.c

	cc dgn_yacc.c dgn_lex.c ../util/dgn_main.c ../src/alloc.c ../util/panic.c -o dgn_comp -I../include -Iinclude

	./lev_comp ../dat/beholder.des
	./lev_comp ../dat/bigroom.des
	./lev_comp ../dat/blkmar.des
	./lev_comp ../dat/castle.des
	./lev_comp ../dat/grund.des
	./lev_comp ../dat/dragons.des
	./lev_comp ../dat/endgame.des
	./lev_comp ../dat/frnknstn.des
	./lev_comp ../dat/gehennom.des
	./lev_comp ../dat/giants.des
	./lev_comp ../dat/guild.des
	./lev_comp ../dat/knox.des
	./lev_comp ../dat/kobold-1.des
	./lev_comp ../dat/kobold-2.des
	./lev_comp ../dat/lich.des
	./lev_comp ../dat/mall-1.des
	./lev_comp ../dat/mall-2.des
	./lev_comp ../dat/medusa.des
	./lev_comp ../dat/mines.des
	./lev_comp ../dat/mtemple.des
	./lev_comp ../dat/newmall.des
	./lev_comp ../dat/nightmar.des
	./lev_comp ../dat/nymph.des
	./lev_comp ../dat/oracle.des
	./lev_comp ../dat/rats.des
	./lev_comp ../dat/sea.des
	./lev_comp ../dat/sokoban.des
	./lev_comp ../dat/spiders.des
	./lev_comp ../dat/stor-1.des
	./lev_comp ../dat/stor-2.des
	./lev_comp ../dat/stor-3.des
	./lev_comp ../dat/tomb.des
	./lev_comp ../dat/tower.des
	./lev_comp ../dat/yendor.des

	./lev_comp ../dat/Arch.des
	./lev_comp ../dat/Barb.des
	./lev_comp ../dat/Caveman.des
	./lev_comp ../dat/Flame.des
	./lev_comp ../dat/Healer.des
	./lev_comp ../dat/Ice.des
	./lev_comp ../dat/Knight.des
	./lev_comp ../dat/Monk.des
	./lev_comp ../dat/Necro.des
	./lev_comp ../dat/Priest.des
	./lev_comp ../dat/Ranger.des
	./lev_comp ../dat/Rogue.des
	./lev_comp ../dat/Samurai.des
	./lev_comp ../dat/Tourist.des
	./lev_comp ../dat/Slayer.des
	./lev_comp ../dat/Valkyrie.des
	./lev_comp ../dat/Wizard.des
	./lev_comp ../dat/Yeoman.des


	./dgn_comp dat/dungeon.pdf
}

spec_levs

mkdir dater
cp ../dat/help ../dat/hh ../dat/cmdhelp ../dat/history ../dat/opthelp ../dat/wizhelp ../dat/gypsy.txt dat/data dat/options dat/oracles dat/quest.dat dat/rumors dater/
cd dater
../dlb cf nhshare *
mv nhshare ..
cd ..
rm -rf dater

cp dat/dungeon .
./dlb cf nhushare dungeon *.lev


touch $install_dir/perm
touch $install_dir/logfile
touch $install_dir/xlogfile
mkdir $install_dir/save
