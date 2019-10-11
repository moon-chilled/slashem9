/*	SCCS Id: @(#)topten.c	3.4	2000/01/21	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"
#include "patchlevel.h"

#define done_stopprint program_state.stopprint

#define newttentry() alloc(sizeof(struct toptenentry))
#define dealloc_ttentry(ttent) free((ttent))
#define NAMSZ	10
#define DTHSZ	100
#define ROLESZ   3
#define PERSMAX	 3		/* entries per name/uid per char. allowed */
#define POINTSMIN	1	/* must be > 0 */
#define ENTRYMAX	100	/* must be >= 10 */

#if !defined(MAC) && !defined(WIN32)
#define PERS_IS_UID		/* delete for PERSMAX per name; now per uid */
#endif
struct toptenentry {
	struct toptenentry *tt_next;
	long points;
	int deathdnum, deathlev;
	int maxlvl, hp, maxhp, deaths;
	int ver_major, ver_minor, patchlevel;
	long deathdate, birthdate;
	long conduct;
	int uid;
	char plrole[ROLESZ+1];
	char plrace[ROLESZ+1];
	char plgend[ROLESZ+1];
	char plalign[ROLESZ+1];
	char name[NAMSZ+1];
	char death[DTHSZ+1];
} *tt_head;

static void topten_print(const char *);
static void topten_print_bold(const char *);
static xchar observable_depth(d_level *);
static void outheader(void);
static void outentry(int,struct toptenentry *,boolean);
static void readentry(FILE *,struct toptenentry *);
static void writeentry(FILE *,struct toptenentry *);

static void munge_xlstring(char *dest, char *src, int n);
static void write_xlentry(FILE *,struct toptenentry *);

static void free_ttlist(struct toptenentry *);
static int classmon(char *,boolean);
static int score_wanted(boolean, int,struct toptenentry *,int,const char **,int);

static long encodeconduct(void);
static long encodeachieve(void);

static long encodeconduct(void);

/* must fit with end.c; used in rip.c */
const char * const killed_by_prefix[] = {
	"killed by ", "betrayed by ", "choked on ", "poisoned by ", "died of ",
	"drowned in ", "burned by ", "dissolved in ", "crushed to death by ",
	"petrified by ", "turned to slime by ", "killed by ",
	"", "", "", "", ""
};

static winid toptenwin = WIN_ERR;

static time_t deathtime = 0L;

static void topten_print(const char *x) {
	if (toptenwin == WIN_ERR)
	    raw_print(x);
	else
	    putstr(toptenwin, ATR_NONE, x);
}

static void topten_print_bold(const char *x) {
	if (toptenwin == WIN_ERR)
	    raw_print_bold(x);
	else
	    putstr(toptenwin, ATR_BOLD, x);
}

static xchar observable_depth(d_level *lev) {
#if 0	/* if we ever randomize the order of the elemental planes, we
	   must use a constant external representation in the record file */
	if (In_endgame(lev)) {
	    if (Is_astralevel(lev))	 return -5;
	    else if (Is_waterlevel(lev)) return -4;
	    else if (Is_firelevel(lev))	 return -3;
	    else if (Is_airlevel(lev))	 return -2;
	    else if (Is_earthlevel(lev)) return -1;
	    else			 return 0;	/* ? */
	} else
#endif
	    return depth(lev);
}

