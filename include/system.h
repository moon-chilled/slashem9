/*	SCCS Id: @(#)system.h	3.4	2001/12/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef SYSTEM_H
#define SYSTEM_H

#if !defined(__cplusplus) && !defined(__GO32__)

/* some old <sys/types.h> may not define off_t and size_t; if your system is
 * one of these, define them by hand below
 */
#ifdef MAC
#include <types.h>
#else
#  ifndef       __WATCOMC__
#include <sys/types.h>
# endif
#endif

#if defined(MICRO) || defined(ANCIENT_VAXC)
# if !defined(_SIZE_T) && !defined(__size_t) /* __size_t for CSet/2 */
#  define _SIZE_T
# endif
#endif	// MICRO || ANCIENT_VAXC

#if defined(MAC)
#include <time.h>	/* time_t is not in <sys/types.h> */
#endif
#if defined(ULTRIX) && !(defined(ULTRIX_PROTO) || defined(NHSTDC))
/* The Ultrix v3.0 <sys/types.h> seems to be very wrong. */
# define time_t long
#endif

#ifdef ULTRIX
# define off_t long
#endif

#endif /* !__cplusplus && !__GO32__ */

/* You may want to change this to fit your system, as this is almost
 * impossible to get right automatically.
 * This is the type of signal handling functions.
 */
#if defined(_MSC_VER) || defined(__SC__) || defined(WIN32)
# define SIG_RET_TYPE void (__cdecl *)(int)
#endif
#ifndef SIG_RET_TYPE
# if defined(NHSTDC) || defined(POSIX_TYPES) || defined(__DECC)
#  define SIG_RET_TYPE void (*)(int)
# endif
#endif
#ifndef SIG_RET_TYPE
# if defined(ULTRIX) || defined(SUNOS4) || defined(SVR3) || defined(SVR4)
	/* SVR3 is defined automatically by some systems */
#  define SIG_RET_TYPE void (*)()
# endif
#endif
#ifndef SIG_RET_TYPE	/* BSD, SIII, SVR2 and earlier, Sun3.5 and earlier */
# define SIG_RET_TYPE int (*)()
#endif

#if !defined(__cplusplus) && !defined(__GO32__)

#if defined(BSD) || defined(ULTRIX) || defined(RANDOM)
# ifdef random
# undef random
# endif
# if !defined(__SC__) && !defined(__CYGWIN__) && !defined(LINUX)
extern  long random(void);
# endif
# if (!defined(SUNOS4) && !defined(bsdi) && !defined(__NetBSD__) && !defined(__FreeBSD__) && !defined(__DragonFly__) && !defined(__APPLE__)) || defined(RANDOM)
extern void srandom(unsigned int);
# else
#  if !defined(bsdi) && !defined(LINUX) && !defined(__CYGWIN__) && !defined(__FreeBSD__)
extern int srandom(unsigned int);
#  endif
# endif
#else
extern long lrand48(void);
extern void srand48(long);
#endif /* BSD || ULTRIX || RANDOM */

#if !defined(BSD) || defined(ultrix)
			/* real BSD wants all these to return int */
# ifndef MICRO
extern void exit(int);
# endif /* MICRO */
#if !defined(__SASC_60) && !defined(__SC__)
#  if !(defined(ULTRIX_PROTO) && defined(__GNUC__))
extern void perror(const char *);
# endif
#endif
#endif
#ifndef NeXT
#ifdef POSIX_TYPES
extern void qsort(void *,size_t,size_t,
		     int(*)(const void*,const void*));
#else
# if (defined(BSD) || defined(ULTRIX)) && (!defined(LINUX) && !defined(__CYGWIN__))
extern  int qsort();
# else
#  if !defined(LATTICE)
extern   void qsort(void *,size_t,size_t,
		       int(*)(const void*,const void*));
#  endif
# endif
#endif
#endif /* NeXT */

#ifndef __SASC_60
#if !defined(__GNUC__)
/* may already be defined */

# ifdef ULTRIX
#  ifdef ULTRIX_PROTO
extern int lseek(int,off_t,int);
#  else
extern long lseek(int,off_t,int);
#  endif
  /* Ultrix 3.0 man page mistakenly says it returns an int. */
extern int write(int,char *,int);
extern int link(const char *, const char*);
# else
# ifndef bsdi
extern long lseek(int,long,int);
# endif
#  if defined(POSIX_TYPES) || defined(_MSC_VER)
#   ifndef bsdi
extern int write(int, const void *,unsigned);
#   endif
#  else
#   ifndef __MWERKS__	/* metrowerks defines write via universal headers */
extern int write(int,void *,unsigned);
#   endif
#  endif
# endif /* ULTRIX */
#endif /* __GNUC__ */

#ifdef MAC
#ifndef __CONDITIONALMACROS__	/* universal headers */
extern int close(int);		/* unistd.h */
extern int read(int, char *, int);	/* unistd.h */
extern int chdir(const char *);	/* unistd.h */
extern char *getcwd(char *,int);	/* unistd.h */
#endif

