/* Copyright (c) Patric Mueller.			*/
/* NetHack may be freely redistributed.  See license for details. */

#include <stdio.h>
#include "hack.h"


void unicode_to_utf8(glyph_t c, char buf[5]) {
	memset(buf, 0, 5);
	if (c < 0x80) {
		buf[0] = c;
	} else if(c < 0x800) {
		buf[0] = 0xC0 | (c>>6);
		buf[1] = 0x80 | (c & 0x3F);
	} else if (c < 0x10000) {
		buf[0] = 0xE0 | (c>>12);
		buf[1] = 0x80 | (c>>6 & 0x3F);
		buf[2] = 0x80 | (c & 0x3F);
	} else if (c < 0x200000) {
		buf[0] = 0xF0 | (c>>18);
		buf[1] = 0x80 | (c>>12 & 0x3F);
		buf[2] = 0x80 | (c>>6 & 0x3F);
		buf[3] = 0x80 | (c & 0x3F);
	}
}

char *utf8_str(glyph_t c) {
	static char buf[5];
	unicode_to_utf8(c, buf);
	return buf;
}

void pututf8char(glyph_t c) {
	if (c < 0x80) {
		putchar(c);
	} else if(c < 0x800) {
		putchar(0xC0 | (c>>6));
		putchar(0x80 | (c & 0x3F));
	} else if (c < 0x10000) {
		putchar(0xE0 | (c>>12));
		putchar(0x80 | (c>>6 & 0x3F));
		putchar(0x80 | (c & 0x3F));
	} else if (c < 0x200000) {
		putchar(0xF0 | (c>>18));
		putchar(0x80 | (c>>12 & 0x3F));
		putchar(0x80 | (c>>6 & 0x3F));
		putchar(0x80 | (c & 0x3F));
	}
}
/*unicode.c*/
