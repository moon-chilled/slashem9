/*	SCCS Id: @(#)dungeon.c	3.4	1999/10/30	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include <ctype.h>

#include "hack.h"
#include "dgn_file.h"
#include "dlb.h"
#include "display.h"

#define DUNGEON_FILE "lev/dungeon"

#define X_START	 "x-strt"
#define X_LOCATE "x-loca"
#define X_GOAL	 "x-goal"

struct proto_dungeon {
	struct tmpdungeon tmpdungeon[MAXDUNGEON];
	struct tmplevel tmplevel[LEV_LIMIT];
	s_level *final_lev[LEV_LIMIT]; /* corresponding level pointers */
	struct tmpbranch tmpbranch[BRANCH_LIMIT];
	int tmpparent[BRANCH_LIMIT];

	int start;  /* starting index of current dungeon sp levels */
	int n_levs; /* number of tmplevel entries */
	int n_brs;  /* number of tmpbranch entries */
};

int n_dgns; /* number of dungeons (used here,  */
/*   and mklev.c)		   */
static branch *branches = NULL; /* dungeon branch list		   */

struct lchoice {
	int idx;
	schar lev[MAXLINFO];
	schar playerlev[MAXLINFO];
	xchar dgn[MAXLINFO];
	char menuletter;
};

static void Fread(void *, int, int, dlb *);
static xchar dname_to_dnum(const char *);
static int find_branch(const char *, struct proto_dungeon *);
static int level_range(xchar, int, int, int, struct proto_dungeon *, int *);
static xchar parent_dlevel(int, struct proto_dungeon *);
static int correct_branch_type(struct tmpbranch *);
static branch *add_branch(int, int, struct proto_dungeon *);
static void add_level(s_level *);
static void init_level(int, int, struct proto_dungeon *);
static int possible_places(int, boolean *, struct proto_dungeon *);
static xchar pick_level(boolean *, int);
static boolean place_level(int, struct proto_dungeon *);
static const char *br_string(int);
static void print_branch(winid, int, int, int, boolean, struct lchoice *);

mapseen *mapseenchn = NULL;
//static void free_mapseen(mapseen *);
static mapseen *load_mapseen(int);
static void save_mapseen(int, mapseen *);
static mapseen *find_mapseen(d_level *);
static void print_mapseen(winid, mapseen *, bool);
static bool interest_mapseen(mapseen *);
static char *seen_string(xchar x, const char *);
static const char *br_string2(branch *);

#ifdef DEBUG
#define DD dungeons[i]
static void dumpit(void);

static void dumpit(void) {
	int i;
	s_level *x;
	branch *br;

	for (i = 0; i < n_dgns; i++) {
		fprintf(stderr, "\n#%d \"%s\" (%s):\n", i,
			DD.dname, DD.proto);
		fprintf(stderr, "    num_dunlevs %d, dunlev_ureached %d\n",
			DD.num_dunlevs, DD.dunlev_ureached);
		fprintf(stderr, "    depth_start %d, ledger_start %d\n",
			DD.depth_start, DD.ledger_start);
		fprintf(stderr, "    flags:%s%s%s\n",
			DD.flags.rogue_like ? " rogue_like" : "",
			DD.flags.maze_like ? " maze_like" : "",
			DD.flags.hellish ? " hellish" : "");
		tgetch();
	}
	fprintf(stderr, "\nSpecial levels:\n");
	for (x = sp_levchn; x; x = x->next) {
		fprintf(stderr, "%s (%d): ", x->proto, x->rndlevs);
		fprintf(stderr, "on %d, %d; ", x->dlevel.dnum, x->dlevel.dlevel);
		fprintf(stderr, "flags:%s%s%s%s\n",
			x->flags.rogue_like ? " rogue_like" : "",
			x->flags.maze_like ? " maze_like" : "",
			x->flags.hellish ? " hellish" : "",
			x->flags.town ? " town" : "");
		tgetch();
	}
	fprintf(stderr, "\nBranches:\n");
	for (br = branches; br; br = br->next) {
		fprintf(stderr, "%d: %s, end1 %d %d, end2 %d %d, %s\n",
			br->id,
			br->type == BR_STAIR ? "stair" :
			br->type == BR_NO_END1 ? "no end1" :
			br->type == BR_NO_END2 ? "no end2" :
			br->type == BR_PORTAL ? "portal" : "unknown",
			br->end1.dnum, br->end1.dlevel,
			br->end2.dnum, br->end2.dlevel,
			br->end1_up ? "end1 up" : "end1 down");
	}
	tgetch();
	fprintf(stderr, "\nDone\n");
	tgetch();
}
#endif

/* Save the dungeon structures. */
void save_dungeon(int fd, boolean perform_write, boolean free_data) {
	branch *curr, *next;
	mapseen *curr_ms, *next_ms;
	int count;

	if (perform_write) {
		bwrite(fd, &n_dgns, sizeof(n_dgns));
		bwrite(fd, dungeons, sizeof(dungeon) * n_dgns);
		bwrite(fd, &dungeon_topology, sizeof dungeon_topology);
		bwrite(fd, tune, sizeof(tune));
		bwrite(fd, &poisoned_pit_threshold, sizeof(poisoned_pit_threshold));

		for (count = 0, curr = branches; curr; curr = curr->next)
			count++;
		bwrite(fd, &count, sizeof(count));

		for (curr = branches; curr; curr = curr->next)
			bwrite(fd, curr, sizeof(branch));

		count = maxledgerno();
		bwrite(fd, &count, sizeof count);
		bwrite(fd, level_info, count * sizeof(struct linfo));
		bwrite(fd, &inv_pos, sizeof inv_pos);

		for (count = 0, curr_ms = mapseenchn; curr_ms; curr_ms = curr_ms->next)
			count++;
		bwrite(fd, &count, sizeof(count));

		for (curr_ms = mapseenchn; curr_ms; curr_ms = curr_ms->next)
			save_mapseen(fd, curr_ms);
	}

	if (free_data) {
		for (curr = branches; curr; curr = next) {
			next = curr->next;
			free(curr);
		}

		branches = 0;

		for (curr_ms = mapseenchn; curr_ms; curr_ms = next_ms) {
			next_ms = curr_ms->next;
			if (curr_ms->custom)
				free(curr_ms->custom);
			free(curr_ms);
		}
		mapseenchn = 0;
	}
}

/* Restore the dungeon structures. */
void restore_dungeon(int fd) {
	branch *curr, *last;
	mapseen *curr_ms, *last_ms;
	int count, i;

	mread(fd, &n_dgns, sizeof(n_dgns));
	mread(fd, dungeons, sizeof(dungeon) * (unsigned)n_dgns);
	mread(fd, &dungeon_topology, sizeof(dungeon_topology));
	mread(fd, tune, sizeof(tune));
	mread(fd, &poisoned_pit_threshold, sizeof(poisoned_pit_threshold));

	last = branches = NULL;

	mread(fd, (void *)&count, sizeof(count));
	for (i = 0; i < count; i++) {
		curr = alloc(sizeof(branch));
		mread(fd, (void *)curr, sizeof(branch));
		curr->next = NULL;
		if (last)
			last->next = curr;
		else
			branches = curr;
		last = curr;
	}

	mread(fd, &count, sizeof(count));
	if (count >= MAXLINFO)
		panic("level information count larger (%d) than allocated size", count);
	mread(fd, level_info, count * sizeof(struct linfo));
	mread(fd, &inv_pos, sizeof inv_pos);

	mread(fd, &count, sizeof(count));
	last_ms = NULL;
	for (i = 0; i < count; i++) {
		curr_ms = load_mapseen(fd);
		curr_ms->next = NULL;
		if (last_ms) {
			last_ms->next = curr_ms;
		} else {
			mapseenchn = curr_ms;
		}
		last_ms = curr_ms;
	}
}

static void Fread(void *ptr, int size, int nitems, dlb *stream) {
	int cnt;

	if ((cnt = dlb_fread(ptr, size, nitems, stream)) != nitems) {
		panic("Premature EOF on dungeon description file!\r\nExpected %d bytes - got %d.",
			(size * nitems), (size * cnt));
		terminate(EXIT_FAILURE);
	}
}

static xchar dname_to_dnum(const char *s) {
	xchar i;

	for (i = 0; i < n_dgns; i++)
		if (!strcmp(dungeons[i].dname, s)) return i;

	panic("Couldn't resolve dungeon number for name \"%s\".", s);
	/*NOT REACHED*/
	return 0;
}

s_level *find_level(const char *s) {
	s_level *curr;
	for (curr = sp_levchn; curr; curr = curr->next)
		if (!strcmpi(s, curr->proto)) break;
	return curr;
}

/* Find the branch that links the named dungeon. */
static int find_branch(const char *s, struct proto_dungeon *pd) {
	int i;

	if (pd) {
		for (i = 0; i < pd->n_brs; i++)
			if (!strcmp(pd->tmpbranch[i].name, s)) break;
		if (i == pd->n_brs) panic("find_branch: can't find %s", s);
	} else {
		/* support for level tport by name */
		branch *br;
		const char *dnam;

		for (br = branches; br; br = br->next) {
			dnam = dungeons[br->end2.dnum].dname;
			if (!strcmpi(dnam, s) ||
			    (!strncmpi(dnam, "The ", 4) && !strcmpi(dnam + 4, s)))
				break;
		}
		i = br ? ((ledger_no(&br->end1) << 8) | ledger_no(&br->end2)) : -1;
	}
	return i;
}

