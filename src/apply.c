/*	SCCS Id: @(#)apply.c	3.4	2003/11/18	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "edog.h"
#include "eleash.h"

static const char tools[] = {TOOL_CLASS, WEAPON_CLASS, WAND_CLASS, 0};
static const char tools_too[] = {ALL_CLASSES, TOOL_CLASS, POTION_CLASS,
				 WEAPON_CLASS, WAND_CLASS, GEM_CLASS, 0};
static const char tinnables[] = {ALLOW_FLOOROBJ, FOOD_CLASS, 0};

static int use_camera(struct obj *obj);
static int use_towel(struct obj *obj);
static bool its_dead(int x, int y, int *resp);
static int use_stethoscope(struct obj *obj);
static void use_whistle(struct obj *obj);
static void use_magic_whistle(struct obj *obj);
static void use_leash(struct obj **optr);
static int use_mirror(struct obj *obj);
static void use_bell(struct obj **optr);
static void use_candelabrum(struct obj *obj);
static void use_candle(struct obj **optr);
static void use_lamp(struct obj *obj);
static int use_torch(struct obj *obj);
static void light_cocktail(struct obj *obj);
static void use_tinning_kit(struct obj *obj);
static boolean figurine_location_checks(struct obj *, coord *, boolean);
static void use_figurine(struct obj **optr);
static void use_grease(struct obj *obj);
static void use_stone(struct obj *);
static void use_trap(struct obj *obj);
static int set_trap(void); /* occupation callback */
static int use_whip(struct obj *obj);
static int use_pole(struct obj *obj);
static int use_cream_pie(struct obj *obj);
static int use_grapple(struct obj *obj);
static int do_break_wand(struct obj *obj);
static bool uhave_graystone(void);
static void add_class(char *, char);

static int use_camera(struct obj *obj) {
	struct monst *mtmp;

	if (Underwater) {
		pline("Using your camera underwater would void the warranty.");
		return 0;
	}
	if (!getdir(NULL)) return 0;

	if (obj->spe <= 0) {
		pline("%s", "Nothing happens.");
		return 1;
	}
	consume_obj_charge(obj, true);

	if (obj->cursed && !rn2(2)) {
		zapyourself(obj, true);
	} else if (u.uswallow) {
		pline("You take a picture of %s %s.", s_suffix(mon_nam(u.ustuck)),
		      mbodypart(u.ustuck, STOMACH));
	} else if (u.dz) {
		pline("You take a picture of the %s.",
		      (u.dz > 0) ? surface(u.ux, u.uy) : ceiling(u.ux, u.uy));
	} else if (!u.dx && !u.dy) {
		zapyourself(obj, true);
	} else if ((mtmp = bhit(u.dx, u.dy, COLNO, FLASHED_LIGHT,
				(int (*)(struct monst *, struct obj *))0,
				(int (*)(struct obj *, struct obj *))0,
				&obj)) != 0) {
		obj->ox = u.ux, obj->oy = u.uy;
		flash_hits_mon(mtmp, obj);
	}
	return 1;
}

static int use_towel(struct obj *obj) {
	if (!freehand()) {
		pline("You have no free %s!", body_part(HAND));
		return 0;
	} else if (obj->owornmask) {
		pline("You cannot use it while you're wearing it!");
		return 0;
	} else if (obj->cursed) {
		long old;
		switch (rn2(3)) {
			case 2:
				old = Glib;
				incr_itimeout(&Glib, rn1(10, 3));
				pline("Your %s %s!", makeplural(body_part(HAND)),
				      (old ? "are filthier than ever" : "get slimy"));
				return 1;
			case 1:
				if (!ublindf) {
					old = u.ucreamed;
					u.ucreamed += rn1(10, 3);
					pline("Yecch! Your %s %s gunk on it!", body_part(FACE),
					      (old ? "has more" : "now has"));
					make_blinded(Blinded + (long)u.ucreamed - old, true);
				} else {
					const char *what = (ublindf->otyp == LENSES) ?
								   "lenses" :
								   "blindfold";
					if (ublindf->cursed) {
						pline("You push your %s %s.", what,
						      rn2(2) ? "cock-eyed" : "crooked");
					} else {
						struct obj *saved_ublindf = ublindf;
						pline("You push your %s off.", what);
						Blindf_off(ublindf);
						dropx(saved_ublindf);
					}
				}
				return 1;
			case 0:
				break;
		}
	}

	if (Glib) {
		Glib = 0;
		pline("You wipe off your %s.", makeplural(body_part(HAND)));
		return 1;
	} else if (u.ucreamed) {
		Blinded -= u.ucreamed;
		u.ucreamed = 0;

		if (!Blinded) {
			pline("You've got the glop off.");
			Blinded = 1;
			make_blinded(0L, true);
		} else {
			pline("Your %s feels clean now.", body_part(FACE));
		}
		return 1;
	}

	pline("Your %s and %s are already clean.",
	      body_part(FACE), makeplural(body_part(HAND)));

	return 0;
}

/* maybe give a stethoscope message based on floor objects */
static bool its_dead(int rx, int ry, int *resp) {
	struct obj *otmp;
	struct trap *ttmp;

	if (!can_reach_floor()) return false;

	/* additional stethoscope messages from jyoung@apanix.apana.org.au */
	if (Hallucination && sobj_at(CORPSE, rx, ry)) {
		/* (a corpse doesn't retain the monster's sex,
		   so we're forced to use generic pronoun here) */
		You_hear("a voice say, \"It's dead, Jim.\"");
		*resp = 1;
		return true;
	} else if (Role_if(PM_HEALER) && ((otmp = sobj_at(CORPSE, rx, ry)) != 0 ||
					  (otmp = sobj_at(STATUE, rx, ry)) != 0)) {
		/* possibly should check uppermost {corpse,statue} in the pile
		   if both types are present, but it's not worth the effort */
		if (vobj_at(rx, ry)->otyp == STATUE) otmp = vobj_at(rx, ry);
		if (otmp->otyp == CORPSE) {
			pline("You determine that %s unfortunate being is dead.",
			      (rx == u.ux && ry == u.uy) ? "this" : "that");
		} else {
			ttmp = t_at(rx, ry);
			pline("%s appears to be in %s health for a statue.",
			      The(mons[otmp->corpsenm].mname),
			      (ttmp && ttmp->ttyp == STATUE_TRAP) ?
				      "extraordinary" :
				      "excellent");
		}
		return true;
	}
	return false;
}

static const char hollow_str[] = "a hollow sound.  This must be a secret %s!";

/* Strictly speaking it makes no sense for usage of a stethoscope to
   not take any time; however, unless it did, the stethoscope would be
   almost useless.  As a compromise, one use per turn is free, another
   uses up the turn; this makes curse status have a tangible effect. */
static int use_stethoscope(struct obj *obj) {
	static long last_used_move = -1;
	static short last_used_movement = 0;
	struct monst *mtmp;
	struct rm *lev;
	int rx, ry, res;
	boolean interference = (u.uswallow && is_whirly(u.ustuck->data) &&
				!rn2(Role_if(PM_HEALER) ? 10 : 3));

	if (nohands(youmonst.data)) {	     /* should also check for no ears and/or deaf */
		pline("You have no hands!"); /* not `body_part(HAND)' */
		return 0;
	} else if (!freehand()) {
		pline("You have no free %s.", body_part(HAND));
		return 0;
	}
	if (!getdir(NULL)) return 0;

	res = (moves == last_used_move) &&
	      (youmonst.movement == last_used_movement);
	last_used_move = moves;
	last_used_movement = youmonst.movement;

	if (u.usteed && u.dz > 0) {
		if (interference) {
			pline("%s interferes.", Monnam(u.ustuck));
			mstatusline(u.ustuck);
		} else
			mstatusline(u.usteed);
		return res;
	} else if (u.uswallow && (u.dx || u.dy || u.dz)) {
		mstatusline(u.ustuck);
		return res;
	} else if (u.uswallow && interference) {
		pline("%s interferes.", Monnam(u.ustuck));
		mstatusline(u.ustuck);
		return res;
	} else if (u.dz) {
		if (Underwater)
			You_hear("faint splashing.");
		else if (u.dz < 0 || !can_reach_floor())
			pline("You can't reach the %s.",
			      (u.dz > 0) ? surface(u.ux, u.uy) : ceiling(u.ux, u.uy));
		else if (its_dead(u.ux, u.uy, &res))
			; /* message already given */
		else if (Is_stronghold(&u.uz))
			You_hear("the crackling of hellfire.");
		else
			pline("The %s seems healthy enough.", surface(u.ux, u.uy));
		return res;
	} else if (obj->cursed && !rn2(2)) {
		You_hear("your heart beat.");
		return res;
	}
	if (Stunned || (Confusion && !rn2(5))) confdir();
	if (!u.dx && !u.dy) {
		ustatusline();
		return res;
	}
	rx = u.ux + u.dx;
	ry = u.uy + u.dy;
	if (!isok(rx, ry)) {
		You_hear("a faint typing noise.");
		return 0;
	}
	if ((mtmp = m_at(rx, ry)) != 0) {
		mstatusline(mtmp);
		if (mtmp->mundetected) {
			mtmp->mundetected = 0;
			if (cansee(rx, ry)) newsym(mtmp->mx, mtmp->my);
		}
		if (!canspotmon(mtmp))
			map_invisible(rx, ry);
		return res;
	}
	if (memory_is_invisible(rx, ry)) {
		unmap_object(rx, ry);
		newsym(rx, ry);
		pline("The invisible monster must have moved.");
	}
	lev = &levl[rx][ry];
	switch (lev->typ) {
		case SDOOR:
			You_hearf(hollow_str, "door");
			cvt_sdoor_to_door(lev); /* ->typ = DOOR */
			if (Blind)
				feel_location(rx, ry);
			else
				newsym(rx, ry);
			return res;
		case SCORR:
			You_hearf(hollow_str, "passage");
			lev->typ = CORR;
			unblock_point(rx, ry);
			if (Blind)
				feel_location(rx, ry);
			else
				newsym(rx, ry);
			return res;
	}

	if (!its_dead(rx, ry, &res))
		pline("You hear nothing special."); /* not You_hear()  */
	return res;
}

static void use_whistle(struct obj *obj) {
	if (!can_blow(&youmonst)) {
		pline("You are physically incapable of being a whistleblower.");
	} else if (Underwater) {
		pline("You blow bubbles through %s.", yname(obj));
	} else {
		pline("You produce a %s whistling sound.", obj->cursed ? "shrill" : "high");
		wake_nearby();
	}
}

static void use_magic_whistle(struct obj *obj) {
	struct monst *mtmp, *nextmon;

	if (!can_blow(&youmonst)) {
		pline("You are physically incapable of being a whistleblower.");
	} else if (obj->cursed && !rn2(2)) {
		pline("You produce a %shigh-pitched humming noise.", Underwater ? "very " : "");
		wake_nearby();
	} else {
		int pet_cnt = 0;
		pline("You produce a %s whistling sound.", Hallucination ? "normal" :
									   Underwater ? "strangely high-pitched" :
											"strange");
		for (mtmp = fmon; mtmp; mtmp = nextmon) {
			nextmon = mtmp->nmon; /* trap might kill mon */
			if (DEADMONSTER(mtmp)) continue;
			if (mtmp->mtame) {
				if (mtmp->mtrapped) {
					/* no longer in previous trap (affects mintrap) */
					mtmp->mtrapped = 0;
					fill_pit(mtmp->mx, mtmp->my);
				}
				mnexto(mtmp);
				if (canspotmon(mtmp)) ++pet_cnt;
				if (mintrap(mtmp) == 2) change_luck(-1);
			}
		}
		if (pet_cnt > 0) makeknown(obj->otyp);
	}
}

boolean um_dist(xchar x, xchar y, xchar n) {
	return abs(u.ux - x) > n || abs(u.uy - y) > n;
}

int number_leashed(void) {
	int i = 0;
	struct obj *obj;

	for (obj = invent; obj; obj = obj->nobj)
		if (obj->otyp == LEASH && obj->leashmon != 0) i++;
	return i;
}

void o_unleash(struct obj *otmp /* otmp is about to be destroyed or stolen */) {
	struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if (mtmp->m_id == (unsigned)otmp->leashmon)
			mtmp->mleashed = 0;
	otmp->leashmon = 0;
}

void m_unleash(struct monst *mtmp, boolean feedback) { /* mtmp is about to die, or become untame */
	struct obj *otmp;

	if (feedback) {
		if (canseemon(mtmp))
			pline("%s pulls free of %s leash!", Monnam(mtmp), mhis(mtmp));
		else
			pline("Your leash falls slack.");
	}
	for (otmp = invent; otmp; otmp = otmp->nobj)
		if (otmp->otyp == LEASH &&
		    otmp->leashmon == (int)mtmp->m_id)
			otmp->leashmon = 0;
	mtmp->mleashed = 0;
}

void unleash_all(void) { /* player is about to die (for bones) */
	struct obj *otmp;
	struct monst *mtmp;

	for (otmp = invent; otmp; otmp = otmp->nobj)
		if (otmp->otyp == LEASH) otmp->leashmon = 0;
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		mtmp->mleashed = 0;
}

#define MAXLEASHED 2

static void use_leash(struct obj **optr) {
	struct obj *obj = *optr;
	coord cc;
	struct monst *mtmp;
	int spotmon;

	if (!obj->leashmon && number_leashed() >= MAXLEASHED) {
		pline("You cannot leash any more pets.");
		return;
	}

	if (!get_adjacent_loc(NULL, NULL, u.ux, u.uy, &cc)) return;

	if ((cc.x == u.ux) && (cc.y == u.uy)) {
		if (u.usteed && u.dz > 0) {
			mtmp = u.usteed;
			spotmon = 1;
			goto got_target;
		}
		pline("Leash yourself?  Very funny...");
		return;
	}

	if (!(mtmp = m_at(cc.x, cc.y))) {
		pline("There is no creature there.");
		return;
	}

	spotmon = canspotmon(mtmp);
got_target:

	/* KMH, balance patch -- This doesn't work properly.
	 * Pets need extra memory for their edog structure.
	 * Normally, this is handled by tamedog(), but that
	 * rejects all demons.  Our other alternative would
	 * be to duplicate tamedog()'s functionality here.
	 * Yuck.  So I've merged it into the nymph code below.
	if (((mtmp->data == &mons[PM_SUCCUBUS]) || (mtmp->data == &mons[PM_INCUBUS]))
	     && (!mtmp->mtame) && (spotmon) && (!mtmp->mleashed)) {
	       pline("%s smiles seductively at the sight of this prop!", Monnam(mtmp));
	       mtmp->mtame = 10;
	       mtmp->mpeaceful = 1;
	       set_malign(mtmp);
	}*/
	if ((mtmp->data->mlet == S_NYMPH || mtmp->data == &mons[PM_SUCCUBUS] || mtmp->data == &mons[PM_INCUBUS]) && (spotmon) && (!mtmp->mleashed)) {
		pline("%s looks shocked! \"I'm not that way!\"", Monnam(mtmp));
		mtmp->mtame = 0;
		mtmp->mpeaceful = 0;
		mtmp->msleeping = 0;
	}
	if (!mtmp->mtame) {
		if (!spotmon)
			pline("There is no creature there.");
		else
			pline("%s %s leashed!", Monnam(mtmp), (!obj->leashmon) ? "cannot be" : "is not");
		return;
	}
	if (!obj->leashmon) {
		struct eleash eleash = {
			0,
		};
		if (mtmp->mleashed) {
			pline("This %s is already leashed.",
			      spotmon ? l_monnam(mtmp) : "monster");
			return;
		}
		pline("You slip the leash around %s%s.",
		      spotmon ? "your " : "", l_monnam(mtmp));
		mtmp->mleashed = 1;
		obj->leashmon = (int)mtmp->m_id;
		*optr = realloc_obj(obj, sizeof(eleash), (void *)&eleash,
				    obj->onamelth ? strlen(ONAME(obj)) + 1 : 0, ONAME(obj));
		mtmp->msleeping = 0;
		return;
	}
	if (obj->leashmon != (int)mtmp->m_id) {
		pline("This leash is not attached to that creature.");
		return;
	} else {
		if (obj->cursed) {
			pline("The leash would not come off!");
			obj->bknown = true;
			return;
		}
		mtmp->mleashed = 0;
		obj->leashmon = 0;
		*optr = realloc_obj(obj, 0, NULL,
				    obj->onamelth ? strlen(ONAME(obj)) + 1 : 0, ONAME(obj));
		pline("You remove the leash from %s%s.",
		      spotmon ? "your " : "", l_monnam(mtmp));
		/* KMH, balance patch -- this is okay */
		if ((mtmp->data == &mons[PM_SUCCUBUS]) ||
		    (mtmp->data == &mons[PM_INCUBUS])) {
			pline("%s is infuriated!", Monnam(mtmp));
			mtmp->mtame = 0;
			mtmp->mpeaceful = 0;
		}
	}
	return;
}

