#ifndef NHSTR_H
#define NHSTR_H

typedef struct {
	glyph_t *str;
	nhstyle *style;
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

// All function do in-place modification of the string they're passed; they
// return the same string for convenience.  Exceptions are labled with
// exclamation marks.

#define nhstyle_default() ((nhstyle){.fg=NO_COLOR, .bg=NO_COLOR})
#define new_nhs() ((nhstr){0})
void del_nhs(nhstr *str);
nhstr *nhscatznc(nhstr *str, const char *cat, usize catlen, nhstyle style);
nhstr *nhscatzc(nhstr *str, const char *cat, nhstyle style);
nhstr *nhscatzn(nhstr *str, const char *cat, usize catlen);
nhstr *nhscatz(nhstr *str, const char *cat);
nhstr nhsdup(const nhstr str); // !no in-place modification!
nhstr nhsdupz(const char *str);
nhstr *nhscat(nhstr *str, const nhstr cat);
//nhstr *nhscatfc_v(nhstr *str, nhstyle style, char *cat, va_list the_args);
nhstr *nhscatfc(nhstr *str, nhstyle style, const char *cat, ...);
nhstr *nhscatf(nhstr *str, const char *cat, ...);

nhstr *nhsmove(nhstr *target, nhstr *src); // !destroys src!
nhstr *nhscopyf(nhstr *target, const char *fmt, ...);
nhstr *nhscopyz(nhstr *str, const char *replacement);
nhstr *nhsreplace(nhstr *str, const nhstr replacement);
nhstr nhsfmt(const char *fmt, ...);

// when you need a string to have a 'short lifetime', but don't want to leak it
// to free the backing store size, do something like:
// for (int i = 0; i < NHSTMP_BACKING_STORE_SIZE; i++) nhstmp(new_nhs());
#define NHSTMP_BACKING_STORE_SIZE 16
nhstr *nhstmp(nhstr *str); // !destroys str!

// for library functions that return a direct nhstr (not a pointer)
// so you don't need an intermediate variable
nhstr *nhstmpt(nhstr str);


char *nhs2cstr_tmp(const nhstr str);
char *nhs2cstr_trunc_tmp(const nhstr str);  // doesn't do unicode conversion
char *nhs2cstr_tmp_destroy(nhstr *str); // !destroys src!
nhstr *nhstrim(nhstr *str, usize maxlen);
nhstr *nhslice(nhstr *str, usize new_start);

isize nhsindex(const nhstr str, glyph_t ch);

void save_nhs(int fd, const nhstr str);
nhstr restore_nhs(int fd);

#endif
