/*	SCCS Id: @(#)pcunix.c	3.4	1994/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file collects some Unix dependencies; pager.c contains some more */

#include "hack.h"
#include "wintty.h"

#include <sys/stat.h>
#ifdef WIN32
#include <errno.h>
#endif

#ifdef WIN32
extern char orgdir[];
extern void backsp(void);
extern void clear_screen(void);
#endif

#ifdef WANT_GETHDATE
static struct stat hbuf;
#endif

#ifdef PC_LOCKING
static int eraseoldlocks(void);
#endif

#ifdef PC_LOCKING
static int eraseoldlocks(void) {
	int i;

	/* cannot use maxledgerno() here, because we need to find a lock name
	 * before starting everything (including the dungeon initialization
	 * that sets astral_level, needed for maxledgerno()) up
	 */
	for (i = 1; i <= MAXDUNGEON * MAXLEVEL + 1; i++) {
		/* try to remove all */
		set_levelfile_name(lock, i);
		unlink(lock);
	}
	set_levelfile_name(lock, 0);
	if (unlink(lock))
		return 0; /* cannot remove it */
	return 1;	  /* success! */
}

void getlock(void) {
	int fd, c, ci, ct;
	char tbuf[BUFSZ];
	/* we ignore QUIT and INT at this point */
	if (!lock_file(HLOCK, LOCKPREFIX, 10)) {
		wait_synch();
		chdirx(orgdir, 0);
		error("Quitting.");
	}

	/* regularize(lock); */ /* already done in pcmain */
	sprintf(tbuf, lockj);
	set_levelfile_name(lock, 0);
	rf((fd = open(lock, 0)) == -1) {
		if (errno == ENOENT) goto gotlock; /* no such file */
		chdirx(orgdir, 0);
		perror(lock);
		unlock_file(HLOCK);
		error("Cannot open %s", lock);
	}

	close(fd);

	if (iflags.window_inited) {
		pline("There is already a game in progress under your name.");
		pline("You may be able to use \"recover %s\" to get it back.\n", tbuf);
		c = yn("Do you want to destroy the old game?");
	} else {
		c = 'n';
		ct = 0;
		msmsg("\nThere is already a game in progress under your name.\n");
		msmsg("If this is unexpected, you may be able to use \n");
		msmsg("\"recover %s\" to get it back.", tbuf);
		msmsg("\nDo you want to destroy the old game? [yn] ");
		while ((ci = nhgetch()) != '\n') {
			if (ct > 0) {
				msmsg("\b \b");
				ct = 0;
				c = 'n';
			}
			if (ci == 'y' || ci == 'n' || ci == 'Y' || ci == 'N') {
				ct = 1;
				c = ci;
				msmsg("%c", c);
			}
		}
	}
	if (c == 'y' || c == 'Y')
		if (eraseoldlocks()) {
			goto gotlock;
		} else {
			unlock_file(HLOCK);
			chdirx(orgdir, 0);
			error("Couldn't destroy old game.");
		}
	else {
		unlock_file(HLOCK);
		chdirx(orgdir, 0);
		error("%s", "");
	}

gotlock:
	fd = creat(lock, FCMASK);
	unlock_file(HLOCK);
	if (fd == -1) {
		chdirx(orgdir, 0);
		error("cannot creat lock file (%s.)", lock);
	} else {
		if (write(fd, (char *)&hackpid, sizeof(hackpid)) != sizeof(hackpid)) {
			chdirx(orgdir, 0);
			error("cannot write lock (%s)", lock);
		}
		if (close(fd) == -1) {
			chdirx(orgdir, 0);
			error("cannot close lock (%s)", lock);
		}
	}
}
#endif /* PC_LOCKING */

#ifndef WIN32
/*
 * normalize file name - we don't like .'s, /'s, spaces, and
 * lots of other things
 */
void regularize(char *s) {
	char *lp;

	for (lp = s; *lp; lp++)
		if (*lp <= ' ' || *lp == '"' || (*lp >= '*' && *lp <= ',') ||
		    *lp == '.' || *lp == '/' || (*lp >= ':' && *lp <= '?') ||
		    *lp == '|' || *lp >= 127 || (*lp >= '[' && *lp <= ']'))
			*lp = '_';
}
#endif /* WIN32 */

#ifdef __EMX__
void seteuid(int i) {
}
#endif
