/*	SCCS Id: @(#)do.c	3.4	2003/12/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Contains code for 'd', 'D' (drop), '>', '<' (up, down) */

#include "hack.h"
#include "lev.h"

static void trycall(struct obj *);
static void dosinkring(struct obj *);

static int drop(struct obj *);
static int wipeoff(void);

static int menu_drop(int);
static int currentlevel_rewrite(void);
static void final_level(void);
/* static boolean badspot(xchar,xchar); */

static const char drop_types[] =
	{ALLOW_COUNT, COIN_CLASS, ALL_CLASSES, 0};

/* 'd' command: drop one inventory item */
int dodrop(void) {
	int result, i = (invent) ? 0 : (SIZE(drop_types) - 1);

	if (*u.ushops) sellobj_state(SELL_DELIBERATE);
	result = drop(getobj(&drop_types[i], "drop"));
	if (*u.ushops) sellobj_state(SELL_NORMAL);
	reset_occupations();

	return result;
}

/* Called when a boulder is dropped, thrown, or pushed.  If it ends up
 * in a pool, it either fills the pool up or sinks away.  In either case,
 * it's gone for good...  If the destination is not a pool, returns false.
 */
boolean boulder_hits_pool(struct obj *otmp, int rx, int ry, boolean pushing) {
	if (!otmp || otmp->otyp != BOULDER)
		impossible("Not a boulder?");
	else if (!Is_waterlevel(&u.uz) && (is_pool(rx, ry) || is_lava(rx, ry))) {
		boolean lava = is_lava(rx, ry), fills_up;
		const char *what = waterbody_name(rx, ry);
		schar ltyp = levl[rx][ry].typ;
		int chance = rn2(10); /* water: 90%; lava: 10% */
		fills_up = lava ? chance == 0 : chance != 0;

		if (fills_up) {
			struct trap *ttmp = t_at(rx, ry);

			if (ltyp == DRAWBRIDGE_UP) {
				levl[rx][ry].drawbridgemask &= ~DB_UNDER; /* clear lava */
				levl[rx][ry].drawbridgemask |= DB_FLOOR;
			} else
				levl[rx][ry].typ = ROOM;

			if (ttmp) delfloortrap(ttmp);
			bury_objs(rx, ry);

			newsym(rx, ry);
			if (pushing) {
				char *who;
				if (u.usteed)
					who = y_monnam(u.usteed);
				else
					who = "you";

				pline("%s %s %s into the %s.", upstart(who), vtense(who, "push"), the(xname(otmp)), what);
				if (flags.verbose && !Blind)
					pline("Now you can cross it!");
				/* no splashing in this case */
			}
		}
		if (!fills_up || !pushing) { /* splashing occurs */
			if (!u.uinwater) {
				if (pushing ? !Blind : cansee(rx, ry)) {
					pline("There is a large splash as %s %s the %s.",
					      the(xname(otmp)), fills_up ? "fills" : "falls into",
					      what);
				} else {
					You_hearf("a%s splash.", lava ? " sizzling" : "");
				}
				wake_nearto(rx, ry, 40);
			}

			if (fills_up && u.uinwater && distu(rx, ry) == 0) {
				u.uinwater = 0;
				docrt();
				vision_full_recalc = 1;
				pline("You find yourself on dry land again!");
			} else if (lava && distu(rx, ry) <= 2) {
				pline("You are hit by molten lava%c",
				      Fire_resistance ? '.' : '!');
				burn_away_slime();
				if (Slimed) {
					pline("The slime is burned off!");
					Slimed = 0;
				}
				losehp(Maybe_Half_Phys(d((Fire_resistance ? 1 : 3), 6)), "molten lava", KILLED_BY);
			} else if (!fills_up && flags.verbose &&
				   (pushing ? !Blind : cansee(rx, ry)))
				pline("It sinks without a trace!");
		}

		/* boulder is now gone */
		if (pushing)
			delobj(otmp);
		else
			obfree(otmp, NULL);
		return true;
	}
	return false;
}

/* Used for objects which sometimes do special things when dropped; must be
 * called with the object not in any chain.  Returns true if the object goes
 * away.
 */
boolean flooreffects(struct obj *obj, int x, int y, const char *verb) {
	struct trap *t;
	struct monst *mtmp;

	if (obj->where != OBJ_FREE)
		panic("flooreffects: obj not free");

	/* make sure things like water_damage() have no pointers to follow */
	obj->nobj = obj->nexthere = NULL;

	if (obj->otyp == BOULDER && boulder_hits_pool(obj, x, y, false))
		return true;
	else if (obj->otyp == BOULDER && (t = t_at(x, y)) != 0 &&
		 (is_pitlike(t->ttyp) || is_holelike(t->ttyp))) {
		if (((mtmp = m_at(x, y)) && mtmp->mtrapped) ||
		    (u.utrap && u.ux == x && u.uy == y)) {
			if (*verb)
				pline("The boulder %s into the pit%s.",
				      vtense(NULL, verb),
				      (mtmp) ? "" : " with you");
			if (mtmp) {
				if (!passes_walls(mtmp->data) &&
				    !throws_rocks(mtmp->data)) {
					if (hmon(mtmp, obj, 3) && !is_whirly(mtmp->data))
						return false; /* still alive */
				}
				mtmp->mtrapped = 0;
			} else {
				if (!Passes_walls && !throws_rocks(youmonst.data)) {
					losehp(Maybe_Half_Phys(rnd(15)), "squished under a boulder", NO_KILLER_PREFIX);
					return false; /* player remains trapped */
				} else
					u.utrap = 0;
			}
		}
		if (*verb) {
			if (Blind && (x == u.ux) && (y == u.uy)) {
				You_hear("a CRASH! beneath you.");
			} else if (cansee(x, y)) {
				pline("The boulder %s%s.",
				      t->tseen ? "" : "triggers and ",
				      t->ttyp == TRAPDOOR ? "plugs a trap door" :
				      t->ttyp == HOLE ? "plugs a hole" :
				      "fills a pit");
			} else {
				You_hearf("a boulder %s.", verb);
			}
		}
		deltrap(t);
		obfree(obj, NULL);
		bury_objs(x, y);
		newsym(x, y);
		return true;
	} else if (is_lava(x, y)) {
		return fire_damage(obj, false, false, x, y);
	} else if (is_pool(x, y)) {
		/* Reasonably bulky objects (arbitrary) splash when dropped.
		 * If you're floating above the water even small things make noise.
		 * Stuff dropped near fountains always misses */
		if ((Blind || (Levitation || Flying)) && !Deaf &&
		    ((x == u.ux) && (y == u.uy))) {
			if (!Underwater) {
				if (weight(obj) > 9) {
					pline("Splash!");
				} else if (Levitation || Flying) {
					pline("Plop!");
				}
			}
			map_background(x, y, 0);
			newsym(x, y);
		}
		return water_damage(obj, false, false);
	} else if ((x == u.ux && y == u.uy) && uteetering_at_seen_trap()) {
		/* you escaped a pit and are standing on the precipice */
		if (Blind)
			You_hearf("%s tumble downwards.", the(xname(obj)));
		else
			pline("%s %s into %s pit.",
			      The(xname(obj)), otense(obj, "tumble"),
			      t_at(x, y)->madeby_u ? "your" : "the");
	}
	if (is_lightsaber(obj) && obj->lamplit) {
		if (cansee(x, y)) pline("You see %s deactivate.", an(xname(obj)));
		lightsaber_deactivate(obj, true);
	}

	return false;
}

