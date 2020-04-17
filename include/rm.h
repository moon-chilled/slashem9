/*	SCCS Id: @(#)rm.h	3.4	1999/12/12	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef RM_H
#define RM_H

/*
 * The dungeon presentation graphics code and data structures were rewritten
 * and generalized for NetHack's release 2 by Eric S. Raymond (eric@snark)
 * building on Don G. Kneller's MS-DOS implementation.	See drawing.c for
 * the code that permits the user to set the contents of the symbol structure.
 *
 * The door representation was changed by Ari Huttunen(ahuttune@niksula.hut.fi)
 */

/*
 * TLCORNER	TDWALL		TRCORNER
 * +-		-+-		-+
 * |		 |		 |
 *
 * TRWALL	CROSSWALL	TLWALL		HWALL
 * |		 |		 |
 * +-		-+-		-+		---
 * |		 |		 |
 *
 * BLCORNER	TUWALL		BRCORNER	VWALL
 * |		 |		 |		|
 * +-		-+-		-+		|
 */

/* Level location types */
enum {
	STONE = 0,
	VWALL = 1,
	HWALL = 2,
	TLCORNER = 3,
	TRCORNER = 4,
	BLCORNER = 5,
	BRCORNER = 6,
	CROSSWALL = 7, // For pretty mazes and special levels
	TUWALL = 8,
	TDWALL = 9,
	TLWALL = 10,
	TRWALL = 11,
	DBWALL = 12,
	TREE = 13, // Added by KMH
	SDOOR = 14,
	SCORR = 15,
	POOL = 16,
	MOAT = 17, // pool that doesn't boil, adjust messages
	WATER = 18,
	DRAWBRIDGE_UP = 19,
	LAVAPOOL = 20,
	IRONBARS = 21, // Added by KMH
	DOOR = 22,
	CORR = 23,
	ROOM = 24,
	STAIRS = 25,
	LADDER = 26,
	FOUNTAIN = 27,
	THRONE = 28,
	SINK = 29,
	TOILET = 30,
	GRAVE = 31,
	ALTAR = 32,
	ICE = 33,
	DRAWBRIDGE_DOWN = 34,
	AIR = 35,
	CLOUD = 36,

	MAX_TYPE = 37,
	INVALID_TYPE = 127,
};

/*
 * Avoid using the level types in inequalities:
 * these types are subject to change.
 * Instead, use one of the macros below.
 */
#define IS_WALL(typ)   ((typ) && (typ) <= DBWALL)
#define IS_STWALL(typ) ((typ) <= DBWALL) /* STONE <= (typ) <= DBWALL */
#define IS_ROCK(typ)   ((typ) < POOL)	 /* absolutely nonaccessible */
#define IS_DOOR(typ)   ((typ) == DOOR)
#define IS_TREE(typ)   ((typ) == TREE || \
		      (level.flags.arboreal && (typ) == STONE))
#define ACCESSIBLE(typ)	   ((typ) >= DOOR) /* good position */
#define IS_ROOM(typ)	   ((typ) >= ROOM) /* ROOM, STAIRS, furniture.. */
#define ZAP_POS(typ)	   ((typ) >= POOL)
#define IS_GRAVE(typ)	   ((typ) == GRAVE)
#define SPACE_POS(typ)	   ((typ) > DOOR)
#define IS_POOL(typ)	   ((typ) >= POOL && (typ) <= DRAWBRIDGE_UP)
#define IS_THRONE(typ)	   ((typ) == THRONE)
#define IS_FOUNTAIN(typ)   ((typ) == FOUNTAIN)
#define IS_SINK(typ)	   ((typ) == SINK)
#define IS_TOILET(typ)	   ((typ) == TOILET)
#define IS_GRAVE(typ)	   ((typ) == GRAVE)
#define IS_ALTAR(typ)	   ((typ) == ALTAR)
#define IS_DRAWBRIDGE(typ) ((typ) == DRAWBRIDGE_UP || (typ) == DRAWBRIDGE_DOWN)
#define IS_FURNITURE(typ)  ((typ) >= STAIRS && (typ) <= ALTAR)
#define IS_AIR(typ)	   ((typ) == AIR || (typ) == CLOUD)
#define IS_SOFT(typ)	   ((typ) == AIR || (typ) == CLOUD || IS_POOL(typ))

