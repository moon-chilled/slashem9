/*	SCCS Id: @(#)ntconf.h	3.4	2002/03/10	*/
/* Copyright (c) NetHack PC Development Team 1993, 1994.  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef NTCONF_H
#define NTCONF_H

#define SHELL /* nt use of pcsys routines caused a hang */

#define RANDOM /* have Berkeley random(3) */

#define EXEPATH		     /* Allow .exe location to be used as HACKDIR */
#define TRADITIONAL_GLYPHMAP /* Store glyph mappings at level change time */

#define PC_LOCKING /* Prevent overwrites of aborted or in-progress games */
/* without first receiving confirmation. */

#define HOLD_LOCKFILE_OPEN /* Keep an exclusive lock on the .0 file */

#define SELF_RECOVER /* Allow the game itself to recover from an aborted game */

//#define USER_SOUNDS
/*
 * -----------------------------------------------------------------
 *  The remaining code shouldn't need modification.
 * -----------------------------------------------------------------
 */

#define PORT_HELP "porthelp"

/* Stuff to help the user with some common, yet significant errors */
#define INTERJECT_PANIC	   0
#define INTERJECTION_TYPES (INTERJECT_PANIC + 1)
extern void interject_assistance(int, int, void *, void *);
extern void interject(int);

/* The following is needed for prototypes of certain functions */
#if defined(_MSC_VER)
#include <process.h> /* Provides prototypes of exit(), spawn()      */
#endif

#include <string.h> /* Provides prototypes of strncmpi(), etc.     */
#ifdef STRNCMPI
#ifndef __CYGWIN__
#define strncmpi(a, b, c) strnicmp(a, b, c)
#endif
#endif

#include <sys/types.h>
#include <stdlib.h>

#define PATHLEN	 BUFSZ /* maximum pathlength */
#define FILENAME BUFSZ /* maximum filename length (conservative) */

#if defined(_MAX_PATH) && defined(_MAX_FNAME)
#if (_MAX_PATH < BUFSZ) && (_MAX_FNAME < BUFSZ)
#undef PATHLEN
#undef FILENAME
#define PATHLEN	 _MAX_PATH
#define FILENAME _MAX_FNAME
#endif
#endif

#define NO_SIGNAL
#define index  strchr
#define rindex strrchr
#include <time.h>
#define USE_STDARG

#define FCMASK	   0660 /* file creation mask */
#define regularize nt_regularize
#define HLOCK	   "NHPERM"

#if 0
extern char levels[], bones[], permbones[],
#endif /* 0 */

/* this was part of the micro stuff in the past */
extern const char *alllevels, *allbones;
extern char hackdir[];
#define ABORT	   C('a')
#define getuid()   1
#define getlogin() (NULL)
extern void win32_abort(void);

#include <fcntl.h>

#ifdef LAN_FEATURES
#define MAX_LAN_USERNAME  20
#define LAN_RO_PLAYGROUND /* not implemented in 3.3.0 */
#define LAN_SHARED_BONES  /* not implemented in 3.3.0 */
#include "nhlan.h"
#endif

#ifndef alloca
#define ALLOCA_HACK /* used in util/panic.c */
#endif

#ifdef _MSC_VER
#if 0
#pragma warning(disable : 4018) /* signed/unsigned mismatch */
#pragma warning(disable : 4305) /* init, conv from 'const int' to 'char' */
#endif
#pragma warning(disable : 4761) /* integral size mismatch in arg; conv supp*/
#ifdef YYPREFIX
#pragma warning(disable : 4102) /* unreferenced label */
#endif
#endif

extern int set_win32_option(const char *, const char *);

#endif /* NTCONF_H */