/*
 * Return a starting point and number of successive positions a level
 * or dungeon entrance can occupy.
 *
 * Note: This follows the acouple (instead of the rcouple) rules for a
 *	 negative random component (rand < 0).  These rules are found
 *	 in dgn_comp.y.  The acouple [absolute couple] section says that
 *	 a negative random component means from the (adjusted) base to the
 *	 end of the dungeon.
 */
static int level_range(xchar dgn, int base, int rand, int chain, struct proto_dungeon *pd, int *adjusted_base) {
	int lmax = dungeons[dgn].num_dunlevs;

	if (chain >= 0) { /* relative to a special level */
		s_level *levtmp = pd->final_lev[chain];
		if (!levtmp) panic("level_range: empty chain level!");

		base += levtmp->dlevel.dlevel;
	} else { /* absolute in the dungeon */
		/* from end of dungeon */
		if (base < 0) base = (lmax + base + 1);
	}

	if (base < 1 || base > lmax)
		panic("level_range: base value out of range");

	*adjusted_base = base;

	if (rand == -1) { /* from base to end of dungeon */
		return lmax - base + 1;
	} else if (rand) {
		/* make sure we don't run off the end of the dungeon */
		return ((base + rand - 1) > lmax) ? lmax - base + 1 : rand;
	} /* else only one choice */
	return 1;
}

static xchar parent_dlevel(int i, struct proto_dungeon *pd) {
	int j, num, base, dnum = pd->tmpparent[i];
	branch *curr;

	num = level_range(dnum, pd->tmpbranch[i].lev.base,
			  pd->tmpbranch[i].lev.rand,
			  pd->tmpbranch[i].chain,
			  pd, &base);

	/* KMH -- Try our best to find a level without an existing branch */
	i = j = rn2(num);
	do {
		if (++i >= num) i = 0;
		for (curr = branches; curr; curr = curr->next)
			if ((curr->end1.dnum == dnum && curr->end1.dlevel == base + i) ||
			    (curr->end2.dnum == dnum && curr->end2.dlevel == base + i))
				break;
	} while (curr && i != j);
	return base + i;
}

/* Convert from the temporary branch type to the dungeon branch type. */
static int correct_branch_type(struct tmpbranch *tbr) {
	switch (tbr->type) {
		case TBR_STAIR:
			return BR_STAIR;
		case TBR_NO_UP:
			return tbr->up ? BR_NO_END1 : BR_NO_END2;
		case TBR_NO_DOWN:
			return tbr->up ? BR_NO_END2 : BR_NO_END1;
		case TBR_PORTAL:
			return BR_PORTAL;
	}
	impossible("correct_branch_type: unknown branch type");
	return BR_STAIR;
}

/*
 * Add the given branch to the branch list.  The branch list is ordered
 * by end1 dungeon and level followed by end2 dungeon and level.  If
 * extract_first is true, then the branch is already part of the list
 * but needs to be repositioned.
 */
void insert_branch(branch *new_branch, boolean extract_first) {
	branch *curr, *prev;
	long new_val, curr_val, prev_val;

	if (extract_first) {
		for (prev = 0, curr = branches; curr; prev = curr, curr = curr->next)
			if (curr == new_branch) break;

		if (!curr) panic("insert_branch: not found");
		if (prev)
			prev->next = curr->next;
		else
			branches = curr->next;
	}
	new_branch->next = NULL;

	/* Convert the branch into a unique number so we can sort them. */
#define branch_val(bp)                              \
	((((long)(bp)->end1.dnum * (MAXLEVEL + 1) + \
	   (long)(bp)->end1.dlevel) *               \
	  (MAXDUNGEON + 1) * (MAXLEVEL + 1)) +      \
	 ((long)(bp)->end2.dnum * (MAXLEVEL + 1) + (long)(bp)->end2.dlevel))

	/*
	 * Insert the new branch into the correct place in the branch list.
	 */
	prev = NULL;
	prev_val = -1;
	new_val = branch_val(new_branch);
	for (curr = branches; curr;
	     prev_val = curr_val, prev = curr, curr = curr->next) {
		curr_val = branch_val(curr);
		if (prev_val < new_val && new_val <= curr_val) break;
	}
	if (prev) {
		new_branch->next = curr;
		prev->next = new_branch;
	} else {
		new_branch->next = branches;
		branches = new_branch;
	}
}

/* Add a dungeon branch to the branch list. */
static branch *add_branch(int dgn, int branch_num, struct proto_dungeon *pd) {
	static int branch_id = 0;
	branch *new_branch;
	int entry_lev;

	new_branch = alloc(sizeof(branch));
	new_branch->next = NULL;
	new_branch->id = branch_id++;
	new_branch->type = correct_branch_type(&pd->tmpbranch[branch_num]);
	new_branch->end1.dnum = pd->tmpparent[branch_num];
	new_branch->end1.dlevel = parent_dlevel(branch_num, pd);
	new_branch->end2.dnum = dgn;
	/*
	 * Calculate the entry level for target dungeon.  The pd.tmpbranch entry
	 * value means:
	 *		< 0	from bottom (-1 == bottom level)
	 *		  0	default (top)
	 *		> 0	actual level (1 = top)
	 */
	if (pd->tmpbranch[branch_num].entry_lev < 0) {
		entry_lev = dungeons[dgn].num_dunlevs + pd->tmpbranch[branch_num].entry_lev + 1;
		if (entry_lev <= 0) entry_lev = 1;
	} else if (pd->tmpbranch[dgn].entry_lev > 0) {
		entry_lev = pd->tmpbranch[branch_num].entry_lev;
		if (entry_lev > dungeons[dgn].num_dunlevs)
			entry_lev = dungeons[dgn].num_dunlevs;
	} else
		entry_lev = 1; /* defaults to top level */

	new_branch->end2.dlevel = entry_lev;
	new_branch->end1_up = pd->tmpbranch[branch_num].up ? true : false;

	insert_branch(new_branch, false);
	return new_branch;
}

/*
 * Add new level to special level chain.  Insert it in level order with the
 * other levels in this dungeon.  This assumes that we are never given a
 * level that has a dungeon number less than the dungeon number of the
 * last entry.
 */
static void add_level(s_level *new_lev) {
	s_level *prev, *curr;

	prev = NULL;
	for (curr = sp_levchn; curr; curr = curr->next) {
		if (curr->dlevel.dnum == new_lev->dlevel.dnum &&
		    curr->dlevel.dlevel > new_lev->dlevel.dlevel)
			break;
		prev = curr;
	}
	if (!prev) {
		new_lev->next = sp_levchn;
		sp_levchn = new_lev;
	} else {
		new_lev->next = curr;
		prev->next = new_lev;
	}
}

static void init_level(int dgn, int proto_index, struct proto_dungeon *pd) {
	s_level *new_level;
	struct tmplevel *tlevel = &pd->tmplevel[proto_index];

	pd->final_lev[proto_index] = NULL; /* no "real" level */
	/*      if (!wizard)   */
	if (tlevel->chance <= rn2(100)) return;

	pd->final_lev[proto_index] = new_level =
		alloc(sizeof(s_level));
	/* load new level with data */
	strcpy(new_level->proto, tlevel->name);
	new_level->boneid = tlevel->boneschar;
	new_level->dlevel.dnum = dgn;
	new_level->dlevel.dlevel = 0; /* for now */

	new_level->flags.town = !!(tlevel->flags & TOWN);
	new_level->flags.hellish = !!(tlevel->flags & HELLISH);
	new_level->flags.maze_like = !!(tlevel->flags & MAZELIKE);
	new_level->flags.rogue_like = !!(tlevel->flags & ROGUELIKE);
	new_level->flags.align = ((tlevel->flags & D_ALIGN_MASK) >> 4);
	if (!new_level->flags.align)
		new_level->flags.align =
			((pd->tmpdungeon[dgn].flags & D_ALIGN_MASK) >> 4);

	new_level->rndlevs = tlevel->rndlevs;
	new_level->next = NULL;
}

static int possible_places(int idx, boolean *map, struct proto_dungeon *pd) {
	int i, start, count;
	s_level *lev = pd->final_lev[idx];

	/* init level possibilities */
	for (i = 0; i <= MAXLEVEL; i++)
		map[i] = false;

	/* get base and range and set those entried to true */
	count = level_range(lev->dlevel.dnum, pd->tmplevel[idx].lev.base,
			    pd->tmplevel[idx].lev.rand,
			    pd->tmplevel[idx].chain,
			    pd, &start);
	for (i = start; i < start + count; i++)
		map[i] = true;

	/* mark off already placed levels */
	for (i = pd->start; i < idx; i++) {
		if (pd->final_lev[i] && map[pd->final_lev[i]->dlevel.dlevel]) {
			map[pd->final_lev[i]->dlevel.dlevel] = false;
			--count;
		}
	}

	return count;
}

/* Pick the nth true entry in the given boolean array. */
static xchar pick_level(boolean *map, int nth) {
	int i;
	for (i = 1; i <= MAXLEVEL; i++)
		if (map[i] && !nth--) return i;
	panic("pick_level:  ran out of valid levels");
	return 0;
}

#ifdef DDEBUG
static void indent(int);

static void indent(int d) {
	while (d-- > 0)
		fputs("    ", stderr);
}
#endif

