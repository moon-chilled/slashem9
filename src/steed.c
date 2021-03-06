/*	SCCS Id: @(#)steed.c	3.4	2003/01/10	*/
/* Copyright (c) Kevin Hugo, 1998-1999. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

/* Monsters that might be ridden */
static const char steeds[] = {
	S_QUADRUPED, S_UNICORN, S_ANGEL, S_CENTAUR, S_DRAGON, S_JABBERWOCK, '\0'};

static boolean landing_spot(coord *, int, int);

/* caller has decided that hero can't reach something while mounted */
void rider_cant_reach(void) {
	pline("You aren't skilled enough to reach from %s.", y_monnam(u.usteed));
}

/*** Putting the saddle on ***/

/* Can this monster wear a saddle? */
boolean can_saddle(struct monst *mtmp) {
	struct permonst *ptr = mtmp->data;

	return (index(steeds, ptr->mlet) && (ptr->msize >= MZ_MEDIUM) &&
		(!humanoid(ptr) || ptr->mlet == S_CENTAUR) &&
		!amorphous(ptr) && !noncorporeal(ptr) &&
		!is_whirly(ptr) && !unsolid(ptr));
}

int use_saddle(struct obj *otmp) {
	struct monst *mtmp;
	struct permonst *ptr;
	int chance;
	const char *s;

	/* Can you use it? */
	if (nohands(youmonst.data)) {
		pline("You have no hands!"); /* not `body_part(HAND)' */
		return 0;
	} else if (!freehand()) {
		pline("You have no free %s.", body_part(HAND));
		return 0;
	}

	/* Select an animal */
	if (u.uswallow || Underwater || !getdir(NULL)) {
		pline("Never mind.");
		return 0;
	}
	if (!u.dx && !u.dy) {
		pline("Saddle yourself?  Very funny...");
		return 0;
	}
	if (!isok(u.ux + u.dx, u.uy + u.dy) ||
	    !(mtmp = m_at(u.ux + u.dx, u.uy + u.dy)) ||
	    !canspotmon(mtmp)) {
		pline("I see nobody there.");
		return 1;
	}

	/* Is this a valid monster? */
	if (mtmp->misc_worn_check & W_SADDLE ||
	    which_armor(mtmp, W_SADDLE)) {
		pline("%s doesn't need another one.", Monnam(mtmp));
		return 1;
	}
	ptr = mtmp->data;
	if (touch_petrifies(ptr) && !uarmg && !Stone_resistance) {
		char kbuf[BUFSZ];

		pline("You touch %s.", mon_nam(mtmp));
		if (!(poly_when_stoned(youmonst.data) && polymon(PM_STONE_GOLEM))) {
			sprintf(kbuf, "attempting to saddle %s", an(mtmp->data->mname));
			instapetrify(kbuf);
		}
	}
	if (is_foocubus(ptr)) {
		pline("Shame on you!");
		exercise(A_WIS, false);
		return 1;
	}
	if (mtmp->isminion || mtmp->isshk || mtmp->ispriest ||
	    mtmp->isgd || mtmp->iswiz) {
		pline("I think %s would mind.", mon_nam(mtmp));
		return 1;
	}
	if (!can_saddle(mtmp)) {
		pline("You can't saddle such a creature.");
		return 1;
	}

	/* Calculate your chance */
	chance = ACURR(A_DEX) + ACURR(A_CHA) / 2 + 2 * mtmp->mtame;
	chance += u.ulevel * (mtmp->mtame ? 20 : 5);
	if (!mtmp->mtame) chance -= 10 * mtmp->m_lev;
	if (Role_if(PM_KNIGHT))
		chance += 20;
	switch (P_SKILL(P_RIDING)) {
		case P_ISRESTRICTED:
		case P_UNSKILLED:
		default:
			chance -= 20;
			break;
		case P_BASIC:
			break;
		case P_SKILLED:
			chance += 15;
			break;
		case P_EXPERT:
			chance += 30;
			break;
	}
	if (Confusion || Fumbling || Glib)
		chance -= 20;
	else if (uarmg &&
		 (s = OBJ_DESCR(objects[uarmg->otyp])) != NULL &&
		 !strncmp(s, "riding ", 7))
		/* Bonus for wearing "riding" (but not fumbling) gloves */
		chance += 10;
	else if (uarmf &&
		 (s = OBJ_DESCR(objects[uarmf->otyp])) != NULL &&
		 !strncmp(s, "riding ", 7))
		/* ... or for "riding boots" */
		chance += 10;
	if (otmp->cursed)
		chance -= 50;

	/* Make the attempt */
	if (rn2(100) < chance) {
		pline("You put the saddle on %s.", mon_nam(mtmp));
		if (otmp->owornmask) remove_worn_item(otmp, false);
		freeinv(otmp);
		/* mpickobj may free otmp it if merges, but we have already
		   checked for a saddle above, so no merger should happen */
		mpickobj(mtmp, otmp);
		mtmp->misc_worn_check |= W_SADDLE;
		otmp->owornmask = W_SADDLE;
		otmp->leashmon = mtmp->m_id;
		update_mon_intrinsics(mtmp, otmp, true, false);
	} else
		pline("%s resists!", Monnam(mtmp));
	return 1;
}