static void readentry(FILE *rfile, struct toptenentry *tt) {
	static const char fmt[] = "%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ";
	static const char fmt005[] = "%s %c %[^,],%[^\n]%*c";
	static const char fmt33[] = "%s %s %s %s %[^,],%[^\n]%*c";

#define TTFIELDS 13

	tt->conduct = 4095;

	if(fscanf(rfile, fmt,
			&tt->ver_major, &tt->ver_minor, &tt->patchlevel,
			&tt->points, &tt->deathdnum, &tt->deathlev,
			&tt->maxlvl, &tt->hp, &tt->maxhp, &tt->deaths,
			&tt->deathdate, &tt->birthdate,
			&tt->uid) != TTFIELDS)
#undef TTFIELDS
		tt->points = 0;
	else {
		/* Check for backwards compatibility */
		if (!tt->ver_major && !tt->ver_minor && tt->patchlevel < 6) {
			int i;

		    if (fscanf(rfile, fmt005,
				tt->plrole, tt->plgend,
				tt->name, tt->death) != 4)
			tt->points = 0;
		    tt->plrole[1] = '\0';
		    if ((i = str2role(tt->plrole)) >= 0)
			strcpy(tt->plrole, roles[i].filecode);
		    tt->plrole[ROLESZ] = 0;
		    strcpy(tt->plrace, "?");
		    strcpy(tt->plgend, (tt->plgend[0] == 'M') ? "Mal" : "Fem");
		    strcpy(tt->plalign, "?");
		} else if (fscanf(rfile, fmt33,
				tt->plrole, tt->plrace, tt->plgend,
				tt->plalign, tt->name, tt->death) != 6)
			tt->points = 0;

		if(tt->points > 0) {
			/* If the string "Conduct=%d" appears, set tt->conduct and remove that
			 * portion of the string */
			char *dp, *dp2;
			for(dp = tt->death; *dp; dp++) {
				if(!strncmp(dp, " Conduct=", 9)) {
					dp2 = dp + 9;
					sscanf(dp2, "%ld", &tt->conduct);
					/* Find trailing null or space */
					while(*dp2 && *dp2 != ' ')
						dp2++;

					/* Cut out the " Conduct=" portion of the death string */
					while(*dp2) {
						*dp = *dp2;
						dp2++;
						dp++;
					}

					*dp = *dp2;
				}
			}

			/* Sanity check */
			if(tt->conduct < 0 || tt->conduct > 4095)
				tt->conduct = 4095;
		}
	}

	/* check old score entries for Y2K problem and fix whenever found */
	if (tt->points > 0) {
		if (tt->birthdate < 19000000L) tt->birthdate += 19000000L;
		if (tt->deathdate < 19000000L) tt->deathdate += 19000000L;
	}
}

static void writeentry(FILE *rfile, struct toptenentry *tt) {
	char *cp = eos(tt->death);

	/* Add a trailing " Conduct=%d" to tt->death */
	sprintf(cp, " Conduct=%ld", tt->conduct);

	fprintf(rfile,"%d.%d.%d %ld %d %d %d %d %d %d %ld %ld %d ",
		tt->ver_major, tt->ver_minor, tt->patchlevel,
		tt->points, tt->deathdnum, tt->deathlev,
		tt->maxlvl, tt->hp, tt->maxhp, tt->deaths,
		tt->deathdate, tt->birthdate, tt->uid);
	if (!tt->ver_major && !tt->ver_minor && tt->patchlevel < 6)
		fprintf(rfile,"%s %c %s,%s\n",
			tt->plrole, tt->plgend[0],
			onlyspace(tt->name) ? "_" : tt->name, tt->death);
	else
		fprintf(rfile,"%s %s %s %s %s,%s\n",
			tt->plrole, tt->plrace, tt->plgend, tt->plalign,
			onlyspace(tt->name) ? "_" : tt->name, tt->death);


	/* Return the tt->death line to the original form */
	*cp = '\0';
}

#define SEP "\t"

/* copy a maximum of n-1 characters from src to dest, changing ':' and '\n'
 * to '_'; always null-terminate. */
static void munge_xlstring(char *dest, char *src, int n) {
	int i;

	for(i = 0; i < (n - 1) && src[i] != '\0'; i++) {
		if(src[i] == '\n')
			dest[i] = '_';
		else
			dest[i] = src[i];
	}

	dest[i] = '\0';
}

static void write_xlentry(FILE *rfile, struct toptenentry *tt) {

  char buf[DTHSZ+1];

  /* Log all of the data found in the regular logfile */
  fprintf(rfile,
                "version=%d.%d.%d"
                SEP "points=%ld"
                SEP "deathdnum=%d"
                SEP "deathlev=%d"
                SEP "maxlvl=%d"
                SEP "hp=%d"
                SEP "maxhp=%d"
                SEP "deaths=%d"
                SEP "deathdate=%ld"
                SEP "birthdate=%ld"
                SEP "uid=%d",
                tt->ver_major, tt->ver_minor, tt->patchlevel,
                tt->points, tt->deathdnum, tt->deathlev,
                tt->maxlvl, tt->hp, tt->maxhp, tt->deaths,
                tt->deathdate, tt->birthdate, tt->uid);

  fprintf(rfile,
                SEP "role=%s"
                SEP "race=%s"
                SEP "gender=%s"
                SEP "align=%s",
                tt->plrole, tt->plrace, tt->plgend, tt->plalign);

   munge_xlstring(buf, plname, DTHSZ + 1);
  fprintf(rfile, SEP "name=%s", buf);

   munge_xlstring(buf, tt->death, DTHSZ + 1);
  fprintf(rfile, SEP "death=%s", buf);

  fprintf(rfile, SEP "conduct=0x%lx", encodeconduct());

  fprintf(rfile, SEP "turns=%ld", moves);

  fprintf(rfile, SEP "achieve=0x%lx", encodeachieve());

  fprintf(rfile, SEP "realtime=%ld", (long)realtime_data.realtime);

  fprintf(rfile, SEP "starttime=%ld", (long)u.ubirthday);
  fprintf(rfile, SEP "endtime=%ld", (long)deathtime);

  fprintf(rfile, SEP "gender0=%s", genders[flags.initgend].filecode);

  fprintf(rfile, SEP "align0=%s",
          aligns[1 - u.ualignbase[A_ORIGINAL]].filecode);

  fprintf(rfile, "\n");

}
#undef SEP