/*
 * Place a level.  First, find the possible places on a dungeon map
 * template.  Next pick one.  Then try to place the next level.  If
 * sucessful, we're done.  Otherwise, try another (and another) until
 * all possible places have been tried.  If all possible places have
 * been exausted, return false.
 */
static boolean place_level(int proto_index, struct proto_dungeon *pd) {
	boolean map[MAXLEVEL + 1]; /* valid levels are 1..MAXLEVEL inclusive */
	s_level *lev;
	int npossible;
#ifdef DDEBUG
	int i;
#endif

	if (proto_index == pd->n_levs) return true; /* at end of proto levels */

	lev = pd->final_lev[proto_index];

	/* No level created for this prototype, goto next. */
	if (!lev) return place_level(proto_index + 1, pd);

	npossible = possible_places(proto_index, map, pd);

	for (; npossible; --npossible) {
		lev->dlevel.dlevel = pick_level(map, rn2(npossible));
#ifdef DDEBUG
		indent(proto_index - pd->start);
		fprintf(stderr, "%s: trying %d [ ", lev->proto, lev->dlevel.dlevel);
		for (i = 1; i <= MAXLEVEL; i++)
			if (map[i]) fprintf(stderr, "%d ", i);
		fprintf(stderr, "]\n");
#endif
		if (place_level(proto_index + 1, pd)) return true;
		map[lev->dlevel.dlevel] = false; /* this choice didn't work */
	}
#ifdef DDEBUG
	indent(proto_index - pd->start);
	fprintf(stderr, "%s: failed\n", lev->proto);
#endif
	return false;
}

struct level_map {
	const char *lev_name;
	d_level *lev_spec;
} level_map[] = {
	{"air", &air_level},
	{"asmodeus", &asmodeus_level},
	{"demogorg", &demogorgon_level},
	{"geryon", &geryon_level},
	{"dispater", &dispater_level},
	{"yeenoghu", &yeenoghu_level},
	{"astral", &astral_level},
	{"baalz", &baalzebub_level},
	{"bigroom", &bigroom_level},
	{"castle", &stronghold_level},
	{"earth", &earth_level},
	{"fakewiz1", &portal_level},
	{"fire", &fire_level},
	{"juiblex", &juiblex_level},
	{"knox", &knox_level},
	{"blkmar", &blackmarket_level},
	{"medusa", &medusa_level},
	{"minetn", &minetn_level},
	{"mine_end", &mineend_level},
	{"oracle", &oracle_level},
	{"orcus", &orcus_level},
	{"rogue", &rogue_level},
	{"sanctum", &sanctum_level},
	{"valley", &valley_level},
	{"water", &water_level},
	{"wizard1", &wiz1_level},
	{"wizard2", &wiz2_level},
	{"wizard3", &wiz3_level},
	{"minend", &mineend_level},
	{"soko1", &sokoend_level},
	{"nightmar", &lawful_quest_level},
	{"beholder", &neutral_quest_level},
	{"lich", &chaotic_quest_level},
	{X_START, &qstart_level},
	{X_LOCATE, &qlocate_level},
	{X_GOAL, &nemesis_level},
	{"", NULL}};

void init_dungeons(void) {
	dlb *dgn_file;
	int i, cl = 0, cb = 0;
	s_level *x;
	struct proto_dungeon pd;
	struct level_map *lev_map;
	struct version_info vers_info;

	/* [ALI] Cope with being called more than once. The GTK interface
	 * can currently do this, although it really should use popen().
	 */
	free_dungeons();

	pd.n_levs = pd.n_brs = 0;

	dgn_file = dlb_fopen(DUNGEON_FILE, RDBMODE);
	if (!dgn_file) {
		char tbuf[BUFSZ];
		sprintf(tbuf, "Cannot open dungeon description - \"%s",
			DUNGEON_FILE);
#ifdef DLBLIB
		strcat(tbuf, "\" from \"");
		strcat(tbuf, DLB_LIB_FILE);
#endif
		strcat(tbuf, "\" file!");
#ifdef WIN32
		interject_assistance(1, INTERJECT_PANIC, (void *)tbuf,
				     (void *)fqn_prefix[DATAPREFIX]);
#endif
		panic("%s", tbuf);
	}

	/* validate the data's version against the program's version */
	Fread((void *)&vers_info, sizeof vers_info, 1, dgn_file);
	/* we'd better clear the screen now, since when error messages come from
	 * check_version() they will be printed using pline(), which doesn't
	 * mix with the raw messages that might be already on the screen
	 */
	if (iflags.window_inited) clear_nhwindow(WIN_MAP);
	if (!check_version(&vers_info, DUNGEON_FILE, true))
		panic("Dungeon description not valid.");

	/*
	 * Read in each dungeon and transfer the results to the internal
	 * dungeon arrays.
	 */
	sp_levchn = NULL;
	Fread((void *)&n_dgns, sizeof(int), 1, dgn_file);
	if (n_dgns >= MAXDUNGEON)
		panic("init_dungeons: too many dungeons");

	for (i = 0; i < n_dgns; i++) {
		Fread((void *)&pd.tmpdungeon[i],
		      sizeof(struct tmpdungeon), 1, dgn_file);

		if (!wizard && pd.tmpdungeon[i].chance && (pd.tmpdungeon[i].chance <= rn2(100))) {
			int j;

			/* skip over any levels or branches */
			for (j = 0; j < pd.tmpdungeon[i].levels; j++)
				Fread((void *)&pd.tmplevel[cl], sizeof(struct tmplevel),
				      1, dgn_file);

			for (j = 0; j < pd.tmpdungeon[i].branches; j++)
				Fread((void *)&pd.tmpbranch[cb],
				      sizeof(struct tmpbranch), 1, dgn_file);
			n_dgns--;
			i--;
			continue;
		}

		strcpy(dungeons[i].dname, pd.tmpdungeon[i].name);
		strcpy(dungeons[i].proto, pd.tmpdungeon[i].protoname);
		dungeons[i].boneid = pd.tmpdungeon[i].boneschar;

		if (pd.tmpdungeon[i].lev.rand)
			dungeons[i].num_dunlevs = (xchar)rn1(pd.tmpdungeon[i].lev.rand,
							     pd.tmpdungeon[i].lev.base);
		else
			dungeons[i].num_dunlevs = (xchar)pd.tmpdungeon[i].lev.base;

		if (!i) {
			dungeons[i].ledger_start = 0;
			dungeons[i].depth_start = 1;
			dungeons[i].dunlev_ureached = 1;
		} else {
			dungeons[i].ledger_start = dungeons[i - 1].ledger_start +
						   dungeons[i - 1].num_dunlevs;
			dungeons[i].dunlev_ureached = 0;

			if (dungeons[i].ledger_start + dungeons[i].num_dunlevs > 127)
				panic("init_dungeons: too many levels");
		}

		dungeons[i].flags.hellish = !!(pd.tmpdungeon[i].flags & HELLISH);
		dungeons[i].flags.maze_like = !!(pd.tmpdungeon[i].flags & MAZELIKE);
		dungeons[i].flags.rogue_like = !!(pd.tmpdungeon[i].flags & ROGUELIKE);
		dungeons[i].flags.align = ((pd.tmpdungeon[i].flags & D_ALIGN_MASK) >> 4);
		dungeons[i].entry_lev = 1; /* defaults to top level */

		if (i) { /* set depth */
			branch *br = NULL;
			schar from_depth;
			boolean from_up;
			int branch_num;

			for (branch_num = 0; branch_num < pd.n_brs; branch_num++) {
				if (!strcmp(pd.tmpbranch[branch_num].name, dungeons[i].dname)) {
					br = add_branch(i, branch_num, &pd);
					break;
				}
			}

			/* Set the dungeon entry level from the first branch */
			dungeons[i].entry_lev = br->end2.dlevel;

			/* Get the depth of the connecting end. */
			if (br->end1.dnum == i) {
				from_depth = depth(&br->end2);
				from_up = !br->end1_up;
			} else {
				from_depth = depth(&br->end1);
				from_up = br->end1_up;
			}

			/*
			 * Calculate the depth of the top of the dungeon via
			 * its branch.  First, the depth of the entry point:
			 *
			 *	depth of branch from "parent" dungeon
			 *	+ -1 or 1 depending on a up or down stair or
			 *	  0 if portal
			 *
			 * Followed by the depth of the top of the dungeon:
			 *
			 *	- (entry depth - 1)
			 *
			 * We'll say that portals stay on the same depth.
			 */
			dungeons[i].depth_start = from_depth + (br->type == BR_PORTAL ? 0 : (from_up ? -1 : 1)) - (dungeons[i].entry_lev - 1);
		}

		/* this is redundant - it should have been flagged by dgn_comp */
		if (dungeons[i].num_dunlevs > MAXLEVEL)
			dungeons[i].num_dunlevs = MAXLEVEL;

		pd.start = pd.n_levs; /* save starting point */
		pd.n_levs += pd.tmpdungeon[i].levels;
		if (pd.n_levs > LEV_LIMIT)
			panic("init_dungeon: too many special levels");
		/*
		 * Read in the prototype special levels.  Don't add generated
		 * special levels until they are all placed.
		 */
		for (; cl < pd.n_levs; cl++) {
			Fread((void *)&pd.tmplevel[cl],
			      sizeof(struct tmplevel), 1, dgn_file);
			init_level(i, cl, &pd);
		}
		/*
		 * Recursively place the generated levels for this dungeon.  This
		 * routine will attempt all possible combinations before giving
		 * up.
		 */
		if (!place_level(pd.start, &pd))
			panic("init_dungeon:  couldn't place levels");
#ifdef DDEBUG
		fprintf(stderr, "--- end of dungeon %d ---\n", i);
		fflush(stderr);
		tgetch();
#endif
		for (; pd.start < pd.n_levs; pd.start++)
			if (pd.final_lev[pd.start]) add_level(pd.final_lev[pd.start]);

		pd.n_brs += pd.tmpdungeon[i].branches;
		if (pd.n_brs > BRANCH_LIMIT)
			panic("init_dungeon: too many branches");
		for (; cb < pd.n_brs; cb++) {
			int dgn;
			Fread((void *)&pd.tmpbranch[cb],
			      sizeof(struct tmpbranch), 1, dgn_file);
			pd.tmpparent[cb] = i;
			for (dgn = 0; dgn < i; dgn++)
				if (!strcmp(pd.tmpbranch[cb].name, dungeons[dgn].dname)) {
					add_branch(dgn, cb, &pd);
					break;
				}
		}
	}
	dlb_fclose(dgn_file);

	for (i = 0; i < 5; i++)
		tune[i] = 'A' + rn2(7);
	tune[5] = 0;

	// poisoned pits start appearing on levels 8-10
	poisoned_pit_threshold = rn1(3, 8);

	/*
	 * Find most of the special levels and dungeons so we can access their
	 * locations quickly.
	 */
	for (lev_map = level_map; lev_map->lev_name[0]; lev_map++) {
		x = find_level(lev_map->lev_name);
		if (x) {
			assign_level(lev_map->lev_spec, &x->dlevel);
			if (!strncmp(lev_map->lev_name, "x-", 2)) {
				/* This is where the name substitution on the
				 * levels of the quest dungeon occur.
				 */
				sprintf(x->proto, "%s%s", urole.filecode, &lev_map->lev_name[1]);
			} else if (lev_map->lev_spec == &knox_level) {
				branch *br;
				/*
				 * Kludge to allow floating Knox entrance.  We
				 * specify a floating entrance by the fact that
				 * its entrance (end1) has a bogus dnum, namely
				 * n_dgns.
				 */
				for (br = branches; br; br = br->next)
					if (on_level(&br->end2, &knox_level)) break;

				if (br) br->end1.dnum = n_dgns;
				/* adjust the branch's position on the list */
				insert_branch(br, true);
			}
		}
	}
	/*
	 *	I hate hardwiring these names. :-(
	 */
	quest_dnum = dname_to_dnum("The Quest");
	sokoban_dnum = dname_to_dnum("Sokoban");
	mines_dnum = dname_to_dnum("The Gnomish Mines");
	spiders_dnum = dname_to_dnum("The Spider Caves");
	tower_dnum = dname_to_dnum("Vlad's Tower");
	/*
		blackmarket_dnum = dname_to_dnum("The Black Market");
	*/

	/* one special fixup for dummy surface level */
	if ((x = find_level("dummy")) != 0) {
		i = x->dlevel.dnum;
		/* the code above puts earth one level above dungeon level #1,
		   making the dummy level overlay level 1; but the whole reason
		   for having the dummy level is to make earth have depth -1
		   instead of 0, so adjust the start point to shift endgame up */
		if (dunlevs_in_dungeon(&x->dlevel) > 1 - dungeons[i].depth_start)
			dungeons[i].depth_start -= 1;
		/* TO DO: strip "dummy" out all the way here,
		   so that it's hidden from <ctrl/O> feedback. */
	}

#ifdef DEBUG
	dumpit();
#endif
}

