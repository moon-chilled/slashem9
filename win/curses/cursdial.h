#ifndef CURSDIAL_H
#define CURSDIAL_H

/* Global declarations */

void curses_line_input_dialog(const char *prompt, char *answer, int buffer, bool (*exit_early)(char *answer));
int curses_character_input_dialog(const char *prompt, const char *choices,
				  char def);
int curses_ext_cmd(void);
void curses_create_nhmenu(winid wid);
void curses_add_nhmenu_item(winid wid, int glyph, const anything *identifier,
			    char accelerator, char group_accel, int attr,
			    const char *str, bool presel);
bool get_menu_coloring(char *, int *, attr_t *);
void curses_finalize_nhmenu(winid wid, const char *prompt);
int curses_display_nhmenu(winid wid, int how, menu_item **_selected);
bool curses_menu_exists(winid wid);
void curses_del_menu(winid wid);

#endif /* CURSDIAL_H */