/*
 * The screen symbols may be the default or defined at game startup time.
 * See drawing.c for defaults.
 * Note: {ibm|dec}_graphics[] arrays (also in drawing.c) must be kept in synch.
 */

/* begin dungeon characters */

typedef enum {
	S_stone = 0,
	S_vwall = 1,
	S_hwall = 2,
	S_tlcorn = 3,
	S_trcorn = 4,
	S_blcorn = 5,
	S_brcorn = 6,
	S_crwall = 7,
	S_tuwall = 8,
	S_tdwall = 9,
	S_tlwall = 10,
	S_trwall = 11,
	S_ndoor = 12,
	S_vodoor = 13,
	S_hodoor = 14,
	S_vcdoor = 15,	// closed door, vertical wall
	S_hcdoor = 16,	// closed door, horizontal wall
	S_bars = 17,	// Added by KMH
	S_tree = 18,	// Added by KMH
	S_room = 19,
	S_darkroom = 20,
	S_corr = 21,
	S_litcorr = 22,
	S_upstair = 23,
	S_dnstair = 24,
	S_upladder = 25,
	S_dnladder = 26,
	S_altar = 27,
	S_grave = 28,
	S_throne = 29,
	S_sink = 30,
	S_toilet = 31,
	S_fountain = 32,
	S_pool = 33,
	S_ice = 34,
	S_lava = 35,
	S_vodbridge = 36,
	S_hodbridge = 37,
	S_vcdbridge = 38, /* closed drawbridge, vertical wall */
	S_hcdbridge = 39, /* closed drawbridge, horizontal wall */
	S_air = 40,
	S_cloud = 41,
	S_water = 42,

	// end dungeon characters, begin traps

	S_arrow_trap = 43,
	S_dart_trap = 44,
	S_falling_rock_trap = 45,
	S_squeaky_board = 46,
	S_bear_trap = 47,
	S_land_mine = 48,
	S_rolling_boulder_trap = 49,
	S_sleeping_gas_trap = 50,
	S_rust_trap = 51,
	S_fire_trap = 52,
	S_pit = 53,
	S_spiked_pit = 54,
	S_hole = 55,
	S_trap_door = 56,
	S_teleportation_trap = 57,
	S_level_teleporter = 58,
	S_magic_portal = 59,
	S_web = 60,
	S_statue_trap = 61,
	S_magic_trap = 62,
	S_anti_magic_trap = 63,
	S_polymorph_trap = 64,

	// end traps, begin special effects

	S_vbeam = 65,	// The 4 zap beam symbols.  Do NOT separate.
	S_hbeam = 66,	// To change order or add, see function
	S_lslant = 67,	// zapdir_to_glyph() in display.c.
	S_rslant = 68,
	S_digbeam = 69,	// dig beam symbol
	S_flashbeam =70,// camera flash symbol
	S_boomleft = 71,// thrown boomerang, open left, e.g ')'
	S_boomright =72,// thrown boomerand, open right, e.g. '('
	S_ss1 = 73,	// 4 magic shield glyphs
	S_ss2 = 74,
	S_ss3 = 75,
	S_ss4 = 76,

	// The 8 swallow symbols.  Do NOT separate.  To change order or add, see
	// the function swallow_to_glyph() in display.c.
	S_sw_tl = 77, // swallow top left	[1]
	S_sw_tc = 78, // swallow top center	[2]	Order:
	S_sw_tr = 79, // swallow top right	[3]
	S_sw_ml = 80, // swallow middle left	[4]	1 2 3
	S_sw_mr = 81, // swallow middle right	[6]	4 5 6
	S_sw_bl = 82, // swallow bottom left	[7]	7 8 9
	S_sw_bc = 83, // swallow bottom center	[8]
	S_sw_br = 84, // swallow bottom right	[9]

	S_explode1 = 85, /* explosion top left			*/
	S_explode2 = 86, /* explosion top center		*/
	S_explode3 = 87, /* explosion top right		 Ex.	*/
	S_explode4 = 88, /* explosion middle left		*/
	S_explode5 = 89, /* explosion middle center	 /-\	*/
	S_explode6 = 90, /* explosion middle right	 |@|	*/
	S_explode7 = 91, /* explosion bottom left	 \-/	*/
	S_explode8 = 92, /* explosion bottom center		*/
	S_explode9 = 93, /* explosion bottom right		*/

	// end effects, begin warning characters

	S_warn0 = 94,
	S_warn1 = 95,
	S_warn2 = 96,
	S_warn3 = 97,
	S_warn4 = 98,
	S_warn5 = 99
	// end warning characters
} DungeonSym;

