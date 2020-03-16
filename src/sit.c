/*	SCCS Id: @(#)sit.c	3.4	2002/09/21	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "artifact.h"

void take_gold() {
	struct obj *otmp, *nobj;
	int lost_money = 0;
	for (otmp = invent; otmp; otmp = nobj) {
		nobj = otmp->nobj;
		if (otmp->oclass == COIN_CLASS) {
			lost_money = 1;
			delobj(otmp);
		}
	}
	if (!lost_money) {
		pline("You feel a strange sensation.");
	} else {
		pline("You notice you have no money!");
		context.botl = 1;
	}
}

int dosit() {
	struct trap *trap = t_at(u.ux, u.uy);
	int typ = levl[u.ux][u.uy].typ;

	if (u.usteed) {
		pline("You are already sitting on %s.", mon_nam(u.usteed));
		return 0;
	}

	if (u.uundetected && is_hider(youmonst.data) && u.umonnum != PM_TRAPPER)
		u.uundetected = 0; // no longer on the ceiling

	if (!can_reach_floor()) {
		if (Levitation)
			pline("You tumble in place.");
		else
			pline("You are sitting on air.");
		return 0;
	} else if (is_pool(u.ux, u.uy) && !Underwater) { /* water walking */
		goto in_water;
	}

	if (OBJ_AT(u.ux, u.uy) && !uteetering_at_seen_pit()) {
		struct obj *obj;

		obj = level.objects[u.ux][u.uy];
		pline("You sit on %s.", the(xname(obj)));
		if (!(Is_box(obj) || objects[obj->otyp].oc_material == CLOTH))
			pline("It's not very comfortable...");
	} else if (trap || (u.utrap && (u.utraptype >= TT_LAVA))) {
		if (u.utrap) {
			exercise(A_WIS, false); /* you're getting stuck longer */
			if (u.utraptype == TT_BEARTRAP) {
				pline("You can't sit down with your %s in the bear trap.", body_part(FOOT));
				u.utrap++;
			} else if (trap && u.utraptype == TT_PIT) {
				if (trap->ttyp == SPIKED_PIT) {
					pline("You sit down on a spike.  Ouch!");
					losehp(Maybe_Half_Phys(1), "sitting on an iron spike", KILLED_BY);
					exercise(A_STR, false);
				} else
					pline("You sit down in the pit.");
				u.utrap += rn2(5);
			} else if (u.utraptype == TT_WEB) {
				pline("You sit in the spider web and get entangled further!");
				u.utrap += rn1(10, 5);
			} else if (u.utraptype == TT_LAVA) {
				/* Must have fire resistance or they'd be dead already */
				pline("You sit in the lava!");
				u.utrap += rnd(4);
				losehp(d(2, 10), "sitting in lava", KILLED_BY);
			} else if (u.utraptype == TT_INFLOOR) {
				pline("You can't maneuver to sit!");
				u.utrap++;
			}
		} else {
			pline("You sit down.");
			dotrap(trap, 0);
		}
	} else if (Underwater || Is_waterlevel(&u.uz)) {
		if (Is_waterlevel(&u.uz))
			pline("There are no cushions floating nearby.");
		else
			pline("You sit down on the muddy bottom.");
	} else if (is_pool(u.ux, u.uy)) {
	in_water:
		pline("You sit in the water.");
		if (!rn2(10) && uarm)
			rust_dmg(uarm, "armor", 1, true, &youmonst);
		else if (!rn2(10) && uarmf && uarmf->otyp != WATER_WALKING_BOOTS)
			rust_dmg(uarm, "armor", 1, true, &youmonst);
	} else if (IS_SINK(typ)) {
		pline("You sit on the %s.", sym_desc[S_sink].explanation);
		pline("Your %s gets wet.", humanoid(youmonst.data) ? "rump" : "underside");
	} else if (IS_TOILET(typ)) {
		pline("You sit on the %s.", sym_desc[S_toilet].explanation);
		if ((!Sick) && (u.uhs > 0))
			pline("You don't have to go...");
		else {
			if (Role_if(PM_BARBARIAN) || Role_if(PM_CAVEMAN))
				pline("You miss...");
			else
				pline("You grunt.");
			if (Sick) make_sick(0L, NULL, true, SICK_ALL);
			if (u.uhs == 0) morehungry(rn2(400) + 200);
		}
	} else if (IS_ALTAR(typ)) {
		pline("You sit on the %s.", sym_desc[S_altar].explanation);
		altar_wrath(u.ux, u.uy);

	} else if (IS_GRAVE(typ)) {
		pline("You sit on the %s.", sym_desc[S_grave].explanation);

	} else if (typ == STAIRS) {
		pline("You sit on the stairs.");

	} else if (typ == LADDER) {
		pline("You sit on the ladder.");

	} else if (is_lava(u.ux, u.uy)) {
		/* must be WWalking */
		pline("You sit on the lava.");
		burn_away_slime();
		if (likes_lava(youmonst.data)) {
			pline("The lava feels warm.");
			return 1;
		}
		pline("The lava burns you!");
		if (Slimed) {
			pline("The slime is burned away!");
			Slimed = 0;
		}
		losehp(d((Fire_resistance ? 2 : 10), 10), "sitting on lava", KILLED_BY);

	} else if (is_ice(u.ux, u.uy)) {
		pline("You sit on the %s.", sym_desc[S_ice].explanation);
		if (!Cold_resistance) pline("The ice feels cold.");

	} else if (typ == DRAWBRIDGE_DOWN) {
		pline("You sit on the drawbridge.");

	} else if (IS_THRONE(typ)) {
		pline("You sit on the %s.", sym_desc[S_throne].explanation);
		if (rnd(6) > 4) {
			switch (rnd(13)) {
				case 1:
					adjattrib(rn2(A_MAX), -rn1(4, 3), false);
					losehp(rnd(10), "cursed throne", KILLED_BY_AN);
					break;
				case 2:
					adjattrib(rn2(A_MAX), 1, false);
					break;
				case 3:
					pline("A%s electric shock shoots through your body!",
					      (Shock_resistance) ? "n" : " massive");
					losehp(Shock_resistance ? rnd(6) : rnd(30),
					       "electric chair", KILLED_BY_AN);
					exercise(A_CON, false);
					break;
				case 4:
					pline("You feel much, much better!");
					if (Upolyd) {
						if (u.mh >= (u.mhmax - 5)) u.mhmax += 4;
						u.mh = u.mhmax;
					}
					if (u.uhp >= (u.uhpmax - 5)) u.uhpmax += 4;
					u.uhp = u.uhpmax;
					make_blinded(0L, true);
					make_sick(0L, NULL, false, SICK_ALL);
					heal_legs();
					context.botl = 1;
					break;
				case 5:
					take_gold();
					break;
				case 6:
					/* ------------===========STEPHEN WHITE'S NEW CODE============------------ */
					if (u.uluck < 7) {
						pline("You feel your luck is changing.");
						change_luck(5);
					} else
						makewish();
					break;
				case 7: {
					int cnt = rnd(10);

					pline("A voice echoes:");
					verbalize("Thine audience hath been summoned, %s!",
						  flags.female ? "Dame" : "Sire");
					while (cnt--)
						makemon(courtmon(), u.ux, u.uy, NO_MM_FLAGS);
					break;
				}
				case 8:
					pline("A voice echoes:");
					verbalize("By thine Imperious order, %s...",
						  flags.female ? "Dame" : "Sire");
					do_genocide(5); /* REALLY|ONTHRONE, see do_genocide() */
					break;
				case 9:
					pline("A voice echoes:");
					verbalize("A curse upon thee for sitting upon this most holy throne!");
					if (Luck > 0) {
						make_blinded(Blinded + rn1(100, 250), true);
					} else
						rndcurse();
					break;
				case 10:
					if (Luck < 0 || (HSee_invisible & INTRINSIC)) {
						if (level.flags.nommap) {
							pline(
								"A terrible drone fills your head!");
							make_confused(HConfusion + rnd(30),
								      false);
						} else {
							pline("An image forms in your mind.");
							do_mapping();
						}
					} else {
						pline("Your vision becomes clear.");
						HSee_invisible |= FROMOUTSIDE;
						newsym(u.ux, u.uy);
					}
					break;
				case 11:
					if (Luck < 0) {
						pline("You feel threatened.");
						aggravate();
					} else {
						pline("You feel a wrenching sensation.");
						tele(); /* teleport him */
					}
					break;
				case 12:
					pline("You are granted an insight!");
					if (invent) {
						/* rn2(5) agrees w/seffects() */
						identify_pack(rn2(5));
					}
					break;
				case 13:
					pline("Your mind turns into a pretzel!");
					make_confused(HConfusion + rn1(7, 16), false);
					break;
				default:
					impossible("throne effect");
					break;
			}
		} else {
			if (is_prince(youmonst.data))
				pline("You feel very comfortable here.");
			else
				pline("You feel somehow out of place...");
		}

		if (!rn2(3) && IS_THRONE(levl[u.ux][u.uy].typ)) {
			/* may have teleported */
			levl[u.ux][u.uy].typ = ROOM;
			pline("The throne vanishes in a puff of logic.");
			newsym(u.ux, u.uy);
		}

	} else if (lays_eggs(youmonst.data)) {
		struct obj *uegg;

		if (!flags.female) {
			pline("Males can't lay eggs!");
			return 0;
		}

		if (u.uhunger < (int)objects[EGG].oc_nutrition) {
			pline("You don't have enough energy to lay an egg.");
			return 0;
		}

		uegg = mksobj(EGG, false, false);
		uegg->spe = 1;
		uegg->quan = 1;
		uegg->owt = weight(uegg);
		uegg->corpsenm = egg_type_from_parent(u.umonnum, false);
		uegg->known = uegg->dknown = 1;
		attach_egg_hatch_timeout(uegg);
		pline("You lay an egg.");
		dropy(uegg);
		stackobj(uegg);
		morehungry((int)objects[EGG].oc_nutrition);
	} else if (u.uswallow)
		pline("There are no seats in here!");
	else
		pline("Having fun sitting on the %s?", surface(u.ux, u.uy));
	return 1;
}

