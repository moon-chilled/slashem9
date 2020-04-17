/*	SCCS Id: @(#)drawing.c	3.4	1999/12/02	*/
/* Copyright (c) NetHack Development Team 1992.			  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "tcap.h"
/* Relevent header information in rm.h and objclass.h. */

#ifdef C
#undef C
#endif

#define C(n) n

glyph_t oc_syms[MAXOCLASSES] = {0};	/* the current object  display symbols */
glyph_t showsyms[MAXPCHARS] = {0};	/* the current feature display symbols */
uchar monsyms[MAXMCLASSES] = {0};	/* the current monster display symbols */
uchar showsymcolors[MAXPCHARS] = {0}; /* current feature display colors */

/* Default object class symbols.  See objclass.h. */
const glyph_t def_oc_syms[MAXOCLASSES] = {
/* 0*/	'\0', /* placeholder for the "random class" */
	ILLOBJ_SYM,
	WEAPON_SYM,
	ARMOR_SYM,
	RING_SYM,
/* 5*/	AMULET_SYM,
	TOOL_SYM,
	FOOD_SYM,
	POTION_SYM,
	SCROLL_SYM,
/*10*/	SPBOOK_SYM,
	WAND_SYM,
	GOLD_SYM,
	GEM_SYM,
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM};

const char invisexplain[] = "remembered, unseen, creature";

/* Object descriptions.  Used in do_look(). */
const char *const objexplain[] = {/* these match def_oc_syms, above */
/* 0*/	NULL,
	"strange object",
	"weapon",
	"suit or piece of armor",
	"ring",
/* 5*/	"amulet",
	"useful item (pick-axe, key, lamp...)",
	"piece of food",
	"potion",
	"scroll",
/*10*/	"spell book",
	"wand",
	"pile of coins",
	"gem or rock",
	"boulder or statue",
/*15*/	"iron ball",
	"iron chain",
	"splash of venom"};

/* Object class names.  Used in object_detect(). */
const char *const oclass_names[] = {
/* 0*/	NULL,
	"illegal objects",
	"weapons",
	"armor",
	"rings",
/* 5*/	"amulets",
	"tools",
	"food",
	"potions",
	"scrolls",
/*10*/	"spell books",
	"wands",
	"coins",
	"rocks",
	"large stones",
/*15*/	"iron balls",
	"chains",
	"venoms"};

/* Default monster class symbols.  See monsym.h. */
const char def_monsyms[MAXMCLASSES] = {
	'\0', /* holder */
	DEF_ANT,
	DEF_BLOB,
	DEF_COCKATRICE,
	DEF_DOG,
	DEF_EYE,
	DEF_FELINE,
	DEF_GREMLIN,
	DEF_HUMANOID,
	DEF_IMP,
	DEF_JELLY, /* 10 */
	DEF_KOBOLD,
	DEF_LEPRECHAUN,
	DEF_MIMIC,
	DEF_NYMPH,
	DEF_ORC,
	DEF_PIERCER,
	DEF_QUADRUPED,
	DEF_RODENT,
	DEF_SPIDER,
	DEF_TRAPPER, /* 20 */
	DEF_UNICORN,
	DEF_VORTEX,
	DEF_WORM,
	DEF_XAN,
	DEF_LIGHT,
	DEF_ZRUTY,
	DEF_ANGEL,
	DEF_BAT,
	DEF_CENTAUR,
	DEF_DRAGON, /* 30 */
	DEF_ELEMENTAL,
	DEF_FUNGUS,
	DEF_GNOME,
	DEF_GIANT,
	'\0',
	DEF_JABBERWOCK,
	DEF_KOP,
	DEF_LICH,
	DEF_MUMMY,
	DEF_NAGA, /* 40 */
	DEF_OGRE,
	DEF_PUDDING,
	DEF_QUANTMECH,
	DEF_RUSTMONST,
	DEF_SNAKE,
	DEF_TROLL,
	DEF_UMBER,
	DEF_VAMPIRE,
	DEF_WRAITH,
	DEF_XORN, /* 50 */
	DEF_YETI,
	DEF_ZOMBIE,
	DEF_HUMAN,
	DEF_GHOST,
	DEF_GOLEM,
	DEF_DEMON,
	DEF_EEL,
	DEF_LIZARD,
	DEF_BAD_FOOD,
	DEF_BAD_COINS, /* 60 */
	DEF_WORM_TAIL,
	DEF_MIMIC_DEF,
};

/* The explanations below are also used when the user gives a string
 * for blessed genocide, so no text should wholly contain any later
 * text.  They should also always contain obvious names (eg. cat/feline).
 */
