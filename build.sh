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
