/*	SCCS Id: @(#)mapglyph.c	3.4	2003/01/08	*/
/* Copyright (c) David Cohrs, 1991				  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#if defined(TTY_GRAPHICS)
#include "wintty.h"	/* for prototype of has_color() only */
#endif
#include "color.h"
#define HI_DOMESTIC CLR_WHITE	/* monst.c */

int explcolors[] = {
	CLR_BLACK,	/* dark    */
	CLR_GREEN,	/* noxious */
	CLR_BROWN,	/* muddy   */
	CLR_BLUE,	/* wet     */
	CLR_MAGENTA,	/* magical */
	CLR_ORANGE,	/* fiery   */
	CLR_WHITE,	/* frosty  */
};

#if !defined(TTY_GRAPHICS)
#define has_color(n)  true
#endif

#define zap_color(n)  color = iflags.use_color ? zapcolors[n] : NO_COLOR
#ifndef USER_DUNGEONCOLOR
#define cmap_color(n) color = iflags.use_color ? sym_desc[n].color : NO_COLOR
#else
#define cmap_color(n) color = iflags.use_color ? showsymcolors[n] : NO_COLOR
#endif
#define obj_color(n)  color = iflags.use_color ? objects[n].oc_color : NO_COLOR
#define mon_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define invis_color(n) color = NO_COLOR
#define pet_color(n)  color = iflags.use_color ? mons[n].mcolor : NO_COLOR
#define warn_color(n) color = iflags.use_color ? sym_desc[S_warn0 + n].color : NO_COLOR
#define explode_color(n) color = iflags.use_color ? explcolors[n] : NO_COLOR

/** Returns the correct monster glyph.
 *  Returns a Unicode codepoint in UTF8graphics and an ASCII character otherwise. */
static glyph_t get_monsym(int glyph) {
	if (permonst_unicode_codepoint[glyph]) {
		/* only return a Unicode codepoint when there is one configured */
		return permonst_unicode_codepoint[glyph];
	} else {
		return monsyms[mons[glyph].mlet];
	}
}

/** Returns the correct object glyph.
 *  Returns a Unicode codepoint in UTF8graphics and an ASCII character otherwise. */
static glyph_t get_objsym(int glyph) {
	if (objclass_unicode_codepoint[glyph]) {
		/* only return a Unicode codepoint when there is one configured */
		return objclass_unicode_codepoint[glyph];
	} else {
		return oc_syms[objects[glyph].oc_class];
	}
}


