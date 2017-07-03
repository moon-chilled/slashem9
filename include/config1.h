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
# define MAC
# define NEED_VARARGS
# define USE_STDARG
#endif

#ifdef MAC
# define DLB
# undef UNIX
#endif

#ifdef __APPLE__        /* defined by GCC on Mac OS X */
# define OSX
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

#endif	/* CONFIG1_H */