#define MAXPCHARS   100 /* maximum number of mapped characters */
#define MAXDCHARS   43	/* maximum of mapped dungeon characters */
#define MAXTCHARS   22	/* maximum of mapped trap characters */
#define MAXECHARS   29	/* maximum of mapped effects characters */
#define MAXEXPCHARS 9	/* number of explosion characters */
#define MAXWARNINGS 6	/* number of warning characters */

struct symdef {
	const char *explanation;
	uchar color;
};

extern const struct symdef sym_desc[MAXPCHARS];
extern glyph_t showsyms[MAXPCHARS];
extern uchar showsymcolors[MAXPCHARS];

/*
 * Graphics sets for display symbols
 */
#define ASCII_GRAPHICS	    0  // regular characters: '-', '+', &c
#define UTF8_GRAPHICS	    1  // UTF-8 characters
#define UTF8COMPAT_GRAPHICS 2  // UTF-8 characters compatible with WGL4

/*
 * The 5 possible states of doors
 */

#define D_NODOOR  0
#define D_BROKEN  1
#define D_ISOPEN  2
#define D_CLOSED  4
#define D_LOCKED  8
#define D_TRAPPED 16

/*
 * Some altars are considered as shrines, so we need a flag.
 */
#define AM_SHRINE 8

/*
 * Thrones should only be looted once.
 */
#define T_LOOTED 1

/*
 * Trees have more than one kick result.
 */
#define TREE_LOOTED 1
#define TREE_SWARM  2

/*
 * Fountains have limits, and special warnings.
 */
#define F_LOOTED		    1
#define F_WARNED		    2
#define FOUNTAIN_IS_WARNED(x, y)    (levl[x][y].looted & F_WARNED)
#define FOUNTAIN_IS_LOOTED(x, y)    (levl[x][y].looted & F_LOOTED)
#define SET_FOUNTAIN_WARNED(x, y)   levl[x][y].looted |= F_WARNED;
#define SET_FOUNTAIN_LOOTED(x, y)   levl[x][y].looted |= F_LOOTED;
#define CLEAR_FOUNTAIN_WARNED(x, y) levl[x][y].looted &= ~F_WARNED;
#define CLEAR_FOUNTAIN_LOOTED(x, y) levl[x][y].looted &= ~F_LOOTED;

/*
 * Doors are even worse :-) The special warning has a side effect
 * of instantly trapping the door, and if it was defined as trapped,
 * the guards consider that you have already been warned!
 */
#define D_WARNED 16

/*
 * Sinks have 3 different types of loot that shouldn't be abused
 */
#define S_LPUDDING 1
#define S_LDWASHER 2
#define S_LRING	   4

/*
 * The four directions for a DrawBridge.
 */