struct obj *get_mleash(struct monst *mtmp /* assuming mtmp->mleashed has been checked */) {
	struct obj *otmp;

	otmp = invent;
	while (otmp) {
		if (otmp->otyp == LEASH && otmp->leashmon == (int)mtmp->m_id)
			return otmp;
		otmp = otmp->nobj;
	}
	return NULL;
}

boolean next_to_u() {
	struct monst *mtmp;
	struct obj *otmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if (mtmp->mleashed) {
			if (distu(mtmp->mx, mtmp->my) > 2) mnexto(mtmp);
			if (distu(mtmp->mx, mtmp->my) > 2) {
				for (otmp = invent; otmp; otmp = otmp->nobj)
					if (otmp->otyp == LEASH &&
					    otmp->leashmon == (int)mtmp->m_id) {
						if (otmp->cursed) return false;
						pline("You feel %s leash go slack.",
						      (number_leashed() > 1) ? "a" : "the");
						mtmp->mleashed = 0;
						otmp->leashmon = 0;
					}
			}
		}
	}

	/* no pack mules for the Amulet */
	if (u.usteed && mon_has_amulet(u.usteed)) return false;

	return true;
}

struct leash_check {
	struct eleash *eleash;
	struct monst *pet;
	boolean you_moving; /* True if hero is the one moving */
};

static boolean check_leash_pos(void *arg, int x, int y) {
	int i, d, n, lx, ly, xx, yy;
	struct leash_check *check = (struct leash_check *)arg;
	struct eleash *eleash = check->eleash;

	if (check->you_moving) {
		if (eleash->pathlen == 2) {
			if (dist2(x, y, check->pet->mx, check->pet->my) > 2) {
				eleash->pathlen++;
				eleash->path[2] = eleash->path[1];
				eleash->path[1] = eleash->path[0];
			}
			eleash->path[0].x = x;
			eleash->path[0].y = y;
			return true;
		} else if (x == eleash->path[1].x && y == eleash->path[1].y) {
			eleash->pathlen--;
			memmove(eleash->path, eleash->path + 1,
				(eleash->pathlen - 1) * sizeof(*eleash->path));
			return true;
		} else {
			memmove(eleash->path + 1, eleash->path,
				eleash->pathlen * sizeof(*eleash->path));
			eleash->pathlen++;
			eleash->path[0].x = x;
			eleash->path[0].y = y;
		}
	} else {
		if (eleash->pathlen == 1) {
			eleash->path[1].x = x;
			eleash->path[1].y = y;
			eleash->pathlen = 2;
			return true;
		} else if (eleash->pathlen == 2) {
			if (distu(x, y) <= 2) {
				eleash->path[1].x = x;
				eleash->path[1].y = y;
				return true;
			} else {
				eleash->path[eleash->pathlen].x = x;
				eleash->path[eleash->pathlen].y = y;
				eleash->pathlen++;
			}
		} else if (x == eleash->path[eleash->pathlen - 2].x &&
			   y == eleash->path[eleash->pathlen - 2].y) {
			eleash->pathlen--;
			return true;
		} else {
			eleash->path[eleash->pathlen].x = x;
			eleash->path[eleash->pathlen].y = y;
			eleash->pathlen++;
		}
	}
	do {
		n = 0;
		for (i = 1; i < eleash->pathlen - 1; i++) {
			lx = eleash->path[i - 1].x;
			ly = eleash->path[i - 1].y;
			d = dist2(lx, ly, eleash->path[i + 1].x, eleash->path[i + 1].y);
			if (d <= 2) {
				eleash->pathlen--;
				memmove(eleash->path + i, eleash->path + i + 1,
					(eleash->pathlen - i) * sizeof(*eleash->path));
				n++;
				break;
			} else {
				xx = lx + (eleash->path[i + 1].x - lx) / 2;
				yy = ly + (eleash->path[i + 1].y - ly) / 2;
				if (ACCESSIBLE(levl[xx][yy].typ) && !closed_door(xx, yy) &&
				    (xx != eleash->path[i].x || yy != eleash->path[i].y)) {
					n++;
					eleash->path[i].x = xx;
					eleash->path[i].y = yy;
				}
			}
		}
	} while (n);
	return eleash->pathlen <= ELEASH_PSZ;
}

static void action_leash(struct obj *leash, struct monst *mon, boolean endstop) {
	long save_pacifism;

	if (leash->cursed && !breathless(mon->data)) {
		if (endstop || (mon->mhp -= rnd(2)) <= 0) {
			save_pacifism = u.uconduct.killer;
			pline("Your leash chokes %s to death!", mon_nam(mon));
			/* hero might not have intended to kill pet, but
			   that's the result of his actions; gain experience,
			   lose pacifism, take alignment and luck hit, make
			   corpse less likely to remain tame after revival */
			xkilled(mon, 0); /* no "you kill it" message */
			/* life-saving doesn't ordinarily reset this */
			if (mon->mhp > 0) u.uconduct.killer = save_pacifism;
		} else {
			pline("%s chokes on the leash!", Monnam(mon));
			/* tameness eventually drops to 1 here (never 0) */
			if (mon->mtame && rn2(mon->mtame)) mon->mtame--;
		}
	} else if (endstop) {
		pline("%s leash snaps loose!", s_suffix(Monnam(mon)));
		m_unleash(mon, false);
	} else {
		pline("You pull on the leash.");
		if (mon->data->msound != MS_SILENT)
			switch (rn2(3)) {
				case 0:
					growl(mon);
					break;
				case 1:
					yelp(mon);
					break;
				default:
					whimper(mon);
					break;
			}
	}
}

static void check_leashed_pet(struct obj *leash, struct monst *pet, boolean you_moving, boolean reset, xchar x, xchar y) {
	struct leash_check check;
	coord start, dest;

	check.eleash = ELEASH(leash);
	check.pet = pet;
	if (reset || !check.eleash->pathlen) {
		check.eleash->pathlen = 1;
		check.eleash->path[0].x = u.ux;
		check.eleash->path[0].y = u.uy;
		check.eleash->pathd2 = 0;
		check.you_moving = false;
		start.x = u.ux;
		start.y = u.uy;
		dest.x = pet->mx;
		dest.y = pet->my;
	} else {
		check.you_moving = you_moving;
		start.x = x;
		start.y = y;
		dest.x = you_moving ? u.ux : pet->mx;
		dest.y = you_moving ? u.uy : pet->my;
	}
	if (wpathto(&start, &dest, check_leash_pos, (void *)&check,
		    pet->data, check.eleash->pathlen + ELEASH_PSZ)) {
		if (check.eleash->pathlen >= ELEASH_PSZ)
			action_leash(leash, pet, check.eleash->pathlen > ELEASH_PSZ);
	} else
		action_leash(leash, pet, true);
}

/*
 * Check the leash(es) attached to mon (which might be the hero).
 * Monster's old positon is given in (x, y).
 * If reset is true then throw away the old path data and
 * choose a suitable new path (typically after teleport).
 */
void check_leash(struct monst *mon, xchar x, xchar y, boolean reset) {
	struct obj *otmp;
	struct monst *mtmp;

	if (mon == &youmonst)
		for (otmp = invent; otmp; otmp = otmp->nobj) {
			if (otmp->otyp != LEASH || otmp->leashmon == 0) continue;
			for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
				if (DEADMONSTER(mtmp)) continue;
				if ((int)mtmp->m_id == otmp->leashmon) break;
			}
			if (!mtmp) {
				impossible("leash in use isn't attached to anything?");
				otmp->leashmon = 0;
				continue;
			}
			check_leashed_pet(otmp, mtmp, true, reset, x, y);
		}
	else {
		for (otmp = invent; otmp; otmp = otmp->nobj)
			if (otmp->otyp == LEASH && (int)mon->m_id == otmp->leashmon)
				break;
		if (!otmp) {
			impossible("leashed monster isn't held by hero?");
			mon->mleashed = 0;
			return;
		}
		check_leashed_pet(otmp, mon, false, reset, x, y);
	}
}

#define WEAK 3 /* from eat.c */

static int use_mirror(struct obj *obj) {
	struct monst *mtmp;
	char mlet;
	boolean vis = !Blind && (!obj->oinvis || See_invisible);

	if (!getdir(NULL)) return 0;
	if (obj->cursed && !rn2(2)) {
		if (vis)
			pline("The mirror fogs up and doesn't reflect!");
		return 1;
	}
	if (!u.dx && !u.dy && !u.dz) {
		if (vis && !Invisible) {
			if (u.umonnum == PM_FLOATING_EYE) {
				if (!Free_action) {
					pline(Hallucination ?
						      "Yow!  The mirror stares back!" :
						      "Yikes!  You've frozen yourself!");
					nomul(-rnd((MAXULEV + 6) - u.ulevel));
					nomovemsg = 0;
				} else
					pline("You stiffen momentarily under your gaze.");
			} else if (is_vampire(youmonst.data))
				pline("You don't have a reflection.");
			else if (u.umonnum == PM_UMBER_HULK) {
				pline("Huh?  That doesn't look like you!");
				make_confused(HConfusion + d(3, 4), false);
			} else if (Hallucination)
				pline("You look %s.", hcolor(NULL));
			else if (Sick)
				pline("You look peaked.");
			else if (u.uhs >= WEAK)
				pline("You look undernourished.");
			else
				pline("You look as %s as ever.",
				      ACURR(A_CHA) > 14 ?
					      (poly_gender() == 1 ? "beautiful" : "handsome") :
					      "ugly");
		} else {
			pline("You can't see your %s %s.",
			      ACURR(A_CHA) > 14 ?
				      (poly_gender() == 1 ? "beautiful" : "handsome") :
				      "ugly",
			      body_part(FACE));
		}
		return 1;
	}
	if (u.uswallow) {
		if (vis) pline("You reflect %s %s.", s_suffix(mon_nam(u.ustuck)),
			       mbodypart(u.ustuck, STOMACH));
		return 1;
	}
	if (Underwater) {
		if (!obj->oinvis)
			pline(Hallucination ?
				      "You give the fish a chance to fix their makeup." :
				      "You reflect the murky water.");
		return 1;
	}
	if (u.dz) {
		if (vis)
			pline("You reflect the %s.",
			      (u.dz > 0) ? surface(u.ux, u.uy) : ceiling(u.ux, u.uy));
		return 1;
	}
	mtmp = bhit(u.dx, u.dy, COLNO, INVIS_BEAM,
		    (int (*)(struct monst *, struct obj *))0,
		    (int (*)(struct obj *, struct obj *))0,
		    &obj);
	if (!mtmp || !haseyes(mtmp->data))
		return 1;

	vis = canseemon(mtmp);
	mlet = mtmp->data->mlet;
	if (mtmp->msleeping) {
		if (vis)
			pline("%s is too tired to look at your mirror.",
			      Monnam(mtmp));
	} else if (!mtmp->mcansee) {
		if (vis)
			pline("%s can't see anything right now.", Monnam(mtmp));
	} else if (obj->oinvis && !perceives(mtmp->data)) {
		if (vis)
			pline("%s can't see your mirror.", Monnam(mtmp));
		/* some monsters do special things */
	} else if (is_vampire(mtmp->data) || mlet == S_GHOST) {
		if (vis)
			pline("%s doesn't have a reflection.", Monnam(mtmp));
	} else if (!mtmp->mcan && !mtmp->minvis &&
		   mtmp->data == &mons[PM_MEDUSA]) {
		if (mon_reflects(mtmp, "The gaze is reflected away by %s %s!"))
			return 1;
		if (vis)
			pline("%s is turned to stone!", Monnam(mtmp));
		stoned = true;
		killed(mtmp);
	} else if (!mtmp->mcan && !mtmp->minvis &&
		   mtmp->data == &mons[PM_FLOATING_EYE]) {
		int tmp = d((int)mtmp->m_lev, (int)mtmp->data->mattk[0].damd);
		if (!rn2(4)) tmp = 120;
		if (vis)
			pline("%s is frozen by its reflection.", Monnam(mtmp));
		else
			You_hear("something stop moving.");
		mtmp->mcanmove = 0;
		if ((int)mtmp->mfrozen + tmp > 127)
			mtmp->mfrozen = 127;
		else
			mtmp->mfrozen += tmp;
	} else if (!mtmp->mcan && !mtmp->minvis &&
		   mtmp->data == &mons[PM_UMBER_HULK]) {
		if (vis)
			pline("%s confuses itself!", Monnam(mtmp));
		mtmp->mconf = 1;
	} else if (!mtmp->mcan && !mtmp->minvis && (mlet == S_NYMPH || mtmp->data == &mons[PM_SUCCUBUS])) {
		if (vis) {
			pline("%s admires herself in your mirror.", Monnam(mtmp));
			pline("She takes it!");
		} else
			pline("It steals your mirror!");
		setnotworn(obj); /* in case mirror was wielded */
		freeinv(obj);
		mpickobj(mtmp, obj);
		if (!tele_restrict(mtmp)) (void)rloc(mtmp, false);
	} else if (!is_unicorn(mtmp->data) && !humanoid(mtmp->data) &&
		   (!mtmp->minvis || perceives(mtmp->data)) && rn2(5)) {
		if (vis)
			pline("%s is frightened by its reflection.", Monnam(mtmp));
		monflee(mtmp, d(2, 4), false, false);
	} else if (!Blind) {
		if (mtmp->minvis && !See_invisible)
			;
		else if ((mtmp->minvis && !perceives(mtmp->data)) || !haseyes(mtmp->data))
			pline("%s doesn't seem to notice its reflection.",
			      Monnam(mtmp));
		else
			pline("%s ignores %s reflection.",
			      Monnam(mtmp), mhis(mtmp));
	}
	return 1;
}

