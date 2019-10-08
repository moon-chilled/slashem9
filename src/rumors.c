/*	SCCS Id: @(#)rumors.c	3.4	1996/04/20	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include "dlb.h"

static void init_oracles(dlb *);

/* exclude_cookie is a hack used because we sometimes want to get rumors in a
 * context where messages such as "You swallowed the fortune!" that refer to
 * cookies should not appear.  This has no effect for true rumors since none
 * of them contain such references anyway.
 */
// truth: 1=true, -1=false, 0=either
char *getrumor(int truth, char *rumor_buf, bool exclude_cookie) {
	dlb *rumors;
	long tidbit, beginning;
	char *endp;
	char line[BUFSZ];

	rumor_buf[0] = '\0';

	if (truth == 1) {
		rumors = dlb_fopen_area(NH_RUMORAREA, NH_RUMORFILE_TRU, "r");
	} else if (truth == 0) {
		rumors = dlb_fopen_area(NH_RUMORAREA, rn2(2) ? NH_RUMORFILE_TRU : NH_RUMORFILE_FAL, "r");
	} else if (truth == -1) {
		rumors = dlb_fopen_area(NH_RUMORAREA, NH_RUMORFILE_FAL, "r");
	} else {
		impossible("Bad fortune truth %d", truth);
		return rumor_buf = "Viva fortuna";
	}

	if (!rumors) {
		impossible("Can't open rumors file!");
		return rumor_buf = "The Slash'EM rumors file is currently closed for rennovations.";
	}

	dlb_fseek(rumors, 0, SEEK_END);
	size_t len = dlb_ftell(rumors);

	do {
		// HACK ALERT!
		// This chooses a random position within the file, and then finds the next fortune after that position
		// Eventually, the dlb interface should support newline-tabulated files
		// So this doesn't have to be so complex
		size_t tidbit = rn2(len);
		dlb_fseek(rumors, tidbit, SEEK_SET);

		dlb_fgets(line, sizeof line, rumors);

		// reached end of rumors -- go back to beginning
		if (!dlb_fgets(line, sizeof line, rumors)) {
			dlb_fseek(rumors, beginning, SEEK_SET);
			dlb_fgets(line, sizeof line, rumors);
		}
		if ((endp = index(line, '\n')) != 0) *endp = 0;
	} while (exclude_cookie && (strstri(line, "fortune") || strstri(line, "pity")));
	// pity => 'What a pity, you cannot read it!'

	strcpy(rumor_buf, line);

	// XXX this currently exercises wisdom only when you get a guaranteed true fortune
	// should it also if you get a random fortune that happens to be true?
	exercise(A_WIS, truth > 0);

	dlb_fclose(rumors);
	return rumor_buf;
}

void
outrumor (
    int truth, /* 1=true, -1=false, 0=either */
    int mechanism
)
{
	static const char fortune_msg[] =
		"This cookie has a scrap of paper inside.";
	const char *line;
	char buf[BUFSZ];
	boolean reading = (mechanism == BY_COOKIE ||
			   mechanism == BY_PAPER);

	if (reading) {
	    /* deal with various things that prevent reading */
	    if (is_fainted() && mechanism == BY_COOKIE)
	    	return;
	    else if (Blind) {
		if (mechanism == BY_COOKIE)
			pline(fortune_msg);
		pline("What a pity that you cannot read it!");
	    	return;
	    }
	}
	line = getrumor(truth, buf, reading ? false : true);
	if (!*line)
		line = "NetHack rumors file closed for renovation.";
	switch (mechanism) {
	    case BY_ORACLE:
	 	/* Oracle delivers the rumor */
		pline("True to her word, the Oracle %ssays: ",
		  (!rn2(4) ? "offhandedly " : (!rn2(3) ? "casually " :
		  (rn2(2) ? "nonchalantly " : ""))));
		verbalize("%s", line);
		exercise(A_WIS, true);
		return;
	    case BY_COOKIE:
		pline(fortune_msg);
		/* FALLTHRU */
	    case BY_PAPER:
		pline("It reads:");
		break;
	}
	pline("%s", line);
}

