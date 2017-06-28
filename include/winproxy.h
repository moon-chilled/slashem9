/* $Id$ */
/* Copyright (c) Slash'EM Development Team 2001-2004 */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef WINPROXY_H
#define WINPROXY_H

#define E extern

extern struct window_procs proxy_procs;

extern unsigned long proxy_interface_mode;

extern short glyph2proxy[MAX_GLYPH];

extern FILE *proxy_config_fp;

/* external declarations */
E void FDECL(proxy_init_nhwindows, (int *, char **));
E void proxy_player_selection(void);
E void proxy_askname(void);
E void proxy_get_nh_event(void);
E void FDECL(proxy_exit_nhwindows, (const char *));
E void FDECL(proxy_suspend_nhwindows, (const char *));
E void proxy_resume_nhwindows(void);
E winid FDECL(proxy_create_nhwindow, (int));
E void FDECL(proxy_clear_nhwindow, (winid));
E void FDECL(proxy_display_nhwindow, (winid, BOOLEAN_P));
E void FDECL(proxy_dismiss_nhwindow, (winid));
E void FDECL(proxy_destroy_nhwindow, (winid));
E void FDECL(proxy_curs, (winid, int, int));
E void FDECL(proxy_putstr, (winid, int, const char *));
#ifdef FILE_AREAS
E void FDECL(proxy_display_file, (const char *, const char *, BOOLEAN_P));
#else
E void FDECL(proxy_display_file, (const char *, BOOLEAN_P));
#endif
E void FDECL(proxy_start_menu, (winid));
E void FDECL(proxy_add_menu, (winid, int, const ANY_P *,
			CHAR_P, CHAR_P, int, const char *, BOOLEAN_P));
E void FDECL(proxy_end_menu, (winid, const char *));
E int FDECL(proxy_select_menu, (winid, int, MENU_ITEM_P **));
E char FDECL(proxy_message_menu, (CHAR_P, int, const char *));
E void proxy_update_inventory(void);
E void proxy_mark_synch(void);
E void proxy_wait_synch(void);
#ifdef CLIPPING
E void FDECL(proxy_cliparound, (int, int));
#endif
#ifdef POSITIONBAR
E void FDECL(proxy_update_positionbar, (char *));
#endif
E void FDECL(proxy_print_glyph, (winid, XCHAR_P, XCHAR_P, int));
E void FDECL(proxy_raw_print, (const char *));
E void FDECL(proxy_raw_print_bold, (const char *));
E int proxy_nhgetch(void);
E int FDECL(proxy_nh_poskey, (int *, int *, int *));
E void proxy_nhbell(void);
E int proxy_doprev_message(void);
E char FDECL(proxy_yn_function, (const char *, const char *, CHAR_P));
E void FDECL(proxy_getlin, (const char *,char *));
E int proxy_get_ext_cmd(void);
E void FDECL(proxy_number_pad, (int));
E void proxy_delay_output(void);
#ifdef CHANGE_COLOR
E void FDECL(proxy_change_color, (int, long, int));
#ifdef MAC
E void FDECL(proxy_change_background, (int));
E short FDECL(set_proxy_font_name, (winid, char *));
#endif
E char *proxy_get_color_string(void);
#endif
E void proxy_start_screen(void);
E void proxy_end_screen(void);
E void FDECL(proxy_outrip, (winid, int));
E void FDECL(proxy_preference_update, (const char *));
E void FDECL(proxy_status, (int, int, const char **));
E FILE *proxy_config_file_open(void);

/* riputil.c */
E char *FDECL(get_killer_string, (int));

/* getopt.c */
E char *FDECL(get_option, (const char *));

/* glyphmap.c */
E void set_glyph_mapping(void);
E struct nhproxy_cb_get_glyph_mapping_res *get_glyph_mapping(void);
E void FDECL(free_glyph_mapping, (struct nhproxy_cb_get_glyph_mapping_res *));
E int get_no_glyph(void);

/* dlbh.c */
#ifndef FILE_AREAS
E int FDECL(dlbh_fopen, (const char *, const char *));
#else
E int FDECL(dlbh_fopen_area(const char *, const char *, const char *));
#endif
E int FDECL(dlbh_fclose, (int));
E int FDECL(dlbh_fread, (char *, int, int, int));
E int FDECL(dlbh_fseek, (int, long, int));
E char *FDECL(dlbh_fgets, (char *, int, int));
E int FDECL(dlbh_fgetc, (int));
E long FDECL(dlbh_ftell, (int));

#undef E

#endif /* WINPROXY_H */
