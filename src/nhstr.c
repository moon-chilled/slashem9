#include "hack.h"
#include "config.h"
#include "unixconf.h"
#include "color.h"
#include "extern.h"
#include "nhstr.h"

void del_nhs(nhstr *str) {
	str->str = NULL;
	str->style = NULL;
	str->len = 0;
}

nhstr nhscatznc(const nhstr str, const char *cat, usize catlen, nhstyle style) {
	glyph_t *g = new(glyph_t, str.len + catlen);
	memcpy(g, str.str, str.len * sizeof(glyph_t));

	for (usize i = 0; i < catlen; i++) {
		g[i + str.len] = cat[i];
	}

	nhstyle *s = new(nhstyle, str.len + catlen);
	memcpy(s, str.style, str.len * sizeof(nhstyle));

	for (usize i = 0; i < catlen; i++) {
		s[i + str.len] = style;
	}

	return (nhstr){.str=g, .style=s, .len=str.len+catlen};
}

nhstr nhscatzc(const nhstr str, const char *cat, nhstyle style) {
	return nhscatznc(str, cat, cat ? strlen(cat) : 0, style);
}

nhstr nhscatzn(const nhstr str, const char *cat, usize catlen) {
	return nhscatznc(str, cat, catlen, nhstyle_default());
}

nhstr nhscatz(const nhstr str, const char *cat) {
	return nhscatzn(str, cat, cat ? strlen(cat) : 0);
}

nhstr nhsdup(const nhstr str) {
	nhstr ret;
	ret.str = memcpy(new(glyph_t, str.len), str.str, str.len * sizeof(glyph_t));
	ret.style = memcpy(new(nhstyle, str.len), str.style, str.len * sizeof(nhstyle));
	ret.len = str.len;

	return ret;
}

nhstr nhsdupzn(const char *str, usize len) {
	return nhscatzn(new_nhs(), str, len);
}

nhstr nhsdupzc(const char *str, nhstyle style) {
	return nhscatzc(new_nhs(), str, style);
}

nhstr nhsdupz(const char *str) {
	return nhscatz(new_nhs(), str);
}

nhstr nhscat(const nhstr str, const nhstr cat) {
	glyph_t *g = new(glyph_t, str.len + cat.len);
	memcpy(g, str.str, str.len * sizeof(glyph_t));
	memcpy(g + str.len, cat.str, cat.len * sizeof(glyph_t));

	nhstyle *s = new(nhstyle, str.len + cat.len);
	memcpy(s, str.style, str.len * sizeof(nhstyle));
	memcpy(s + str.len, cat.style, cat.len * sizeof(nhstyle));

	return (nhstr){.str=g, .style=s, .len=str.len+cat.len};
}

nhstr nhsins(const nhstr str, usize i, nhstr cat) {
	i = min(i, str.len);
	glyph_t *g = new(glyph_t, str.len + cat.len);
	memcpy(g, str.str, i * sizeof(glyph_t));
	memcpy(g + i, cat.str, cat.len * sizeof(glyph_t));
	memcpy(g + i + cat.len, str.str + i, (str.len - i) * sizeof(glyph_t));;

	nhstyle *s = new(nhstyle, str.len + cat.len);
	memcpy(s, str.style, i * sizeof(nhstyle));
	memcpy(s + i, cat.style, cat.len * sizeof(nhstyle));
	memcpy(s + i + cat.len, str.style + i, (str.len - i) * sizeof(nhstyle));;

	return (nhstr){.str=g, .style=s, .len=str.len+cat.len};
}

nhstr nhsinscg(const nhstr str, usize i, glyph_t l, nhstyle style) {
	return nhsins(str, i, (nhstr){.str=&l, .style=&style, .len=1});
}
nhstr nhsinsg(const nhstr str, usize i, glyph_t l) {
	return nhsinscg(str, i, l, nhstyle_default());
}

