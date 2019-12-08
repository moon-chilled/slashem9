/*	SCCS Id: @(#)were.c	3.4	2002/11/07	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

void were_change(struct monst *mon) {
	if (!is_were(mon->data))
		return;

	if (is_human(mon->data)) {
		if (!Protection_from_shape_changers &&
		    !rn2(night() ? (flags.moonphase == FULL_MOON ? 3 : 30) : (flags.moonphase == FULL_MOON ? 10 : 50))) {
			new_were(mon); /* change into animal form */
			if (!Deaf && !canseemon(mon)) {
				const char *howler;

				switch (monsndx(mon->data)) {
					case PM_HUMAN_WEREWOLF:
						howler = "wolf";
						break;
					case PM_HUMAN_WEREJACKAL:
						howler = "jackal";
						break;
					case PM_HUMAN_WEREPANTHER:
						howler = "panther";
						break;
					case PM_HUMAN_WERETIGER:
						howler = "tiger";
						break;
					default:
						howler = NULL;
						break;
				}
				if (howler)
					You_hearf("a %s howling at the moon.", howler);
			}
		}
	} else if (!rn2(30) || Protection_from_shape_changers) {
		new_were(mon); /* change back into human form */
	}
}

int counter_were(int pm) {
	switch (pm) {
		case PM_WEREWOLF:
			return PM_HUMAN_WEREWOLF;
		case PM_HUMAN_WEREWOLF:
			return PM_WEREWOLF;
		case PM_WEREJACKAL:
			return PM_HUMAN_WEREJACKAL;
		case PM_HUMAN_WEREJACKAL:
			return PM_WEREJACKAL;
		case PM_WERERAT:
			return PM_HUMAN_WERERAT;
		case PM_HUMAN_WERERAT:
			return PM_WERERAT;
		case PM_WEREPANTHER:
			return PM_HUMAN_WEREPANTHER;
		case PM_HUMAN_WEREPANTHER:
			return PM_WEREPANTHER;
		case PM_WERETIGER:
			return PM_HUMAN_WERETIGER;
		case PM_HUMAN_WERETIGER:
			return PM_WERETIGER;
		case PM_WERESNAKE:
			return PM_HUMAN_WERESNAKE;
		case PM_HUMAN_WERESNAKE:
			return PM_WERESNAKE;
		case PM_WERESPIDER:
			return PM_HUMAN_WERESPIDER;
		case PM_HUMAN_WERESPIDER:
			return PM_WERESPIDER;
		default:
			return 0;
	}
}

void new_were(struct monst *mon) {
	int pm;

	pm = counter_were(monsndx(mon->data));
	if (!pm) {
		impossible("unknown lycanthrope %s.", mon->data->mname);
		return;
	}

	if (canseemon(mon) && !Hallucination)
		pline("%s changes into a %s.", Monnam(mon),
		      is_human(&mons[pm]) ? "human" :
					    mons[pm].mname + 4);

	set_mon_data(mon, &mons[pm], 0);
	if (mon->msleeping || !mon->mcanmove) {
		/* transformation wakens and/or revitalizes */
		mon->msleeping = 0;
		mon->mfrozen = 0; /* not asleep or paralyzed */
		mon->mcanmove = 1;
	}
	/* regenerate by 1/4 of the lost hit points */
	mon->mhp += (mon->mhpmax - mon->mhp) / 4;
	newsym(mon->mx, mon->my);
	mon_break_armor(mon, false);
	possibly_unwield(mon, false);
	stop_timer(UNPOLY_MON, monst_to_any(mon));
	start_timer(rn1(1000, 1000), TIMER_MONSTER, UNPOLY_MON, monst_to_any(mon));
}

