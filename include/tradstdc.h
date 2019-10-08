/*	SCCS Id: @(#)tradstdc.h 3.4	1993/05/30	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef TRADSTDC_H
#define TRADSTDC_H

#include <stdarg.h>
# define VA_NEXT(var1,typ1)	var1 = va_arg(the_args, typ1)
# define VA_ARGS		the_args
# define VA_START(x)		va_list the_args; va_start(the_args, x)
# define VA_END()		va_end(the_args)

/*
 * Allow gcc to check parameters of printf-like calls with -Wformat;
 * append this to a prototype declaration (see pline() in extern.h).
 */
#ifdef __GNUC__
# define PRINTF_F(f,v) __attribute__ ((format (printf, f, v)))
#else
# define PRINTF_F(f,v)
#endif

#endif /* TRADSTDC_H */
