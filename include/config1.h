/*	SCCS Id: @(#)config1.h	3.4	1999/12/05	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef CONFIG1_H
#define CONFIG1_H

#ifdef MAC
#define DLB
#undef UNIX
#endif

#ifdef __APPLE__ /* defined by GCC on Mac OS X */
#define OSX
#endif

/*
 * Windows NT Autodetection
 *
 */
#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#ifdef WIN32
#undef UNIX
#define STRNCMPI
#endif

#if __STDC_VERSION__ >= 201112L
#include <stdnoreturn.h>
#else
#define noreturn
#endif

#endif /* CONFIG1_H */