/* KMH -- changed u and z */
/* Robin Johnson -  changed Q */
const char *const monexplain[MAXMCLASSES] = {
	0,
	"ant or other insect", "blob", "cockatrice",
	"dog or other canine", "eye or sphere", "cat or other feline",
	"gremlin", "humanoid", "imp or minor demon",
	"jelly", "kobold", "leprechaun",
	"mimic", "nymph", "orc",
	"piercer", "quadruped", "rodent",
	"arachnid or centipede", "trapper or lurker above", "unicorn or horse",
	"vortex", "worm", "xan or other mythical/fantastic insect",
	"light", "Zouthern animal",
#if 0
	"light",			"zruty",
#endif
	"angelic being", "bat or bird", "centaur",
	"dragon", "elemental", "fungus or mold",
	"gnome", "giant humanoid", 0,
	"jabberwock", "Keystone Kop", "lich",
	"mummy", "naga", "ogre",
	"pudding or ooze", "quantum mechanic or other scientist",
	"rust monster or disenchanter",
	"snake", "troll", "umber hulk",
	"vampire", "wraith", "xorn",
	"apelike creature", "zombie",

	"human or elf", "ghost", "golem",
	"major demon", "sea monster", "lizard",
	"piece of food", "pile of coins",
	"long worm tail", "mimic"};

/*
 *  Default screen symbol explanations and colors.
 *  Note: {ascii|utf8}_graphics[] arrays also depend on this symbol order.
 */