// obj is an object dropped on an altar
void doaltarobj(struct obj *obj) {
	if (Blind)
		return;

	/* KMH, conduct */
	u.uconduct.gnostic++;

	if ((obj->blessed || obj->cursed) && obj->oclass != COIN_CLASS) {
		pline("There is %s flash as %s %s the altar.",
		      an(hcolor(obj->blessed ? NH_AMBER : NH_BLACK)),
		      doname(obj), otense(obj, "hit"));
		if (!Hallucination) obj->bknown = 1;
	} else {
		pline("%s %s on the altar.", Doname2(obj),
		      otense(obj, "land"));
		obj->bknown = 1;
	}
}

static void trycall(struct obj *obj) {
	if (!objects[obj->otyp].oc_name_known &&
	    !objects[obj->otyp].oc_uname)
		docall(obj);
}

// obj is a ring being dropped over a kitchen sink
static void dosinkring(struct obj *obj) {
	struct obj *otmp, *otmp2;
	boolean ideed = true;

	pline("You drop %s down the drain.", doname(obj));
	obj->in_use = true;  /* block free identification via interrupt */
	switch (obj->otyp) { /* effects that can be noticed without eyes */
		case RIN_SEARCHING:
			pline("You thought %s got lost in the sink, but there it is!", yname(obj));
			goto giveback;
		case RIN_SLOW_DIGESTION:
			pline("The ring is regurgitated!");
		giveback:
			obj->in_use = false;
			dropx(obj);
			trycall(obj);
			return;
		case RIN_LEVITATION:
			pline("The sink quivers upward for a moment.");
			break;
		case RIN_POISON_RESISTANCE:
			pline("You smell rotten %s.", makeplural(fruitname(false)));
			break;
		case RIN_AGGRAVATE_MONSTER:
			pline("Several flies buzz angrily around the sink.");
			break;
		case RIN_SHOCK_RESISTANCE:
			pline("Static electricity surrounds the sink.");
			break;
		/* KMH, balance patch -- now an amulet
		    case RIN_DRAIN_RESISTANCE:
			pline("The sink looks weaker for a moment, but it passes.");
			break; */
		case RIN_CONFLICT:
			You_hear("loud noises coming from the drain.");
			break;
		case RIN_SLEEPING: /* ALI */
			You_hear("loud snores coming from the drain.");
			break;
		case RIN_SUSTAIN_ABILITY: /* KMH */
			pline("The water flow seems fixed.");
			break;
		case RIN_GAIN_STRENGTH:
			pline("The water flow seems %ser now.",
			      (obj->spe < 0) ? "weak" : "strong");
			break;
		case RIN_GAIN_CONSTITUTION:
			pline("The water flow seems %ser now.",
			      (obj->spe < 0) ? "less" : "great");
			break;
		case RIN_GAIN_INTELLIGENCE:
		case RIN_GAIN_WISDOM:
			pline("The water flow seems %ser now.",
			      (obj->spe < 0) ? "dull" : "quick");
			break;
		case RIN_GAIN_DEXTERITY:
			pline("The water flow seems %ser now.",
			      (obj->spe < 0) ? "slow" : "fast");
			break;
		case RIN_INCREASE_ACCURACY: /* KMH */
			pline("The water flow %s the drain.",
			      (obj->spe < 0) ? "misses" : "hits");
			break;
		case RIN_INCREASE_DAMAGE:
			pline("The water's force seems %ser now.",
			      (obj->spe < 0) ? "small" : "great");
			break;
		case RIN_HUNGER:
			ideed = false;
			for (otmp = level.objects[u.ux][u.uy]; otmp; otmp = otmp2) {
				otmp2 = otmp->nexthere;
				if (otmp != uball && otmp != uchain &&
				    !obj_resists(otmp, 1, 99)) {
					if (!Blind) {
						pline("Suddenly, %s %s from the sink!",
						      doname(otmp), otense(otmp, "vanish"));
						ideed = true;
					}
					delobj(otmp);
				}
			}
			break;
		case MEAT_RING:
			/* Not the same as aggravate monster; besides, it's obvious. */
			pline("Several flies buzz around the sink.");
			break;
		default:
			ideed = false;
			break;
	}
	if (!Blind && !ideed && obj->otyp != RIN_HUNGER) {
		ideed = true;
		switch (obj->otyp) { /* effects that need eyes */
			case RIN_ADORNMENT:
				pline("The faucets flash brightly for a moment.");
				break;
			case RIN_REGENERATION:
				pline("The sink looks as good as new.");
				break;
			case RIN_INVISIBILITY:
				pline("You don't see anything happen to the sink.");
				break;
			case RIN_FREE_ACTION:
				pline("You see the ring slide right down the drain!");
				break;
			case RIN_SEE_INVISIBLE:
				pline("You see some air in the sink.");
				break;
			case RIN_STEALTH:
				pline("The sink seems to blend into the floor for a moment.");
				break;
			case RIN_FIRE_RESISTANCE:
				pline("The hot water faucet flashes brightly for a moment.");
				break;
			case RIN_COLD_RESISTANCE:
				pline("The cold water faucet flashes brightly for a moment.");
				break;
			case RIN_PROTECTION_FROM_SHAPE_CHANGERS:
				pline("The sink looks nothing like a fountain.");
				break;
			case RIN_PROTECTION:
				pline("The sink glows %s for a moment.",
				      hcolor((obj->spe < 0) ? NH_BLACK : NH_SILVER));
				break;
			case RIN_WARNING:
				pline("The sink glows %s for a moment.", hcolor(NH_WHITE));
				break;
			case RIN_MOOD:
				pline("The sink looks groovy.");
				break;
			case RIN_TELEPORTATION:
				pline("The sink momentarily vanishes.");
				break;
			case RIN_TELEPORT_CONTROL:
				pline("The sink looks like it is being beamed aboard somewhere.");
				break;
			case RIN_POLYMORPH:
				pline("The sink momentarily looks like a fountain.");
				break;
			case RIN_POLYMORPH_CONTROL:
				pline("The sink momentarily looks like a regularly erupting geyser.");
				break;
		}
	}
	if (ideed)
		trycall(obj);
	else
		You_hear("the ring bouncing down the drainpipe.");
	if (!rn2(20)) {
		pline("The sink backs up, leaving %s.", doname(obj));
		obj->in_use = false;
		dropx(obj);
	} else
		useup(obj);
}

