/*	SCCS Id: @(#)do_wear.c	3.4	2003/11/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static const char see_yourself[] = "see yourself";
static const char unknown_type[] = "Unknown type of %s (%d)";
static const char c_armor[] = "armor",
		  c_suit[] = "suit",
		  c_shirt[] = "shirt",
		  c_cloak[] = "cloak",
		  c_gloves[] = "gloves",
		  c_boots[] = "boots",
		  c_helmet[] = "helmet",
		  c_shield[] = "shield",
		  c_weapon[] = "weapon",
		  c_sword[] = "sword",
		  c_axe[] = "axe",
		  c_that_[] = "that";

static const long takeoff_order[] = {WORN_BLINDF, W_WEP,
				     WORN_SHIELD, WORN_GLOVES, LEFT_RING, RIGHT_RING, WORN_CLOAK,
				     WORN_HELMET, WORN_AMUL, WORN_ARMOR, WORN_SHIRT, WORN_BOOTS,
				     W_SWAPWEP, W_QUIVER, 0L};

static void on_msg(struct obj *otmp);
static void Ring_off_or_gone(struct obj *obj, bool gone);
static int select_off(struct obj *otmp);
static struct obj *do_takeoff(void);
static int take_off(void);
static int menu_remarm(int retry);
static void already_wearing(const char *cc);
static void already_wearing2(const char *cc1, const char *cc2);
static void wielding_corpse(struct obj *obj, bool voluntary);

void off_msg(struct obj *otmp) {
	if (flags.verbose)
		pline("You were wearing %s.", doname(otmp));
}

/* for items that involve no delay */
static void on_msg(struct obj *otmp) {
	if (flags.verbose) {
		char how[BUFSZ];

		how[0] = '\0';
		if (otmp->otyp == TOWEL)
			sprintf(how, " around your %s", body_part(HEAD));
		pline("You are now wearing %s%s.",
		      obj_is_pname(otmp) ? the(xname(otmp)) : an(xname(otmp)),
		      how);
	}
}

/*
 * The Type_on() functions should be called *after* setworn().
 * The Type_off() functions call setworn() themselves.
 */

int Boots_on(void) {
	long oldprop =
		u.uprops[objects[uarmf->otyp].oc_oprop].extrinsic & ~WORN_BOOTS;

	switch (uarmf->otyp) {
		case LOW_BOOTS:
		case IRON_SHOES:
		case HIGH_BOOTS:
		case JUMPING_BOOTS:
		case KICKING_BOOTS:
			break;
		case WATER_WALKING_BOOTS:
			if (u.uinwater) spoteffects(true);
			break;
		case SPEED_BOOTS:
			/* Speed boots are still better than intrinsic speed, */
			/* though not better than potion speed */
			if (!oldprop && !(HFast & TIMEOUT)) {
				makeknown(uarmf->otyp);
				pline("You feel yourself speed up%s.",
				      (oldprop || HFast) ? " a bit more" : "");
			}
			break;
		case ELVEN_BOOTS:
			if (!oldprop && !HStealth && !BStealth) {
				makeknown(uarmf->otyp);
				pline("You walk very quietly.");
			}
			break;
		case FUMBLE_BOOTS:
			if (!oldprop && !(HFumbling & ~TIMEOUT))
				incr_itimeout(&HFumbling, rnd(20));
			break;
		case LEVITATION_BOOTS:
			if (!oldprop && !HLevitation) {
				context.botl = 1;
				makeknown(uarmf->otyp);
				float_up();
				spoteffects(false);
			}
			break;
		default:
			impossible(unknown_type, c_boots, uarmf->otyp);
	}
	return 0;
}

int Boots_off(void) {
	int otyp = uarmf->otyp;
	long oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_BOOTS;

	context.takeoff.mask &= ~W_ARMF;
	/* For levitation, float_down() returns if Levitation, so we
	 * must do a setworn() _before_ the levitation case.
	 */
	setworn(NULL, W_ARMF);
	switch (otyp) {
		case SPEED_BOOTS:
			if (!Very_fast && !context.takeoff.cancelled_don) {
				makeknown(otyp);
				pline("You feel yourself slow down%s.",
				      Fast ? " a bit" : "");
			}
			break;
		case WATER_WALKING_BOOTS:
			if ((is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) && !Levitation && !Flying
			    && !is_clinger(youmonst.data) && !context.takeoff.cancelled_don
			    // avoid recursive call to lava_effects()
			    && !iflags.in_lava_effects) {
				makeknown(otyp);
				/* make boots known in case you survive the drowning */
				spoteffects(true);
			}
			break;
		case ELVEN_BOOTS:
			if (!oldprop && !HStealth && !BStealth && !context.takeoff.cancelled_don) {
				makeknown(otyp);
				pline("You sure are noisy.");
			}
			break;
		case FUMBLE_BOOTS:
			if (!oldprop && !(HFumbling & ~TIMEOUT))
				HFumbling = EFumbling = 0;
			break;
		case LEVITATION_BOOTS:
			if (!oldprop && !HLevitation && !context.takeoff.cancelled_don) {
				context.botl = 1;
				float_down(0L, 0L);
				makeknown(otyp);
			}
			break;
		case LOW_BOOTS:
		case IRON_SHOES:
		case HIGH_BOOTS:
		case JUMPING_BOOTS:
		case KICKING_BOOTS:
			break;
		default:
			impossible(unknown_type, c_boots, otyp);
	}
	context.takeoff.cancelled_don = false;
	return 0;
}

int Cloak_on(void) {
	long oldprop =
		u.uprops[objects[uarmc->otyp].oc_oprop].extrinsic & ~WORN_CLOAK;

	switch (uarmc->otyp) {
		case ELVEN_CLOAK:
		case CLOAK_OF_PROTECTION:
		case CLOAK_OF_DISPLACEMENT:
			makeknown(uarmc->otyp);
			break;
		case ORCISH_CLOAK:
		case DWARVISH_CLOAK:
		case CLOAK_OF_MAGIC_RESISTANCE:
		case ROBE:
		case LEATHER_CLOAK:
			/* KMH, balance patch -- removed
		case CLOAK_OF_DRAIN_RESISTANCE: */
			break;
		/* KMH, balance patch -- lab coat gives poison _and_ acid resistance */
		case LAB_COAT:
			EAcid_resistance |= WORN_CLOAK;
			break;
		case MUMMY_WRAPPING:
			/* Note: it's already being worn, so we have to cheat here. */
			if ((HInvis || EInvis || pm_invisible(youmonst.data)) && !Blind) {
				newsym(u.ux, u.uy);
				pline("You can %s!",
				      See_invisible ? "no longer see through yourself" : see_yourself);
			}
			break;
		case CLOAK_OF_INVISIBILITY:
			/* since cloak of invisibility was worn, we know mummy wrapping
		   wasn't, so no need to check `oldprop' against blocked */
			if (!oldprop && !HInvis && !Blind) {
				makeknown(uarmc->otyp);
				newsym(u.ux, u.uy);
				pline("Suddenly you can%s yourself.",
				      See_invisible ? " see through" : "not see");
			}
			break;
		case POISONOUS_CLOAK:
			if (Poison_resistance)
				pline("This cloak feels a little itchy.");
			else {
				makeknown(uarmc->otyp);
				poisoned("cloak", A_STR, "poisonous cloak", 3);
			}
			break;
		case OILSKIN_CLOAK:
			pline("%s very tightly.", Tobjnam(uarmc, "fit"));
			break;
			/* Alchemy smock gives poison _and_ acid resistance */
		default:
			impossible(unknown_type, c_cloak, uarmc->otyp);
	}
	return 0;
}

int Cloak_off(void) {
	int otyp = uarmc->otyp;
	long oldprop = u.uprops[objects[otyp].oc_oprop].extrinsic & ~WORN_CLOAK;

	context.takeoff.mask &= ~W_ARMC;
	/* For mummy wrapping, taking it off first resets `Invisible'. */
	setworn(NULL, W_ARMC);
	switch (otyp) {
		case ELVEN_CLOAK:
		case ORCISH_CLOAK:
		case DWARVISH_CLOAK:
		case CLOAK_OF_PROTECTION:
		case CLOAK_OF_MAGIC_RESISTANCE:
		case CLOAK_OF_DISPLACEMENT:
		case POISONOUS_CLOAK:
		case OILSKIN_CLOAK:
		case ROBE:
		case LEATHER_CLOAK:
			/* KMH, balance patch -- removed
		case CLOAK_OF_DRAIN_RESISTANCE: */
			break;
		/* KMH, balance patch -- lab coat gives poison _and_ acid resistance */
		case LAB_COAT:
			EAcid_resistance &= ~WORN_CLOAK;
			break;
		case MUMMY_WRAPPING:
			if (Invis && !Blind) {
				newsym(u.ux, u.uy);
				pline("You can %s.",
				      See_invisible ? "see through yourself" : "no longer see yourself");
			}
			break;
		case CLOAK_OF_INVISIBILITY:
			if (!oldprop && !HInvis && !Blind) {
				makeknown(CLOAK_OF_INVISIBILITY);
				newsym(u.ux, u.uy);
				pline("Suddenly you can %s.",
				      See_invisible ? "no longer see through yourself" : see_yourself);
			}
			break;
		default:
			impossible(unknown_type, c_cloak, otyp);
	}
	return 0;
}

