/*	SCCS Id: @(#)config.h	3.4	2003/12/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG_H /* make sure the compiler does not see the typedefs twice */
#define CONFIG_H

/*#define DEBUG*/
/*#define DDEBUG*/

/*
 * Section 1:	Operating and window systems selection.
 *		Select the version of the OS you are using.
 *		For "UNIX" select BSD, ULTRIX, SYSV, or HPUX in unixconf.h.
 */

#define UNIX		/* delete if no fork(), exec() available */

/* #define MINIMAL_TERM */
			/* if a terminal handles highlighting or tabs poorly,
  			   try this define, used in pager.c and termcap.c */
/* #define ULTRIX_CC20 */
			/* define only if using cc v2.0 on a DECstation */
/* #define ULTRIX_PROTO */
			/* define for Ultrix 4.0 (or higher) on a DECstation;
  			 * if you get compiler errors, don't define this. */
  			/* Hint: if you're not developing code, don't define
  			   ULTRIX_PROTO. */

#include "config1.h"	/* should auto-detect MAC and WIN32 */

/* Windowing systems...
 * Define all of those you want supported in your binary.
 * Some combinations make no sense.  See the installation document.
 */
#define TTY_GRAPHICS		/* good old tty based graphics */
#define CURSES_GRAPHICS		/* awful curses interface */
/* #define KDE */		/* KDE Interface */
/* #define PROXY_GRAPHICS */	/* Plug-in interfaces */
/* #define MSWIN_GRAPHICS */	/* Windows NT, CE, Graphics */
/* #define GL_GRAPHICS */	/* OpenGL graphics */
/* #define SDL_GRAPHICS */	/* Software SDL graphics */

/*
 * Define the default window system.  This should be one that is compiled
 * into your system (see defines above).  Known window systems are:
 *
 *	tty, mac, Gnome, gtk, proxy, GL, SDL
 */

/* MAC also means MAC windows */
#ifdef MAC
# ifndef	AUX
#  define DEFAULT_WINDOW_SYS "Mac"
# endif
#endif

#if 0 /* Removed in 3.3.0 */
/* Windows NT supports TTY_GRAPHICS */
#ifdef WIN32
#  define DEFAULT_WINDOW_SYS "tty"
#endif
#endif

#ifdef PROXY_GRAPHICS
# define USE_XPM		/* Use XPM format for images */
/*
 * The proxy interface shouldn't be used as the default window system.
 * This will cause it to always be initialized with undesirable side
 * effects. Instead, use the windowtype option.  --ALI
 */
#endif

#ifdef MSWIN_GRAPHICS
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "mswin"
# endif
# define HACKDIR "\\nethack"
#endif

#ifdef GL_GRAPHICS
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "GL"
# endif
#endif

#ifdef SDL_GRAPHICS
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "SDL"
# endif
#endif

#if defined(GL_GRAPHICS) || defined(SDL_GRAPHICS)
# define GRAPHIC_TOMBSTONE     /* Use graphical tombstone */
/* -AJA- workaround for clash with ZLIB headers */
# if defined(VANILLA_GLHACK)
#  define compress    nh_compress
#  define uncompress  nh_uncompress
# endif
#endif

#ifdef CURSES_GRAPHICS
# ifndef DEFAULT_WINDOW_SYS
#  define DEFAULT_WINDOW_SYS "curses"
# endif
#endif

#ifndef DEFAULT_WINDOW_SYS
# define DEFAULT_WINDOW_SYS "tty"
#endif

/*
 * Section 2:	Some global parameters and filenames.
 *		Commenting out WIZARD, LOGFILE, NEWS or PANICLOG removes that
 *		feature from the game; otherwise set the appropriate wizard
 *		name.  LOGFILE, NEWS and PANICLOG refer to files in the
 *		playground.
 */

#ifndef WIZARD		/* allow for compile-time or Makefile changes */
# define WIZARD  "wizard" /* the person allowed to use the -D option */
#endif