// some common tests when trying to drop or throw items
boolean canletgo(struct obj *obj, const char *word) {
	if (obj->owornmask & (W_ARMOR | W_RING | W_AMUL | W_TOOL)) {
		if (*word)
			Norep("You cannot %s something you are wearing.", word);
		return false;
	}
	/* KMH, balance patch -- removed stone of rotting */
	if (obj->otyp == LOADSTONE && obj->cursed) {
		/* getobj() kludge sets corpsenm to user's specified count
		   when refusing to split a stack of cursed loadstones */
		if (*word) {
			/* getobj() ignores a count for throwing since that is
			   implicitly forced to be 1; replicate its kludge... */
			if (!strcmp(word, "throw") && obj->quan > 1L)
				obj->corpsenm = 1;
			pline("For some reason, you cannot %s%s the stone%s!",
			      word, obj->corpsenm ? " any of" : "",
			      plur(obj->quan));
		}
		obj->corpsenm = 0; /* reset */
		obj->bknown = 1;
		return false;
	}
	if (obj->otyp == LEASH && obj->leashmon != 0) {
		if (*word)
			pline("The leash is tied around your %s.",
			      body_part(HAND));
		return false;
	}
	if (obj->owornmask & W_SADDLE) {
		if (*word)
			pline("You cannot %s something you are sitting on.", word);
		return false;
	}
	return true;
}

static int drop(struct obj *obj) {
	if (!obj) return 0;
	if (!canletgo(obj, "drop"))
		return 0;
	if (obj == uwep) {
		if (welded(uwep)) {
			weldmsg(obj);
			return 0;
		}
		setuwep(NULL, false);
	}
	if (obj == uswapwep) {
		setuswapwep(NULL, false);
	}
	if (obj == uquiver) {
		setuqwep(NULL);
	}

	if (u.uswallow) {
		/* barrier between you and the floor */
		if (flags.verbose) {
			char buf[BUFSZ];

			/* doname can call s_suffix, reusing its buffer */
			strcpy(buf, s_suffix(mon_nam(u.ustuck)));
			pline("You drop %s into %s %s.", doname(obj), buf,
			      mbodypart(u.ustuck, STOMACH));
		}
	} else {
		if ((obj->oclass == RING_CLASS || obj->otyp == MEAT_RING) &&
		    IS_SINK(levl[u.ux][u.uy].typ)) {
			dosinkring(obj);
			return 1;
		}
		if (!can_reach_floor()) {
			if (flags.verbose) pline("You drop %s.", doname(obj));
			/* Ensure update when we drop gold objects */
			if (obj->oclass == COIN_CLASS) context.botl = 1;
			freeinv(obj);
			hitfloor(obj);
			return 1;
		}
		if (!IS_ALTAR(levl[u.ux][u.uy].typ) && flags.verbose)
			pline("You drop %s.", doname(obj));
	}
	dropx(obj);
	return 1;
}

/* Called in several places - may produce output */
/* eg ship_object() and dropy() -> sellobj() both produce output */
void dropx(struct obj *obj) {
	/* Ensure update when we drop gold objects */
	if (obj->oclass == COIN_CLASS) context.botl = 1;
	freeinv(obj);
	if (!u.uswallow) {
		if (ship_object(obj, u.ux, u.uy, false)) return;
		if (IS_ALTAR(levl[u.ux][u.uy].typ))
			doaltarobj(obj); /* set bknown */
	}
	dropy(obj);
}

void dropy(struct obj *obj) {
	if (obj == uwep) setuwep(NULL, false);
	if (obj == uquiver) setuqwep(NULL);
	if (obj == uswapwep) setuswapwep(NULL, false);

	if (!u.uswallow && flooreffects(obj, u.ux, u.uy, "drop")) return;
	/* uswallow check done by GAN 01/29/87 */
	if (u.uswallow) {
		boolean could_petrify = false;
		boolean could_poly = false;
		boolean could_slime = false;
		boolean could_grow = false;
		boolean could_heal = false;

		if (obj != uball) { /* mon doesn't pick up ball */
			if (obj->otyp == CORPSE) {
				could_petrify = touch_petrifies(&mons[obj->corpsenm]);
				could_poly = polyfodder(obj);
				could_slime = (obj->corpsenm == PM_GREEN_SLIME);
				could_grow = (obj->corpsenm == PM_WRAITH);
				could_heal = (obj->corpsenm == PM_NURSE);
			}
			mpickobj(u.ustuck, obj);
			if (is_animal(u.ustuck->data)) {
				if (could_poly || could_slime) {
					newcham(u.ustuck,
						could_poly ? NULL : &mons[PM_GREEN_SLIME],
						false, could_slime);
					delobj(obj); /* corpse is digested */
				} else if (could_petrify) {
					minstapetrify(u.ustuck, true);
					/* Don't leave a cockatrice corpse in a statue */
					if (!u.uswallow) delobj(obj);
				} else if (could_grow) {
					grow_up(u.ustuck, NULL);
					delobj(obj); /* corpse is digested */
				} else if (could_heal) {
					u.ustuck->mhp = u.ustuck->mhpmax;
					delobj(obj); /* corpse is digested */
				}
			}
		}
	} else {
		place_object(obj, u.ux, u.uy);
		if (obj == uball)
			drop_ball(u.ux, u.uy);
		else
			sellobj(obj, u.ux, u.uy);
		stackobj(obj);
		if (Blind && Levitation)
			map_object(obj, 0);
		newsym(u.ux, u.uy); /* remap location under self */
	}
}

/* things that must change when not held; recurse into containers.
   Called for both player and monsters */
void obj_no_longer_held(struct obj *obj) {
	if (!obj) {
		return;
	} else if ((Is_container(obj) || obj->otyp == STATUE) && obj->cobj) {
		struct obj *contents;
		for (contents = obj->cobj; contents; contents = contents->nobj)
			obj_no_longer_held(contents);
	}
	switch (obj->otyp) {
		case CRYSKNIFE:
			/* KMH -- Fixed crysknives have only 10% chance of reverting */
			/* only changes when not held by player or monster */
			if (!obj->oerodeproof || !rn2(10)) {
				obj->otyp = WORM_TOOTH;
				obj->oerodeproof = 0;
			}
			break;
	}
}

// 'D' command: drop several things
int doddrop(void) {
	int result = 0;

	add_valid_menu_class(0); /* clear any classes already there */
	if (*u.ushops) sellobj_state(SELL_DELIBERATE);
	if (flags.menu_style != MENU_TRADITIONAL ||
	    (result = ggetobj("drop", drop, 0, false, NULL)) < -1)
		result = menu_drop(result);
	if (*u.ushops) sellobj_state(SELL_NORMAL);
	reset_occupations();

	return result;
}