static void free_ttlist(struct toptenentry *tt) {
	struct toptenentry *ttnext;

	while (tt->points > 0) {
		ttnext = tt->tt_next;
		dealloc_ttentry(tt);
		tt = ttnext;
	}
	dealloc_ttentry(tt);
}

void topten (int how) {
	int uid = getuid();
	int rank, rank0 = -1, rank1 = 0;
	int occ_cnt = PERSMAX;
	struct toptenentry *t0, *tprev;
	struct toptenentry *t1;
	FILE *rfile;
	int flg = 0;
	boolean t0_used;
	FILE *xlfile;

/* If we are in the midst of a panic, cut out topten entirely.
 * topten uses alloc() several times, which will lead to
 * problems if the panic was the result of an alloc() failure.
 */
	if (program_state.panicking)
		return;

	if (flags.toptenwin) {
	    toptenwin = create_nhwindow(NHW_TEXT);
	}

#if defined(UNIX) || defined(__EMX__)
#define HUP	if (!program_state.done_hup)
#else
#define HUP
#endif

	/* create a new 'topten' entry */
	t0_used = false;
	t0 = newttentry();
	/* deepest_lev_reached() is in terms of depth(), and reporting the
	 * deepest level reached in the dungeon death occurred in doesn't
	 * seem right, so we have to report the death level in depth() terms
	 * as well (which also seems reasonable since that's all the player
	 * sees on the screen anyway)
	 */
	t0->ver_major = VERSION_MAJOR;
	t0->ver_minor = VERSION_MINOR;
	t0->patchlevel = PATCHLEVEL;
	t0->points = u.urexp;
	t0->deathdnum = u.uz.dnum;
	t0->deathlev = observable_depth(&u.uz);
	t0->maxlvl = deepest_lev_reached(true);
	t0->hp = u.uhp;
	t0->maxhp = u.uhpmax;
	t0->deaths = u.umortality;
	t0->uid = uid;
	strncpy(t0->plrole, urole.filecode, ROLESZ);
	t0->plrole[ROLESZ] = '\0';
	strncpy(t0->plrace, urace.filecode, ROLESZ);
	t0->plrace[ROLESZ] = '\0';
	strncpy(t0->plgend, genders[flags.female].filecode, ROLESZ);
	t0->plgend[ROLESZ] = '\0';
	strncpy(t0->plalign, aligns[1-u.ualign.type].filecode, ROLESZ);
	t0->plalign[ROLESZ] = '\0';
	strncpy(t0->name, plname, NAMSZ);
	t0->name[NAMSZ] = '\0';
	t0->death[0] = '\0';
	switch (killer_format) {
		default: impossible("bad killer format?");
		case KILLED_BY_AN:
			strcat(t0->death, killed_by_prefix[how]);
			strncat(t0->death, an(killer),
						DTHSZ-strlen(t0->death));
			break;
		case KILLED_BY:
			strcat(t0->death, killed_by_prefix[how]);
			strncat(t0->death, killer,
						DTHSZ-strlen(t0->death));
			break;
		case NO_KILLER_PREFIX:
			strncat(t0->death, killer, DTHSZ);
			break;
	}
	t0->birthdate = yyyymmdd(u.ubirthday);

  /* Make sure that deathdate and deathtime refer to the same time; it
   * wouldn't be good to have deathtime refer to the day after deathdate. */
# if defined(BSD) && !defined(POSIX_TYPES)
	time((long *)&deathtime);
# else
	time(&deathtime);
# endif
	t0->deathdate = yyyymmdd(deathtime);

	t0->conduct = encodeconduct();
	t0->tt_next = 0;

         if(lock_file(XLOGFILE, SCOREPREFIX, 10)) {
             if(!(xlfile = fopen_datafile(XLOGFILE, "a", SCOREPREFIX))) {
                  HUP raw_print("Cannot open extended log file!");
             } else {
                  write_xlentry(xlfile, t0);
                  fclose(xlfile);
             }
             unlock_file(XLOGFILE);
         }

	if (wizard || discover) {
	    if (how != PANICKED) HUP {
		char pbuf[BUFSZ];
		topten_print("");
		sprintf(pbuf,
	      "Since you were in %s mode, the score list will not be checked.",
		    wizard ? "wizard" : "discover");
		topten_print(pbuf);
#ifdef DUMP_LOG
		if (dump_fn[0]) {
		  dump("", pbuf);
		  dump("", "");
		}
#endif
	    }
	    goto showwin;
	}

	if (!lock_file(NH_RECORD, SCOREPREFIX, 60))
		goto destroywin;

	rfile = fopen_datafile( NH_RECORD, "r", SCOREPREFIX);

	if (!rfile) {
		HUP raw_print("Cannot open record file!");
		unlock_file(NH_RECORD);
		goto destroywin;
	}

	HUP topten_print("");
#ifdef DUMP_LOG
	dump("", "");
#endif

	/* assure minimum number of points */
	if(t0->points < POINTSMIN) t0->points = 0;

	t1 = tt_head = newttentry();
	tprev = 0;
	/* rank0: -1 undefined, 0 not_on_list, n n_th on list */
	for(rank = 1; ; ) {
	    readentry(rfile, t1);
	    if (t1->points < POINTSMIN) t1->points = 0;
	    if(rank0 < 0 && t1->points < t0->points) {
		rank0 = rank++;
		if(tprev == 0)
			tt_head = t0;
		else
			tprev->tt_next = t0;
		t0->tt_next = t1;
		t0_used = true;
		occ_cnt--;
		flg++;		/* ask for a rewrite */
	    } else tprev = t1;

	    if(t1->points == 0) break;
	    if(
#ifdef PERS_IS_UID
		t1->uid == t0->uid &&
#else
		strncmp(t1->name, t0->name, NAMSZ) == 0 &&
#endif
		!strncmp(t1->plrole, t0->plrole, ROLESZ) &&
		--occ_cnt <= 0) {
		    if(rank0 < 0) {
			rank0 = 0;
			rank1 = rank;
			HUP {
			    char pbuf[BUFSZ];
			    sprintf(pbuf,
			  "You didn't beat your previous score of %ld points.",
				    t1->points);
			    topten_print(pbuf);
			    topten_print("");
#ifdef DUMP_LOG
			    dump("", pbuf);
			    dump("", "");
#endif
			}
		    }
		    if(occ_cnt < 0) {
			flg++;
			continue;
		    }
		}
	    if(rank <= ENTRYMAX) {
		t1->tt_next = newttentry();
		t1 = t1->tt_next;
		rank++;
	    }
	    if(rank > ENTRYMAX) {
		t1->points = 0;
		break;
	    }
	}
	if(flg) {	/* rewrite record file */
		fclose(rfile);
		if(!(rfile = fopen_datafile(NH_RECORD, "w", SCOREPREFIX))){
			HUP raw_print("Cannot write record file");
			unlock_file(NH_RECORD);
			free_ttlist(tt_head);
			goto destroywin;
		}
		if(!done_stopprint) if(rank0 > 0){
		    if(rank0 <= 10) {
			topten_print("You made the top ten list!");
#ifdef DUMP_LOG
			dump("", "You made the top ten list!");
#endif
		    } else {
			char pbuf[BUFSZ];
			sprintf(pbuf,
			  "You reached the %d%s place on the top %d list.",
				rank0, ordin(rank0), ENTRYMAX);
			topten_print(pbuf);
#ifdef DUMP_LOG
			dump("", pbuf);
#endif
		    }
		    topten_print("");
#ifdef DUMP_LOG
		    dump("", "");
#endif
		}
	}
	if(rank0 == 0) rank0 = rank1;
	if(rank0 <= 0) rank0 = rank;
	if(!done_stopprint) outheader();
	t1 = tt_head;
	for(rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
	    if(flg) writeentry(rfile, t1);
	    if (done_stopprint) continue;
	    if (rank > flags.end_top &&
		    (rank < rank0 - flags.end_around ||
		     rank > rank0 + flags.end_around) &&
		    (!flags.end_own ||
#ifdef PERS_IS_UID
					t1->uid != t0->uid
#else
					strncmp(t1->name, t0->name, NAMSZ)
#endif
		)) continue;
	    if (rank == rank0 - flags.end_around &&
		    rank0 > flags.end_top + flags.end_around + 1 &&
		    !flags.end_own) {
		topten_print("");
#ifdef DUMP_LOG
		dump("", "");
#endif
	    }
	    if(rank != rank0)
		outentry(rank, t1, false);
	    else if(!rank1)
		outentry(rank, t1, true);
	    else {
		outentry(rank, t1, true);
		outentry(0, t0, true);
	    }
	}
	if(rank0 >= rank) if(!done_stopprint)
		outentry(0, t0, true);
	fclose(rfile);
	unlock_file(NH_RECORD);
	free_ttlist(tt_head);

  showwin:
	if (flags.toptenwin && !done_stopprint) display_nhwindow(toptenwin, 1);
  destroywin:
	if (!t0_used) dealloc_ttentry(t0);
	if (flags.toptenwin) {
	    destroy_nhwindow(toptenwin);
	    toptenwin=WIN_ERR;
	}
}