void rndcurse(void) { /* curse a few inventory items at random! */
	int nobj = 0;
	int cnt, onum;
	struct obj *otmp;

	if (uwep && (uwep->oartifact == ART_MAGICBANE) && rn2(20)) {
		pline("You feel a malignant aura surround the magic-absorbing blade.");
		return;
	}

	if (Antimagic) {
		shieldeff(u.ux, u.uy);
		pline("You feel a malignant aura surround you.");
	}

	for (otmp = invent; otmp; otmp = otmp->nobj) {
		/* gold isn't subject to being cursed or blessed */
		if (otmp->oclass == COIN_CLASS) continue;
		nobj++;
	}
	if (nobj) {
		for (cnt = rnd(6 / ((!!Antimagic) + (!!Half_spell_damage) + 1));
		     cnt > 0;
		     cnt--) {
			onum = rnd(nobj);
			for (otmp = invent; otmp; otmp = otmp->nobj) {
				/* as above */
				if (otmp->oclass == COIN_CLASS) continue;
				if (--onum == 0) break; /* found the target */
			}
			/* the !otmp case should never happen; picking an already
			   cursed item happens--avoid "resists" message in that case */
			if (!otmp || otmp->cursed) continue; /* next target */

			if (otmp->oartifact && spec_ability(otmp, SPFX_INTEL) &&
			    rn2(10) < 8) {
				pline("%s!", Tobjnam(otmp, "resist"));
				continue;
			}

			if (otmp->blessed)
				unbless(otmp);
			else
				curse(otmp);
		}
		update_inventory();
	}

	/* treat steed's saddle as extended part of hero's inventory */
	if (u.usteed && !rn2(4) &&
	    (otmp = which_armor(u.usteed, W_SADDLE)) != 0 &&
	    !otmp->cursed) { /* skip if already cursed */
		if (otmp->blessed)
			unbless(otmp);
		else
			curse(otmp);
		if (!Blind) {
			pline("%s %s.",
			      Yobjnam2(otmp, "glow"),
			      hcolor(otmp->cursed ? NH_BLACK : (const char *)"brown"));
			otmp->bknown = true;
		}
	}
}