/* Drop things from the hero's inventory, using a menu. */
static int menu_drop(int retry) {
	int n, i, n_dropped = 0;
	long cnt;
	struct obj *otmp, *otmp2;
	menu_item *pick_list;
	boolean all_categories = true;
	boolean drop_everything = false;

	if (retry) {
		all_categories = (retry == -2);
	} else if (flags.menu_style == MENU_FULL) {
		all_categories = false;
		n = query_category("Drop what type of items?",
				   invent,
				   UNPAID_TYPES | ALL_TYPES | CHOOSE_ALL |
					   BUC_BLESSED | BUC_CURSED | BUC_UNCURSED | BUC_UNKNOWN,
				   &pick_list, PICK_ANY);
		if (!n) goto drop_done;
		for (i = 0; i < n; i++) {
			if (pick_list[i].item.a_int == ALL_TYPES_SELECTED)
				all_categories = true;
			else if (pick_list[i].item.a_int == 'A')
				drop_everything = true;
			else
				add_valid_menu_class(pick_list[i].item.a_int);
		}
		free(pick_list);
	} else if (flags.menu_style == MENU_COMBINATION) {
		unsigned ggoresults = 0;
		all_categories = false;
		/* Gather valid classes via traditional NetHack method */
		i = ggetobj("drop", drop, 0, true, &ggoresults);
		if (i == -2) all_categories = true;
		if (ggoresults & ALL_FINISHED) {
			n_dropped = i;
			goto drop_done;
		}
	}

	if (drop_everything) {
		for (otmp = invent; otmp; otmp = otmp2) {
			otmp2 = otmp->nobj;
			n_dropped += drop(otmp);
		}
	} else {
		/* should coordinate with perm invent, maybe not show worn items */
		n = query_objlist("What would you like to drop?", invent,
				  USE_INVLET | INVORDER_SORT, &pick_list,
				  PICK_ANY, all_categories ? allow_all : allow_category);
		if (n > 0) {
			for (i = 0; i < n; i++) {
				otmp = pick_list[i].item.a_obj;
				cnt = pick_list[i].count;
				if (cnt < otmp->quan) {
					if (welded(otmp)) {
						; /* don't split */
					} else if (otmp->otyp == LOADSTONE && otmp->cursed) {
						/* same kludge as getobj(), for canletgo()'s use */
						otmp->corpsenm = (int)cnt; /* don't split */
					} else {
						otmp = splitobj(otmp, cnt);
					}
				}
				n_dropped += drop(otmp);
			}
			free(pick_list);
		}
	}

drop_done:
	return n_dropped;
}

/* on a ladder, used in goto_level */
static boolean at_ladder = false;

int dodown(void) {
	struct trap *trap = 0;
	boolean stairs_down = ((u.ux == xdnstair && u.uy == ydnstair) ||
			       (u.ux == sstairs.sx && u.uy == sstairs.sy && !sstairs.up)),
		ladder_down = (u.ux == xdnladder && u.uy == ydnladder);

	if (Role_if(PM_GNOME) && on_level(&mineend_level, &u.uz)) {
		pline("The staircase is filled with tons of rubble and debris.");
		pline("Poor Ruggo!");
		return 0;
	}

	if (u.usteed && !u.usteed->mcanmove) {
		pline("%s won't move!", Monnam(u.usteed));
		return 0;
	} else if (u.usteed && u.usteed->meating) {
		pline("%s is still eating.", Monnam(u.usteed));
		return 0;
	} else if (Levitation) {
		if ((HLevitation & I_SPECIAL) || (ELevitation & W_ARTI)) {
			/* end controlled levitation */
			if (ELevitation & W_ARTI) {
				struct obj *obj;

				for (obj = invent; obj; obj = obj->nobj) {
					if (obj->oartifact &&
					    artifact_has_invprop(obj, LEVITATION)) {
						if (obj->age < monstermoves)
							obj->age = monstermoves + rnz(100);
						else
							obj->age += rnz(100);
					}
				}
			}
			if (float_down(I_SPECIAL | TIMEOUT, W_ARTI))
				return 1; /* came down, so moved */
		}
		if (Blind) {
			/* Avoid alerting player to an unknown stair or ladder.
			 * Changes the message for a covered, known staircase
			 * too; staircase knowledge is not stored anywhere.
			 */
			if (stairs_down)
				stairs_down = (levl[u.ux][u.uy].mem_bg == S_dnstair);
			else if (ladder_down)
				ladder_down = (levl[u.ux][u.uy].mem_bg == S_dnladder);
		}

		floating_above(stairs_down ? "stairs" : ladder_down ? "ladder" : surface(u.ux, u.uy));
		return 0; /* didn't move */
	}
	if (!stairs_down && !ladder_down) {
		trap = t_at(u.ux, u.uy);
		if (trap && uteetering_at_seen_pit()) {
			dotrap(trap, TOOKPLUNGE);
			return 1;
		} else if (!trap || !is_holelike(trap->ttyp) || !Can_fall_thru(&u.uz) || !trap->tseen) {
			if (flags.autodig && !context.nopick &&
			    uwep && is_pick(uwep)) {
				return use_pick_axe2(uwep);
			} else {
				pline("You can't go down here.");
				return 0;
			}
		}
	}
	if (u.ustuck) {
		pline("You are %s, and cannot go down.",
		      !u.uswallow ? "being held" : is_animal(u.ustuck->data) ? "swallowed" : "engulfed");
		return 1;
	}
	if (on_level(&valley_level, &u.uz) && !u.uevent.gehennom_entered) {
		pline("You are standing at the gate to Gehennom.");
		pline("Unspeakable cruelty and harm lurk down there.");
		if (yn("Are you sure you want to enter?") != 'y')
			return 0;
		else
			pline("So be it.");
		u.uevent.gehennom_entered = 1; /* don't ask again */
	}

	if (!next_to_u()) {
		pline("You are held back by your pet!");
		return 0;
	}

	if (trap)
		pline("You %s %s.", Flying ? "fly" : locomotion(youmonst.data, "jump"),
		      trap->ttyp == HOLE ? "down the hole" : "through the trap door");

	if (trap && Is_stronghold(&u.uz)) {
		goto_hell(false, true);
	} else {
		at_ladder = (boolean)(levl[u.ux][u.uy].typ == LADDER);
		next_level(!trap);
		at_ladder = false;
	}
	return 1;
}

int doup(void) {
	if ((u.ux != xupstair || u.uy != yupstair) && (!xupladder || u.ux != xupladder || u.uy != yupladder) && (!sstairs.sx || u.ux != sstairs.sx || u.uy != sstairs.sy || !sstairs.up)) {
		pline("You can't go up here.");
		return 0;
	}
	if (u.usteed && !u.usteed->mcanmove) {
		pline("%s won't move!", Monnam(u.usteed));
		return 0;
	} else if (u.usteed && u.usteed->meating) {
		pline("%s is still eating.", Monnam(u.usteed));
		return 0;
	} else if (u.ustuck) {
		pline("You are %s, and cannot go up.",
		      !u.uswallow ? "being held" : is_animal(u.ustuck->data) ? "swallowed" : "engulfed");
		return 1;
	}
	if (near_capacity() > SLT_ENCUMBER) {
		/* No levitation check; inv_weight() already allows for it */
		pline("Your load is too heavy to climb the %s.",
		      levl[u.ux][u.uy].typ == STAIRS ? "stairs" : "ladder");
		return 1;
	}
	if (ledger_no(&u.uz) == 1) {
		if (iflags.debug_fuzzer) {
			return 0;
		}
		if (yn("Beware, there will be no return! Still climb?") != 'y') {
			return 0;
		}
	}
	if (!next_to_u()) {
		pline("You are held back by your pet!");
		return 0;
	}
	at_ladder = (boolean)(levl[u.ux][u.uy].typ == LADDER);
	prev_level(true);
	at_ladder = false;
	return 1;
}

d_level save_dlevel = {0, 0};

/* check that we can write out the current level */
static int currentlevel_rewrite(void) {
	int fd;
	char whynot[BUFSZ];

	/* since level change might be a bit slow, flush any buffered screen
	 *  output (like "you fall through a trap door") */
	mark_synch();

	fd = create_levelfile(ledger_no(&u.uz), whynot);
	if (fd < 0) {
		/*
		 * This is not quite impossible: e.g., we may have
		 * exceeded our quota. If that is the case then we
		 * cannot leave this level, and cannot save either.
		 * Another possibility is that the directory was not
		 * writable.
		 */
		pline("%s", whynot);
		return -1;
	}

	return fd;
}

