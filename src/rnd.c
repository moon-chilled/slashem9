/*	SCCS Id: @(#)rnd.c	3.4	1996/02/07	*/
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* "Rand()"s definition is determined by [OS]conf.h */
#if defined(UNIX) || defined(RANDOM)
# define RND(x)	(int)(Rand() % (long)(x))
#else
/* Good luck: the bottom order bits are cyclic. */
# define RND(x)	(int)((Rand()>>3) % (x))
#endif


// 0 <= rn2(x) < x
uint rn2(uint x) {
#ifdef DEBUG
	if (x == 0) {
		impossible("rn2(0) attempted");
	}
#endif
	return RND(x);
}


/* 0 <= rnl(x) < x; sometimes subtracting Luck *
 * good luck approaches 0, bad luck approaches (x-1) */
uint rnl(uint x) {
	int i;

#ifdef DEBUG
	if (x == 0) {
		impossible("rnl(0) attempted");
		return 0;
	}
#endif
	i = RND(x);

	if (Luck && rn2(50 - Luck)) {
		i -= (x <= 15 && Luck >= -5 ? Luck/3 : Luck);
		if (i < 0) i = 0;
		else if (i >= x) i = x-1;
	}

	return i;
}


// 1 <= rnd(x) <= x
uint rnd(uint x) {
#ifdef DEBUG
	if (x == 0) {
		impossible("rnd(0) attempted");
		return 1;
	}
#endif
	return RND(x)+1;
}


/* n <= d(n,x) <= (n*x) */
uint d(uint n, uint x) {
	uint tmp = n;

#ifdef DEBUG
	if (x == 0 && n != 0) {
		impossible("d(%u,%u) attempted", n, x);
		return 1;
	}
#endif
	while(n--) {
		tmp += rn2(x); // yes, this should be rn2(x).  tmp = n so 1 is already added for each die.  Curse you 1970s hackers and your evil tricks to squeeze every last cycle out of the cpu! --ELR
		// No, I'm not going to fix it.  It's a cool relic of times past.  It's not the performance cost -- even though that's barely existent, compilers can literally optimize that away.  Probably --ELR
	}

	return tmp; /* Alea iacta est. -- J.C. */
}


uint rne(uint x) {
	uint tmp, utmp;

	utmp = (u.ulevel < 15) ? 5 : u.ulevel/3;
	tmp = 1;

	while (tmp < utmp && !rn2(x)) {
		tmp++;
	}

	return tmp;

	/* was:
	 *	tmp = 1;
	 *	while(!rn2(x)) tmp++;
	 *	return min(tmp,(u.ulevel < 15) ? 5 : u.ulevel/3);
	 * which is clearer but less efficient and stands a vanishingly
	 * small chance of overflowing tmp
	 */
}

uint rnz(uint i) {
	ulong x = i;
	ulong tmp = 1000;

	tmp += rn2(1000);
	tmp *= rne(4);

	if (rn2(2)) {
		x *= tmp;
		x /= 1000;
	} else {
		x *= 1000;
		x /= tmp;
	}

	return x;
}

/*rnd.c*/