// return the level number for lev in *this* dungeon
xchar dunlev(d_level *lev) {
	return lev->dlevel;
}

// return the lowest level number for *this* dungeon
xchar dunlevs_in_dungeon(d_level *lev) {
	/* lowest level of Gnome Mines is gone for Gnomes */
	if (Role_if(PM_GNOME) && lev->dnum == mines_dnum) {
		return (dungeons[lev->dnum].num_dunlevs) - 1;
	} else
		return dungeons[lev->dnum].num_dunlevs;
}

// return the lowest level number for *this* dungeon
xchar real_dunlevs_in_dungeon(d_level *lev) {
	/* this one is not altered for Gnomes */
	return dungeons[lev->dnum].num_dunlevs;
}

// return the lowest level explored in the game
xchar deepest_lev_reached(boolean noquest) {
	/* this function is used for three purposes: to provide a factor
	 * of difficulty in monster generation; to provide a factor of
	 * difficulty in experience calculations (botl.c and end.c); and
	 * to insert the deepest level reached in the game in the topten
	 * display.  the 'noquest' arg switch is required for the latter.
	 *
	 * from the player's point of view, going into the Quest is _not_
	 * going deeper into the dungeon -- it is going back "home", where
	 * the dungeon starts at level 1.  given the setup in dungeon.def,
	 * the depth of the Quest (thought of as starting at level 1) is
	 * never lower than the level of entry into the Quest, so we exclude
	 * the Quest from the topten "deepest level reached" display
	 * calculation.  _However_ the Quest is a difficult dungeon, so we
	 * include it in the factor of difficulty calculations.
	 */
	int i;
	d_level tmp;
	schar ret = 0;

	for (i = 0; i < n_dgns; i++) {
		if ((tmp.dlevel = dungeons[i].dunlev_ureached) == 0) continue;
		if (!strcmp(dungeons[i].dname, "The Quest") && noquest) continue;

		tmp.dnum = i;
		if (depth(&tmp) > ret) ret = depth(&tmp);
	}
	return ret;
}

/* return a bookkeeping level number for purpose of comparisons and
 * save/restore */
xchar ledger_no(d_level *lev) {
	return lev->dlevel + dungeons[lev->dnum].ledger_start;
}

/*
 * The last level in the bookkeeping list of level is the bottom of the last
 * dungeon in the dungeons[] array.
 *
 * Maxledgerno() -- which is the max number of levels in the bookkeeping
 * list, should not be confused with dunlevs_in_dungeon(lev) -- which
 * returns the max number of levels in lev's dungeon, and both should
 * not be confused with deepest_lev_reached() -- which returns the lowest
 * depth visited by the player.
 */
xchar maxledgerno(void) {
	return dungeons[n_dgns - 1].ledger_start + dungeons[n_dgns - 1].num_dunlevs;
}

/* return the dungeon that this ledgerno exists in */
xchar ledger_to_dnum(xchar ledgerno) {
	int i;

	/* find i such that (i->base + 1) <= ledgerno <= (i->base + i->count) */
	for (i = 0; i < n_dgns; i++)
		if (dungeons[i].ledger_start < ledgerno &&
		    ledgerno <= dungeons[i].ledger_start + dungeons[i].num_dunlevs)
			return i;

	panic("level number out of range [ledger_to_dnum(%d)]", (int)ledgerno);
	/*NOT REACHED*/
	return 0;
}

/* return the level of the dungeon this ledgerno exists in */
xchar ledger_to_dlev(xchar ledgerno) {
	return ledgerno - dungeons[ledger_to_dnum(ledgerno)].ledger_start;
}

/* returns the depth of a level, in floors below the surface	*/
/* (note levels in different dungeons can have the same depth).	*/
schar depth(d_level *lev) {
	return dungeons[lev->dnum].depth_start + lev->dlevel - 1;
}

// are "lev1" and "lev2" actually the same?
boolean on_level(d_level *lev1, d_level *lev2) {
	return (lev1->dnum == lev2->dnum) && (lev1->dlevel == lev2->dlevel);
}

// is this level referenced in the special level chain?
s_level *Is_special(d_level *lev) {
	s_level *levtmp;

	for (levtmp = sp_levchn; levtmp; levtmp = levtmp->next)
		if (on_level(lev, &levtmp->dlevel)) return levtmp;

	return NULL;
}

/*
 * Is this a multi-dungeon branch level?  If so, return a pointer to the
 * branch.  Otherwise, return null.
 */
branch *Is_branchlev(d_level *lev) {
	branch *curr;

	for (curr = branches; curr; curr = curr->next) {
		if (on_level(lev, &curr->end1) || on_level(lev, &curr->end2))
			return curr;
	}
	return NULL;
}

/* goto the next level (or appropriate dungeon) */
void next_level(boolean at_stairs) {
	if (at_stairs && u.ux == sstairs.sx && u.uy == sstairs.sy) {
		/* Taking a down dungeon branch. */
		goto_level(&sstairs.tolev, at_stairs, false, false);
	} else {
		/* Going down a stairs or jump in a trap door. */
		d_level newlevel;

		newlevel.dnum = u.uz.dnum;
		newlevel.dlevel = u.uz.dlevel + 1;
		goto_level(&newlevel, at_stairs, !at_stairs, false);
	}
}

