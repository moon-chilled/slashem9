/*	SCCS Id: @(#)config1.h	3.4	1999/12/05	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG1_H
#define CONFIG1_H

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
#if defined(__CYGWIN__) && !defined(UNIX)
# define WIN32
#endif
#ifdef _WIN32
# define WIN32
#endif

#ifdef WIN32
# undef UNIX
# define NHSTDC
# define STRNCMPI
#endif


#if defined(__linux__) && defined(__GNUC__) && !defined(_GNU_SOURCE)
/* ensure _GNU_SOURCE is defined before including any system headers */
# define _GNU_SOURCE
#endif

#endif	/* CONFIG1_H */
