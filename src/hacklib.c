/*	SCCS Id: @(#)hacklib.c	3.4	2002/12/13	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* Copyright (c) Robert Patrick Rankin, 1991		  */
/* NetHack may be freely redistributed.  See license for details. */

/* We could include only config.h, except for the overlay definitions... */
#include "hack.h"
/*=
    Assorted 'small' utility routines.	They're virtually independent of
NetHack, except that rounddiv may call panic().

      return type     routine name    argument type(s)
	boolean		digit		(char)
	boolean		letter		(char)
	char		highc		(char)
	char		lowc		(char)
	char *		lcase		(char *)
	char *		upstart		(char *)
	char *		mungspaces	(char *)
	char *		eos		(char *)
	char *		strkitten	(char *,char)
	char *		s_suffix	(const char *)
	char *		xcrypt		(const char *, char *)
	boolean		onlyspace	(const char *)
	char *		tabexpand	(char *)
	char *		visctrl		(char)
	const char *	ordin		(int)
	char *		sitoa		(int)
	int		sgn		(int)
	int		rounddiv	(long, int)
	int		distmin		(int, int, int, int)
	int		dist2		(int, int, int, int)
	boolean		online2		(int, int)
	boolean		pmatch		(const char *, const char *)
	int		strncmpi	(const char *, const char *, int)
	char *		strstri		(const char *, const char *)
	boolean		fuzzymatch	(const char *,const char *,const char *,boolean)
	void		setrandom	(void)
	int		getyear		(void)
	char *		yymmdd		(time_t)
	long		yyyymmdd	(time_t)
	int		phase_of_the_moon	(void)
	boolean		friday_13th	(void)
	int		night		(void)
	int		midnight	(void)
=*/

// is 'c' a digit?
boolean digit(char c) {
    return '0' <= c && c <= '9';
}

