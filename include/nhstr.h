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

extern nhstr *new_nhs(void);
extern void del_nhs(nhstr *str);
extern nhstr *nhscatznc(nhstr *str, char *cat, usize catlen, int colour);
extern nhstr *nhscatzc(nhstr *str, char *cat, int colour);
extern nhstr *nhscatzn(nhstr *str, char *cat, usize catlen);
extern nhstr *nhscatz(nhstr *str, char *cat);
extern nhstr *nhscopy(nhstr *str);
extern nhstr *nhscat(nhstr *str, nhstr *cat);
//extern nhstr *nhscatfc_v(nhstr *str, int colour, char *cat, va_list the_args);
extern nhstr *nhscatfc(nhstr *str, int colour, char *cat, ...);
extern nhstr *nhscatf(nhstr *str, char *cat, ...);
char *nhs2cstr_tmp(nhstr *str/*, usize *olen*/);


#endif
