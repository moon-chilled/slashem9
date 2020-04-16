/*	SCCS Id: @(#)decl.h	3.4	2001/12/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DECL_H
#define DECL_H

#include "nhstr.h"

extern int (*occupation)(void);
extern int (*afternmv)(void);

extern const char *hname;
extern int hackpid;
#ifdef UNIX
extern int locknum;
#endif
#ifdef DEF_PAGER
extern char *catmore;
#endif /* DEF_PAGER */

extern char SAVEF[];

extern int bases[MAXOCLASSES];

extern int multi;
extern int lastuse;
extern int nextuse;
extern int nroom;
extern int nsubroom;
extern int occtime;

extern int x_maze_max, y_maze_max;
extern int otg_temp;

extern int in_doagain;

extern struct dgn_topology { /* special dungeon levels for speed */
	d_level d_oracle_level;
	d_level d_bigroom_level; /* unused */
	d_level d_rogue_level;
	d_level d_medusa_level;
	d_level d_minetn_level;
	d_level d_mineend_level;
	d_level d_stronghold_level;
	d_level d_valley_level;
	d_level d_wiz1_level;
	d_level d_wiz2_level;
	d_level d_wiz3_level;
	d_level d_juiblex_level;
	d_level d_orcus_level;
	d_level d_baalzebub_level;  /* unused */
	d_level d_demogorgon_level; /* unused */
	d_level d_dispater_level;   /* unused */
	d_level d_geryon_level;	    /* unused */
	d_level d_yeenoghu_level;   /* unused */
	d_level d_asmodeus_level;   /* unused */
	d_level d_portal_level;	    /* only in goto_level() [do.c] */
	d_level d_sanctum_level;
	d_level d_earth_level;
	d_level d_water_level;
	d_level d_fire_level;
	d_level d_air_level;
	d_level d_astral_level;
	xchar d_tower_dnum;
	xchar d_sokoban_dnum;
	xchar d_mines_dnum, d_quest_dnum;
	xchar d_spiders_dnum;
	d_level d_lawful_quest_level;
	d_level d_neutral_quest_level;
	d_level d_chaotic_quest_level;
	d_level d_qstart_level, d_qlocate_level, d_nemesis_level;
	d_level d_knox_level;
	d_level d_sokoend_level;
	d_level d_blackmarket_level;
} dungeon_topology;
/* macros for accesing the dungeon levels by their old names */
#define oracle_level	    (dungeon_topology.d_oracle_level)
#define bigroom_level	    (dungeon_topology.d_bigroom_level)
#define rogue_level	    (dungeon_topology.d_rogue_level)
#define medusa_level	    (dungeon_topology.d_medusa_level)
#define stronghold_level    (dungeon_topology.d_stronghold_level)
#define valley_level	    (dungeon_topology.d_valley_level)
#define minetn_level	    (dungeon_topology.d_minetn_level)
#define mineend_level	    (dungeon_topology.d_mineend_level)
#define wiz1_level	    (dungeon_topology.d_wiz1_level)
#define wiz2_level	    (dungeon_topology.d_wiz2_level)
#define wiz3_level	    (dungeon_topology.d_wiz3_level)
#define juiblex_level	    (dungeon_topology.d_juiblex_level)
#define orcus_level	    (dungeon_topology.d_orcus_level)
#define baalzebub_level	    (dungeon_topology.d_baalzebub_level)
#define yeenoghu_level	    (dungeon_topology.d_yeenoghu_level)
#define geryon_level	    (dungeon_topology.d_geryon_level)
#define dispater_level	    (dungeon_topology.d_dispater_level)
#define demogorgon_level    (dungeon_topology.d_demogorgon_level)
#define asmodeus_level	    (dungeon_topology.d_asmodeus_level)
#define portal_level	    (dungeon_topology.d_portal_level)
#define sanctum_level	    (dungeon_topology.d_sanctum_level)
#define earth_level	    (dungeon_topology.d_earth_level)
#define water_level	    (dungeon_topology.d_water_level)
#define fire_level	    (dungeon_topology.d_fire_level)
#define air_level	    (dungeon_topology.d_air_level)
#define astral_level	    (dungeon_topology.d_astral_level)
#define tower_dnum	    (dungeon_topology.d_tower_dnum)
#define sokoban_dnum	    (dungeon_topology.d_sokoban_dnum)
#define mines_dnum	    (dungeon_topology.d_mines_dnum)
#define quest_dnum	    (dungeon_topology.d_quest_dnum)
#define qstart_level	    (dungeon_topology.d_qstart_level)
#define qlocate_level	    (dungeon_topology.d_qlocate_level)
#define nemesis_level	    (dungeon_topology.d_nemesis_level)
#define knox_level	    (dungeon_topology.d_knox_level)
#define sokoend_level	    (dungeon_topology.d_sokoend_level)
#define spiders_dnum	    (dungeon_topology.d_spiders_dnum)
#define lawful_quest_level  (dungeon_topology.d_lawful_quest_level)
#define neutral_quest_level (dungeon_topology.d_neutral_quest_level)
#define chaotic_quest_level (dungeon_topology.d_chaotic_quest_level)
#define blackmarket_level   (dungeon_topology.d_blackmarket_level)

