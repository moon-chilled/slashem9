/*	SCCS Id: @(#)config1.h	3.4	1999/12/05	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG1_H
#define CONFIG1_H

#ifdef __TURBOC__
# define __MSC		/* increase Borland C compatibility in libraries */
#endif

/*
 * Mac Stuff.
 */
#if defined(__SC__) || defined(__MRC__) /* MPW compilers, but not Metrowerks */
# define MAC
# define MAC_MPW
#endif

#ifdef THINK_C		/* Think C auto-defined symbol */
# define MAC
# define NEED_VARARGS
#endif

#ifdef __MWERKS__	/* defined by Metrowerks' Codewarrior compiler */
# ifndef __BEOS__	/* BeOS */
#  define MAC
# endif
# define NEED_VARARGS
# define USE_STDARG
#endif

#if defined(MAC) || defined(__BEOS__)
# define DLB
# undef UNIX
#endif

#ifdef __BEOS__
# define NEED_VARARGS
#endif

#ifdef __APPLE__        /* defined by GCC on Mac OS X */
# define OSX
#endif

/*
 * Amiga setup.
 */
#ifdef AZTEC_C	/* Manx auto-defines this */
#endif
#ifdef __SASC_60
# define NEARDATA __near /* put some data close */
#else
# ifdef _DCC
# define NEARDATA __near /* put some data close */
# else
# define NEARDATA
# endif
#endif

/*
 * Atari auto-detection
 */

#ifdef atarist
# undef UNIX
# ifndef TOS
# define TOS
# endif
#else
# ifdef __MINT__
#  undef UNIX
#  ifndef TOS
#  define TOS
#  endif
# endif
#endif

/*
 * Windows NT Autodetection
 *
 */
#ifdef _WIN32_WCE
# ifndef WIN32
#  define WIN32
# endif
#endif

#if defined(__CYGWIN__) && !defined(UNIX)
# define WIN32
#endif
#ifdef WIN32
# undef UNIX
# define NHSTDC
# define USE_STDARG
# define NEED_VARARGS

#define STRNCMPI
#define STRCMPI

#endif


#if defined(__linux__) && defined(__GNUC__) && !defined(_GNU_SOURCE)
/* ensure _GNU_SOURCE is defined before including any system headers */
# define _GNU_SOURCE
#endif

#ifdef vax
/* just in case someone thinks a DECstation is a vax. It's not, it's a mips */
# ifdef ULTRIX_PROTO
#  undef ULTRIX_PROTO
# endif
# ifdef ULTRIX_CC20
#  undef ULTRIX_CC20
# endif
#endif

#ifdef KR1ED		/* For compilers which cannot handle defined() */
#define defined(x) (-x-1 != -1)
/* Because:
 * #define FOO => FOO={} => defined( ) => (-1 != - - 1) => 1
 * #define FOO 1 or on command-line -DFOO
 *	=> defined(1) => (-1 != - 1 - 1) => 1
 * if FOO isn't defined, FOO=0. But some compilers default to 0 instead of 1
 * for -DFOO, oh well.
 *	=> defined(0) => (-1 != - 0 - 1) => 0
 *
 * But:
 * defined("") => (-1 != - "" - 1)
 *   [which is an unavoidable catastrophe.]
 */
#endif

#if defined(__OS2__) || defined(__EMX__)
# ifndef OS2
#  define OS2
# endif
# undef UNIX
#endif

#endif	/* CONFIG1_H */
