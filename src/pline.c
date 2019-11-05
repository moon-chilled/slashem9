/*	SCCS Id: @(#)pline.c	3.4	1999/11/28	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "epri.h"
#include "edog.h"


usize saved_pline_index = 0;
char *saved_plines[DUMPLOG_MSG_COUNT] = {0};

// keep the most recent DUMPLOG_MSG_COUNT messages
void dumplogmsg(const char *line) {
	// since it's zero-initialized and free(NULL) is valid, this always works
	free(saved_plines[saved_pline_index]);

	saved_plines[saved_pline_index] = strdup(line);

	saved_pline_index = (saved_pline_index + 1) % DUMPLOG_MSG_COUNT;
}
void dumplogfreemsgs(void) {
	for (usize i = 0; i < DUMPLOG_MSG_COUNT; i++) {
		free(saved_plines[i]);
	}
}

static boolean no_repeat = false;
static char *You_buf(int);

void msgpline_add(int typ, char *pattern) {
	struct _plinemsg *tmp = new(struct _plinemsg);
	if (!tmp) return;
	tmp->msgtype = typ;
	tmp->pattern = strdup(pattern);
	tmp->next = pline_msg;
	pline_msg = tmp;
}

void msgpline_free (void) {
	struct _plinemsg *tmp = pline_msg;
	struct _plinemsg *tmp2;
	while (tmp) {
		free(tmp->pattern);
		tmp2 = tmp;
		tmp = tmp->next;
		free(tmp2);
	}
	pline_msg = NULL;
}

static int msgpline_type(const char *msg) {
	struct _plinemsg *tmp = pline_msg;
	while (tmp) {
		if (pmatch(tmp->pattern, msg)) return tmp->msgtype;
		tmp = tmp->next;
	}
	return MSGTYP_NORMAL;
}


char prevmsg[BUFSZ];

static void vpline(const char *line, va_list the_args) {
	char pbuf[BUFSZ];
	int typ;

	if (!line || !*line) return;
	if (index(line, '%')) {
		vsprintf(pbuf,line,VA_ARGS);
		line = pbuf;
	}

	typ = msgpline_type(line);
	if (typ != MSGTYP_NOSHOW) {
		dumplogmsg(line);
	}

	if (!iflags.window_inited) {
		raw_print(line);
		return;
	}
#ifndef MAC
	if (no_repeat && !strcmp(line, toplines))
		return;
#endif /* MAC */
	if (vision_full_recalc) vision_recalc(0);
	if (u.ux) flush_screen(1);		/* %% */
	if (typ == MSGTYP_NOSHOW) return;
	if (typ == MSGTYP_NOREP && !strcmp(line, prevmsg)) return;
	putstr(WIN_MESSAGE, 0, line);
	strncpy(prevmsg, line, BUFSZ);
	if (typ == MSGTYP_STOP) display_nhwindow(WIN_MESSAGE, true); /* --more-- */
}

void pline(const char *line, ...) {
	VA_START(line);
	vpline(line, VA_ARGS);
	VA_END();
}

void Norep(const char *line, ...) {
	VA_START(line);
	no_repeat = true;
	vpline(line, VA_ARGS);
	no_repeat = false;
	VA_END();
	return;
}

/* work buffer for You(), &c and verbalize() */
static char *you_buf = 0;
static int you_buf_siz = 0;

static char *You_buf(int siz) {
	if (siz > you_buf_siz) {
		if (you_buf) free(you_buf);
		you_buf_siz = siz + 10;
		you_buf = alloc((unsigned) you_buf_siz);
	}
	return you_buf;
}

void free_youbuf(void) {
	if (you_buf) free(you_buf),  you_buf = NULL;
	you_buf_siz = 0;
}

/* `prefix' must be a string literal, not a pointer */
#define YouPrefix(pointer,prefix,text) \
 strcpy((pointer = You_buf((int)(strlen(text) + sizeof prefix))), prefix)

#define YouMessage(pointer,prefix,text) \
 strcat((YouPrefix(pointer, prefix, text), pointer), text)

void You_hearf(const char *line, ...) {
	char *tmp;
	VA_START(line);
	if (Underwater)
		YouPrefix(tmp, "You barely hear ", line);
	else if (u.usleep)
		YouPrefix(tmp, "You dream that you hear ", line);
	else
		YouPrefix(tmp, "You hear ", line);
	vpline(strcat(tmp, line), VA_ARGS);
	VA_END();
}