const struct symdef sym_desc[MAXPCHARS] = {
/* 0*/	{"unexplored area", C(NO_COLOR)},	/* stone */
	{"wall", C(CLR_GRAY)},			/* vwall */
	{"wall", C(CLR_GRAY)},			/* hwall */
	{"wall", C(CLR_GRAY)},			/* tlcorn */
	{"wall", C(CLR_GRAY)},			/* trcorn */
	{"wall", C(CLR_GRAY)},			/* blcorn */
	{"wall", C(CLR_GRAY)},			/* brcorn */
	{"wall", C(CLR_GRAY)},			/* crwall */
	{"wall", C(CLR_GRAY)},			/* tuwall */
	{"wall", C(CLR_GRAY)},			/* tdwall */
/*10*/	{"wall", C(CLR_GRAY)},			/* tlwall */
	{"wall", C(CLR_GRAY)},			/* trwall */
	{"doorway", C(CLR_GRAY)},		/* ndoor */
	{"open door", C(CLR_BROWN)},		/* vodoor */
	{"open door", C(CLR_BROWN)},		/* hodoor */
	{"closed door", C(CLR_BROWN)},		/* vcdoor */
	{"closed door", C(CLR_BROWN)},		/* hcdoor */
	{"iron bars", C(HI_METAL)},		/* bars */
	{"tree", C(CLR_GREEN)},			/* tree */
	{"floor of a room", C(CLR_GRAY)},	/* room */
/*20*/	{"dark part of a room", C(CLR_BLACK)},	/* dark room */
	{"corridor", C(CLR_BLACK)},		/* dark corr */
	{"lit corridor", C(CLR_GRAY)},		/* lit corr (see mapglyph.c) */
	{"staircase up", C(CLR_WHITE)},		/* upstair */
	{"staircase down", C(CLR_WHITE)},	/* dnstair */
	{"ladder up", C(CLR_BROWN)},		/* upladder */
	{"ladder down", C(CLR_BROWN)},		/* dnladder */
	{"altar", C(CLR_GRAY)},			/* altar */
	{"grave", C(CLR_GRAY)},			/* grave */
	{"opulent throne", C(HI_GOLD)},		/* throne */
/*30*/	{"sink", C(CLR_GRAY)},			/* sink */
	{"toilet", C(CLR_WHITE)},		/* toilet */
	{"fountain", C(CLR_BLUE)},		/* fountain */
	{"water", C(CLR_BLUE)},			/* pool */
	{"ice", C(CLR_CYAN)},			/* ice */
	{"molten lava", C(CLR_RED)},		/* lava */
	{"lowered drawbridge", C(CLR_BROWN)},	/* vodbridge */
	{"lowered drawbridge", C(CLR_BROWN)},	/* hodbridge */
	{"raised drawbridge", C(CLR_BROWN)},	/* vcdbridge */
	{"raised drawbridge", C(CLR_BROWN)},	/* hcdbridge */
/*40*/	{"air", C(CLR_CYAN)},			/* open air */
	{"cloud", C(CLR_GRAY)},			/* [part of] a cloud */
	{"water", C(CLR_BLUE)},			/* under water */
	{"arrow trap", C(HI_METAL)},		/* trap */
	{"dart trap", C(HI_METAL)},		/* trap */
	{"falling rock trap", C(CLR_GRAY)},	/* trap */
	{"squeaky board", C(CLR_BROWN)},	/* trap */
	{"bear trap", C(HI_METAL)},		/* trap */
	{"land mine", C(CLR_RED)},		/* trap */
	{"rolling boulder trap", C(CLR_GRAY)},	/* trap */
/*50*/	{"sleeping gas trap", C(HI_ZAP)},	/* trap */
	{"rust trap", C(CLR_BLUE)},		/* trap */
	{"fire trap", C(CLR_ORANGE)},		/* trap */
	{"pit", C(CLR_BLACK)},			/* trap */
	{"spiked pit", C(CLR_BLACK)},		/* trap */
	{"hole", C(CLR_BROWN)},			/* trap */
	{"trap door", C(CLR_BROWN)},		/* trap */
	{"teleportation trap", C(CLR_MAGENTA)},	/* trap */
	{"level teleporter", C(CLR_MAGENTA)},	/* trap */
	{"magic portal", C(CLR_BRIGHT_MAGENTA)},/* trap */
/*60*/	{"web", C(CLR_GRAY)},			/* web */
	{"statue trap", C(CLR_GRAY)},		/* trap */
	{"magic trap", C(HI_ZAP)},		/* trap */
	{"anti-magic field", C(HI_ZAP)},	/* trap */
	{"polymorph trap", C(CLR_BRIGHT_GREEN)},/* trap */
	{"wall", C(CLR_GRAY)},			/* vbeam */
	{"wall", C(CLR_GRAY)},			/* hbeam */
	{"wall", C(CLR_GRAY)},			/* lslant */
	{"wall", C(CLR_GRAY)},			/* rslant */
	{"", C(CLR_WHITE)},			/* dig beam */
/*70*/	{"", C(CLR_WHITE)},			/* camera flash beam */
	{"", C(HI_WOOD)},			/* boomerang open left */
	{"", C(HI_WOOD)},			/* boomerang open right */
	{"", C(HI_ZAP)},			/* 4 magic shield symbols */
	{"", C(HI_ZAP)},
	{"", C(HI_ZAP)},
	{"", C(HI_ZAP)},
	{"", C(CLR_GREEN)},			/* swallow top left	*/
	{"", C(CLR_GREEN)},			/* swallow top center	*/
	{"", C(CLR_GREEN)},			/* swallow top right	*/
/*80*/	{"", C(CLR_GREEN)},			/* swallow middle left	*/
	{"", C(CLR_GREEN)},			/* swallow middle right	*/
	{"", C(CLR_GREEN)},			/* swallow bottom left	*/
	{"", C(CLR_GREEN)},			/* swallow bottom center*/
	{"", C(CLR_GREEN)},			/* swallow bottom right	*/
	{"", C(CLR_ORANGE)},			/* explosion top left     */
	{"", C(CLR_ORANGE)},			/* explosion top center   */
	{"", C(CLR_ORANGE)},			/* explosion top right    */
	{"", C(CLR_ORANGE)},			/* explosion middle left  */
	{"", C(CLR_ORANGE)},			/* explosion middle center*/
/*90*/	{"", C(CLR_ORANGE)},			/* explosion middle right */
	{"", C(CLR_ORANGE)},			/* explosion bottom left  */
	{"", C(CLR_ORANGE)},			/* explosion bottom center*/
	{"", C(CLR_ORANGE)},			/* explosion bottom right */
	{"unknown creature causing you worry", C(CLR_WHITE)},		/* white warning  */
	{"unknown creature causing you concern", C(CLR_RED)},		/* pink warning   */
	{"unknown creature causing you anxiety", C(CLR_RED)},		/* red warning    */
	{"unknown creature causing you disquiet", C(CLR_RED)},		/* ruby warning   */
	{"unknown creature causing you alarm", C(CLR_MAGENTA)},		/* purple warning */
	{"unknown creature causing you dread", C(CLR_BRIGHT_MAGENTA)}	/* black warning  */

	/*
	 *  Note: Additions to this array should be reflected in the
	 *	  {ascii,utf8}_graphics[] arrays below.
	 */
};

#undef C