int Helmet_on(void) {
	if (ARMOR_SHOULD_AUTOCURSE(uarmh) && !uarmh->cursed) {
		if (Blind)
			pline("%s for a moment.", Tobjnam(uarmh, "vibrate"));
		else
			pline("%s %s for a moment.",
					Tobjnam(uarmh, "glow"), hcolor(NH_BLACK));
		curse(uarmh);
	}

	switch (uarmh->otyp) {
		case FEDORA:
			set_moreluck();
			context.botl = 1;
			break;
		case HELMET:
		case DENTED_POT:
		case ELVEN_LEATHER_HELM:
		case DWARVISH_IRON_HELM:
		case ORCISH_HELM:
		case HELM_OF_TELEPATHY:
			break;
		case HELM_OF_BRILLIANCE:
			adj_abon(uarmh, uarmh->spe);
			break;
		case CORNUTHAUM:
			/* people think marked wizards know what they're talking
			 * about, but it takes trained arrogance to pull it off,
			 * and the actual enchantment of the hat is irrelevant.
			 */
			ABON(A_CHA) += (Role_if(PM_WIZARD) ? 1 : -1);
			context.botl = 1;
			makeknown(uarmh->otyp);
			break;
		case HELM_OF_OPPOSITE_ALIGNMENT:
			if (u.ualign.type == A_NEUTRAL)
				u.ualign.type = rn2(2) ? A_CHAOTIC : A_LAWFUL;
			else
				u.ualign.type = -(u.ualign.type);
			u.ublessed = 0; /* lose your god's protection */
			context.botl = 1; /* reveal new alignment */

			if (Hallucination) {
				pline("My brain hurts!"); /* Monty Python's Flying Circus */
			} else {
				pline("Your mind oscillates briefly.");
				makeknown(HELM_OF_OPPOSITE_ALIGNMENT);
			}

			break;

		case DUNCE_CAP:
			context.botl = 1; /* reveal new INT/WIS */
			if (Hallucination) {
				pline("My brain hurts!"); /* Monty Python's Flying Circus */
			} else {
				pline("You feel %s.", /* track INT change; ignore WIS */
				      ACURR(A_INT) <= (ABASE(A_INT) + ABON(A_INT) + ATEMP(A_INT)) ?
					      "like sitting in a corner" :
					      "giddy");
			}

			break;

		default:
			impossible(unknown_type, c_helmet, uarmh->otyp);
	}
	return 0;
}

int Helmet_off(void) {
	context.takeoff.mask &= ~W_ARMH;

	switch (uarmh->otyp) {
		case FEDORA:
			setworn(NULL, W_ARMH);
			set_moreluck();
			context.botl = 1;
			return 0;
		case HELMET:
		case DENTED_POT:
		case ELVEN_LEATHER_HELM:
		case DWARVISH_IRON_HELM:
		case ORCISH_HELM:
			/* KMH, balance patch -- removed
		case FIRE_HELMET: */
			break;
		case DUNCE_CAP:
			context.botl = 1;
			break;
		case CORNUTHAUM:
			if (!context.takeoff.cancelled_don) {
				ABON(A_CHA) += (Role_if(PM_WIZARD) ? -1 : 1);
				context.botl = 1;
			}
			break;
		case HELM_OF_TELEPATHY:
			/* need to update ability before calling see_monsters() */
			setworn(NULL, W_ARMH);
			see_monsters();
			return 0;
		case HELM_OF_BRILLIANCE:
			if (!context.takeoff.cancelled_don) adj_abon(uarmh, -uarmh->spe);
			break;
		case HELM_OF_OPPOSITE_ALIGNMENT:
			u.ualign.type = u.ualignbase[A_CURRENT];
			u.ublessed = 0; /* lose the other god's protection */
			context.botl = 1;
			break;
		default:
			impossible(unknown_type, c_helmet, uarmh->otyp);
	}
	setworn(NULL, W_ARMH);
	context.takeoff.cancelled_don = false;
	return 0;
}

int Gloves_on(void) {
	long oldprop =
		u.uprops[objects[uarmg->otyp].oc_oprop].extrinsic & ~WORN_GLOVES;

	switch (uarmg->otyp) {
		case LEATHER_GLOVES:
			break;
		case GAUNTLETS_OF_SWIMMING:
			if (u.uinwater) {
				pline("Hey! You can swim!");
				spoteffects(true);
			}
			break;
		case GAUNTLETS_OF_FUMBLING:
			if (!oldprop && !(HFumbling & ~TIMEOUT))
				incr_itimeout(&HFumbling, rnd(20));
			break;
		case GAUNTLETS_OF_POWER:
			makeknown(uarmg->otyp);
			context.botl = 1; /* taken care of in attrib.c */
			break;
		case GAUNTLETS_OF_DEXTERITY:
			adj_abon(uarmg, uarmg->spe);
			break;
		default:
			impossible(unknown_type, c_gloves, uarmg->otyp);
	}
	return 0;
}

// voluntary => taking gloves off on purpose?
static void wielding_corpse(struct obj *obj, bool voluntary) {
	char kbuf[BUFSZ];

	if (!obj || obj->otyp != CORPSE) return;
	if (obj != uwep && (obj != uswapwep || !u.twoweap)) return;

	if (touch_petrifies(&mons[obj->corpsenm]) && !Stone_resistance) {
		pline("You now wield %s in your bare %s.",
		      the(corpse_xname(obj, true)),
		      makeplural(body_part(HAND)));
		sprintf(kbuf, "%s gloves while wielding %s",
		        voluntary ? "removing" : "losing",
		        killer_cxname(obj, true));
		instapetrify(kbuf);
		// life-saved; can't continue wielding cockatrice corpse though
		remove_worn_item(obj, false);
	}
}



int Gloves_off(void) {
	long oldprop = u.uprops[objects[uarmg->otyp].oc_oprop].extrinsic & ~WORN_GLOVES;

	context.takeoff.mask &= ~W_ARMG;
	bool on_purpose = !context.mon_moving && !uarmg->in_use;

	switch (uarmg->otyp) {
		case LEATHER_GLOVES:
			break;
		case GAUNTLETS_OF_SWIMMING:
			if (u.uinwater) {
				pline("You begin to thrash about!");
				spoteffects(true);
			}
			break;
		case GAUNTLETS_OF_FUMBLING:
			if (!oldprop && !(HFumbling & ~TIMEOUT))
				HFumbling = EFumbling = 0;
			break;
		case GAUNTLETS_OF_POWER:
			makeknown(uarmg->otyp);
			context.botl = 1; /* taken care of in attrib.c */
			break;
		case GAUNTLETS_OF_DEXTERITY:
			if (!context.takeoff.cancelled_don) adj_abon(uarmg, -uarmg->spe);
			break;
		default:
			impossible(unknown_type, c_gloves, uarmg->otyp);
	}
	setworn(NULL, W_ARMG);
	context.takeoff.cancelled_don = false;
	encumber_msg(); /* immediate feedback for GoP */

	/* prevent wielding cockatrice when not wearing gloves */
	if (uwep && uwep->otyp == CORPSE)
		wielding_corpse(uwep, on_purpose);

	/* KMH -- ...or your secondary weapon when you're wielding it
	   [This case can't actually happen; twoweapon mode won't
	   engage if a corpse has been set up as the alternate weapon.] */
	if (u.twoweap && uswapwep && uswapwep->otyp == CORPSE)
		wielding_corpse(uswapwep, on_purpose);


	return 0;
}

int Shield_on(void) {
	/*
	    switch (uarms->otyp) {
		case SMALL_SHIELD:
		case ELVEN_SHIELD:
		case URUK_HAI_SHIELD:
		case ORCISH_SHIELD:
		case DWARVISH_ROUNDSHIELD:
		case LARGE_SHIELD:
		case SHIELD_OF_REFLECTION:
			break;
		default: impossible(unknown_type, c_shield, uarms->otyp);
	    }
	*/
	return 0;
}

int Shield_off(void) {
	context.takeoff.mask &= ~W_ARMS;
	/*
	    switch (uarms->otyp) {
		case SMALL_SHIELD:
		case ELVEN_SHIELD:
		case URUK_HAI_SHIELD:
		case ORCISH_SHIELD:
		case DWARVISH_ROUNDSHIELD:
		case LARGE_SHIELD:
		case SHIELD_OF_REFLECTION:
			break;
		default: impossible(unknown_type, c_shield, uarms->otyp);
	    }
	*/
	setworn(NULL, W_ARMS);
	return 0;
}

int Shirt_on(void) {
	/*
	    switch (uarmu->otyp) {
		case HAWAIIAN_SHIRT:
		case T_SHIRT:
			break;
		default: impossible(unknown_type, c_shirt, uarmu->otyp);
	    }
	*/
	return 0;
}

int Shirt_off(void) {
	context.takeoff.mask &= ~W_ARMU;
	/*
	    switch (uarmu->otyp) {
		case HAWAIIAN_SHIRT:
		case T_SHIRT:
			break;
		default: impossible(unknown_type, c_shirt, uarmu->otyp);
	    }
	*/
	setworn(NULL, W_ARMU);
	return 0;
}

/* This must be done in worn.c, because one of the possible intrinsics conferred
 * is fire resistance, and we have to immediately set HFire_resistance in worn.c
 * since worn.c will check it before returning.
 */
int Armor_on(void) {
	/* KMH -- certain armor is obvious when worn */
	switch (uarm->otyp) {
		case ROBE_OF_PROTECTION:
		case ROBE_OF_WEAKNESS:
			makeknown(uarm->otyp);
			break;
	}
	return 0;
}

int Armor_off(void) {
	context.takeoff.mask &= ~W_ARM;
	setworn(NULL, W_ARM);
	context.takeoff.cancelled_don = false;
	return 0;
}

/* The gone functions differ from the off functions in that if you die from
 * taking it off and have life saving, you still die.
 */
int Armor_gone(void) {
	context.takeoff.mask &= ~W_ARM;
	setnotworn(uarm);
	context.takeoff.cancelled_don = false;
	return 0;
}