static void use_bell(struct obj **optr) {
	struct obj *obj = *optr;
	struct monst *mtmp;
	boolean wakem = false, learno = false,
		ordinary = (obj->otyp != BELL_OF_OPENING || !obj->spe),
		invoking = (obj->otyp == BELL_OF_OPENING &&
			    invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy));

	pline("You ring %s.", the(xname(obj)));

	if (Underwater || (u.uswallow && ordinary)) {
		pline("But the sound is muffled.");

	} else if (invoking && ordinary) {
		/* needs to be recharged... */
		pline("But it makes no sound.");
		learno = true; /* help player figure out why */

	} else if (ordinary) {
		if (obj->cursed && !rn2(4) &&
		    /* note: once any of them are gone, we stop all of them */
		    !(mvitals[PM_WOOD_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_WATER_NYMPH].mvflags & G_GONE) &&
		    !(mvitals[PM_MOUNTAIN_NYMPH].mvflags & G_GONE) &&
		    (mtmp = makemon(mkclass(S_NYMPH, 0),
				    u.ux, u.uy, NO_MINVENT)) != 0) {
			pline("You summon %s!", a_monnam(mtmp));
			if (!obj_resists(obj, 93, 100)) {
				pline("%s shattered!", Tobjnam(obj, "have"));
				useup(obj);
				*optr = 0;
			} else
				switch (rn2(3)) {
					default:
						break;
					case 1:
						mon_adjust_speed(mtmp, 2, NULL);
						break;
					case 2: /* no explanation; it just happens... */
						nomovemsg = "";
						nomul(-rnd(2));
						break;
				}
		}
		wakem = true;

	} else {
		/* charged Bell of Opening */
		consume_obj_charge(obj, true);

		if (u.uswallow) {
			if (!obj->cursed)
				openit();
			else
				pline("%s", "Nothing happens.");

		} else if (obj->cursed) {
			coord mm;

			mm.x = u.ux;
			mm.y = u.uy;
			mkundead(&mm, false, NO_MINVENT);
			wakem = true;

		} else if (invoking) {
			pline("%s an unsettling shrill sound...",
			      Tobjnam(obj, "issue"));
			obj->age = moves;
			learno = true;
			wakem = true;

		} else if (obj->blessed) {
			int res = 0;

			if (uchain) {
				unpunish();
				res = 1;
			}
			res += openit();
			switch (res) {
				case 0:
					pline("%s", "Nothing happens.");
					break;
				case 1:
					pline("Something opens...");
					learno = true;
					break;
				default:
					pline("Things open around you...");
					learno = true;
					break;
			}

		} else { /* uncursed */
			if (findit() != 0)
				learno = true;
			else
				pline("%s", "Nothing happens.");
		}

	} /* charged BofO */

	if (learno) {
		makeknown(BELL_OF_OPENING);
		obj->known = 1;
	}
	if (wakem) wake_nearby();
}

static void use_candelabrum(struct obj *obj) {
	const char *s = (obj->spe != 1) ? "candles" : "candle";

	if (Underwater) {
		pline("You cannot make fire under water.");
		return;
	}
	if (obj->lamplit) {
		pline("You snuff the %s.", s);
		end_burn(obj, true);
		return;
	}
	if (obj->spe <= 0) {
		pline("This %s has no %s.", xname(obj), s);
		return;
	}
	if (u.uswallow || obj->cursed) {
		if (!Blind)
			pline("The %s %s for a moment, then %s.",
			      s, vtense(s, "flicker"), vtense(s, "die"));
		return;
	}
	if (obj->spe < 7) {
		pline("There %s only %d %s in %s.",
		      vtense(s, "are"), obj->spe, s, the(xname(obj)));
		if (!Blind)
			pline("%s lit.  %s dimly.",
			      obj->spe == 1 ? "It is" : "They are",
			      Tobjnam(obj, "shine"));
	} else {
		pline("%s's %s burn%s", The(xname(obj)), s,
		      (Blind ? "." : " brightly!"));
	}
	if (!invocation_pos(u.ux, u.uy)) {
		pline("The %s %s being rapidly consumed!", s, vtense(s, "are"));
		obj->age /= 2;
	} else {
		if (obj->spe == 7) {
			if (Blind)
				pline("%s a strange warmth!", Tobjnam(obj, "radiate"));
			else
				pline("%s with a strange light!", Tobjnam(obj, "glow"));
		}
		obj->known = 1;
	}
	begin_burn(obj, false);
}

static void use_candle(struct obj **optr) {
	struct obj *obj = *optr;
	struct obj *otmp;
	const char *s = (obj->quan != 1) ? "candles" : "candle";
	char qbuf[QBUFSZ];

	if (u.uswallow) {
		pline("You don't have enough elbow-room to maneuver.");
		return;
	}
	if (Underwater) {
		pline("Sorry, fire and water don't mix.");
		return;
	}

	otmp = carrying(CANDELABRUM_OF_INVOCATION);
	/* [ALI] Artifact candles can't be attached to candelabrum
	 *       (magic candles still can be).
	 */
	if (obj->oartifact || !otmp || otmp->spe == 7) {
		use_lamp(obj);
		return;
	}

	sprintf(qbuf, "Attach %s", the(xname(obj)));
	sprintf(eos(qbuf), " to %s?",
		safe_qbuf(qbuf, sizeof(" to ?"), the(xname(otmp)),
			  the(simple_typename(otmp->otyp)), "it"));
	if (yn(qbuf) == 'n') {
		if (!obj->lamplit)
			pline("You try to light %s...", the(xname(obj)));
		use_lamp(obj);
		return;
	} else {
		if ((long)otmp->spe + obj->quan > 7L)
			obj = splitobj(obj, 7L - (long)otmp->spe);
		else
			*optr = 0;
		pline("You attach %ld%s %s to %s.",
		      obj->quan, !otmp->spe ? "" : " more",
		      s, the(xname(otmp)));
		if (obj->otyp == MAGIC_CANDLE) {
			if (obj->lamplit)
				pline("The new %s %s very ordinary.", s,
				      vtense(s, "look"));
			else
				pline("%s very ordinary.",
				      (obj->quan > 1L) ? "They look" : "It looks");
			if (!otmp->spe)
				otmp->age = 600L;
		} else if (!otmp->spe || otmp->age > obj->age)
			otmp->age = obj->age;
		otmp->spe += (int)obj->quan;
		if (otmp->lamplit && !obj->lamplit)
			pline("The new %s magically %s!", s, vtense(s, "ignite"));
		else if (!otmp->lamplit && obj->lamplit)
			pline("%s out.", (obj->quan > 1L) ? "They go" : "It goes");
		if (obj->unpaid)
			verbalize("You %s %s, you bought %s!",
				  otmp->lamplit ? "burn" : "use",
				  (obj->quan > 1L) ? "them" : "it",
				  (obj->quan > 1L) ? "them" : "it");
		if (obj->quan < 7L && otmp->spe == 7)
			pline("%s now has seven%s candles attached.",
			      The(xname(otmp)), otmp->lamplit ? " lit" : "");
		/* candelabrum's light range might increase */
		if (otmp->lamplit) obj_merge_light_sources(otmp, otmp);
		/* candles are no longer a separate light source */
		if (obj->lamplit) end_burn(obj, true);
		/* candles are now gone */
		useupall(obj);
	}
}

boolean snuff_candle(struct obj *otmp) { /* call in drop, throw, and put in box, etc. */
	boolean candle = Is_candle(otmp);

	if (((candle && otmp->oartifact != ART_CANDLE_OF_ETERNAL_FLAME) || otmp->otyp == CANDELABRUM_OF_INVOCATION) &&
	    otmp->lamplit) {
		char buf[BUFSZ];
		xchar x, y;
		boolean many = candle ? otmp->quan > 1L : otmp->spe > 1;

		get_obj_location(otmp, &x, &y, 0);
		if (otmp->where == OBJ_MINVENT ? cansee(x, y) : !Blind)
			pline("%s %scandle%s flame%s extinguished.",
			      Shk_Your(buf, otmp),
			      (candle ? "" : "candelabrum's "),
			      (many ? "s'" : "'s"), (many ? "s are" : " is"));
		end_burn(otmp, true);
		return true;
	}
	return false;
}

/* called when lit lamp is hit by water or put into a container or
   you've been swallowed by a monster; obj might be in transit while
   being thrown or dropped so don't assume that its location is valid */
boolean snuff_lit(struct obj *obj) {
	xchar x, y;

	if (obj->lamplit) {
		if (artifact_light(obj)) return false; /* Artifact lights are never snuffed */
		if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		    obj->otyp == BRASS_LANTERN || obj->otyp == POT_OIL ||
		    obj->otyp == TORCH) {
			get_obj_location(obj, &x, &y, 0);
			if (obj->where == OBJ_MINVENT ? cansee(x, y) : !Blind)
				pline("%s %s out!", Yname2(obj), otense(obj, "go"));
			end_burn(obj, true);
			return true;
		}
		if (snuff_candle(obj)) return true;
	}
	return false;
}

/* Called when potentially lightable object is affected by fire_damage().
   Return true if object was lit and false otherwise --ALI */
boolean catch_lit(struct obj *obj) {
	xchar x, y;

	if (!obj->lamplit && (obj->otyp == MAGIC_LAMP || ignitable(obj))) {
		if ((obj->otyp == MAGIC_LAMP ||
		     obj->otyp == CANDELABRUM_OF_INVOCATION) &&
		    obj->spe == 0)
			return false;
		else if (obj->otyp != MAGIC_LAMP && obj->age == 0)
			return false;
		if (!get_obj_location(obj, &x, &y, 0))
			return false;
		if (obj->otyp == CANDELABRUM_OF_INVOCATION && obj->cursed)
			return false;
		if ((obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		     obj->otyp == BRASS_LANTERN) &&
		    obj->cursed && !rn2(2))
			return false;
		if (obj->where == OBJ_MINVENT ? cansee(x, y) : !Blind)
			pline("%s %s light!", Yname2(obj), otense(obj, "catch"));
		if (obj->otyp == POT_OIL) makeknown(obj->otyp);
		if (obj->unpaid && costly_spot(u.ux, u.uy) && (obj->where == OBJ_INVENT)) {
			/* if it catches while you have it, then it's your tough luck */
			check_unpaid(obj);
			verbalize("That's in addition to the cost of %s %s, of course.",
				  Yname2(obj), obj->quan == 1 ? "itself" : "themselves");
			bill_dummy_object(obj);
		}
		begin_burn(obj, false);
		return true;
	}
	return false;
}

static void use_lamp(struct obj *obj) {
	char buf[BUFSZ];
	char qbuf[QBUFSZ];

	if (Underwater) {
		pline("This is not a diving lamp.");
		return;
	}
	if (obj->lamplit) {
		if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		    obj->otyp == BRASS_LANTERN) {
			pline("%s lamp is now off.", Shk_Your(buf, obj));
		} else if (is_lightsaber(obj)) {
			if (obj->otyp == RED_DOUBLE_LIGHTSABER) {
				/* Do we want to activate dual bladed mode? */
				if (!obj->altmode && (!obj->cursed || rn2(4))) {
					pline("You ignite the second blade of %s.", yname(obj));
					obj->altmode = true;
					return;
				} else
					obj->altmode = false;
			}
			lightsaber_deactivate(obj, true);
			return;
		} else if (artifact_light(obj)) {
			pline("You can't snuff out %s.", yname(obj));
			return;
		} else {
			pline("You snuff out %s.", yname(obj));
		}
		end_burn(obj, true);
		return;
	}
	/* magic lamps with an spe == 0 (wished for) cannot be lit */
	if ((!Is_candle(obj) && obj->age == 0) || (obj->otyp == MAGIC_LAMP && obj->spe == 0)) {
		if ((obj->otyp == BRASS_LANTERN) || is_lightsaber(obj))
			pline("Your %s has run out of power.", xname(obj));
		else if (obj->otyp == TORCH) {
			pline("Your torch has burnt out and cannot be relit.");
		} else
			pline("This %s has no oil.", xname(obj));
		return;
	}
	if (obj->cursed && !rn2(2)) {
		pline("%s for a moment, then %s.",
		      Tobjnam(obj, "flicker"), otense(obj, "die"));
	} else {
		if (obj->otyp == OIL_LAMP || obj->otyp == MAGIC_LAMP ||
		    obj->otyp == BRASS_LANTERN) {
			check_unpaid(obj);
			pline("%s lamp is now on.", Shk_Your(buf, obj));
		} else if (obj->otyp == TORCH) {
			check_unpaid(obj);
			pline("%s flame%s burn%s%s",
			      s_suffix(Yname2(obj)),
			      plur(obj->quan),
			      obj->quan > 1L ? "" : "s",
			      Blind ? "." : " brightly!");
		} else if (is_lightsaber(obj)) {
			/* WAC -- lightsabers */
			/* you can see the color of the blade */

			if (!Blind) makeknown(obj->otyp);
			pline("You ignite %s.", yname(obj));
			unweapon = false;
		} else { /* candle(s) */
			sprintf(qbuf, "Light all of %s?", the(xname(obj)));
			if (obj->quan > 1L && (yn(qbuf) == 'n')) {
				/* Check if player wants to light all the candles */
				struct obj *rest; /* the remaining candles */
				rest = splitobj(obj, obj->quan - 1L);
				obj_extract_self(rest); /* free from inv */
				obj->spe++;		/* this prevents merging */
				hold_another_object(rest, "You drop %s!",
						    doname(rest), NULL);
				obj->spe--;
			}
			pline("%s flame%s %s%s",
			      s_suffix(Yname2(obj)),
			      plur(obj->quan), otense(obj, "burn"),
			      Blind ? "." : " brightly!");
			if (obj->unpaid && costly_spot(u.ux, u.uy) &&
			    obj->otyp != MAGIC_CANDLE) {
				const char *ithem = obj->quan > 1L ? "them" : "it";
				verbalize("You burn %s, you bought %s!", ithem, ithem);
				bill_dummy_object(obj);
			}
		}
		begin_burn(obj, false);
	}
}

/* MRKR: Torches */

static int use_torch(struct obj *obj) {
	struct obj *otmp = NULL;
	if (u.uswallow) {
		pline("You don't have enough elbow-room to maneuver.");
		return 0;
	}
	if (Underwater) {
		pline("Sorry, fire and water don't mix.");
		return 0;
	}
	if (obj->quan > 1L) {
		if (obj == uwep && welded(obj)) {
			pline("You can only hold one lit torch, but can't drop any to hold only one.");
			return 0;
		}
		otmp = obj;
		obj = splitobj(otmp, 1L);
		obj_extract_self(otmp); /* free from inv */
	}
	/* You can use a torch in either wielded weapon slot */
	if (obj != uwep && (obj != uswapwep || !u.twoweap))
		if (!wield_tool(obj, NULL)) return 0;
	use_lamp(obj);
	/* shouldn't merge */
	if (otmp)
		otmp = hold_another_object(otmp, "You drop %s!",
					   doname(otmp), NULL);
	return 1;
}

static void light_cocktail(struct obj *obj /* obj is a potion of oil or a stick of dynamite */) {
	char buf[BUFSZ];
	const char *objnam = obj->otyp == POT_OIL ? "potion" : "stick";

	if (u.uswallow) {
		pline("You don't have enough elbow-room to maneuver.");
		return;
	}

	if (Underwater) {
		pline("You can't light this underwater!");
		return;
	}

	if (obj->lamplit) {
		pline("You snuff the lit %s.", objnam);
		end_burn(obj, true);
		/*
		 * Free & add to re-merge potion.  This will average the
		 * age of the potions.  Not exactly the best solution,
		 * but its easy.
		 */
		freeinv(obj);
		addinv(obj);
		return;
	} else if (Underwater) {
		pline("There is not enough oxygen to sustain a fire.");
		return;
	}

	pline("You light %s %s.%s", shk_your(buf, obj), objnam,
	      Blind ? "" : "  It gives off a dim light.");
	if (obj->unpaid && costly_spot(u.ux, u.uy)) {
		/* Normally, we shouldn't both partially and fully charge
		 * for an item, but (Yendorian Fuel) Taxes are inevitable...
		 */
		if (obj->otyp != STICK_OF_DYNAMITE) {
			check_unpaid(obj);
			verbalize("That's in addition to the cost of the potion, of course.");
		} else {
			const char *ithem = obj->quan > 1L ? "them" : "it";
			verbalize("You burn %s, you bought %s!", ithem, ithem);
		}
		bill_dummy_object(obj);
	}
	makeknown(obj->otyp);

	if (obj->otyp == STICK_OF_DYNAMITE)
		obj->yours = true;

	if (obj->quan > 1L) {
		obj = splitobj(obj, 1L);
		begin_burn(obj, false); /* burn before free to get position */
		obj_extract_self(obj);	/* free from inv */

		/* shouldn't merge */
		obj = hold_another_object(obj, "You drop %s!",
					  doname(obj), NULL);
	} else
		begin_burn(obj, false);
}

static const char cuddly[] = {TOOL_CLASS, GEM_CLASS, 0};

