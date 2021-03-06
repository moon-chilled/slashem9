/*	SCCS Id: @(#)mextra.h	3.5	2006/01/03	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef MEXTRA_H
#define MEXTRA_H

#ifndef ALIGN_H
#include "align.h"
#endif

/*
 *  Adding new mextra structures:
 *
 *      1. Add the structure definition and any required macros in this file
 *         above the mextra struct.
 *
 *      2. Add a pointer to your new struct to the mextra struct in this file.
 *
 *      3. Add a referencing macro at the bottom of this file after the mextra
 *         struct (see MNAME, EGD, EPRI, ESHK, EMIN, or EDOG for examples).
 *
 *      4. Create a newXX(mtmp) function and possibly a free_XX(mtmp) function
 *         in an appropriate new or existing source file and add a prototype
 *         for it to include/extern.h.
 *
 *		void newXX(struct monst *);
 *		void free_XX(struct monst *);
 *
 *		void *newXX(struct monst *mtmp) {
 *			if (!mtmp->mextra) mtmp->mextra = newmextra();
 *			if (!XX(mtmp)) {
 *				XX(mtmp) = new(struct XX);
 *			}
 *		}
 *
 *      5. Consider adding a new makemon flag MM_XX flag to include/hack.h and
 *         a corresponding change to makemon() if you require your structure
 *          to be added at monster creation time. Initialize your struct
 *         after a successful return from makemon().
 *
 *          src/makemon.c:  if (mmflags & MM_XX) newXX(mtmp);
 *          your new code:  mon = makemon(&mons[mnum], x, y, MM_XX);
 *
 *      6. Adjust size_monst() in src/cmd.c appropriately.
 *
 *      7. Adjust dealloc_mextra() in src/mon.c to clean up
 *         properly during monst deallocation.
 *
 *      8. Adjust restmonchn() in src/restore.c to deal with your
 *         struct during a restore.
 *
 *      9. Adjust buffer_to_mon() in src/restore.c to properly
 *
 *     10. Adjust savemonchn() in src/save.c to deal with your
 *         struct during a save.
 *
 *     11. Adjust mon_to_buffer() in src/save.c to properly package
 *         up your struct when the rest of the monst struct is
 *         packaged up.
 */

#define FCSIZ	(ROWNO+COLNO)

struct fakecorridor {
	xchar fx, fy, ftyp;
};

struct egd {
	int fcbeg, fcend;	/* fcend: first unused pos */
	int vroom;		/* room number of the vault */
	xchar gdx, gdy;		/* goal of guard's walk */
	xchar ogx, ogy;		/* guard's last position */
	d_level gdlevel;	/* level (& dungeon) guard was created in */
	xchar warncnt;		/* number of warnings to follow */
	bool gddone;		/* true iff guard has released player */
	struct fakecorridor fakecorr[FCSIZ];
};

struct epri {
	aligntyp shralign;	/* alignment of priest's shrine */
				/* leave as first field to match emin */
	schar shroom;		/* index in rooms */

	coord shrpos;		/* position of shrine */
	d_level shrlevel;	/* level (& dungeon) of shrine */
};

/* note: roaming priests (no shrine) switch from ispriest to isminion
   (and emin extension) */

#define REPAIR_DELAY	5	/* minimum delay between shop damage & repair */

struct bill_x {
	unsigned bo_id;
	boolean useup;
	long price;		/* price per unit */
	long bquan;		/* amount used up */
};

struct eshk {
	long robbed;		/* amount stolen by most recent customer */
	long credit;		/* amount credited to customer */
	long debit;		/* amount of debt for using unpaid items */
	long loan;		/* shop-gold picked (part of debit) */
	int shoptype;		/* the value of rooms[shoproom].rtype */
	schar shoproom;		/* index in rooms; set by inshop() */
	schar unused;		/* to force alignment for stupid compilers */
	boolean following;	/* following customer since he owes us sth */
	boolean surcharge;	/* angry shk inflates prices */
	coord shk;		/* usual position shopkeeper */
	coord shd;		/* position shop door */
	d_level shoplevel;	/* level (& dungeon) of his shop */
	int billct;		/* no. of entries of bill_p[] in use */
	int billsz;		/* no. of entries of bill_p[] allocated */
	struct bill_x *bill_p;
	int visitct;		/* nr of visits by most recent customer */
	char customer[PL_NSIZ]; /* most recent customer */
	char shknam[PL_NSIZ];
	long services; /* Services offered */
#define SHK_ID_BASIC   0x1
#define SHK_ID_PREMIUM 0x2
#define SHK_UNCURSE    0x4
#define SHK_APPRAISE   0x8
#define SHK_SPECIAL_A  0x10
#define SHK_SPECIAL_B  0x20
#define SHK_SPECIAL_C  0x40
};