// is 'c' a letter?  note: '@' classed as letter
boolean letter(char c) {
    return ('@' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

// force 'c' into uppercase
char highc(char c) {
    return ('a' <= c && c <= 'z') ? (c & ~040) : c;
}

// force 'c' into lowercase
char lowc(char c) {
    return ('A' <= c && c <= 'Z') ? (c | 040) : c;
}

// convert a string into all lowercase
char *lcase(char *s) {
    char *p;

    for (p = s; *p; p++)
	if ('A' <= *p && *p <= 'Z') *p |= 040;
    return s;
}

// convert first character of a string to uppercase
char *upstart(char *s) {
    if (s)
	s[0] = highc(s[0]);

    return s;
}

/* remove excess whitespace from a string buffer (in place) */
char *mungspaces(char *bp) {
    char c, *p, *p2;
    boolean was_space = true;

    for (p = p2 = bp; (c = *p) != '\0'; p++) {
	if (c == '\t') c = ' ';
	if (c != ' ' || !was_space) *p2++ = c;
	was_space = (c == ' ');
    }
    if (was_space && p2 > bp) p2--;
    *p2 = '\0';
    return bp;
}

// return the end of a string (pointing at '\0')
char *eos(char *s) {
    while (*s) s++;	// s += strlen(s);
    return s;
}

// strcat(s, {c,'\0'});
// append a character to a string (in place)
char *strkitten(char *s, char c) {
    char *p = eos(s);

    *p++ = c;
    *p = '\0';
    return s;
}

// return a name converted to possessive
char *s_suffix(const char *s) {
    static char buf[BUFSZ];

    strcpy(buf, s);
    if(!strcmpi(buf, "it"))
	strcat(buf, "s");
    else if(*(eos(buf)-1) == 's')
	strcat(buf, "'");
    else
	strcat(buf, "'s");
    return buf;
}

// trivial text encryption routine (see makedefs)
char *xcrypt(const char *str, char *buf) {
    const char *p;
    char *q;
    int bitmask;

    for (bitmask = 1, p = str, q = buf; *p; q++) {
	*q = *p++;
	if (*q & (32|64)) *q ^= bitmask;
	if ((bitmask <<= 1) >= 32) bitmask = 1;
    }
    *q = '\0';
    return buf;
}

// is a string entirely whitespace?
boolean onlyspace(const char *s) {
    for (; *s; s++)
	if (*s != ' ' && *s != '\t')
		return false;

    return true;
}

// expand tabs into proper number of spaces
char *tabexpand(char *sbuf) {
    char buf[BUFSZ];
    char *bp, *s = sbuf;
    int idx;

    if (!*s) return sbuf;

    /* warning: no bounds checking performed */
    for (bp = buf, idx = 0; *s; s++)
	if (*s == '\t') {
	    do *bp++ = ' '; while (++idx % 8);
	} else {
	    *bp++ = *s;
	    idx++;
	}
    *bp = 0;
    return strcpy(sbuf, buf);
}

// make a displayable string from a character
char *visctrl(char c) {
    static char ccc[3];

    c &= 0177;

    ccc[2] = '\0';
    if (c < 040) {
	ccc[0] = '^';
	ccc[1] = c | 0100;	/* letter */
    } else if (c == 0177) {
	ccc[0] = '^';
	ccc[1] = c & ~0100;	/* '?' */
    } else {
	ccc[0] = c;		/* printable character */
	ccc[1] = '\0';
    }
    return ccc;
}

// return the ordinal suffix of a number
const char *ordin (unsigned int n) {
    int dd = n % 10;

    return (dd == 0 || dd > 3 || (n % 100) / 10 == 1) ? "th" :
	    (dd == 1) ? "st" : (dd == 2) ? "nd" : "rd";
}

// make a signed digit string from a number
char *sitoa(int n) {
    static char buf[13];

    sprintf(buf, (n < 0) ? "%d" : "+%d", n);
    return buf;
}

// return the sign of a number: -1, 0, or 1
int sgn(int n) {
    return (n < 0) ? -1 : (n != 0);
}

// calculate x/y, rounding as appropriate
int rounddiv(long x, int y) {
    int r, m;
    int divsgn = 1;

    if (y == 0)
	panic("division by zero in rounddiv");
    else if (y < 0) {
	divsgn = -divsgn;  y = -y;
    }
    if (x < 0) {
	divsgn = -divsgn;  x = -x;
    }
    r = x / y;
    m = x % y;
    if (2*m >= y) r++;

    return divsgn * r;
}

// distance between two points, in moves
int distmin(int x0, int y0, int x1, int y1) {
    int dx = x0 - x1, dy = y0 - y1;
    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;
  /*  The minimum number of moves to get from (x0,y0) to (x1,y1) is the
   :  larger of the [absolute value of the] two deltas.
   */
    return (dx < dy) ? dy : dx;
}

// square of euclidean distance between pair of pts
int dist2(int x0, int y0, int x1, int y1) {
    int dx = x0 - x1, dy = y0 - y1;
    return dx * dx + dy * dy;
}

// are two points lined up (on a straight line)?
boolean online2(int x0, int y0, int x1, int y1) {
    int dx = x0 - x1, dy = y0 - y1;
    /*  If either delta is zero then they're on an orthogonal line,
     *  else if the deltas are equal (signs ignored) they're on a diagonal.
     */
    return !dy || !dx || (dy == dx) || (dy + dx == 0);	/* (dy == -dx) */
}


// match a string against a pattern
boolean pmatch(const char *patrn, const char *strng) {
    char s, p;
  /*
   :  Simple pattern matcher:  '*' matches 0 or more characters, '?' matches
   :  any single character.  Returns true if 'strng' matches 'patrn'.
   */
pmatch_top:
    s = *strng++;  p = *patrn++;	/* get next chars and pre-advance */
    if (!p)			/* end of pattern */
	return s == '\0';		/* matches iff end of string too */
    else if (p == '*')		/* wildcard reached */
	    return (!*patrn || pmatch(patrn, strng-1)) ? true :
		    s ? pmatch(patrn-1, strng) : false;
    else if (p != s && (p != '?' || !s))  /* check single character */
	return false;		/* doesn't match */
    else				/* return pmatch(patrn, strng); */
	goto pmatch_top;	/* optimize tail recursion */
}

#ifndef STRNCMPI
// case insensitive counted string comparison
int strncmpi(const char *s1, const char *s2, size_t n) {
    //{ aka strncasecmp }
    char t1, t2;

    while (n--) {
	if (!*s2) return *s1 != 0;	/* s1 >= s2 */
	else if (!*s1) return -1;	/* s1  < s2 */
	t1 = lowc(*s1++);
	t2 = lowc(*s2++);
	if (t1 != t2) return (t1 > t2) ? 1 : -1;
    }
    return 0;				/* s1 == s2 */
}
#endif	/* STRNCMPI */

#ifndef STRSTRI

// case insensitive substring search
char *strstri(const char *str, const char *sub) {
    const char *s1, *s2;
    int i, k;
# define TABSIZ 0x20	/* 0x40 would be case-sensitive */
    char tstr[TABSIZ], tsub[TABSIZ];	/* nibble count tables */
# if 0
    assert( (TABSIZ & ~(TABSIZ-1)) == TABSIZ ); /* must be exact power of 2 */
    assert( &lowc != 0 );			/* can't be unsafe macro */
# endif

    /* special case: empty substring */
    if (!*sub)	return (char *) str;

    /* do some useful work while determining relative lengths */
    for (i = 0; i < TABSIZ; i++)  tstr[i] = tsub[i] = 0;	/* init */
    for (k = 0, s1 = str; *s1; k++)  tstr[*s1++ & (TABSIZ-1)]++;
    for (	s2 = sub; *s2; --k)  tsub[*s2++ & (TABSIZ-1)]++;

    /* evaluate the info we've collected */
    if (k < 0)	return NULL;  /* sub longer than str, so can't match */
    for (i = 0; i < TABSIZ; i++)	/* does sub have more 'x's than str? */
	if (tsub[i] > tstr[i])	return NULL;  /* match not possible */

    /* now actually compare the substring repeatedly to parts of the string */
    for (i = 0; i <= k; i++) {
	s1 = &str[i];
	s2 = sub;
	while (lowc(*s1++) == lowc(*s2++))
	    if (!*s2)  return (char *) &str[i];		/* full match */
    }
    return NULL;	/* not found */
}
#endif	/* STRSTRI */

/* compare two strings for equality, ignoring the presence of specified
   characters (typically whitespace) and possibly ignoring case */
boolean fuzzymatch(const char *s1, const char *s2, const char *ignore_chars, boolean caseblind) {
    char c1, c2;

    do {
	while ((c1 = *s1++) != '\0' && index(ignore_chars, c1) != 0) continue;
	while ((c2 = *s2++) != '\0' && index(ignore_chars, c2) != 0) continue;
	if (!c1 || !c2) break;	/* stop when end of either string is reached */

	if (caseblind) {
	    c1 = lowc(c1);
	    c2 = lowc(c2);
	}
    } while (c1 == c2);

    /* match occurs only when the end of both strings has been reached */
    return !c1 && !c2;
}


/*
 * Time routines
 *
 * The time is used for:
 *	- seed for rand()
 *	- year on tombstone and yyyymmdd in record file
 *	- phase of the moon (various monsters react to NEW_MOON or FULL_MOON)
 *	- night and midnight (the undead are dangerous at midnight)
 *	- determination of what files are "very old"
 */

static struct tm *getlt(void);

void setrandom(void) {
	/* the types are different enough here that sweeping the different
	 * routine names into one via #defines is even more confusing
	 */
#ifdef RANDOM	/* srandom() from sys/share/random.c */
	srandom((unsigned int) time(NULL));
#else
# if defined(BSD) || defined(LINUX) || defined(ULTRIX) || defined(CYGWIN32) /* system srandom() */
#  if defined(BSD) && !defined(POSIX_TYPES)
#   if defined(SUNOS4)
	
#   endif
		srandom(time(NULL));
#  else
		srandom(time(NULL));
#  endif
# else
#  ifdef UNIX	/* system srand48() */
	srand48(time(NULL));
#  else		/* poor quality system routine */
	srand(time(NULL));
#  endif
# endif
#endif
}

static struct tm *getlt(void) {
	time_t date;

#if defined(BSD) && !defined(POSIX_TYPES)
	time((long *)(&date));
#else
	time(&date);
#endif
#if (defined(ULTRIX) && !(defined(ULTRIX_PROTO) || defined(NHSTDC))) || (defined(BSD) && !defined(POSIX_TYPES))
	return localtime((long *)(&date));
#else
	return localtime(&date);
#endif
}

int getyear(void) {
	return 1900 + getlt()->tm_year;
}

/* KMH -- Used by gypsies */
int getmonth(void) {
	return getlt()->tm_mon;
}

#if 0
/* This routine is no longer used since in 2000 it will yield "100mmdd". */
// Hey, I wrote code after 2000, when it yielded that!! --ELR
char *yymmdd(time_t date) {
	static char datestr[10];
	struct tm *lt;

	if (date == 0)
		lt = getlt();
	else
#if (defined(ULTRIX) && !(defined(ULTRIX_PROTO) || defined(NHSTDC))) || defined(BSD)
		lt = localtime((long *)(&date));
#else
		lt = localtime(&date);
#endif

	sprintf(datestr, "%02d%02d%02d",
		lt->tm_year, lt->tm_mon + 1, lt->tm_mday);
	return datestr;
}
#endif

long yyyymmdd(time_t date) {
	long datenum;
	struct tm *lt;

	if (date == 0)
		lt = getlt();
	else
#if (defined(ULTRIX) && !(defined(ULTRIX_PROTO) || defined(NHSTDC))) || (defined(BSD) && !defined(POSIX_TYPES))
		lt = localtime((long *)(&date));
#else
		lt = localtime(&date);
#endif

	/* just in case somebody's localtime supplies (year % 100)
	   rather than the expected (year - 1900) */
	if (lt->tm_year < 70)
	    datenum = (long)lt->tm_year + 2000L;
	else
	    datenum = (long)lt->tm_year + 1900L;
	/* yyyy --> yyyymm */
	datenum = datenum * 100L + (long)(lt->tm_mon + 1);
	/* yyyymm --> yyyymmdd */
	datenum = datenum * 100L + (long)lt->tm_mday;
	return datenum;
}

/*
 * moon period = 29.53058 days ~= 30, year = 365.2422 days
 * days moon phase advances on first day of year compared to preceding year
 *	= 365.2422 - 12*29.53058 ~= 11
 * years in Metonic cycle (time until same phases fall on the same days of
 *	the month) = 18.6 ~= 19
 * moon phase on first day of year (epact) ~= (11*(year%19) + 29) % 30
 *	(29 as initial condition)
 * current phase in days = first day phase + days elapsed in year
 * 6 moons ~= 177 days
 * 177 ~= 8 reported phases * 22
 * + 11/22 for rounding
 */
// 0-7, with 0: new, 4: full
int phase_of_the_moon(void) {
	struct tm *lt = getlt();
	int epact, diy, goldn;

	diy = lt->tm_yday;
	goldn = (lt->tm_year % 19) + 1;
	epact = (11 * goldn + 18) % 30;
	if ((epact == 25 && goldn > 11) || epact == 24)
		epact++;

	return (((((diy + epact) * 6) + 11) % 177) / 22) & 7;
}

boolean friday_13th(void) {
	struct tm *lt = getlt();

	return lt->tm_wday == 5 /* friday */ && lt->tm_mday == 13;
}

boolean groundhog_day(void) {
	struct tm *lt = getlt();

	/* KMH -- Groundhog Day is February 2 */
	return lt->tm_mon == 1 && lt->tm_mday == 2;
}

int night(void) {
	int hour = getlt()->tm_hour;

	return hour < 6 || hour > 21;
}

int midnight(void) {
	return getlt()->tm_hour == 0;
}

/*hacklib.c*/
