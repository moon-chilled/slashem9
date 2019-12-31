/*	SCCS Id: @(#)context.h	3.4	$DATE$	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */


/* If you change the context structure make sure you increment EDITLEVEL in   */
/* patchlevel.h if needed.                                                    */

#ifndef CONTEXT_H
#define CONTEXT_H

#define CONTEXTVERBSZ 20

/*
 * The context structure houses things that the game tracks
 * or adjusts during the game, to preserve game state or context.
 *
 * The entire structure is saved with the game.
 *
 */

struct dig_info {		/* apply.c, hack.c */
	int	effort;
	d_level level;
	coord	pos;
	long lastdigtime;
	bool down, chew, warned, quiet;
};

struct tin_info {
	struct	obj *tin;
	unsigned o_id;		/* o_id of tin in save file */
	int	usedtime, reqtime;
};

struct book_info {
	struct obj *book;	// last/current book being xscribed
	unsigned o_id;		// o_id of book in save file
	int delay;		// moves left for this spell
	int end_delay;		// when to stop studying
};

struct takeoff_info {
	long mask;
	long what;
	int delay;
	bool cancelled_don;
	char disrobing[CONTEXTVERBSZ+1];
};

struct victual_info {
	struct	obj *piece;	/* the thing being eaten, or last thing that
				 * was partially eaten, unless that thing was
				 * a tin, which uses the tin structure above,
				 * in which case this should be 0 */
	unsigned o_id;		/* o_id of food object in save file */
	/* doeat() initializes these when piece is valid */
	int	usedtime,	/* turns spent eating */
		reqtime;	/* turns required to eat */
	int	nmod;		/* coded nutrition per turn */
	bool canchoke;	/* was satiated at beginning */

	/* start_eating() initializes these */
	bool fullwarn;	/* have warned about being full */
	bool eating;	/* victual currently being eaten */
	bool doreset;	/* stop eating at end of turn */
};

struct warntype_info {
	unsigned long obj;	/* object warn_of_mon monster type M2 */
	unsigned long polyd;	/* warn_of_mon monster type M2 due to poly */
	unsigned long intrins;	/* intrinsic warn_of_mon monster type M2 */
	struct permonst *species;	/* particular species due to poly */
	short speciesidx;	/* index of above in mons[] (for save/restore) */
};

struct context_info {
	unsigned ident;		/* social security number for each monster */
	unsigned no_of_wizards; /* 0, 1 or 2 (wizard and his shadow) */
	unsigned run;		/* 0: h (etc), 1: H (etc), 2: fh (etc) */
				/* 3: FH, 4: ff+, 5: ff-, 6: FF+, 7: FF- */
				/* 8: travel */
	int	warnlevel;
	int	djinni_count, ghost_count;	/* potion effect tuning */
	long next_attrib_check;
	long stethoscope_move;
	short stethoscope_movement;
	bool travel;	/* find way automatically to u.tx,u.ty */
	bool travel1;	/* first travel step */
	bool forcefight;
	bool nopick;	/* do not pickup objects (as when running) */
	bool made_amulet;
	bool mon_moving;	/* monsters' turn to move */
	bool move;
	bool mv;
	bool bypasses;	/* bypass flag is set on at least one fobj */
	bool botl;		/* partially redo status line */
	bool botlx;		/* print an entirely new bottom line */
	struct dig_info digging;
	struct victual_info victual;
	struct tin_info tin;
	struct book_info spbook;
	struct takeoff_info takeoff;
	struct warntype_info warntype;
};

extern struct context_info context;

#endif /* CONTEXT_H */
