#ifndef NHSTR_H
#define NHSTR_H

typedef struct {
	const glyph_t *str;
	const nhstyle *style;
	usize len;
} nhstr;

///NAMING CONVENTIONS:

/* everything should be prefixed with 'nhs' (N)et(H)ack (S)tring
 * things that deal with (z)ero-terminated strings should be z-suffixed
 * 	Of those, the ones that explicitly take a (n)umber of chars to consider
 * 	should be n-suffixed
 * things that deal with (f)ormatted strings should be f-suffixed
 * things that deal with (c)oloured strings should be c-suffixed
 */

///If a function has multiple suffixes, they should go in that order.
/* However, a function which does formatting is assumed to deal with zero-
 * terminated strings, and the z should be left out; if it does formatting with
 * nhstr, then it should be suffixed with n instead of z.
 */

#define nhstyle_default() ((nhstyle){.fg=NO_COLOR, .bg=NO_COLOR})
#define new_nhs() ((nhstr){0})
void del_nhs(nhstr *str);
nhstr nhscatznc(const nhstr str, const char *cat, usize catlen, nhstyle style);
nhstr nhscatzc(const nhstr str, const char *cat, nhstyle style);
nhstr nhscatzn(const nhstr str, const char *cat, usize catlen);
nhstr nhscatz(const nhstr str, const char *cat);
nhstr nhsdup(const nhstr str); // !no in-place modification!
nhstr nhsdupz(const char *str);
nhstr nhscat(const nhstr str, const nhstr cat);
//nhstr nhsfmtc_v(nhstyle style, char *cat, va_list the_args);
nhstr nhscatfc(const nhstr str, nhstyle style, const char *cat, ...);
nhstr nhscatf(const nhstr str, const char *cat, ...);
nhstr nhsfmt(const char *fmt, ...);

bool nhseq(const nhstr x, const nhstr y);

char *nhs2cstr(const nhstr str);
char *nhs2cstr_trunc(const nhstr str);  // doesn't do unicode conversion
nhstr nhstrim(const nhstr str, usize maxlen);
nhstr nhslice(const nhstr str, usize new_start);

isize nhsindex(const nhstr str, glyph_t ch);

void save_nhs(int fd, const nhstr str);
nhstr restore_nhs(int fd);

#endif
