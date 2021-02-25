/*	SCCS Id: @(#)alloc.c	3.4	1995/10/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include <stdlib.h>
#include <gc/gc.h>

#include "config.h"

extern void panic(const char *, ...);

void *alloc(usize lth) {
	void *ptr = GC_malloc(lth);

	if (!ptr) {
		panic("Memory allocation failure; cannot get %zu bytes", lth);
	}

	return memset(ptr, 0, lth);
}

void *nhrealloc(void *ptr, usize sz) {
	void *ret = GC_realloc(ptr, sz);
	if (!ret) panic("Memory allocation failure; cannot get %zu bytes", sz);
	return ret;
}

/* format a pointer for display purposes; caller supplies the result buffer */
char *fmt_ptr(const void *ptr, char *buf) {
	sprintf(buf, "%p", ptr);
	return buf;
}

/*alloc.c*/