const glyph_t ascii_graphics[MAXPCHARS] = {
/* 0*/	' ',  // stone
	'|',	     // vwall
	'-',	     // hwall
	'-',	     // tlcorn
	'-',	     // trcorn
	'-',	     // blcorn
	'-',	     // brcorn
	'-',	     // crwall
	'-',	     // tuwall
	'-',	     // tdwall
/*10*/	'|',  // tlwall
	'|',	     // trwall
	'.',	     // ndoor
	'-',	     // vodoor
	'|',	     // hodoor
	'+',	     // vcdoor
	'+',	     // hcdoor
	'#',	     // bars
	'#',	     // tree
	'.',	     // room
/*20*/	'.',  // dark room
	'#',	     // dark corr
	'#',	     // lit corr (see mapglyph.c)
	'<',	     // upstair
	'>',	     // dnstair
	'<',	     // upladder
	'>',	     // dnladder
	'_',	     // altar
	'|',	     // grave
	'\\',	     // throne
/*30*/	'#',  // sink
	'#',	     // toilet
	'{',	     // fountain
	'}',	     // pool
	'.',	     // ice
	'}',	     // lava
	'.',	     // vodbridge
	'.',	     // hodbridge
	'#',	     // vcdbridge
	'#',	     // hcdbridge
/*40*/	' ',  // open air
	'#',	     // [part of] a cloud
	'}',	     // under water
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
/*50*/	'^',  // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
/*60*/	'"',  // web
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'^',	     // trap
	'|',	     // vbeam
	'-',	     // hbeam
	'\\',	     // lslant
	'/',	     // rslant
	'*',	     // dig beam
/*70*/	'!',  // camera flash beam
	')',	     // boomerang open left
	'(',	     // boomerang open right
	'0',	     // 4 magic shield symbols (S_ss1)
	'#',	     // S_ss2
	'@',	     // S_ss3
	'*',	     // S_ss4
	'/',	     // swallow top left
	'-',	     // swallow top center
	'\\',	     // swallow top right
/*80*/	'|',  // swallow middle left
	'|',	     // swallow middle right
	'\\',	     // swallow bottom left
	'-',	     // swallow bottom center
	'/',	     // swallow bottom right
	'/',	     // explosion top left
	'-',	     // explosion top center
	'\\',	     // explosion top right
	'|',	     // explosion middle left
	' ',	     // explosion middle center
/*90*/	'|',  // explosion middle right
	'\\',	     // explosion bottom left
	'-',	     // explosion bottom center
	'/',	     // explosion bottom right
	'0',	     // white warning
	'1',	     // pink warning
	'2',	     // red warning
	'3',	     // ruby warning
	'4',	     // purple warning
	'5',	     // black warning
};