int dorub(void) {
	struct obj *obj = getobj(cuddly, "rub");

	if (obj && obj->oclass == GEM_CLASS) {
		if (is_graystone(obj)) {
			use_stone(obj);
			return 1;
		} else {
			pline("Sorry, I don't know how to use that.");
			return 0;
		}
	}

	if (!obj || !wield_tool(obj, "rub")) return 0;

	/* now uwep is obj */
	if (uwep->otyp == MAGIC_LAMP) {
		if (uwep->spe > 0 && !rn2(3)) {
			check_unpaid_usage(uwep, true); /* unusual item use */
			djinni_from_bottle(uwep);
			makeknown(MAGIC_LAMP);
			uwep->otyp = OIL_LAMP;
			uwep->spe = 0; /* for safety */
			uwep->age = rn1(500, 1000);
			if (uwep->lamplit) begin_burn(uwep, true);
			update_inventory();
		} else if (rn2(2) && !Blind)
			pline("You see a puff of smoke.");
		else
			pline("%s", "Nothing happens.");
	} else if (obj->otyp == BRASS_LANTERN) {
		/* message from Adventure */
		pline("Rubbing the electric lamp is not particularly rewarding.");
		pline("Anyway, nothing exciting happens.");
	} else
		pline("%s", "Nothing happens.");
	return 1;
}

int dojump(void) {
	/* Physical jump */
	return jump(0);
}

int jump(int magic /* 0=Physical, otherwise skill level */) {
	coord cc;

	if (!magic && (nolimbs(youmonst.data) || slithy(youmonst.data))) {
		/* normally (nolimbs || slithy) implies !Jumping,
		   but that isn't necessarily the case for knights */
		pline("You can't jump; you have no legs!");
		return 0;
	} else if (!magic && !Jumping) {
		pline("You can't jump very far.");
		return 0;
	} else if (u.uswallow) {
		if (magic) {
			pline("You bounce around a little.");
			return 1;
		} else {
			pline("You've got to be kidding!");
			return 0;
		}
		return 0;
	} else if (u.uinwater) {
		if (magic) {
			pline("You swish around a little.");
			return 1;
		} else {
			pline("This calls for swimming, not jumping!");
			return 0;
		}
		return 0;
	} else if (u.ustuck) {
		if (u.ustuck->mtame && !Conflict && !u.ustuck->mconf) {
			pline("You pull free from %s.", mon_nam(u.ustuck));
			setustuck(0);
			return 1;
		}
		if (magic) {
			pline("You writhe a little in the grasp of %s!", mon_nam(u.ustuck));
			return 1;
		} else {
			pline("You cannot escape from %s!", mon_nam(u.ustuck));
			return 0;
		}

		return 0;
	} else if (Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
		if (magic) {
			pline("You flail around a little.");
			return 1;
		} else {
			pline("You don't have enough traction to jump.");
			return 0;
		}
	} else if (!magic && near_capacity() > UNENCUMBERED) {
		pline("You are carrying too much to jump!");
		return 0;
	} else if (!magic && (u.uhunger <= 100 || ACURR(A_STR) < 6)) {
		pline("You lack the strength to jump!");
		return 0;
	} else if (Wounded_legs) {
		long wl = (EWounded_legs & BOTH_SIDES);
		const char *bp = body_part(LEG);

		if (wl == BOTH_SIDES) bp = makeplural(bp);

		if (u.usteed)
			pline("%s is in no shape for jumping.", Monnam(u.usteed));
		else
			pline("Your %s%s %s in no shape for jumping.",
			      (wl == LEFT_SIDE) ? "left " :
						  (wl == RIGHT_SIDE) ? "right " : "",
			      bp, (wl == BOTH_SIDES) ? "are" : "is");
		return 0;
	} else if (u.usteed && u.utrap) {
		pline("%s is stuck in a trap.", Monnam(u.usteed));
		return 0;
	}

	pline("Where do you want to jump?");
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, true, "the desired position") < 0)
		return 0; /* user pressed ESC */
	if (!magic && !(HJumping & ~INTRINSIC) && !EJumping &&
	    distu(cc.x, cc.y) != 5) {
		/* The Knight jumping restriction still applies when riding a
		 * horse.  After all, what shape is the knight piece in chess?
		 */
		pline("Illegal move!");
		return 0;
	} else if (distu(cc.x, cc.y) > (magic ? 6 + magic * 3 : 9)) {
		pline("Too far!");
		return 0;
	} else if (!cansee(cc.x, cc.y)) {
		pline("You cannot see where to land!");
		return 0;
	} else if (!isok(cc.x, cc.y)) {
		pline("You cannot jump there!");
		return 0;
	} else {
		coord uc;
		int range, temp;

		if (u.utrap)
			switch (u.utraptype) {
				case TT_BEARTRAP: {
					long side = rn2(3) ? LEFT_SIDE : RIGHT_SIDE;
					pline("You rip yourself free of the bear trap!  Ouch!");
					if (!u.usteed)
						losehp(rnd(10), "jumping out of a bear trap", KILLED_BY);
					set_wounded_legs(side, rn1(1000, 500));
					break;
				}
				case TT_PIT:
					pline("You leap from the pit!");
					break;
				case TT_WEB:
					pline("You tear the web apart as you pull yourself free!");
					deltrap(t_at(u.ux, u.uy));
					break;
				case TT_LAVA:
					pline("You pull yourself above the lava!");
					u.utrap = 0;
					return 1;
				case TT_INFLOOR:
					pline("You strain your %s, but you're still stuck in the floor.",
					      makeplural(body_part(LEG)));
					set_wounded_legs(LEFT_SIDE, rn1(10, 11));
					set_wounded_legs(RIGHT_SIDE, rn1(10, 11));
					return 1;
			}

		/*
		 * Check the path from uc to cc, calling hurtle_step at each
		 * location.  The final position actually reached will be
		 * in cc.
		 */
		uc.x = u.ux;
		uc.y = u.uy;
		/* calculate max(abs(dx), abs(dy)) as the range */
		range = cc.x - uc.x;
		if (range < 0) range = -range;
		temp = cc.y - uc.y;
		if (temp < 0) temp = -temp;
		if (range < temp)
			range = temp;
		walk_path(&uc, &cc, hurtle_step, (void *)&range);

		/* A little Sokoban guilt... */
		if (In_sokoban(&u.uz))
			change_luck(-1);

		teleds(cc.x, cc.y, true);
		nomul(-1);
		nomovemsg = "";
		morehungry(rnd(25));
		return 1;
	}
}

boolean tinnable(struct obj *corpse) {
	if (corpse->otyp != CORPSE) return 0;
	if (corpse->oeaten) return 0;
	if (corpse->odrained) return 0;
	if (!mons[corpse->corpsenm].cnutrit) return 0;
	return 1;
}

static void use_tinning_kit(struct obj *obj) {
	struct obj *corpse, *can;
	/*
		char *badmove;
	 */
	/* This takes only 1 move.  If this is to be changed to take many
	 * moves, we've got to deal with decaying corpses...
	 */
	if (obj->spe <= 0) {
		pline("You seem to be out of tins.");
		return;
	}
	if (!(corpse = getobj((const char *)tinnables, "tin"))) return;
	if (corpse->otyp == CORPSE && (corpse->oeaten || corpse->odrained)) {
		pline("You cannot tin something which is partly eaten.");
		return;
	}
	if (!tinnable(corpse)) {
		pline("You can't tin that!");
		return;
	}
	if (touch_petrifies(&mons[corpse->corpsenm]) && !Stone_resistance && !uarmg) {
		char kbuf[BUFSZ];

		if (poly_when_stoned(youmonst.data))
			pline("You tin %s without wearing gloves.",
			      an(mons[corpse->corpsenm].mname));
		else {
			pline("Tinning %s without wearing gloves is a fatal mistake...",
			      an(mons[corpse->corpsenm].mname));
			sprintf(kbuf, "trying to tin %s without gloves",
				an(mons[corpse->corpsenm].mname));
		}
		instapetrify(kbuf);
	}
	if (is_rider(&mons[corpse->corpsenm])) {
		revive_corpse(corpse, false);
		verbalize("Yes...  But War does not preserve its enemies...");
		return;
	}
	if (mons[corpse->corpsenm].cnutrit == 0) {
		pline("That's too insubstantial to tin.");
		return;
	}
	consume_obj_charge(obj, true);

	if ((can = mksobj(TIN, false, false)) != 0) {
		static const char you_buy_it[] = "You tin it, you bought it!";

		can->corpsenm = corpse->corpsenm;
		can->cursed = obj->cursed;
		can->blessed = obj->blessed;
		can->owt = weight(can);
		can->known = 1;

		/* WAC You know the type of tinned corpses */
		if (mvitals[corpse->corpsenm].eaten < 255)
			mvitals[corpse->corpsenm].eaten++;

		can->spe = -1; /* Mark tinned tins. No spinach allowed... */
		if (carried(corpse)) {
			if (corpse->unpaid)
				verbalize(you_buy_it);
			useup(corpse);
		} else if (mcarried(corpse)) {
			m_useup(corpse->ocarry, corpse);
		} else {
			if (costly_spot(corpse->ox, corpse->oy) && !corpse->no_charge)
				verbalize(you_buy_it);
			useupf(corpse, 1L);
		}
		can = hold_another_object(can, "You make, but cannot pick up, %s.",
					  doname(can), NULL);
	} else
		impossible("Tinning failed.");
}

void use_unicorn_horn(struct obj *obj) {
#define PROP_COUNT 6	       /* number of properties we're dealing with */
#define ATTR_COUNT (A_MAX * 3) /* number of attribute points we might fix */
	int idx, val, val_limit,
		trouble_count, unfixable_trbl, did_prop, did_attr;
	int trouble_list[PROP_COUNT + ATTR_COUNT];
	int chance; /* KMH */

	if (obj && obj->cursed) {
		long lcount = (long)rnd(100);

		switch (rn2(6)) {
			case 0:
				make_sick(Sick ? Sick / 3L + 1L : (long)rn1(ACURR(A_CON), 20),
					  xname(obj), true, SICK_NONVOMITABLE);
				break;
			case 1:
				make_blinded(Blinded + lcount, true);
				break;
			case 2:
				if (!Confusion)
					pline("You suddenly feel %s.",
					      Hallucination ? "trippy" : "confused");
				make_confused(HConfusion + lcount, true);
				break;
			case 3:
				make_stunned(HStun + lcount, true);
				break;
			case 4:
				(void)adjattrib(rn2(A_MAX), -1, false);
				break;
			case 5:
				(void)make_hallucinated(HHallucination + lcount, true, 0L);
				break;
		}
		return;
	}

	/*
	 * Entries in the trouble list use a very simple encoding scheme.
	 */
#define prop2trbl(X)	((X) + A_MAX)
#define attr2trbl(Y)	(Y)
#define prop_trouble(X) trouble_list[trouble_count++] = prop2trbl(X)
#define attr_trouble(Y) trouble_list[trouble_count++] = attr2trbl(Y)

	trouble_count = unfixable_trbl = did_prop = did_attr = 0;

	/* collect property troubles */
	if (Sick) prop_trouble(SICK);
	if (Blinded > (long)u.ucreamed) prop_trouble(BLINDED);
	if (HHallucination) prop_trouble(HALLUC);
	if (Vomiting) prop_trouble(VOMITING);
	if (HConfusion) prop_trouble(CONFUSION);
	if (HStun) prop_trouble(STUNNED);

	unfixable_trbl = unfixable_trouble_count(true);

	/* collect attribute troubles */
	for (idx = 0; idx < A_MAX; idx++) {
		val_limit = AMAX(idx);
		/* don't recover strength lost from hunger */
		if (idx == A_STR && u.uhs >= WEAK) val_limit--;
		/* don't recover more than 3 points worth of any attribute */
		if (val_limit > ABASE(idx) + 3) val_limit = ABASE(idx) + 3;

		for (val = ABASE(idx); val < val_limit; val++)
			attr_trouble(idx);
		/* keep track of unfixed trouble, for message adjustment below */
		unfixable_trbl += (AMAX(idx) - val_limit);
	}

	if (trouble_count == 0) {
		pline("%s", "Nothing happens.");
		return;
	} else if (trouble_count > 1) { /* shuffle */
		int i, j, k;

		for (i = trouble_count - 1; i > 0; i--)
			if ((j = rn2(i + 1)) != i) {
				k = trouble_list[j];
				trouble_list[j] = trouble_list[i];
				trouble_list[i] = k;
			}
	}

#if 0 /* Old NetHack success rate */
	/*
	 *		Chances for number of troubles to be fixed
	 *		 0	1      2      3      4	    5	   6	  7
	 *   blessed:  22.7%  22.7%  19.5%  15.4%  10.7%   5.7%   2.6%	 0.8%
	 *  uncursed:  35.4%  35.4%  22.9%   6.3%    0	    0	   0	  0
	 */
	val_limit = rn2( d(2, (obj && obj->blessed) ? 4 : 2) );
	if (val_limit > trouble_count) val_limit = trouble_count;
#else /* KMH's new success rate */
	/*
	 * blessed:  Tries all problems, each with chance given below.
	 * uncursed: Tries one problem, with chance given below.
	 * ENCHANT  +0 or less  +1   +2   +3   +4   +5   +6 or more
	 * CHANCE       30%     40%  50%  60%  70%  80%     90%
	 */
	val_limit = (obj && obj->blessed) ? trouble_count : 1;
	if (obj && obj->spe > 0)
		chance = (obj->spe < 6) ? obj->spe + 3 : 9;
	else
		chance = 3;
#endif

	/* fix [some of] the troubles */
	for (val = 0; val < val_limit; val++) {
		idx = trouble_list[val];

		if (rn2(10) < chance) /* KMH */
			switch (idx) {
				case prop2trbl(SICK):
					make_sick(0L, NULL, true, SICK_ALL);
					did_prop++;
					break;
				case prop2trbl(BLINDED):
					make_blinded((long)u.ucreamed, true);
					did_prop++;
					break;
				case prop2trbl(HALLUC):
					make_hallucinated(0L, true, 0L);
					did_prop++;
					break;
				case prop2trbl(VOMITING):
					make_vomiting(0L, true);
					did_prop++;
					break;
				case prop2trbl(CONFUSION):
					make_confused(0L, true);
					did_prop++;
					break;
				case prop2trbl(STUNNED):
					make_stunned(0L, true);
					did_prop++;
					break;
				default:
					if (idx >= 0 && idx < A_MAX) {
						ABASE(idx) += 1;
						did_attr++;
					} else
						panic("use_unicorn_horn: bad trouble? (%d)", idx);
					break;
			}
	}

	if (did_attr)
		pline("This makes you feel %s!",
		      (did_prop + did_attr) == (trouble_count + unfixable_trbl) ?
			      "great" :
			      "better");
	else if (!did_prop)
		pline("Nothing seems to happen.");

	flags.botl = (did_attr || did_prop);
#undef PROP_COUNT
#undef ATTR_COUNT
#undef prop2trbl
#undef attr2trbl
#undef prop_trouble
#undef attr_trouble
}

/*
 * Timer callback routine: turn figurine into monster
 */
