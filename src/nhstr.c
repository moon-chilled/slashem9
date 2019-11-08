#include "hack.h"
#include "config.h"
#include "unixconf.h"
#include "color.h"
#include "extern.h"
#include "nhstr.h"

nhstr *new_nhs(void) {
	return new(nhstr);
}

void del_nhs(nhstr *str) {
	free(str->str);
	free(str->colouration);
	free(str);
}

nhstr *nhscatznc(nhstr *str, char *cat, usize catlen, int colour) {
	str->str = realloc(str->str, (str->len + catlen) * sizeof(glyph_t));
	for (usize i = str->len; i < str->len + catlen; i++) {
		str->str[i] = cat[i - str->len];
	}

	str->colouration = realloc(str->colouration, (str->len + catlen) * sizeof(int));
	for (usize i = str->len; i < str->len + catlen; i++) {
		str->colouration[i] = colour;
	}

	str->len += catlen;
	return str;
}

nhstr *nhscatzc(nhstr *str, char *cat, int colour) {
	return nhscatznc(str, cat, strlen(cat), colour);
}

nhstr *nhscatzn(nhstr *str, char *cat, usize catlen) {
	return nhscatznc(str, cat, catlen, NO_COLOR);
}

nhstr *nhscatz(nhstr *str, char *cat) {
	return nhscatzn(str, cat, strlen(cat));
}

nhstr *nhscopy(nhstr *str) {
	nhstr *ret = new(nhstr);
	ret->str = new(glyph_t, str->len);
	memmove(ret->str, str->str, str->len * sizeof(glyph_t));

	ret->colouration = new(int, str->len);
	memmove(ret->colouration, str->colouration, str->len * sizeof(int));
	ret->len = str->len;

	return ret;
}

nhstr *nhscat(nhstr *str, nhstr *cat) {
	str->str = realloc(str->str, (str->len + cat->len) * sizeof(glyph_t));
	memcpy(str->str + str->len, cat->str, cat->len * sizeof(glyph_t));
	str->colouration = realloc(str->colouration, (str->len + cat->len) * sizeof(int));
	memcpy(str->colouration + str->len, cat->colouration, cat->len * sizeof(int));

	str->len += cat->len;
	return str;
}

// limited at the moment
// %s: nhstr
// %S: char*
// %c: glyph_t (unicode conversion)
// %l: long
// %i: int
// %u: uint
// %%: literal %
nhstr *nhscatfc_v(nhstr *str, int colour, char *cat, va_list the_args) {
	for (usize i = 0; cat[i]; i++) {
		if (cat[i] == '%') {
			i++;
			switch (cat[i]) {
				case 's':
					nhscat(str, va_arg(the_args, nhstr*));
					break;
				case 'S': {
					char *s = va_arg(the_args, char*);
					nhscatzc(str, s, colour);
					break;
				}
				case 'c':
					nhscatzc(str, utf8_tmpstr(va_arg(the_args, glyph_t)), colour);
					break;
				case 'l': {
					static char buf[BUFSZ];
					sprintf(buf, "%ld", va_arg(the_args, long));
					nhscatzc(str, buf, colour);
					break;
				}
				case 'i': {
					static char buf[BUFSZ];
					sprintf(buf, "%i", va_arg(the_args, int));
					nhscatzc(str, buf, colour);
					break;
				}
				case 'u': {
					static char buf[BUFSZ];
					sprintf(buf, "%u", va_arg(the_args, uint));
					nhscatzc(str, buf, colour);
					break;
				}
				case '%':
					nhscatzc(str, "%", colour);
					break;
				// bad format
				default:
					nhscatzc(str, "%", colour);
					nhscatzc(str, (char[]){cat[i], 0}, colour);
			}
		} else {
			nhscatzc(str, (char[]){cat[i], 0}, colour);
		}
	}

	return str;
}

nhstr *nhscatfc(nhstr *str, int colour, char *cat, ...) {
	VA_START(cat);
	nhstr *ret = nhscatfc_v(str, colour, cat, VA_ARGS);
	VA_END();

	return ret;
}

nhstr *nhscatf(nhstr *str, char *cat, ...) {
	VA_START(cat);
	nhstr *ret = nhscatfc_v(str, NO_COLOR, cat, VA_ARGS);
	VA_END();

	return ret;
}

// does utf-8 conversion
char *nhs2cstr_tmp(nhstr *str) {
	static char ret[BUFSZ];
	ret[0] = 0;

	for (usize i = 0; i < str->len; i++) {
		strcat(ret, utf8_tmpstr(str->str[i]));
	}

	return ret;
}

// doesn't do unicode conversion
extern char *nhs2cstr_trunc_tmp(nhstr *str) {
	static char ret[BUFSZ];

	usize i;
	for (i = 0; i < str->len; i++) {
		ret[i] = str->str[i];
	}
	ret[i+1] = 0;

	return ret;
}

nhstr *nhstrim(nhstr *str, usize maxlen) {
	if (maxlen < str->len) {
		str->len = maxlen;
		str->str = realloc(str->str, str->len * sizeof(glyph_t));
		str->colouration = realloc(str->str, str->len * sizeof(int));
	}

	return str;
}

nhstr *nhslice(nhstr *str, usize new_start) {
	if (new_start < str->len) {
		memmove(str->str, (str->str + new_start), (str->len - new_start) * sizeof(glyph_t));
		str->str = realloc(str->str, (str->len - new_start) * sizeof(glyph_t));

		memmove(str->colouration, (str->colouration + new_start), (str->len - new_start) * sizeof(int));
		str->colouration = realloc(str->colouration, (str->len - new_start) * sizeof(int));

		str->len -= new_start;
	}

	return str;
}

isize nhsindex(nhstr *str, glyph_t ch) {
	for (usize i = 0; i < str->len; i++) {
		if (str->str[i] == ch) {
			return i;
		}
	}

	return -1; // no match
}
