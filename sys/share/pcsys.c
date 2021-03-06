/*	SCCS Id: @(#)pcsys.c	3.4	2002/01/22		  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  System related functions for OS/2, and Windows NT
 */

#include "hack.h"
#include "wintty.h"

#include <ctype.h>
#include <fcntl.h>
#include <process.h>

#ifdef WIN32
void nethack_exit(int);
#else
#define nethack_exit exit
#endif
static void msexit(void);

#ifdef WIN32
void flushout(void) {
	fflush(stdout);
	return;
}

static const char *COMSPEC = "COMSPEC";

#define getcomspec() nh_getenv(COMSPEC)

#ifdef SHELL
int dosh(void) {
	extern char orgdir[];
	char *comspec;
	int spawnstat;
	if ((comspec = getcomspec())) {
		suspend_nhwindows("To return to NetHack, enter \"exit\" at the system prompt.\n");
		chdirx(orgdir, 0);
		spawnstat = spawnl(P_WAIT, comspec, comspec, NULL);

		if (spawnstat < 0) {
			raw_printf("Can't spawn \"%s\"!", comspec);
			getreturn("to continue");
		}
		chdirx(hackdir, 0);
		get_scr_size(); /* maybe the screen mode changed (TH) */
		resume_nhwindows();
	} else
		pline("Can't find %s.", COMSPEC);
	return 0;
}
#endif	/* SHELL */
#endif	//WIN32

/*
 * Add a backslash to any name not ending in /, \ or :	 There must
 * be room for the \
 */
void append_slash(char *name) {
	char *ptr;

	if (!*name)
		return;
	ptr = name + (strlen(name) - 1);
	if (*ptr != '\\' && *ptr != '/' && *ptr != ':') {
		*++ptr = '\\';
		*++ptr = '\0';
	}
	return;
}

#ifdef WIN32
bool getreturn_enabled;
#endif

void getreturn(const char *str) {
#ifdef WIN32
	if (!getreturn_enabled) return;
#endif
	msmsg("Hit <Enter> %s.", str);
	while (Getchar() != '\n')
		;
	return;
}

void msmsg(const char *fmt, ...) {
	VA_START(fmt);
	vprintf(fmt, VA_ARGS);
	flushout();
	VA_END();
	return;
}

FILE *fopenp(const char *name, const char *mode) {
	char buf[BUFSIZ], *bp, *pp, lastch = 0;
	FILE *fp;

	/* Try the default directory first.  Then look along PATH.
	*/
	strncpy(buf, name, BUFSIZ - 1);
	buf[BUFSIZ - 1] = '\0';
	if ((fp = fopen(buf, mode))) {
		return fp;
	} else {
		int ccnt = 0;
		pp = getenv("PATH");
		while (pp && *pp) {
			bp = buf;
			while (*pp && *pp != ':') {
				lastch = *bp++ = *pp++;
				ccnt++;
			}
			if (lastch != '\\' && lastch != '/') {
				*bp++ = '\\';
				ccnt++;
			}
			strncpy(bp, name, (BUFSIZ - ccnt) - 2);
			bp[BUFSIZ - ccnt - 1] = '\0';
			if ((fp = fopen(buf, mode)))
				return fp;
			if (*pp)
				pp++;
		}
	}
	return NULL;
}

#ifdef WIN32
void nethack_exit(int code) {
	msexit();
	exit(code);
}

static void msexit(void) {
#ifdef CHDIR
	extern char orgdir[];
#endif

	flushout();
#ifndef WIN32
	enable_ctrlP(); /* in case this wasn't done */
#endif
#ifdef CHDIR
	chdir(orgdir); /* chdir, not chdirx */
#ifndef __CYGWIN__
	chdrive(orgdir);
#endif
#endif
	return;
}
#ifdef WIN32
/*
 * This is a kludge.  Just before the release of 3.3.0 the latest
 * version of a popular MAPI mail product was found to exhibit
 * a strange result where the current directory was changed out
 * from under NetHack resulting in a failure of all subsequent
 * file operations in NetHack.  This routine is called prior
 * to all file open/renames/deletes in file.c.
 *
 * A more elegant solution will be sought after 3.3.0 is released.
 */
void dircheck(void) {
	char dirbuf[BUFSZ];
	dirbuf[0] = '\0';
	if (getcwd(dirbuf, sizeof dirbuf) != NULL)
		/* pline("%s,%s",dirbuf,hackdir); */
		if (strcmp(dirbuf, hackdir) != 0)
			chdir(hackdir); /* chdir, not chdirx */
}
#endif
#endif	// WIN32