#define DB_NORTH 0
#define DB_SOUTH 1
#define DB_EAST	 2
#define DB_WEST	 3
#define DB_DIR	 3 /* mask for direction */

/*
 * What's under a drawbridge.
 */
#define DB_MOAT	 0
#define DB_LAVA	 4
#define DB_ICE	 8
#define DB_FLOOR 16
#define DB_UNDER 28 /* mask for underneath */

/*
 * Wall information.
 */
#define WM_MASK	      0x07 /* wall mode (bottom three bits) */
#define W_NONDIGGABLE 0x08
#define W_NONPASSWALL 0x10

/*
 * Ladders (in Vlad's tower) may be up or down.
 */
#define LA_UP	1
#define LA_DOWN 2

/*
 * Room areas may be iced pools
 */
#define ICED_POOL 8
#define ICED_MOAT 16

/*
 * The structure describing a coordinate position.
 * Before adding fields, remember that this will significantly affect
 * the size of temporary files and save files.
 */
struct rm {
	Bitfield(mem_bg, 6);	// Remembered background
	Bitfield(mem_trap, 5);	// Remembered trap
	Bitfield(mem_obj, 10);	// Remembered object/corpse
	bool mem_corpse;	// Set if mem_obj refers to a corpse
	bool mem_invis;		// Set if invisible monster remembered
	Bitfield(mem_spare, 9);
	schar typ;		// what is really there
	Bitfield(styp, 6);	// last seen/touched dungeon 
	uchar seenv;		// seen vector
	Bitfield(flags, 5); 	// extra information for typ
	bool horizontal;	// wall/door/etc is horiz. (more typ info)
	bool lit;		// speed hack for lit rooms
	bool waslit;		// remember if a location was lit
	Bitfield(roomno, 6);	// room # for special rooms
	bool edge;		// marks boundaries for special rooms
	bool candig;		// Exception to Can_dig_down; was a trapdoor
};

/*
 * Add wall angle viewing by defining "modes" for each wall type.  Each
 * mode describes which parts of a wall are finished (seen as as wall)
 * and which are unfinished (seen as rock).
 *
 * We use the bottom 3 bits of the flags field for the mode.  This comes
 * in conflict with secret doors, but we avoid problems because until
 * a secret door becomes discovered, we know what sdoor's bottom three
 * bits are.
 *
 * The following should cover all of the cases.
 *
 *	type	mode				Examples: R=rock, F=finished
 *	-----	----				----------------------------
 *	WALL:	0 none				hwall, mode 1
 *		1 left/top (1/2 rock)			RRR
 *		2 right/bottom (1/2 rock)		---
 *							FFF
 *
 *	CORNER: 0 none				trcorn, mode 2
 *		1 outer (3/4 rock)			FFF
 *		2 inner (1/4 rock)			F+-
 *							F|R
 *
 *	TWALL:	0 none				tlwall, mode 3
 *		1 long edge (1/2 rock)			F|F
 *		2 bottom left (on a tdwall)		-+F
 *		3 bottom right (on a tdwall)		R|F
 *
 *	CRWALL: 0 none				crwall, mode 5
 *		1 top left (1/4 rock)			R|F
 *		2 top right (1/4 rock)			-+-
 *		3 bottom left (1/4 rock)		F|R
 *		4 bottom right (1/4 rock)
 *		5 top left & bottom right (1/2 rock)
 *		6 bottom left & top right (1/2 rock)
 */

#define WM_W_LEFT   1 /* vertical or horizontal wall */
#define WM_W_RIGHT  2
#define WM_W_TOP    WM_W_LEFT
#define WM_W_BOTTOM WM_W_RIGHT

#define WM_C_OUTER 1 /* corner wall */
#define WM_C_INNER 2
#define WM_T_LONG  1 /* T wall */
#define WM_T_BL	   2
#define WM_T_BR	   3