/* goto the previous level (or appropriate dungeon) */
void prev_level(boolean at_stairs) {
	if (at_stairs && u.ux == sstairs.sx && u.uy == sstairs.sy) {
		/* Taking an up dungeon branch. */
		/* KMH -- Upwards branches are okay if not level 1 */
		/* (Just make sure it doesn't go above depth 1) */
		if (!u.uz.dnum && u.uz.dlevel == 1 && !u.uhave.amulet)
			done(ESCAPED);
		else
			goto_level(&sstairs.tolev, at_stairs, false, false);
	} else {
		/* Going up a stairs or rising through the ceiling. */
		d_level newlevel;
		newlevel.dnum = u.uz.dnum;
		newlevel.dlevel = u.uz.dlevel - 1;
		goto_level(&newlevel, at_stairs, false, false);
	}
}

void u_on_newpos(int x, int y) {
	u.ux = x;
	u.uy = y;
	cliparound(u.ux, u.uy);

	// ridden steed always shares hero's location
	if (u.usteed) u.usteed->mx = u.ux, u.usteed->my = u.uy;
}

// place you on the special staircase
void u_on_sstairs(void) {
	if (sstairs.sx) {
		u_on_newpos(sstairs.sx, sstairs.sy);
	} else {
		/* code stolen from goto_level */
		int trycnt = 0;
		xchar x, y;
#ifdef DEBUG
		pline("u_on_sstairs: picking random spot");
#endif
#define badspot(x, y) ((levl[x][y].typ != ROOM && levl[x][y].typ != CORR) || MON_AT(x, y))
		do {
			x = rnd(COLNO - 1);
			y = rn2(ROWNO);
			if (!badspot(x, y)) {
				u_on_newpos(x, y);
				return;
			}
		} while (++trycnt <= 500);
		panic("u_on_sstairs: could not relocate player!");
#undef badspot
	}
}

// place you on upstairs (or special equivalent)
void u_on_upstairs(void) {
	if (xupstair) {
		u_on_newpos(xupstair, yupstair);
	} else
		u_on_sstairs();
}

// place you on dnstairs (or special equivalent)
void u_on_dnstairs(void) {
	if (xdnstair) {
		u_on_newpos(xdnstair, ydnstair);
	} else
		u_on_sstairs();
}

boolean On_stairs(xchar x, xchar y) {
	return (x == xupstair && y == yupstair) ||
	       (x == xdnstair && y == ydnstair) ||
	       (x == xdnladder && y == ydnladder) ||
	       (x == xupladder && y == yupladder) ||
	       (x == sstairs.sx && y == sstairs.sy);
}

boolean Is_botlevel(d_level *lev) {
	return lev->dlevel == dungeons[lev->dnum].num_dunlevs;
}

boolean Can_dig_down(d_level *lev) {
	return !level.flags.hardfloor && !Is_botlevel(lev) && !Invocation_lev(lev);
}

/*
 * Like Can_dig_down (above), but also allows falling through on the
 * stronghold level.  Normally, the bottom level of a dungeon resists
 * both digging and falling.
 */
boolean Can_fall_thru(d_level *lev) {
	return Can_dig_down(lev) || Is_stronghold(lev);
}

/*
 * True if one can rise up a level (e.g. cursed gain level).
 * This happens on intermediate dungeon levels or on any top dungeon
 * level that has a stairwell style branch to the next higher dungeon.
 * Checks for amulets and such must be done elsewhere.
 */
boolean Can_rise_up(int x, int y, d_level *lev) {
	/* can't rise up from inside the top of the Wizard's tower */
	/* KMH -- or in sokoban */
	if (In_endgame(lev) || In_sokoban(lev) ||
	    (Is_wiz1_level(lev) && In_W_tower(x, y, lev)))
		return false;
	return lev->dlevel > 1 ||
	       (dungeons[lev->dnum].entry_lev == 1 && ledger_no(lev) != 1 &&
		sstairs.sx && sstairs.up);
}

/*
 * It is expected that the second argument of get_level is a depth value,
 * either supplied by the user (teleport control) or randomly generated.
 * But more than one level can be at the same depth.  If the target level
 * is "above" the present depth location, get_level must trace "up" from
 * the player's location (through the ancestors dungeons) the dungeon
 * within which the target level is located.  With only one exception
 * which does not pass through this routine (see level_tele), teleporting
 * "down" is confined to the current dungeon.  At present, level teleport
 * in dungeons that build up is confined within them.
 */
void get_level(d_level *newlevel, int levnum) {
	branch *br;
	xchar dgn = u.uz.dnum;

	if (levnum <= 0) {
		/* can only currently happen in endgame */
		levnum = u.uz.dlevel;
	} else if (levnum > dungeons[dgn].depth_start + dungeons[dgn].num_dunlevs - 1) {
		/* beyond end of dungeon, jump to last level */
		levnum = dungeons[dgn].num_dunlevs;
	} else {
		/* The desired level is in this dungeon or a "higher" one. */

		/*
		 * Branch up the tree until we reach a dungeon that contains the
		 * levnum.
		 */
		if (levnum < dungeons[dgn].depth_start) {
			do {
				/*
				 * Find the parent dungeon of this dungeon.
				 *
				 * This assumes that end2 is always the "child" and it is
				 * unique.
				 */
				for (br = branches; br; br = br->next)
					if (br->end2.dnum == dgn) break;
				if (!br)
					panic("get_level: can't find parent dungeon");

				dgn = br->end1.dnum;
			} while (levnum < dungeons[dgn].depth_start);
		}

		/* We're within the same dungeon; calculate the level. */
		levnum = levnum - dungeons[dgn].depth_start + 1;
	}

	newlevel->dnum = dgn;
	newlevel->dlevel = levnum;
}

// are you in the quest dungeon?
boolean In_quest(d_level *lev) {
	return lev->dnum == quest_dnum;
}

// are you in the mines dungeon?
boolean In_mines(d_level *lev) {
	return lev->dnum == mines_dnum;
}

// are you in the spider dungeon?
boolean In_spiders(d_level *lev) {
	return lev->dnum == spiders_dnum;
}

/*
 * Return the branch for the given dungeon.
 *
 * This function assumes:
 *	+ This is not called with "Dungeons of Doom".
 *	+ There is only _one_ branch to a given dungeon.
 *	+ Field end2 is the "child" dungeon.
 */
branch *dungeon_branch(const char *s) {
	branch *br;
	xchar dnum;

	dnum = dname_to_dnum(s);

	/* Find the branch that connects to dungeon i's branch. */
	for (br = branches; br; br = br->next)
		if (br->end2.dnum == dnum) break;

	if (!br) panic("dgn_entrance: can't find entrance to %s", s);

	return br;
}

/*
 * This returns true if the hero is on the same level as the entrance to
 * the named dungeon.
 *
 * Called from do.c and mklev.c.
 *
 * Assumes that end1 is always the "parent".
 */
boolean at_dgn_entrance(const char *s) {
	branch *br;

	br = dungeon_branch(s);
	return on_level(&u.uz, &br->end1) ? true : false;
}

// is `lev' part of Vlad's tower?
boolean In_V_tower(d_level *lev) {
	return lev->dnum == tower_dnum;
}

// is `lev' a level containing the Wizard's tower?
boolean On_W_tower_level(d_level *lev) {
	return Is_wiz1_level(lev) ||
	       Is_wiz2_level(lev) ||
	       Is_wiz3_level(lev);
}

// is <x,y> of `lev' inside the Wizard's tower?
boolean In_W_tower(int x, int y, d_level *lev) {
	if (!On_W_tower_level(lev)) return false;
	/*
	 * Both of the exclusion regions for arriving via level teleport
	 * (from above or below) define the tower's boundary.
	 *	assert( updest.nIJ == dndest.nIJ for I={l|h},J={x|y} );
	 */
	if (dndest.nlx > 0)
		return within_bounded_area(x, y, dndest.nlx, dndest.nly, dndest.nhx, dndest.nhy);
	else
		impossible("No boundary for Wizard's Tower?");
	return false;
}

// are you in one of the Hell levels?
boolean In_hell(d_level *lev) {
	return dungeons[lev->dnum].flags.hellish;
}

// sets *lev to be the gateway to Gehennom...
void find_hell(d_level *lev) {
	lev->dnum = valley_level.dnum;
	lev->dlevel = 1;
}

// go directly to hell...
void goto_hell(boolean at_stairs, boolean falling) {
	d_level lev;

	find_hell(&lev);
	goto_level(&lev, at_stairs, falling, false);
}

// equivalent to dest = source
void assign_level(d_level *dest, d_level *src) {
	dest->dnum = src->dnum;
	dest->dlevel = src->dlevel;
}

// dest = src + rn1(range)
void assign_rnd_level(d_level *dest, d_level *src, int range) {
	dest->dnum = src->dnum;
	dest->dlevel = src->dlevel + ((range > 0) ? rnd(range) : -rnd(-range));

	if (dest->dlevel > dunlevs_in_dungeon(dest))
		dest->dlevel = dunlevs_in_dungeon(dest);
	else if (dest->dlevel < 1)
		dest->dlevel = 1;
}

int induced_align(int pct) {
	s_level *lev = Is_special(&u.uz);
	aligntyp al;

	if (lev && lev->flags.align)
		if (rn2(100) < pct) return lev->flags.align;

	if (dungeons[u.uz.dnum].flags.align)
		if (rn2(100) < pct) return dungeons[u.uz.dnum].flags.align;

	al = rn2(3) - 1;
	return Align2amask(al);
}

boolean Invocation_lev(d_level *lev) {
	return In_hell(lev) && lev->dlevel == (dungeons[lev->dnum].num_dunlevs - 1);
}

