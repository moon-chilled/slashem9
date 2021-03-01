#include "s7.h"
#include "hack.h"
#include "dlb.h"

static bool s7_is_stringish(s7_pointer p) { return s7_is_string(p) || s7_is_symbol(p); }
static const char *s7_stringish(s7_pointer p) { return s7_is_symbol(p) ? s7_symbol_name(p) : s7_string(p); }

s7_scheme *s7;
s7_pointer options_env;

static s7_pointer set_boolopt(s7_scheme *s7, s7_pointer args) {
#define H_set_boolopt "(s9-option-set-boolopt option-name boolean?) sets option-name to boolean (if provided) or #t (if not).  The name can be a symbol or a string"
#define Q_set_boolopt s7_make_signature(s7, 3, s7_make_symbol(s7, "null?"), s7_make_signature(s7, 2, s7_make_symbol(s7, "string?"), s7_make_symbol(s7, "symbol?")), s7_make_symbol(s7, "boolean?"))
	const char *oname = NULL;
	bool value = true;

	enforce(s7_is_pair(args));

	s7_pointer car = s7_car(args), cdr = s7_cdr(args);
	enforce(s7_is_stringish(car));
	oname = s7_stringish(car);

	if (!s7_is_null(s7, cdr)) {
		enforce(s7_is_pair(cdr));
		enforce(s7_is_null(s7, s7_cdr(cdr)));

		s7_pointer cadr = s7_cadr(args);
		enforce(s7_is_boolean(cadr));

		value = s7_boolean(s7, cadr);
	}

	if (*oname == '!') {
		value = !value;
		oname++;
	}

	for (usize i = 0; boolopt[i].name; i++) {
		if (!strcmp(oname, boolopt[i].name)) {
			assign_boolopt(boolopt + i, value, s7_curlet(s7) == options_env);
			return s7_nil(s7);
		}
	}

	pline("No such boolean option '%s'.", oname);
	return s7_nil(s7);
}

static nhstyle s7_to_nhstyle(s7_scheme *s7, s7_pointer p) {
	nhstyle s = nhstyle_default();
	struct name_value {
		char *name;
		int value;
	};

	const static struct name_value clr[] = {
		{"black", CLR_BLACK},
		{"red", CLR_RED},
		{"green", CLR_GREEN},
		{"brown", CLR_BROWN},
		{"blue", CLR_BLUE},
		{"magenta", CLR_MAGENTA},
		{"cyan", CLR_CYAN},
		{"gray", CLR_GRAY},
		{"orange", CLR_ORANGE},
		{"lightgreen", CLR_BRIGHT_GREEN},
		{"yellow", CLR_YELLOW},
		{"lightblue", CLR_BRIGHT_BLUE},
		{"lightmagenta", CLR_BRIGHT_MAGENTA},
		{"lightcyan", CLR_BRIGHT_CYAN},
		{"white", CLR_WHITE},
	};

	const static struct name_value attr[] = {
		{"bold", ATR_BOLD},
		{"blink", ATR_BLINK},
		{"italic", ATR_ITALIC},
		{"inverse", ATR_INVERSE},
		{"underline", ATR_UNDERLINE},
	};

	if (!s7_is_list(s7, p)) p = s7_list(s7, 1, p);

	for (s7_pointer o = s7_car(p); !s7_is_null(s7, p); o = s7_car(p), p = s7_cdr(p)) {
		const char *n = s7_stringish(o);
		for (usize i = 0; i < SIZE(clr); i++) {
			if (!strcmp(n, clr[i].name)) {
				s.fg = clr[i].value;
				goto e;
			}
		}

		for (usize i = 0; i < SIZE(attr); i++) {
			if (!strcmp(n, attr[i].name)) {
				s.attr |= attr[i].value;
			}
		}
e:;
	}

	return s;
}

static s7_pointer add_menucolor(s7_scheme *s7, s7_pointer args) {
#define H_add_menucolor "(s9-option-add-menucolor pattern style) creates a menu coloring rule associating ‘pattern’ with ‘style’"
#define Q_add_menucolor s7_make_signature(s7, 3, s7_make_symbol(s7, "null?"), s7_make_symbol(s7, "string?"), s7_make_symbol(s7, "s9-style?"))
	const char *k = s7_string(s7_car(args));
	nhstyle s = s7_to_nhstyle(s7, s7_cadr(args));
	create_menu_coloring(k, s);
	return s7_nil(s7);
}

void s9s7_init(void) {
	s7 = s7_init();
	s7_define_variable(s7, "s9-env", s7_make_symbol(s7, "global"));

	{
		options_env = s7_inlet(s7, s7_list(s7, 2, s7_make_symbol(s7, "s9-env"), s7_make_symbol(s7, "options")));
		s7_pointer p;
		p = s7_make_typed_function(s7, "s9-option-set-boolopt", set_boolopt, 1, 2, false, H_set_boolopt, Q_set_boolopt);
		enforce(p);
		s7_define(s7, options_env, s7_make_symbol(s7, "s9-option-set-boolopt"), p);
		p = s7_make_typed_function(s7, "s9-option-add-menucolor", add_menucolor, 2, 2, false, H_add_menucolor, Q_add_menucolor);
		enforce(p);
		s7_define(s7, options_env, s7_make_symbol(s7, "s9-option-add-menucolor"), p);
	}

	{
		dlb *d = dlb_fopen("scm/options.scm", "r");
		enforce(d);
		dlb_fseek(d, 0, SEEK_END);
		long pos = dlb_ftell(d);
		dlb_fseek(d, 0, SEEK_SET);
		char *s = new(char, pos);
		dlb_fread(s, 1, pos, d);
		dlb_fclose(d);
		s7_load_c_string_with_environment(s7, s, pos, options_env);
	}
}

void s9s7_load_options(const char *fname) {
	s7_load_with_environment(s7, fname, options_env);
}

/*s9s7.c*/