void Amulet_on(void) {
	switch (uamul->otyp) {
		case AMULET_OF_ESP:
		case AMULET_OF_LIFE_SAVING:
		case AMULET_VERSUS_POISON:
		case AMULET_OF_DRAIN_RESISTANCE:
		case AMULET_OF_REFLECTION:
		case AMULET_OF_MAGICAL_BREATHING:
		/* KMH, balance patch -- removed
	case AMULET_OF_REGENERATION:
	case AMULET_OF_CONFLICT:*/
		case FAKE_AMULET_OF_YENDOR:
			break;
		case AMULET_OF_UNCHANGING:
			if (Slimed) make_slimed(0, NULL);
			break;
		case AMULET_OF_CHANGE: {
			int orig_sex = poly_gender();

			if (Unchanging) break;
			change_sex();
			/* Don't use same message as polymorph */
			if (orig_sex != poly_gender()) {
				makeknown(AMULET_OF_CHANGE);
				pline("You are suddenly very %s!", flags.female ? "feminine" : "masculine");
				context.botl = 1;
			} else
				/* already polymorphed into single-gender monster; only
			   changed the character's base sex */
				pline("You don't feel like yourself.");
			pline("The amulet disintegrates!");
			if (orig_sex == poly_gender() && uamul->dknown &&
			    !objects[AMULET_OF_CHANGE].oc_name_known &&
			    !objects[AMULET_OF_CHANGE].oc_uname)
				docall(uamul);
			useup(uamul);
			break;
		}
		/* KMH, balance patch -- removed
	case AMULET_OF_POLYMORPH:
		makeknown(AMULET_OF_POLYMORPH);
		pline("You feel rather strange.");
		polyself();
		context.botl = 1;
		pline("The amulet disintegrates!");
		useup(uamul);
		break;*/
		case AMULET_OF_STRANGULATION:
			makeknown(AMULET_OF_STRANGULATION);
			pline("It constricts your throat!");
			Strangled = 6;
			break;
		case AMULET_OF_RESTFUL_SLEEP:
			if (uamul->blessed) {
				char buf[BUFSZ];
				int sleeptime;

				makeknown(AMULET_OF_RESTFUL_SLEEP);
				do {
					getlin("How many moves do you wish to sleep for? [1-500]", buf);
					sleeptime = (!*buf || *buf == '\033') ? 0 : atoi(buf);
				} while (sleeptime < 1 || sleeptime > 500);
				if (sleeptime > 0) {
					pline("You sit down and fall asleep.");
					nomul(-sleeptime);
					u.usleep = 1;
					nomovemsg = "You wake up from your refreshing nap.";
					HSleeping = rnd(100);
				}
			} else
				HSleeping = rnd(100);
			break;
		case AMULET_OF_YENDOR:
			break;
		/* KMH, balance patch -- added */
		case AMULET_VERSUS_STONE:
			uunstone();
			break;
		case AMULET_OF_FLYING:
			if (!(EFlying & ~W_AMUL) && !is_flyer(youmonst.data)) {
				pline("You feel like flying!");
				if (!Levitation)
					float_up();
				makeknown(AMULET_OF_FLYING);
			}
			break;
	}
}

void Amulet_off(void) {
	context.takeoff.mask &= ~W_AMUL;

	switch (uamul->otyp) {
		case AMULET_OF_ESP:
			/* need to update ability before calling see_monsters() */
			setworn(NULL, W_AMUL);
			see_monsters();
			return;
		case AMULET_OF_LIFE_SAVING:
		case AMULET_VERSUS_POISON:
		case AMULET_OF_DRAIN_RESISTANCE:
		/* KMH, balance patch -- removed
	case AMULET_OF_REGENERATION:
	case AMULET_OF_CONFLICT:*/
		/* KMH, balance patch -- added */
		case AMULET_VERSUS_STONE:
		case AMULET_OF_REFLECTION:
		case AMULET_OF_CHANGE:
		case AMULET_OF_UNCHANGING:
		case FAKE_AMULET_OF_YENDOR:
			break;
		case AMULET_OF_MAGICAL_BREATHING:
			if (Underwater) {
				/* HMagical_breathing must be set off
			before calling drown() */
				setworn(NULL, W_AMUL);
				if (!breathless(youmonst.data) && !amphibious(youmonst.data) && !Swimming) {
					pline("You suddenly inhale an unhealthy amount of water!");
					drown();
				}
				return;
			}
			break;
		case AMULET_OF_STRANGULATION:
			if (Strangled) {
				pline("You can breathe more easily!");
				Strangled = 0;
			}
			break;
		case AMULET_OF_RESTFUL_SLEEP:
			setworn(NULL, W_AMUL);
			if (!ESleeping)
				HSleeping = 0;
			return;
		/* KMH, balance patch -- added */
		case AMULET_OF_FLYING:
			setworn(NULL, W_AMUL);
			float_down(0L, 0L);
			return;
		case AMULET_OF_YENDOR:
			break;
	}
	setworn(NULL, W_AMUL);
	return;
}

void Ring_on(struct obj *obj) {
	long oldprop = u.uprops[objects[obj->otyp].oc_oprop].extrinsic;
	int old_attrib, which;

	if (obj == uwep) setuwep(NULL, true);
	if (obj == uswapwep) setuswapwep(NULL, true);
	if (obj == uquiver) setuqwep(NULL);

	/* only mask out W_RING when we don't have both
	   left and right rings of the same type */
	if ((oldprop & W_RING) != W_RING) oldprop &= ~W_RING;

	switch (obj->otyp) {
		case RIN_TELEPORTATION:
		case RIN_REGENERATION:
		case RIN_SEARCHING:
		case RIN_STEALTH:
		case RIN_HUNGER:
		case RIN_AGGRAVATE_MONSTER:
		case RIN_POISON_RESISTANCE:
		case RIN_FIRE_RESISTANCE:
		case RIN_COLD_RESISTANCE:
		case RIN_SHOCK_RESISTANCE:
		case RIN_CONFLICT:
		case RIN_TELEPORT_CONTROL:
		case RIN_POLYMORPH:
		case RIN_POLYMORPH_CONTROL:
		/* KMH -- added */
		case RIN_MOOD:
		case RIN_FREE_ACTION:
		case RIN_SLOW_DIGESTION:
		case RIN_SUSTAIN_ABILITY:
		case MEAT_RING:
			break;
		case RIN_SLEEPING:
			HSleeping = rnd(100);
			break;
		case RIN_WARNING:
			see_monsters();
			break;
		case RIN_SEE_INVISIBLE:
			/* can now see invisible monsters */
			set_mimic_blocking(); /* do special mimic handling */
			see_monsters();
			see_objects();

			if (Invis && !oldprop && !HSee_invisible &&
			    !perceives(youmonst.data) && !Blind) {
				newsym(u.ux, u.uy);
				pline("Suddenly you are transparent, but there!");
				makeknown(RIN_SEE_INVISIBLE);
			}
			break;
		case RIN_INVISIBILITY:
			if (!oldprop && !HInvis && !BInvis && !Blind) {
				makeknown(RIN_INVISIBILITY);
				newsym(u.ux, u.uy);
				self_invis_message();
			}
			break;
		case RIN_LEVITATION:
			if (!oldprop && !HLevitation) {
				context.botl = 1;
				float_up();
				makeknown(RIN_LEVITATION);
				spoteffects(false); /* for sinks */
			}
			break;
		case RIN_GAIN_STRENGTH:
			which = A_STR;
			goto adjust_attrib;
		case RIN_GAIN_CONSTITUTION:
			which = A_CON;
			goto adjust_attrib;
		case RIN_ADORNMENT:
			which = A_CHA;
adjust_attrib:
			old_attrib = ACURR(which);
			ABON(which) += obj->spe;
			if (ACURR(which) != old_attrib ||
			    (objects[obj->otyp].oc_name_known &&
			     old_attrib != 25 && old_attrib != 3)) {
				context.botl = 1;
				makeknown(obj->otyp);
				obj->known = 1;
				update_inventory();
			}
			break;
		case RIN_GAIN_INTELLIGENCE:
			ABON(A_INT) += obj->spe;
			context.botl = 1;
			if (obj->spe || objects[RIN_GAIN_INTELLIGENCE].oc_name_known) {
				makeknown(RIN_GAIN_INTELLIGENCE);
				obj->known = true;
			}
			break;
		case RIN_GAIN_WISDOM:
			ABON(A_WIS) += obj->spe;
			context.botl = 1;
			if (obj->spe || objects[RIN_GAIN_WISDOM].oc_name_known) {
				makeknown(RIN_GAIN_WISDOM);
				obj->known = true;
			}
			break;
		case RIN_GAIN_DEXTERITY:
			ABON(A_DEX) += obj->spe;
			context.botl = 1;
			if (obj->spe || objects[RIN_GAIN_DEXTERITY].oc_name_known) {
				makeknown(RIN_GAIN_DEXTERITY);
				obj->known = true;
			}
			break;
		case RIN_INCREASE_ACCURACY: /* KMH */
			u.uhitinc += obj->spe;
			break;
		case RIN_INCREASE_DAMAGE:
			u.udaminc += obj->spe;
			break;
		case RIN_PROTECTION_FROM_SHAPE_CHANGERS:
			rescham();
			break;
		case RIN_PROTECTION:
			if (obj->spe || objects[RIN_PROTECTION].oc_name_known) {
				context.botl = 1;
				makeknown(RIN_PROTECTION);
				obj->known = 1;
				update_inventory();
			}
			break;
	}
}

