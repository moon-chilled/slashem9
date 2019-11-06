/*	SCCS Id: @(#)system.h	3.4	2001/12/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#include <sys/types.h>
#include <string.h>
#include <stdarg.h>

/* You may want to change this to fit your system, as this is almost
 * impossible to get right automatically.
 * This is the type of signal handling functions.
 */
#if defined(_MSC_VER) || defined(__SC__) || defined(WIN32)
# define SIG_RET_TYPE void (__cdecl *)(int)
#else
# define SIG_RET_TYPE void (*)(int)
#endif

#if defined(_POSIX_SOURCE) || defined(_DEFAULT_SOURCE) || defined(_XOPEN_SOURCE)
extern int tgetent(char *,const char *);
extern void tputs(const char *,int,int (*)());
#endif
int   tgetflag(const char *);
int   tgetnum(const char *);
char *tgetstr(const char *,char **);
char *tgoto(const char *,int,int);
char *tparam(const char *,char *,int,int,int,int,int);


#define VA_ARGS			the_args
#define VA_START(x)		va_list the_args; va_start(the_args, x)
#define VA_END()		va_end(the_args)

/*
 * Allow gcc to check parameters of printf-like calls with -Wformat;
 * append this to a prototype declaration (see pline() in extern.h).
 */
#ifdef __GNUC__
# define PRINTF_F(f,v) __attribute__ ((format (printf, f, v)))
#else
# define PRINTF_F(f,v)
#endif

#endif /* SYSTEM_H */