void mapglyph(int glyph, glyph_t *ochar, int *ocolor, unsigned *ospecial, int x, int y) {
	int offset;
	int color = NO_COLOR;
	glyph_t ch;
	unsigned special = 0;

	/*
	 *  Map the glyph back to a character and color.
	 *
	 *  Warning:  For speed, this makes an assumption on the order of
	 *		  offsets.  The order is set in display.h.
	 */
	if ((offset = (glyph - GLYPH_WARNING_OFF)) >= 0) {	/* a warning flash */
		ch = showsyms[S_warn0 + offset];
		if (Is_rogue_level(&u.uz))
			color = NO_COLOR;
		else
			warn_color(offset);
	} else if ((offset = (glyph - GLYPH_SWALLOW_OFF)) >= 0) {	/* swallow */
		/* see swallow_to_glyph() in display.c */
		ch = showsyms[S_sw_tl + (offset & 0x7)];
		if (Is_rogue_level(&u.uz) && iflags.use_color)
			color = NO_COLOR;
		else
			mon_color(offset >> 3);
	} else if ((offset = (glyph - GLYPH_ZAP_OFF)) >= 0) {	/* zap beam */
		/* see zapdir_to_glyph() in display.c */
		ch = showsyms[S_vbeam + (offset & 0x3)];
		if (Is_rogue_level(&u.uz) && iflags.use_color)
			color = NO_COLOR;
		else
			zap_color((offset >> 2));
	} else if ((offset = (glyph - GLYPH_EXPLODE_OFF)) >= 0) {	/* explosion */
		ch = showsyms[(offset % MAXEXPCHARS) + S_explode1];
		explode_color(offset / MAXEXPCHARS);
	} else if ((offset = (glyph - GLYPH_CMAP_OFF)) >= 0) {	/* cmap */
		ch = showsyms[offset];
		if (Is_rogue_level(&u.uz) && iflags.use_color) {
			if (offset >= S_vwall && offset <= S_hcdoor)
				color = CLR_BROWN;
			else if (offset >= S_arrow_trap && offset <= S_polymorph_trap)
				color = CLR_MAGENTA;
			else if (offset == S_corr || offset == S_litcorr)
				color = CLR_GRAY;
			else if (offset >= S_room && offset <= S_water && offset != S_darkroom)
				color = CLR_GREEN;
			else
				color = NO_COLOR;
		} else
			/* provide a visible difference if normal and lit corridor
			 * use the same symbol */
			if (iflags.use_color &&
#ifndef USER_DUNGEONCOLOR
			                offset == S_litcorr && ch == showsyms[S_corr])
				color = CLR_WHITE;
#else
			                offset == S_litcorr && ch == showsyms[S_corr] &&
			                showsymcolors[S_corr] == showsymcolors[S_litcorr]) {
				if (showsymcolors[S_corr] != CLR_WHITE) {
					color = showsymcolors[S_litcorr] = CLR_WHITE;
				} else {
					color = showsymcolors[S_litcorr] = CLR_GRAY;
				}
			}
#endif /* USER_DUNGEONCOLOR */
			else
				cmap_color(offset);
	} else if ((offset = (glyph - GLYPH_OBJ_OFF)) >= 0) {	/* object */
		if (On_stairs(x,y) && levl[x][y].seenv) special |= MG_STAIRS;

		ch = get_objsym(offset);

		if (Is_rogue_level(&u.uz) && iflags.use_color) {
			switch(objects[offset].oc_class) {
			case COIN_CLASS:
				color = CLR_YELLOW;
				break;
			case FOOD_CLASS:
				color = CLR_RED;
				break;
			default:
				color = CLR_BRIGHT_BLUE;
				break;
			}
		} else {
			obj_color(offset);
		}

		if (offset != BOULDER &&
		                level.objects[x][y] &&
		                level.objects[x][y]->nexthere) {
			special |= MG_OBJPILE;
		}
	} else if ((offset = (glyph - GLYPH_RIDDEN_OFF)) >= 0) {	/* mon ridden */
		ch = get_monsym(offset);
		if (Is_rogue_level(&u.uz)) {
			/* This currently implies that the hero is here -- monsters */
			/* don't ride (yet...).  Should we set it to yellow like in */
			/* the monster case below?  There is no equivalent in rogue. */
			color = NO_COLOR;	/* no need to check iflags.use_color */
		} else {
			mon_color(offset);
		}
		special |= MG_RIDDEN;
	} else if ((offset = (glyph - GLYPH_BODY_OFF)) >= 0) {	/* a corpse */
		if (On_stairs(x,y) && levl[x][y].seenv) special |= MG_STAIRS;
		ch = get_objsym(CORPSE);
		if (Is_rogue_level(&u.uz) && iflags.use_color)
			color = CLR_RED;
		else
			mon_color(offset);

		special |= MG_CORPSE;
		if (offset != BOULDER &&
		                level.objects[x][y] &&
		                level.objects[x][y]->nexthere) {
			special |= MG_OBJPILE;
		}
	} else if ((offset = (glyph - GLYPH_DETECT_OFF)) >= 0) {	/* mon detect */
		ch = get_monsym(offset);
		if (Is_rogue_level(&u.uz))
			color = NO_COLOR;	/* no need to check iflags.use_color */
		else
			mon_color(offset);
		/* Disabled for now; anyone want to get reverse video to work? */
		/* is_reverse = true; */
		special |= MG_DETECT;
	} else if ((offset = (glyph - GLYPH_INVIS_OFF)) >= 0) {	/* invisible */
		ch = DEF_INVISIBLE;

		if (Is_rogue_level(&u.uz))
			color = NO_COLOR;	/* no need to check iflags.use_color */
		else
			invis_color(offset);

		special |= MG_INVIS;
	} else if ((offset = (glyph - GLYPH_PET_OFF)) >= 0) {	/* a pet */
		ch = get_monsym(offset);
		if (Is_rogue_level(&u.uz))
			color = NO_COLOR;	/* no need to check iflags.use_color */
		else
			pet_color(offset);
		special |= MG_PET;
	} else {							/* a monster */
		ch = get_monsym(glyph);
		if (Is_rogue_level(&u.uz) && iflags.use_color) {
			if (x == u.ux && y == u.uy)
				/* actually player should be yellow-on-gray if in a corridor */
				color = CLR_YELLOW;
			else
				color = NO_COLOR;
		} else {
			mon_color(glyph);
			/* special case the hero for `showrace' option */
			if (iflags.use_color && x == u.ux && y == u.uy &&
			                iflags.showrace && !Upolyd)
				color = HI_DOMESTIC;
		}
	}

	/* Turn off color if no color defined, or rogue level w/o fancy graphics. */
	if (!has_color(color) || (Is_rogue_level(&u.uz) && (iflags.graphics == ASCII_GRAPHICS)))
		color = NO_COLOR;

	*ochar = ch;
	*ospecial = special;
	*ocolor = color;
	return;
}

/*mapglyph.c*/