#ifdef INSURANCE
void save_currentstate(void) {
	int fd;

	if (flags.ins_chkpt) {
		/* write out just-attained level, with pets and everything */
		fd = currentlevel_rewrite();
		if (fd < 0) return;
		bufon(fd);
		savelev(fd, ledger_no(&u.uz), WRITE_SAVE);
		bclose(fd);
	}

	/* write out non-level state */
	savestateinlock();
}
#endif

/*
static boolean badspot(xchar x, xchar y) {
	return((levl[x][y].typ != ROOM && levl[x][y].typ != AIR &&
			 levl[x][y].typ != CORR) || MON_AT(x, y));
}
*/

d_level new_dlevel = {0, 0};

void goto_level(d_level *newlevel, boolean at_stairs, boolean falling, boolean portal) {
	int fd, l_idx;
	xchar new_ledger;
	boolean cant_go_back,
		up = (depth(newlevel) < depth(&u.uz)),
		newdungeon = (u.uz.dnum != newlevel->dnum),
		was_in_W_tower = In_W_tower(u.ux, u.uy, &u.uz),
		familiar = false;
	boolean new = false; /* made a new level? */
	struct monst *mtmp;
	char whynot[BUFSZ];

	// 'You descend/fly up/down the stairs' is common enough that you
	// don't want to get a --More-- prompt every time it shows up, so
	// print it after going through the stairs.
	// (Going through a ladder is a rare enough occurance that taking
	//  extra time about it is ok.)
	nhstr deferred_msg = new_nhs();

	if (dunlev(newlevel) > dunlevs_in_dungeon(newlevel))
		newlevel->dlevel = dunlevs_in_dungeon(newlevel);
	if (newdungeon && In_endgame(newlevel)) { /* 1st Endgame Level !!! */
		if (u.uhave.amulet)
			assign_level(newlevel, &earth_level);
		else
			return;
	}
	new_ledger = ledger_no(newlevel);
	if (new_ledger <= 0)
		done(ESCAPED); /* in fact < 0 is impossible */

	assign_level(&new_dlevel, newlevel);

	/* If you have the amulet and are trying to get out of Gehennom, going
	 * up a set of stairs sometimes does some very strange things!
	 * Biased against law and towards chaos, but not nearly as strongly
	 * as it used to be (prior to 3.2.0).
	 * Odds:	    old				    new
	 *	"up"    L      N      C		"up"    L      N      C
	 *	 +1   75.0   75.0   75.0	 +1   75.0   75.0   75.0
	 *	  0    0.0   12.5   25.0	  0    6.25   8.33  12.5
	 *	 -1    8.33   4.17   0.0	 -1    6.25   8.33  12.5
	 *	 -2    8.33   4.17   0.0	 -2    6.25   8.33   0.0
	 *	 -3    8.33   4.17   0.0	 -3    6.25   0.0    0.0
	 * [Tom] I removed this... it's indescribably annoying.
	 *
	 * if (Inhell && up && u.uhave.amulet && !newdungeon && !portal &&
	 *			(dunlev(&u.uz) < dunlevs_in_dungeon(&u.uz)-3)) {
	 *	if (!rn2(4)) {
	 *	    int odds = 3 + (int)u.ualign.type,          * 2..4 *
	 *		diff = odds <= 1 ? 0 : rn2(odds);       * paranoia *
	 *
	 *	    if (diff != 0) {
	 *		assign_rnd_level(newlevel, &u.uz, diff);
	 *		* if inside the tower, stay inside *
	 *		if (was_in_W_tower &&
	 *		    !On_W_tower_level(newlevel)) diff = 0;
	 *	    }
	 *	    if (diff == 0)
	 *		assign_level(newlevel, &u.uz);
	 *
	 *	    new_ledger = ledger_no(newlevel);
	 *
	 *	    pline("A mysterious force momentarily surrounds you...");
	 *	    if (on_level(newlevel, &u.uz)) {
	 *		(void) safe_teleds(false);
	 *		(void) next_to_u();
	 *		return;
	 *	    } else
	 *		at_stairs = at_ladder = false;
	 *	}
	}*/

	/* Prevent the player from going past the first quest level unless
	 * (s)he has been given the go-ahead by the leader.
	 */
	if (on_level(&u.uz, &qstart_level) && !newdungeon && !ok_to_quest()) {
		pline("A mysterious force prevents you from descending.");
		return;
	}

	if (on_level(newlevel, &u.uz)) return; /* this can happen */

	fd = currentlevel_rewrite();
	if (fd < 0) return;

	if (falling) /* assuming this is only trap door or hole */
		impact_drop(NULL, u.ux, u.uy, newlevel->dlevel);

	check_special_room(true); /* probably was a trap door */
	if (Punished) unplacebc();
	u.utrap = 0; /* needed in level_tele */
	fill_pit(u.ux, u.uy);
	setustuck(0); /* idem */
	u.uinwater = 0;
	u.uundetected = 0; /* not hidden, even if means are available */
	keepdogs(false);
	if (u.uswallow) /* idem */
		u.uswldtim = u.uswallow = 0;
	recalc_mapseen();  // recalculate map overview before we leave the level

	/*
	 *  We no longer see anything on the level.  Make sure that this
	 *  follows u.uswallow set to null since uswallow overrides all
	 *  normal vision.
	 */
	vision_recalc(2);

	/*
	 * Save the level we're leaving.  If we're entering the endgame,
	 * we can get rid of all existing levels because they cannot be
	 * reached any more.  We still need to use savelev()'s cleanup
	 * for the level being left, to recover dynamic memory in use and
	 * to avoid dangling timers and light sources.
	 */
	cant_go_back = (newdungeon && In_endgame(newlevel));
	if (!cant_go_back) {
		update_mlstmv(); /* current monsters are becoming inactive */
		bufon(fd);	 /* use buffered output */
	}
	savelev(fd, ledger_no(&u.uz),
		cant_go_back ? FREE_SAVE : (WRITE_SAVE | FREE_SAVE));
	bclose(fd);
	if (cant_go_back) {
		/* discard unreachable levels; keep #0 */
		for (l_idx = maxledgerno(); l_idx > 0; --l_idx)
			delete_levelfile(l_idx);
	}

	if (Is_rogue_level(newlevel) || Is_rogue_level(&u.uz))
		assign_rogue_graphics(Is_rogue_level(newlevel));

#ifdef USE_TILES
	substitute_tiles(newlevel);
#endif
	/* record this level transition as a potential seen branch unless using
	 * some non-standard means of transportation (level teleport).
	 */
	if ((at_stairs || falling || portal) && (u.uz.dnum != newlevel->dnum))
		recbranch_mapseen(&u.uz, newlevel);
	assign_level(&u.uz0, &u.uz);
	assign_level(&u.uz, newlevel);
	assign_level(&u.utolev, newlevel);
	u.utotype = 0;
	if (dunlev_reached(&u.uz) < dunlev(&u.uz))
		dunlev_reached(&u.uz) = dunlev(&u.uz);
	reset_rndmonst(NON_PM); /* u.uz change affects monster generation */

	/* set default level change destination areas */
	/* the special level code may override these */
	memset(&updest, 0, sizeof(updest));
	memset(&dndest, 0, sizeof(dndest));

	if (!(level_info[new_ledger].flags & LFILE_EXISTS)) {
		/* entering this level for first time; make it now */
		if (level_info[new_ledger].flags & VISITED) {
			impossible("goto_level: returning to discarded level?");
			level_info[new_ledger].flags &= ~VISITED;
		}
		mklev();
		new = true; /* made the level */

		familiar = find_ghost_with_name(plname) != NULL;
	} else {
		/* returning to previously visited level; reload it */
		fd = open_levelfile(new_ledger, whynot);
		if (fd < 0) {
			pline("%s", whynot);
			pline("Probably someone removed it.");
			nhscopyz(&killer.name, whynot);
			done(TRICKED);
			/* we'll reach here if running in wizard mode */
			error("Cannot continue this game.");
		}
		getlev(fd, hackpid, new_ledger, false);
		close(fd);
	}

	/* do this prior to level-change pline messages */
	vision_reset();		/* clear old level's line-of-sight */
	vision_full_recalc = 0; /* don't let that reenable vision yet */
	flush_screen(-1);	/* ensure all map flushes are postponed */

	if (portal && !In_endgame(&u.uz)) {
		/* find the portal on the new level */
		struct trap *ttrap;

		for (ttrap = ftrap; ttrap; ttrap = ttrap->ntrap)
			if (ttrap->ttyp == MAGIC_PORTAL) break;

		if (!ttrap) panic("goto_level: no corresponding portal!");
		seetrap(ttrap);
		u_on_newpos(ttrap->tx, ttrap->ty);
	} else if (at_stairs && !In_endgame(&u.uz)) {
		if (up) {
			if (at_ladder) {
				u_on_newpos(xdnladder, ydnladder);
			} else {
				if (newdungeon) {
					if (Is_stronghold(&u.uz)) {
						xchar x, y;

						do {
							x = (COLNO - 2 - rnd(5));
							y = rn1(ROWNO - 4, 3);
						} while (occupied(x, y) ||
							 IS_WALL(levl[x][y].typ));
						u_on_newpos(x, y);
					} else
						u_on_sstairs();
				} else
					u_on_dnstairs();
			}
			/* you climb up the {stairs|ladder};
			   fly up the stairs; fly up along the ladder */
			if (at_ladder) {
				pline("%s %s %s the ladder.",
						(Punished && !Levitation) ? "With great effort you" :
						"You",
						Flying ? "fly" : "climb",
						Flying ? "up along" : "up");
			} else {
				nhscopyf(&deferred_msg, "%S %S up the stairs.",
						(Punished && !Levitation) ? "With great effort you" : "You",
						Flying ? "fly" : "climb");
			}
		} else { /* down */
			if (at_ladder) {
				u_on_newpos(xupladder, yupladder);
			} else {
				if (newdungeon)
					u_on_sstairs();
				else
					u_on_upstairs();
			}
			if (!u.dz) {
				;   /* stayed on same level? (no transit effects) */
			} else if (Flying) {
				if (flags.verbose) {
					if (at_ladder) pline("You fly down along the ladder.");
					else nhscopyz(&deferred_msg, "You fly down the stairs.");
				}
			} else if (near_capacity() > UNENCUMBERED || Punished || Fumbling) {
				pline("You fall down the %s.", at_ladder ? "ladder" : "stairs");
				if (Punished) {
					drag_down();
					if (carried(uball)) {
						if (uswapwep == uball)
							setuswapwep(NULL, false);
						if (uquiver == uball)
							setuqwep(NULL);
						if (uwep == uball)
							setuwep(NULL, false);
						freeinv(uball);
					}
				}
				/* falling off steed has its own losehp() call */
				if (u.usteed)
					dismount_steed(DISMOUNT_FELL);
				else
					losehp(Maybe_Half_Phys(rnd(3)), at_ladder ? "falling off a ladder" : "tumbling down a flight of stairs", KILLED_BY);
				selftouch("Falling, you");
			} else {        /* ordinary descent */
				if (flags.verbose) {
					if (at_ladder) pline("You climb down the ladder.");
					else nhscopyz(&deferred_msg, "You descend the stairs.");
				}
			}
		}
	} else { /* trap door or level_tele or In_endgame */
		if (was_in_W_tower && On_W_tower_level(&u.uz))
			/* Stay inside the Wizard's tower when feasible.	*/
			/* Note: up vs down doesn't really matter in this case. */
			place_lregion(dndest.nlx, dndest.nly,
				      dndest.nhx, dndest.nhy,
				      0, 0, 0, 0, LR_DOWNTELE, NULL);
		else if (up)
			place_lregion(updest.lx, updest.ly,
				      updest.hx, updest.hy,
				      updest.nlx, updest.nly,
				      updest.nhx, updest.nhy,
				      LR_UPTELE, NULL);
		else
			place_lregion(dndest.lx, dndest.ly,
				      dndest.hx, dndest.hy,
				      dndest.nlx, dndest.nly,
				      dndest.nhx, dndest.nhy,
				      LR_DOWNTELE, NULL);
		if (falling) {
			if (Punished) ballfall();
			selftouch("Falling, you");
		}
	}

	if (Punished) placebc();
	obj_delivery(false);
	losedogs();
	kill_genocided_monsters(); /* for those wiped out while in limbo */
	/*
	 * Expire all timers that have gone off while away.  Must be
	 * after migrating monsters and objects are delivered
	 * (losedogs and obj_delivery).
	 */
	run_timers();

	initrack();

	if ((mtmp = m_at(u.ux, u.uy)) != 0 && mtmp != u.usteed) {
		/* There's a monster at your target destination; it might be one
		   which accompanied you--see mon_arrive(dogmove.c)--or perhaps
		   it was already here.  Randomly move you to an adjacent spot
		   or else the monster to any nearby location.  Prior to 3.3.0
		   the latter was done unconditionally. */
		coord cc;

		if (!rn2(2) &&
		    enexto(&cc, u.ux, u.uy, youmonst.data) &&
		    distu(cc.x, cc.y) <= 2)
			u_on_newpos(cc.x, cc.y); /*[maybe give message here?]*/
		else
			mnexto(mtmp);

		if ((mtmp = m_at(u.ux, u.uy)) != 0) {
			impossible("mnexto failed (do.c)?");
			rloc(mtmp, false);
		}
	}

	/* initial movement of bubbles just before vision_recalc */
	if (Is_waterlevel(&u.uz))
		movebubbles();

	/* Reset the screen. */
	vision_reset(); /* reset the blockages */
	docrt();	/* does a full vision recalc */
	flush_screen(-1);

	if (deferred_msg.len) {
		plines(nhs2cstr_tmp(deferred_msg));
		del_nhs(&deferred_msg);
	}

	/*
	 *  Move all plines beyond the screen reset.
	 */

	/* give room entrance message, if any */
	check_special_room(false);

	/* deliver objects traveling with player */
	obj_delivery(true);

	/* Check whether we just entered Gehennom. */
	if (!In_hell(&u.uz0) && Inhell) {
		if (Is_valley(&u.uz)) {
			pline("You arrive at the Valley of the Dead...");
			pline("The odor of burnt flesh and decay pervades the air.");
			pline("You hear groans and moans everywhere.");
		} else {
			pline("It is hot here.  You smell smoke...");
		}

		achieve.enter_gehennom = true;
	}

	// in case we've managed to bypass the Valley's stairway down
	if (Inhell && !Is_valley(&u.uz)) u.uevent.gehennom_entered = true;



	if (familiar) {
		static const char *const fam_msgs[4] = {
			"You have a sense of deja vu.",
			"You feel like you've been here before.",
			"This place %s familiar...",
			0 /* no message */
		};
		static const char *const halu_fam_msgs[4] = {
			"Whoa!  Everything %s different.",
			"You are surrounded by twisty little passages, all alike.",
			"Gee, this %s like uncle Conan's place...",
			0 /* no message */
		};
		const char *mesg;
		char buf[BUFSZ];
		int which = rn2(4);

		if (Hallucination)
			mesg = halu_fam_msgs[which];
		else
			mesg = fam_msgs[which];
		if (mesg && index(mesg, '%')) {
			sprintf(buf, mesg, !Blind ? "looks" : "seems");
			mesg = buf;
		}
		if (mesg) pline("%s", mesg);
	}

	if (new && Is_rogue_level(&u.uz))
		pline("You enter what seems to be an older, more primitive world.");

	if (depth(&u.uz) >= poisoned_pit_threshold && !know_poisoned_pit_threshold) {
		if (!Poison_resistance) {
			pline("You feel worried about your health.");
		} else {
			pline("You feel more secure about your healthful choices.");
		}

		know_poisoned_pit_threshold = true;
	}

	/* Final confrontation */
	if (In_endgame(&u.uz) && newdungeon && u.uhave.amulet)
		resurrect();
	if (newdungeon && In_V_tower(&u.uz) && In_hell(&u.uz0))
		pline("The heat and smoke are gone.");

	/* the message from your quest leader */
	if (!In_quest(&u.uz0) && at_dgn_entrance("The Quest") &&
	    !(u.uevent.qexpelled || u.uevent.qcompleted || quest_status.leader_is_dead)) {
		if (u.uevent.qcalled) {
			com_pager(Role_if(PM_ROGUE) ? 4 : 3);
		} else {
			com_pager(2);
			u.uevent.qcalled = true;
		}
	}

	/* once Croesus is dead, his alarm doesn't work any more */
	if (Is_knox(&u.uz) && (new || !mvitals[PM_CROESUS].died)) {
		pline("You penetrated a high security area!");
		pline("An alarm sounds!");
		for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
			if (!DEADMONSTER(mtmp) && mtmp->msleeping) mtmp->msleeping = 0;
	}

	if (on_level(&u.uz, &astral_level))
		final_level();
	else
		onquest();
	assign_level(&u.uz0, &u.uz); /* reset u.uz0 */

#ifdef INSURANCE
	save_currentstate();
#endif

	/* assume this will always return true when changing level */
	in_out_region(u.ux, u.uy);
	pickup(1);
}