#define NOTANGRY(mon)	((mon)->mpeaceful)
#define ANGRY(mon)	(!NOTANGRY(mon))

#define SHK_NOMATCH 0 /* Shk !know this class of object       */
#define SHK_MATCH   1 /* Shk is expert                        */
#define SHK_GENERAL 2 /* Shk runs a general store             */

/*
 * FUNCTION shk_class_match
 *
 * Return true if a object class matches the shop type.
 * I.e. shk_class_match(WEAPON_CLASS, shkp)
 *
 * Return:      SHK_MATCH, SHK_NOMATCH, SHK_GENERAL
 */

#define shk_class_match(class, shkp) \
	((shtypes[ESHK(shkp)->shoptype - SHOPBASE].symb == RANDOM_CLASS) ? SHK_GENERAL : \
	 (shtypes[ESHK(shkp)->shoptype - SHOPBASE].symb == class) ? SHK_MATCH : \
	 SHK_NOMATCH)


struct emin {
	aligntyp min_align;	// alignment of minion
	bool renegade;		// hostile co-aligned priest or Angel
};

/*	various types of pet food, the lower, the better liked.	*/

#define DOGFOOD 0
#define CADAVER 1
#define ACCFOOD 2
#define MANFOOD 3
#define APPORT	4
#define POISON	5
#define UNDEF	6
#define TABU	7

struct edog {
	long droptime;		/* moment dog dropped object */
	unsigned dropdist;	/* dist of drpped obj from @ */
	int apport;		/* amount of training */
	long whistletime;	/* last time he whistled */
	long hungrytime;	/* will get hungry at this time */
	coord ogoal;		/* previous goal location */
	int abuse;		/* track abuses to this pet */
	int revivals;		/* count pet deaths */
	int mhpmax_penalty;	/* while starving, points reduced */
	bool killed_by_u;	/* you attempted to kill him */
};


/*** Card definitions ***/
#define CARD_SUITS  4  /* Number of suits */
#define CARD_RANKS  13 /* Number of cards in each suit */
#define CARD_TRUMPS 20 /* Number of trump cards */
#define CARD_SUITED (CARD_SUITS * CARD_RANKS)
#define CARD_FOOL   CARD_SUITED
#define CARD_TOTAL  (CARD_SUITED + CARD_TRUMPS)

#define card_istrump(c) ((c) >= CARD_SUITED)
#define card_suit(c)    ((c) / CARD_RANKS)
#define card_rank(c)    ((c) % CARD_RANKS)
#define card_trump(c)   ((c)-CARD_SUITED)

struct egyp {
	bool cant_grant_wish;
	long credit;             /* Amount credited to player */
	int top;                 /* Index of top of the deck */
	xchar cards[CARD_TOTAL]; /* Shuffled cards */
};

struct mextra {
	char *mname;
	struct egd *egd;
	struct epri *epri;
	struct eshk *eshk;
	struct emin *emin;
	struct edog *edog;
	struct egyp *egyp;
};

#define MNAME(mon)	((mon)->mextra->mname)
#define EGD(mon)	((mon)->mextra->egd)
#define EPRI(mon)	((mon)->mextra->epri)
#define ESHK(mon)	((mon)->mextra->eshk)
#define EMIN(mon)	((mon)->mextra->emin)
#define EDOG(mon)	((mon)->mextra->edog)
#define EGYP(mon)	((mon)->mextra->egyp)
#define has_name(mon)	((mon)->mextra && MNAME(mon))

#endif /* MEXTRA_H */