// limited at the moment
// %s: nhstr
// %S: char*
// %c: glyph_t
// %l: long
// %i: int
// %u: uint
// %%: literal %
nhstr nhsfmtc_v(nhstyle style, const char *cat, va_list the_args) {
	nhstr ret = new_nhs();

	for (usize i = 0; cat[i]; i++) {
		if (cat[i] == '%') {
			i++;

			bool left_justify = false; // currently only supported for s
			if (cat[i] == '-') { left_justify = true; i++; }

			int num1 = -1, num2 = -1; // currently only supported for %l,u,i
			if (digit(cat[i])) { num1 = 0; while (digit(cat[i])) { num1 *= 10; num1 += cat[i] - '0'; i++; } }
			if (cat[i] == '.' && digit(cat[i+1])) {
				i++;
				num2 = 0;
				while ('0' <= cat[i] && cat[i] <= '9') { num2 *= 10; num2 = cat[i] - '0'; i++; }
			}

			switch (cat[i]) {
				case 's': {
					nhstr s = va_arg(the_args, nhstr);
					ret = nhscat(ret, s);
					if (left_justify && num1 > (int)s.len) {
						for (int i = 0; i < ((int)s.len) - num1; i++) ret = nhscatz(ret, " ");
					}

					break;
				}
				case 'S': {
					char *s = va_arg(the_args, char*);
					ret = nhscatzc(ret, s, style);
					break;
				}
				case 'c':
					ret = nhscatzc(ret, utf8_tmpstr(va_arg(the_args, glyph_t)), style);
					break;
				case 'l': {
					static char buf[BUFSZ];
					if (num1 != -1) {
						if (num2 != -1) sprintf(buf, "%*.*ld", num1, num2, va_arg(the_args, long));
						else sprintf(buf, "%*ld", num1, va_arg(the_args, long));
					} else {
						sprintf(buf, "%ld", va_arg(the_args, long));
					}

					ret = nhscatzc(ret, buf, style);
					break;
				}
				case 'i': {
					static char buf[BUFSZ];
					if (num1 != -1) {
						if (num2 != -1) sprintf(buf, "%*.*i", num1, num2, va_arg(the_args, int));
						else sprintf(buf, "%*i", num1, va_arg(the_args, int));
					} else {
						sprintf(buf, "%i", va_arg(the_args, int));
					}

					ret = nhscatzc(ret, buf, style);
					break;
				}
				case 'u': {
					static char buf[BUFSZ];
					if (num1 != -1) {
						if (num2 != -1) sprintf(buf, "%*.*u", num1, num2, va_arg(the_args, uint));
						else sprintf(buf, "%*u", num1, va_arg(the_args, uint));
					} else {
						sprintf(buf, "%u", va_arg(the_args, uint));
					}

					ret = nhscatzc(ret, buf, style);
					break;
				}
				case '%':
					ret = nhscatzc(ret, "%", style);
					break;
				// bad format
				default:
					impossible("bad string format '%c'", cat[i]);
					ret = nhscatzc(ret, "%", style);
					ret = nhscatznc(ret, cat + i, 1, style);
			}
		} else {
			ret = nhscatznc(ret, cat + i, 1, style);
		}
	}

	return ret;
}

nhstr nhscatfc(const nhstr str, nhstyle style, const char *cat, ...) {
	VA_START(cat);
	nhstr t = nhsfmtc_v(style, cat, VA_ARGS);
	VA_END();

	return nhscat(str, t);
}

nhstr nhscatf(const nhstr str, const char *cat, ...) {
	VA_START(cat);
	nhstr t = nhsfmtc_v(nhstyle_default(), cat, VA_ARGS);
	VA_END();

	return nhscat(str, t);
}

nhstr nhsfmt(const char *fmt, ...) {
	VA_START(fmt);
	nhstr ret = nhsfmtc_v(nhstyle_default(), fmt, VA_ARGS);
	VA_END();
	return ret;
}

bool nhseq(const nhstr x, const nhstr y) {
	return x.len == y.len && !memcmp(x.str, y.str, x.len * sizeof(glyph_t)) && !memcmp(x.style, y.style, x.len * sizeof(nhstyle));
}

usize utf8len(const nhstr str) {
	usize ret = 0;
	for (usize i = 0; i < str.len; i++) {
		ret += strlen(utf8_tmpstr(str.str[i]));
	}

	return ret;
}

// does utf-8 conversion
char *nhs2cstr(const nhstr str) {
	usize l = utf8len(str);
	char *ret = alloc(l+1);
	ret[0] = 0;

	for (usize i = 0; i < str.len; i++) {
		strcat(ret, utf8_tmpstr(str.str[i]));
	}

	return ret;
}

// doesn't do unicode conversion
char *nhs2cstr_trunc(const nhstr str) {
	char *ret = alloc(str.len+1);
	ret[str.len] = 0;

	for (usize i = 0; i < str.len; i++) {
		ret[i] = str.str[i];
	}

	return ret;
}

nhstr nhstrim(nhstr str, usize maxlen) {
	nhstr ret = str;
	ret.len = min(ret.len, maxlen);
	return ret;
}

nhstr nhslice(nhstr str, usize new_start) {
	if (new_start >= str.len) return new_nhs();
	return (nhstr){.str = str.str + new_start, .style = str.style + new_start, .len = str.len - new_start};
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
		bwrite(fd, str.style, str.len * sizeof(nhstyle));
	}
}

nhstr restore_nhs(int fd) {
	nhstr ret;

	mread(fd, &ret.len, sizeof(usize));

	glyph_t *g = new(glyph_t, ret.len);
	nhstyle *s = new(nhstyle, ret.len);

	if (ret.len) {
		mread(fd, g, ret.len * sizeof(glyph_t));
		mread(fd, s, ret.len * sizeof(nhstyle));
	}

	ret.str = g;
	ret.style = s;

	return ret;
}
