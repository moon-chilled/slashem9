/*	SCCS Id: @(#)alloc.c	3.4	1995/10/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include <stdlib.h>

#include "config.h"

extern void panic(const char *, ...);

void *alloc(usize lth) {
	void *ptr = calloc(1, lth);

	if (!ptr) {
		panic("Memory allocation failure; cannot get %zu bytes", lth);
	}

	return ptr;
}

void nhfree(const void *ptr) {
	free((void *)ptr);
}

/* format a pointer for display purposes; caller supplies the result buffer */
char *fmt_ptr(const void *ptr, char *buf) {
	sprintf(buf, "%p", ptr);
	return buf;
}

/*alloc.c*/