static glyph_t utf8_graphics[MAXPCHARS] = {
/* 0*/	' ',		// S_stone
	0x2502,		// S_vwall:     BOX DRAWINGS LIGHT VERTICAL
	0x2500,		// S_hwall:     BOX DRAWINGS LIGHT HORIZONTAL
	0x250c,		// S_tlcorn:    BOX DRAWINGS LIGHT DOWN AND RIGHT
	0x2510,		// S_trcorn:    BOX DRAWINGS LIGHT DOWN AND LEFT
	0x2514,		// S_blcorn:    BOX DRAWINGS LIGHT UP AND RIGHT
	0x2518,		// S_brcorn:    BOX DRAWINGS LIGHT UP AND LEFT
	0x253c,		// S_crwall:    BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
	0x2534,		// S_tuwall:    BOX DRAWINGS LIGHT UP AND HORIZONTAL
	0x252c,		// S_tdwall:    BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
/*10*/	0x2524,		// S_tlwall:    BOX DRAWINGS LIGHT VERTICAL AND LEFT
	0x251c,		// S_trwall:    BOX DRAWINGS LIGHT VERTICAL AND RIGHT
	0x00b7,		// S_ndoor:     MIDDLE DOT
	0x25a0,		// S_vodoor:    BLACK SQUARE
	0x25a0,		// S_hodoor:    BLACK SQUARE
	'+',		// S_vcdoor
	'+',		// S_hcdoor
	0x2261,		// S_bars:      IDENTICAL TO
	0x03a8,		// S_tree:      GREEK CAPITAL LETTER PSI
	0x00b7,		// S_room:      MIDDLE DOT
/*20*/	' ',		// S_stone
	0x2591,		// S_corr:	LIGHT SHADE
	0x2592,		// S_litcorr	MEDIUM SHADE
	'<',		// S_upstair
	'>',		// S_dnstair
	0x2264,		// S_upladder:  LESS-THAN OR EQUAL TO
	0x2265,		// S_dnladder:  GREATER-THAN OR EQUAL TO
	0x03A9,		// S_altar:     GREEK CAPITAL LETTER OMEGA
	0x2020,		// S_grave:     DAGGER
	'\\',		// S_throne
/*30*/	'#',		// S_sink
	'#',		// S_toilet
	0x2320,		// S_fountain:  TOP HALF INTEGRAL
	0x224b,		// S_pool:      TRIPLE TILDE
	0x00b7,		// S_ice:       MIDDLE DOT
	0x224b,		// S_lava:      TRIPLE TILDE
	0x00b7,		// S_vodbridge: MIDDLE DOT
	0x00b7,		// S_hodbridge: MIDDLE DOT
	'#',		// S_vcdbridge
	'#',		// S_hcdbridge
/*40*/	' ',		// S_air
	'#',		// S_cloud
	0x2248,		// S_water:     ALMOST EQUAL TO
	'^',		// S_arrow_trap
	'^',		// S_dart_trap
	'^',		// S_falling_rock_trap
	'^',		// S_squeaky_board
	'^',		// S_bear_trap
	'^',		// S_land_mine
	'^',		// S_rolling_boulder_trap
/*50*/	'^',		// S_sleeping_gas_trap
	'^',		// S_rust_trap
	'^',		// S_fire_trap
	'^',		// S_pit
	'^',		// S_spiked_pit
	'^',		// S_hole
	'^',		// S_trap_door
	'^',		// S_teleportation_trap
	'^',		// S_level_teleporter
	'^',		// S_magic_portal
/*60*/	0x00A4,		// S_web:       CURRENCY SIGN
	'^',		// S_statue_trap
	'^',		// S_magic_trap
	'^',		// S_anti_magic_trap
	'^',		// S_polymorph_trap
	0x2502,		// S_vbeam:     BOX DRAWINGS LIGHT VERTICAL
	0x2500,		// S_hbeam:     BOX DRAWINGS LIGHT HORIZONTAL
	0x2572,		// S_lslant:	BOX DRAWINGS LIGHT DIAGONAL UPPER LEFT TO LOWER RIGHT
	0x2571,		// S_rslant:	BOX DRAWINGS LIGHT DIAGONAL UPPER RIGHT TO LOWER LEFT
	'*',		// S_digbeam
/*70*/	'!',		// S_flashbeam
	')',		// S_boomleft
	'(',		// S_boomright
	'0',		// S_ss1
	'#',		// S_ss2
	'@',		// S_ss3
	'*',		// S_ss4
	0x2571,		// S_sw_tl:	BOX DRAWINGS LIGHT DIAGONAL UPPER RIGHT TO LOWER LEFT
	0x2594,		// S_sw_tc:     UPPER ONE EIGHTH BLOCK
	0x2572,		// S_sw_tr:	BOX DRAWINGS LIGHT DIAGONAL UPPER LEFT TO LOWER RIGHT
/*80*/	0x258f,		// S_sw_ml:     LEFT ONE EIGHTH BLOCK
	0x2595,		// S_sw_mr:     RIGHT ONE EIGHTH BLOCK
	0x2572,		// S_sw_bl:	BOX DRAWINGS LIGHT DIAGONAL UPPER LEFT TO LOWER RIGHT
	0x2581,		// S_sw_bc:     LOWER ONE EIGHTH BLOCK
	0x2571,		// S_sw_br:	BOX DRAWINGS LIGHT DIAGONAL UPPER RIGHT TO LOWER LEFT
	0x2571,		// S_explode1:	BOX DRAWINGS LIGHT DIAGONAL UPPER RIGHT TO LOWER LEFT
	0x2594,		// S_explode2:  UPPER ONE EIGHTH BLOCK
	0x2572,		// S_explode3:	BOX DRAWINGS LIGHT DIAGONAL UPPER LEFT TO LOWER RIGHT
	0x258f,		// S_explode4:  LEFT ONE EIGHTH BLOCK
	' ',		// S_explode5
/*90*/	0x2595,		// S_explode6:  RIGHT ONE EIGHTH BLOCK
	0x2572,		// S_explode7:	BOX DRAWINGS LIGHT DIAGONAL UPPER LEFT TO LOWER RIGHT
	0x2581,		// S_explode8:  LOWER ONE EIGHTH BLOCK
	0x2571,		// S_explode9:	BOX DRAWINGS LIGHT DIAGONAL UPPER RIGHT TO LOWER LEFT
	'0',		// white warning
	'1',		// pink warning
	'2',		// red warning
	'3',		// ruby warning
	'4',		// purple warning
	'5',		// black warning
};

/* Can only use characters from this list
 * http://en.wikipedia.org/wiki/WGL4 */
