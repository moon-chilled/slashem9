/*	SCCS Id: @(#)alloc.c	3.4	1995/10/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include <stdlib.h>

#include "tradstdc.h"
#include "extern.h"

#ifdef WIZARD
char *fmt_ptr(const void*,char *);
#endif

void *alloc(size_t lth) {
	void *ptr = calloc(1, lth);

	if (!ptr) panic("Memory allocation failure; cannot get %zu bytes", lth);

	return ptr;
}


#ifdef WIZARD

# if defined(MICRO) || defined(WIN32)
/* we actually want to know which systems have an ANSI run-time library
 * to know which support the new %p format for printing pointers.
 * due to the presence of things like gcc, NHSTDC is not a good test.
 * so we assume microcomputers have all converted to ANSI and bigger
 * computers which may have older libraries give reasonable results with
 * the cast.
 */
#  define MONITOR_PTR_FMT
# endif

# ifdef MONITOR_PTR_FMT
#  define PTR_FMT "%p"
#  define PTR_TYP void*
# else
#  define PTR_FMT "%06lx"
#  define PTR_TYP unsigned long
# endif

/* format a pointer for display purposes; caller supplies the result buffer */
char * fmt_ptr(const void *ptr, char *buf) {
	sprintf(buf, PTR_FMT, (PTR_TYP)ptr);
	return buf;
}

#endif	/* WIZARD */

/*alloc.c*/