void attrcurse(void) { /* remove a random INTRINSIC ability */
	switch (rnd(11)) {
		case 1:
			if (HFire_resistance & INTRINSIC) {
				HFire_resistance &= ~INTRINSIC;
				pline("You feel warmer.");
				break;
			}
		//fallthru
		case 2:
			if (HTeleportation & INTRINSIC) {
				HTeleportation &= ~INTRINSIC;
				pline("You feel less jumpy.");
				break;
			}
		//fallthru
		case 3:
			if (HPoison_resistance & INTRINSIC) {
				HPoison_resistance &= ~INTRINSIC;
				pline("You feel a little sick!");
				break;
			}
		//fallthru
		case 4:
			if (HTelepat & INTRINSIC) {
				HTelepat &= ~INTRINSIC;
				if (Blind && !Blind_telepat)
					see_monsters(); /* Can't sense mons anymore! */
				pline("Your senses fail!");
				break;
			}
		//fallthru
		case 5:
			if (HCold_resistance & INTRINSIC) {
				HCold_resistance &= ~INTRINSIC;
				pline("You feel cooler.");
				break;
			}
		//fallthru
		case 6:
			if (HInvis & INTRINSIC) {
				HInvis &= ~INTRINSIC;
				pline("You feel paranoid.");
				break;
			}
		//fallthru
		case 7:
			if (HSee_invisible & INTRINSIC) {
				HSee_invisible &= ~INTRINSIC;
				pline("You %s!", Hallucination ? "tawt you taw a puttie tat" : "thought you saw something");
				break;
			}
		//fallthru
		case 8:
			if (HFast & INTRINSIC) {
				HFast &= ~INTRINSIC;
				pline("You feel slower.");
				break;
			}
		//fallthru
		case 9:
			if (HStealth & INTRINSIC) {
				HStealth &= ~INTRINSIC;
				pline("You feel clumsy.");
				break;
			}
		//fallthru
		case 10:
			if (HProtection & INTRINSIC) {
				HProtection &= ~INTRINSIC;
				pline("You feel vulnerable.");
				break;
			}
		//fallthru
		case 11:
			if (HAggravate_monster & INTRINSIC) {
				HAggravate_monster &= ~INTRINSIC;
				pline("You feel less attractive.");
				break;
			}
			break;
	}
}

/*sit.c*/