void fig_transform(void *arg, long timeout) {
	struct obj *figurine = (struct obj *)arg;
	struct monst *mtmp;
	coord cc;
	boolean cansee_spot, silent, okay_spot;
	boolean redraw = false;
	char monnambuf[BUFSZ], carriedby[BUFSZ];

	if (!figurine) {
#ifdef DEBUG
		pline("null figurine in fig_transform()");
#endif
		return;
	}
	silent = (timeout != monstermoves); /* happened while away */
	okay_spot = get_obj_location(figurine, &cc.x, &cc.y, 0);
	if (figurine->where == OBJ_INVENT ||
	    figurine->where == OBJ_MINVENT)
		okay_spot = enexto(&cc, cc.x, cc.y,
				   &mons[figurine->corpsenm]);
	if (!okay_spot ||
	    !figurine_location_checks(figurine, &cc, true)) {
		/* reset the timer to try again later */
		start_timer(rnd(5000), TIMER_OBJECT, FIG_TRANSFORM, obj_to_any(figurine));
		return;
	}

	cansee_spot = cansee(cc.x, cc.y);
	mtmp = make_familiar(figurine, cc.x, cc.y, true);
	if (mtmp) {
		sprintf(monnambuf, "%s", an(m_monnam(mtmp)));
		switch (figurine->where) {
			case OBJ_INVENT:
				if (Blind)
					pline("You feel something %s from your pack!", locomotion(mtmp->data, "drop"));
				else
					pline("You see %s %s out of your pack!",
					      monnambuf,
					      locomotion(mtmp->data, "drop"));
				break;

			case OBJ_FLOOR:
				if (cansee_spot && !silent) {
					pline("You suddenly see a figurine transform into %s!",
					      monnambuf);
					redraw = true; /* update figurine's map location */
				}
				break;

			case OBJ_MINVENT:
				if (cansee_spot && !silent) {
					struct monst *mon;
					mon = figurine->ocarry;
					/* figurine carring monster might be invisible */
					if (canseemon(figurine->ocarry)) {
						sprintf(carriedby, "%s pack",
							s_suffix(a_monnam(mon)));
					} else if (is_pool(mon->mx, mon->my))
						strcpy(carriedby, "empty water");
					else
						strcpy(carriedby, "thin air");
					pline("You see %s %s out of %s!", monnambuf,
					      locomotion(mtmp->data, "drop"), carriedby);
				}
				break;
#if 0
		case OBJ_MIGRATING:
			break;
#endif

			default:
				impossible("figurine came to life where? (%d)",
					   (int)figurine->where);
				break;
		}
	}
	/* free figurine now */
	obj_extract_self(figurine);
	obfree(figurine, NULL);
	if (redraw) newsym(cc.x, cc.y);
}

static boolean figurine_location_checks(struct obj *obj, coord *cc, boolean quietly) {
	xchar x, y;

	if (carried(obj) && u.uswallow) {
		if (!quietly)
			pline("You don't have enough room in here.");
		return false;
	}
	x = cc->x;
	y = cc->y;
	if (!isok(x, y)) {
		if (!quietly)
			pline("You cannot put the figurine there.");
		return false;
	}
	if (IS_ROCK(levl[x][y].typ) &&
	    !(passes_walls(&mons[obj->corpsenm]) && may_passwall(x, y))) {
		if (!quietly)
			pline("You cannot place a figurine in %s!",
			      IS_TREE(levl[x][y].typ) ? "a tree" : "solid rock");
		return false;
	}
	if (sobj_at(BOULDER, x, y) && !passes_walls(&mons[obj->corpsenm]) && !throws_rocks(&mons[obj->corpsenm])) {
		if (!quietly)
			pline("You cannot fit the figurine on the boulder.");
		return false;
	}
	return true;
}

static void use_figurine(struct obj **optr) {
	struct obj *obj = *optr;
	xchar x, y;
	coord cc;

	if (u.uswallow) {
		/* can't activate a figurine while swallowed */
		if (!figurine_location_checks(obj, NULL, false))
			return;
	}
	if (!getdir(NULL)) {
		flags.move = false;
		multi = 0;
		return;
	}
	x = u.ux + u.dx;
	y = u.uy + u.dy;
	cc.x = x;
	cc.y = y;
	/* Passing false arg here will result in messages displayed */
	if (!figurine_location_checks(obj, &cc, false)) return;
	pline("You %s and it transforms.",
	      (u.dx || u.dy) ? "set the figurine beside you" :
			       (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ||
				is_pool(cc.x, cc.y)) ?
			       "release the figurine" :
			       (u.dz < 0 ?
					"toss the figurine into the air" :
					"set the figurine on the ground"));
	make_familiar(obj, cc.x, cc.y, false);
	stop_timer(FIG_TRANSFORM, obj_to_any(obj));
	useup(obj);
	*optr = 0;
}

static const char lubricables[] = {ALL_CLASSES, ALLOW_NONE, 0};
static void use_grease(struct obj *obj) {
	struct obj *otmp;
	char buf[BUFSZ];

	if (Glib) {
		pline("%s from your %s.", Tobjnam(obj, "slip"),
		      makeplural(body_part(FINGER)));
		dropx(obj);
		return;
	}

	if (obj->spe > 0) {
		if ((obj->cursed || Fumbling) && !rn2(2)) {
			consume_obj_charge(obj, true);

			pline("%s from your %s.", Tobjnam(obj, "slip"),
			      makeplural(body_part(FINGER)));
			dropx(obj);
			return;
		}
		otmp = getobj(lubricables, "grease");
		if (!otmp) return;
		if ((otmp->owornmask & WORN_ARMOR) && uarmc) {
			strcpy(buf, xname(uarmc));
			pline("You need to remove your %s to grease your %s.", buf, xname(otmp));
			return;
		}
		if ((otmp->owornmask & WORN_SHIRT) && (uarmc || uarm)) {
			strcpy(buf, uarmc ? xname(uarmc) : "");
			if (uarmc && uarm) strcat(buf, " and ");
			strcat(buf, uarm ? xname(uarm) : "");
			pline("You need to remove your %s to grease your %s.", buf, xname(otmp));
			return;
		}
		consume_obj_charge(obj, true);

		if (otmp != &zeroobj) {
			pline("You cover %s with a thick layer of grease.",
			      yname(otmp));
			otmp->greased = 1;
			if (obj->cursed && !nohands(youmonst.data)) {
				incr_itimeout(&Glib, rnd(15));
				pline("Some of the grease gets all over your %s.",
				      makeplural(body_part(HAND)));
			}
		} else {
			Glib += rnd(15);
			pline("You coat your %s with grease.",
			      makeplural(body_part(FINGER)));
		}
	} else {
		if (obj->known)
			pline("%s empty.", Tobjnam(obj, "are"));
		else
			pline("%s to be empty.", Tobjnam(obj, "seem"));
	}
	update_inventory();
}

static struct trapinfo {
	struct obj *tobj;
	xchar tx, ty;
	int time_needed;
	boolean force_bungle;
} trapinfo;

void reset_trapset(void) {
	trapinfo.tobj = 0;
	trapinfo.force_bungle = 0;
}

static struct whetstoneinfo {
	struct obj *tobj, *wsobj;
	int time_needed;
} whetstoneinfo;

void reset_whetstone(void) {
	whetstoneinfo.tobj = 0;
	whetstoneinfo.wsobj = 0;
}

/* occupation callback */
static int set_whetstone(void) {
	struct obj *otmp = whetstoneinfo.tobj, *ows = whetstoneinfo.wsobj;
	int chance;

	if (!otmp || !ows) {
		reset_whetstone();
		return 0;
	} else if (!carried(otmp) || !carried(ows)) {
		pline("You seem to have mislaid %s.",
		      !carried(otmp) ? yname(otmp) : yname(ows));
		reset_whetstone();
		return 0;
	}

	if (--whetstoneinfo.time_needed > 0) {
		int adj = 2;
		if (Blind) adj--;
		if (Fumbling) adj--;
		if (Confusion) adj--;
		if (Stunned) adj--;
		if (Hallucination) adj--;
		if (adj > 0)
			whetstoneinfo.time_needed -= adj;
		return 1;
	}

	chance = 4 - (ows->blessed) + (ows->cursed * 2) + (otmp->oartifact ? 3 : 0);

	if (!rn2(chance) && (ows->otyp == WHETSTONE)) {
		/* Remove rust first, then sharpen dull edges */
		if (otmp->oeroded) {
			otmp->oeroded--;
			pline("%s %s%s now.", Yname2(otmp),
			      (Blind ? "probably " : (otmp->oeroded ? "almost " : "")),
			      otense(otmp, "shine"));
		} else if (otmp->spe < 0) {
			otmp->spe++;
			pline("%s %s %ssharper now.%s", Yname2(otmp),
			      otense(otmp, Blind ? "feel" : "look"),
			      (otmp->spe >= 0 ? "much " : ""),
			      Blind ? "  (Ow!)" : "");
		}
		makeknown(WHETSTONE);
		reset_whetstone();
	} else {
		if (Hallucination)
			pline("%s %s must be faulty!",
			      is_plural(ows) ? "These" : "This", xname(ows));
		else
			pline("%s", Blind ? "Pheww!  This is hard work!" :
					    "There are no visible effects despite your efforts.");
		reset_whetstone();
	}

	return 0;
}

/* use stone on obj. the stone doesn't necessarily need to be a whetstone. */
static void use_whetstone(struct obj *stone, struct obj *obj) {
	boolean fail_use = true;
	const char *occutext = "sharpening";
	int tmptime = 130 + (rnl(13) * 5);

	if (u.ustuck && sticks(youmonst.data)) {
		pline("You should let go of %s first.", mon_nam(u.ustuck));
	} else if ((welded(uwep) && (uwep != stone)) ||
		   (uswapwep && u.twoweap && welded(uswapwep) && (uswapwep != obj))) {
		pline("You need both hands free.");
	} else if (nohands(youmonst.data)) {
		pline("You can't handle %s with your %s.",
		      an(xname(stone)), makeplural(body_part(HAND)));
	} else if (verysmall(youmonst.data)) {
		pline("You are too small to use %s effectively.", an(xname(stone)));
	} else if (!is_pool(u.ux, u.uy) && !IS_FOUNTAIN(levl[u.ux][u.uy].typ) && !IS_SINK(levl[u.ux][u.uy].typ) && !IS_TOILET(levl[u.ux][u.uy].typ)) {
		if (carrying(POT_WATER) && objects[POT_WATER].oc_name_known) {
			pline("Better not waste bottled water for that.");
		} else
			pline("You need some water when you use that.");
	} else if (Levitation && !Lev_at_will && !u.uinwater) {
		pline("You can't reach the water.");
	} else
		fail_use = false;

	if (fail_use) {
		reset_whetstone();
		return;
	}

	if (stone == whetstoneinfo.wsobj && obj == whetstoneinfo.tobj &&
	    carried(obj) && carried(stone)) {
		pline("You resume %s %s.", occutext, yname(obj));
		set_occupation(set_whetstone, occutext, 0);
		return;
	}

	if (obj) {
		int ttyp = obj->otyp;
		boolean isweapon = (obj->oclass == WEAPON_CLASS || is_weptool(obj));
		boolean isedged = (is_pick(obj) ||
				   (objects[ttyp].oc_dir & (PIERCE | SLASH)));
		if (obj == &zeroobj) {
			pline("You file your nails.");
		} else if (!isweapon || !isedged) {
			pline("%s sharp enough already.",
			      is_plural(obj) ? "They are" : "It is");
		} else if (stone->quan > 1) {
			pline("Using one %s is easier.", singular(stone, xname));
		} else if (obj->quan > 1) {
			pline("You can apply %s only on one %s at a time.",
			      the(xname(stone)),
			      (obj->oclass == WEAPON_CLASS ? "weapon" : "item"));
		} else if (!is_metallic(obj)) {
			pline("That would ruin the %s %s.",
			      materialnm[objects[ttyp].oc_material],
			      xname(obj));
		} else if (((obj->spe >= 0) || !obj->known) && !obj->oeroded) {
			pline("%s %s sharp and pointy enough.",
			      is_plural(obj) ? "They" : "It",
			      otense(obj, Blind ? "feel" : "look"));
		} else {
			if (stone->cursed) tmptime *= 2;
			whetstoneinfo.time_needed = tmptime;
			whetstoneinfo.tobj = obj;
			whetstoneinfo.wsobj = stone;
			pline("You start %s %s.", occutext, yname(obj));
			set_occupation(set_whetstone, occutext, 0);
			if (IS_FOUNTAIN(levl[u.ux][u.uy].typ))
				whetstone_fountain_effects(obj);
			else if (IS_SINK(levl[u.ux][u.uy].typ))
				whetstone_sink_effects(obj);
			else if (IS_TOILET(levl[u.ux][u.uy].typ))
				whetstone_toilet_effects(obj);
		}
	} else
		pline("You wave %s in the %s.", the(xname(stone)),
		      (IS_POOL(levl[u.ux][u.uy].typ) && Underwater) ? "water" : "air");
}

/* touchstones - by Ken Arnold */
static void use_stone(struct obj *tstone) {
	struct obj *obj;
	boolean do_scratch;
	const char *streak_color, *choices;
	char stonebuf[QBUFSZ];
	static const char scritch[] = "\"scritch, scritch\"";
	static const char allowall[3] = {COIN_CLASS, ALL_CLASSES, 0};
	static const char justgems[3] = {ALLOW_NONE, GEM_CLASS, 0};

	/* in case it was acquired while blinded */
	if (!Blind) tstone->dknown = 1;
	/* when the touchstone is fully known, don't bother listing extra
	   junk as likely candidates for rubbing */
	choices = (tstone->otyp == TOUCHSTONE && tstone->dknown &&
		   objects[TOUCHSTONE].oc_name_known) ?
			  justgems :
			  allowall;
	sprintf(stonebuf, "rub on the stone%s", plur(tstone->quan));
	if ((obj = getobj(choices, stonebuf)) == 0)
		return;

	if (obj == tstone && obj->quan == 1) {
		pline("You can't rub %s on itself.", the(xname(obj)));
		return;
	}

	if (tstone->otyp == TOUCHSTONE && tstone->cursed &&
	    obj->oclass == GEM_CLASS && !is_graystone(obj) &&
	    !obj_resists(obj, 80, 100)) {
		if (Blind)
			pline("You feel something shatter.");
		else if (Hallucination)
			pline("Oh, wow, look at the pretty shards.");
		else
			pline("A sharp crack shatters %s%s.",
			      (obj->quan > 1) ? "one of " : "", the(xname(obj)));
		useup(obj);
		return;
	}

	if (Blind) {
		pline(scritch);
		return;
	} else if (Hallucination) {
		pline("Oh wow, man: Fractals!");
		return;
	}

	do_scratch = false;
	streak_color = 0;

	switch (obj->oclass) {
		case COIN_CLASS:
			pline("Shopkeepers would spot the lighter coin%s immediately.", obj->quan > 1 ? "s" : "");
			return;
		case WEAPON_CLASS:
		case TOOL_CLASS:
			use_whetstone(tstone, obj);
			return;
		case GEM_CLASS: /* these have class-specific handling below */
		case RING_CLASS:
			if (tstone->otyp != TOUCHSTONE) {
				do_scratch = true;
			} else if (obj->oclass == GEM_CLASS && (tstone->blessed ||
								(!tstone->cursed &&
								 (Role_if(PM_ARCHEOLOGIST) || Race_if(PM_GNOME))))) {
				makeknown(TOUCHSTONE);
				makeknown(obj->otyp);
				prinv(NULL, obj, 0L);
				return;
			} else {
				/* either a ring or the touchstone was not effective */
				if (objects[obj->otyp].oc_material == GLASS) {
					do_scratch = true;
					break;
				}
			}
			streak_color = c_obj_colors[objects[obj->otyp].oc_color];
			break; /* gem or ring */

		default:
			switch (objects[obj->otyp].oc_material) {
				case CLOTH:
					pline("%s a little more polished now.", Tobjnam(tstone, "look"));
					return;
				case LIQUID:
					if (!obj->known) /* note: not "whetstone" */
						pline("You must think this is a wetstone, do you?");
					else
						pline("%s a little wetter now.", Tobjnam(tstone, "are"));
					return;
				case WAX:
					streak_color = "waxy";
					break; /* okay even if not touchstone */
				case WOOD:
					streak_color = "wooden";
					break; /* okay even if not touchstone */
				case GOLD:
					do_scratch = true; /* scratching and streaks */
					streak_color = "golden";
					break;
				case SILVER:
					do_scratch = true; /* scratching and streaks */
					streak_color = "silvery";
					break;
				default:
					/* Objects passing the is_flimsy() test will not
			   scratch a stone.  They will leave streaks on
			   non-touchstones and touchstones alike. */
					if (is_flimsy(obj))
						streak_color = c_obj_colors[objects[obj->otyp].oc_color];
					else
						do_scratch = (tstone->otyp != TOUCHSTONE);
					break;
			}
			break; /* default oclass */
	}

	sprintf(stonebuf, "stone%s", plur(tstone->quan));
	if (do_scratch)
		pline("You make %s%sscratch marks on the %s.",
		      streak_color ? streak_color : (const char *)"",
		      streak_color ? " " : "", stonebuf);
	else if (streak_color)
		pline("You see %s streaks on the %s.", streak_color, stonebuf);
	else
		pline(scritch);
	return;
}