static void
outheader()
{
	char linebuf[BUFSZ];
	char *bp;

	strcpy(linebuf, " No  Points     Name");
	bp = eos(linebuf);
	while(bp < linebuf + COLNO - 9) *bp++ = ' ';
	strcpy(bp, "Hp [max]");
	topten_print(linebuf);
#ifdef DUMP_LOG
	dump("", linebuf);
#endif
}

/* so>0: standout line; so=0: ordinary line */
static void outentry(int rank, struct toptenentry *t1, boolean so) {
	boolean second_line = true;
	char linebuf[BUFSZ];
	char *bp, hpbuf[24], linebuf3[BUFSZ];
	int hppos, lngr;


	linebuf[0] = '\0';
	if (rank) sprintf(eos(linebuf), "%3d", rank);
	else strcat(linebuf, "   ");

	sprintf(eos(linebuf), " %10ld  %.10s", t1->points, t1->name);
	sprintf(eos(linebuf), "-%s", t1->plrole);
	if (t1->plrace[0] != '?')
		sprintf(eos(linebuf), "-%s", t1->plrace);
	/* Printing of gender and alignment is intentional.  It has been
	 * part of the NetHack Geek Code, and illustrates a proper way to
	 * specify a character from the command line.
	 */
	sprintf(eos(linebuf), "-%s", t1->plgend);
	if (t1->plalign[0] != '?')
		sprintf(eos(linebuf), "-%s ", t1->plalign);
	else
		strcat(linebuf, " ");
	if (!strncmp("escaped", t1->death, 7)) {
	    sprintf(eos(linebuf), "escaped the dungeon %s[max level %d]",
		    !strncmp(" (", t1->death + 7, 2) ? t1->death + 7 + 2 : "",
		    t1->maxlvl);
	    /* fixup for closing paren in "escaped... with...Amulet)[max..." */
	    if ((bp = index(linebuf, ')')) != 0)
		*bp = (t1->deathdnum == astral_level.dnum) ? '\0' : ' ';
	    second_line = false;
	} else if (!strncmp("ascended", t1->death, 8)) {

		/* Add a notation for conducts kept */
		if(t1->conduct != 4095) {
			int i, m;
			char dash = 0, skip;
			const char *conduct_names[] = {
				"Food", "Vgn", "Vgt", "Ath", "Weap", "Pac",
				"Ill", "Poly", "Form", "Wish", "Art", "Geno",
				NULL };

			strcat(eos(linebuf), "(");
			for(i = 0, m = 1; conduct_names[i]; i += skip + 1, m <<= (skip + 1)) {
				skip = 0;
				if(t1->conduct & m)
					continue;

				/* Only show one of foodless, vegan, vegetarian */
				if(i == 0) skip = 2;
				if(i == 1) skip = 1;

				/* Only show one of wishless, artiwishless */
				if(i == 9) skip = 1;

				/* Add a hyphen for multiple conducts */
				if(dash) strcat(eos(linebuf), "-");
				strcat(eos(linebuf), conduct_names[i]);
				dash = 1;
			}
			strcat(eos(linebuf), ") ");
		}

	    sprintf(eos(linebuf), "ascended to demigod%s-hood",
		    (t1->plgend[0] == 'F') ? "dess" : "");
	    second_line = false;
	} else {
	    if (!strncmp(t1->death, "quit", 4)) {
		strcat(linebuf, "quit");
		second_line = false;
	    } else if (!strncmp(t1->death, "died of st", 10)) {
		strcat(linebuf, "starved to death");
		second_line = false;
	    } else if (!strncmp(t1->death, "choked", 6)) {
		sprintf(eos(linebuf), "choked on h%s food",
			(t1->plgend[0] == 'F') ? "er" : "is");
	    } else if (!strncmp(t1->death, "poisoned", 8)) {
		strcat(linebuf, "was poisoned");
	    } else if (!strncmp(t1->death, "crushed", 7)) {
		strcat(linebuf, "was crushed to death");
	    } else if (!strncmp(t1->death, "petrified by ", 13)) {
		strcat(linebuf, "turned to stone");
	    } else strcat(linebuf, "died");

	    if (t1->deathdnum == astral_level.dnum) {
		int deathlev = t1->deathlev;
		const char *arg, *fmt = " on the Plane of %s";

		if (!t1->ver_major && !t1->ver_minor && t1->patchlevel < 7)
			deathlev--;

		switch (deathlev) {
		case -5:
			fmt = " on the %s Plane";
			arg = "Astral";	break;
		case -4:
			arg = "Water";	break;
		case -3:
			arg = "Fire";	break;
		case -2:
			arg = "Air";	break;
		case -1:
			arg = "Earth";	break;
		default:
			arg = "Void";	break;
		}
		sprintf(eos(linebuf), fmt, arg);
	    } else {
		sprintf(eos(linebuf), " in %s", dungeons[t1->deathdnum].dname);
		if (t1->deathdnum != knox_level.dnum)
		    sprintf(eos(linebuf), " on level %d", t1->deathlev);
		if (t1->deathlev != t1->maxlvl)
		    sprintf(eos(linebuf), " [max %d]", t1->maxlvl);
	    }

	    /* kludge for "quit while already on Charon's boat" */
	    if (!strncmp(t1->death, "quit ", 5))
		strcat(linebuf, t1->death + 4);
	}
	strcat(linebuf, ".");

	/* Quit, starved, ascended, and escaped contain no second line */
	if (second_line)
	    sprintf(eos(linebuf), "  %c%s.", highc(*(t1->death)), t1->death+1);

	lngr = (int)strlen(linebuf);
	if (t1->hp <= 0) hpbuf[0] = '-', hpbuf[1] = '\0';
	else sprintf(hpbuf, "%d", t1->hp);
	/* beginning of hp column after padding (not actually padded yet) */
	hppos = COLNO - (sizeof("  Hp [max]")-1); /* sizeof(str) includes \0 */
	while (lngr >= hppos) {
	    for(bp = eos(linebuf);
		    !(*bp == ' ' && (bp-linebuf < hppos));
		    bp--)
		;
	    /* special case: if about to wrap in the middle of maximum
	       dungeon depth reached, wrap in front of it instead */
	    if (bp > linebuf + 5 && !strncmp(bp - 5, " [max", 5)) bp -= 5;
	    strcpy(linebuf3, bp+1);
	    *bp = 0;
	    if (so) {
		while (bp < linebuf + (COLNO-1)) *bp++ = ' ';
		*bp = 0;
		topten_print_bold(linebuf);
#ifdef DUMP_LOG
		dump("*", linebuf[0]==' '? linebuf+1: linebuf);
#endif
	    } else {
		topten_print(linebuf);
#ifdef DUMP_LOG
		dump(" ", linebuf[0]==' '? linebuf+1: linebuf);
#endif
	    }
	    sprintf(linebuf, "%15s %s", "", linebuf3);
	    lngr = strlen(linebuf);
	}
	/* beginning of hp column not including padding */
	hppos = COLNO - 7 - (int)strlen(hpbuf);
	bp = eos(linebuf);

	if (bp <= linebuf + hppos) {
	    /* pad any necessary blanks to the hit point entry */
	    while (bp < linebuf + hppos) *bp++ = ' ';
	    strcpy(bp, hpbuf);
	    sprintf(eos(bp), " %s[%d]",
		    (t1->maxhp < 10) ? "  " : (t1->maxhp < 100) ? " " : "",
		    t1->maxhp);
	}

	if (so) {
	    bp = eos(linebuf);
	    if (so >= COLNO) so = COLNO-1;
	    while (bp < linebuf + so) *bp++ = ' ';
	    *bp = 0;
	    topten_print_bold(linebuf);
	} else
	    topten_print(linebuf);
#ifdef DUMP_LOG
	dump(" ", linebuf[0]==' '? linebuf+1: linebuf);
#endif
}

