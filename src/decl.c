/*	SCCS Id: @(#)decl.c	3.2	2001/12/10	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

int (*afternmv)(void);
int (*occupation)(void);

/* from xxxmain.c */
const char *hname = 0;		/* name of the game (argv[0] of main) */
int hackpid = 0;		/* current process id */
#ifdef UNIX
int locknum = 0;		/* max num of simultaneous users */
#endif
#ifdef DEF_PAGER
char *catmore = 0;		/* default pager */
#endif

int bases[MAXOCLASSES] = DUMMY;

int multi = 0;
boolean multi_one = false;	/* used by dofire() and throw_the_obj() */
#if 0
int warnlevel = 0;		/* used by movemon and dochugw */
#endif
int nroom = 0;
int nsubroom = 0;
int occtime = 0;

int x_maze_max, y_maze_max;	/* initialized in main, used in mkmaze.c */
int otg_temp;			/* used by object_to_glyph() [otg] */

int in_doagain = 0;

/*
 *	The following structure will be initialized at startup time with
 *	the level numbers of some "important" things in the game.
 */
struct dgn_topology dungeon_topology = {DUMMY};

#include "quest.h"
struct q_score	quest_status = DUMMY;

int smeq[MAXNROFROOMS+1] = DUMMY;
int doorindex = 0;

char *save_cm = 0;
int killer_format = 0;
const char *killer = 0;
const char *delayed_killer = 0;
long done_money = 0;
char killer_buf[BUFSZ] = DUMMY;
const char *nomovemsg = 0;
const char nul[40] = DUMMY;			/* contains zeros */
char plname[PL_NSIZ] = DUMMY;		/* player name */
char pl_character[PL_CSIZ] = DUMMY;
char pl_race = '\0';

char pl_fruit[PL_FSIZ] = DUMMY;
int current_fruit = 0;
struct fruit *ffruit = NULL;

char tune[6] = DUMMY;

const char *occtxt = DUMMY;
const char quitchars[] = " \r\n\033";
const char vowels[] = "aeiouAEIOU";
const char ynchars[] = "yn";
const char ynqchars[] = "ynq";
const char ynaqchars[] = "ynaq";
const char ynNaqchars[] = "yn#aq";
long yn_number = 0L;

const char disclosure_options[] = "iavgc";

#ifdef WIN32
char hackdir[PATHLEN];		/* where rumors, help, record are */
#endif


struct linfo level_info[MAXLINFO];

struct sinfo program_state;

/* 'rogue'-like direction commands (cmd.c) */
char sdir[] = "hykulnjb><";
const char ndir[] = "47896321><";	/* number pad mode */
const schar xdir[10] = {-1,-1, 0, 1, 1, 1, 0,-1, 0, 0};
const schar ydir[10] = { 0,-1,-1,-1, 0, 1, 1, 1, 0, 0};
const schar zdir[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 1,-1};
char misc_cmds[] = {'g', 'G', 'F', 'm', 'M', '\033', '\001'};

schar tbx = 0, tby = 0;	/* mthrowu: target */

/* for xname handling of multiple shot missile volleys:
   number of shots, index of current one, validity check, shoot vs throw */
struct multishot m_shot = { 0, 0, STRANGE_OBJECT, false };

struct dig_info digging;

dungeon dungeons[MAXDUNGEON];	/* ini'ed by init_dungeon() */
s_level *sp_levchn;
stairway upstair = { 0, 0 }, dnstair = { 0, 0 };
stairway upladder = { 0, 0 }, dnladder = { 0, 0 };
stairway sstairs = { 0, 0 };
dest_area updest = { 0, 0, 0, 0, 0, 0, 0, 0 };
dest_area dndest = { 0, 0, 0, 0, 0, 0, 0, 0 };
coord inv_pos = { 0, 0 };

boolean defer_see_monsters = false;
boolean in_mklev = false;
boolean stoned = false;	/* done to monsters hit by 'c' */
boolean unweapon = false;
boolean mrg_to_wielded = false;
			 /* weapon picked is merged with wielded one */
struct obj *current_wand = 0;	/* wand currently zapped/applied */

boolean in_steed_dismounting = false;

coord bhitpos = DUMMY;
struct door doors[DOORMAX] = {DUMMY};

struct mkroom rooms[(MAXNROFROOMS+1)*2] = {DUMMY};
struct mkroom* subrooms = &rooms[MAXNROFROOMS+1];
struct mkroom *upstairs_room, *dnstairs_room, *sstairs_room;

dlevel_t level;		/* level map */
struct trap *ftrap = NULL;
struct monst youmonst = DUMMY;
struct permonst upermonst = DUMMY;
struct flag flags = DUMMY;
struct instance_flags iflags = DUMMY;
struct you u = DUMMY;

struct obj *invent = NULL,
	*uwep = NULL, *uarm = NULL,
	*uswapwep = NULL,
	*uquiver = NULL, /* quiver */
	*uarmu = NULL, /* under-wear, so to speak */
	*uskin = NULL, /* dragon armor, if a dragon */
	*uarmc = NULL, *uarmh = NULL,
	*uarms = NULL, *uarmg = NULL,
	*uarmf = NULL, *uamul = NULL,
	*uright = NULL,
	*uleft = NULL,
	*ublindf = NULL,
	*usaddle = NULL,
	*uchain = NULL,
	*uball = NULL;

/*
 *  This must be the same order as used for buzz() in zap.c.
 */
const int zapcolors[NUM_ZAP] = {
    HI_ZAP,		/* 0 - missile */
    CLR_ORANGE,		/* 1 - fire */
    CLR_WHITE,		/* 2 - frost */
    HI_ZAP,		/* 3 - sleep */
    CLR_BLACK,		/* 4 - death */
    CLR_WHITE,		/* 5 - lightning */
    CLR_YELLOW,		/* 6 - poison gas */
    CLR_GREEN,		/* 7 - acid */
};