static void Ring_off_or_gone(struct obj *obj, bool gone) {
	long mask = (obj->owornmask & W_RING);
	int old_attrib, which;

	context.takeoff.mask &= ~mask;
	if (!(u.uprops[objects[obj->otyp].oc_oprop].extrinsic & mask))
		impossible("Strange... I didn't know you had that ring.");
	if (gone)
		setnotworn(obj);
	else
		setworn(NULL, obj->owornmask);

	switch (obj->otyp) {
		case RIN_TELEPORTATION:
		case RIN_REGENERATION:
		case RIN_SEARCHING:
		case RIN_STEALTH:
		case RIN_HUNGER:
		case RIN_AGGRAVATE_MONSTER:
		case RIN_POISON_RESISTANCE:
		case RIN_FIRE_RESISTANCE:
		case RIN_COLD_RESISTANCE:
		case RIN_SHOCK_RESISTANCE:
		case RIN_CONFLICT:
		case RIN_TELEPORT_CONTROL:
		case RIN_POLYMORPH:
		case RIN_POLYMORPH_CONTROL:
		/* KMH, balance patch -- now an amulet
	case RIN_DRAIN_RESISTANCE: */
		/* KMH, balance patch -- added */
		case RIN_MOOD:
		case RIN_FREE_ACTION:
		case RIN_SLOW_DIGESTION:
		case RIN_SUSTAIN_ABILITY:
		case MEAT_RING:
			break;
		case RIN_SLEEPING:
			if (!ESleeping)
				HSleeping = 0;
			break;
		case RIN_WARNING:
			see_monsters();
			break;
		case RIN_SEE_INVISIBLE:
			/* Make invisible monsters go away */
			if (!See_invisible) {
				set_mimic_blocking(); /* do special mimic handling */
				see_monsters();
				see_objects();
			}

			if (Invisible && !Blind) {
				newsym(u.ux, u.uy);
				pline("Suddenly you cannot see yourself.");
				makeknown(RIN_SEE_INVISIBLE);
			}
			break;
		case RIN_INVISIBILITY:
			if (!Invis && !BInvis && !Blind) {
				newsym(u.ux, u.uy);
				pline("Your body seems to unfade%s.",
				      See_invisible ? " completely" : "..");
				makeknown(RIN_INVISIBILITY);
			}
			break;
		case RIN_LEVITATION:
			context.botl = 1;
			float_down(0L, 0L);
			if (!Levitation) makeknown(RIN_LEVITATION);
			break;
		case RIN_GAIN_STRENGTH:
			which = A_STR;
			goto adjust_attrib;
		case RIN_GAIN_INTELLIGENCE:
			which = A_INT;
			goto adjust_attrib;
		case RIN_GAIN_WISDOM:
			which = A_WIS;
			goto adjust_attrib;
		case RIN_GAIN_DEXTERITY:
			which = A_DEX;
			goto adjust_attrib;
		case RIN_GAIN_CONSTITUTION:
			which = A_CON;
			goto adjust_attrib;
		case RIN_ADORNMENT:
			which = A_CHA;
adjust_attrib:
			old_attrib = ACURR(which);
			ABON(which) -= obj->spe;
			if (ACURR(which) != old_attrib) {
				context.botl = 1;
				makeknown(obj->otyp);
				obj->known = 1;
				update_inventory();
			}
			break;
		case RIN_INCREASE_ACCURACY: /* KMH */
			u.uhitinc -= obj->spe;
			break;
		case RIN_INCREASE_DAMAGE:
			u.udaminc -= obj->spe;
			break;
		case RIN_PROTECTION:
			// might have forgotten it due to amnesia
			if (obj->spe) {
				context.botl = 1;
				makeknown(RIN_PROTECTION);
				obj->known = 1;
				update_inventory();
			}
			break;
		case RIN_PROTECTION_FROM_SHAPE_CHANGERS:
			// If you're no longer protected, let the chameleons
			// change shape again -dgk
			restartcham();
			break;
	}
}

void Ring_off(struct obj *obj) {
	Ring_off_or_gone(obj, false);
}

void Ring_gone(struct obj *obj) {
	Ring_off_or_gone(obj, true);
}

void Blindf_on(struct obj *otmp) {
	long already_blind = Blind, changed = false;

	if (otmp == uwep)
		setuwep(NULL, true);
	if (otmp == uswapwep)
		setuswapwep(NULL, true);
	if (otmp == uquiver)
		setuqwep(NULL);
	setworn(otmp, W_TOOL);
	on_msg(otmp);

	if (Blind && !already_blind) {
		changed = true;
		if (flags.verbose) pline("You can't see any more.");
		/* set ball&chain variables before the hero goes blind */
		if (Punished) set_bc(0);
	} else if (already_blind && !Blind) {
		changed = true;
		/* "You are now wearing the Eyes of the Overworld." */
		pline("You can see!");
	}
	if (changed) {
		/* blindness has just been toggled */
		if (Blind_telepat || Infravision) see_monsters();
		vision_full_recalc = 1; /* recalc vision limits */
		context.botl = 1;
	}
}

void Blindf_off(struct obj *otmp) {
	boolean was_blind = Blind, changed = false;

	context.takeoff.mask &= ~W_TOOL;
	setworn(NULL, otmp->owornmask);
	off_msg(otmp);

	if (Blind) {
		if (was_blind) {
			/* "still cannot see" makes no sense when removing lenses
			   since they can't have been the cause of your blindness */
			if (otmp->otyp != LENSES)
				pline("You still cannot see.");
		} else {
			changed = true; /* !was_blind */
			/* "You were wearing the Eyes of the Overworld." */
			pline("You can't see anything now!");
			/* set ball&chain variables before the hero goes blind */
			if (Punished) set_bc(0);
		}
	} else if (was_blind) {
		if (!gulp_blnd_check()) {
			changed = true; /* !Blind */
			pline("You can see again.");
		}
	}
	if (changed) {
		/* blindness has just been toggled */
		if (Blind_telepat || Infravision) see_monsters();
		vision_full_recalc = 1; /* recalc vision limits */
		context.botl = 1;
	}
}

/* called in main to set intrinsics of worn start-up items */
void set_wear(void) {
	if (uarmu) Shirt_on();
	if (uarm) Armor_on();
	if (uarmc) Cloak_on();
	if (uarmf) Boots_on();
	if (uarmg) Gloves_on();
	if (uarmh) Helmet_on();
	if (uarms) Shield_on();
}

/* check whether the target object is currently being put on (or taken off) */
boolean donning(struct obj *otmp) {
	/* long what = (occupation == take_off) ? context.takeoff.what : 0L; */
	long what = context.takeoff.what; /* if nonzero, occupation is implied */
	boolean result = false;

	if (otmp == uarm)
		result = (afternmv == Armor_on || afternmv == Armor_off ||
			  what == WORN_ARMOR);
	else if (otmp == uarmu)
		result = (afternmv == Shirt_on || afternmv == Shirt_off ||
			  what == WORN_SHIRT);
	else if (otmp == uarmc)
		result = (afternmv == Cloak_on || afternmv == Cloak_off ||
			  what == WORN_CLOAK);
	else if (otmp == uarmf)
		result = (afternmv == Boots_on || afternmv == Boots_off ||
			  what == WORN_BOOTS);
	else if (otmp == uarmh)
		result = (afternmv == Helmet_on || afternmv == Helmet_off ||
			  what == WORN_HELMET);
	else if (otmp == uarmg)
		result = (afternmv == Gloves_on || afternmv == Gloves_off ||
			  what == WORN_GLOVES);
	else if (otmp == uarms)
		result = (afternmv == Shield_on || afternmv == Shield_off ||
			  what == WORN_SHIELD);

	return result;
}

void cancel_don(void) {
	/* the piece of armor we were donning/doffing has vanished, so stop
	 * wasting time on it (and don't dereference it when donning would
	 * otherwise finish)
	 */
	context.takeoff.cancelled_don = (afternmv == Boots_on || afternmv == Helmet_on || afternmv == Gloves_on || afternmv == Armor_on);
	afternmv = 0;
	nomovemsg = NULL;
	multi = 0;
	context.takeoff.delay = 0;
	context.takeoff.what = 0L;
}

static const char clothes[] = {ARMOR_CLASS, 0};
static const char accessories[] = {RING_CLASS, AMULET_CLASS, TOOL_CLASS, FOOD_CLASS, 0};

/* the 'T' command */
int dotakeoff(void) {
	struct obj *otmp = NULL;
	int armorpieces = 0;

#define MOREARM(x)             \
	if (x) {               \
		armorpieces++; \
		otmp = x;      \
	}
	MOREARM(uarmh);
	MOREARM(uarms);
	MOREARM(uarmg);
	MOREARM(uarmf);
	if (uarmc) {
		armorpieces++;
		otmp = uarmc;
	} else if (uarm) {
		armorpieces++;
		otmp = uarm;
	} else if (uarmu) {
		armorpieces++;
		otmp = uarmu;
	}
	if (!armorpieces) {
		/* assert( GRAY_DRAGON_SCALES > YELLOW_DRAGON_SCALE_MAIL ); */
		if (uskin)
			pline("The %s merged with your skin!",
			      uskin->otyp >= GRAY_DRAGON_SCALES ?
				      "dragon scales are" :
				      "dragon scale mail is");
		else
			pline("Not wearing any armor.%s", (iflags.cmdassist &&
							   (uleft || uright || uamul || ublindf)) ?
								  "  Use 'R' command to remove accessories." :
								  "");
		return 0;
	}
	if (armorpieces > 1)
		otmp = getobj(clothes, "take off");
	if (otmp == 0) return 0;
	if (!(otmp->owornmask & W_ARMOR)) {
		pline("You are not wearing that.");
		return 0;
	}
	/* note: the `uskin' case shouldn't be able to happen here; dragons
	   can't wear any armor so will end up with `armorpieces == 0' above */
	if (otmp == uskin || ((otmp == uarm) && uarmc) || ((otmp == uarmu) && (uarmc || uarm))) {
		pline("You can't take that off.");
		return 0;
	}

	reset_remarm(); /* clear context.takeoff.mask and context.takeoff.what */
	select_off(otmp);
	if (!context.takeoff.mask) return 0;
	reset_remarm(); /* armoroff() doesn't use context.takeoff.mask */

	armoroff(otmp);
	return 1;
}

