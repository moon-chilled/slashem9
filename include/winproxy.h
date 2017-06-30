/* $Id$ */
/* Copyright (c) Slash'EM Development Team 2001-2004 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINPROXY_H
#define WINPROXY_H

extern struct window_procs proxy_procs;

extern unsigned long proxy_interface_mode;

extern short glyph2proxy[MAX_GLYPH];

extern FILE *proxy_config_fp;

/* external declarations */
extern void proxy_init_nhwindows(int *, char **);
extern void proxy_player_selection(void);
extern void proxy_askname(void);
extern void proxy_get_nh_event(void);
extern void proxy_exit_nhwindows(const char *);
extern void proxy_suspend_nhwindows(const char *);
extern void proxy_resume_nhwindows(void);
extern winid proxy_create_nhwindow(int);
extern void proxy_clear_nhwindow(winid);
extern void proxy_display_nhwindow(winid, boolean);
extern void proxy_dismiss_nhwindow(winid);
extern void proxy_destroy_nhwindow(winid);
extern void proxy_curs(winid, int, int);
extern void proxy_putstr(winid, int, const char *);
#ifdef FILE_AREAS
extern void proxy_display_file(const char *, const char *, boolean);
#else
extern void proxy_display_file(const char *, boolean);
#endif
extern void proxy_start_menu(winid);
extern void proxy_add_menu(winid, int, const ANY_P *,
			char, char, int, const char *, boolean);
extern void proxy_end_menu(winid, const char *);
extern int proxy_select_menu(winid, int, MENU_ITEM_P **);
extern char proxy_message_menu(char, int, const char *);
extern void proxy_update_inventory(void);
extern void proxy_mark_synch(void);
extern void proxy_wait_synch(void);
#ifdef CLIPPING
extern void proxy_cliparound(int, int);
#endif
#ifdef POSITIONBAR
extern void proxy_update_positionbar(char *);
#endif
extern void proxy_print_glyph(winid, xchar, xchar, int);
extern void proxy_raw_print(const char *);
extern void proxy_raw_print_bold(const char *);
extern int proxy_nhgetch(void);
extern int proxy_nh_poskey(int *, int *, int *);
extern void proxy_nhbell(void);
extern int proxy_doprev_message(void);
extern char proxy_yn_function(const char *, const char *, char);
extern void proxy_getlin(const char *,char *);
extern int proxy_get_ext_cmd(void);
extern void proxy_number_pad(int);
extern void proxy_delay_output(void);
#ifdef CHANGE_COLOR
extern void proxy_change_color(int, long, int);
#ifdef MAC
extern void proxy_change_background(int);
extern short set_proxy_font_name(winid, char *);
#endif
extern char *proxy_get_color_string(void);
#endif
extern void proxy_start_screen(void);
extern void proxy_end_screen(void);
extern void proxy_outrip(winid, int);
extern void proxy_preference_update(const char *);
extern void proxy_status(int, int, const char **);
extern FILE *proxy_config_file_open(void);

/* riputil.c */
extern char *get_killer_string(int);

/* getopt.c */
extern char *get_option(const char *);

/* glyphmap.c */
extern void set_glyph_mapping(void);
extern struct nhproxy_cb_get_glyph_mapping_res *get_glyph_mapping(void);
extern void free_glyph_mapping(struct nhproxy_cb_get_glyph_mapping_res *);
extern int get_no_glyph(void);

/* dlbh.c */
#ifndef FILE_AREAS
extern int dlbh_fopen(const char *, const char *);
#else
extern int dlbh_fopen_area(const char *, const char *, const char *);
#endif
extern int dlbh_fclose(int);
extern int dlbh_fread(char *, int, int, int);
extern int dlbh_fseek(int, long, int);
extern char *dlbh_fgets(char *, int, int);
extern int dlbh_fgetc(int);
extern long dlbh_ftell(int);

#endif /* WINPROXY_H */
