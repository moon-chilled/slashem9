#ifndef NHSTR_H
#define NHSTR_H

typedef struct {
	glyph_t *str;
	int *colouration;
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
// return the same string for convenience.  Only exception is nhscopy.

nhstr *new_nhs(void);
void del_nhs(nhstr *str);
nhstr *nhscatznc(nhstr *str, char *cat, usize catlen, int colour);
nhstr *nhscatzc(nhstr *str, char *cat, int colour);
nhstr *nhscatzn(nhstr *str, char *cat, usize catlen);
nhstr *nhscatz(nhstr *str, char *cat);
nhstr *nhscopy(nhstr *str);
nhstr *nhscat(nhstr *str, nhstr *cat);
//nhstr *nhscatfc_v(nhstr *str, int colour, char *cat, va_list the_args);
nhstr *nhscatfc(nhstr *str, int colour, char *cat, ...);
nhstr *nhscatf(nhstr *str, char *cat, ...);

char *nhs2cstr_tmp(nhstr *str);
char *nhs2cstr_trunc_tmp(nhstr *str);  // doesn't do unicode conversion
char *nhs2cstr_tmp_destroy(nhstr *str); //
nhstr *nhstrim(nhstr *str, usize maxlen);
nhstr *nhslice(nhstr *str, usize new_start);

isize nhsindex(nhstr *str, glyph_t ch);

#endif