/* the 'R' command */
int doremring(void) {
	struct obj *otmp = 0;
	int Accessories = 0;

#define MOREACC(x)             \
	if (x) {               \
		Accessories++; \
		otmp = x;      \
	}
	MOREACC(uleft);
	MOREACC(uright);
	MOREACC(uamul);
	MOREACC(ublindf);

	if (!Accessories) {
		pline("Not wearing any accessories.%s", (iflags.cmdassist &&
							 (uarm || uarmc || uarmu ||
							  uarms || uarmh || uarmg || uarmf)) ?
								"  Use 'T' command to take off armor." :
								"");
		return 0;
	}
	if (Accessories != 1) otmp = getobj(accessories, "remove");
	if (!otmp) return 0;
	if (!(otmp->owornmask & (W_RING | W_AMUL | W_TOOL))) {
		pline("You are not wearing that.");
		return 0;
	}

	reset_remarm(); /* clear context.takeoff.mask and context.takeoff.what */
	select_off(otmp);
	if (!context.takeoff.mask) return 0;
	reset_remarm(); /* not used by Ring_/Amulet_/Blindf_off() */

	if (otmp == uright || otmp == uleft) {
		/* Sometimes we want to give the off_msg before removing and
		 * sometimes after; for instance, "you were wearing a moonstone
		 * ring (on right hand)" is desired but "you were wearing a
		 * square amulet (being worn)" is not because of the redundant
		 * "being worn".
		 */
		off_msg(otmp);
		Ring_off(otmp);
	} else if (otmp == uamul) {
		Amulet_off();
		off_msg(otmp);
	} else if (otmp == ublindf) {
		Blindf_off(otmp); /* does its own off_msg */
	} else {
		impossible("removing strange accessory?");
	}
	return 1;
}

/* Check if something worn is cursed _and_ unremovable. */
int cursed(struct obj *otmp) {
	/* Curses, like chickens, come home to roost. */
	if ((otmp == uwep) ? welded(otmp) : (int)otmp->cursed) {
		pline("You can't.  %s cursed.",
		      (is_boots(otmp) || is_gloves(otmp) || otmp->quan > 1L) ? "They are" : "It is");
		otmp->bknown = true;
		return 1;
	}
	return 0;
}

int armoroff(struct obj *otmp) {
	int delay = -objects[otmp->otyp].oc_delay;

	if (cursed(otmp)) return 0;
	if (delay) {
		nomul(delay);
		if (is_helmet(otmp)) {
			static char nomovemsg_helm[BUFSZ];
			sprintf(nomovemsg_helm, "You finish taking off your %s.", helm_simple_name(otmp));
			nomovemsg = nomovemsg_helm;
			afternmv = Helmet_off;
		} else if (is_gloves(otmp)) {
			nomovemsg = "You finish taking off your gloves.";
			afternmv = Gloves_off;
		} else if (is_boots(otmp)) {
			nomovemsg = "You finish taking off your boots.";
			afternmv = Boots_off;
		} else {
			nomovemsg = "You finish taking off your suit.";
			afternmv = Armor_off;
		}
	} else {
		/* Be warned!  We want off_msg after removing the item to
		 * avoid "You were wearing ____ (being worn)."  However, an
		 * item which grants fire resistance might cause some trouble
		 * if removed in Hell and lifesaving puts it back on; in this
		 * case the message will be printed at the wrong time (after
		 * the messages saying you died and were lifesaved).  Luckily,
		 * no cloak, shield, or fast-removable armor grants fire
		 * resistance, so we can safely do the off_msg afterwards.
		 * Rings do grant fire resistance, but for rings we want the
		 * off_msg before removal anyway so there's no problem.  Take
		 * care in adding armors granting fire resistance; this code
		 * might need modification.
		 * 3.2 (actually 3.1 even): this comment is obsolete since
		 * fire resistance is not needed for Gehennom.
		 */
		if (is_cloak(otmp))
			Cloak_off();
		else if (is_shield(otmp))
			Shield_off();
		else if (is_helmet(otmp))
			/* [Finn E. Theodorsen] For fedoras */
			Helmet_off();
		else
			setworn(NULL, otmp->owornmask & W_ARMOR);
		off_msg(otmp);
	}
	context.takeoff.mask = context.takeoff.what = 0L;
	return 1;
}

static void already_wearing(const char *cc) {
	pline("You are already wearing %s%c", cc, (cc == c_that_) ? '!' : '.');
}

static void already_wearing2(const char *cc1, const char *cc2) {
	pline("You can't wear %s because you're wearing %s there already.", cc1, cc2);
}

/*
 * canwearobj checks to see whether the player can wear a piece of armor
 *
 * inputs: otmp (the piece of armor)
 *         noisy (if true give error messages, otherwise be quiet about it)
 * output: mask (otmp's armor type)
 */
int canwearobj(struct obj *otmp, long *mask, boolean noisy) {
	int err = 0;
	const char *which;

	which = is_cloak(otmp) ? c_cloak :
				 is_shirt(otmp) ? c_shirt :
						  is_suit(otmp) ? c_suit : 0;
	if (which && cantweararm(youmonst.data) &&
	    /* same exception for cloaks as used in m_dowear() */
	    (which != c_cloak || youmonst.data->msize != MZ_SMALL) &&
	    (racial_exception(&youmonst, otmp) < 1)) {
		if (noisy) pline("The %s will not fit on your body.", which);
		return 0;
	} else if (otmp->owornmask & W_ARMOR) {
		if (noisy) already_wearing(c_that_);
		return 0;
	}

	if (welded(uwep) && bimanual(uwep) &&
	    (is_suit(otmp) || is_shirt(otmp))) {
		if (noisy)
			pline("You cannot do that while holding your %s.",
			      is_sword(uwep) ? c_sword : c_weapon);
		return 0;
	}

	if (is_helmet(otmp)) {
		if (uarmh) {
			if (noisy) already_wearing(an(helm_simple_name(uarmh)));
			err++;
		} else if (Upolyd && has_horns(youmonst.data) && !is_flimsy(otmp)) {
			/* (flimsy exception matches polyself handling) */
			if (noisy)
				pline("The %s won't fit over your horn%s.",
				      helm_simple_name(uarmh), plur(num_horns(youmonst.data)));
			err++;
		} else
			*mask = W_ARMH;
	} else if (is_shield(otmp)) {
		if (uarms) {
			if (noisy) already_wearing(an(c_shield));
			err++;
		} else if (uwep && bimanual(uwep)) {
			if (noisy)
				pline("You cannot wear a shield while wielding a two-handed %s.",
				      is_sword(uwep) ? c_sword :
						       (uwep->otyp == BATTLE_AXE) ? c_axe : c_weapon);
			err++;
		} else if (u.twoweap) {
			if (noisy) {
				if (uwep && uswapwep)
					pline("You cannot wear a shield while wielding two weapons.");
				else
					pline("You cannot wear a shield while fighting with two %s.",
					      makeplural(body_part(HAND)));
			}
			err++;
		} else
			*mask = W_ARMS;
	} else if (is_boots(otmp)) {
		if (uarmf) {
			if (noisy) already_wearing(c_boots);
			err++;
		} else if (Upolyd && slithy(youmonst.data)) {
			if (noisy) pline("You have no feet..."); /* not body_part(FOOT) */
			err++;
		} else if (Upolyd && youmonst.data->mlet == S_CENTAUR) {
			/* break_armor() pushes boots off for centaurs,
			   so don't let dowear() put them back on... */
			if (noisy) pline("You have too many hooves to wear %s.",
					 c_boots); /* makeplural(body_part(FOOT)) yields
					   "rear hooves" which sounds odd */
			err++;
		} else if (u.utrap && (u.utraptype == TT_BEARTRAP ||
				       u.utraptype == TT_INFLOOR)) {
			if (u.utraptype == TT_BEARTRAP) {
				if (noisy) pline("Your %s is trapped!", body_part(FOOT));
			} else {
				if (noisy) pline("Your %s are stuck in the %s!",
						 makeplural(body_part(FOOT)),
						 surface(u.ux, u.uy));
			}
			err++;
		} else
			*mask = W_ARMF;
	} else if (is_gloves(otmp)) {
		if (uarmg) {
			if (noisy) already_wearing(c_gloves);
			err++;
		} else if (welded(uwep)) {
			if (noisy) pline("You cannot wear gloves over your %s.",
					 is_sword(uwep) ? c_sword : c_weapon);
			err++;
		} else
			*mask = W_ARMG;
	} else if (is_shirt(otmp)) {
		if (uarm || uarmc || uarmu) {
			if (uarmu) {
				if (noisy) already_wearing(an(c_shirt));
			} else {
				if (noisy) pline("You can't wear that over your %s.",
						 (uarm && !uarmc) ? c_armor : cloak_simple_name(uarmc));
			}
			err++;
		} else
			*mask = W_ARMU;
	} else if (is_cloak(otmp)) {
		if (uarmc) {
			if (noisy) already_wearing(an(cloak_simple_name(uarmc)));
			err++;
		} else
			*mask = W_ARMC;
	} else if (is_suit(otmp)) {
		if (uarmc) {
			if (noisy) pline("You cannot wear armor over a %s.", cloak_simple_name(uarmc));
			err++;
		} else if (uarm) {
			if (noisy) already_wearing("some armor");
			err++;
		} else
			*mask = W_ARM;
	} else {
		/* getobj can't do this after setting its allow_all flag; that
		   happens if you have armor for slots that are covered up or
		   extra armor for slots that are filled */
		if (noisy) silly_thing("wear", otmp);
		err++;
	}
	/* Unnecessary since now only weapons and special items like pick-axes get
	 * welded to your hand, not armor
	    if (welded(otmp)) {
		if (!err++) {
		    if (noisy) weldmsg(otmp);
		}
	    }
	 */
	return !err;
}

