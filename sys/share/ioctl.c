/*	SCCS Id: @(#)ioctl.c	3.4	1990/22/02 */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This cannot be part of hack.tty.c (as it was earlier) since on some
   systems (e.g. MUNIX) the include files <termio.h> and <sgtty.h>
   define the same constants, and the C preprocessor complains. */

#include "hack.h"

#include <termios.h>
#include <sys/ioctl.h>
struct termios termio;

#ifdef SUSPEND /* BSD isn't alone anymore... */
#include <signal.h>
#endif

#if defined(TIOCGWINSZ) && (defined(BSD) || defined(ULTRIX) || defined(AIX_31) || defined(_BULL_SOURCE) || defined(SVR4))
#define USE_WIN_IOCTL
#include "tcap.h" /* for LI and CO */
#endif

#ifdef AUX
void catch_stp(void) {
	signal(SIGTSTP, SIG_DFL);
	dosuspend();
}
#endif /* AUX */

void getwindowsz(void) {
	struct winsize ttsz;

	if (ioctl(0, TIOCGWINSZ, &ttsz) != -1) {
		if (ttsz.ws_row)
			LI = ttsz.ws_row;
		if (ttsz.ws_col)
			CO = ttsz.ws_col;
	}
}

void getioctls(void) {
	getwindowsz();
#define POSIX_TYPES
#ifdef BSD_JOB_CONTROL
	ioctl(fileno(stdin), (int)TIOCGLTC, (char *)&ltchars);
	ioctl(fileno(stdin), (int)TIOCSLTC, (char *)&ltchars0);
#else
#ifdef POSIX_TYPES
	tcgetattr(fileno(stdin), &termio);
#else
#if defined(TCSETS)
	ioctl(fileno(stdin), (int)TCGETS, &termio);
#else
	ioctl(fileno(stdin), (int)TCGETA, &termio);
#endif
#endif
#endif
#ifdef AUX
	signal(SIGTSTP, catch_stp);
#endif
}

void setioctls(void) {
#ifdef BSD_JOB_CONTROL
	ioctl(fileno(stdin), (int)TIOCSLTC, (char *)&ltchars);
#else
#ifdef POSIX_TYPES
	tcsetattr(fileno(stdin), TCSADRAIN, &termio);
#else
#if defined(TCSETS) && !defined(AIX_31)
	ioctl(fileno(stdin), (int)TCSETSW, &termio);
#else
	ioctl(fileno(stdin), (int)TCSETAW, &termio);
#endif
#endif
#endif
}

#ifdef SUSPEND /* No longer implies BSD */
int dosuspend(void) {
#ifdef SIGTSTP
	if (signal(SIGTSTP, SIG_IGN) == SIG_DFL) {
		suspend_nhwindows(NULL);
		signal(SIGTSTP, SIG_DFL);
#ifdef AUX
		kill(0, SIGSTOP);
#else
		kill(0, SIGTSTP);
#endif
		resume_nhwindows();
	} else {
		pline("I don't think your shell has job control.");
	}
#else
	pline("Sorry, it seems we have no SIGTSTP here.  Try ! or S.");
#endif
	return 0;
}
#endif /* SUSPEND */