static void final_level(void) {
	struct monst *mtmp;

	/* reset monster hostility relative to player */
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		reset_hostility(mtmp);
	}

	/* create some player-monsters */
	create_mplayers(rn1(4, 3), true);

	/* create a guardian angel next to player, if worthy */
	gain_guardian_angel();
}

static char *dfr_pre_msg = 0, /* pline() before level change */
	*dfr_post_msg = 0;    /* pline() after level change */

/* change levels at the end of this turn, after monsters finish moving */
void schedule_goto(d_level *tolev, boolean at_stairs, boolean falling, int portal_flag, const char *pre_msg, const char *post_msg) {
	int typmask = 0100; /* non-zero triggers `deferred_goto' */

	/* destination flags (`goto_level' args) */
	if (at_stairs) typmask |= 1;
	if (falling) typmask |= 2;
	if (portal_flag) typmask |= 4;
	if (portal_flag < 0) typmask |= 0200; /* flag for portal removal */
	u.utotype = typmask;
	/* destination level */
	assign_level(&u.utolev, tolev);

	if (pre_msg)
		dfr_pre_msg = strcpy(alloc(strlen(pre_msg) + 1), pre_msg);
	if (post_msg)
		dfr_post_msg = strcpy(alloc(strlen(post_msg) + 1), post_msg);
}