extern int open(const char *,int);
#endif

#if defined(MICRO)
extern int close(int);
#ifndef __EMX__
extern int read(int,void *,unsigned int);
#endif
extern int open(const char *,int,...);
extern int dup2(int, int);
extern int setmode(int,int);
extern int kbhit(void);
# ifndef __MINGW32__
#  ifdef _MSC_VER
extern int chdir(const char *);
#  else
#   ifndef __EMX__
extern int chdir(char *);
#   endif
#  endif
#  ifndef __EMX__
extern char *getcwd(char *,int);
#  endif
# endif /* !MINGW32 */
#endif

#ifdef ULTRIX
extern int close(int);
extern int atoi(const char *);
extern int chdir(const char *);
# if !defined(ULTRIX_CC20) && !defined(__GNUC__)
extern int chmod(const char *,int);
extern mode_t umask(int);
# endif
extern int read(int,void *,unsigned);
/* these aren't quite right, but this saves including lots of system files */
extern int stty(int,void *);
extern int gtty(int,void *);
extern int ioctl(int, int, char*);
extern int isatty(int);	/* 1==yes, 0==no, -1==error */
#include <sys/file.h>
# if defined(ULTRIX_PROTO) || defined(__GNUC__)
extern int fork(void);
# else
extern long fork(void);
# endif
#endif /* ULTRIX */

#endif	/* __SASC_60 */

/* both old & new versions of Ultrix want these, but real BSD does not */
#ifdef ultrix
extern void abort();
extern void bcopy();
# ifdef ULTRIX
extern int system(const char *);
#  ifndef _UNISTD_H_
extern int execl(const char *, ...);
#  endif
# endif
#endif
#ifdef MICRO
extern void abort(void);
extern void _exit(int);
extern int system(const char *);
#endif
#if defined(HPUX) && !defined(_POSIX_SOURCE)
extern long fork(void);
#endif

#ifdef POSIX_TYPES
/* The POSIX string.h is required to define all the mem* and str* functions */
#include <string.h>
#else
#if defined(SYSV) || defined(MAC) || defined(SUNOS4)
# ifdef NHSTDC
#  if !defined(_AIX32) && !(defined(SUNOS4) && defined(__STDC__)) && !defined(LINUX)
/* Solaris unbundled cc (acc) */
extern int memcmp(const void *,const void *,size_t);
extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
#  endif
# else
#  ifndef memcmp	/* some systems seem to macro these back to b*() */
extern int memcmp();
#  endif
#  ifndef memcpy
extern char *memcpy();
#  endif
#  ifndef memset
extern char *memset();
#  endif
# endif
#else
# ifdef HPUX
extern int memcmp(char *,char *,int);
extern void *memcpy(char *,char *,int);
extern void *memset(char*,int,int);
# endif
#endif
#endif /* POSIX_TYPES */

#if defined(MICRO) && !defined(LATTICE)
#  if defined(NHSTDC) || defined(WIN32)
extern int  memcmp(const void *, const void *, size_t);
extern void *memcpy(void *, const void *, size_t);
extern void *memset(void *, int, size_t);
#  else
extern int memcmp(char *,char *,unsigned int);
extern char *memcpy(char *,char *,unsigned int);
extern char *memset(char*,int,int);
#  endif /* NHSTDC */
#endif /* MICRO */

#if defined(BSD) && defined(ultrix)	/* i.e., old versions of Ultrix */
extern void sleep();
#endif
#if defined(ULTRIX) || defined(SYSV)
extern unsigned sleep(unsigned);
#endif
#if defined(HPUX)
extern unsigned int sleep(unsigned int);
#endif

extern char *getenv(const char *);
extern char *getlogin(void);
#if defined(HPUX) && !defined(_POSIX_SOURCE)
extern long getuid(void);
extern long getgid(void);
extern long getpid(void);
#else
# ifdef POSIX_TYPES
extern pid_t getpid(void);
extern uid_t getuid(void);
extern gid_t getgid(void);
# else	/*!POSIX_TYPES*/
#  ifndef getpid		/* Borland C defines getpid() as a macro */
extern int getpid(void);
#  endif
#  if defined(ULTRIX) && !defined(_UNISTD_H_)
extern unsigned getuid(void);
extern unsigned getgid(void);
extern int setgid(int);
extern int setuid(int);
#  endif
# endif	/*?POSIX_TYPES*/
#endif	/*?(HPUX && !_POSIX_SOURCE)*/

/* add more architectures as needed */
#if defined(HPUX)
#define seteuid(x) setreuid(-1, (x));
#endif

/*# string(s).h #*/
#if !defined(_XtIntrinsic_h) && !defined(POSIX_TYPES)
/* <X11/Intrinsic.h> #includes <string[s].h>; so does defining POSIX_TYPES */

