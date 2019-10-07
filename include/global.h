/*	SCCS Id: @(#)global.h	3.4	2003/08/31	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdbool.h>

#define ALPHA		/* if an alpha-test copy */
#define BETA		/* if an alpha- or beta-test copy */

/*
 * Files expected to exist in the playground directory if file areas are not
 * enabled and in the named areas otherwise.
 * [WAC] - put a NH_ prefix on all the names to prevent conflicts
 */

#define NH_RECORD		"record"  /* a file containing list of topscorers */
#define NH_RECORD_AREA	FILE_AREA_VAR
#define NH_HELP		"help"	  /* a file containing command descriptions */
#define NH_HELP_AREA	FILE_AREA_SHARE
#define NH_SHELP		"hh"		/* abbreviated form of the same */
#define NH_SHELP_AREA	FILE_AREA_SHARE
#define NH_DEBUGHELP	"wizhelp"	/* a file containing debug mode cmds */
#define NH_DEBUGHELP_AREA	FILE_AREA_SHARE
#define NH_RUMORFILE	"rumors"	/* a file with fortune cookies */
#define NH_RUMORAREA	FILE_AREA_SHARE
#define NH_ORACLEFILE	"oracles"	/* a file with oracular information */
#define NH_ORACLEAREA	FILE_AREA_SHARE
#define NH_DATAFILE	"data"	/* a file giving the meaning of symbols used */
#define NH_DATAAREA	FILE_AREA_SHARE
#define NH_CMDHELPFILE	"cmdhelp"	/* file telling what commands do */
#define NH_CMDHELPAREA	FILE_AREA_SHARE
#define NH_HISTORY		"history"	/* a file giving nethack's history */
#define NH_HISTORY_AREA	FILE_AREA_SHARE
#define NH_LICENSE		"license"	/* file with license information */
#define NH_LICENSE_AREA	FILE_AREA_DOC
#define NH_OPTIONFILE	"opthelp"	/* a file explaining runtime options */
#define NH_OPTIONAREA	FILE_AREA_SHARE
#define NH_OPTIONS_USED	"options"	/* compile-time options, for #version */
#define NH_OPTIONS_USED_AREA FILE_AREA_SHARE
#define NH_GUIDEBOOK       "Guidebook.txt"       /* Nethack Guidebook*/
#define NH_GUIDEBOOK_AREA	FILE_AREA_DOC

#define LEV_EXT	".lev"		/* extension for special level files */


/* Assorted definitions that may depend on selections in config.h. */

/*
 * type xchar: small integers in the range 0 - 127, usually coordinates
 * although they are nonnegative they must not be declared unsigned
 * since otherwise comparisons with signed quantities are done incorrectly
 */
typedef schar	xchar;
#ifndef SKIP_BOOLEAN
typedef xchar	boolean;		/* 0 or 1 */
#endif

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

#ifndef STRNCMPI
# if !defined(__SASC_60) && !defined(__MINGW32__) /* SAS/C already shifts to stricmp */
#  define strcmpi(a,b) strncmpi((a),(b),-1)
# endif
#endif

/* comment out to test effects of each #define -- these will probably
 * disappear eventually
 */
#ifdef INTERNAL_COMP
# define RLECOMP	/* run-length compression of levl array - JLee */
# define ZEROCOMP	/* zero-run compression of everything - Olaf Seibert */
#endif

/* #define SPECIALIZATION */	/* do "specialized" version of new topology */


#ifdef BITFIELDS
#define Bitfield(x,n)	unsigned x:n
#else
#define Bitfield(x,n)	uchar x
#endif

#define SIZE(x)	(int)(sizeof(x) / sizeof(x[0]))


/* A limit for some NetHack int variables.  It need not, and for comparable
 * scoring should not, depend on the actual limit on integers for a
 * particular machine, although it is set to the minimum required maximum
 * signed integer for C (2^15 -1).
 */
#define LARGEST_INT	32767


#ifdef REDO
#define Getchar pgetchar
#else
#define Getchar nhgetch
#endif

#include "coord.h"
/*
 * Automatic inclusions for the subsidiary files.
 * Please don't change the order.  It does matter.
 */

#ifdef UNIX
#include "unixconf.h"
#endif

#ifdef WIN32
#include "ntconf.h"
#endif

#ifndef FILE_AREAS

#define fopen_datafile_area(area, filename, mode, use_spfx) \
 		fopen_datafile(filename, mode, use_spfx)
#define lock_file_area(area, filename, prefix, retryct) \
 		lock_file(filename, prefix, retryct)
#define unlock_file_area(area, filename) unlock_file(filename)
#define dlb_fopen_area(area, name, mode) dlb_fopen(name, mode)

/*
 * ALI
 *
 * By defining these, functions can pass them around even though they're
 * not actually used. This can make the code easier to read at the cost
 * of some efficiency. Given the high overhead of dealing with files anyway,
 * this is often a good trade-off.
 */