static int score_wanted(boolean current_ver, int rank, struct toptenentry *t1, int playerct, const char **players, int uid) {
	int i;

	if (current_ver && (t1->ver_major != VERSION_MAJOR ||
			    t1->ver_minor != VERSION_MINOR ||
			    t1->patchlevel != PATCHLEVEL))
		return 0;

#ifdef PERS_IS_UID
	if (!playerct && t1->uid == uid)
		return 1;
#endif

	for (i = 0; i < playerct; i++) {
		if (players[i][0] == '-' && index("prga", players[i][1]) &&
                players[i][2] == 0 && i + 1 < playerct) {
		char *arg = (char *)players[i + 1];
		if ((players[i][1] == 'p' &&
		     str2role(arg) == str2role(t1->plrole)) ||
		    (players[i][1] == 'r' &&
		     str2race(arg) == str2race(t1->plrace)) ||
		    (players[i][1] == 'g' &&
		     str2gend(arg) == str2gend(t1->plgend)) ||
		    (players[i][1] == 'a' &&
		     str2align(arg) == str2align(t1->plalign)))
		    return 1;
		i++;
		}
		else if (strcmp(players[i], "all") == 0 ||
		    strncmp(t1->name, players[i], NAMSZ) == 0 ||
		    (players[i][0] == '-' &&
		     players[i][1] == t1->plrole[0] &&
		     players[i][2] == 0) ||
		    (digit(players[i][0]) && rank <= atoi(players[i])))
		return 1;
	}
	return 0;
}