static glyph_t utf8compat_graphics[MAXPCHARS] = {
/* 0*/	' ',		// S_stone
	0x2502,		// S_vwall:     BOX DRAWINGS LIGHT VERTICAL
	0x2500,		// S_hwall:     BOX DRAWINGS LIGHT HORIZONTAL
	0x250c,		// S_tlcorn:    BOX DRAWINGS LIGHT DOWN AND RIGHT
	0x2510,		// S_trcorn:    BOX DRAWINGS LIGHT DOWN AND LEFT
	0x2514,		// S_blcorn:    BOX DRAWINGS LIGHT UP AND RIGHT
	0x2518,		// S_brcorn:    BOX DRAWINGS LIGHT UP AND LEFT
	0x253c,		// S_crwall:    BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
	0x2534,		// S_tuwall:    BOX DRAWINGS LIGHT UP AND HORIZONTAL
	0x252c,		// S_tdwall:    BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
/*10*/	0x2524,		// S_tlwall:    BOX DRAWINGS LIGHT VERTICAL AND LEFT
	0x251c,		// S_trwall:    BOX DRAWINGS LIGHT VERTICAL AND RIGHT
	0x00b7,		// S_ndoor:     MIDDLE DOT
	0x25a0,		// S_vodoor:    BLACK SQUARE
	0x25a0,		// S_hodoor:    BLACK SQUARE
	'+',		// S_vcdoor
	'+',		// S_hcdoor
	0x2261,		// S_bars:      IDENTICAL TO
	0x03a8,		// S_tree:      GREEK CAPITAL LETTER PSI
	0x00b7,		// S_room:      MIDDLE DOT
/*20*/	' ',		// S_stone
	0x2591,		// S_corr:	LIGHT SHADE
	0x2592,		// S_litcorr	MEDIUM SHADE
	'<',		// S_upstair
	'>',		// S_dnstair
	0x2264,		// S_upladder:  LESS-THAN OR EQUAL TO
	0x2265,		// S_dnladder:  GREATER-THAN OR EQUAL TO
	0x03A9,		// S_altar:     GREEK CAPITAL LETTER OMEGA
	0x2020,		// S_grave:     DAGGER
	'\\',		// S_throne
/*30*/	'#',		// S_sink
	'#',		// S_toilet
	0x2320,		// S_fountain:  TOP HALF INTEGRAL
	0x224b,		// S_pool:      TRIPLE TILDE
	0x00b7,		// S_ice:       MIDDLE DOT
	0x224b,		// S_lava:      TRIPLE TILDE
	0x00b7,		// S_vodbridge: MIDDLE DOT
	0x00b7,		// S_hodbridge: MIDDLE DOT
	'#',		// S_vcdbridge
	'#',		// S_hcdbridge
/*40*/	' ',		// S_air
	'#',		// S_cloud),
	0x2248,		// S_water:     ALMOST EQUAL TO
	'^',		// S_arrow_trap
	'^',		// S_dart_trap
	'^',		// S_falling_rock_trap
	'^',		// S_squeaky_board
	'^',		// S_bear_trap
	'^',		// S_land_mine
	'^',		// S_rolling_boulder_trap
/*50*/	'^',		// S_sleeping_gas_trap
	'^',		// S_rust_trap
	'^',		// S_fire_trap
	'^',		// S_pit
	'^',		// S_spiked_pit
	'^',		// S_hole
	'^',		// S_trap_door
	'^',		// S_teleportation_trap
	'^',		// S_level_teleporter
	'^',		// S_magic_portal
/*60*/	0x00A4,		// S_web:       CURRENCY SIGN
	'^',		// S_statue_trap
	'^',		// S_magic_trap
	'^',		// S_anti_magic_trap
	'^',		// S_polymorph_trap
	0x2502,		// S_vbeam:     BOX DRAWINGS LIGHT VERTICAL
	0x2500,		// S_hbeam:     BOX DRAWINGS LIGHT HORIZONTAL
	'\\',		// S_lslant
	'/',		// S_rslant
	'*',		// S_digbeam
/*70*/	'!',		// S_flashbeam
	')',		// S_boomleft
	'(',		// S_boomright
	'0',		// S_ss1
	'#',		// S_ss2
	'@',		// S_ss3
	'*',		// S_ss4
	'/',		// S_sw_tl
	0x2594,		// S_sw_tc:     UPPER ONE EIGHTH BLOCK
	'\\',		// S_sw_tr
/*80*/	0x258f,		// S_sw_ml:     LEFT ONE EIGHTH BLOCK
	0x2595,		// S_sw_mr:     RIGHT ONE EIGHTH BLOCK
	'\\',		// S_sw_bl
	0x2581,		// S_sw_bc:     LOWER ONE EIGHTH BLOCK
	'/',		// S_sw_br
	'/',		// S_explode1
	0x2594,		// S_explode2:  UPPER ONE EIGHTH BLOCK
	'\\',		// S_explode3
	0x258f,		// S_explode4:  LEFT ONE EIGHTH BLOCK
	' ',		// S_explode5
/*90*/	0x2595,		// S_explode6:  RIGHT ONE EIGHTH BLOCK
	'\\',		// S_explode7
	0x2581,		// S_explode8:  LOWER ONE EIGHTH BLOCK
	'/',		// S_explode9
	'0',		// white warning
	'1',		// pink warning
	'2',		// red warning
	'3',		// ruby warning
	'4',		// purple warning
	'5',		// black warning
};