/* Place a landmine/bear trap.  Helge Hafting */
static void use_trap(struct obj *otmp) {
	int ttyp, tmp;
	const char *what = NULL;
	char buf[BUFSZ];
	const char *occutext = "setting the trap";

	if (nohands(youmonst.data))
		what = "without hands";
	else if (Stunned)
		what = "while stunned";
	else if (u.uswallow)
		what = is_animal(u.ustuck->data) ? "while swallowed" :
						   "while engulfed";
	else if (Underwater)
		what = "underwater";
	else if (Levitation)
		what = "while levitating";
	else if (is_pool(u.ux, u.uy))
		what = "in water";
	else if (is_lava(u.ux, u.uy))
		what = "in lava";
	else if (On_stairs(u.ux, u.uy))
		what = (u.ux == xdnladder || u.ux == xupladder) ?
			       "on the ladder" :
			       "on the stairs";
	else if (IS_FURNITURE(levl[u.ux][u.uy].typ) ||
		 IS_ROCK(levl[u.ux][u.uy].typ) ||
		 closed_door(u.ux, u.uy) || t_at(u.ux, u.uy))
		what = "here";
	if (what) {
		pline("You can't set a trap %s!", what);
		reset_trapset();
		return;
	}
	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	if (otmp == trapinfo.tobj &&
	    u.ux == trapinfo.tx && u.uy == trapinfo.ty) {
		pline("You resume setting %s %s.",
		      shk_your(buf, otmp),
		      sym_desc[trap_to_defsym(what_trap(ttyp))].explanation);
		set_occupation(set_trap, occutext, 0);
		return;
	}
	trapinfo.tobj = otmp;
	trapinfo.tx = u.ux, trapinfo.ty = u.uy;
	tmp = ACURR(A_DEX);
	trapinfo.time_needed = (tmp > 17) ? 2 : (tmp > 12) ? 3 : (tmp > 7) ? 4 : 5;
	if (Blind) trapinfo.time_needed *= 2;
	tmp = ACURR(A_STR);
	if (ttyp == BEAR_TRAP && tmp < 18)
		trapinfo.time_needed += (tmp > 12) ? 1 : (tmp > 7) ? 2 : 4;
	/*[fumbling and/or confusion and/or cursed object check(s)
	   should be incorporated here instead of in set_trap]*/
	if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
		boolean chance;

		if (Fumbling || otmp->cursed)
			chance = (rnl(10) > 3);
		else
			chance = (rnl(10) > 5);
		pline("You aren't very skilled at reaching from %s.",
		      mon_nam(u.usteed));
		sprintf(buf, "Continue your attempt to set %s?",
			the(sym_desc[trap_to_defsym(what_trap(ttyp))].explanation));
		if (yn(buf) == 'y') {
			if (chance) {
				switch (ttyp) {
					case LANDMINE: /* set it off */
						trapinfo.time_needed = 0;
						trapinfo.force_bungle = true;
						break;
					case BEAR_TRAP: /* drop it without arming it */
						reset_trapset();
						pline("You drop %s!",
						      the(sym_desc[trap_to_defsym(what_trap(ttyp))].explanation));
						dropx(otmp);
						return;
				}
			}
		} else {
			reset_trapset();
			return;
		}
	}
	pline("You begin setting %s %s.",
	      shk_your(buf, otmp),
	      sym_desc[trap_to_defsym(what_trap(ttyp))].explanation);
	set_occupation(set_trap, occutext, 0);
	return;
}

static int set_trap(void) {
	struct obj *otmp = trapinfo.tobj;
	struct trap *ttmp;
	int ttyp;

	if (!otmp || !carried(otmp) ||
	    u.ux != trapinfo.tx || u.uy != trapinfo.ty) {
		/* ?? */
		reset_trapset();
		return 0;
	}

	if (--trapinfo.time_needed > 0) return 1; /* still busy */

	ttyp = (otmp->otyp == LAND_MINE) ? LANDMINE : BEAR_TRAP;
	ttmp = maketrap(u.ux, u.uy, ttyp);
	if (ttmp) {
		ttmp->tseen = 1;
		ttmp->madeby_u = 1;
		newsym(u.ux, u.uy); /* if our hero happens to be invisible */
		if (*in_rooms(u.ux, u.uy, SHOPBASE)) {
			add_damage(u.ux, u.uy, 0L); /* schedule removal */
		}
		if (!trapinfo.force_bungle)
			pline("You finish arming %s.",
			      the(sym_desc[trap_to_defsym(what_trap(ttyp))].explanation));
		if (((otmp->cursed || Fumbling) && (rnl(10) > 5)) || trapinfo.force_bungle)
			dotrap(ttmp,
			       (unsigned)(trapinfo.force_bungle ? FORCEBUNGLE : 0));
	} else {
		/* this shouldn't happen */
		pline("Your trap setting attempt fails.");
	}
	useup(otmp);
	reset_trapset();
	return 0;
}

static int use_whip(struct obj *obj) {
	char buf[BUFSZ];
	struct monst *mtmp;
	struct obj *otmp;
	int rx, ry, proficient, res = 0;
	const char *msg_slipsfree = "The bullwhip slips free.";
	const char *msg_snap = "Snap!";

	if (obj != uwep) {
		if (!wield_tool(obj, "lash"))
			return 0;
		else
			res = 1;
	}
	if (!getdir(NULL)) return res;

	if (Stunned || (Confusion && !rn2(5))) confdir();
	rx = u.ux + u.dx;
	ry = u.uy + u.dy;
	mtmp = m_at(rx, ry);

	/* fake some proficiency checks */
	proficient = 0;
	if (Role_if(PM_ARCHEOLOGIST)) ++proficient;
	if (ACURR(A_DEX) < 6)
		proficient--;
	else if (ACURR(A_DEX) >= 14)
		proficient += (ACURR(A_DEX) - 14);
	if (Fumbling) --proficient;
	if (proficient > 3) proficient = 3;
	if (proficient < 0) proficient = 0;

	if (u.uswallow && attack(u.ustuck)) {
		pline("There is not enough room to flick your bullwhip.");

	} else if (Underwater) {
		pline("There is too much resistance to flick your bullwhip.");

	} else if (u.dz < 0) {
		pline("You flick a bug off of the %s.", ceiling(u.ux, u.uy));

	} else if ((!u.dx && !u.dy) || (u.dz > 0)) {
		int dam;

		/* Sometimes you hit your steed by mistake */
		if (u.usteed && !rn2(proficient + 2)) {
			pline("You whip %s!", mon_nam(u.usteed));
			kick_steed();
			return 1;
		}
		if (Levitation || u.usteed) {
			/* Have a shot at snaring something on the floor */
			otmp = level.objects[u.ux][u.uy];
			if (otmp && otmp->otyp == CORPSE && otmp->corpsenm == PM_HORSE) {
				pline("Why beat a dead horse?");
				return 1;
			}
			if (otmp && proficient) {
				pline("You wrap your bullwhip around %s on the %s.",
				      an(singular(otmp, xname)), surface(u.ux, u.uy));
				if (rnl(6) || pickup_object(otmp, 1L, true) < 1)
					pline("%s", msg_slipsfree);
				return 1;
			}
		}
		dam = rnd(2) + dbon() + obj->spe;
		if (dam <= 0) dam = 1;
		pline("You hit your %s with your bullwhip.", body_part(FOOT));
		sprintf(buf, "killed %sself with %s bullwhip", uhim(), uhis());
		losehp(dam, buf, NO_KILLER_PREFIX);
		flags.botl = 1;
		return 1;

	} else if ((Fumbling || Glib) && !rn2(5)) {
		pline("The bullwhip slips out of your %s.", body_part(HAND));
		dropx(obj);

	} else if (u.utrap && u.utraptype == TT_PIT) {
		/*
		 *     Assumptions:
		 *
		 *	if you're in a pit
		 *		- you are attempting to get out of the pit
		 *		- or, if you are applying it towards a small
		 *		  monster then it is assumed that you are
		 *		  trying to hit it.
		 *	else if the monster is wielding a weapon
		 *		- you are attempting to disarm a monster
		 *	else
		 *		- you are attempting to hit the monster
		 *
		 *	if you're confused (and thus off the mark)
		 *		- you only end up hitting.
		 *
		 */
		const char *wrapped_what = NULL;

		if (mtmp) {
			if (bigmonst(mtmp->data)) {
				wrapped_what = strcpy(buf, mon_nam(mtmp));
			} else if (proficient) {
				if (attack(mtmp))
					return 1;
				else
					pline("%s", msg_snap);
			}
		}
		if (!wrapped_what) {
			if (IS_FURNITURE(levl[rx][ry].typ))
				wrapped_what = "something";
			else if (sobj_at(BOULDER, rx, ry))
				wrapped_what = "a boulder";
		}
		if (wrapped_what) {
			coord cc;

			cc.x = rx;
			cc.y = ry;
			pline("You wrap your bullwhip around %s.", wrapped_what);
			if (proficient && rn2(proficient + 2)) {
				if (!mtmp || enexto(&cc, rx, ry, youmonst.data)) {
					pline("You yank yourself out of the pit!");
					teleds(cc.x, cc.y, true);
					u.utrap = 0;
					vision_full_recalc = 1;
				}
			} else {
				pline("%s", msg_slipsfree);
			}
			if (mtmp) wakeup(mtmp);
		} else
			pline("%s", msg_snap);

	} else if (mtmp) {
		if (!canspotmon(mtmp) &&
		    !memory_is_invisible(rx, ry)) {
			pline("A monster is there that you couldn't see.");
			map_invisible(rx, ry);
		}
		otmp = MON_WEP(mtmp); /* can be null */
		if (otmp) {
			char onambuf[BUFSZ];
			const char *mon_hand;
			boolean gotit = proficient && (!Fumbling || !rn2(10));

			strcpy(onambuf, cxname(otmp));
			if (gotit) {
				mon_hand = mbodypart(mtmp, HAND);
				if (bimanual(otmp)) mon_hand = makeplural(mon_hand);
			} else
				mon_hand = 0; /* lint suppression */

			pline("You wrap your bullwhip around %s %s.",
			      s_suffix(mon_nam(mtmp)), onambuf);
			if (gotit && otmp->cursed) {
				pline("%s welded to %s %s%c",
				      (otmp->quan == 1L) ? "It is" : "They are",
				      mhis(mtmp), mon_hand,
				      !otmp->bknown ? '!' : '.');
				otmp->bknown = 1;
				gotit = false; /* can't pull it free */
			}
			if (gotit) {
				obj_extract_self(otmp);
				possibly_unwield(mtmp, false);
				setmnotwielded(mtmp, otmp);

				switch (rn2(proficient + 1)) {
					case 2:
						/* to floor near you */
						pline("You yank %s %s to the %s!", s_suffix(mon_nam(mtmp)),
						      onambuf, surface(u.ux, u.uy));
						place_object(otmp, u.ux, u.uy);
						stackobj(otmp);
						break;
					case 3:
						/* right to you */
#if 0
					if (!rn2(25)) {
						/* proficient with whip, but maybe not
						   so proficient at catching weapons */
						int hitu, hitvalu;

						hitvalu = 8 + otmp->spe;
						hitu = thitu(hitvalu,
						             dmgval(otmp, &youmonst),
						             otmp, NULL);
						if (hitu) {
							pline("The %s hits you as you try to snatch it!",
							      the(onambuf));
						}
						place_object(otmp, u.ux, u.uy);
						stackobj(otmp);
						break;
					}
#endif /* 0 */
						/* right into your inventory */
						pline("You snatch %s %s!", s_suffix(mon_nam(mtmp)), onambuf);
						if (otmp->otyp == CORPSE &&
						    touch_petrifies(&mons[otmp->corpsenm]) &&
						    !uarmg && !Stone_resistance &&
						    !(poly_when_stoned(youmonst.data) &&
						      polymon(PM_STONE_GOLEM))) {
							char kbuf[BUFSZ];

							sprintf(kbuf, "%s corpse",
								an(mons[otmp->corpsenm].mname));
							pline("Snatching %s is a fatal mistake.", kbuf);
							instapetrify(kbuf);
						}
						otmp = hold_another_object(otmp, "You drop %s!",
									   doname(otmp), NULL);
						break;
					default:
						/* to floor beneath mon */
						pline("You yank %s from %s %s!", the(onambuf),
						      s_suffix(mon_nam(mtmp)), mon_hand);
						obj_no_longer_held(otmp);
						place_object(otmp, mtmp->mx, mtmp->my);
						stackobj(otmp);
						break;
				}
			} else {
				pline("%s", msg_slipsfree);
			}
			wakeup(mtmp);
		} else {
			if (mtmp->m_ap_type &&
			    !Protection_from_shape_changers && !sensemon(mtmp))
				stumble_onto_mimic(mtmp);
			else
				pline("You flick your bullwhip towards %s.", mon_nam(mtmp));
			if (proficient) {
				if (attack(mtmp))
					return 1;
				else
					pline("%s", msg_snap);
			}
		}

	} else if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) {
		/* it must be air -- water checked above */
		pline("You snap your whip through thin air.");

	} else {
		pline("%s", msg_snap);
	}
	return 1;
}

/* Distance attacks by pole-weapons */
static int use_pole(struct obj *obj) {
	int res = 0, max_range;
	int min_range = obj->otyp == FISHING_POLE ? 1 : 4;
	coord cc;
	struct monst *mtmp;
	struct obj *otmp;
	boolean fishing;

	/* Are you allowed to use the pole? */
	if (u.uswallow) {
		pline("There's not enough room here to use that.");
		return 0;
	}
	if (obj != uwep) {
		if (!wield_tool(obj, "swing"))
			return 0;
		else
			res = 1;
	}

	/* Prompt for a location */
	pline("Where do you want to hit?");
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, true, "the spot to hit") < 0)
		return 0; /* user pressed ESC */

#ifdef WEAPON_SKILLS
	/* Calculate range */
	typ = weapon_type(obj);
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC)
		max_range = 4;
	else if (P_SKILL(typ) <= P_SKILLED)
		max_range = 5;
	else
		max_range = 8;
#else
	max_range = 8;