/*** Riding the monster ***/

/* Can we ride this monster?  Caller should also check can_saddle() */
boolean can_ride(struct monst *mtmp) {
	return (mtmp->mtame && humanoid(youmonst.data) &&
		!verysmall(youmonst.data) && !bigmonst(youmonst.data) &&
		(!Underwater || is_swimmer(mtmp->data)));
}

int doride(void) {
	boolean forcemount = false;

	if (u.usteed)
		dismount_steed(DISMOUNT_BYCHOICE);
	else if (getdir(NULL) && isok(u.ux + u.dx, u.uy + u.dy)) {
		if (wizard && yn("Force the mount to succeed?") == 'y')
			forcemount = true;
		return mount_steed(m_at(u.ux + u.dx, u.uy + u.dy), forcemount);
	} else
		return 0;
	return 1;
}

/* Start riding, with the given monster */
boolean mount_steed(struct monst *mtmp, boolean force) {
	struct obj *otmp;
	char buf[BUFSZ];
	struct permonst *ptr;

	/* Sanity checks */
	if (u.usteed) {
		pline("You are already riding %s.", mon_nam(u.usteed));
		return false;
	}

	/* Is the player in the right form? */
	if (Hallucination && !force) {
		pline("Maybe you should find a designated driver.");
		return false;
	}
	/* While riding Wounded_legs refers to the steed's,
	 * not the hero's legs.
	 * That opens up a potential abuse where the player
	 * can mount a steed, then dismount immediately to
	 * heal leg damage, because leg damage is always
	 * healed upon dismount (Wounded_legs context switch).
	 * By preventing a hero with Wounded_legs from
	 * mounting a steed, the potential for abuse is
	 * minimized, if not eliminated altogether.
	 */
	if (Wounded_legs) {
		pline("Your %s are in no shape for riding.", makeplural(body_part(LEG)));
		if (force && wizard && yn("Heal your legs?") == 'y')
			HWounded_legs = EWounded_legs = 0;
		else
			return false;
	}

	if (Upolyd && (!humanoid(youmonst.data) || verysmall(youmonst.data) ||
		       bigmonst(youmonst.data) || slithy(youmonst.data))) {
		pline("You won't fit on a saddle.");
		return false;
	}
	if (!force && (near_capacity() > SLT_ENCUMBER)) {
		pline("You can't do that while carrying so much stuff.");
		return false;
	}

	/* Can the player reach and see the monster? */
	if (!mtmp || (!force && ((Blind && !Blind_telepat) ||
				 mtmp->mundetected ||
				 mtmp->m_ap_type == M_AP_FURNITURE ||
				 mtmp->m_ap_type == M_AP_OBJECT))) {
		pline("I see nobody there.");
		return false;
	}
	if (u.uswallow || u.ustuck || u.utrap || Punished ||
	    !test_move(u.ux, u.uy, mtmp->mx - u.ux, mtmp->my - u.uy, TEST_MOVE)) {
		if (Punished || !(u.uswallow || u.ustuck || u.utrap))
			pline("You are unable to swing your %s over.", body_part(LEG));
		else
			pline("You are stuck here for now.");
		return false;
	}

	/* Is this a valid monster? */
	otmp = which_armor(mtmp, W_SADDLE);
	if (!otmp) {
		pline("%s is not saddled.", Monnam(mtmp));
		return false;
	}
	ptr = mtmp->data;
	if (touch_petrifies(ptr) && !Stone_resistance) {
		char kbuf[BUFSZ];

		pline("You touch %s.", mon_nam(mtmp));
		sprintf(kbuf, "attempting to ride %s", an(mtmp->data->mname));
		instapetrify(kbuf);
	}
	if (!mtmp->mtame || mtmp->isminion) {
		pline("I think %s would mind.", mon_nam(mtmp));
		return false;
	}
	if (mtmp->mtrapped) {
		struct trap *t = t_at(mtmp->mx, mtmp->my);

		pline("You can't mount %s while %s's trapped in %s.",
		      mon_nam(mtmp), mhe(mtmp),
		      an(sym_desc[trap_to_defsym(t->ttyp)].explanation));
		return false;
	}

	if (!force && !Role_if(PM_KNIGHT) && !(--mtmp->mtame)) {
		/* no longer tame */
		newsym(mtmp->mx, mtmp->my);
		pline("%s resists%s!", Monnam(mtmp),
		      mtmp->mleashed ? " and its leash comes off" : "");
		if (mtmp->mleashed) m_unleash(mtmp, false);
		return false;
	}
	if (!force && Underwater && !is_swimmer(ptr)) {
		pline("You can't ride that creature while under water.");
		return false;
	}
	if (!can_saddle(mtmp) || !can_ride(mtmp)) {
		pline("You can't ride such a creature.");
		return 0;
	}

	/* Is the player impaired? */
	if (!force && !is_floater(ptr) && !is_flyer(ptr) &&
	    Levitation && !Lev_at_will) {
		pline("You cannot reach %s.", mon_nam(mtmp));
		return false;
	}
	if (!force && uarm && is_metallic(uarm) &&
	    greatest_erosion(uarm)) {
		pline("Your %s armor is too stiff to be able to mount %s.",
		      uarm->oeroded ? "rusty" : "corroded",
		      mon_nam(mtmp));
		return false;
	}
	if (!force && (Confusion || Fumbling || Glib || Wounded_legs ||
		       otmp->cursed || (u.ulevel + mtmp->mtame < rnd(MAXULEV / 2 + 5)))) {
		if (Levitation) {
			pline("%s slips away from you.", Monnam(mtmp));
			return false;
		}
		pline("You slip while trying to get on %s.", mon_nam(mtmp));

		sprintf(buf, "slipped while mounting %s",
			/* "a saddled mumak" or "a saddled pony called Dobbin" */
			x_monnam(mtmp, ARTICLE_A, NULL,
				 SUPPRESS_IT | SUPPRESS_INVISIBLE | SUPPRESS_HALLUCINATION,
				 true));
		losehp(Maybe_Half_Phys(rn1(5, 10)), buf, NO_KILLER_PREFIX);
		return false;
	}

	/* Success */
	if (!force) {
		if (Levitation && !is_floater(ptr) && !is_flyer(ptr))
			/* Must have Lev_at_will at this point */
			pline("%s magically floats up!", Monnam(mtmp));
		pline("You mount %s.", mon_nam(mtmp));
	}
	/* setuwep handles polearms differently when you're mounted */
	if (uwep && is_pole(uwep)) unweapon = false;
	u.usteed = mtmp;
	remove_monster(mtmp->mx, mtmp->my);
	teleds(mtmp->mx, mtmp->my, true);
	return true;
}