#if (defined(ULTRIX) || defined(NeXT)) && defined(__GNUC__)
#include <strings.h>
#else
extern char	*strcpy(char *,const char *);
extern char	*strncpy(char *,const char *,size_t);
extern char	*strcat(char *,const char *);
extern char	*strncat(char *,const char *,size_t);
extern char	*strpbrk(const char *,const char *);

# if defined(SYSV) || defined(MICRO) || defined(MAC) || defined(HPUX)
extern char	*strchr(const char *,int);
extern char	*strrchr(const char *,int);
# else /* BSD */
extern char	*index(const char *,int);
extern char	*rindex(const char *,int);
# endif

extern int	strcmp(const char *,const char *);
extern int	strncmp(const char *,const char *,size_t);
# if defined(MICRO) || defined(MAC)
extern size_t strlen(const char *);
# else
# ifdef HPUX
extern unsigned int	strlen(char *);
#  else
#   if !(defined(ULTRIX_PROTO) && defined(__GNUC__)) && !defined(LINUX)
extern int	strlen(const char *);
#   endif
#  endif /* HPUX */
# endif /* MICRO */
#endif /* ULTRIX */

#endif	/* !_XtIntrinsic_h_ && !POSIX_TYPES */

#if defined(ULTRIX) && defined(__GNUC__)
extern char	*index(const char *,int);
extern char	*rindex(const char *,int);
#endif

/* Old varieties of BSD have char *sprintf().
 * Newer varieties of BSD have int sprintf() but allow for the old char *.
 * Several varieties of SYSV and PC systems also have int sprintf().
 * If your system doesn't agree with this breakdown, you may want to change
 * this declaration, especially if your machine treats the types differently.
 * If your system defines sprintf, et al, in stdio.h, add to the initial
 * #if.
 */
#if defined(ULTRIX) || defined(__DECC) || defined(__SASC_60) || defined(WIN32)
#define SPRINTF_PROTO
#endif
#if (defined(SUNOS4) && defined(__STDC__)) || defined(_AIX32)
#define SPRINTF_PROTO
#endif
#if defined(__sgi) || defined(__GNUC__)
	/* problem with prototype mismatches */
#define SPRINTF_PROTO
#endif
#if defined(__MWERKS__) || defined(__SC__)
	/* Metrowerks already has a prototype for sprintf() */
# define SPRINTF_PROTO
#endif

#ifndef SPRINTF_PROTO
# if defined(POSIX_TYPES) || defined(NeXT) || !defined(BSD)
extern  int sprintf(char *,const char *,...);
# else
#  define OLD_SPRINTF
extern  char *sprintf();
# endif
#endif
#ifdef SPRINTF_PROTO
# undef SPRINTF_PROTO
#endif

#ifndef __SASC_60
# if defined(USE_STDARG) || defined(USE_VARARGS)
#  if !defined(SVR4)
#   if !(defined(ULTRIX_PROTO) && defined(__GNUC__))
#    if !(defined(SUNOS4) && defined(__STDC__)) /* Solaris unbundled cc (acc) */
extern int vsprintf(char *, const char *, va_list);
extern int vfprintf(FILE *, const char *, va_list);
extern int vprintf(const char *, va_list);
#    endif
#   endif
#  endif
# else
#  define vprintf	printf
#  define vfprintf	fprintf
#  define vsprintf	sprintf
# endif
#endif


#ifdef MICRO
extern int tgetent(const char *,const char *);
extern void tputs(const char *,int,int (*)());
extern int tgetnum(const char *);
extern int tgetflag(const char *);
extern char *tgetstr(const char *,char **);
extern char *tgoto(const char *,int,int);
#else
# if ! (defined(HPUX) && defined(_POSIX_SOURCE))
extern int tgetent(char *,const char *);
extern void tputs(const char *,int,int (*)());
# endif
extern int tgetnum(const char *);
extern int tgetflag(const char *);
extern char *tgetstr(const char *,char **);
extern char *tgoto(const char *,int,int);
#endif

#ifdef ALLOC_C
extern void * malloc(size_t);
#endif

/* time functions */

# ifndef LATTICE
#  if !(defined(ULTRIX_PROTO) && defined(__GNUC__))
#   ifndef      __WATCOMC__
extern struct tm *localtime(const time_t *);
#  endif
# endif
# endif

# if defined(ULTRIX) || (defined(BSD) && defined(POSIX_TYPES)) || defined(SYSV) || defined(MICRO) || defined(MAC) || (defined(HPUX) && defined(_POSIX_SOURCE))
#  ifndef       __WATCOMC__
extern time_t time(time_t *);
#  endif
# else
extern long time(time_t *);
# endif /* ULTRIX */

#ifdef MICRO
# ifdef abs
# undef abs
# endif
extern int abs(int);
# ifdef atoi
# undef atoi
# endif
extern int atoi(const char *);
#endif

#endif /*  !__cplusplus && !__GO32__ */

#endif /* SYSTEM_H */