long
encodeconduct(void)
{
       long e = 0L;

       if(!u.uconduct.food)            e |= 0x001L;
       if(!u.uconduct.unvegan)         e |= 0x002L;
       if(!u.uconduct.unvegetarian)    e |= 0x004L;
       if(!u.uconduct.gnostic)         e |= 0x008L;
       if(!u.uconduct.weaphit)         e |= 0x010L;
       if(!u.uconduct.killer)          e |= 0x020L;
       if(!u.uconduct.literate)        e |= 0x040L;
       if(!u.uconduct.polypiles)       e |= 0x080L;
       if(!u.uconduct.polyselfs)       e |= 0x100L;
       if(!u.uconduct.wishes)          e |= 0x200L;
       if(!u.uconduct.wisharti)        e |= 0x400L;
       if(!num_genocides())            e |= 0x800L;

       return e;
}

long
encodeachieve(void)
{
  /* Achievement bitfield:
   * bit  meaning
   *  0   obtained the Bell of Opening
   *  1   entered gehennom (by any means)
   *  2   obtained the Candelabrum of Invocation
   *  3   obtained the Book of the Dead
   *  4   performed the invocation ritual
   *  5   obtained the amulet
   *  6   entered elemental planes
   *  7   entered astral plane
   *  8   ascended (not "escaped in celestial disgrace!")
   *  9   obtained the luckstone from the Mines
   *  10  obtained the sokoban prize
   *  11  killed medusa
   */

  long r;

  r = 0;

  if(achieve.get_bell)           r |= 1L << 0;
  if(achieve.enter_gehennom)     r |= 1L << 1;
  if(achieve.get_candelabrum)    r |= 1L << 2;
  if(achieve.get_book)           r |= 1L << 3;
  if(achieve.perform_invocation) r |= 1L << 4;
  if(achieve.get_amulet)         r |= 1L << 5;
  if(In_endgame(&u.uz))          r |= 1L << 6;
  if(Is_astralevel(&u.uz))       r |= 1L << 7;
  if(achieve.ascended)           r |= 1L << 8;
  if(achieve.get_luckstone)      r |= 1L << 9;
  if(achieve.finish_sokoban)     r |= 1L << 10;
  if(achieve.killed_medusa)      r |= 1L << 11;

  return r;
}