/* You and your steed have moved */
void exercise_steed(void) {
	if (!u.usteed)
		return;

	/* It takes many turns of riding to exercise skill */
	if (u.urideturns++ >= 100) {
		u.urideturns = 0;
		use_skill(P_RIDING, 1);
	}
	return;
}

/*
 * Try to find a dismount point adjacent to the steed's location.
 * If all else fails, try enexto().  Use enexto() as a last resort because
 * enexto() chooses its point randomly, possibly even outside the
 * room's walls, which is not what we want.
 * Adapted from mail daemon code.
 */
static boolean landing_spot(coord *spot, int reason, int forceit) {
	int i = 0, x, y, distance, min_distance = -1;
	boolean found = false;
	struct trap *t;

	/* avoid known traps (i == 0) and boulders, but allow them as a backup */
	if (reason != DISMOUNT_BYCHOICE || Stunned || Confusion || Fumbling) i = 1;
	for (; !found && i < 2; ++i) {
		for (x = u.ux - 1; x <= u.ux + 1; x++)
			for (y = u.uy - 1; y <= u.uy + 1; y++) {
				if (!isok(x, y) || (x == u.ux && y == u.uy)) continue;

				if (ACCESSIBLE(levl[x][y].typ) &&
				    !MON_AT(x, y) && !closed_door(x, y)) {
					distance = distu(x, y);
					if (min_distance < 0 || distance < min_distance ||
					    (distance == min_distance && rn2(2))) {
						if (i > 0 || (((t = t_at(x, y)) == 0 || !t->tseen) &&
							      (!sobj_at(BOULDER, x, y) ||
							       throws_rocks(youmonst.data)))) {
							spot->x = x;
							spot->y = y;
							min_distance = distance;
							found = true;
						}
					}
				}
			}
	}

	/* If we didn't find a good spot and forceit is on, try enexto(). */
	if (forceit && min_distance < 0 &&
	    !enexto(spot, u.ux, u.uy, youmonst.data))
		return false;

	return found;
}