#define XLOGFILE "xlogfile"
#define LOGFILE "logfile"	/* larger file for debugging purposes */
#define LOGAREA FILE_AREA_VAR
#define NEWS "news"		/* the file containing the latest hack news */
#define NEWS_AREA FILE_AREA_SHARE
#define PANICLOG "paniclog"	/* log of panic and impossible events */


#define INTERNAL_COMP	/* control use of NetHack's compression routines */


/*
 *	Data librarian.  Defining DLB places most of the support files into
 *	a tar-like file, thus making a neater installation.  See *conf.h
 *	for detailed configuration.
 */
/* #define DLB */             /* not supported on all platforms */

/*
 *	Defining INSURANCE slows down level changes, but allows games that
 *	died due to program or system crashes to be resumed from the point
 *	of the last level change, after running a utility program.
 */
#define INSURANCE	/* allow crashed game recovery */

#ifndef MAC
# define CHDIR		/* delete if no chdir() available */
#endif

#ifdef CHDIR
/*
 * If you define HACKDIR, then this will be the default playground;
 * otherwise it will be the current directory.
 */
# ifndef HACKDIR
#  ifdef __APPLE__
#    define HACKDIR "nethackdir"      /* nethack directory */
#  else
#    define HACKDIR "."
#  endif
# endif

/*
 * Some system administrators are stupid enough to make Hack suid root
 * or suid daemon, where daemon has other powers besides that of reading or
 * writing Hack files.  In such cases one should be careful with chdir's
 * since the user might create files in a directory of his choice.
 * Of course SECURE is meaningful only if HACKDIR is defined.
 */
/* #define SECURE */	/* do setuid(getuid()) after chdir() */

/*
 * If it is desirable to limit the number of people that can play Hack
 * simultaneously, define HACKDIR, SECURE and MAX_NR_OF_PLAYERS.
 * #define MAX_NR_OF_PLAYERS 6
 */
#endif /* CHDIR */



/*
 * Section 3:	Definitions that may vary with system type.
 *		For example, both schar and uchar should be short ints on
 *		the AT&T 3B2/3B5/etc. family.
 */

/*
 * Uncomment the following line if your compiler falsely claims to be
 * a standard C compiler (i.e., defines __STDC__ without cause).
 * Examples are Apollo's cc (in some versions) and possibly SCO UNIX's rcc.
 */
/* #define NOTSTDC */			/* define for lying compilers */

#include "tradstdc.h"

/*
 * type schar: small signed integers (8 bits suffice)
 */
typedef signed char	schar;

/*
 * type uchar: small unsigned integers (8 bits suffice - but 7 bits do not)
 *
 *	typedef unsigned char	uchar;
 *
 *	will be satisfactory if you have an "unsigned char" type;
 *	otherwise use
 *
 *	typedef unsigned short int uchar;
 */
#ifndef _AIX32		/* identical typedef in system file causes trouble */
typedef unsigned char	uchar;
#endif

#define REALTIME_ON_BOTL        /* Show elapsed time on bottom line.  Note:
                                 * this breaks savefile compatibility. */

/*
 * Various structures have the option of using bitfields to save space.
 * If your C compiler handles bitfields well (e.g., it can initialize structs
 * containing bitfields), you can define BITFIELDS.  Otherwise, the game will
 * allocate a separate character for each bitfield.  (The bitfields used never
 * have more than 7 bits, and most are only 1 bit.)
 */
#define BITFIELDS	/* Good bitfield handling */

/* #define STRNCMPI */ /* compiler/library has the strncmpi function */

/*
 * There are various choices for the NetHack vision system.  There is a
 * choice of two algorithms with the same behavior.  Defining VISION_TABLES
 * creates huge (60K) tables at compile time, drastically increasing data
 * size, but runs slightly faster than the alternate algorithm.  (MSDOS in
 * particular cannot tolerate the increase in data size; other systems can
 * flip a coin weighted to local conditions.)
 *
 * If VISION_TABLES is not defined, things will be faster if you can use
 * MACRO_CPATH.  Some cpps, however, cannot deal with the size of the
 * functions that have been macroized.
 */