void outoracle(boolean special, boolean delphi) {
	char	line[COLNO];
	dlb	*oracles;
	char buf[BUFSZ];

	static size_t *oracle_offsets;
	static size_t num_oracles;
	static size_t current_oracle = 0;


	oracles = dlb_fopen_area(NH_ORACLEAREA, NH_ORACLEFILE, "r");
	if (!oracles) {
		pline("Can't open oracles file!");
		return;
	}
	if (!oracle_offsets) {
		num_oracles = 1;
		oracle_offsets = new(size_t, 1);
		oracle_offsets[0] = 0; // First oracle starts at position 0
		while (dlb_fgets(buf, BUFSZ, oracles)) {
			if (!strcmp(buf, "-----\n")) {
				oracle_offsets = realloc(oracle_offsets, sizeof(size_t) * ++num_oracles);
				oracle_offsets[num_oracles-1] = dlb_ftell(oracles);
			}
		}
	}

	winid tmpwin = create_nhwindow(NHW_TEXT);
	if (delphi)
		putstr(tmpwin, 0, special ?
				"The Oracle scornfully takes all your money and says:" :
				"The Oracle meditates for a moment and then intones:");
	else
		putstr(tmpwin, 0, "The message reads:");
	putstr(tmpwin, 0, "");


	// special => print the first (crappy) oracle
	// Subtract 6 because oracle_offsets records the *start* of an oracle
	// which, since they are delimited by "-----\n", is 6 characters after the end of the previous one
	size_t start = oracle_offsets[special ? 0 : rnd(num_oracles - 1)];

	dlb_fseek(oracles, start, SEEK_SET);

	while (dlb_fgets(line, COLNO, oracles) && strcmp(line,"-----\n")) {
		putstr(tmpwin, 0, line);
	}

	display_nhwindow(tmpwin, true);
	destroy_nhwindow(tmpwin);

	dlb_fclose(oracles);
}

int doconsult(struct monst *oracl) {
#ifdef GOLDOBJ
        long umoney = money_cnt(invent);
#endif
	int u_pay, minor_cost = 50, major_cost = 500 + 50 * u.ulevel;
	int add_xpts;
	char qbuf[QBUFSZ];

	multi = 0;

	if (!oracl) {
		pline("There is no one here to consult.");
		return 0;
	} else if (!oracl->mpeaceful) {
		pline("%s is in no mood for consultations.", Monnam(oracl));
		return 0;
#ifndef GOLDOBJ
	} else if (!u.ugold) {
#else
	} else if (!umoney) {
#endif
		pline("You have no money.");
		return 0;
	}

	sprintf(qbuf,
		"\"Wilt thou settle for a minor consultation?\" (%d %s)",
		minor_cost, currency((long)minor_cost));
	switch (ynq(qbuf)) {
	    default:
	    case 'q':
		return 0;
	    case 'y':
#ifndef GOLDOBJ
		if (u.ugold < (long)minor_cost) {
#else
		if (umoney < (long)minor_cost) {
#endif
		    pline("You don't even have enough money for that!");
		    return 0;
		}
		u_pay = minor_cost;
		break;
	    case 'n':
#ifndef GOLDOBJ
		if (u.ugold <= (long)minor_cost	/* don't even ask */
#else
		if (umoney <= (long)minor_cost	/* don't even ask */
#endif
		    ) return 0;
		sprintf(qbuf,
			"\"Then dost thou desire a major one?\" (%d %s)",
			major_cost, currency((long)major_cost));
		if (yn(qbuf) != 'y') return 0;
#ifndef GOLDOBJ
		u_pay = (u.ugold < (long)major_cost ? (int)u.ugold
						    : major_cost);
#else
		u_pay = (umoney < (long)major_cost ? (int)umoney
						    : major_cost);
#endif
		break;
	}
#ifndef GOLDOBJ
	u.ugold -= (long)u_pay;
	oracl->mgold += (long)u_pay;
#else
        money2mon(oracl, (long)u_pay);
#endif
	flags.botl = 1;
	add_xpts = 0;	/* first oracle of each type gives experience points */
	if (u_pay == minor_cost) {
		outrumor(1, BY_ORACLE);
		if (!u.uevent.minor_oracle)
		    add_xpts = u_pay / (u.uevent.major_oracle ? 25 : 10);
		    /* 5 pts if very 1st, or 2 pts if major already done */
		u.uevent.minor_oracle = true;
	} else {
		boolean cheapskate = u_pay < major_cost;
		outoracle(cheapskate, true);
		if (!cheapskate && !u.uevent.major_oracle)
		    add_xpts = u_pay / (u.uevent.minor_oracle ? 25 : 10);
		    /* ~100 pts if very 1st, ~40 pts if minor already done */
		u.uevent.major_oracle = true;
		exercise(A_WIS, !cheapskate);
	}
	if (add_xpts) {
		more_experienced(add_xpts, u_pay/50);
		newexplevel();
	}
	return 1;
}

/*rumors.c*/
