/*	SCCS Id: @(#)unixunix.c	3.4	1994/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* This file collects some Unix dependencies */

#include "hack.h"	/* mainly for index() which depends on BSD */

#include <errno.h>
#include <sys/stat.h>
#if defined(NO_FILE_LINKS) || defined(SUNOS4) || defined(POSIX_TYPES)
#include <fcntl.h>
#endif
#include <signal.h>

static int veryold(int);
static int eraseoldlocks(void);

static struct stat buf;

/* see whether we should throw away this xlock file */
static int veryold(int fd) {
	time_t date;

	if(fstat(fd, &buf)) return 0;			/* cannot get status */
#ifndef INSURANCE
	if(buf.st_size != sizeof(int)) return 0;	/* not an xlock file */
#endif
#if defined(BSD) && !defined(POSIX_TYPES)
	time((long *)(&date));
#else
	time(&date);
#endif
	if(date - buf.st_mtime < 3L*24L*60L*60L) {	/* recent */
		int lockedpid;	/* should be the same size as hackpid */

		if(read(fd, (void *)&lockedpid, sizeof(lockedpid)) !=
			sizeof(lockedpid))
			/* strange ... */
			return 0;

		/* From: Rick Adams <seismo!rick> */
		/* This will work on 4.1cbsd, 4.2bsd and system 3? & 5. */
		/* It will do nothing on V7 or 4.1bsd. */
#ifndef NETWORK
		/* It will do a VERY BAD THING if the playground is shared
		   by more than one machine! -pem */
		if(!(kill(lockedpid, 0) == -1 && errno == ESRCH))
#endif
			return 0;
	}
	close(fd);
	return 1;
}

static int eraseoldlocks(void) {
	int i;

	/* cannot use maxledgerno() here, because we need to find a lock name
	 * before starting everything (including the dungeon initialization
	 * that sets astral_level, needed for maxledgerno()) up
	 */
	for(i = 1; i <= MAXDUNGEON*MAXLEVEL + 1; i++) {
		/* try to remove all */
		set_levelfile_name(lock, i);
		unlink(lock);
	}
	set_levelfile_name(lock, 0);
	if (unlink(lock))
		return 0;				/* cannot remove it */

	return 1;					/* success! */
}

void getlock(void) {
	int i = 0, fd, c;

#ifdef TTY_GRAPHICS
	/* idea from rpick%ucqais@uccba.uc.edu
	 * prevent automated rerolling of characters
	 * test input (fd0) so that tee'ing output to get a screen dump still
	 * works
	 * also incidentally prevents development of any hack-o-matic programs
	 */
	/* added check for window-system type -dlc */
	if (!strcmp(windowprocs.name, "tty"))
	    if (!isatty(0))
		error("You must play from a terminal.");
#endif

	/* we ignore QUIT and INT at this point */
	if (!lock_file(HLOCK, LOCKPREFIX, 10)) {
		wait_synch();
		error("%s", "");
	}

	regularize(lock);
	set_levelfile_name(lock, 0);

	if(locknum) {
		if(locknum > 25) locknum = 25;

		do {
			lock[0] = 'a' + i++;

			if((fd = open(lock, 0, 0)) == -1) {
			    if(errno == ENOENT) goto gotlock; /* no such file */
			    perror(lock);
			    unlock_file(HLOCK);
			    error("Cannot open %s", lock);
			}

			if(veryold(fd) /* closes fd if true */
							&& eraseoldlocks())
				goto gotlock;
			close(fd);
		} while(i < locknum);

		unlock_file(HLOCK);
		error("Too many hacks running now.");
	} else {
		if((fd = open(lock, 0, 0)) == -1) {
			if(errno == ENOENT) goto gotlock;    /* no such file */
			perror(lock);
			unlock_file(HLOCK);
			error("Cannot open %s", lock);
		}

		if(veryold(fd) /* closes fd if true */ && eraseoldlocks())
			goto gotlock;
		close(fd);

		if(iflags.window_inited) {
		    c = yn("There is already a game in progress under your name.  Destroy old game?");
		} else {
		    printf("\nThere is already a game in progress under your name.");
		    printf("  Destroy old game? [yn] ");
		    fflush(stdout);
		    c = getchar();
		    putchar(c);
		    fflush(stdout);
		    while (getchar() != '\n') ; /* eat rest of line and newline */
		}
		if(c == 'y' || c == 'Y') {
			if(eraseoldlocks()) {
				goto gotlock;
			} else {
				unlock_file(HLOCK);
				error("Couldn't destroy old game.");
			}
		} else {
			unlock_file(HLOCK);
			if (iflags.window_inited) {
				exit_nhwindows(NULL);
			}
			error("%s", "");
		}
	}

gotlock:
	fd = creat(lock, FCMASK);
	unlock_file(HLOCK);
	if(fd == -1) {
		error("cannot creat lock file (%s).", lock);
	} else {
		if(write(fd, &hackpid, sizeof(hackpid))
		    != sizeof(hackpid)){
			error("cannot write lock (%s)", lock);
		}
		if(close(fd) == -1) {
			error("cannot close lock (%s)", lock);
		}
	}
}

/* normalize file name - we don't like .'s, /'s, spaces */
void regularize(char *s) {
	char *lp;

	while((lp=index(s, '.')) || (lp=index(s, '/')) || (lp=index(s,' ')))
		*lp = '_';
#if defined(SYSV) && !defined(AIX_31) && !defined(SVR4) && !defined(LINUX) && !defined(__APPLE__)
	/* avoid problems with 14 character file name limit */
	if(strlen(s) > 11)
		/* leave room for .nn appended to level files */
		s[11] = '\0';
#endif
}

#ifdef GETRES_SUPPORT

extern int nh_getresuid(uid_t *, uid_t *, uid_t *);
extern uid_t nh_getuid(void);
extern uid_t nh_geteuid(void);
extern int nh_getresgid(gid_t *, gid_t *, gid_t *);
extern gid_t nh_getgid(void);
extern gid_t nh_getegid(void);

int getresuid(uid_t *ruid, uid_t *euid, uid_t *suid) {
	return nh_getresuid(ruid, euid, suid);
}

uid_t getuid(void) {
	return nh_getuid();
}

uid_t geteuid(void) {
	return nh_geteuid();
}

int getresgid(gid_t *rgid, gid_t *egid, gid_t *sgid) {
	return nh_getresgid(rgid, egid, sgid);
}

gid_t getgid(void) {
	return nh_getgid();
}

gid_t getegid(void) {
	return nh_getegid();
}

#endif	/* GETRES_SUPPORT */
