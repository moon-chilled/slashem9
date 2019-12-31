#include "hack.h"
#include "config.h"
#include "unixconf.h"
#include "color.h"
#include "extern.h"
#include "nhstr.h"

void del_nhs(nhstr *str) {
	free(str->str);
	free(str->colouration);

	str->str = NULL;
	str->colouration = NULL;
	str->len = 0;
}

nhstr *nhscatznc(nhstr *str, const char *cat, usize catlen, int colour) {
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

nhstr *nhscatzc(nhstr *str, const char *cat, int colour) {
	return nhscatznc(str, cat, cat ? strlen(cat) : 0, colour);
}

nhstr *nhscatzn(nhstr *str, const char *cat, usize catlen) {
	return nhscatznc(str, cat, catlen, NO_COLOR);
}

nhstr *nhscatz(nhstr *str, const char *cat) {
	return nhscatzn(str, cat, cat ? strlen(cat) : 0);
}

nhstr nhsdup(const nhstr str) {
	nhstr ret;
	ret.str = new(glyph_t, str.len);
	memcpy(ret.str, str.str, str.len * sizeof(glyph_t));

	ret.colouration = new(int, str.len);
	memcpy(ret.colouration, str.colouration, str.len * sizeof(int));
	ret.len = str.len;

	return ret;
}

nhstr nhsdupz(const char *str) {
	nhstr ret = {0};
	return *nhscatz(&ret, str);
}

nhstr *nhscat(nhstr *str, const nhstr cat) {
	str->str = realloc(str->str, (str->len + cat.len) * sizeof(glyph_t));
	memmove(str->str + str->len, cat.str, cat.len * sizeof(glyph_t));
	str->colouration = realloc(str->colouration, (str->len + cat.len) * sizeof(int));
	memmove(str->colouration + str->len, cat.colouration, cat.len * sizeof(int));

	str->len += cat.len;
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
nhstr *nhscatfc_v(nhstr *str, int colour, const char *cat, va_list the_args) {
	for (usize i = 0; cat[i]; i++) {
		if (cat[i] == '%') {
			i++;

			int num1 = -1, num2 = -1; // currently only supported for %l
			if ('0' <= cat[i] && cat[i] <= '9') { num1 = cat[i] - '0'; i++; }
			if ('0' <= cat[i] && cat[i] <= '9') { num2 = cat[i] - '0'; i++; }

			switch (cat[i]) {
				case 's':
					nhscat(str, va_arg(the_args, nhstr));
					break;
				case 'S': {
					nhscatzc(str, va_arg(the_args, char*), colour);
					break;
				}
				case 'c':
					nhscatzc(str, utf8_tmpstr(va_arg(the_args, glyph_t)), colour);
					break;
				case 'l': {
					static char buf[BUFSZ];
					if (num1 != -1) {
						if (num2 != -1) sprintf(buf, "%*.*ld", num1, num2, va_arg(the_args, long));
						else sprintf(buf, "%*ld", num1, va_arg(the_args, long));
					} else {
						sprintf(buf, "%ld", va_arg(the_args, long));
					}

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
					nhscatznc(str, cat + i, 1, colour);
			}
		} else {
			nhscatznc(str, cat + i, 1, colour);
		}
	}

	return str;
}

nhstr *nhscatfc(nhstr *str, int colour, const char *cat, ...) {
	VA_START(cat);
	nhstr *ret = nhscatfc_v(str, colour, cat, VA_ARGS);
	VA_END();

	return ret;
}

nhstr *nhscatf(nhstr *str, const char *cat, ...) {
	VA_START(cat);
	nhstr *ret = nhscatfc_v(str, NO_COLOR, cat, VA_ARGS);
	VA_END();

	return ret;
}

nhstr *nhsmove(nhstr *target, nhstr *src) {
	if (!memcmp(src, target, sizeof(nhstr))) {
		return target;
	}

	free(target->str);
	target->str = src->str;
	src->str = NULL;

	free(target->colouration);
	target->colouration = src->colouration;
	src->colouration = NULL;

	target->len = src->len;
	src->len = 0;

	return target;
}

nhstr *nhscopyf(nhstr *target, const char *fmt, ...) {
	nhstr tmp = {0};
	VA_START(fmt);
	nhsmove(target, nhscatfc_v(&tmp, NO_COLOR, fmt, VA_ARGS));
	VA_END();
	return target;
}

nhstr *nhscopyz(nhstr *str, const char *replacement) {
	nhstr tmp = {0};
	return nhsmove(str, nhscatz(&tmp, replacement));
}

nhstr *nhsreplace(nhstr *str, const nhstr replacement) {
	nhstr tmp = nhsdup(replacement);
	return nhsmove(str, &tmp);
}

nhstr nhsfmt(const char *fmt, ...) {
	nhstr tmp = {0};
	VA_START(fmt);
	nhscatfc_v(&tmp, NO_COLOR, fmt, VA_ARGS);
	VA_END();
	return tmp;
}

// does utf-8 conversion
char *nhs2cstr_tmp(const nhstr str) {
	static char ret[BUFSZ];
	ret[0] = 0;

	for (usize i = 0; i < str.len; i++) {
		strcat(ret, utf8_tmpstr(str.str[i]));
	}

	return ret;
}

// doesn't do unicode conversion
char *nhs2cstr_trunc_tmp(const nhstr str) {
	static char ret[BUFSZ];

	usize i;
	for (i = 0; i < str.len; i++) {
		ret[i] = str.str[i];
	}
	ret[i + 1] = 0;

	return ret;
}

char *nhs2cstr_tmp_destroy(nhstr *str) {
	char *ret = nhs2cstr_tmp(*str);
	del_nhs(str);
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

isize nhsindex(const nhstr str, glyph_t ch) {
	for (usize i = 0; i < str.len; i++) {
		if (str.str[i] == ch) {
			return i;
		}
	}

	return -1;  // no match
}

void save_nhs(int fd, const nhstr str) {
	usize len = str.len; // silence warning
	bwrite(fd, &len, sizeof(usize));

	if (len) {
		bwrite(fd, str.str, str.len * sizeof(glyph_t));
		bwrite(fd, str.colouration, str.len * sizeof(int));
	}
}

nhstr restore_nhs(int fd) {
	nhstr ret;

	mread(fd, &ret.len, sizeof(usize));

	// note: intentional alloc(0)
	ret.str = new(glyph_t, ret.len);
	ret.colouration = new(int, ret.len);

	if (ret.len) {
		mread(fd, ret.str, ret.len * sizeof(glyph_t));
		mread(fd, ret.colouration, ret.len * sizeof(int));
	}

	return ret;
}