/*
 * Convert the given character to an object class.  If the character is not
 * recognized, then MAXOCLASSES is returned.  Used in detect.c invent.c,
 * options.c, pickup.c, sp_lev.c, and lev_main.c.
 */
int def_char_to_objclass(char ch) {
	int i;
	for (i = 1; i < MAXOCLASSES; i++) {
		if (ch == def_oc_syms[i])
			break;
	}

	return i;
}

/*
 * Convert a character into a monster class.  This returns the _first_
 * match made.  If there are are no matches, return MAXMCLASSES.
 */
int def_char_to_monclass(char ch) {
	int i;
	for (i = 1; i < MAXMCLASSES; i++) {
		if (def_monsyms[i] == ch)
			break;
	}

	return i;
}

void assign_graphics(const glyph_t *graph_chars, int glth, int maxlen, int offset) {
	int i;

	for (i = 0; i < maxlen && i < glth; i++) {
		showsyms[i + offset] = graph_chars[i];
	}
}

void assign_colors(uchar *graph_colors, int glth, int maxlen, int offset) {
	int i;

	for (i = 0; i < maxlen; i++) {
		showsymcolors[i + offset] =
			(((i < glth) && (graph_colors[i] < CLR_MAX)) ?
				 graph_colors[i] :
				 sym_desc[i + offset].color);
	}
}

void switch_graphics(int graphics) {
	switch (graphics) {
		case ASCII_GRAPHICS:
		default:
			assign_graphics(ascii_graphics, SIZE(ascii_graphics), MAXPCHARS, 0);
			iflags.graphics = ASCII_GRAPHICS;
			break;
		case UTF8_GRAPHICS:
			assign_graphics(utf8_graphics, SIZE(utf8_graphics), MAXPCHARS, 0);
			iflags.graphics = UTF8_GRAPHICS;
			break;
		case UTF8COMPAT_GRAPHICS:
			assign_graphics(utf8compat_graphics, SIZE(utf8compat_graphics), MAXPCHARS, 0);
			iflags.graphics = UTF8COMPAT_GRAPHICS;
			break;
	}
}

/*
 * saved display symbols for objects & features.
 */
static glyph_t save_oc_syms[MAXOCLASSES] = {0};
static glyph_t save_showsyms[MAXPCHARS] = {0};
static glyph_t save_monsyms[MAXPCHARS] = {0};

static const glyph_t r_oc_syms[MAXOCLASSES] = {
/* 0*/	'\0',
	ILLOBJ_SYM,
	WEAPON_SYM,
	']', /* armor */
	RING_SYM,
/* 5*/	',', /* amulet */
	TOOL_SYM,
	':', /* food */
	POTION_SYM,
	SCROLL_SYM,
/*10*/	SPBOOK_SYM,
	WAND_SYM,
	GEM_SYM, /* gold -- yes it's the same as gems */
	GEM_SYM,
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM};

/* Rogue level graphics.  Under IBM graphics mode, use the symbols that were
 * used for EPYX Rogue on the IBM PC.
 */
// Translated to unicode from the original CP437 -MC

// note: all of these work with both utf8 and utf8compatgraphics
// probably, wgl4 was designed to be compatible with cp437
static const glyph_t IBM_r_oc_syms[MAXOCLASSES] = {
/* 0*/	'\0',
	ILLOBJ_SYM,
	0x2191,		// weapon: up arrow
	0x25d9,		// armor: Vert rect with o
	0x2642,		// ring: circle with arrow
/* 5*/	0x2640,	// amulet: "female" symbol
	TOOL_SYM,
	0x2663,	 // food: club (cards)
	0x00a1,	 // potion: upside down '!'
	0x266b,	 // scroll: musical note
/*10*/	SPBOOK_SYM,
	0x03c4,	 // wand: greek tau
	0x263c,	 // gold: yes, it's the same as gems
	0x263c,	 // gems: fancy '*'
	ROCK_SYM,
/*15*/	BALL_SYM,
	CHAIN_SYM,
	VENOM_SYM};

