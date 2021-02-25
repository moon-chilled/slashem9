/*	SCCS Id: @(#)panic.c	3.4	1994/03/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *	This code was adapted from the code in end.c to run in a standalone
 *	mode for the makedefs / drg code.
 */

#include "config.h"

boolean panicking;

void panic(char *str, ...) {
	VA_START(str);
	if (panicking++)
		abort(); /* avoid loops - this should never happen*/

	fputs(" ERROR:  ", stderr);
	vfprintf(stderr, str, VA_ARGS);
	fflush(stderr);
#ifdef UNIX
	abort(); /* generate core dump */
#endif
	VA_END();
	exit(EXIT_FAILURE); /* redundant */
}

#ifdef ALLOCA_HACK
/*
 * In case bison-generated foo_yacc.c tries to use alloca(); if we don't
 * have it then just use malloc() instead.  This may not work on some
 * systems, but they should either use yacc or get a real alloca routine.
 */
long *alloca(usize l) {
	return alloc(l);
}
#endif

/*panic.c*/
