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
#ifdef GTK_GRAPHICS
/*
 * GTK interface (By issei@guru.gr.jp)
 */
extern struct window_procs GTK_procs;
extern void win_GTK_init(void);
#endif
#ifdef MAC
extern struct window_procs mac_procs;
#endif
#ifdef WIN32_GRAPHICS
extern struct window_procs win32_procs;
#endif
#ifdef GL_GRAPHICS
#include "winGL.h"
extern struct window_procs sdlgl_hardw_procs;
#endif
#ifdef SDL_GRAPHICS
#include "winGL.h"
extern struct window_procs sdlgl_softw_procs;
#endif
#ifdef PROXY_GRAPHICS
#include "winproxy.h"
extern struct window_procs proxy_procs;
extern void win_proxy_init(void);
#endif
#ifdef MSWIN_GRAPHICS
extern struct window_procs mswin_procs;
#endif

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
#ifdef GTK_GRAPHICS
    { &GTK_procs, win_GTK_init },
#endif
#ifdef MAC
    { &mac_procs, 0 },
#endif
#ifdef WIN32_GRAPHICS
    { &win32_procs, 0 },
#endif
#ifdef GL_GRAPHICS
    { &sdlgl_hardw_procs, 0 },
#endif
#ifdef SDL_GRAPHICS
    { &sdlgl_softw_procs, 0 },
#endif
#ifdef PROXY_GRAPHICS
    { &proxy_procs, win_proxy_init },
#endif
#ifdef MSWIN_GRAPHICS
    { &mswin_procs, 0 },
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

static int windows_lock = FALSE;

int 
lock_windows (int flag)
{
    int retval = windows_lock;
    windows_lock = flag;
    return retval;
}

void 
choose_windows (const char *s)
{
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
genl_message_menu (char let, int how, const char *mesg)
{
#if defined(MAC_MPW)
# pragma unused ( how,let )
#endif
    pline("%s", mesg);
    return 0;
}

/*ARGSUSED*/
void 
genl_preference_update (const char *pref)
{
	/* window ports are expected to provide
	   their own preference update routine
	   for the preference capabilities that
	   they support.
	   Just return in this genl one. */
}
/*windows.c*/
