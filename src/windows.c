/*	SCCS Id: @(#)windows.c	3.4	1996/05/19	*/
/* Copyright (c) D. Cohrs, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#ifdef TTY_GRAPHICS
#include "wintty.h"
#endif
#ifdef CURSES_GRAPHICS
extern struct window_procs curses_procs;
#endif
#ifdef PROXY_GRAPHICS
#include "winproxy.h"
extern struct window_procs proxy_procs;
extern void win_proxy_init(void);
#endif

static winid dump_create_nhwindow(int);
static void dump_clear_nhwindow(winid);
static void dump_display_nhwindow(winid, bool);
static void dump_destroy_nhwindow(winid);
static void dump_start_menu(winid);
static void dump_add_menu(winid win, int glyph, const anything *identifier, char ch, char gch, int attr, const char *str, bool preselected);
static void dump_end_menu(winid, const char *);
static int dump_select_menu(winid, int, menu_item**);
static void dump_putstr(winid, int, const char *);


static void def_raw_print(const char *s);

struct window_procs windowprocs;

static
struct win_choices {
	struct window_procs *procs;
	void (*ini_routine)(void);		/* optional (can be 0) */
} winchoices[] = {
#ifdef TTY_GRAPHICS
	{ &tty_procs, win_tty_init },
#endif
#ifdef CURSES_GRAPHICS
	{ &curses_procs, 0 },
#endif
#ifdef PROXY_GRAPHICS
	{ &proxy_procs, win_proxy_init },
#endif
	{ 0, 0 }		/* must be last */
};

static
void
def_raw_print(s)
const char *s;
{
	puts(s);
}

static int windows_lock = false;

int
lock_windows (int flag) {
	int retval = windows_lock;
	windows_lock = flag;
	return retval;
}

void
choose_windows (const char *s) {
	int i;

	if (windows_lock)
		return;

	for(i=0; winchoices[i].procs; i++)
		if (!strcmpi(s, winchoices[i].procs->name)) {
			windowprocs = *winchoices[i].procs;
			if (winchoices[i].ini_routine) (*winchoices[i].ini_routine)();
			return;
		}

	if (!windowprocs.win_raw_print)
		windowprocs.win_raw_print = def_raw_print;

	raw_printf("Window type %s not recognized.  Choices are:", s);
	for(i=0; winchoices[i].procs; i++)
		raw_printf("        %s", winchoices[i].procs->name);

	if (windowprocs.win_raw_print == def_raw_print)
		terminate(EXIT_SUCCESS);
	wait_synch();
}

/*
 * tty_message_menu() provides a means to get feedback from the
 * --More-- prompt; other interfaces generally don't need that.
 */
/*ARGSUSED*/
char
genl_message_menu (char let, int how, const char *mesg) {
#if defined(MAC_MPW)
# pragma unused ( how,let )
#endif
	pline("%s", mesg);
	return 0;
}

/*ARGSUSED*/
void
genl_preference_update (const char *pref) {
	/* window ports are expected to provide
	   their own preference update routine
	   for the preference capabilities that
	   they support.
	   Just return in this genl one. */
}
static struct window_procs dumplog_windowprocs_backup;
static FILE *dumplog_file;

static time_t dumplog_now;