const int shield_static[SHIELD_COUNT] = {
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,	/* 7 per row */
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
    S_ss1, S_ss2, S_ss3, S_ss2, S_ss1, S_ss2, S_ss4,
};

struct spell spl_book[MAXSPELL + 1] = {DUMMY};

struct tech tech_list[MAXTECH + 1] = {DUMMY};

long moves = 1L, monstermoves = 1L;
	 /* These diverge when player is Fast or Very_fast */
long wailmsg = 0L;

/* objects that are moving to another dungeon level */
struct obj *migrating_objs = NULL;
/* objects not yet paid for */
struct obj *billobjs = NULL;

/* used to zero all elements of a struct obj */
struct obj zeroobj = DUMMY;

/* used as an address returned by getobj() */
struct obj thisplace = DUMMY;

/* originally from dog.c */
char dogname[PL_PSIZ] = DUMMY;
char catname[PL_PSIZ] = DUMMY;
char ghoulname[PL_PSIZ] = DUMMY;
char horsename[PL_PSIZ] = DUMMY;
char wolfname[PL_PSIZ] = DUMMY;
#if 0
char batname[PL_PSIZ] = DUMMY;
char snakename[PL_PSIZ] = DUMMY;
char ratname[PL_PSIZ] = DUMMY;
char badgername[PL_PSIZ] = DUMMY;
char reddragonname[PL_PSIZ] = DUMMY;
char whitedragonname[PL_PSIZ] = DUMMY;
#endif
char preferred_pet;	/* '\0', 'c', 'd', 'n' (none) */
/* monsters that went down/up together with @ */
struct monst *mydogs = NULL;
/* monsters that are moving to another dungeon level */
struct monst *migrating_mons = NULL;

struct mvitals mvitals[NUMMONS];

/* originally from end.c */
#ifdef DUMP_LOG
#ifdef DUMP_FN
char dump_fn[] = DUMP_FN;
#else
char dump_fn[PL_PSIZ] = DUMMY;
#endif
#endif /* DUMP_LOG */

struct c_color_names c_color_names = {
	"black", "amber", "golden",
	"light blue", "red", "green",
	"silver", "blue", "purple",
	"white"
};

const char *c_obj_colors[] = {
	"black",		/* CLR_BLACK */
	"red",			/* CLR_RED */
	"green",		/* CLR_GREEN */
	"brown",		/* CLR_BROWN */
	"blue",			/* CLR_BLUE */
	"magenta",		/* CLR_MAGENTA */
	"cyan",			/* CLR_CYAN */
	"gray",			/* CLR_GRAY */
	"transparent",		/* no_color */
	"orange",		/* CLR_ORANGE */
	"bright green",		/* CLR_BRIGHT_GREEN */
	"yellow",		/* CLR_YELLOW */
	"bright blue",		/* CLR_BRIGHT_BLUE */
	"bright magenta",	/* CLR_BRIGHT_MAGENTA */
	"bright cyan",		/* CLR_BRIGHT_CYAN */
	"white",		/* CLR_WHITE */
};

struct menucoloring *menu_colorings = 0;

/* NOTE: the order of these words exactly corresponds to the
   order of oc_material values #define'd in objclass.h. */
const char *materialnm[] = {
	"mysterious", "liquid", "wax", "organic", "flesh",
	"paper", "cloth", "leather", "wooden", "bone", "dragonhide",
	"iron", "metal", "copper", "silver", "gold", "platinum", "mithril",
	"plastic", "glass", "gemstone", "stone"
};

/* Vision */
boolean vision_full_recalc = 0;
char	 **viz_array = 0;/* used in cansee() and couldsee() macros */

/* Global windowing data, defined here for multi-window-system support */
winid WIN_MESSAGE = WIN_ERR, WIN_STATUS = WIN_ERR;
winid WIN_MAP = WIN_ERR, WIN_INVEN = WIN_ERR;
char toplines[TBUFSZ];
/* Windowing stuff that's really tty oriented, but present for all ports */
struct tc_gbl_data tc_gbl_data = { 0,0, 0,0 };	/* AS,AE, LI,CO */

struct authentication authentication = { "", "" };

struct tileset tilesets[MAXNOTILESETS];
int no_tilesets = 0;
struct tileset def_tilesets[] = {
    {"", "", 0,}
};

char tileset[PL_PSIZ] = DUMMY;

char *fqn_prefix[PREFIX_COUNT] = { NULL, NULL, NULL, NULL,
				NULL, NULL, NULL, NULL, NULL };

#ifdef PREFIXES_IN_USE
char *fqn_prefix_names[PREFIX_COUNT] = { "hackdir", "leveldir", "savedir",
					"bonesdir", "datadir", "scoredir",
					"lockdir", "configdir", "troubledir" };
#endif

struct u_achieve achieve = DUMMY;

struct realtime_data realtime_data = { 0, 0, 0 };

struct _plinemsg *pline_msg = NULL;

/* FIXME: These should be integrated into objclass and permonst structs,
   but that invalidates saves */
glyph_t objclass_unicode_codepoint[NUM_OBJECTS] = DUMMY;
glyph_t permonst_unicode_codepoint[NUMMONS] = DUMMY;


/* FIXME: The curses windowport requires this stupid hack, in the
   case where a game is in progress and the user is asked if he
   wants to destroy old game.
   Without this, curses tries to show the yn() question with pline()
   ...but the message window isn't up yet.
 */
bool curses_stupid_hack = 1;

/* dummy routine used to force linkage */
void decl_init(void) {
    return;
}

/*decl.c*/