/* use instead of depth() wherever a degree of difficulty is made
 * dependent on the location in the dungeon (eg. monster creation).
 */
xchar level_difficulty(void) {
	if (In_endgame(&u.uz))
		return depth(&sanctum_level) + u.ulevel / 2;
	else if (u.uhave.amulet)
		return deepest_lev_reached(false);
	else
		return depth(&u.uz);
}

/* Take one word and try to match it to a level.
 * Recognized levels are as shown by print_dungeon().
 */
schar lev_by_name(const char *nam) {
	schar lev = 0;
	s_level *slev;
	d_level dlev;
	const char *p;
	int idx, idxtoo;
	char buf[BUFSZ];

	/* allow strings like "the oracle level" to find "oracle" */
	if (!strncmpi(nam, "the ", 4)) nam += 4;
	if ((p = strstri(nam, " level")) != 0 && p == eos((char *)nam) - 6) {
		nam = strcpy(buf, nam);
		*(eos(buf) - 6) = '\0';
	}
	/* hell is the old name, and wouldn't match; gehennom would match its
	   branch, yielding the castle level instead of the valley of the dead */
	if (!strcmpi(nam, "gehennom") || !strcmpi(nam, "hell")) {
		if (In_V_tower(&u.uz))
			nam = " to Vlad's tower"; /* branch to... */
		else
			nam = "valley";
	}

	if ((slev = find_level(nam)) != 0) {
		dlev = slev->dlevel;
		idx = ledger_no(&dlev);
		if ((dlev.dnum == u.uz.dnum ||
		     /* within same branch, or else main dungeon <-> gehennom */
		     (u.uz.dnum == valley_level.dnum &&
		      dlev.dnum == medusa_level.dnum) ||
		     (u.uz.dnum == medusa_level.dnum &&
		      dlev.dnum == valley_level.dnum)) &&
		    /* either wizard mode or else seen and not forgotten */
		    (wizard || (level_info[idx].flags & VISITED))) {
			lev = depth(&slev->dlevel);
		}
	} else { /* not a specific level; try branch names */
		idx = find_branch(nam, NULL);
		/* "<branch> to Xyzzy" */
		if (idx < 0 && (p = strstri(nam, " to ")) != 0)
			idx = find_branch(p + 4, NULL);

		if (idx >= 0) {
			idxtoo = (idx >> 8) & 0x00FF;
			idx &= 0x00FF;
			// either wizard mode, or else _both_ sides of branch seen
			if (wizard ||
			    ((level_info[idx].flags & VISITED) &&
			     (level_info[idxtoo].flags & VISITED) == VISITED)) {
				if (ledger_to_dnum(idxtoo) == u.uz.dnum) idx = idxtoo;
				dlev.dnum = ledger_to_dnum(idx);
				dlev.dlevel = ledger_to_dlev(idx);
				lev = depth(&dlev);
			}
		}
	}
	return lev;
}

/* Convert a branch type to a string usable by print_dungeon(). */
static const char *br_string(int type) {
	switch (type) {
		case BR_PORTAL:
			return "Portal";
		case BR_NO_END1:
			return "Connection";
		case BR_NO_END2:
			return "One way stair";
		case BR_STAIR:
			return "Stair";
	}
	return " (unknown)";
}

// Print all child branches between the lower and upper bounds.
static void print_branch(winid win, int dnum, int lower_bound, int upper_bound, boolean bymenu, struct lchoice *lchoices) {
	branch *br;
	char buf[BUFSZ];
	anything any;

	/* This assumes that end1 is the "parent". */
	for (br = branches; br; br = br->next) {
		if (br->end1.dnum == dnum && lower_bound < br->end1.dlevel &&
		    br->end1.dlevel <= upper_bound) {
			sprintf(buf, "   %s to %s: %d",
				br_string(br->type),
				dungeons[br->end2.dnum].dname,
				depth(&br->end1));
			if (bymenu) {
				lchoices->lev[lchoices->idx] = br->end1.dlevel;
				lchoices->dgn[lchoices->idx] = br->end1.dnum;
				lchoices->playerlev[lchoices->idx] = depth(&br->end1);
				any.a_void = 0;
				any.a_int = lchoices->idx + 1;
				add_menu(win, NO_GLYPH, &any, lchoices->menuletter,
					 0, ATR_NONE, buf, MENU_UNSELECTED);
				if (lchoices->menuletter == 'z')
					lchoices->menuletter = 'A';
				else
					lchoices->menuletter++;
				lchoices->idx++;
			} else
				putstr(win, 0, buf);
		}
	}
}

/* Print available dungeon information. */
schar print_dungeon(boolean bymenu, schar *rlev, xchar *rdgn) {
	int i, last_level, nlev;
	char buf[BUFSZ];
	boolean first;
	s_level *slev;
	dungeon *dptr;
	branch *br;
	anything any;
	struct lchoice lchoices;

	winid win = create_nhwindow(NHW_MENU);
	if (bymenu) {
		start_menu(win);
		lchoices.idx = 0;
		lchoices.menuletter = 'a';
	}

	for (i = 0, dptr = dungeons; i < n_dgns; i++, dptr++) {
		nlev = dptr->num_dunlevs;
		if (nlev > 1)
			sprintf(buf, "%s: levels %d to %d", dptr->dname, dptr->depth_start,
				dptr->depth_start + nlev - 1);
		else
			sprintf(buf, "%s: level %d", dptr->dname, dptr->depth_start);

		/* Most entrances are uninteresting. */
		if (dptr->entry_lev != 1) {
			if (dptr->entry_lev == nlev)
				strcat(buf, ", entrance from below");
			else
				sprintf(eos(buf), ", entrance on %d",
					dptr->depth_start + dptr->entry_lev - 1);
		}
		if (bymenu) {
			any.a_void = 0;
			add_menu(win, NO_GLYPH, &any, 0, 0, iflags.menu_headings, buf, MENU_UNSELECTED);
		} else
			putstr(win, 0, buf);

		/*
		 * Circle through the special levels to find levels that are in
		 * this dungeon.
		 */
		for (slev = sp_levchn, last_level = 0; slev; slev = slev->next) {
			if (slev->dlevel.dnum != i) continue;

			/* print any branches before this level */
			print_branch(win, i, last_level, slev->dlevel.dlevel, bymenu, &lchoices);

			sprintf(buf, "   %s: %d", slev->proto, depth(&slev->dlevel));
			if (Is_stronghold(&slev->dlevel))
				sprintf(eos(buf), " (tune %s)", tune);
			if (bymenu) {
				/* If other floating branches are added, this will need to change */
				if (i != knox_level.dnum) {
					lchoices.lev[lchoices.idx] = slev->dlevel.dlevel;
					lchoices.dgn[lchoices.idx] = i;
				} else {
					lchoices.lev[lchoices.idx] = depth(&slev->dlevel);
					lchoices.dgn[lchoices.idx] = 0;
				}
				lchoices.playerlev[lchoices.idx] = depth(&slev->dlevel);
				any.a_void = 0;
				any.a_int = lchoices.idx + 1;
				add_menu(win, NO_GLYPH, &any, lchoices.menuletter,
					 0, ATR_NONE, buf, MENU_UNSELECTED);
				if (lchoices.menuletter == 'z')
					lchoices.menuletter = 'A';
				else
					lchoices.menuletter++;
				lchoices.idx++;
			} else
				putstr(win, 0, buf);

			last_level = slev->dlevel.dlevel;
		}
		/* print branches after the last special level */
		print_branch(win, i, last_level, MAXLEVEL, bymenu, &lchoices);
	}

	/* Print out floating branches (if any). */
	for (first = true, br = branches; br; br = br->next) {
		if (br->end1.dnum == n_dgns) {
			if (first) {
				if (!bymenu) {
					putstr(win, 0, "");
					putstr(win, 0, "Floating branches");
				}
				first = false;
			}
			sprintf(buf, "   %s to %s",
				br_string(br->type), dungeons[br->end2.dnum].dname);
			if (!bymenu)
				putstr(win, 0, buf);
		}
	}
	if (bymenu) {
		int n;
		menu_item *selected;
		int idx;

		end_menu(win, "Level teleport to where:");
		n = select_menu(win, PICK_ONE, &selected);
		destroy_nhwindow(win);
		if (n > 0) {
			idx = selected[0].item.a_int - 1;
			free(selected);
			if (rlev && rdgn) {
				*rlev = lchoices.lev[idx];
				*rdgn = lchoices.dgn[idx];
				return lchoices.playerlev[idx];
			}
		}
		return 0;
	}

	/* I hate searching for the invocation pos while debugging. -dean */
	if (Invocation_lev(&u.uz)) {
		putstr(win, 0, "");
		sprintf(buf, "Invocation position @ (%d,%d), hero @ (%d,%d)",
			inv_pos.x, inv_pos.y, u.ux, u.uy);
		putstr(win, 0, buf);
	}
	/*
	 * The following is based on the assumption that the inter-level portals
	 * created by the level compiler (not the dungeon compiler) only exist
	 * one per level (currently true, of course).
	 */
	else if (Is_earthlevel(&u.uz) || Is_waterlevel(&u.uz) || Is_firelevel(&u.uz) || Is_airlevel(&u.uz)) {
		struct trap *trap;
		for (trap = ftrap; trap; trap = trap->ntrap)
			if (trap->ttyp == MAGIC_PORTAL) break;

		putstr(win, 0, "");
		if (trap)
			sprintf(buf, "Portal @ (%d,%d), hero @ (%d,%d)",
				trap->tx, trap->ty, u.ux, u.uy);
		else
			sprintf(buf, "No portal found.");
		putstr(win, 0, buf);
	}

	display_nhwindow(win, true);
	destroy_nhwindow(win);
	return 0;
}