extern stairway dnstair, upstair; /* stairs up and down */
#define xdnstair (dnstair.sx)
#define ydnstair (dnstair.sy)
#define xupstair (upstair.sx)
#define yupstair (upstair.sy)

extern stairway dnladder, upladder; /* ladders up and down */
#define xdnladder (dnladder.sx)
#define ydnladder (dnladder.sy)
#define xupladder (upladder.sx)
#define yupladder (upladder.sy)

extern stairway sstairs;

extern dest_area updest, dndest; /* level-change destination areas */

extern coord inv_pos;
extern dungeon dungeons[];
extern s_level *sp_levchn;
#define dunlev_reached(x) (dungeons[(x)->dnum].dunlev_ureached)

#include "quest.h"
extern struct q_score quest_status;

extern char pl_fruit[PL_FSIZ];
extern int current_fruit;
extern struct fruit *ffruit;

extern char tune[6];

extern int poisoned_pit_threshold;
extern bool know_poisoned_pit_threshold;

#define MAXLINFO (MAXDUNGEON * MAXLEVEL)
extern struct linfo level_info[MAXLINFO];

extern struct sinfo {
	int gameover;  /* self explanatory? */
	int stopprint; /* inhibit further end of game disclosure */
#if defined(UNIX) || defined(__EMX__) || defined(WIN32)
	int done_hup; /* SIGHUP or moral equivalent received
				 * -- no more screen output */
#endif
	int something_worth_saving; /* in case of panic */
	int panicking;		    /* `panic' is in progress */
#ifdef WIN32
	int exiting; /* an exit handler is executing */
#endif
	int in_impossible;
#ifdef PANICLOG
	int in_paniclog;
#endif
	int wizkit_wishing;
} program_state;

extern bool restoring;

extern const char quitchars[];
extern const char vowels[];
extern const char ynchars[];
extern const char ynqchars[];
extern const char ynaqchars[];
extern const char ynNaqchars[];
extern long yn_number;

extern const char disclosure_options[];

extern int smeq[];
extern int doorindex;
extern char *save_cm;

enum killer_format {
	KILLED_BY_AN = 0,
	KILLED_BY = 1,
	NO_KILLER_PREFIX = 2,
};

extern struct kinfo {
	struct kinfo *next; // chain of delayed killers
	int id; // uprop keys to ID a delayed killer
	enum killer_format format;
	nhstr name; // actual killer name.  This MUST come last.
} killer;

extern long done_money;
extern const char *configfile;
extern char plname[PL_NSIZ];
extern char dogname[];
extern char catname[];
extern char ghoulname[];
extern char horsename[];
extern char wolfname[];
#if 0
extern char batname[];
extern char snakename[];
extern char ratname[];
extern char badgername[];
extern char reddragonname[];
extern char whitedragonname[];
#endif
extern char preferred_pet;
extern const char *occtxt; /* defined when occupation != NULL */
extern const char *nomovemsg;
extern const char nul[];
extern char lock[];

/* sdir is the regular direction keys (modifyable).
 * ndir is the number_pad direction keys.
 * xdir/ydir/zdir holds the directions of movement for each of the sdir and
 *     ndir keys.
 * misc_cmds holds all other special keyboard commands.
 */
extern char sdir[];
extern const char ndir[];

extern const schar xdir[], ydir[], zdir[];

extern char misc_cmds[];