/* the 'W' command */
int dowear(void) {
	struct obj *otmp;
	int delay;
	long mask = 0;

	/* cantweararm checks for suits of armor */
	/* verysmall or nohands checks for shields, gloves, etc... */
	if ((verysmall(youmonst.data) || nohands(youmonst.data))) {
		pline("Don't even bother.");
		return 0;
	}

	otmp = getobj(clothes, "wear");
	if (!otmp) return 0;

	if (!canwearobj(otmp, &mask, true)) return 0;

	if (otmp->oartifact && !touch_artifact(otmp, &youmonst))
		return 1; /* costs a turn even though it didn't get worn */

	if (otmp->otyp == HELM_OF_OPPOSITE_ALIGNMENT &&
	    qstart_level.dnum == u.uz.dnum) { /* in quest */
		if (u.ualignbase[A_CURRENT] == u.ualignbase[A_ORIGINAL])
			pline("You narrowly avoid losing all chance at your goal.");
		else /* converted */
			pline("You are suddenly overcome with shame and change your mind.");
		u.ublessed = 0; /* lose your god's protection */
		makeknown(otmp->otyp);
		context.botl = 1;
		return 1;
	}

	otmp->known = true;
	if (otmp == uwep)
		setuwep(NULL, true);
	if (otmp == uswapwep)
		setuswapwep(NULL, true);
	if (otmp == uquiver)
		setuqwep(NULL);
	setworn(otmp, mask);
	delay = -objects[otmp->otyp].oc_delay;
	if (delay) {
		nomul(delay);
		if (is_boots(otmp)) afternmv = Boots_on;
		if (is_helmet(otmp)) afternmv = Helmet_on;
		if (is_gloves(otmp)) afternmv = Gloves_on;
		if (otmp == uarm) afternmv = Armor_on;
		nomovemsg = "You finish your dressing maneuver.";
	} else {
		if (is_cloak(otmp)) Cloak_on();
		if (is_shield(otmp)) Shield_on();
		if (is_shirt(otmp)) Shirt_on();
		if (is_helmet(otmp)) Helmet_on(); /* fedoras */
		on_msg(otmp);
	}
	context.takeoff.mask = context.takeoff.what = 0L;
	return 1;
}

int doputon(void) {
	struct obj *otmp;
	long mask = 0L;

	if (uleft && uright && uamul && ublindf) {
		pline("Your %s%s are full, and you're already wearing an amulet and %s.",
		      humanoid(youmonst.data) ? "ring-" : "",
		      makeplural(body_part(FINGER)),
		      ublindf->otyp == LENSES ? "some lenses" : "a blindfold");
		return 0;
	}
	otmp = getobj(accessories, "put on");
	if (!otmp) return 0;
	if (otmp->owornmask & (W_RING | W_AMUL | W_TOOL)) {
		already_wearing(c_that_);
		return 0;
	}
	if (welded(otmp)) {
		weldmsg(otmp);
		return 0;
	}
	if (otmp == uwep)
		setuwep(NULL, true);
	if (otmp == uswapwep)
		setuswapwep(NULL, true);
	if (otmp == uquiver)
		setuqwep(NULL);
	if (otmp->oclass == RING_CLASS || otmp->otyp == MEAT_RING) {
		if (nolimbs(youmonst.data)) {
			pline("You cannot make the ring stick to your body.");
			return 0;
		}
		if (uleft && uright) {
			pline("There are no more %s%s to fill.",
			      humanoid(youmonst.data) ? "ring-" : "",
			      makeplural(body_part(FINGER)));
			return 0;
		}
		if (uleft)
			mask = RIGHT_RING;
		else if (uright)
			mask = LEFT_RING;
		else
			do {
				char qbuf[QBUFSZ];
				char answer;

				sprintf(qbuf, "Which %s%s, Right or Left?",
					humanoid(youmonst.data) ? "ring-" : "",
					body_part(FINGER));
				if (!(answer = yn_function(qbuf, "rl", '\0')))
					return 0;
				switch (answer) {
					case 'l':
					case 'L':
						mask = LEFT_RING;
						break;
					case 'r':
					case 'R':
						mask = RIGHT_RING;
						break;
				}
			} while (!mask);
		if (uarmg && uarmg->cursed) {
			uarmg->bknown = true;
			pline("You cannot remove your gloves to put on the ring.");
			return 0;
		}
		if (welded(uwep) && bimanual(uwep)) {
			/* welded will set bknown */
			pline("You cannot free your weapon hands to put on the ring.");
			return 0;
		}
		if (welded(uwep) && mask == RIGHT_RING) {
			/* welded will set bknown */
			pline("You cannot free your weapon hand to put on the ring.");
			return 0;
		}
		if (otmp->oartifact && !touch_artifact(otmp, &youmonst))
			return 1; /* costs a turn even though it didn't get worn */
		setworn(otmp, mask);
		Ring_on(otmp);
	} else if (otmp->oclass == AMULET_CLASS) {
		if (uamul) {
			already_wearing("an amulet");
			return 0;
		}
		if (otmp->oartifact && !touch_artifact(otmp, &youmonst))
			return 1;
		setworn(otmp, W_AMUL);
		if (otmp->otyp == AMULET_OF_CHANGE) {
			Amulet_on();
			/* Don't do a prinv() since the amulet is now gone */
			return 1;
		}
		Amulet_on();
	} else { /* it's a blindfold, towel, or lenses */
		if (ublindf) {
			if (ublindf->otyp == TOWEL)
				pline("Your %s is already covered by a towel.",
				      body_part(FACE));
			else if (ublindf->otyp == BLINDFOLD) {
				if (otmp->otyp == LENSES)
					already_wearing2("lenses", "a blindfold");
				else
					already_wearing("a blindfold");
			} else if (ublindf->otyp == LENSES) {
				if (otmp->otyp == BLINDFOLD)
					already_wearing2("a blindfold", "some lenses");
				else
					already_wearing("some lenses");
			} else
				already_wearing("something"); /* ??? */
			return 0;
		}
		if (otmp->otyp != BLINDFOLD && otmp->otyp != TOWEL && otmp->otyp != LENSES) {
			pline("You can't wear that!");
			return 0;
		}
		if (otmp->oartifact && !touch_artifact(otmp, &youmonst))
			return 1;
		Blindf_on(otmp);
		return 1;
	}
	if (is_worn(otmp))
		prinv(NULL, otmp, 0L);
	return 1;
}

/* Limits of uac (conveniently equal to the limits of an schar ;) */
#define UAC_MIN (-128)
#define UAC_LIM 127

void find_ac(void) {
	int uac = mons[u.umonnum].ac;

	if (uarm) uac -= ARM_BONUS(uarm);
	if (uarmc) uac -= ARM_BONUS(uarmc);
	if (uarmh) uac -= ARM_BONUS(uarmh);
	if (uarmf) uac -= ARM_BONUS(uarmf);
	if (uarms) uac -= ARM_BONUS(uarms);
	if (uarmg) uac -= ARM_BONUS(uarmg);
	if (uarmu) uac -= ARM_BONUS(uarmu);
	if (uleft && uleft->otyp == RIN_PROTECTION) uac -= uleft->spe;
	if (uright && uright->otyp == RIN_PROTECTION) uac -= uright->spe;
	if (HProtection & INTRINSIC) uac -= u.ublessed;
	uac -= u.uspellprot;

	/* STEPHEN WHITE'S NEW CODE */
	/* Dexterity now affects AC */
	if (ACURR(A_DEX) < 4)
		uac += 3;
	else if (ACURR(A_DEX) < 6)
		uac += 2;
	else if (ACURR(A_DEX) < 8)
		uac += 1;
	else if (ACURR(A_DEX) < 14)
		uac -= 0;
	else if (ACURR(A_DEX) < 21)
		uac -= ACURR(A_DEX) - 14;
	else if (ACURR(A_DEX) < 22)
		uac -= 6;
	else if (ACURR(A_DEX) < 24)
		uac -= 7;
	else
		uac -= 8;

	if (Role_if(PM_MONK) && !uwep && (!uarm || uarm->otyp == ROBE || uarm->otyp == ROBE_OF_POWER || uarm->otyp == ROBE_OF_WEAKNESS || uarm->otyp == ROBE_OF_PROTECTION) && !uarms) {
		/*WAC cap off the Monk's ac bonus to -11 */
		if (u.ulevel > 18)
			uac -= 11;
		else
			uac -= (u.ulevel / 2) + 2;
	}
	if (Race_if(PM_DOPPELGANGER) && !uarm) uac -= (u.ulevel / 4) + 1;
	if (Race_if(PM_HUMAN_WEREWOLF) && !uarm) uac -= (u.ulevel / 4) + 1;

	/* Harlow - make sure it doesn't wrap around ;) */
	uac = (uac < UAC_MIN ? UAC_MIN : (uac > UAC_LIM ? UAC_LIM : uac));

	if (uac != u.uac) {
		u.uac = uac;
		context.botl = 1;
	}
}