/* The player kicks or whips the steed */
void kick_steed(void) {
	char He[4];
	if (!u.usteed)
		return;

	/* [ALI] Various effects of kicking sleeping/paralyzed steeds */
	if (u.usteed->msleeping || !u.usteed->mcanmove) {
		/* We assume a message has just been output of the form
		 * "You kick <steed>."
		 */
		strcpy(He, mhe(u.usteed));
		*He = highc(*He);
		if ((u.usteed->mcanmove || u.usteed->mfrozen) && !rn2(2)) {
			if (u.usteed->mcanmove)
				u.usteed->msleeping = 0;
			else if (u.usteed->mfrozen > 2)
				u.usteed->mfrozen -= 2;
			else {
				u.usteed->mfrozen = 0;
				u.usteed->mcanmove = 1;
			}
			if (u.usteed->msleeping || !u.usteed->mcanmove)
				pline("%s stirs.", He);
			else
				pline("%s rouses %sself!", He, mhim(u.usteed));
		} else
			pline("%s does not respond.", He);
		return;
	}

	/* Make the steed less tame and check if it resists */
	if (u.usteed->mtame) u.usteed->mtame--;
	if (!u.usteed->mtame && u.usteed->mleashed) m_unleash(u.usteed, true);
	if (!u.usteed->mtame || (u.ulevel + u.usteed->mtame < rnd(MAXULEV / 2 + 5))) {
		newsym(u.usteed->mx, u.usteed->my);
		dismount_steed(DISMOUNT_THROWN);
		return;
	}

	pline("%s gallops!", Monnam(u.usteed));
	u.ugallop += rn1(20, 30);
	return;
}