/* handle something like portal ejection */
void deferred_goto(void) {
	if (!on_level(&u.uz, &u.utolev)) {
		d_level dest;
		int typmask = u.utotype; /* save it; goto_level zeroes u.utotype */

		assign_level(&dest, &u.utolev);
		if (dfr_pre_msg) pline("%s", dfr_pre_msg);
		goto_level(&dest, !!(typmask & 1), !!(typmask & 2), !!(typmask & 4));
		if (typmask & 0200) { /* remove portal */
			struct trap *t = t_at(u.ux, u.uy);

			if (t) {
				deltrap(t);
				newsym(u.ux, u.uy);
			}
		}
		if (dfr_post_msg) pline("%s", dfr_post_msg);
	}
	u.utotype = 0; /* our caller keys off of this */
	if (dfr_pre_msg)
		free(dfr_pre_msg), dfr_pre_msg = 0;
	if (dfr_post_msg)
		free(dfr_post_msg), dfr_post_msg = 0;
}

/*
 * Return true if we created a monster for the corpse.  If successful, the
 * corpse is gone.
 */
boolean revive_corpse(struct obj *corpse, boolean moldy) {
	struct monst *mtmp, *mcarry;
	boolean is_uwep, chewed;
	xchar where;
	char *cname, cname_buf[BUFSZ];
	struct obj *container = NULL;
	int container_where = 0;

	where = corpse->where;
	is_uwep = corpse == uwep;
	cname = eos(strcpy(cname_buf, "bite-covered "));
	strcpy(cname, corpse_xname(corpse, true));
	mcarry = (where == OBJ_MINVENT) ? corpse->ocarry : 0;

	if (where == OBJ_CONTAINED) {
		struct monst *mtmp2 = NULL;
		container = corpse->ocontainer;
		mtmp2 = get_container_location(container, &container_where, NULL);
		/* container_where is the outermost container's location even if nested */
		if (container_where == OBJ_MINVENT && mtmp2) mcarry = mtmp2;
	}
	mtmp = revive(corpse, false); /* corpse is gone if successful && quan == 1 */

	if (mtmp) {
		/*
		 * [ALI] Override revive's HP calculation. The HP that a mold starts
		 * with do not depend on the HP of the monster whose corpse it grew on.
		 */
		if (moldy)
			mtmp->mhp = mtmp->mhpmax;
		chewed = !moldy && (mtmp->mhp < mtmp->mhpmax);
		if (chewed) cname = cname_buf; /* include "bite-covered" prefix */
		switch (where) {
			case OBJ_INVENT:
				if (is_uwep) {
					if (moldy) {
						pline("Your weapon goes moldy.");
						pline("%s writhes out of your grasp!", Monnam(mtmp));
					} else
						pline("The %s writhes out of your grasp!", cname);
				} else
					pline("You feel squirming in your backpack!");
				break;

			case OBJ_FLOOR:
				if (cansee(mtmp->mx, mtmp->my)) {
					if (moldy)
						pline("%s grows on a moldy corpse!",
						      Amonnam(mtmp));
					else
						pline("%s rises from the dead!", chewed ?
											 Adjmonnam(mtmp, "bite-covered") :
											 Monnam(mtmp));
				}
				break;

			case OBJ_MINVENT: /* probably a nymph's */
				if (cansee(mtmp->mx, mtmp->my)) {
					if (canseemon(mcarry))
						pline("Startled, %s drops %s as it %s!",
						      mon_nam(mcarry), moldy ? "a corpse" : an(cname),
						      moldy ? "goes moldy" : "revives");
					else
						pline("%s suddenly appears!", chewed ?
										      Adjmonnam(mtmp, "bite-covered") :
										      Monnam(mtmp));
				}
				break;
			case OBJ_CONTAINED:
				if (container_where == OBJ_MINVENT && cansee(mtmp->mx, mtmp->my) &&
				    mcarry && canseemon(mcarry) && container) {
					pline("%s writhes out of %s!", Amonnam(mtmp), yname(container));
				} else if (container_where == OBJ_INVENT && container) {
					char sackname[BUFSZ];
					strcpy(sackname, an(xname(container)));
					pline("%s %ss out of %s in your pack!",
					      Blind ? "Something" : Amonnam(mtmp),
					      locomotion(mtmp->data, "writhes"),
					      sackname);
				} else if (container_where == OBJ_FLOOR && container &&
					   cansee(mtmp->mx, mtmp->my)) {
					char sackname[BUFSZ];
					strcpy(sackname, an(xname(container)));
					pline("%s escapes from %s!", Amonnam(mtmp), sackname);
				}
				break;
			default:
				/* we should be able to handle the other cases... */
				impossible("revive_corpse: lost corpse @ %d", where);
				break;
		}
		return true;
	}
	return false;
}