/*
 * print selected parts of score list.
 * argc >= 2, with argv[0] untrustworthy (directory names, et al.),
 * and argv[1] starting with "-s".
 */
void prscore(int argc, char **argv) {
	const char **players;
	int playerct, rank;
	boolean current_ver = true, init_done = false;
	struct toptenentry *t1;
	FILE *rfile;
	boolean match_found = false;
	int i;
	char pbuf[BUFSZ];
	int uid = -1;
#ifndef PERS_IS_UID
	const char *player0;
#endif

	if (argc < 2 || strncmp(argv[1], "-s", 2)) {
		raw_printf("prscore: bad arguments (%d)", argc);
		return;
	}

	rfile = fopen_datafile(NH_RECORD, "r", SCOREPREFIX);
	if (!rfile) {
		raw_print("Cannot open record file!");
		return;
	}


	/* If the score list isn't after a game, we never went through
	 * initialization. */
	if (wiz1_level.dlevel == 0) {
		dlb_init();
		init_dungeons();
		init_done = true;
	}

	if (!argv[1][2]){	/* plain "-s" */
		argc--;
		argv++;
	} else	argv[1] += 2;

	if (argc > 1 && !strcmp(argv[1], "-v")) {
		current_ver = false;
		argc--;
		argv++;
	}

	if (argc <= 1) {
#ifdef PERS_IS_UID
		uid = getuid();
		playerct = 0;
		players = (const char **)0;
#else
		player0 = plname;
		if (!*player0)
			player0 = "hackplayer";
		playerct = 1;
		players = &player0;
#endif
	} else {
		playerct = --argc;
		players = (const char **)++argv;
	}
	raw_print("");

	t1 = tt_head = newttentry();
	for (rank = 1; ; rank++) {
	    readentry(rfile, t1);
	    if (t1->points == 0) break;
	    if (!match_found &&
		    score_wanted(current_ver, rank, t1, playerct, players, uid))
		match_found = true;
	    t1->tt_next = newttentry();
	    t1 = t1->tt_next;
	}

	fclose(rfile);
	if (init_done) {
	    free_dungeons();
	    dlb_cleanup();
	}

	if (match_found) {
	    outheader();
	    t1 = tt_head;
	    for (rank = 1; t1->points != 0; rank++, t1 = t1->tt_next) {
		if (score_wanted(current_ver, rank, t1, playerct, players, uid))
		    outentry(rank, t1, 0);
	    }
	} else {
	    sprintf(pbuf, "Cannot find any %sentries for ",
				current_ver ? "current " : "");
	    if (playerct < 1) strcat(pbuf, "you.");
	    else {
		if (playerct > 1) strcat(pbuf, "any of ");
		for (i = 0; i < playerct; i++) {
		    /* stop printing players if there are too many to fit */
		    if (strlen(pbuf) + strlen(players[i]) + 2 >= BUFSZ) {
			if (strlen(pbuf) < BUFSZ-4) strcat(pbuf, "...");
			else strcpy(pbuf+strlen(pbuf)-4, "...");
			break;
		    }
		    strcat(pbuf, players[i]);
		    if (i < playerct-1) {
			if (players[i][0] == '-' &&
			    index("prga", players[i][1]) && players[i][2] == 0)
			    strcat(pbuf, " ");
			else strcat(pbuf, ":");
		    }
		}
	    }
	    raw_print(pbuf);
	    raw_printf("Usage: %s -s [-v] <playertypes> [maxrank] [playernames]",

			 hname);
	    raw_printf("Player types are: [-p role] [-r race] [-g gender] [-a align]");
	}
	free_ttlist(tt_head);
}