#define DORUSH		'g'
#define DORUN		'G'
#define DOFORCEFIGHT	'F'
#define DONOPICKUP	'm'
#define DORUN_NOPICKUP	'M'
#define DOESCAPE	'\033'
#define DOAGAIN		'\001' // ^A

/* the number of miscellaneous commands */
#define MISC_CMD_COUNT 7

extern schar tbx, tby; /* set in mthrowu.c */

extern struct multishot {
	int n, i;
	short o;
	boolean s;
} m_shot;

extern long moves, monstermoves;
extern long wailmsg;

extern boolean in_mklev;
extern boolean stoned;
extern boolean unweapon;
extern boolean mrg_to_wielded;
extern struct obj *current_wand;
extern boolean defer_see_monsters;

extern boolean in_steed_dismounting;

extern const int shield_static[];

/*** Objects ***/
#include "obj.h"

extern struct obj *invent, *uarm, *uarmc, *uarmh, *uarms, *uarmg, *uarmf, *uarmu /* under-wear, so to speak */,
	*usaddle,
	*uskin, *uamul, *uleft, *uright, *ublindf,
	*uwep, *uswapwep, *uquiver;

extern struct obj *uchain; /* defined only when punished */
extern struct obj *uball;
extern struct obj *migrating_objs;
extern struct obj *billobjs;
extern struct obj zeroobj;   /* init'd and defined in decl.c */
extern struct obj thisplace; /* init'd and defined in decl.c */

#include "spell.h"
extern struct spell spl_book[]; /* sized in decl.c */

#ifndef TECH_H
#include "tech.h"
#endif
extern struct tech tech_list[]; /* sized in decl.c */

/*** The player ***/
extern char pl_character[PL_CSIZ];
extern char pl_race; /* character's race */
/* KMH, role patch -- more maintainable when declared as an array */
extern const char pl_classes[];

#include "you.h"
#include "onames.h"

extern struct you u;
extern struct Role urole;

/*** Monsters ***/
#ifndef PM_H
#include "pm.h"
#endif

extern struct permonst playermon, *uasmon;
/* also decl'd extern in permonst.h */
/* init'd in monst.c */

extern struct monst youmonst; /* init'd and defined in decl.c */
extern struct monst *mydogs, *migrating_mons;

extern struct permonst upermonst; /* init'd in decl.c,
				   * defined in polyself.c
				   */

extern struct mvitals {
	uchar born;
	uchar died;
	uchar mvflags;
	uchar eaten; /* WAC -- eaten memory */
} mvitals[NUMMONS];

/* The names of the colors used for gems, etc. */
extern const char *c_obj_colors[];

/* material strings */
extern const char *materialnm[];

/* Monster name articles */
#define ARTICLE_NONE 0
#define ARTICLE_THE  1
#define ARTICLE_A    2
#define ARTICLE_YOUR 3

/* Monster name suppress masks */
#define SUPPRESS_IT	       0x01
#define SUPPRESS_INVISIBLE     0x02
#define SUPPRESS_HALLUCINATION 0x04
#define SUPPRESS_SADDLE	       0x08
#define EXACT_NAME	       0x0F

/*** Vision ***/
extern boolean vision_full_recalc; /* true if need vision recalc */
extern char **viz_array;	   /* could see/in sight row pointers */

/*** Window system stuff ***/
#include "color.h"
extern const int zapcolors[];

extern const glyph_t def_oc_syms[MAXOCLASSES]; /* default class symbols */
extern glyph_t oc_syms[MAXOCLASSES];	       /* current class symbols */
extern const char def_monsyms[MAXMCLASSES];    /* default class symbols */
extern uchar monsyms[MAXMCLASSES];	       /* current class symbols */

extern struct c_color_names {
	const char *const c_black, *const c_amber, *const c_golden,
		   *const c_light_blue, *const c_red, *const c_green,
		   *const c_silver, *const c_blue, *const c_purple,
		   *const c_white;
} c_color_names;
#define NH_BLACK      c_color_names.c_black
#define NH_AMBER      c_color_names.c_amber
#define NH_GOLDEN     c_color_names.c_golden
#define NH_LIGHT_BLUE c_color_names.c_light_blue
#define NH_RED	      c_color_names.c_red
#define NH_GREEN      c_color_names.c_green
#define NH_SILVER     c_color_names.c_silver
#define NH_BLUE	      c_color_names.c_blue
#define NH_PURPLE     c_color_names.c_purple
#define NH_WHITE      c_color_names.c_white