void verbalize(const char *line, ...) {
	char *tmp;
	if (!flags.soundok) return;
	VA_START(line);
	tmp = You_buf((int)strlen(line) + sizeof "\"\"");
	strcpy(tmp, "\"");
	strcat(tmp, line);
	strcat(tmp, "\"");
	vpline(tmp, VA_ARGS);
	VA_END();
}

static void vraw_printf(const char *, va_list);

void raw_printf(const char *line, ...) {
	VA_START(line);
	vraw_printf(line, VA_ARGS);
	VA_END();
}

static void vraw_printf(const char *line, va_list the_args) {
	if (!index(line, '%')) {
		raw_print(line);
	} else {
		char pbuf[BUFSZ];
		vsprintf(pbuf,line,VA_ARGS);
		raw_print(pbuf);
	}
}


void impossible(const char *s, ...) {
	VA_START(s);
	if (program_state.in_impossible)
		panic("impossible called impossible");
	program_state.in_impossible = 1;
	{
		char pbuf[ BUFSZ];
		vsprintf(pbuf,s,VA_ARGS);
		paniclog("impossible", pbuf);
		if (iflags.debug_fuzzer) {
			panic("%s", pbuf);
		}
	}
	vpline(s,VA_ARGS);
	pline("Program in disorder!  Saving and reloading may fix the problem.");
	program_state.in_impossible = 0;
	VA_END();
}

const char *align_str(aligntyp alignment) {
	switch (alignment) {
	case A_CHAOTIC:
		return "chaotic";
	case A_NEUTRAL:
		return "neutral";
	case A_LAWFUL:
		return "lawful";
	case A_NONE:
		return "unaligned";
	}
	return "unknown";
}

void mstatusline(struct monst *mtmp) {
	aligntyp alignment;
	char info[BUFSZ], monnambuf[BUFSZ];

	if (mtmp->ispriest || mtmp->data == &mons[PM_ALIGNED_PRIEST]
	                || mtmp->data == &mons[PM_ANGEL])
		alignment = EPRI(mtmp)->shralign;
	else
		alignment = mtmp->data->maligntyp;
	alignment = (alignment > 0) ? A_LAWFUL :
	            (alignment < 0) ? A_CHAOTIC :
	            A_NEUTRAL;

	info[0] = 0;
	if (mtmp->mtame) {
		strcat(info, ", tame");
		if (wizard) {
			sprintf(eos(info), " (%d", mtmp->mtame);
			if (!mtmp->isminion)
				sprintf(eos(info), "; hungry %ld; apport %d",
				        EDOG(mtmp)->hungrytime, EDOG(mtmp)->apport);
			strcat(info, ")");
		}
	} else if (mtmp->mpeaceful) strcat(info, ", peaceful");
	else if (mtmp->mtraitor)  strcat(info, ", traitor");
	if (mtmp->meating)	  strcat(info, ", eating");
	if (mtmp->mcan)		  strcat(info, ", cancelled");
	if (mtmp->mconf)	  strcat(info, ", confused");
	if (mtmp->mblinded || !mtmp->mcansee)
		strcat(info, ", blind");
	if (mtmp->mstun)	  strcat(info, ", stunned");
	if (mtmp->msleeping)	  strcat(info, ", asleep");
#if 0	/* unfortunately mfrozen covers temporary sleep and being busy
	   (donning armor, for instance) as well as paralysis */
	else if (mtmp->mfrozen)	  strcat(info, ", paralyzed");
#else
	else if (mtmp->mfrozen || !mtmp->mcanmove)
		strcat(info, ", can't move");
#endif
	/* [arbitrary reason why it isn't moving] */
	else if (mtmp->mstrategy & STRAT_WAITMASK)
		strcat(info, ", meditating");
	else if (mtmp->mflee) {
		strcat(info, ", scared");
		if (wizard)		  sprintf(eos(info), " (%d)", mtmp->mfleetim);
	}
	if (mtmp->mtrapped)	  strcat(info, ", trapped");
	if (mtmp->mspeed)	  strcat(info,
		                                 mtmp->mspeed == MFAST ? ", fast" :
		                                 mtmp->mspeed == MSLOW ? ", slow" :
		                                 ", ???? speed");
	if (mtmp->mundetected)	  strcat(info, ", concealed");
	if (mtmp->minvis)	  strcat(info, ", invisible");
	if (mtmp == u.ustuck)	  strcat(info,
		                                 (sticks(youmonst.data)) ? ", held by you" :
		                                 u.uswallow ? (is_animal(u.ustuck->data) ?
		                                                 ", swallowed you" :
		                                                 ", engulfed you") :
		                                 ", holding you");
	if (mtmp == u.usteed)	  strcat(info, ", carrying you");

	/* avoid "Status of the invisible newt ..., invisible" */
	/* and unlike a normal mon_nam, use "saddled" even if it has a name */
	strcpy(monnambuf, x_monnam(mtmp, ARTICLE_THE, NULL,
	                           (SUPPRESS_IT|SUPPRESS_INVISIBLE), false));

	pline("Status of %s (%s):  Level %d  HP %d(%d)  Pw %d(%d)  AC %d%s.",
	      monnambuf,
	      align_str(alignment),
	      mtmp->m_lev,
	      mtmp->mhp,
	      mtmp->mhpmax,
	      mtmp->m_en,
	      mtmp->m_enmax,
	      find_mac(mtmp),
	      info);
}