void glibr(void) {
	struct obj *otmp;
	int xfl = 0;
	bool leftfall, rightfall, wastwoweap = false;
	const char *otherwep = NULL, *thiswep, *which, *hand;

	leftfall = (uleft && !uleft->cursed &&
		    (!uwep || !welded(uwep) || !bimanual(uwep)));
	rightfall = (uright && !uright->cursed && (!welded(uwep)));
	if (!uarmg && (leftfall || rightfall) && !nolimbs(youmonst.data)) {
		/* changed so cursed rings don't fall off, GAN 10/30/86 */
		pline("Your %s off your %s.",
		      (leftfall && rightfall) ? "rings slip" : "ring slips",
		      (leftfall && rightfall) ? makeplural(body_part(FINGER)) :
						body_part(FINGER));
		xfl++;
		if (leftfall) {
			otmp = uleft;
			Ring_off(uleft);
			dropx(otmp);
		}
		if (rightfall) {
			otmp = uright;
			Ring_off(uright);
			dropx(otmp);
		}
	}

	otmp = uswapwep;
	if (u.twoweap && otmp) {
		/* secondary weapon doesn't need nearly as much handling as
                 * primary; when in two-weapon mode, we know it's one-handed
                 * with something else in the other hand and also that it's
                 * a weapon or weptool rather than something unusual, plus
                 * we don't need to compare its type with the primary */
		otherwep = is_sword(otmp) ? c_sword : weapon_descr(otmp);
		if (otmp->quan > 1L) otherwep = makeplural(otherwep);
		hand = body_part(HAND);
		which = "left ";
		pline("Your %s %s%s from your %s%s.",
				otherwep, xfl ? "also " : "",
				otense(otmp, "slip"), which, hand);

		xfl++;
		wastwoweap = true;
		setuswapwep(NULL, false); // clears u.twoweap
		if (otmp->otyp != LOADSTONE || !otmp->cursed)
			dropx(otmp);
	}
	otmp = uwep;
	if (otmp && !welded(otmp)) {
		long savequan = otmp->quan;

		// nice wording if both weapons are the same type
		thiswep = is_sword(otmp) ? c_sword : weapon_descr(otmp);
		if (otherwep && strcmp(thiswep, makesingular(otherwep)))
			otherwep = 0;
		if (otmp->quan > 1L) {
			/* most class names for unconventional wielded items
			 * are ok, but if wielding multiple apples or rations
			 * we don't want "your foods slip", so force non-corpse
			 * food to be singular; skipping makeplural() isn't
			 * enough--we need to fool otense() too */
			if (!strcmp(thiswep, "food")) otmp->quan = 1L;
			else thiswep = makeplural(thiswep);
		}
		hand = body_part(HAND);
		which = "";
		if (bimanual(otmp))
			hand = makeplural(hand);
		else if (wastwoweap)
			which = "right ";   /* preceding msg was about left */
		pline("%s %s%s %s%s from your %s%s.",
				!strncmp(thiswep, "corpse", 6) ? "The" : "Your",
				otherwep ? "other " : "", thiswep, xfl ? "also " : "",
				otense(otmp, "slip"), which, hand);
		/* xfl++; */
		otmp->quan = savequan;

		setuwep(NULL, false);
		if (otmp->otyp != LOADSTONE || !otmp->cursed)
			dropx(otmp);
	}
}

struct obj *some_armor(struct monst *victim) {
	struct obj *otmph, *otmp;

	otmph = (victim == &youmonst) ? uarmc : which_armor(victim, W_ARMC);
	if (!otmph)
		otmph = (victim == &youmonst) ? uarm : which_armor(victim, W_ARM);
	if (!otmph)
		otmph = (victim == &youmonst) ? uarmu : which_armor(victim, W_ARMU);

	otmp = (victim == &youmonst) ? uarmh : which_armor(victim, W_ARMH);
	if (otmp && (!otmph || !rn2(4))) otmph = otmp;
	otmp = (victim == &youmonst) ? uarmg : which_armor(victim, W_ARMG);
	if (otmp && (!otmph || !rn2(4))) otmph = otmp;
	otmp = (victim == &youmonst) ? uarmf : which_armor(victim, W_ARMF);
	if (otmp && (!otmph || !rn2(4))) otmph = otmp;
	otmp = (victim == &youmonst) ? uarms : which_armor(victim, W_ARMS);
	if (otmp && (!otmph || !rn2(4))) otmph = otmp;
	return otmph;
}

/* erode some arbitrary armor worn by the victim */
void erode_armor(struct monst *victim, boolean acid_dmg) {
	struct obj *otmph = some_armor(victim);

	if (otmph && (otmph != uarmf)) {
		erode_obj(otmph, acid_dmg, false, false);
		if (carried(otmph)) update_inventory();
	}
}

/* used for praying to check and fix levitation trouble */
struct obj *stuck_ring(struct obj *ring, int otyp) {
	if (ring != uleft && ring != uright) {
		impossible("stuck_ring: neither left nor right?");
		return NULL;
	}

	if (ring && ring->otyp == otyp) {
		/* reasons ring can't be removed match those checked by select_off();
		   limbless case has extra checks because ordinarily it's temporary */
		if (nolimbs(youmonst.data) &&
		    uamul && uamul->otyp == AMULET_OF_UNCHANGING && uamul->cursed)
			return uamul;
		if (welded(uwep) && (ring == uright || bimanual(uwep))) return uwep;
		if (uarmg && uarmg->cursed) return uarmg;
		if (ring->cursed) return ring;
	}
	/* either no ring or not right type or nothing prevents its removal */
	return NULL;
}

/* also for praying; find worn item that confers "Unchanging" attribute */
struct obj *unchanger(void) {
	if (uamul && uamul->otyp == AMULET_OF_UNCHANGING) return uamul;
	return 0;
}

/* occupation callback for 'A' */
static int select_off(struct obj *otmp) {
	struct obj *why;
	char buf[BUFSZ];

	if (!otmp) return 0;
	*buf = '\0'; /* lint suppresion */

	/* special ring checks */
	if (otmp == uright || otmp == uleft) {
		if (nolimbs(youmonst.data)) {
			pline("The ring is stuck.");
			return 0;
		}
		why = 0; /* the item which prevents ring removal */
		if (welded(uwep) && (otmp == uright || bimanual(uwep))) {
			sprintf(buf, "free a weapon %s", body_part(HAND));
			why = uwep;
		} else if (uarmg && uarmg->cursed) {
			sprintf(buf, "take off your %s", c_gloves);
			why = uarmg;
		}
		if (why) {
			pline("You cannot %s to remove the ring.", buf);
			why->bknown = true;
			return 0;
		}
	}
	/* special glove checks */
	if (otmp == uarmg) {
		if (welded(uwep)) {
			pline("You are unable to take off your %s while wielding that %s.",
			      c_gloves, is_sword(uwep) ? c_sword : c_weapon);
			uwep->bknown = true;
			return 0;
		} else if (Glib) {
			pline("You can't take off the slippery %s with your slippery %s.",
			      c_gloves, makeplural(body_part(FINGER)));
			return 0;
		}
	}
	/* special boot checks */
	if (otmp == uarmf) {
		if (u.utrap && u.utraptype == TT_BEARTRAP) {
			pline("The bear trap prevents you from pulling your %s out.",
			      body_part(FOOT));
			return 0;
		} else if (u.utrap && u.utraptype == TT_INFLOOR) {
			pline("You are stuck in the %s, and cannot pull your %s out.",
			      surface(u.ux, u.uy), makeplural(body_part(FOOT)));
			return 0;
		}
	}
	/* special suit and shirt checks */
	if (otmp == uarm || otmp == uarmu) {
		why = 0; /* the item which prevents context.takeoff.disrobing */
		if (uarmc && uarmc->cursed) {
			sprintf(buf, "remove your %s", cloak_simple_name(uarmc));
			why = uarmc;
		} else if (otmp == uarmu && uarm && uarm->cursed) {
			sprintf(buf, "remove your %s", c_suit);
			why = uarm;
		} else if (welded(uwep) && bimanual(uwep)) {
			sprintf(buf, "release your %s",
				is_sword(uwep) ? c_sword :
						 (uwep->otyp == BATTLE_AXE) ? c_axe : c_weapon);
			why = uwep;
		}
		if (why) {
			pline("You cannot %s to take off %s.", buf, the(xname(otmp)));
			why->bknown = true;
			return 0;
		}
	}
	/* basic curse check */
	if (otmp == uquiver || (otmp == uswapwep && !u.twoweap)) {
		; /* some items can be removed even when cursed */
	} else {
		/* otherwise, this is fundamental */
		if (cursed(otmp)) return 0;
	}

	if (otmp == uarm)
		context.takeoff.mask |= WORN_ARMOR;
	else if (otmp == uarmc)
		context.takeoff.mask |= WORN_CLOAK;
	else if (otmp == uarmf)
		context.takeoff.mask |= WORN_BOOTS;
	else if (otmp == uarmg)
		context.takeoff.mask |= WORN_GLOVES;
	else if (otmp == uarmh)
		context.takeoff.mask |= WORN_HELMET;
	else if (otmp == uarms)
		context.takeoff.mask |= WORN_SHIELD;
	else if (otmp == uarmu)
		context.takeoff.mask |= WORN_SHIRT;
	else if (otmp == uleft)
		context.takeoff.mask |= LEFT_RING;
	else if (otmp == uright)
		context.takeoff.mask |= RIGHT_RING;
	else if (otmp == uamul)
		context.takeoff.mask |= WORN_AMUL;
	else if (otmp == ublindf)
		context.takeoff.mask |= WORN_BLINDF;
	else if (otmp == uwep)
		context.takeoff.mask |= W_WEP;
	else if (otmp == uswapwep)
		context.takeoff.mask |= W_SWAPWEP;
	else if (otmp == uquiver)
		context.takeoff.mask |= W_QUIVER;

	else
		impossible("select_off: %s???", doname(otmp));

	return 0;
}

static struct obj *do_takeoff() {
	struct obj *otmp = NULL;

