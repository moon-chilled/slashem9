#!/bin/sh

HACKDIR=@HACKDIR@
export HACKDIR
HACK=$HACKDIR/slashem9

# see if we can find the full path name of PAGER, so help files work properly
# assume that if someone sets up a special variable (HACKPAGER) for NetHack,
# it will already be in a form acceptable to NetHack
# ideas from brian@radio.astro.utoronto.ca
if test \( "xxx$PAGER" != xxx \) -a \( "xxx$HACKPAGER" = xxx \)
then
	HACKPAGER=$PAGER

#	use only the first word of the pager variable
#	this prevents problems when looking for file names with trailing
#	options, but also makes the options unavailable for later use from
#	NetHack
	for i in $HACKPAGER
	do
		HACKPAGER=$i
		break
	done

	if test ! -f $HACKPAGER
	then
		IFS=:
		for i in $PATH
		do
			if test -f $i/$HACKPAGER
			then
				HACKPAGER=$i/$HACKPAGER
				export HACKPAGER
				break
			fi
		done
		IFS=' 	'
	fi
	if test ! -f $HACKPAGER
	then
		echo Cannot find $PAGER -- unsetting PAGER.
		unset HACKPAGER
		unset PAGER
	fi
fi

if [ x$NH_USE_VALGRIND != x ]; then
	HACK="valgrind --log-file=slashemlog.txt --leak-check=full --track-origins=yes --track-fds=yes $NH_VALGRIND_FLAGS $HACK"
fi
if [ x$NH_USE_GDB != x ]; then
	HACK="gdb $NH_GDB_FLAGS $HACK"
fi
if [ x$NH_USE_LLDB != x ]; then
	HACK="lldb $NH_LLDB_FLAGS $HACK"
fi
if [ x$NH_USE_RR != x ]; then
	HACK="rr record $NH_RR_FLAGS $HACK"
fi

cd $HACKDIR
exec env ASAN_OPTIONS=log_path=asan.log $HACK "$@"