static int classmon(char *plch, boolean fem) {
	int i;

	/* Look for this role in the role table */
	for (i = 0; roles[i].name.m; i++)
	    if (!strncmp(plch, roles[i].filecode, ROLESZ)) {
		if (fem && roles[i].femalenum != NON_PM)
		    return roles[i].femalenum;
		else if (roles[i].malenum != NON_PM)
		    return roles[i].malenum;
		else
		    return PM_HUMAN;
	    }
	/* this might be from a 3.2.x score for former Elf class */
	if (!strcmp(plch, "E")) return PM_RANGER;

	impossible("What weird role is this? (%s)", plch);
	return PM_HUMAN_MUMMY;
}

/*
 * Get a random player name and class from the high score list,
 * and attach them to an object (for statues or morgue corpses).
 */
struct obj * tt_oname(struct obj *otmp) {
	int rank;
	int i;
	struct toptenentry *tt;
	FILE *rfile;
	struct toptenentry tt_buf;

	if (!otmp) return NULL;

	rfile = fopen_datafile(NH_RECORD, "r", SCOREPREFIX);
	if (!rfile) {
		impossible("Cannot open record file!");
		return NULL;
	}

	tt = &tt_buf;
	rank = rnd(10);
pickentry:
	for(i = rank; i; i--) {
	    readentry(rfile, tt);
	    if(tt->points == 0) break;
	}

	if(tt->points == 0) {
		if(rank > 1) {
			rank = 1;
			rewind(rfile);
			goto pickentry;
		}
		otmp = NULL;
	} else {
		/* reset timer in case corpse started out as lizard or troll */
		if (otmp->otyp == CORPSE) obj_stop_timers(otmp);
		otmp->corpsenm = classmon(tt->plrole, (tt->plgend[0] == 'F'));
		otmp->owt = weight(otmp);
		otmp = oname(otmp, tt->name);
		if (otmp->otyp == CORPSE) start_corpse_timeout(otmp);
	}

	fclose(rfile);
	return otmp;
}

#if defined(PROXY_GRAPHICS)
winid create_toptenwin()
{
    toptenwin = create_nhwindow(NHW_TEXT);

    return toptenwin;
}

void
destroy_toptenwin()
{
    destroy_nhwindow(toptenwin);
    toptenwin = WIN_ERR;
}
#endif
/*topten.c*/