/* WAC Can be defined under DJGPP,  even though it's DOS*/
/*#define VISION_TABLES */ /* use vision tables generated at compile time */
#ifndef VISION_TABLES
# ifndef NO_MACRO_CPATH
#  define MACRO_CPATH	/* use clear_path macros instead of functions */
# endif
#endif

/*
 * Section 4:  THE FUN STUFF!!!
 *
 * Conditional compilation of special options are controlled here.
 * If you define the following flags, you will add not only to the
 * complexity of the game but also to the size of the load module.
 */

/* dungeon features */
#define LIGHT_SRC_SPELL /* WAC Light sourced spells (wac@intergate.bc.ca)*/

/* dungeon levels */
/* #define REINCARNATION */     /* Special Rogue-like levels */

/* monsters & objects */
#define INVISIBLE_OBJECTS /* Not yet fully implemented */
#define UNPOLYPILE	/* WAC -- Items can unpolymorph */
#define STEED		/* The ability to ride monsters */

/* Roles */
/* #define ZOUTHERN */	/* KMH -- Zoutherner class and its animals */

/* I/O */
#define REDO		/* support for redoing last command - DGK */
#if !defined(MAC)
# define CLIPPING	/* allow smaller screens -- ERS */
#endif
#ifdef TTY_GRAPHICS
# define MENU_COLOR
#endif

#if defined(UNIX)
#define USE_REGEX_MATCH
/* if USE_REGEX_MATCH is defined, use regular expressions (GNU regex.h)
 * otherwise use pmatch() to match menu color lines.
 * pmatch() provides basic globbing: '*' and '?' wildcards.
 */
#endif

/* difficulty */
/* #define NOARTIFACTWISH */  /* No wishing for special artifacts -- swhite@cs.mun.ca */
/* #define NO_BONES */	/*Disables loading and saving bones levels*/

/* The following are best left disabled until their bugs are completely fixed */


/* User_sounds are sounds matches with messages.  The messages are defined
 * in the player's .nethackrc using lines of the form:
 *
 * SOUND=MESG <message-regex-pattern> <sound-filename> <volume>
 *
 * For example:
 *
 * SOUND=MESG "board beneath .....* squeaks" "squeak.au" 60
 *
 * By default, the filenames are relative to the nethack install directory,
 * but this can be set in the .nethackrc via:
 *
 * SOUNDDIR=<directory>
 */
/* #define USER_SOUNDS */   /* Allow user-defined regex mappings from messages to sounds */
                      /* Only supported on Qt with NAS - Network Audio System */


#ifdef REDO
# define DOAGAIN '\001'	/* ^A, the "redo" key used in cmd.c and getline.c */
#endif

#define BORG            // Fixed for non-DOS --ELR
/* #define KEEP_SAVE */       /* Keep savefiles after Restore (wac@intergate.bc.ca)*/
/* #define CHARON */	/* Charon's boat, enables Cerebus - not implemented */
#define SHOW_DMG        /* WAC made dmg reports optional (wac@intergate.bc.ca)*/
#define SHOW_WEIGHT     /* [max] added display of object weight when picking up */
                        /* and in inventory (madmax@fly.cc.fer.hr). */
                        /* Originally added by zaga. */
#define OTHER_SERVICES  /* shopkeeper services */
#define DUNGEON_GROWTH

/* #define SHOUT */ /* JRN -- shouting and petcommands - not implemented */

#define DISPLAY_LAYERS	/* Improved support for transparent tile sets - ALI */

/*
 * Section 5:  EXPERIMENTAL STUFF
 *
 * Conditional compilation of new or experimental options are controlled here.
 * Enable any of these at your own risk -- there are almost certainly
 * bugs left here.
 */

#define USER_DUNGEONCOLOR

/*#define GOLDOBJ */	/* Gold is kept on obj chains - Helge Hafting */

/* End of Section 5 */

#include "global.h"	/* Define everything else according to choices above */

#endif /* CONFIG_H */