extern winid WIN_MESSAGE, WIN_STATUS;
extern winid WIN_MAP, WIN_INVEN;
extern char toplines[];
#ifndef TCAP_H
extern struct tc_gbl_data {  /* also declared in tcap.h */
	char *tc_AS, *tc_AE; /* graphics start and end (tty font swapping) */
	int tc_LI, tc_CO;    /* lines and columns */
} tc_gbl_data;
#define AS tc_gbl_data.tc_AS
#define AE tc_gbl_data.tc_AE
#define LI tc_gbl_data.tc_LI
#define CO tc_gbl_data.tc_CO
#endif

extern struct authentication {
	char prog[BUFSZ];
	char args[BUFSZ];
} authentication;

#define MAXNOTILESETS 20
#ifndef TILESET_MAX_FILENAME
#define TILESET_MAX_FILENAME 256
#endif

#define TILESET_TRANSPARENT 1
#define TILESET_PSEUDO3D    2

extern struct tileset {
	char name[PL_PSIZ];
	char file[TILESET_MAX_FILENAME];
	unsigned long flags;
	void *data; /* For windowing port's use */
} tilesets[MAXNOTILESETS];
extern int no_tilesets;
extern struct tileset def_tilesets[];
extern char tileset[PL_PSIZ];

/* xxxexplain[] is in drawing.c */
extern const char *const monexplain[], invisexplain[], *const objexplain[], *const oclass_names[];

/* Some systems want to use full pathnames for some subsets of file names,
 * rather than assuming that they're all in the current directory.  This
 * provides all the subclasses that seem reasonable, and sets up for all
 * prefixes being null.  Port code can set those that it wants.
 */
#define HACKPREFIX    0
#define LEVELPREFIX   1
#define SAVEPREFIX    2
#define BONESPREFIX   3
#define DATAPREFIX    4 /* this one must match hardcoded value in dlb.c */
#define SCOREPREFIX   5
#define LOCKPREFIX    6
#define CONFIGPREFIX  7
#define TROUBLEPREFIX 8
#define PREFIX_COUNT  9
/* used in files.c; xxconf.h can override if needed */
#ifndef FQN_MAX_FILENAME
#define FQN_MAX_FILENAME 512
#endif

extern char *fqn_prefix[PREFIX_COUNT];

struct autopickup_exception {
	regex_t pattern;
	char *text_pattern;
	bool grab;
	struct autopickup_exception *next;
};

struct u_achieve {
	bool get_bell;		 /* You have obtained the bell of opening */
	bool get_candelabrum;	 /* You have obtained the candelabrum */
	bool get_book;		 /* You have obtained the book of the dead */
	bool enter_gehennom;	 /* Entered Gehennom (including the Valley) by any means */
	bool perform_invocation; /* You have performed the invocation ritual */
	bool get_amulet;	 /* You have obtained the amulet of Yendor */
	bool ascended;		 /* You ascended to demigod[dess]hood.
				  * Not quite the same as u.uevent.ascended. */
	bool get_luckstone;	 /* You obtained the luckstone at the end of the mines. */
	bool finish_sokoban;	 /* You obtained the sokoban prize. */
	bool killed_medusa;	 /* You defeated Medusa. */
};

extern struct u_achieve achieve;

extern struct realtime_data {
	time_t realtime;	    /* Amount of actual playing time up until the last time
				     * the game was restored. */
	time_t restoretime;	    /* The time that the game was started or restored. */
	time_t last_displayed_time; /* Last time displayed on the status line */
} realtime_data;

struct _plinemsg {
	xchar msgtype;
	regex_t pattern;
	struct _plinemsg *next;
};

extern struct _plinemsg *pline_msg;
extern uint saved_pline_index;
extern char *saved_plines[DUMPLOG_MSG_COUNT];

#define MSGTYP_NORMAL 0
#define MSGTYP_NOREP  1
#define MSGTYP_NOSHOW 2
#define MSGTYP_STOP   3

extern glyph_t permonst_unicode_codepoint[NUMMONS];

// FIXME: fix this
extern const glyph_t ascii_graphics[100];

#endif /* DECL_H */
