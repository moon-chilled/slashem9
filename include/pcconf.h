/*	SCCS Id: @(#)pcconf.h	3.4	1995/10/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef PCCONF_H
#define PCCONF_H

#define MICRO		/* always define this! */

#define PATHLEN		64	/* maximum pathlength */
#define FILENAMELEN	80	/* maximum filename length (conservative) */
#ifndef MICRO_H
#include "micro.h"		/* contains necessary externs for [os_name].c */
#endif


/* ===================================================
 *  The remaining code shouldn't need modification.
 */

#ifndef SYSTEM_H
#include "system.h"
#endif

#ifdef __DJGPP__
#include <unistd.h> /* close(), etc. */
/* lock() in io.h interferes with lock[] in decl.h */
#define lock djlock
#include <io.h>
#undef lock
#include <pc.h> /* kbhit() */
#define PC_LOCKING
#define HOLD_LOCKFILE_OPEN
#define SELF_RECOVER		/* NetHack itself can recover games */
#endif

#ifdef PC_LOCKING
#define HLOCK "NHPERM"
#endif

#ifndef index
# define index	strchr
#endif
#ifndef rindex
# define rindex strrchr
#endif

#include <time.h>

#ifdef RANDOM
/* Use the high quality random number routines. */
# define Rand() random()
#else
# define Rand() rand()
#endif

# define FCMASK 0660	/* file creation mask */

#include <fcntl.h>

#ifndef REDO
# undef Getchar
# define Getchar nhgetch
#endif

#ifdef MSC7_WARN	/* define with cl /DMSC7_WARN	*/
#pragma warning(disable:4131)
#endif

#ifdef TIMED_DELAY
# ifdef __DJGPP__
# define msleep(k) (void) usleep((k)*1000)
# endif
# ifdef __SC__
# define msleep(k) (void) usleep((long)((k)*1000))
# endif
#endif

#endif /* PCCONF_H */