#define FILE_AREA_VAR		NULL
#define FILE_AREA_SAVE		NULL
#define FILE_AREA_LEVL		NULL
#define FILE_AREA_BONES		NULL
#define FILE_AREA_SHARE		NULL
#define FILE_AREA_UNSHARE	NULL

#endif

/* Displayable name of this port; don't redefine if defined in *conf.h */
#ifndef PORT_ID
# ifdef MAC
#  define PORT_ID	"Mac"
#  if 0
#  ifdef MAC_MPW_PPC
#   define PORT_SUB_ID "PPC"
#  else
#   ifdef MAC_MPW_68K
#    define PORT_SUB_ID "68K"
#   endif
#  endif
#  endif
# endif
# ifdef UNIX
#  define PORT_ID	"Unix"
# endif
# ifdef WIN32
#  define PORT_ID	"Windows"
#  ifndef PORT_SUB_ID
#   define PORT_SUB_ID	"tty"
#  endif
# endif
#endif

#ifndef EXIT_SUCCESS
# define EXIT_SUCCESS 0
#endif
#ifndef EXIT_FAILURE
# define EXIT_FAILURE 1
#endif

void *alloc(size_t);		/* alloc.c */
#define _newtype(typ, amnt) alloc(sizeof(typ) * amnt)
//#define _new_defvar(type, varname, amnt) type *varname = _new(sizeof(type), amnt)
//#define _new_setvar(varname, amnt) varname = _new(sizeof(*varname), amnt)



// This is magic.  It means #define new(type typ, size_t amnt = 1)
#define _newtype_f1(...) _newtype(__VA_ARGS__, 1)
#define _newtype_f2(...) _newtype(__VA_ARGS__)
#define _newtype_fx(_1, _2, n, ...) n
#define new(...) _newtype_fx(__VA_ARGS__, _newtype_f2(__VA_ARGS__), _newtype_f1(__VA_ARGS__), 0)

/*
#define new_setvar_f1(...) _new_setvar(__VA_ARGS__, 1)
#define new_setvar_f2(...) _new_setvar(__VA_ARGS__)
#define new_setvar_fx(_1, _2, n, ...) n
#define new_setvar(...) new_setvar_fx(__VA_ARGS__, new_setvar_f2(__VA_ARGS__), new_setvar_f1(__VA_ARGS__), 0)


#define new_defvar_f1(...) _new_defvar(__VA_ARGS__, 1)
#define new_defvar_f2(...) _new_defvar(__VA_ARGS__)
#define new_defvar_fx(_1, _2, _3, n, ...) n
#define new_defvar(...) new_defvar_fx(__VA_ARGS__, new_defvar_f2(__VA_ARGS__), new_defvar_f1(__VA_ARGS__), 0)
*/


/* Used for consistency checks of various data files; declare it here so
   that utility programs which include config.h but not hack.h can see it. */
struct version_info {
	unsigned long	incarnation;	/* actual version number */
	unsigned long	feature_set;	/* bitmask of config settings */
	unsigned long	entity_count;	/* # of monsters and objects */
	unsigned long	struct_sizes;	/* size of key structs */
};

/*
 * Configurable internal parameters.
 *
 * Please be very careful if you are going to change one of these.  Any
 * changes in these parameters, unless properly done, can render the
 * executable inoperative.
 */

/* size of terminal screen is (at least) (ROWNO+3) by COLNO */
#define COLNO	80
#define ROWNO	21

/* MAXCO must hold longest uncompressed status line, and must be larger
 * than COLNO
 *
 * longest practical second status line at the moment is
 *      Astral Plane $:12345 HP:700(700) Pw:111(111) AC:-127 Xp:30/123456789
 *      Wt:5000/1000 T:123456 Satiated Lev Conf FoodPois Ill Blind Stun Hallu
 *      Slime Held Overloaded
 * -- or somewhat over 160 characters
 */
#if COLNO <= 170
#define MAXCO 190
#else
#define MAXCO (COLNO+20)
#endif

#define MAXNROFROOMS	40	/* max number of rooms per level */
#define MAX_SUBROOMS	24	/* max # of subrooms in a given room */
#define DOORMAX		120	/* max number of doors per level */

#define BUFSZ		256	/* for getlin buffers */
#define QBUFSZ		128	/* for building question text */
#define TBUFSZ		300	/* toplines[] buffer max msg: 3 81char names */
				/* plus longest prefix plus a few extra words */

#define PL_NSIZ		32	/* name of player, ghost, shopkeeper */
#define PL_CSIZ		32	/* sizeof pl_character */
#define PL_FSIZ		32	/* fruit name */
#define PL_PSIZ		63	/* player-given names for pets, other
				 * monsters, objects */

#define MAXDUNGEON	32	/* current maximum number of dungeons */
#define MAXLEVEL	50	/* max number of levels in one dungeon */
#define MAXSTAIRS	1	/* max # of special stairways in a dungeon */
#define ALIGNWEIGHT	10	/* generation weight of alignment */

#define MAXULEV		30	/* max character experience level */

#define MAXMONNO	120	/* extinct monst after this number created */
#define MHPMAX		500	/* maximum monster hp */

#endif /* GLOBAL_H */