/* were-creature (even you) summons a horde */
// visible => number of visible helpers created
int were_summon(struct permonst *ptr, boolean yours, int *visible, char *genbuf) {
	int i, typ, pm = monsndx(ptr);
	struct monst *mtmp;
	int total = 0;

	*visible = 0;
	if (Protection_from_shape_changers && !yours)
		return 0;
	/*
	 * Allow lycanthropes in normal form to summon hordes as well.  --ALI
	 */
	if (pm == PM_PLAYERMON)
		pm = urace.malenum;
	for (i = rnd(2); i > 0; i--) {
		switch (pm) {
			case PM_WERERAT:
			case PM_HUMAN_WERERAT:
				typ = rn2(3) ? PM_SEWER_RAT : rn2(3) ? PM_GIANT_RAT : PM_RABID_RAT;
				if (genbuf) strcpy(genbuf, "rat");
				break;
			case PM_WEREJACKAL:
			case PM_HUMAN_WEREJACKAL:
				typ = PM_JACKAL;
				if (genbuf) strcpy(genbuf, "jackal");
				break;
			case PM_WEREWOLF:
			case PM_HUMAN_WEREWOLF:
				typ = rn2(5) ? PM_WOLF : PM_WINTER_WOLF;
				if (genbuf) strcpy(genbuf, "wolf");
				break;
			case PM_WEREPANTHER:
			case PM_HUMAN_WEREPANTHER:
				typ = rn2(5) ? PM_JAGUAR : PM_PANTHER;
				if (genbuf) strcpy(genbuf, "large cat");
				break;
			case PM_WERETIGER:
			case PM_HUMAN_WERETIGER:
				typ = rn2(5) ? PM_JAGUAR : PM_TIGER;
				if (genbuf) strcpy(genbuf, "large cat");
				break;
			case PM_WERESNAKE:
			case PM_HUMAN_WERESNAKE:
				typ = rn2(5) ? PM_SNAKE : PM_PIT_VIPER;
				if (genbuf) strcpy(genbuf, "snake");
				break;
			case PM_WERESPIDER:
			case PM_HUMAN_WERESPIDER:
				typ = rn2(5) ? PM_CAVE_SPIDER : PM_RECLUSE_SPIDER;
				if (genbuf) strcpy(genbuf, "spider");
				break;
			default:
				continue;
		}
		mtmp = makemon(&mons[typ], u.ux, u.uy, NO_MM_FLAGS);
		if (mtmp) {
			total++;
			if (canseemon(mtmp)) *visible += 1;
		}
		if (yours && mtmp)
			tamedog(mtmp, NULL);
	}
	return total;
}

void you_were(void) {
	char qbuf[QBUFSZ];

	if (Unchanging || (u.umonnum == u.ulycn)) return;
	if (Polymorph_control) {
		/* `+4' => skip "were" prefix to get name of beast */
		sprintf(qbuf, "Do you want to change into %s? ",
			an(mons[u.ulycn].mname + 4));
		if (yn(qbuf) == 'n') return;
	}
	polymon(u.ulycn);
}

void you_unwere(boolean purify) {
	boolean in_wereform = (u.umonnum == u.ulycn);

	if (purify) {
		if (Race_if(PM_HUMAN_WEREWOLF)) {
			/* An attempt to purify you has been made! */
			if (in_wereform && Unchanging) {
				pline("The purification was deadly...");

				killer.format = NO_KILLER_PREFIX;
				nhscopyz(&killer.name, "purified while stuck in creature form");
				done(DIED);
			} else {
				pline("You feel very bad!");
				if (in_wereform)
					rehumanize();
				adjattrib(A_STR, -rn1(3, 3), 2);
				adjattrib(A_CON, -rn1(3, 3), 1);
				losehp(u.uhp - (u.uhp > 10 ? rnd(5) : 1), "purification",
				       KILLED_BY);
			}
			return;
		}
		pline("You feel purified.");
		u.ulycn = NON_PM; /* cure lycanthropy */
		upermonst.mflags2 &= ~M2_WERE;
	}
	if (!Unchanging && in_wereform &&
	    (!Polymorph_control || yn("Remain in beast form?") == 'n'))
		rehumanize();
}
/*were.c*/
