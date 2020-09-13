/*	SCCS Id: @(#)unixconf.h 3.4	1999/07/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef UNIX
#ifndef UNIXCONF_H
#define UNIXCONF_H

#include "config.h"

/*
 * Some include files are in a different place under SYSV
 *	BSD		   SYSV
 * <sys/time.h>		<time.h>
 * <sgtty.h>		<termio.h>
 *
 * Some routines are called differently
 * index		strchr
 * rindex		strrchr
 *
 */

/* define exactly one of the following four choices */
/* #define BSD 1 */ /* define for 4.n/Free/Open/Net BSD  */
/* also for relatives like SunOS 4.x, DG/UX, and */
/* older versions of Linux */
/* #define ULTRIX */ /* define for Ultrix v3.0 or higher (but not lower) */
/* Use BSD for < v3.0 */
/* "ULTRIX" not to be confused with "ultrix" */
#define SYSV /* define for System V, Solaris 2.x, newer versions */
/* of Linux */
/* #define HPUX */ /* Hewlett-Packard's Unix, version 6.5 or higher */
/* use SYSV for < v6.5 */

/* define any of the following that are appropriate */
#define SVR4 /* use in addition to SYSV for System V Release 4 */
/* including Solaris 2+ */
#define NETWORK /* if running on a networked system */
/* e.g. Suns sharing a playground through NFS */
/* #define SUNOS4 */ /* SunOS 4.x */
#ifdef __linux__
#define LINUX /* Another Unix clone */
#endif
/* #define CYGWIN32 */ /* Unix on Win32 -- use with case sensitive defines */
/* #define GENIX */    /* Yet Another Unix Clone */
/* #define HISX */     /* Bull Unix for XPS Machines */
/* #define BOS */      /* Bull Open Software - Unix for DPX/2 Machines */
/* #define AIX_31 */   /* In AIX 3.1 (IBM RS/6000) use BSD ioctl's to gain
			 * job control (note that AIX is SYSV otherwise)
			 * Also define this for AIX 3.2 */

#define TERMINFO /* uses terminfo rather than termcap */
/* Should be defined for most SYSV, SVR4 (including
 * Solaris 2+), HPUX, and Linux systems.
 */
#define POSIX_JOB_CONTROL /* use System V / Solaris 2.x / POSIX job control */
/* (e.g., VSUSP) */
#define POSIX_TYPES /* use POSIX types for system calls and termios */
/* Define for many recent OS releases, including
 * those with specific defines (since types are
 * changing toward the standard from earlier chaos).
 * For example, platforms using the GNU libraries,
 * Linux, Solaris 2.x
 */

/* #define RANDOM */ /* if neither random/srandom nor lrand48/srand48
				   is available from your system */

/* see sys/unix/snd86unx.shr for more information on these */
/* #define UNIX386MUSIC */ /* play real music through speaker on systems
				   with music driver installed */
/* #define VPIX_MUSIC */   /* play real music through speaker on systems
				   with built-in VPIX support */

/*
 * Define PORT_HELP to be the name of the port-specfic help file.
 * This file is found in HACKDIR.
 * Normally, you shouldn't need to change this.
 * There is currently no port-specific help for Unix systems.
 */
/* #define PORT_HELP "Unixhelp" */

/*
 * If you define MAIL, then the player will be notified of new mail
 * when it arrives.
 * A stat system call is done on the mailbox every MAILCKFREQ moves.
 */

/* #define MAIL */ /* Deliver mail during the game */

/* The Andrew Message System does mail a little differently from normal
 * UNIX.  Mail is deposited in the user's own directory in ~/Mailbox
 * (another directory).  MAILBOX is the element that will be added on to
 * the user's home directory path to generate the Mailbox path - just in
 * case other Andrew sites do it differently from CMU.
 *
 *		dan lovinger
 *		dl2n+@andrew.cmu.edu (dec 19 1989)
 */

/* NO_MAILREADER is for kerberos authenticating filesystems where it is
 * essentially impossible to securely exec child processes, like mail
 * readers, when the game is running under a special token.
 *
 *	       dan
 */

/* #define NO_MAILREADER */ /* have mail daemon just tell player of mail */

#ifdef MAIL
# define MAILCKFREQ 50
#endif /* MAIL */

#define FCMASK 0660 /* file creation mask */

/* fcntl(2) is a POSIX-portable call for manipulating file descriptors.
 * Comment out the USE_FCNTL if for some reason you have a strange
 * os/filesystem combination for which fcntl(2) does not work. */
#ifdef POSIX_TYPES
#define USE_FCNTL
#endif

/*
 * The remainder of the file should not need to be changed.
 */

#ifdef _AUX_SOURCE
#ifdef AUX /* gcc ? */
#define _SYSV_SOURCE
#define _BSD_SOURCE
#else
#define AUX
#endif
#endif /* _AUX_SOURCE */

#if defined(LINUX) || defined(bsdi)
#ifndef POSIX_TYPES
#define POSIX_TYPES
#endif
#ifndef POSIX_JOB_CONTROL
#define POSIX_JOB_CONTROL
#endif
#endif

/*
 * BSD/ULTRIX systems are normally the only ones that can suspend processes.
 * Suspending NetHack processes cleanly should be easy to add to other systems
 * that have SIGTSTP in the Berkeley sense.  Currently the only such systems
 * known to work are HPUX and AIX 3.1; other systems will probably require
 * tweaks to unixtty.c and ioctl.c.
 *
 * POSIX defines a slightly different type of job control, which should be
 * equivalent for NetHack's purposes.  POSIX_JOB_CONTROL should work on
 * various recent SYSV versions (with possibly tweaks to unixtty.c again).
 */
#ifndef POSIX_JOB_CONTROL
#if defined(BSD) || defined(ULTRIX) || defined(HPUX) || defined(AIX_31)
#define BSD_JOB_CONTROL
#else
#if defined(SVR4)
#define POSIX_JOB_CONTROL
#endif
#endif
#endif
#if defined(BSD_JOB_CONTROL) || defined(POSIX_JOB_CONTROL) || defined(AUX)
#define SUSPEND /* let ^Z suspend the game */
#endif

#if defined(BSD) || defined(ULTRIX)
#include <sys/time.h>
#else
#include <time.h>
#endif

#define HLOCK "perm" /* an empty file used for locking purposes */

#define tgetch getchar

#include "system.h"

#if defined(POSIX_TYPES) || defined(__GNUC__)
#include <stdlib.h>
#include <unistd.h>
#endif

#if defined(POSIX_TYPES) || defined(__GNUC__) || defined(BSD) || defined(ULTRIX)
#include <sys/wait.h>
#endif

#if defined(BSD) || defined(ULTRIX)
#if !defined(SUNOS4)
#define memcpy(d, s, n)	  bcopy(s, d, n)
#define memcmp(s1, s2, n) bcmp(s2, s1, n)
#endif
#ifdef SUNOS4
#include <memory.h>
#endif
#else	      /* therefore SYSV */
#ifndef index /* some systems seem to do this for you */
#define index strchr
#endif
#ifndef rindex
#define rindex strrchr
#endif
#endif

#include "tre.h"

#endif /* UNIXCONF_H */
#endif /* UNIX */