/* Record that the player knows about a branch from a level. This function
 * will determine whether or not it was a "real" branch that was taken.
 * This function should not be called for a transition done via level
 * teleport or via the Eye.
 */
void recbranch_mapseen(d_level *source, d_level *dest) {
	mapseen *mptr;
	branch *br;

	/* not a branch */
	if (source->dnum == dest->dnum) return;

	/* we only care about forward branches */
	for (br = branches; br; br = br->next) {
		if (on_level(source, &br->end1) && on_level(dest, &br->end2)) break;
		if (on_level(source, &br->end2) && on_level(dest, &br->end1)) return;
	}

	/* branch not found, so not a real branch. */
	if (!br) return;

	if ((mptr = find_mapseen(source))) {
		if (mptr->br && br != mptr->br)
			impossible("Two branches on the same level?");
		mptr->br = br;
	} else {
		impossible("Can't note branch for unseen level (%d, %d)", source->dnum, source->dlevel);
	}
}

/* add a custom name to the current level */
int donamelevel(void) {
	mapseen *mptr;
	char qbuf[QBUFSZ]; /* Buffer for query text */
	char nbuf[BUFSZ];  /* Buffer for response */
	int len;

	if (!(mptr = find_mapseen(&u.uz))) return 0;

	sprintf(qbuf, "What do you want to call this dungeon level?");
	getlin(qbuf, nbuf);

	if (index(nbuf, '\033')) return 0;

	len = strlen(nbuf) + 1;
	if (mptr->custom) {
		free(mptr->custom);
		mptr->custom = NULL;
		mptr->custom_lth = 0;
	}

	if (*nbuf) {
		mptr->custom = new(char, len);
		mptr->custom_lth = len;
		strcpy(mptr->custom, nbuf);
	}

	return 0;
}

/* find the particular mapseen object in the chain */
/* may return 0 */
static mapseen *find_mapseen(d_level *lev) {
	mapseen *mptr;

	for (mptr = mapseenchn; mptr; mptr = mptr->next)
		if (on_level(&(mptr->lev), lev)) break;

	return mptr;
}

static void save_mapseen(int fd, mapseen *mptr) {
	branch *curr;
	int count;

	count = 0;
	for (curr = branches; curr; curr = curr->next) {
		if (curr == mptr->br) break;
		count++;
	}

	bwrite(fd, &count, sizeof(count));
	bwrite(fd, &mptr->lev, sizeof(d_level));
	bwrite(fd, &mptr->feat, sizeof(mapseen_feat));
	bwrite(fd, &mptr->custom_lth, sizeof(mptr->custom_lth));
	if (mptr->custom_lth)
		bwrite(fd, mptr->custom, mptr->custom_lth);
	bwrite(fd, &mptr->rooms, sizeof(mptr->rooms));
}

static mapseen *load_mapseen(int fd) {
	int branchnum, count;
	mapseen *load;
	branch *curr;

	load = (mapseen *)alloc(sizeof(mapseen));
	mread(fd, &branchnum, sizeof(int));

	count = 0;
	for (curr = branches; curr; curr = curr->next) {
		if (count == branchnum) break;
		count++;
	}
	load->br = curr;

	mread(fd, &load->lev, sizeof(d_level));
	mread(fd, &load->feat, sizeof(mapseen_feat));
	mread(fd, &load->custom_lth, sizeof(unsigned));
	if (load->custom_lth > 0) {
		load->custom = new(char, load->custom_lth);
		mread(fd, load->custom, load->custom_lth);
	} else {
		load->custom = NULL;
	}
	mread(fd, &load->rooms, sizeof(load->rooms));

	return load;
}

/* Remove all mapseen objects for a particular dnum.
 * Useful during quest expulsion to remove quest levels.
 */
void remdun_mapseen(int dnum) {
	mapseen *mptr, *prev;

	prev = mapseenchn;
	if (!prev) return;
	mptr = prev->next;

	for (; mptr; prev = mptr, mptr = mptr->next) {
		if (mptr->lev.dnum == dnum) {
			prev->next = mptr->next;
			free(mptr);
			mptr = prev;
		}
	}
}

void init_mapseen(d_level *lev) {
	/* Create a level and insert in "sorted" order.  This is an insertion
	 * sort first by dungeon (in order of discovery) and then by level number.
	 */
	mapseen *mptr;
	mapseen *init;
	mapseen *old;

	init = new(mapseen);
	memset(init, 0, sizeof(mapseen));
	init->lev.dnum = lev->dnum;
	init->lev.dlevel = lev->dlevel;

	if (!mapseenchn) {
		mapseenchn = init;
		return;
	}

	/* walk until we get to the place where we should
	 * insert init between mptr and mptr->next
	 */
	for (mptr = mapseenchn; mptr->next; mptr = mptr->next) {
		if (mptr->next->lev.dnum == init->lev.dnum) break;
	}
	for (; mptr->next; mptr = mptr->next) {
		if ((mptr->next->lev.dnum != init->lev.dnum) ||
		    (mptr->next->lev.dlevel > init->lev.dlevel)) break;
	}

	old = mptr->next;
	mptr->next = init;
	init->next = old;
}

#define INTEREST(feat)	   \
	((feat).nfount)	|| \
	((feat).nsink)	|| \
	((feat).ntoilet)|| \
	((feat).nthrone)|| \
	((feat).naltar) || \
	((feat).nshop)	|| \
	((feat).ntemple)|| \
	((feat).ntree)
/*
   || ((feat).water) || \
   ((feat).ice) || \
   ((feat).lava)
   */

/* returns true if this level has something interesting to print out */
static bool interest_mapseen(mapseen *mptr) {
	return (on_level(&u.uz, &mptr->lev) ||
		((!mptr->feat.forgot) &&
		 (INTEREST(mptr->feat) ||
		  (mptr->custom) ||
		  (mptr->br) ||
		  Is_special(&mptr->lev))));
}

/* recalculate mapseen for the current level */
void recalc_mapseen(void) {
	mapseen *mptr;
	struct monst *shkp;
	int x, y, ridx;

	/* Should not happen in general, but possible if in the process
	 * of being booted from the quest.  The mapseen object gets
	 * removed during the expulsion but prior to leaving the level
	 */
	if (!(mptr = find_mapseen(&u.uz))) return;

	/* reset all features */
	memset(&mptr->feat, 0, sizeof(mapseen_feat));

	/* track rooms the hero is in */
	for (x = 0; x < sizeof(u.urooms); x++) {
		if (!u.urooms[x]) continue;

		ridx = u.urooms[x] - ROOMOFFSET;
		if (rooms[ridx].rtype < SHOPBASE || ((shkp = shop_keeper(u.urooms[x])) && inhishop(shkp))) {
			mptr->rooms[ridx] |= MSR_SEEN;
		} else {
			/* shops without shopkeepers are no shops at all */
			mptr->rooms[ridx] &= ~MSR_SEEN;
		}
	}

	/* recalculate room knowledge: for now, just shops and temples
	 * this could be extended to an array of 0..SHOPBASE
	 */
	for (x = 0; x < sizeof(mptr->rooms); x++) {
		if (mptr->rooms[x] & MSR_SEEN) {
			if (rooms[x].rtype >= SHOPBASE) {
				if (!mptr->feat.nshop)
					mptr->feat.shoptype = rooms[x].rtype;
				else if (mptr->feat.shoptype != rooms[x].rtype)
					mptr->feat.shoptype = 0;
				mptr->feat.nshop = min(mptr->feat.nshop + 1, 3);
			} else if (rooms[x].rtype == TEMPLE)
				/* altar and temple alignment handled below */
				mptr->feat.ntemple = min(mptr->feat.ntemple + 1, 3);
		}
	}

	/* Update styp with typ if and only if it is in sight or the hero can
	 * feel it on their current location (i.e. not levitating).  This *should*
	 * give the "last known typ" for each dungeon location.  (At the very least,
	 * it's a better assumption than determining what the player knows from
	 * the glyph and the typ (which is isn't quite enough information in some
	 * cases).
	 *
	 * It was reluctantly added to struct rm to track.  Alternatively
	 * we could track "features" and then update them all here, and keep
	 * track of when new features are created or destroyed, but this
	 * seemed the most elegant, despite adding more data to struct rm.
	 *
	 * Although no current windowing systems (can) do this, this would add the
	 * ability to have non-dungeon glyphs float above the last known dungeon
	 * glyph (i.e. items on fountains).
	 *
	 * (vision-related styp update done in loop below)
	 */
	if (!Levitation)
		levl[u.ux][u.uy].styp = levl[u.ux][u.uy].typ;

	for (x = 0; x < COLNO; x++) {
		for (y = 0; y < ROWNO; y++) {
			/* update styp from viz_array */
			if (viz_array[y][x] & IN_SIGHT)
				levl[x][y].styp = levl[x][y].typ;

			switch (levl[x][y].styp) {
				/*
				   case ICE:
				   mptr->feat.ice = 1;
				   break;
				   case POOL:
				   case MOAT:
				   case WATER:
				   mptr->feat.water = 1;
				   break;
				   case LAVAPOOL:
				   mptr->feat.lava = 1;
				   break;
				   */
				case TREE:
					mptr->feat.ntree = min(mptr->feat.ntree + 1, 3);
					break;
				case FOUNTAIN:
					mptr->feat.nfount = min(mptr->feat.nfount + 1, 3);
					break;
				case THRONE:
					mptr->feat.nthrone = min(mptr->feat.nthrone + 1, 3);
					break;
				case SINK:
					mptr->feat.nsink = min(mptr->feat.nsink + 1, 3);
					break;
				case TOILET:
					mptr->feat.ntoilet = min(mptr->feat.ntoilet + 1, 3);
					break;
				case ALTAR:
					if (!mptr->feat.naltar)
						mptr->feat.msalign = Amask2msa(levl[x][y].altarmask);
					else if (mptr->feat.msalign != Amask2msa(levl[x][y].altarmask))
						mptr->feat.msalign = MSA_NONE;

					mptr->feat.naltar = min(mptr->feat.naltar + 1, 3);
					break;
			}
		}
	}
}

