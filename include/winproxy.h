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
extern void FDECL(proxy_init_nhwindows, (int *, char **));
extern void proxy_player_selection(void);
extern void proxy_askname(void);
extern void proxy_get_nh_event(void);
extern void FDECL(proxy_exit_nhwindows, (const char *));
extern void FDECL(proxy_suspend_nhwindows, (const char *));
extern void proxy_resume_nhwindows(void);
extern winid FDECL(proxy_create_nhwindow, (int));
extern void FDECL(proxy_clear_nhwindow, (winid));
extern void FDECL(proxy_display_nhwindow, (winid, BOOLEAN_P));
extern void FDECL(proxy_dismiss_nhwindow, (winid));
extern void FDECL(proxy_destroy_nhwindow, (winid));
extern void FDECL(proxy_curs, (winid, int, int));
extern void FDECL(proxy_putstr, (winid, int, const char *));
#ifdef FILE_AREAS
extern void FDECL(proxy_display_file, (const char *, const char *, BOOLEAN_P));
#else
extern void FDECL(proxy_display_file, (const char *, BOOLEAN_P));
#endif
extern void FDECL(proxy_start_menu, (winid));
extern void FDECL(proxy_add_menu, (winid, int, const ANY_P *,
			CHAR_P, CHAR_P, int, const char *, BOOLEAN_P));
extern void FDECL(proxy_end_menu, (winid, const char *));
extern int FDECL(proxy_select_menu, (winid, int, MENU_ITEM_P **));
extern char FDECL(proxy_message_menu, (CHAR_P, int, const char *));
extern void proxy_update_inventory(void);
extern void proxy_mark_synch(void);
extern void proxy_wait_synch(void);
#ifdef CLIPPING
extern void FDECL(proxy_cliparound, (int, int));
#endif
#ifdef POSITIONBAR
extern void FDECL(proxy_update_positionbar, (char *));
#endif
extern void FDECL(proxy_print_glyph, (winid, XCHAR_P, XCHAR_P, int));
extern void FDECL(proxy_raw_print, (const char *));
extern void FDECL(proxy_raw_print_bold, (const char *));
extern int proxy_nhgetch(void);
extern int FDECL(proxy_nh_poskey, (int *, int *, int *));
extern void proxy_nhbell(void);
extern int proxy_doprev_message(void);
extern char FDECL(proxy_yn_function, (const char *, const char *, CHAR_P));
extern void FDECL(proxy_getlin, (const char *,char *));
extern int proxy_get_ext_cmd(void);
extern void FDECL(proxy_number_pad, (int));
extern void proxy_delay_output(void);
#ifdef CHANGE_COLOR
extern void FDECL(proxy_change_color, (int, long, int));
#ifdef MAC
extern void FDECL(proxy_change_background, (int));
extern short FDECL(set_proxy_font_name, (winid, char *));
#endif
extern char *proxy_get_color_string(void);
#endif
extern void proxy_start_screen(void);
extern void proxy_end_screen(void);
extern void FDECL(proxy_outrip, (winid, int));
extern void FDECL(proxy_preference_update, (const char *));
extern void FDECL(proxy_status, (int, int, const char **));
extern FILE *proxy_config_file_open(void);

/* riputil.c */
extern char *FDECL(get_killer_string, (int));

/* getopt.c */
extern char *FDECL(get_option, (const char *));

/* glyphmap.c */
extern void set_glyph_mapping(void);
extern struct nhproxy_cb_get_glyph_mapping_res *get_glyph_mapping(void);
extern void FDECL(free_glyph_mapping, (struct nhproxy_cb_get_glyph_mapping_res *));
extern int get_no_glyph(void);

/* dlbh.c */
#ifndef FILE_AREAS
extern int FDECL(dlbh_fopen, (const char *, const char *));
#else
extern int FDECL(dlbh_fopen_area(const char *, const char *, const char *));
#endif
extern int FDECL(dlbh_fclose, (int));
extern int FDECL(dlbh_fread, (char *, int, int, int));
extern int FDECL(dlbh_fseek, (int, long, int));
extern char *FDECL(dlbh_fgets, (char *, int, int));
extern int FDECL(dlbh_fgetc, (int));
extern long FDECL(dlbh_ftell, (int));

#endif /* WINPROXY_H */