static char *dump_fmtstr(const char *fmt, char *buf) {
	const char *fp = fmt;
	char *bp = buf;
	int slen, len = 0;
	char tmpbuf[BUFSZ];
	time_t now;

	now = dumplog_now;

	/*
	 * Note: %t and %T assume that time_t is a 'long int' number of
	 * seconds since some epoch value.  That's quite iffy....  The
	 * unit of time might be different and the datum size might be
	 * some variant of 'long long int'.  [Their main purpose is to
	 * construct a unique file name rather than record the date and
	 * time; violating the 'long seconds since base-date' assumption
	 * may or may not interfere with that usage.]
	 */

	while (fp && *fp && len < BUFSZ-1) {
		if (*fp == '%') {
			fp++;
			switch (*fp) {
				default:
					goto finish;
				case '\0': /* fallthrough */
				case '%':  /* literal % */
					sprintf(tmpbuf, "%%");
					break;
				case 't': /* game start, timestamp */
					sprintf(tmpbuf, "%lu", (unsigned long) u.ubirthday);
					break;
				case 'T': /* current time, timestamp */
					sprintf(tmpbuf, "%lu", (unsigned long) now);
					break;
				case 'd': /* game start, YYYYMMDDhhmmss */
					sprintf(tmpbuf, "%08ld%06ld", yyyymmdd(u.ubirthday), hhmmss(u.ubirthday));
					break;
				case 'D': /* current time, YYYYMMDDhhmmss */
					sprintf(tmpbuf, "%08ld%06ld", yyyymmdd(now), hhmmss(now));
					break;
				case 'v': /* version, eg. "3.6.2-0" */
					sprintf(tmpbuf, "%s", version_string_tmp());
					break;
				case 'n': /* player name */
					sprintf(tmpbuf, "%s", *plname ? plname : "unknown");
					break;
				case 'N': /* first character of player name */
					sprintf(tmpbuf, "%c", *plname ? *plname : 'u');
					break;
			}

			slen = strlen(tmpbuf);
			if (len + slen < BUFSZ-1) {
				len += slen;
				sprintf(bp, "%s", tmpbuf);
				bp += slen;
				if (*fp) fp++;
			} else
				break;
		} else {
			*bp = *fp;
			bp++;
			fp++;
			len++;
		}
	}
finish:
	*bp = '\0';
	return buf;
}

void dump_open_log(time_t now) {
	char buf[BUFSZ];
	char *fname;

	dumplog_now = now;
#ifdef SYSCF
	if (!sysopt.dumplogfile)
		return;
	fname = dump_fmtstr(sysopt.dumplogfile, buf);
#else
	fname = dump_fmtstr(DUMPLOG_FILE, buf);
#endif
	dumplog_file = fopen(fname, "w");
	dumplog_windowprocs_backup = windowprocs;

}

void dump_close_log(void) {
	if (dumplog_file) {
		fclose(dumplog_file);
		dumplog_file = NULL;
	}
}

void dump_forward_putstr(winid win, int attr, const char *str, int no_forward) {
	if (dumplog_file)
		fprintf(dumplog_file, "%s\n", str);
	if (!no_forward)
		putstr(win, attr, str);
}

static void dump_putstr(winid win, int attr, const char *str) {
	if (dumplog_file)
		fprintf(dumplog_file, "%s\n", str);
}

static winid dump_create_nhwindow(int dummy) {}

static void dump_clear_nhwindow(winid win) {}

static void dump_display_nhwindow(winid win, bool p) {}

static void dump_destroy_nhwindow(winid win) {}

static void dump_start_menu(winid win) {}

static void dump_add_menu(winid win, int glyph, const anything *identifier, char ch, char gch, int attr, const char *str, bool preselected) {
	if (dumplog_file) {
		if (glyph == NO_GLYPH)
			fprintf(dumplog_file, " %s\n", str);
		else
			fprintf(dumplog_file, "  %c - %s\n", ch, str);
	}
}

static void dump_end_menu(winid win, const char *str) {
	if (dumplog_file) {
		if (str) {
			fprintf(dumplog_file, "%s\n", str);
		} else {
			fputs("\n", dumplog_file);
		}
	}
}

static int dump_select_menu(winid win, int how, menu_item **item) {
    *item = NULL;
    return 0;
}

void dump_redirect(bool onoff_flag) {
	if (dumplog_file) {
		if (onoff_flag) {
			windowprocs.win_create_nhwindow = dump_create_nhwindow;
			windowprocs.win_clear_nhwindow = dump_clear_nhwindow;
			windowprocs.win_display_nhwindow = dump_display_nhwindow;
			windowprocs.win_destroy_nhwindow = dump_destroy_nhwindow;
			windowprocs.win_start_menu = dump_start_menu;
			windowprocs.win_add_menu = dump_add_menu;
			windowprocs.win_end_menu = dump_end_menu;
			windowprocs.win_select_menu = dump_select_menu;
			windowprocs.win_putstr = dump_putstr;
		} else {
			windowprocs = dumplog_windowprocs_backup;
		}
		iflags.in_dumplog = onoff_flag;
	} else {
		iflags.in_dumplog = false;
	}
}


/*windows.c*/