/* Revive the corpse via a timeout. */
/*ARGSUSED*/
void revive_mon(void *arg, long timeout) {
#if defined(MAC_MPW)
#pragma unused(timeout)
#endif
	struct obj *body = arg;

	/* if we succeed, the corpse is gone, otherwise, rot it away */
	if (!revive_corpse(body, false)) {
		if (is_rider(&mons[body->corpsenm]))
			pline("You feel less hassled.");
		start_timer(250 - (monstermoves - body->age), TIMER_OBJECT, ROT_CORPSE, obj_to_any(body));
	}
}

/* Revive the corpse as a mold via a timeout. */
void moldy_corpse(void *arg, long timeout) {
	int pmtype, oldtyp, oldquan, oldnamelth;
	struct obj *body = (struct obj *)arg;

	/* Turn the corpse into a mold corpse if molds are available */
	oldtyp = body->corpsenm;
	oldnamelth = body->onamelth;

	/* Weight towards non-motile fungi.
	 */
	pmtype = pm_mkclass(S_FUNGUS, 0);
	if ((pmtype != -1) && (mons[pmtype].mmove)) pmtype = pm_mkclass(S_FUNGUS, 0);

	/* [ALI] Molds don't grow in adverse conditions.  If it ever
	 * becomes possible for molds to grow in containers we should
	 * check for iceboxes here as well.
	 */
	if ((body->where == OBJ_FLOOR || body->where == OBJ_BURIED) &&
	    (is_pool(body->ox, body->oy) || is_lava(body->ox, body->oy) ||
	     is_ice(body->ox, body->oy)))
		pmtype = -1;

	if (pmtype != -1) {
		/* We don't want special case revivals */
		if (cant_create(&pmtype, true) || (body->oxlth &&
						   (body->oattached == OATTACHED_MONST)))
			pmtype = -1; /* cantcreate might have changed it so change it back */
		else {
			body->corpsenm = pmtype;
			body->onamelth = 0; /* Molds shouldn't be named */

			/* oeaten isn't used for hp calc here, and zeroing it
			 * prevents eaten_stat() from worrying when you've eaten more
			 * from the corpse than the newly grown mold's nutrition
			 * value.
			 */
			body->oeaten = 0;

			/* [ALI] If we allow revive_corpse() to get rid of revived
			 * corpses from hero's inventory then we run into problems
			 * with unpaid corpses.
			 */
			if (body->where == OBJ_INVENT)
				body->quan++;
			oldquan = body->quan;
			if (revive_corpse(body, true)) {
				if (oldquan != 1) { /* Corpse still valid */
					body->corpsenm = oldtyp;
					if (body->where == OBJ_INVENT) {
						useup(body);
						oldquan--;
					}
				}
				if (oldquan == 1)
					body = NULL; /* Corpse gone */
			}
		}
	}

	/* If revive_corpse succeeds, it handles the reviving corpse.
	 * If there was more than one corpse, or the revive failed,
	 * set the remaining corpse(s) to rot away normally.
	 * Revive_corpse handles genocides
	 */
	if (body) {
		body->corpsenm = oldtyp; /* Fixup corpse after (attempted) revival */
		body->onamelth = oldnamelth;
		body->owt = weight(body);
		start_timer(250L - (monstermoves - peek_at_iced_corpse_age(body)), TIMER_OBJECT, ROT_CORPSE, obj_to_any(body));
	}
}

int donull(void) {
	return 1;  // Do nothing, but let other things happen
}

static int wipeoff(void) {
	if (u.ucreamed < 4)
		u.ucreamed = 0;
	else
		u.ucreamed -= 4;
	if (Blinded < 4)
		Blinded = 0;
	else
		Blinded -= 4;
	if (!Blinded) {
		pline("You've got the glop off.");
		u.ucreamed = 0;
		if (!gulp_blnd_check()) {
			Blinded = 1;
			make_blinded(0L, true);
		}
		return 0;
	} else if (!u.ucreamed) {
		pline("Your %s feels clean now.", body_part(FACE));
		return 0;
	}
	return 1; /* still busy */
}

int dowipe(void) {
	if (u.ucreamed) {
		static char buf[39];

		sprintf(buf, "wiping off your %s", body_part(FACE));
		set_occupation(wipeoff, buf, 0);
		/* Not totally correct; what if they change back after now
		 * but before they're finished wiping?
		 */
		return 1;
	}
	pline("Your %s is already clean.", body_part(FACE));
	return 1;
}

void set_wounded_legs(long side, int timex) {
	/* KMH -- STEED
	 * If you are riding, your steed gets the wounded legs instead.
	 * You still call this function, but don't lose hp.
	 * Caller is also responsible for adjusting messages.
	 */

	if (!Wounded_legs) {
		ATEMP(A_DEX)
		--;
		context.botl = 1;
	}

	/* KMH, intrinsics patch */
	if (!Wounded_legs || (HWounded_legs & TIMEOUT))
		HWounded_legs = timex;
	EWounded_legs = side;
	encumber_msg();
}

void heal_legs(void) {
	if (Wounded_legs) {
		if (ATEMP(A_DEX) < 0) {
			ATEMP(A_DEX)
			++;
			context.botl = 1;
		}

		if (!u.usteed) {
			/* KMH, intrinsics patch */
			if ((EWounded_legs & BOTH_SIDES) == BOTH_SIDES) {
				pline("Your %s feel somewhat better.",
				      makeplural(body_part(LEG)));
			} else {
				pline("Your %s feels somewhat better.",
				      body_part(LEG));
			}
		}
		HWounded_legs = EWounded_legs = 0;
	}
	encumber_msg();
}
/*do.c*/