#endif

	if (distu(cc.x, cc.y) > max_range) {
		pline("Too far!");
		return res;
	} else if (distu(cc.x, cc.y) < min_range) {
		pline("Too close!");
		return res;
	} else if (!cansee(cc.x, cc.y) &&
		   ((mtmp = m_at(cc.x, cc.y)) == NULL ||
		    !canseemon(mtmp))) {
		pline("You won't hit anything if you can't see that spot.");
		return res;
	} else if (!couldsee(cc.x, cc.y)) { /* Eyes of the Overworld */
		pline("You can't reach that spot from here.");
		return res;
	}

	/* What is there? */
	mtmp = m_at(cc.x, cc.y);

	if (obj->otyp == FISHING_POLE) {
		fishing = is_pool(cc.x, cc.y);
		/* Try a random effect */
		switch (rnd(6)) {
			case 1:
				/* Snag yourself */
				pline("You hook yourself!");
				losehp(rn1(10, 10), "a fishing hook", KILLED_BY);
				return 1;
			case 2:
				/* Reel in a fish */
				if (mtmp) {
					if ((bigmonst(mtmp->data) || strongmonst(mtmp->data)) && !rn2(2)) {
						pline("You are yanked toward the %s", surface(cc.x, cc.y));
						hurtle(sgn(cc.x - u.ux), sgn(cc.y - u.uy), 1, true);
						return 1;
					} else if (enexto(&cc, u.ux, u.uy, 0)) {
						pline("You reel in %s!", mon_nam(mtmp));
						mtmp->mundetected = 0;
						rloc_to(mtmp, cc.x, cc.y);
						return 1;
					}
				}
				break;
			case 3:
				/* Snag an existing object */
				if ((otmp = level.objects[cc.x][cc.y]) != NULL) {
					pline("You snag an object from the %s!", surface(cc.x, cc.y));
					pickup_object(otmp, 1, false);
					/* If pickup fails, leave it alone */
					newsym(cc.x, cc.y);
					return 1;
				}
				break;
			case 4:
				/* Snag some garbage */
				if (fishing && flags.boot_count < 1 &&
				    (otmp = mksobj(LOW_BOOTS, true, false)) !=
					    NULL) {
					flags.boot_count++;
					pline("You snag some garbage from the %s!",
					      surface(cc.x, cc.y));
					if (pickup_object(otmp, 1, false) <= 0) {
						obj_extract_self(otmp);
						place_object(otmp, u.ux, u.uy);
						newsym(u.ux, u.uy);
					}
					return 1;
				}
				/* Or a rat in the sink/toilet */
				if (!(mvitals[PM_SEWER_RAT].mvflags & G_GONE) &&
				    (IS_SINK(levl[cc.x][cc.y].typ) ||
				     IS_TOILET(levl[cc.x][cc.y].typ))) {
					mtmp = makemon(&mons[PM_SEWER_RAT], cc.x, cc.y,
						       NO_MM_FLAGS);
					pline("Eek!  There's %s there!",
					      Blind ? "something squirmy" : a_monnam(mtmp));
					return 1;
				}
				break;
			case 5:
				/* Catch your dinner */
				if (fishing && (otmp = mksobj(CRAM_RATION, true, false)) !=
						       NULL) {
					pline("You catch tonight's dinner!");
					if (pickup_object(otmp, 1, false) <= 0) {
						obj_extract_self(otmp);
						place_object(otmp, u.ux, u.uy);
						newsym(u.ux, u.uy);
					}
					return 1;
				}
				break;
			default:
			case 6:
				/* Untrap */
				/* FIXME -- needs to deal with non-adjacent traps */
				break;
		}
	}

	/* The effect didn't apply.  Attack the monster there. */
	if (mtmp) {
		int oldhp = mtmp->mhp;

		bhitpos = cc;
		check_caitiff(mtmp);
		thitmonst(mtmp, uwep, 1);
		/* check the monster's HP because thitmonst() doesn't return
		 * an indication of whether it hit.  Not perfect (what if it's a
		 * non-silver weapon on a shade?)
		 */
		if (mtmp->mhp < oldhp)
			u.uconduct.weaphit++;
	} else
		/* Now you know that nothing is there... */
		pline("%s", "Nothing happens.");
	return 1;
}

static int use_cream_pie(struct obj *obj) {
	boolean wasblind = Blind;
	boolean wascreamed = u.ucreamed;
	boolean several = false;

	if (obj->quan > 1L) {
		several = true;
		obj = splitobj(obj, 1L);
	}
	if (Hallucination)
		pline("You give yourself a facial.");
	else
		pline("You immerse your %s in %s%s.", body_part(FACE),
		      several ? "one of " : "",
		      several ? makeplural(the(xname(obj))) : the(xname(obj)));
	if (can_blnd(NULL, &youmonst, AT_WEAP, obj)) {
		int blindinc = rnd(25);
		u.ucreamed += blindinc;
		make_blinded(Blinded + (long)blindinc, false);
		if (!Blind || (Blind && wasblind))
			pline("There's %ssticky goop all over your %s.",
			      wascreamed ? "more " : "",
			      body_part(FACE));
		else /* Blind  && !wasblind */
			pline("You can't see through all the sticky goop on your %s.",
			      body_part(FACE));
	}
	if (obj->unpaid) {
		verbalize("You used it, you bought it!");
		bill_dummy_object(obj);
	}
	obj_extract_self(obj);
	delobj(obj);
	return 0;
}

static int use_grapple(struct obj *obj) {
	int res = 0, typ, max_range = 4, tohit;
	coord cc;
	struct monst *mtmp;
	struct obj *otmp;

	/* Are you allowed to use the hook? */
	if (u.uswallow) {
		pline("There's not enough room here to use that.");
		return 0;
	}
	if (obj != uwep) {
		if (!wield_tool(obj, "cast"))
			return 0;
		else
			res = 1;
	}
	/* assert(obj == uwep); */

	/* Prompt for a location */
	pline("Where do you want to hit?");
	cc.x = u.ux;
	cc.y = u.uy;
	if (getpos(&cc, true, "the spot to hit") < 0)
		return 0; /* user pressed ESC */

	/* Calculate range */
	typ = uwep_skill_type();
	if (typ == P_NONE || P_SKILL(typ) <= P_BASIC)
		max_range = 4;
	else if (P_SKILL(typ) == P_SKILLED)
		max_range = 5;
	else
		max_range = 8;
	if (distu(cc.x, cc.y) > max_range) {
		pline("Too far!");
		return res;
	} else if (!cansee(cc.x, cc.y)) {
		pline("You won't hit anything if you can't see that spot.");
		return res;
	}

	/* What do you want to hit? */
	tohit = rn2(5);
	if (typ != P_NONE && P_SKILL(typ) >= P_SKILLED) {
		winid tmpwin = create_nhwindow(NHW_MENU);
		anything any;
		char buf[BUFSZ];
		menu_item *selected;

		any.a_void = 0; /* set all bits to zero */
		any.a_int = 1;	/* use index+1 (cant use 0) as identifier */
		start_menu(tmpwin);
		any.a_int++;
		sprintf(buf, "an object on the %s", surface(cc.x, cc.y));
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
		any.a_int++;
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 "a monster", MENU_UNSELECTED);
		any.a_int++;
		sprintf(buf, "the %s", surface(cc.x, cc.y));
		add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
		end_menu(tmpwin, "Aim for what?");
		tohit = rn2(4);
		if (select_menu(tmpwin, PICK_ONE, &selected) > 0 &&
		    rn2(P_SKILL(typ) > P_SKILLED ? 20 : 2))
			tohit = selected[0].item.a_int - 1;
		free(selected);
		destroy_nhwindow(tmpwin);
	}

	/* What did you hit? */
	switch (tohit) {
		case 0: /* Trap */
			/* FIXME -- untrap needs to deal with non-adjacent traps */
			break;
		case 1: /* Object */
			if ((otmp = level.objects[cc.x][cc.y]) != 0) {
				pline("You snag an object from the %s!", surface(cc.x, cc.y));
				pickup_object(otmp, 1L, false);
				/* If pickup fails, leave it alone */
				newsym(cc.x, cc.y);
				return 1;
			}
			break;
		case 2: /* Monster */
			if ((mtmp = m_at(cc.x, cc.y)) == NULL) break;
			if (verysmall(mtmp->data) && !rn2(4) &&
			    enexto(&cc, u.ux, u.uy, NULL)) {
				pline("You pull in %s!", mon_nam(mtmp));
				mtmp->mundetected = 0;
				rloc_to(mtmp, cc.x, cc.y);
				return 1;
			} else if ((!bigmonst(mtmp->data) && !strongmonst(mtmp->data)) ||
				   rn2(4)) {
				thitmonst(mtmp, uwep, 1);
				return 1;
			}
		/* FALL THROUGH */
		case 3: /* Surface */
			if (IS_AIR(levl[cc.x][cc.y].typ) || is_pool(cc.x, cc.y))
				pline("The hook slices through the %s.", surface(cc.x, cc.y));
			else {
				pline("You are yanked toward the %s!", surface(cc.x, cc.y));
				hurtle(sgn(cc.x - u.ux), sgn(cc.y - u.uy), 1, false);
				spoteffects(true);
			}
			return 1;
		default: /* Yourself (oops!) */
			if (P_SKILL(typ) <= P_BASIC) {
				pline("You hook yourself!");
				losehp(rn1(10, 10), "a grappling hook", KILLED_BY);
				return 1;
			}
			break;
	}
	pline("%s", "Nothing happens.");
	return 1;
}

#define BY_OBJECT (NULL)

/* return 1 if the wand is broken, hence some time elapsed */
static int do_break_wand(struct obj *obj) {
	char confirm[QBUFSZ], the_wand[BUFSZ];

	strcpy(the_wand, yname(obj));
	sprintf(confirm, "Are you really sure you want to break %s?",
		safe_qbuf("", sizeof("Are you really sure you want to break ?"),
			  the_wand, ysimple_name(obj), "the wand"));
	if (yn(confirm) == 'n') return 0;

	if (nohands(youmonst.data)) {
		pline("You can't break %s without hands!", the_wand);
		return 0;
	} else if (ACURR(A_STR) < 10) {
		pline("You don't have the strength to break %s!", the_wand);
		return 0;
	}
	pline("Raising %s high above your %s, you break it in two!",
	      the_wand, body_part(HEAD));
	return wand_explode(obj, true);
}

/* This function takes care of the effects wands exploding, via
 * user-specified 'applying' as well as wands exploding by accident
 * during use (called by backfire() in zap.c)
 *
 * If the effect is directly recognisable as pertaining to a
 * specific wand, the wand should be makeknown()
 * Otherwise, if there is an ambiguous or indirect but visible effect
 * the wand should be allowed to be named by the user.
 *
 * If there is no obvious effect,  do nothing. (Should this be changed
 * to letting the user call that type of wand?)
 *
 * hero_broke is nonzero if the user initiated the action that caused
 * the wand to explode (zapping or applying).
 */
int wand_explode(struct obj *obj, boolean hero_broke) {
	static const char nothing_else_happens[] = "But nothing else happens...";
	int i, x, y;
	struct monst *mon;
	int dmg, damage;
	boolean affects_objects;
	boolean shop_damage = false;
	int expltype = EXPL_MAGICAL;
	char buf[BUFSZ];

	/* [ALI] Do this first so that wand is removed from bill. Otherwise,
	 * the freeinv() below also hides it from setpaid() which causes problems.
	 */
	if (carried(obj) ? obj->unpaid :
			   !obj->no_charge && costly_spot(obj->ox, obj->oy)) {
		if (hero_broke)
			check_unpaid(obj); /* Extra charge for use */
		bill_dummy_object(obj);
	}

	current_wand = obj; /* destroy_item might reset this */
	freeinv(obj);	    /* hide it from destroy_item instead... */
	setnotworn(obj);    /* so we need to do this ourselves */

	if (obj->spe <= 0) {
		pline(nothing_else_happens);
		goto discard_broken_wand;
	}
	obj->ox = u.ux;
	obj->oy = u.uy;
	dmg = obj->spe * 4;
	affects_objects = false;

	switch (obj->otyp) {
		case WAN_WISHING:
		case WAN_NOTHING:
		case WAN_LOCKING:
		case WAN_PROBING:
		case WAN_ENLIGHTENMENT:
		case WAN_OPENING:
		case WAN_SECRET_DOOR_DETECTION:
			pline(nothing_else_happens);
			goto discard_broken_wand;
		case WAN_DEATH:
		case WAN_LIGHTNING:
			dmg *= 4;
			goto wanexpl;
		case WAN_COLD:
			expltype = EXPL_FROSTY;
			dmg *= 2;
		case WAN_MAGIC_MISSILE:
		wanexpl:
			explode(u.ux, u.uy, ZT_MAGIC_MISSILE, dmg, WAND_CLASS, expltype);
			makeknown(obj->otyp); /* explode described the effect */
			goto discard_broken_wand;
		/*WAC for wands of fireball- no double damage
	 * As well, effect is the same as fire, so no makeknown
	 */
		case WAN_FIRE:
			dmg *= 2;
		case WAN_FIREBALL:
			expltype = EXPL_FIERY;
			explode(u.ux, u.uy, ZT_FIRE, dmg, WAND_CLASS, expltype);
			if (obj->dknown && !objects[obj->otyp].oc_name_known &&
			    !objects[obj->otyp].oc_uname)
				docall(obj);
			goto discard_broken_wand;
		case WAN_STRIKING:
			/* we want this before the explosion instead of at the very end */
			pline("A wall of force smashes down around you!");
			dmg = d(1 + obj->spe, 6); /* normally 2d12 */
		case WAN_CANCELLATION:
		case WAN_POLYMORPH:
		case WAN_UNDEAD_TURNING:
		case WAN_DRAINING: /* KMH */
			affects_objects = true;
			break;
		case WAN_TELEPORTATION:
			/* WAC make tele trap if you broke a wand of teleport */
			/* But make sure the spot is valid! */
			if ((obj->spe > 2) && rn2(obj->spe - 2) && !level.flags.noteleport &&
			    !u.uswallow && !On_stairs(u.ux, u.uy) && (!IS_FURNITURE(levl[u.ux][u.uy].typ) && !IS_ROCK(levl[u.ux][u.uy].typ) && !closed_door(u.ux, u.uy) && !t_at(u.ux, u.uy))) {
				struct trap *ttmp;

				ttmp = maketrap(u.ux, u.uy, TELEP_TRAP);
				if (ttmp) {
					ttmp->madeby_u = 1;
					newsym(u.ux, u.uy); /* if our hero happens to be invisible */
					if (*in_rooms(u.ux, u.uy, SHOPBASE)) {
						/* shopkeeper will remove it */
						add_damage(u.ux, u.uy, 0L);
					}
				}
			}
			affects_objects = true;
			break;
		case WAN_CREATE_HORDE: /* More damage than Create monster */
			dmg *= 2;
			break;
		case WAN_HEALING:
		case WAN_EXTRA_HEALING:
			dmg = 0;
			break;
		default:
			break;
	}

	/* magical explosion and its visual effect occur before specific effects */
	explode(obj->ox, obj->oy, ZT_MAGIC_MISSILE, dmg ? rnd(dmg) : 0, WAND_CLASS,
		EXPL_MAGICAL);

	/* this makes it hit us last, so that we can see the action first */
	for (i = 0; i <= 8; i++) {
		bhitpos.x = x = obj->ox + xdir[i];
		bhitpos.y = y = obj->oy + ydir[i];
		if (!isok(x, y)) continue;

		if (obj->otyp == WAN_DIGGING) {
			if (dig_check(BY_OBJECT, false, x, y)) {
				if (IS_WALL(levl[x][y].typ) || IS_DOOR(levl[x][y].typ)) {
					/* normally, pits and holes don't anger guards, but they
					 * do if it's a wall or door that's being dug */
					watch_dig(NULL, x, y, true);
					if (*in_rooms(x, y, SHOPBASE)) shop_damage = true;
				}
				digactualhole(x, y, BY_OBJECT,
					      (rn2(obj->spe) < 3 || !Can_dig_down(&u.uz)) ?
						      PIT :
						      HOLE);
			}
			continue;
			/* WAC catch Create Horde wands too */
			/* MAR make the monsters around you */
		} else if (obj->otyp == WAN_CREATE_MONSTER || obj->otyp == WAN_CREATE_HORDE) {
			/* u.ux,u.uy creates it near you--x,y might create it in rock */
			makemon(NULL, u.ux, u.uy, NO_MM_FLAGS);
			continue;
		} else {
			if (x == u.ux && y == u.uy) {
				/* teleport objects first to avoid race with tele control and
				   autopickup.  Other wand/object effects handled after
				   possible wand damage is assessed */
				if (obj->otyp == WAN_TELEPORTATION &&
				    affects_objects && level.objects[x][y]) {
					bhitpile(obj, bhito, x, y);
					if (flags.botl) bot(); /* potion effects */
							       /* makeknown is handled in zapyourself */
				}
				damage = zapyourself(obj, false);
				if (damage) {
					if (hero_broke) {
						sprintf(buf, "killed %sself by breaking a wand", uhim());
						losehp(damage, buf, NO_KILLER_PREFIX);
					} else
						losehp(damage, "exploding wand", KILLED_BY_AN);
				}
				if (flags.botl) bot(); /* blindness */
			} else if ((mon = m_at(x, y)) != 0 && !DEADMONSTER(mon)) {
				bhitm(mon, obj);
				/* if (flags.botl) bot(); */
			}
			if (affects_objects && level.objects[x][y]) {
				bhitpile(obj, bhito, x, y);
				if (flags.botl) bot(); /* potion effects */
			}
		}
	}

	/* Note: if player fell thru, this call is a no-op.
	   Damage is handled in digactualhole in that case */
	if (shop_damage) pay_for_damage("dig into", false);

	if (obj->otyp == WAN_LIGHT)
		litroom(true, obj); /* only needs to be done once */

discard_broken_wand:
	obj = current_wand; /* [see dozap() and destroy_item()] */
	current_wand = 0;
	if (obj)
		delobj(obj);
	nomul(0);
	return 1;
}