void assign_rogue_graphics(bool is_rlevel) {
	/* Adjust graphics display characters on Rogue levels */
	if (is_rlevel) {
		int i;

		memcpy(save_showsyms, showsyms, sizeof showsyms);
		memcpy(save_oc_syms, oc_syms, sizeof oc_syms);
		memcpy(save_monsyms, monsyms, sizeof monsyms);

		/* Use a loop: char != uchar on some machines. */
		for (i = 0; i < MAXMCLASSES; i++)
			monsyms[i] = def_monsyms[i];

		/*
		if (iflags.graphics == UTF8_GRAPHICS || iflags.graphics == UTF8COMPAT_GRAPHICS) {
			monsyms[S_HUMAN] = 0x01; // smiley face
		}
		*/

		for (i = 0; i < MAXPCHARS; i++)
			showsyms[i] = ascii_graphics[i];

		/*
		 * Some day if these rogue showsyms get much more extensive than this,
		 * we may want to create r_showsyms, and IBM_r_showsyms arrays to hold
		 * all of this info and to simply initialize it via a for() loop like r_oc_syms.
		 */

		if (iflags.graphics == ASCII_GRAPHICS) {
			showsyms[S_vodoor] = showsyms[S_hodoor] = showsyms[S_ndoor] = '+';
			showsyms[S_upstair] = showsyms[S_dnstair] = '%';
		} else if (iflags.graphics == UTF8_GRAPHICS || iflags.graphics == UTF8COMPAT_GRAPHICS) {
			/* a la EPYX Rogue */
			showsyms[S_vwall] = 0x2551; /* all walls now use	*/
			showsyms[S_hwall] = 0x2550; /* double line graphics	*/
			showsyms[S_tlcorn] = 0x2554;
			showsyms[S_trcorn] = 0x2557;
			showsyms[S_blcorn] = 0x255a;
			showsyms[S_brcorn] = 0x255d;
			showsyms[S_crwall] = 0x256c;
			showsyms[S_tuwall] = 0x2569;
			showsyms[S_tdwall] = 0x2566;
			showsyms[S_tlwall] = 0x2563;
			showsyms[S_trwall] = 0x2560;
			showsyms[S_ndoor] = 0x256c;
			showsyms[S_vodoor] = 0x256c;
			showsyms[S_hodoor] = 0x256c;

			showsyms[S_room] = 0x00b7;  // centered dot
			showsyms[S_darkroom] = 0x00b7;
			showsyms[S_corr] = 0x2592;
			showsyms[S_litcorr] = 0x2593;
			showsyms[S_upstair] = 0x039e;  // Greek Xi (ibm: was 0x2261, which is just a triple bar)
			showsyms[S_dnstair] = 0x039e;
			showsyms[S_arrow_trap] = 0x2666;  // diamond (cards)
			showsyms[S_dart_trap] = 0x2666;
			showsyms[S_falling_rock_trap] = 0x2666;
			showsyms[S_squeaky_board] = 0x2666;
			showsyms[S_bear_trap] = 0x2666;
			showsyms[S_land_mine] = 0x2666;
			showsyms[S_rolling_boulder_trap] = 0x2666;
			showsyms[S_sleeping_gas_trap] = 0x2666;
			showsyms[S_rust_trap] = 0x2666;
			showsyms[S_fire_trap] = 0x2666;
			showsyms[S_pit] = 0x2666;
			showsyms[S_spiked_pit] = 0x2666;
			showsyms[S_hole] = 0x2666;
			showsyms[S_trap_door] = 0x2666;
			showsyms[S_teleportation_trap] = 0x2666;
			showsyms[S_level_teleporter] = 0x2666;
			showsyms[S_magic_portal] = 0x2666;
			showsyms[S_web] = 0x2666;
			showsyms[S_statue_trap] = 0x2666;
			showsyms[S_magic_trap] = 0x2666;
			showsyms[S_anti_magic_trap] = 0x2666;
			showsyms[S_polymorph_trap] = 0x2666;
		}

		for (i = 0; i < MAXOCLASSES; i++) {
			if (iflags.graphics == UTF8_GRAPHICS || iflags.graphics == UTF8COMPAT_GRAPHICS) {
				oc_syms[i] = IBM_r_oc_syms[i];
			} else if (iflags.graphics == ASCII_GRAPHICS) {
				oc_syms[i] = r_oc_syms[i];
			}
		}
	} else {
		memcpy(showsyms, save_showsyms, sizeof showsyms);
		memcpy(oc_syms, save_oc_syms, sizeof oc_syms);
		memcpy(monsyms, save_monsyms, sizeof monsyms);
	}
}

/*drawing.c*/