int dooverview(void) {
	winid win;
	mapseen *mptr;
	bool printdun;
	int lastdun;

	bool first = true;

	/* lazy intialization */
	recalc_mapseen();

	win = create_nhwindow(NHW_MENU);

	for (mptr = mapseenchn; mptr; mptr = mptr->next) {
		/* only print out info for a level or a dungeon if interest */
		if (interest_mapseen(mptr)) {
			printdun = (first || lastdun != mptr->lev.dnum);
			/* if (!first) putstr(win, 0, ""); */
			print_mapseen(win, mptr, printdun);

			if (printdun) {
				first = false;
				lastdun = mptr->lev.dnum;
			}
		}
	}

	display_nhwindow(win, true);
	destroy_nhwindow(win);

	return 0;
}

static char *seen_string(xchar x, const char *obj) {
	/* players are computer scientists: 0, 1, 2, n */
	switch (x) {
		case 0:
			return "no";
			/* an() returns too much.  index is ok in this case */
		case 1: return index(vowels, *obj) ? "an" : "a";
		case 2: return "some";
		case 3: return "many";
	}

	return "(unknown)";
}

/* better br_string */
static const char *br_string2(branch *br) {
	/* Special case: quest portal says closed if kicked from quest */
	bool closed_portal = br->end2.dnum == quest_dnum && u.uevent.qexpelled;
	switch (br->type) {
		case BR_PORTAL: return closed_portal ? "Sealed portal" : "Portal";
		case BR_NO_END1: return "Connection";
		case BR_NO_END2: return br->end1_up ? "One way stairs up" : "One way stairs down";
		case BR_STAIR: return br->end1_up ? "Stairs up" : "Stairs down";
	}

	return "(unknown)";
}

static const char *shop_string(int rtype) {
	/* Yuck, redundancy...but shclass.name doesn't cut it as a noun */
	switch (rtype) {
		case SHOPBASE:	return "general store";
		case ARMORSHOP:	return "armor shop";
		case SCROLLSHOP:return "scroll shop";
		case POTIONSHOP:return "potion shop";
		case WEAPONSHOP:return "weapon shop";
		case FOODSHOP:	return "delicatessen";
		case RINGSHOP:	return "jewelers";
		case WANDSHOP:	return "wand shop";
		case TOOLSHOP:	return "tool shop";
		case PETSHOP:	return "pet store";
		case TINSHOP:	return "tin shop";
		case BOOKSHOP:	return "bookstore";
		case HEALTHSHOP:return "health foods mart";
		case CANDLESHOP:return "lighting shop";
		case BLACKSHOP:	return "The Black Market";
		default:
			/* In case another patch adds a shop type that doesn't exist,
			 * do something reasonable like "a shop".
			 */
			impossible("Unknown shop type %d??", rtype);
			return "shop";
	}
}

/* some utility macros for print_mapseen */
#define TAB    "   "
#define BULLET ""
#define PREFIX TAB TAB BULLET
#define COMMA  (i++ > 0 ? ", " : PREFIX)
#define ADDNTOBUF(nam, var)                                                                   \
	do {                                                                                  \
		if (var)                                                                      \
			sprintf(eos(buf), "%s%s " nam "%s", COMMA, seen_string((var), (nam)), \
				((var) != 1 ? "s" : ""));                                     \
	} while (0)
#define ADDTOBUF(nam, var)                                    \
	do {                                                  \
		if (var) sprintf(eos(buf), "%s " nam, COMMA); \
	} while (0)

static void print_mapseen(winid win, mapseen *mptr, bool printdun) {
	char buf[BUFSZ];
	int i, depthstart;

	/* Damnable special cases */
	/* The quest and knox should appear to be level 1 to match
	 * other text.
	 */
	if (mptr->lev.dnum == quest_dnum || mptr->lev.dnum == knox_level.dnum) {
		depthstart = 1;
	} else {
		depthstart = dungeons[mptr->lev.dnum].depth_start;
	}

	if (printdun) {
		/* Sokoban lies about dunlev_ureached and we should
		 * suppress the negative numbers in the endgame.
		 */
		if (dungeons[mptr->lev.dnum].dunlev_ureached == 1 ||
		    mptr->lev.dnum == sokoban_dnum || In_endgame(&mptr->lev))
			sprintf(buf, "%s:", dungeons[mptr->lev.dnum].dname);
		else
			sprintf(buf, "%s: levels %d to %d",
				dungeons[mptr->lev.dnum].dname,
				depthstart, depthstart + dungeons[mptr->lev.dnum].dunlev_ureached - 1);
		putstr(win, ATR_INVERSE, buf);
	}

	/* calculate level number */
	i = depthstart + mptr->lev.dlevel - 1;
	if (Is_astralevel(&mptr->lev)) {
		sprintf(buf, TAB "Astral Plane:");
	} else if (In_endgame(&mptr->lev)) {
		/* Negative numbers are mildly confusing, since they are never
		 * shown to the player, except in wizard mode.  We could show
		 * "Level -1" for the earth plane, for example.  Instead,
		 * show "Plane 1" for the earth plane to differentiate from
		 * level 1.  There's not much to show, but maybe the player
		 * wants to #annotate them for some bizarre reason.
		 */
		sprintf(buf, TAB "Plane %d:", -i);
	} else {
		sprintf(buf, TAB "Level %d:", i);
	}

	/* wizmode prints out proto dungeon names for clarity */
	if (wizard) {
		if (depth(&mptr->lev) == poisoned_pit_threshold) {
			strcat(buf, " [poison start]");
		}
		s_level *slev;
		if ((slev = Is_special(&mptr->lev)))
			sprintf(eos(buf), " [%s]", slev->proto);
	}

	if (mptr->custom)
		sprintf(eos(buf), " (%s)", mptr->custom);

	/* print out glyph or something more interesting? */
	if (on_level(&u.uz, &mptr->lev)) {
		strcat(buf, " <- You are here");
	}
	putstr(win, ATR_BOLD, buf);

	if (mptr->feat.forgot) return;

	if (INTEREST(mptr->feat)) {
		buf[0] = 0;

		i = 0; /* interest counter */

		/* List interests in an order vaguely corresponding to
		 * how important they are.
		 */
		if (mptr->feat.nshop > 1) {
			ADDNTOBUF("shop", mptr->feat.nshop);
		} else if (mptr->feat.nshop == 1) {
			sprintf(eos(buf), "%s%s", COMMA, an(shop_string(mptr->feat.shoptype)));
		}

		/* Temples + non-temple altars get munged into just "altars" */
		if (!mptr->feat.ntemple || mptr->feat.ntemple != mptr->feat.naltar) {
			ADDNTOBUF("altar", mptr->feat.naltar);
		} else {
			ADDNTOBUF("temple", mptr->feat.ntemple);
		}

		/* only print out altar's god if they are all to your god */
		if (Amask2align(Msa2amask(mptr->feat.msalign)) == u.ualign.type)
			sprintf(eos(buf), " to %s", align_gname(u.ualign.type));

		ADDNTOBUF("fountain", mptr->feat.nfount);
		ADDNTOBUF("sink", mptr->feat.nsink);
		ADDNTOBUF("toilet", mptr->feat.ntoilet);
		ADDNTOBUF("throne", mptr->feat.nthrone);
		ADDNTOBUF("tree", mptr->feat.ntree);
		/*
		   ADDTOBUF("water", mptr->feat.water);
		   ADDTOBUF("lava", mptr->feat.lava);
		   ADDTOBUF("ice", mptr->feat.ice);
		   */

		/* capitalize afterwards */
		i = strlen(PREFIX);
		buf[i] = toupper(buf[i]);

		putstr(win, 0, buf);
	}

	/* print out branches */
	if (mptr->br) {
		sprintf(buf, PREFIX "%s to %s", br_string2(mptr->br),
			dungeons[mptr->br->end2.dnum].dname);

		/* since mapseen objects are printed out in increasing order
		 * of dlevel, clarify which level this branch is going to
		 * if the branch goes upwards.  Unless it's the end game
		 */
		if (mptr->br->end1_up && !In_endgame(&(mptr->br->end2)))
			sprintf(eos(buf), ", level %d", depth(&(mptr->br->end2)));
		putstr(win, 0, buf);
	}
}

/*dungeon.c*/