/* Stop riding the current steed */
void dismount_steed(int reason) {
	struct monst *mtmp;
	struct obj *otmp;
	coord cc;
	const char *verb = "fall";
	boolean repair_leg_damage = true;
	unsigned save_utrap = u.utrap;
	boolean have_spot = landing_spot(&cc, reason, 0);

	mtmp = u.usteed; /* make a copy of steed pointer */
	/* Sanity check */
	if (!mtmp) /* Just return silently */
		return;

	/* Check the reason for dismounting */
	otmp = which_armor(mtmp, W_SADDLE);
	switch (reason) {
		case DISMOUNT_THROWN:
			verb = "are thrown";
		fallthru;
		case DISMOUNT_FELL:
			pline("You %s off of %s!", verb, mon_nam(mtmp));
			if (!have_spot) have_spot = landing_spot(&cc, reason, 1);
			losehp(Maybe_Half_Phys(rn1(10, 10)), "riding accident", KILLED_BY_AN);
			set_wounded_legs(BOTH_SIDES, HWounded_legs + rn1(5, 5));
			repair_leg_damage = false;
			break;
		case DISMOUNT_POLY:
			pline("You can no longer ride %s.", mon_nam(u.usteed));
			if (!have_spot) have_spot = landing_spot(&cc, reason, 1);
			break;
		case DISMOUNT_ENGULFED:
			/* caller displays message */
			break;
		case DISMOUNT_BONES:
			/* hero has just died... */
			break;
		case DISMOUNT_GENERIC:
			/* no messages, just make it so */
			break;
		case DISMOUNT_BYCHOICE:
		default:
			if (otmp && otmp->cursed) {
				pline("You can't.  The saddle %s cursed.",
				      otmp->bknown ? "is" : "seems to be");
				otmp->bknown = true;
				return;
			}
			if (!have_spot) {
				pline("You can't. There isn't anywhere for you to stand.");
				return;
			}
			if (!has_name(mtmp)) {
				pline("You've been through the dungeon on %s with no name.",
				      an(mtmp->data->mname));
				if (Hallucination)
					pline("It felt good to get out of the rain.");
			} else
				pline("You dismount %s.", mon_nam(mtmp));
	}
	/* While riding these refer to the steed's legs
	 * so after dismounting they refer to the player's
	 * legs once again.
	 */
	if (repair_leg_damage) HWounded_legs = EWounded_legs = 0;

	/* Release the steed and saddle */
	u.usteed = 0;
	u.ugallop = 0L;

	/* Set player and steed's position.  Try moving the player first
	   unless we're in the midst of creating a bones file. */
	if (reason == DISMOUNT_BONES) {
		/* move the steed to an adjacent square */
		if (enexto(&cc, u.ux, u.uy, mtmp->data))
			rloc_to(mtmp, cc.x, cc.y);
		else /* evidently no room nearby; move steed elsewhere */
			rloc(mtmp, false);
		return;
	}
	if (!DEADMONSTER(mtmp)) {
		place_monster(mtmp, u.ux, u.uy);
		if (!u.uswallow && !u.ustuck && have_spot) {
			struct permonst *mdat = mtmp->data;

			/* The steed may drop into water/lava */
			if (!is_flyer(mdat) && !is_floater(mdat) && !is_clinger(mdat)) {
				if (is_pool(u.ux, u.uy)) {
					if (!Underwater)
						pline("%s falls into the %s!", Monnam(mtmp),
						      surface(u.ux, u.uy));
					if (!is_swimmer(mdat) && !amphibious(mdat)) {
						killed(mtmp);
						adjalign(-1);
					}
				} else if (is_lava(u.ux, u.uy)) {
					pline("%s is pulled into the lava!", Monnam(mtmp));
					if (!likes_lava(mdat)) {
						killed(mtmp);
						adjalign(-1);
					}
				}
			}
			/* Steed dismounting consists of two steps: being moved to another
			 * square, and descending to the floor.  We have functions to do
			 * each of these activities, but they're normally called
			 * individually and include an attempt to look at or pick up the
			 * objects on the floor:
			 * teleds() --> spoteffects() --> pickup()
			 * float_down() --> pickup()
			 * We use this kludge to make sure there is only one such attempt.
			 *
			 * Clearly this is not the best way to do it.  A full fix would
			 * involve having these functions not call pickup() at all, instead
			 * calling them first and calling pickup() afterwards.  But it
			 * would take a lot of work to keep this change from having any
			 * unforseen side effects (for instance, you would no longer be
			 * able to walk onto a square with a hole, and autopickup before
			 * falling into the hole).
			 */
			/* [ALI] No need to move the player if the steed died. */
			if (!DEADMONSTER(mtmp)) {
				/* Keep steed here, move the player to cc;
				 * teleds() clears u.utrap
				 */
				in_steed_dismounting = true;
				teleds(cc.x, cc.y, true);
				in_steed_dismounting = false;

				/* Put your steed in your trap */
				if (save_utrap)
					mintrap(mtmp);
			}
			/* Couldn't... try placing the steed */
		} else if (enexto(&cc, u.ux, u.uy, mtmp->data)) {
			/* Keep player here, move the steed to cc */
			rloc_to(mtmp, cc.x, cc.y);
			/* Player stays put */
			/* Otherwise, kill the steed */
		} else {
			killed(mtmp);
			adjalign(-1);
		}
	}

	/* Return the player to the floor */
	if (reason != DISMOUNT_ENGULFED) {
		in_steed_dismounting = true;
		float_down(0L, W_SADDLE);
		in_steed_dismounting = false;
		context.botl = 1;
		encumber_msg();
		vision_full_recalc = 1;
	} else
		context.botl = 1;
	/* polearms behave differently when not mounted */
	if (uwep && is_pole(uwep)) unweapon = true;
	return;
}

void place_monster(struct monst *mon, int x, int y) {
	if (mon == u.usteed ||
	    /* special case is for convoluted vault guard handling */
	    (DEADMONSTER(mon) && !(mon->isgd && x == 0 && y == 0))) {
		impossible("placing %s onto map?",
			   (mon == u.usteed) ? "steed" : "defunct monster");
		return;
	}
	mon->mx = x, mon->my = y;
	level.monsters[x][y] = mon;
}

/*steed.c*/