static bool uhave_graystone(void) {
	struct obj *otmp;

	for (otmp = invent; otmp; otmp = otmp->nobj)
		if (is_graystone(otmp))
			return true;
	return false;
}

static void add_class(char *cl, char class) {
	char tmp[2];
	tmp[0] = class;
	tmp[1] = '\0';
	strcat(cl, tmp);
}

int doapply(void) {
	struct obj *obj;
	int res = 1;
	boolean can_use = false;
	char class_list[MAXOCLASSES + 2];

	if (check_capacity(NULL)) return 0;

	if (carrying(POT_OIL) || uhave_graystone())
		strcpy(class_list, tools_too);
	else
		strcpy(class_list, tools);
	if (carrying(CREAM_PIE) || carrying(EUCALYPTUS_LEAF))
		add_class(class_list, FOOD_CLASS);

	obj = getobj(class_list, "use or apply");
	if (!obj) return 0;

	if (obj->oartifact && !touch_artifact(obj, &youmonst))
		return 1; /* evading your grasp costs a turn; just be
			   grateful that you don't drop it as well */

	if (obj->oclass == WAND_CLASS)
		return do_break_wand(obj);

	switch (obj->otyp) {
		case BLINDFOLD:
		case LENSES:
			if (obj == ublindf) {
				if (!cursed(obj)) Blindf_off(obj);
			} else if (!ublindf)
				Blindf_on(obj);
			else
				pline("You are already %s.",
				      ublindf->otyp == TOWEL ? "covered by a towel" :
							       ublindf->otyp == BLINDFOLD ? "wearing a blindfold" :
											    "wearing lenses");
			break;
		case CREAM_PIE:
			res = use_cream_pie(obj);
			break;
		case BULLWHIP:
			res = use_whip(obj);
			break;
		case GRAPPLING_HOOK:
			res = use_grapple(obj);
			break;
		case LARGE_BOX:
		case CHEST:
		case ICE_BOX:
		case SACK:
		case BAG_OF_HOLDING:
		case OILSKIN_SACK:
			res = use_container(&obj, 1);
			break;
		case BAG_OF_TRICKS:
			bagotricks(obj);
			break;
		case CAN_OF_GREASE:
			use_grease(obj);
			break;
		case CREDIT_CARD:
		case LOCK_PICK:
		case SKELETON_KEY:
			pick_lock(&obj);
			break;
		case PICK_AXE:
		case DWARVISH_MATTOCK: /* KMH, balance patch -- the mattock is a pick, too */
			res = use_pick_axe(obj);
			break;
		case FISHING_POLE:
			res = use_pole(obj);
			break;
		case TINNING_KIT:
			use_tinning_kit(obj);
			break;
		case LEASH:
			use_leash(&obj);
			break;
		case SADDLE:
			res = use_saddle(obj);
			break;
		case MAGIC_WHISTLE:
			use_magic_whistle(obj);
			break;
		case TIN_WHISTLE:
			use_whistle(obj);
			break;
		case EUCALYPTUS_LEAF:
			/* MRKR: Every Australian knows that a gum leaf makes an */
			/*	 excellent whistle, especially if your pet is a  */
			/*	 tame kangaroo named Skippy.			 */
			if (obj->blessed) {
				use_magic_whistle(obj);
				/* sometimes the blessing will be worn off */
				if (!rn2(49)) {
					if (!Blind) {
						char buf[BUFSZ];

						pline("%s %s %s.", Shk_Your(buf, obj),
						      aobjnam(obj, "glow"), hcolor("brown"));
						obj->bknown = 1;
					}
					unbless(obj);
				}
			} else {
				use_whistle(obj);
			}
			break;
		case STETHOSCOPE:
			res = use_stethoscope(obj);
			break;
		case MIRROR:
			res = use_mirror(obj);
			break;
		case SPOON:
			pline("It's a finely crafted antique spoon; what do you want to do with it?");
			break;
		case BELL:
		case BELL_OF_OPENING:
			use_bell(&obj);
			break;
		case CANDELABRUM_OF_INVOCATION:
			use_candelabrum(obj);
			break;
		case WAX_CANDLE:
		/* STEPHEN WHITE'S NEW CODE */
		case MAGIC_CANDLE:
		case TALLOW_CANDLE:
			use_candle(&obj);
			break;
		case GREEN_LIGHTSABER:
		case BLUE_LIGHTSABER:
		case RED_LIGHTSABER:
		case RED_DOUBLE_LIGHTSABER:
			if (uwep != obj && !wield_tool(obj, NULL)) break;
		/* Fall through - activate via use_lamp */
		case OIL_LAMP:
		case MAGIC_LAMP:
		case BRASS_LANTERN:
			use_lamp(obj);
			break;
		case TORCH:
			res = use_torch(obj);
			break;
		case POT_OIL:
			light_cocktail(obj);
			break;
		case EXPENSIVE_CAMERA:
			res = use_camera(obj);
			break;
		case TOWEL:
			res = use_towel(obj);
			break;
		case CRYSTAL_BALL:
			use_crystal_ball(obj);
			break;
			/* STEPHEN WHITE'S NEW CODE */
			/* KMH, balance patch -- source of abuse */
#if 0
	case ORB_OF_ENCHANTMENT:
		if(obj->spe > 0) {

			check_unpaid(obj);
			if(uwep && (uwep->oclass == WEAPON_CLASS ||
			                uwep->otyp == PICK_AXE ||
			                uwep->otyp == UNICORN_HORN)) {
				if (uwep->spe < 5) {
					if (obj->blessed) {
						if (!Blind) pline("Your %s glows silver.",xname(uwep));
						uwep->spe += rnd(2);
					} else if (obj->cursed) {
						if (!Blind) pline("Your %s glows black.",xname(uwep));
						uwep->spe -= rnd(2);
					} else {
						if (rn2(3)) {
							if (!Blind) pline("Your %s glows bright for a moment.",xname(uwep));
							uwep->spe += 1;
						} else {
							if (!Blind) pline("Your %s glows dark for a moment.",xname(uwep));
							uwep->spe -= 1;
						}
					}
				} else pline("Nothing seems to happen.");

				if (uwep->spe > 5) uwep->spe = 5;

			} else pline("The orb glows for a moment, then fades.");
			consume_obj_charge(obj, false);

		} else pline("This orb is burnt out.");
		break;
	case ORB_OF_CHARGING:
		if(obj->spe > 0) {
			struct obj *otmp;
			makeknown(ORB_OF_CHARGING);
			consume_obj_charge(obj, true);
			otmp = getobj(all_count, "charge");
			if (!otmp) break;
			recharge(otmp, obj->cursed ? -1 : (obj->blessed ? 1 : 0));
		} else pline("This orb is burnt out.");
		break;
	case ORB_OF_DESTRUCTION:
		useup(obj);
		pline("As you activate the orb, it explodes!");
		explode(u.ux, u.uy, ZT_SPELL(ZT_MAGIC_MISSILE), d(12,6), WAND_CLASS);
		check_unpaid(obj);
		break;
#endif
		case MAGIC_MARKER:
			res = dowrite(obj);
			break;
		case TIN_OPENER:
			if (!carrying(TIN)) {
				pline("You have no tin to open.");
				goto xit;
			}
			pline("You cannot open a tin without eating or discarding its contents.");
			if (flags.verbose)
				pline("In order to eat, use the 'e' command.");
			if (obj != uwep)
				pline("Opening the tin will be much easier if you wield the tin opener.");
			goto xit;

		case FIGURINE:
			use_figurine(&obj);
			break;
		case UNICORN_HORN:
			use_unicorn_horn(obj);
			break;
		case WOODEN_FLUTE:
		case MAGIC_FLUTE:
		case TOOLED_HORN:
		case FROST_HORN:
		case FIRE_HORN:
		case WOODEN_HARP:
		case MAGIC_HARP:
		case BUGLE:
		case LEATHER_DRUM:
		case DRUM_OF_EARTHQUAKE:
			/* KMH, balance patch -- removed
		case PAN_PIPE_OF_SUMMONING:
		case PAN_PIPE_OF_THE_SEWERS:
		case PAN_PIPE:*/
			res = do_play_instrument(obj);
			break;
		case MEDICAL_KIT:
			if (Role_if(PM_HEALER))
				can_use = true;
			else if ((Role_if(PM_PRIEST) || Role_if(PM_MONK) ||
				  Role_if(PM_UNDEAD_SLAYER) || Role_if(PM_SAMURAI)) &&
				 !rn2(2))
				can_use = true;
			else if (!rn2(4))
				can_use = true;

			if (obj->cursed && rn2(3)) can_use = false;
			if (obj->blessed && rn2(3)) can_use = true;

			makeknown(MEDICAL_KIT);
			if (obj->cobj) {
				struct obj *otmp;
				for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
					if (otmp->otyp == PILL)
						break;
				if (!otmp)
					pline("You can't find any more pills in %s.", yname(obj));
				else if (!is_edible(otmp))
					pline("You find, but cannot eat, a white pill in %s.",
					      yname(obj));
				else {
					check_unpaid(obj);
					if (otmp->quan > 1L) {
						otmp->quan--;
						obj->owt = weight(obj);
					} else {
						obj_extract_self(otmp);
						obfree(otmp, NULL);
					}
					/*
				 * Note that while white and pink pills share the
				 * same otyp value, they are quite different.
				 */
					pline("You take a white pill from %s and swallow it.",
					      yname(obj));
					if (can_use) {
						if (Sick)
							make_sick(0L, NULL, true, SICK_ALL);
						else if (Blinded > (long)(u.ucreamed + 1))
							make_blinded(u.ucreamed ?
									     (long)(u.ucreamed + 1) :
									     0L,
								     true);
						else if (HHallucination)
							make_hallucinated(0L, true, 0L);
						else if (Vomiting)
							make_vomiting(0L, true);
						else if (HConfusion)
							make_confused(0L, true);
						else if (HStun)
							make_stunned(0L, true);
						else if (u.uhp < u.uhpmax) {
							u.uhp += rn1(10, 10);
							if (u.uhp > u.uhpmax) u.uhp = u.uhpmax;
							pline("You feel better.");
							flags.botl = true;
						} else
							pline("%s", "Nothing happens.");
					} else if (!rn2(3))
						pline("Nothing seems to happen.");
					else if (!Sick)
						make_sick(rn1(10, 10), "bad pill", true,
							  SICK_VOMITABLE);
					else {
						pline("You seem to have made your condition worse!");
						losehp(rn1(10, 10), "a drug overdose", KILLED_BY);
					}
				}
			} else
				pline("You seem to be out of medical supplies");
			break;
		case HORN_OF_PLENTY: /* not a musical instrument */
			if (obj->spe > 0) {
				struct obj *otmp;
				const char *what;

				consume_obj_charge(obj, true);
				if (!rn2(13)) {
					otmp = mkobj(POTION_CLASS, false);
					/* KMH, balance patch -- rewritten */
					while ((otmp->otyp == POT_SICKNESS) ||
					       objects[otmp->otyp].oc_magic)
						otmp->otyp = rnd_class(POT_BOOZE, POT_WATER);
					what = "A potion";
				} else {
					otmp = mkobj(FOOD_CLASS, false);
					if (otmp->otyp == FOOD_RATION && !rn2(7))
						otmp->otyp = LUMP_OF_ROYAL_JELLY;
					what = "Some food";
				}
				pline("%s spills out.", what);
				otmp->blessed = obj->blessed;
				otmp->cursed = obj->cursed;
				otmp->owt = weight(otmp);
				otmp = hold_another_object(otmp, u.uswallow ? "Oops!  %s out of your reach!" : (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) || levl[u.ux][u.uy].typ < IRONBARS || levl[u.ux][u.uy].typ >= ICE) ? "Oops!  %s away from you!" : "Oops!  %s to the floor!",
							   The(aobjnam(otmp, "slip")),
							   NULL);
				makeknown(HORN_OF_PLENTY);
			} else
				pline("%s", "Nothing happens.");
			break;
		case LAND_MINE:
		case BEARTRAP:
			use_trap(obj);
			break;
		case FLINT:
		case LUCKSTONE:
		case LOADSTONE:
		case TOUCHSTONE:
		case HEALTHSTONE:
		case WHETSTONE:
			use_stone(obj);
			break;
		case ASSAULT_RIFLE:
			/* Switch between WP_MODE_SINGLE, WP_MODE_BURST and WP_MODE_AUTO */

			if (obj->altmode == WP_MODE_AUTO) {
				obj->altmode = WP_MODE_BURST;
			} else if (obj->altmode == WP_MODE_BURST) {
				obj->altmode = WP_MODE_SINGLE;
			} else {
				obj->altmode = WP_MODE_AUTO;
			}

			pline("You switch %s to %s mode.", yname(obj),
			      ((obj->altmode == WP_MODE_SINGLE) ? "single shot" :
								  ((obj->altmode == WP_MODE_BURST) ? "burst" :
												     "full automatic")));
			break;
		case AUTO_SHOTGUN:
		case SUBMACHINE_GUN:
			if (obj->altmode == WP_MODE_AUTO)
				obj->altmode = WP_MODE_SINGLE;
			else
				obj->altmode = WP_MODE_AUTO;
			pline("You switch %s to %s mode.", yname(obj),
			      (obj->altmode ? "semi-automatic" : "full automatic"));
			break;
		case FRAG_GRENADE:
		case GAS_GRENADE:
			if (!obj->oarmed) {
				pline("You arm %s.", yname(obj));
				arm_bomb(obj, true);
			} else
				pline("It's already armed!");
			break;
		case STICK_OF_DYNAMITE:
			light_cocktail(obj);
			break;
		default:
			/* KMH, balance patch -- polearms can strike at a distance */
			if (is_pole(obj)) {
				res = use_pole(obj);
				break;
			} else if (is_pick(obj) || is_axe(obj)) {
				res = use_pick_axe(obj);
				break;
			}
			pline("Sorry, I don't know how to use that.");
		xit:
			nomul(0);
			return 0;
	}
	if (res && obj && obj->oartifact) arti_speak(obj);
	nomul(0);
	return res;
}

/* Keep track of unfixable troubles for purposes of messages saying you feel
 * great.
 */
int unfixable_trouble_count(boolean is_horn) {
	int unfixable_trbl = 0;

	if (Stoned) unfixable_trbl++;
	if (Strangled) unfixable_trbl++;
	if (Wounded_legs && !u.usteed) unfixable_trbl++;
	if (Slimed) unfixable_trbl++;
	/* lycanthropy is not desirable, but it doesn't actually make you feel
	   bad */

	/* we'll assume that intrinsic stunning from being a bat/stalker
	   doesn't make you feel bad */
	if (!is_horn) {
		if (Confusion) unfixable_trbl++;
		if (Sick) unfixable_trbl++;
		if (HHallucination) unfixable_trbl++;
		if (Vomiting) unfixable_trbl++;
		if (HStun) unfixable_trbl++;
	}
	return unfixable_trbl;
}

/*apply.c*/