#define WM_X_TL	  1 /* cross wall */
#define WM_X_TR	  2
#define WM_X_BL	  3
#define WM_X_BR	  4
#define WM_X_TLBR 5
#define WM_X_BLTR 6

/*
 * Seen vector values.	The seen vector is an array of 8 bits, one for each
 * octant around a given center x:
 *
 *			0 1 2
 *			7 x 3
 *			6 5 4
 *
 * In the case of walls, a single wall square can be viewed from 8 possible
 * directions.	If we know the type of wall and the directions from which
 * it has been seen, then we can determine what it looks like to the hero.
 */
#define SV0   0x1
#define SV1   0x2
#define SV2   0x4
#define SV3   0x8
#define SV4   0x10
#define SV5   0x20
#define SV6   0x40
#define SV7   0x80
#define SVALL 0xFF

#define doormask       flags
#define altarmask      flags
#define wall_info      flags
#define ladder	       flags
#define drawbridgemask flags
#define looted	       flags
#define icedpool       flags

#define blessedftn horizontal /* a fountain that grants attribs */
#define disturbed  horizontal /* a grave that has been disturbed */

struct damage {
	struct damage *next;
	long when, cost;
	coord place;
	schar typ;
};

struct levelflags {
	uchar nfountains; /* number of fountains on level */
	uchar nsinks;	  /* number of sinks + toilets on the level */
	/* Several flags that give hints about what's on the level */
	bool has_shop;
	bool has_vault;
	bool has_zoo;
	bool has_court;
	bool has_morgue;
	bool has_beehive;
	bool has_barracks;
	bool has_temple;
	bool has_lemurepit;
	bool has_migohive;
	bool has_fungusfarm;

	bool has_swamp;
	bool noteleport;
	bool hardfloor;
	bool nommap;
	bool hero_memory;  /* hero has memory */
	bool shortsighted; /* monsters are shortsighted */
	bool graveyard;	   /* has_morgue, but remains set */
	bool is_maze_lev;

	bool is_cavernous_lev;
	bool arboreal; /* Trees replace rock */
	bool spooky;   /* Spooky sounds (Tina Hall) */
	bool lethe;    /* All water on level causes amnesia */
};

typedef struct {
	struct rm locations[COLNO][ROWNO];
	struct obj *objects[COLNO][ROWNO];
	struct monst *monsters[COLNO][ROWNO];
	struct obj *objlist;
	struct obj *buriedobjlist;
	struct monst *monlist;
	struct damage *damagelist;
	struct levelflags flags;
} dlevel_t;

extern dlevel_t level; /* structure describing the current level */

/*
 * Macros for compatibility with old code. Someday these will go away.
 */
#define levl level.locations
#define fobj level.objlist
#define fmon level.monlist

/*
 * Covert a trap number into the defsym graphics array.
 * Convert a defsym number into a trap number.
 * Assumes that arrow trap will always be the first trap.
 */
#define trap_to_defsym(t) (S_arrow_trap + (t)-1)
#define defsym_to_trap(d) ((d)-S_arrow_trap + 1)

#define OBJ_AT(x, y) (level.objects[x][y] != NULL)
/*
 * Macros for encapsulation of level.monsters references.
 */
#define MON_AT(x, y) (level.monsters[x][y] != NULL && \
		      !(level.monsters[x][y])->mburied)
#define MON_BURIED_AT(x, y) (level.monsters[x][y] != NULL && \
			     (level.monsters[x][y])->mburied)
#define place_worm_seg(m, x, y) level.monsters[x][y] = m
#define remove_monster(x, y)	level.monsters[x][y] = NULL
#define m_at(x, y)		(MON_AT(x, y) ? level.monsters[x][y] : \
				   NULL)
#define m_buried_at(x, y) (MON_BURIED_AT(x, y) ? level.monsters[x][y] : \
						 NULL)

#endif /* RM_H */