	if (context.takeoff.what == W_WEP) {
		if (!cursed(uwep)) {
			setuwep(NULL, true);
			pline("You are empty %s.", body_part(HANDED));
			u.twoweap = false;
		}
	} else if (context.takeoff.what == W_SWAPWEP) {
		setuswapwep(NULL, true);
		pline("You no longer have a second weapon readied.");
		u.twoweap = false;
	} else if (context.takeoff.what == W_QUIVER) {
		setuqwep(NULL);
		pline("You no longer have ammunition readied.");
	} else if (context.takeoff.what == WORN_ARMOR) {
		otmp = uarm;
		if (!cursed(otmp)) Armor_off();
	} else if (context.takeoff.what == WORN_CLOAK) {
		otmp = uarmc;
		if (!cursed(otmp)) Cloak_off();
	} else if (context.takeoff.what == WORN_BOOTS) {
		otmp = uarmf;
		if (!cursed(otmp)) Boots_off();
	} else if (context.takeoff.what == WORN_GLOVES) {
		otmp = uarmg;
		if (!cursed(otmp)) Gloves_off();
	} else if (context.takeoff.what == WORN_HELMET) {
		otmp = uarmh;
		if (!cursed(otmp)) Helmet_off();
	} else if (context.takeoff.what == WORN_SHIELD) {
		otmp = uarms;
		if (!cursed(otmp)) Shield_off();
	} else if (context.takeoff.what == WORN_SHIRT) {
		otmp = uarmu;
		if (!cursed(otmp)) Shirt_off();
	} else if (context.takeoff.what == WORN_AMUL) {
		otmp = uamul;
		if (!cursed(otmp)) Amulet_off();
	} else if (context.takeoff.what == LEFT_RING) {
		otmp = uleft;
		if (!cursed(otmp)) Ring_off(uleft);
	} else if (context.takeoff.what == RIGHT_RING) {
		otmp = uright;
		if (!cursed(otmp)) Ring_off(uright);
	} else if (context.takeoff.what == WORN_BLINDF) {
		if (!cursed(ublindf)) Blindf_off(ublindf);
	} else
		impossible("do_takeoff: taking off %lx", context.takeoff.what);

	return otmp;
}

static int take_off(void) {
	int i;
	struct obj *otmp;

	if (context.takeoff.what) {
		if (context.takeoff.delay > 0) {
			context.takeoff.delay--;
			return 1; /* still busy */
		} else {
			if ((otmp = do_takeoff())) off_msg(otmp);
		}
		context.takeoff.mask &= ~context.takeoff.what;
		context.takeoff.what = 0L;
	}

	for (i = 0; takeoff_order[i]; i++)
		if (context.takeoff.mask & takeoff_order[i]) {
			context.takeoff.what = takeoff_order[i];
			break;
		}

	otmp = NULL;
	context.takeoff.delay = 0;

	if (context.takeoff.what == 0L) {
		pline("You finish %s.", context.takeoff.disrobing);
		return 0;
	} else if (context.takeoff.what == W_WEP) {
		context.takeoff.delay = 1;
	} else if (context.takeoff.what == W_SWAPWEP) {
		context.takeoff.delay = 1;
	} else if (context.takeoff.what == W_QUIVER) {
		context.takeoff.delay = 1;
	} else if (context.takeoff.what == WORN_ARMOR) {
		otmp = uarm;
		/* If a cloak is being worn, add the time to take it off and put
		 * it back on again.  Kludge alert! since that time is 0 for all
		 * known cloaks, add 1 so that it actually matters...
		 */
		if (uarmc) context.takeoff.delay += 2 * objects[uarmc->otyp].oc_delay + 1;
	} else if (context.takeoff.what == WORN_CLOAK) {
		otmp = uarmc;
	} else if (context.takeoff.what == WORN_BOOTS) {
		otmp = uarmf;
	} else if (context.takeoff.what == WORN_GLOVES) {
		otmp = uarmg;
	} else if (context.takeoff.what == WORN_HELMET) {
		otmp = uarmh;
	} else if (context.takeoff.what == WORN_SHIELD) {
		otmp = uarms;
	} else if (context.takeoff.what == WORN_SHIRT) {
		otmp = uarmu;
		/* add the time to take off and put back on armor and/or cloak */
		if (uarm) context.takeoff.delay += 2 * objects[uarm->otyp].oc_delay;
		if (uarmc) context.takeoff.delay += 2 * objects[uarmc->otyp].oc_delay + 1;
	} else if (context.takeoff.what == WORN_AMUL) {
		context.takeoff.delay = 1;
	} else if (context.takeoff.what == LEFT_RING) {
		context.takeoff.delay = 1;
	} else if (context.takeoff.what == RIGHT_RING) {
		context.takeoff.delay = 1;
	} else if (context.takeoff.what == WORN_BLINDF) {
		context.takeoff.delay = 2;
	} else {
		impossible("take_off: taking off %lx", context.takeoff.what);
		return 0; /* force done */
	}

	if (otmp) context.takeoff.delay += objects[otmp->otyp].oc_delay;

	/* Since setting the occupation now starts the counter next move, that
	 * would always produce a delay 1 too big per item unless we subtract
	 * 1 here to account for it.
	 */
	if (context.takeoff.delay > 0) context.takeoff.delay--;

	set_occupation(take_off, context.takeoff.disrobing[0] ? context.takeoff.disrobing : NULL, 0);
	return 1; /* get busy */
}

/* clear saved context to avoid inappropriate resumption of interrupted 'A' */
void reset_remarm(void) {
	context.takeoff.what = context.takeoff.mask = 0L;
	context.takeoff.disrobing[0] = 0;
}

/* the 'A' command -- remove multiple worn items */
int doddoremarm(void) {
	int result = 0;

	if (context.takeoff.what || context.takeoff.mask) {
		pline("You continue %s.", context.takeoff.disrobing);
		set_occupation(take_off, context.takeoff.disrobing, 0);
		return 0;
	} else if (!uwep && !uswapwep && !uquiver && !uamul && !ublindf &&
		   !uleft && !uright && !wearing_armor()) {
		pline("You are not wearing anything.");
		return 0;
	}

	add_valid_menu_class(0); /* reset */
	if (flags.menu_style != MENU_TRADITIONAL ||
	    (result = ggetobj("take off", select_off, 0, false, NULL)) < -1)
		result = menu_remarm(result);

	if (context.takeoff.mask) {
		/* default activity for armor and/or accessories,
		   possibly combined with weapons */
		strncpy(context.takeoff.disrobing, "disrobing", CONTEXTVERBSZ);
		/* specific activity when handling weapons only */
		if (!(context.takeoff.mask & ~(W_WEP | W_SWAPWEP | W_QUIVER)))
			strncpy(context.takeoff.disrobing, "disarming", CONTEXTVERBSZ);
		take_off();
	}
	/* The time to perform the command is already completely accounted for
	 * in take_off(); if we return 1, that would add an extra turn to each
	 * disrobe.
	 */
	return 0;
}

static int menu_remarm(int retry) {
	int n, i = 0;
	menu_item *pick_list;
	boolean all_worn_categories = true;

	if (retry) {
		all_worn_categories = (retry == -2);
	} else if (flags.menu_style == MENU_FULL) {
		all_worn_categories = false;
		n = query_category("What type of things do you want to take off?",
				   invent, WORN_TYPES | ALL_TYPES, &pick_list, PICK_ANY);
		if (!n) return 0;
		for (i = 0; i < n; i++) {
			if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
				all_worn_categories = true;
			else
				add_valid_menu_class(pick_list[i].item.a_int);
		}
		free(pick_list);
	} else if (flags.menu_style == MENU_COMBINATION) {
		all_worn_categories = false;
		if (ggetobj("take off", select_off, 0, true, NULL) == -2)
			all_worn_categories = true;
	}

	n = query_objlist("What do you want to take off?", invent,
			  SIGNAL_NOMENU | USE_INVLET | INVORDER_SORT,
			  &pick_list, PICK_ANY,
			  all_worn_categories ? is_worn : is_worn_by_type);
	if (n > 0) {
		for (i = 0; i < n; i++)
			select_off(pick_list[i].item.a_obj);
		free(pick_list);
	} else if (n < 0 && flags.menu_style != MENU_COMBINATION) {
		pline("There is nothing else you can remove or unwield.");
	}
	return 0;
}

/* hit by destroy armor scroll/black dragon breath/monster spell */
int destroy_arm(struct obj *atmp) {
	struct obj *otmp;
#define DESTROY_ARM(o) ((otmp = (o)) != 0 && \
                       (!atmp || atmp == otmp) && \
                       (!obj_resists(otmp, 0, 90)) ? \
                       (otmp->in_use = true) : false)

	if (DESTROY_ARM(uarmc)) {
		if (donning(otmp)) cancel_don();
		pline("Your %s crumbles and turns to dust!",
		      cloak_simple_name(uarmc));
		Cloak_off();
		useup(otmp);
	} else if (DESTROY_ARM(uarm)) {
		if (donning(otmp)) cancel_don();
		pline("Your armor turns to dust and falls to the %s!",
		      surface(u.ux, u.uy));
		Armor_gone();
		useup(otmp);
	} else if (DESTROY_ARM(uarmu)) {
		if (donning(otmp)) cancel_don();
		pline("Your shirt crumbles into tiny threads and falls apart!");
		Shirt_off();
		useup(otmp);
	} else if (DESTROY_ARM(uarmh)) {
		if (donning(otmp)) cancel_don();
		pline("Your %s turns to dust and is blown away!", helm_simple_name(uarmh));
		Helmet_off();
		useup(otmp);
	} else if (DESTROY_ARM(uarmg)) {
		if (donning(otmp)) cancel_don();
		pline("Your gloves vanish!");
		Gloves_off();
		useup(otmp);
		selftouch("You");
	} else if (DESTROY_ARM(uarmf)) {
		if (donning(otmp)) cancel_don();
		pline("Your boots disintegrate!");
		Boots_off();
		useup(otmp);
	} else if (DESTROY_ARM(uarms)) {
		if (donning(otmp)) cancel_don();
		pline("Your shield crumbles away!");
		Shield_off();
		useup(otmp);
	} else {
		return 0; /* could not destroy anything */
	}

#undef DESTROY_ARM
	stop_occupation();
	return 1;
}

void adj_abon(struct obj *otmp, schar delta) {
	if (uarmg && uarmg == otmp && otmp->otyp == GAUNTLETS_OF_DEXTERITY) {
		if (delta) {
			makeknown(uarmg->otyp);
			ABON(A_DEX) += (delta);
		}
		context.botl = 1;
	}
	if (uarmh && uarmh == otmp && otmp->otyp == HELM_OF_BRILLIANCE) {
		if (delta) {
			makeknown(uarmh->otyp);
			ABON(A_INT) += (delta);
			ABON(A_WIS) += (delta);
		}
		context.botl = 1;
	}
}
/*do_wear.c*/