void ustatusline(void) {
	char info[BUFSZ];

	info[0] = '\0';
	if (Sick) {
		strcat(info, ", dying from");
		if (u.usick_type & SICK_VOMITABLE)
			strcat(info, " food poisoning");
		if (u.usick_type & SICK_NONVOMITABLE) {
			if (u.usick_type & SICK_VOMITABLE)
				strcat(info, " and");
			strcat(info, " illness");
		}
	}
	if (Stoned)		strcat(info, ", solidifying");
	if (Slimed)		strcat(info, ", becoming slimy");
	if (Strangled)		strcat(info, ", being strangled");
	if (Vomiting)		strcat(info, ", nauseated"); /* !"nauseous" */
	if (Confusion)		strcat(info, ", confused");
	if (Blind) {
		strcat(info, ", blind");
		if (u.ucreamed) {
			if ((long)u.ucreamed < Blinded || Blindfolded
			                || !haseyes(youmonst.data))
				strcat(info, ", cover");
			strcat(info, "ed by sticky goop");
		}	/* note: "goop" == "glop"; variation is intentional */
	}
	if (Stunned)		strcat(info, ", stunned");
	if (!u.usteed && Wounded_legs) {
		const char *what = body_part(LEG);
		if ((Wounded_legs & BOTH_SIDES) == BOTH_SIDES)
			what = makeplural(what);
		sprintf(eos(info), ", injured %s", what);
	}
	if (Glib)		sprintf(eos(info), ", slippery %s",
		                                makeplural(body_part(HAND)));
	if (u.utrap)		strcat(info, ", trapped");
	if (Fast)		strcat(info, Very_fast ?
		                               ", very fast" : ", fast");
	if (u.uundetected)	strcat(info, ", concealed");
	if (Invis)		strcat(info, ", invisible");
	if (u.ustuck) {
		if (sticks(youmonst.data))
			strcat(info, ", holding ");
		else
			strcat(info, ", held by ");
		strcat(info, mon_nam(u.ustuck));
	}

	pline("Status of %s (%s%s):  Level %d  HP %d(%d)  Pw %d(%d)  AC %d%s.",
	      plname,
	      (u.ualign.record >= 20) ? "piously " :
	      (u.ualign.record > 13) ? "devoutly " :
	      (u.ualign.record > 8) ? "fervently " :
	      (u.ualign.record > 3) ? "stridently " :
	      (u.ualign.record == 3) ? "" :
	      (u.ualign.record >= 1) ? "haltingly " :
	      (u.ualign.record == 0) ? "nominally " :
	      "insufficiently ",
	      align_str(u.ualign.type),
	      Upolyd ? mons[u.umonnum].mlevel : u.ulevel,
	      Upolyd ? u.mh : u.uhp,
	      Upolyd ? u.mhmax : u.uhpmax,
	      u.uen,
	      u.uenmax,
	      u.uac,
	      info);
}

void self_invis_message(void) {
	pline("%s %s.",
	      Hallucination ? "Far out, man!  You" : "Gee!  All of a sudden, you",
	      See_invisible ? "can see right through yourself" :
	      "can't see yourself");
}
/*pline.c*/
