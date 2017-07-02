/*	SCCS Id: @(#)alloc.c	3.4	1995/10/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* to get the malloc() prototype from system.h */
#define ALLOC_C		/* comment line for pre-compiled headers */
/* since this file is also used in auxiliary programs, don't include all the
 * function declarations for all of nethack
 */
#define EXTERN_H	/* comment line for pre-compiled headers */
#include "config.h"

#ifdef WIZARD
char *fmt_ptr(const genericptr,char *);
#endif

long *alloc(unsigned int);
extern void panic(const char *,...) PRINTF_F(1,2);


long *alloc (register unsigned int lth) {
	register genericptr_t ptr;

	ptr = malloc(lth);

	if (!ptr) panic("Memory allocation failure; cannot get %u bytes", lth);

	return((long *) ptr);
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
#  define PTR_TYP genericptr_t
# else
#  define PTR_FMT "%06lx"
#  define PTR_TYP unsigned long
# endif

/* format a pointer for display purposes; caller supplies the result buffer */
char * fmt_ptr(const genericptr ptr, char *buf) {
	sprintf(buf, PTR_FMT, (PTR_TYP)ptr);
	return buf;
}

#endif	/* WIZARD */

/*alloc.c*/
