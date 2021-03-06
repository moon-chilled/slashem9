/*	SCCS Id: @(#)artifact.c 3.4	2003/08/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "artilist.h"
#include "hack.h"
#include "artifact.h"
/*
 * Note:  both artilist[] and artiexist[] have a dummy element #0,
 *	  so loops over them should normally start at #1.  The primary
 *	  exception is the save & restore code, which doesn't care about
 *	  the contents, just the total size.
 */

extern boolean notonhead; /* for long worms */

#define get_artifact(o) \
	(((o) && (o)->oartifact) ? &artilist[(int)(o)->oartifact] : 0)

static int spec_applies(const struct artifact *weap, struct monst *mtmp);
static int arti_invoke(struct obj *obj);
static bool Mb_hit(struct monst *magr, struct monst *mdef, struct obj *mb, int *dmgptr, int dieroll, bool vis, char *hittee);

/* The amount added to the victim's total hit points to insure that the
   victim will be killed even after damage bonus/penalty adjustments.
   Most such penalties are small, and 200 is plenty; the exception is
   half physical damage.  3.3.1 and previous versions tried to use a very
   large number to account for this case; now, we just compute the fatal
   damage by adding it to 2 times the total hit points instead of 1 time.
   Note: this will still break if they have more than about half the number
   of hit points that will fit in a 15 bit integer. */
#define FATAL_DAMAGE_MODIFIER 200

/* coordinate effects from spec_dbon() with messages in artifact_hit() */
static int spec_dbon_applies = 0;

/* flags including which artifacts have already been created */
static bool artiexist[1 + NROFARTIFACTS + 1];
/* and a discovery list for them (no dummy first entry here) */

static int artidisco[NROFARTIFACTS];

static void hack_artifacts(void);
static bool attacks(int, struct obj *);

/* handle some special cases; must be called after role_init() */
static void hack_artifacts(void) {
	struct artifact *art;
	int alignmnt = aligns[flags.initalign].value;

	/* Fix up the alignments of "gift" artifacts */
	for (art = artilist + 1; art->otyp; art++)
		if (art->role == Role_switch && art->alignment != A_NONE)
			art->alignment = alignmnt;

	/* Excalibur can be used by any lawful character, not just knights */
	if (!Role_if(PM_KNIGHT))
		artilist[ART_EXCALIBUR].role = NON_PM;

#if 0
	/* Fix up the gifts */
	if (urole.gift1arti) {
		artilist[urole.gift1arti].alignment = alignmnt;
		artilist[urole.gift1arti].role = Role_switch;
	}
	if (urole.gift2arti) {
		artilist[urole.gift2arti].alignment = alignmnt;
		artilist[urole.gift2arti].role = Role_switch;
	}
#endif
	/* Fix up the quest artifact */
	if (urole.questarti) {
		artilist[urole.questarti].alignment = alignmnt;
		artilist[urole.questarti].role = Role_switch;
	}
	return;
}

/* zero out the artifact existence list */
void init_artifacts(void) {
	memset(artiexist, 0, sizeof(artiexist));
	memset(artidisco, 0, sizeof(artidisco));
	hack_artifacts();
}

/* Post u_init() initialization */
void init_artifacts1(void) {
#if 0
	/* KMH -- Should be at least skilled in first artifact gifts */
	if (urole.gift1arti &&
	                (objects[artilist[urole.gift1arti].otyp].oc_class == WEAPON_CLASS ||
	                 objects[artilist[urole.gift1arti].otyp].oc_class == TOOL_CLASS)) {
		int skill = objects[artilist[urole.gift1arti].otyp].oc_skill;

		if (skill > P_NONE && P_SKILL(skill) < P_UNSKILLED)
			P_SKILL(skill) = P_UNSKILLED;
		if (skill > P_NONE && P_MAX_SKILL(skill) < P_SKILLED) {
			pline("Warning: %s should be at least skilled.  Fixing...",
			      artilist[urole.gift1arti].name);
			P_MAX_SKILL(skill) = P_SKILLED;
		}
	}
	if (urole.gift2arti &&
	                (objects[artilist[urole.gift2arti].otyp].oc_class == WEAPON_CLASS ||
	                 objects[artilist[urole.gift2arti].otyp].oc_class == TOOL_CLASS)) {
		int skill = objects[artilist[urole.gift2arti].otyp].oc_skill;

		if (skill > P_NONE && P_SKILL(skill) < P_UNSKILLED)
			P_SKILL(skill) = P_UNSKILLED;
		if (skill > P_NONE && P_MAX_SKILL(skill) < P_SKILLED) {
			pline("Warning: %s should be at least skilled.  Fixing...",
			      artilist[urole.gift1arti].name);
			P_MAX_SKILL(skill) = P_SKILLED;
		}
	}
#endif
	/* KMH -- Should be expert in quest artifact */
	if (urole.questarti &&
	    (objects[artilist[urole.questarti].otyp].oc_class == WEAPON_CLASS ||
	     objects[artilist[urole.questarti].otyp].oc_class == TOOL_CLASS)) {
		int skill = objects[artilist[urole.questarti].otyp].oc_skill;

		if (skill > P_NONE && P_SKILL(skill) < P_UNSKILLED)
			P_SKILL(skill) = P_UNSKILLED;
		if (skill > P_NONE && P_MAX_SKILL(skill) < P_EXPERT) {
			pline("Warning: %s should be at least expert.  Fixing...",
			      artilist[urole.questarti].name);
			P_MAX_SKILL(skill) = P_EXPERT;
		}
	}
}

void save_artifacts(int fd) {
	bwrite(fd, artiexist, sizeof(artiexist));
	bwrite(fd, artidisco, sizeof(artidisco));
}

void restore_artifacts(int fd) {
	mread(fd, artiexist, sizeof(artiexist));
	mread(fd, artidisco, sizeof(artidisco));
	hack_artifacts(); /* redo non-saved special cases */
}

const char *artiname(int artinum) {
	if (artinum <= 0 || artinum > NROFARTIFACTS) return "";
	return artilist[artinum].name;
}

/*
   Make an artifact.  If a specific alignment is specified, then an object of
   the appropriate alignment is created from scratch, or 0 is returned if
   none is available.  (If at least one aligned artifact has already been
   given, then unaligned ones also become eligible for this.)
   If no alignment is given, then 'otmp' is converted
   into an artifact of matching type, or returned as-is if that's not possible.
   For the 2nd case, caller should use ``obj = mk_artifact(obj, A_NONE);''
   for the 1st, ``obj = mk_artifact(NULL, some_alignment);''.
 */
struct obj *mk_artifact(
	struct obj *otmp, /* existing object; ignored if alignment specified */
	aligntyp alignment /* target alignment, or A_NONE */) {
	const struct artifact *a;
	int n, m;
	const bool by_align = (alignment != A_NONE);
	short o_typ = (by_align || !otmp) ? 0 : otmp->otyp;
	const bool unique = !by_align && otmp && objects[o_typ].oc_unique;
	short eligible[NROFARTIFACTS];

#if 0
	/* KMH, role patch -- Try first or second gift */
	m = urole.gift1arti;
	if (by_align && m && !artiexist[m]) {
		a = (struct artifact *)&artilist[m];
		goto make_artif;
	}
	m = urole.gift2arti;
	if (by_align && m && !artiexist[m]) {
		a = (struct artifact *)&artilist[m];
		goto make_artif;
	}
#endif

	/* gather eligible artifacts */
	for (n = 0, a = artilist + 1, m = 1; a->otyp; a++, m++)
		if ((!by_align ? a->otyp == o_typ :
				 (a->alignment == alignment ||
				  (a->alignment == A_NONE && u.ugifts > 0))) &&
		    (!(a->spfx & SPFX_NOGEN) || unique) && !artiexist[m]) {
			/*
			 * [ALI] The determination of whether an artifact is
			 * hostile to the player is a little more complex in
			 * Slash'EM than Vanilla since there are artifacts
			 * which are hostile to humans (eg., Deathsword) which
			 * aren't aligned to any race.
			 * Nevertheless, the rule remains the same: Gods don't
			 * grant artifacts which would be hostile to the player
			 * _in their normal form_.
			 */
			bool hostile = false;
			if (by_align) {
				if (a->race != NON_PM && race_hostile(&mons[a->race]))
					hostile = true; /* enemies' equipment */
				else if (a->spfx & SPFX_DBONUS) {
					struct artifact tmp;

					tmp = *a;
					tmp.spfx &= SPFX_DBONUS;
					if (Upolyd)
						set_mon_data(&youmonst, &upermonst, 0);
					if (spec_applies(&tmp, &youmonst))
						hostile = true; /* can blast unpolyd player */
					if (Upolyd)
						set_mon_data(&youmonst, &mons[u.umonnum], 0);
				}
			}
			if (hostile)
				continue;
			if (by_align && Role_if(a->role))
				goto make_artif; /* 'a' points to the desired one */
			else
				eligible[n++] = m;
		}

	if (n) {		      /* found at least one candidate */
		m = eligible[rn2(n)]; /* [0..n-1] */
		a = &artilist[m];

		/* make an appropriate object if necessary, then christen it */
make_artif:
		if (by_align) otmp = mksobj((int)a->otyp, true, false);
		otmp = oname(otmp, a->name);
		otmp->oartifact = m;
		artiexist[m] = true;
	} else {
		/* nothing appropriate could be found; return the original object */
		if (by_align) otmp = 0; /* (there was no original object) */
	}
	return otmp;
}

/*
 * Returns the full name (with articles and correct capitalization) of an
 * artifact named "name" if one exists, or NULL, it not.
 * The given name must be rather close to the real name for it to match.
 * The object type of the artifact is returned in otyp if the return value
 * is non-NULL.
 */
const char *artifact_name(const char *name, short *otyp) {
	const struct artifact *a;
	const char *aname;

	if (!strncmpi(name, "the ", 4)) name += 4;
	if (!strncmpi(name, "poisoned ", 9)) name += 9;

	for (a = artilist + 1; a->otyp; a++) {
		aname = a->name;
		if (!strncmpi(aname, "the ", 4)) aname += 4;
		if (!strcmpi(name, aname)) {
			*otyp = a->otyp;
			return a->name;
		}
	}

	return 0;
}

bool exist_artifact(int otyp, const char *name) {
	const struct artifact *a;
	bool *arex;

	if (otyp && *name)
		for (a = artilist + 1, arex = artiexist + 1; a->otyp; a++, arex++)
			if ((int)a->otyp == otyp && !strcmp(a->name, name))
				return *arex;
	return false;
}

void artifact_exists(struct obj *otmp, const char *name, bool mod) {
	const struct artifact *a;

	if (otmp && *name)
		for (a = artilist + 1; a->otyp; a++)
			if (a->otyp == otmp->otyp && !strcmp(a->name, name)) {
				int m = a - artilist;
				otmp->oartifact = (char)(mod ? m : 0);
				if (mod) {
					/* Light up Candle of Eternal Flame and
					 * Holy Spear of Light on creation.
					 */
					if (!artiexist[m] && artifact_light(otmp) &&
					    otmp->oartifact != ART_SUNSWORD)
						begin_burn(otmp, false);
					otmp->quan = 1; /* guarantee only one of this artifact */
					/* Artifacts are immune to unpolypile --ALI */
					if (is_hazy(otmp)) {
						stop_timer(UNPOLY_OBJ, obj_to_any(otmp));
						otmp->oldtyp = STRANGE_OBJECT;
					}
				}
				otmp->age = 0;
				if (otmp->otyp == RIN_INCREASE_DAMAGE)
					otmp->spe = 0;
				artiexist[m] = mod;
				break;
			}
	return;
}

int nartifact_exist(void) {
	int a = 0;
	int n = SIZE(artiexist);

	while (n > 1)
		if (artiexist[--n]) a++;

	return a;
}

bool spec_ability(struct obj *otmp, unsigned long abil) {
	const struct artifact *arti = get_artifact(otmp);

	return arti && (arti->spfx & abil);
}

/* used so that callers don't need to known about SPFX_ codes */
bool confers_luck(struct obj *obj) {
	/* might as well check for this too */
	if (obj->otyp == LUCKSTONE) return true;

	if (obj->otyp == FEDORA && obj == uarmh) return true;

	return obj->oartifact && spec_ability(obj, SPFX_LUCK);
}

/* used to check whether a monster is getting reflection from an artifact */
bool arti_reflects(struct obj *obj) {
	const struct artifact *arti = get_artifact(obj);

	if (arti) {
		/* while being worn */
		if ((obj->owornmask & ~W_ART) && (arti->spfx & SPFX_REFLECT))
			return true;
		/* just being carried */
		if (arti->cspfx & SPFX_REFLECT) return true;
	}
	return false;
}

/* returns true if name is restricted for otmp->otyp */
bool restrict_name(struct obj *otmp, const char *name) {
	const char *aname;

	if (!*name) return false;
	if (!strncmpi(name, "the ", 4)) name += 4;

	int lo, hi;
	obj_shuffle_range(otmp->otyp, &lo, &hi);

	/* Since almost every artifact is SPFX_RESTR, it doesn't cost
	   us much to do the string comparison before the spfx check.
	   Bug fix:  don't name multiple elven daggers "Sting".
	 */
	for (const struct artifact *a = artilist + 1; a->otyp; a++) {
		// disallowed if:
		//
		// either they're the same type of object
		// or they look similar to you
		//
		// when do they look similar?
		// When the artifact is in the objects shuffle group or the object descriptions are the same,
		// and you don't know what either of them looks like
		// (if you know what one looks like but not the other, that's ok; since the other is unidentified, you know it's not the same)

		const char *odescr = OBJ_DESCR(objects[otmp->otyp]);
		if (!odescr) odescr = "";

		const char *adescr = OBJ_DESCR(objects[a->otyp]);
		if (!adescr) adescr = "";

		if ((a->otyp == otmp->otyp) || ((((lo <= a->otyp) && (a->otyp <= hi)) || !strcmp(odescr, adescr)) && !objects[otmp->otyp].oc_name_known && !objects[a->otyp].oc_name_known)) {
			aname = a->name;
			if (!strncmpi(aname, "the ", 4)) aname += 4;
			if (!strcmpi(aname, name))
				return (a->spfx & (SPFX_NOGEN | SPFX_RESTR)) || otmp->quan > 1L;
		}
	}

	return false;
}

static bool attacks(int adtyp, struct obj *otmp) {
	const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return weap->attk.adtyp == adtyp;
	return false;
}

bool defends(int adtyp, struct obj *otmp) {
	const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return weap->defn.adtyp == adtyp;
	return false;
}

/* used for monsters */
bool protects(int adtyp, struct obj *otmp) {
	const struct artifact *weap;

	if ((weap = get_artifact(otmp)) != 0)
		return weap->cary.adtyp == adtyp;
	return false;
}

/*
 * a potential artifact has just been worn/wielded/picked-up or
 * unworn/unwielded/dropped.  Pickup/drop only set/reset the W_ART mask.
 */
void set_artifact_intrinsic(struct obj *otmp, bool on, long wp_mask) {
	long *mask = 0;
	const struct artifact *oart = get_artifact(otmp);
	uchar dtyp;
	long spfx;

	if (!oart) return;

	/* effects from the defn field */
	dtyp = (wp_mask != W_ART) ? oart->defn.adtyp : oart->cary.adtyp;

	if (dtyp == AD_FIRE)
		mask = &EFire_resistance;
	else if (dtyp == AD_COLD)
		mask = &ECold_resistance;
	else if (dtyp == AD_ELEC)
		mask = &EShock_resistance;
	else if (dtyp == AD_MAGM)
		mask = &EAntimagic;
	else if (dtyp == AD_DISN)
		mask = &EDisint_resistance;
	else if (dtyp == AD_DRST)
		mask = &EPoison_resistance;

	if (mask && wp_mask == W_ART && !on) {
		/* find out if some other artifact also confers this intrinsic */
		/* if so, leave the mask alone */
		struct obj *obj;
		for (obj = invent; obj; obj = obj->nobj)
			if (obj != otmp && obj->oartifact) {
				const struct artifact *art = get_artifact(obj);
				if (art->cary.adtyp == dtyp) {
					mask = NULL;
					break;
				}
			}
	}
	if (mask) {
		if (on)
			*mask |= wp_mask;
		else
			*mask &= ~wp_mask;
	}

	/* intrinsics from the spfx field; there could be more than one */
	spfx = (wp_mask != W_ART) ? oart->spfx : oart->cspfx;
	if (spfx && wp_mask == W_ART && !on) {
		/* don't change any spfx also conferred by other artifacts */
		struct obj *obj;
		for (obj = invent; obj; obj = obj->nobj)
			if (obj != otmp && obj->oartifact) {
				const struct artifact *art = get_artifact(obj);
				spfx &= ~art->cspfx;
			}
	}

	if (spfx & SPFX_SEARCH) {
		if (on)
			ESearching |= wp_mask;
		else
			ESearching &= ~wp_mask;
	}
	if (spfx & SPFX_HALRES) {
		/* make_hallucinated must (re)set the mask itself to get
		 * the display right */
		/* restoring needed because this is the only artifact intrinsic
		 * that can print a message--need to guard against being printed
		 * when restoring a game
		 */
		make_hallucinated((long)!on, restoring ? false : true, wp_mask);
	}
	if (spfx & SPFX_ESP) {
		if (on)
			ETelepat |= wp_mask;
		else
			ETelepat &= ~wp_mask;
		see_monsters();
	}
	if (spfx & SPFX_STLTH) {
		if (on)
			EStealth |= wp_mask;
		else
			EStealth &= ~wp_mask;
	}
	if (spfx & SPFX_REGEN) {
		if (on)
			ERegeneration |= wp_mask;
		else
			ERegeneration &= ~wp_mask;
	}
	if (spfx & SPFX_TCTRL) {
		if (on)
			ETeleport_control |= wp_mask;
		else
			ETeleport_control &= ~wp_mask;
	}
	/* weapon warning is specially handled in mon.c */
	if (spfx & SPFX_WARN) {
		if (spec_m2(otmp)) {
			if (on) {
				EWarn_of_mon |= wp_mask;
				context.warntype.obj |= spec_m2(otmp);
			} else {
				EWarn_of_mon &= ~wp_mask;
				context.warntype.obj &= ~spec_m2(otmp);
			}
			see_monsters();
		} else {
			if (on)
				EWarning |= wp_mask;
			else
				EWarning &= ~wp_mask;
		}
	}
	if (spfx & SPFX_EREGEN) {
		if (on)
			EEnergy_regeneration |= wp_mask;
		else
			EEnergy_regeneration &= ~wp_mask;
	}
	if (spfx & SPFX_HSPDAM) {
		if (on)
			EHalf_spell_damage |= wp_mask;
		else
			EHalf_spell_damage &= ~wp_mask;
	}
	if (spfx & SPFX_HPHDAM) {
		if (on)
			EHalf_physical_damage |= wp_mask;
		else
			EHalf_physical_damage &= ~wp_mask;
	}
	if (spfx & SPFX_XRAY) {
		/* this assumes that no one else is using xray_range */
		if (on)
			u.xray_range = 3;
		else
			u.xray_range = -1;
		vision_full_recalc = 1;
	}
	/* KMH -- Reflection when wielded */
	if ((spfx & SPFX_REFLECT) && (wp_mask & W_WEP)) {
		if (on)
			EReflecting |= wp_mask;
		else
			EReflecting &= ~wp_mask;
	}

	if (wp_mask == W_ART && !on && oart->inv_prop) {
		/* might have to turn off invoked power too */
		if (oart->inv_prop <= LAST_PROP &&
		    (u.uprops[oart->inv_prop].extrinsic & W_ARTI))
			arti_invoke(otmp);
	}
}

/*
 * creature (usually player) tries to touch (pick up or wield) an artifact obj.
 * Returns 0 if the object refuses to be touched.
 * This routine does not change any object chains.
 * Ignores such things as gauntlets, assuming the artifact is not
 * fooled by such trappings.
 */
int touch_artifact(struct obj *obj, struct monst *mon) {
	const struct artifact *oart = get_artifact(obj);
	bool badclass, badalign;

	if (!oart) return 1;

	/* [ALI] Thiefbane has a special affinity with shopkeepers */
	if (mon->isshk && obj->oartifact == ART_THIEFBANE) return 1;

	const bool yours = (mon == &youmonst);
	/* all quest artifacts are self-willed; it this ever changes, `badclass'
	   will have to be extended to explicitly include quest artifacts */
	const bool self_willed = ((oart->spfx & SPFX_INTEL) != 0);
	if (yours) {
		badclass = self_willed &&
			   ((oart->role != NON_PM && !Role_if(oart->role)) ||
			    (oart->race != NON_PM && !Race_if(oart->race)));
		badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE &&
			   (oart->alignment != u.ualign.type || u.ualign.record < 0);
	} else if (!is_covetous(mon->data) && !is_mplayer(mon->data)) {
		badclass = self_willed &&
			   oart->role != NON_PM && oart != &artilist[ART_EXCALIBUR];
		badalign = (oart->spfx & SPFX_RESTR) && oart->alignment != A_NONE &&
			   (oart->alignment != mon_aligntyp(mon));
	} else { /* an M3_WANTSxxx monster or a fake player */
		/* special monsters trying to take the Amulet, invocation tools or
		   quest item can touch anything except for `spec_applies' artifacts */
		badclass = badalign = false;
	}
	/* weapons which attack specific categories of monsters are
	   bad for them even if their alignments happen to match */
	if (!badalign && (oart->spfx & SPFX_DBONUS) != 0) {
		struct artifact tmp;

		tmp = *oart;
		tmp.spfx &= SPFX_DBONUS;
		badalign = spec_applies(&tmp, mon);
	}

	if (((badclass || badalign) && self_willed) ||
	    (badalign && (!yours || !rn2(4)))) {
		int dmg;
		char buf[BUFSZ];

		if (!yours) return 0;
		pline("You are blasted by %s power!", s_suffix(the(xname(obj))));
		dmg = d((Antimagic ? 6 : 8), (self_willed ? 10 : 6));
		sprintf(buf, "touching %s", oart->name);
		losehp(dmg, buf, KILLED_BY);
		exercise(A_WIS, false);
	}

	/* can pick it up unless you're totally non-synch'd with the artifact */
	if (badclass && badalign && self_willed) {
		if (yours) pline("%s your grasp!", Tobjnam(obj, "evade"));
		return 0;
	}

	return 1;
}

/* decide whether an artifact itself is vulnerable to a particular type
   of erosion damage, independent of the properties of its bearer */
bool arti_immune(struct obj *obj, int dtyp) {
	const struct artifact *weap = get_artifact(obj);

	if (!weap) return false;
	if (dtyp == AD_PHYS) return false; /* nothing is immune to phys dmg */

	return weap->attk.adtyp == dtyp
	       || weap->defn.adtyp == dtyp
	       || weap->cary.adtyp == dtyp;
}


/* decide whether an artifact's special attacks apply against mtmp */
static int spec_applies(const struct artifact *weap, struct monst *mtmp) {
	int retval = true;
	struct permonst *ptr;

	if (!(weap->spfx & (SPFX_DBONUS | SPFX_ATTK)))
		return weap->attk.adtyp == AD_PHYS;

	const bool yours = (mtmp == &youmonst);
	ptr = mtmp->data;

	/* [ALI] Modified to support multiple DBONUS and ATTK flags set.
	 * Not all combinations are possible because many DBONUS flags
	 * use mtype and would conflict. Where combinations are possible,
	 * both checks must pass in order for the special attack to
	 * apply against mtmp.
	 */
	if (weap->spfx & SPFX_DMONS) {
		retval &= (ptr == &mons[(int)weap->mtype]);
	} else if (weap->spfx & SPFX_DCLAS) {
		retval &= (weap->mtype == (unsigned long)ptr->mlet);
	} else if (weap->spfx & SPFX_DFLAG1) {
		retval &= ((ptr->mflags1 & weap->mtype) != 0L);
	} else if (weap->spfx & SPFX_DFLAG2) {
		retval &= ((ptr->mflags2 & weap->mtype) || (yours &&
							    ((!Upolyd && (urace.selfmask & weap->mtype)) ||
							     ((weap->mtype & M2_WERE) && u.ulycn >= LOW_PM))));
	}
	if (weap->spfx & SPFX_DALIGN) {
		retval &= yours ? (u.ualign.type != weap->alignment) :
				  (ptr->maligntyp == A_NONE ||
				   sgn(ptr->maligntyp) != weap->alignment);
	}
	if (weap->spfx & SPFX_ATTK) {
		struct obj *defending_weapon = (yours ? uwep : MON_WEP(mtmp));

		if (defending_weapon && defending_weapon->oartifact &&
		    defends((int)weap->attk.adtyp, defending_weapon))
			return false;
		switch (weap->attk.adtyp) {
			case AD_FIRE:
				if (yours ? Fire_resistance : resists_fire(mtmp))
					retval = false;
				break;
			case AD_COLD:
				if (yours ? Cold_resistance : resists_cold(mtmp))
					retval = false;
				break;
			case AD_ELEC:
				if (yours ? Shock_resistance : resists_elec(mtmp))
					retval = false;
				break;
			case AD_MAGM:
			case AD_STUN:
				if (yours ? Antimagic : (rn2(100) < ptr->mr))
					retval = false;
				break;
			case AD_DRST:
				if (yours ? Poison_resistance : resists_poison(mtmp))
					retval = false;
				break;
			case AD_DRLI:
				if (yours ? Drain_resistance : resists_drli(mtmp))
					retval = false;
				break;
			case AD_STON:
				if (yours ? Stone_resistance : resists_ston(mtmp))
					retval = false;
				break;
			default:
				impossible("Weird weapon special attack.");
		}
	}
	return retval;
}

/* return the M2 flags of monster that an artifact's special attacks apply against */
long spec_m2(struct obj *otmp) {
	const struct artifact *artifact = get_artifact(otmp);
	if (artifact)
		return artifact->mtype;
	return 0L;
}

/* special attack bonus */
int spec_abon(struct obj *otmp, struct monst *mon) {
	const struct artifact *weap = get_artifact(otmp);

	/* no need for an extra check for `NO_ATTK' because this will
	   always return 0 for any artifact which has that attribute */

	if (weap && weap->attk.damn && spec_applies(weap, mon))
		return weap->attk.damn;
	return 0;
}

/* special damage bonus */
int spec_dbon(struct obj *otmp, struct monst *mon, int tmp) {
	const struct artifact *weap = get_artifact(otmp);

	if (!weap || (weap->attk.adtyp == AD_PHYS && /* check for `NO_ATTK' */
		      weap->attk.damn == 0 && weap->attk.damd == 0))
		spec_dbon_applies = false;
	else
		spec_dbon_applies = spec_applies(weap, mon);

	if (spec_dbon_applies)
		return weap->attk.damd ? (int)weap->attk.damd :
					 /* [ALI] Unlike melee weapons, damd == 0 means no
		        * bonus for launchers.
		        */
			       is_launcher(otmp) ? 0 : max(tmp, 1);
	return 0;
}

/* add identified artifact to discoveries list */
void discover_artifact(int m) {
	int i;

	/* look for this artifact in the discoveries list;
	   if we hit an empty slot then it's not present, so add it */
	for (i = 0; i < NROFARTIFACTS; i++)
		if (artidisco[i] == 0 || artidisco[i] == m) {
			artidisco[i] = m;
			return;
		}
	/* there is one slot per artifact, so we should never reach the
	   end without either finding the artifact or an empty slot... */
	impossible("couldn't discover artifact (%d)", m);
}

/* used to decide whether an artifact has been fully identified */
bool undiscovered_artifact(xchar m) {
	int i;

	/* look for this artifact in the discoveries list;
	   if we hit an empty slot then it's undiscovered */
	for (i = 0; i < NROFARTIFACTS; i++)
		if (artidisco[i] == m)
			return false;
		else if (artidisco[i] == 0)
			break;
	return true;
}

/* display a list of discovered artifacts; return their count */
int disp_artifact_discoveries(winid tmpwin /* supplied by dodiscover() */) {
	int i, m, otyp;
	char buf[BUFSZ];
	anything any;

	any.a_void = 0;
	for (i = 0; i < NROFARTIFACTS; i++) {
		if (artidisco[i] == 0) break; /* empty slot implies end of list */
		if (i == 0)
			add_menu(tmpwin, NO_GLYPH, &any, 0, 0, iflags.menu_headings,
				 "Artifacts", MENU_UNSELECTED);
		m = artidisco[i];
		otyp = artilist[m].otyp;
		sprintf(buf, "  %s [%s %s]", artiname(m),
			align_str(artilist[m].alignment), simple_typename(otyp));
		add_menu(tmpwin, objnum_to_glyph(otyp), &any, 0, 0, ATR_NONE,
			 buf, MENU_UNSELECTED);
	}
	return i;
}

/*
 * Magicbane's intrinsic magic is incompatible with normal
 * enchantment magic.  Thus, its effects have a negative
 * dependence on spe.  Against low mr victims, it typically
 * does "double athame" damage, 2d4.  Occasionally, it will
 * cast unbalancing magic which effectively averages out to
 * 4d4 damage (3d4 against high mr victims), for spe = 0.
 *
 * Prior to 3.4.1, the cancel (aka purge) effect always
 * included the scare effect too; now it's one or the other.
 * Likewise, the stun effect won't be combined with either
 * of those two; it will be chosen separately or possibly
 * used as a fallback when scare or cancel fails.
 *
 * [Historical note: a change to artifact_hit() for 3.4.0
 * unintentionally made all of Magicbane's special effects
 * be blocked if the defender successfully saved against a
 * stun attack.  As of 3.4.1, those effects can occur but
 * will be slightly less likely than they were in 3.3.x.]
 */
#define MB_MAX_DIEROLL 8 /* rolls above this aren't magical */
static const char *const mb_verb[2][4] = {
	{"probe", "stun", "scare", "cancel"},
	{"prod", "amaze", "tickle", "purge"},
};
#define MB_INDEX_PROBE	0
#define MB_INDEX_STUN	1
#define MB_INDEX_SCARE	2
#define MB_INDEX_CANCEL 3

/* called when someone is being hit by Magicbane */
static bool Mb_hit(
	struct monst *magr, struct monst *mdef, /* attacker and defender */
	struct obj *mb,				/* Magicbane */
	int *dmgptr,				/* extra damage target will suffer */
	int dieroll,				/* d20 that has already scored a hit */
	bool vis,				/* whether the action can be seen */
	char *hittee /* target's name: "you" or mon_nam(mdef) */) {
	struct permonst *old_uasmon;
	const char *verb;
	const bool youattack = (magr == &youmonst),
		   youdefend = (mdef == &youmonst);
	bool do_stun, do_confuse, resisted = false;
	int attack_indx, scare_dieroll = MB_MAX_DIEROLL / 2;

	bool result = false; /* no message given yet */
	/* the most severe effects are less likely at higher enchantment */
	if (mb->spe >= 3)
		scare_dieroll /= (1 << (mb->spe / 3));
	/* if target successfully resisted the artifact damage bonus,
	   reduce overall likelihood of the assorted special effects */
	if (!spec_dbon_applies) dieroll += 1;

	/* might stun even when attempting a more severe effect, but
	   in that case it will only happen if the other effect fails;
	   extra damage will apply regardless; 3.4.1: sometimes might
	   just probe even when it hasn't been enchanted */
	do_stun = max(mb->spe, 0) < rn2(spec_dbon_applies ? 11 : 7);

	/* the special effects also boost physical damage; increments are
	   generally cumulative, but since the stun effect is based on a
	   different criterium its damage might not be included; the base
	   damage is either 1d4 (athame) or 2d4 (athame+spec_dbon) depending
	   on target's resistance check against AD_STUN (handled by caller)
	   [note that a successful save against AD_STUN doesn't actually
	   prevent the target from ending up stunned] */
	attack_indx = MB_INDEX_PROBE;
	*dmgptr += rnd(4); /* (2..3)d4 */
	if (do_stun) {
		attack_indx = MB_INDEX_STUN;
		*dmgptr += rnd(4); /* (3..4)d4 */
	}
	if (dieroll <= scare_dieroll) {
		attack_indx = MB_INDEX_SCARE;
		*dmgptr += rnd(4); /* (3..5)d4 */
	}
	if (dieroll <= (scare_dieroll / 2)) {
		attack_indx = MB_INDEX_CANCEL;
		*dmgptr += rnd(4); /* (4..6)d4 */
	}

	/* give the hit message prior to inflicting the effects */
	verb = mb_verb[!!Hallucination][attack_indx];
	if (youattack || youdefend || vis) {
		result = true;
		pline("The magic-absorbing blade %s %s!",
		      vtense(NULL, verb), hittee);
		/* assume probing has some sort of noticeable feedback
		   even if it is being done by one monster to another */
		if (attack_indx == MB_INDEX_PROBE && !canspotmon(mdef))
			map_invisible(mdef->mx, mdef->my);
	}

	/* now perform special effects */
	switch (attack_indx) {
		case MB_INDEX_CANCEL:
			old_uasmon = youmonst.data;
			/* No mdef->mcan check: even a cancelled monster can be polymorphed
			 * into a golem, and the "cancel" effect acts as if some magical
			 * energy remains in spellcasting defenders to be absorbed later.
			 */
			if (!cancel_monst(mdef, mb, youattack, false, false)) {
				resisted = true;
			} else {
				do_stun = false;
				if (youdefend) {
					if (youmonst.data != old_uasmon)
						*dmgptr = 0; /* rehumanized, so no more damage */
					if (u.uenmax > 0) {
						pline("You lose magical energy!");
						u.uenmax--;
						if (u.uen > 0) u.uen--;
						context.botl = 1;
					}
				} else {
					if (mdef->data == &mons[PM_CLAY_GOLEM])
						mdef->mhp = 1; /* cancelled clay golems will die */
					if (youattack && attacktype(mdef->data, AT_MAGC)) {
						pline("You absorb magical energy!");
						u.uenmax++;
						u.uen++;
						context.botl = 1;
					}
				}
			}
			break;

		case MB_INDEX_SCARE:
			if (youdefend) {
				if (Antimagic) {
					resisted = true;
				} else {
					nomul(-3);
					nomovemsg = "";
					if (magr && magr == u.ustuck && sticks(youmonst.data)) {
						setustuck(NULL);
						pline("You release %s!", mon_nam(magr));
					}
				}
			} else {
				if (rn2(2) && resist(mdef, WEAPON_CLASS, 0, NOTELL))
					resisted = true;
				else
					monflee(mdef, 3, false, (mdef->mhp > *dmgptr));
			}
			if (!resisted) do_stun = false;
			break;

		case MB_INDEX_STUN:
			do_stun = true; /* (this is redundant...) */
			break;

		case MB_INDEX_PROBE:
			if (youattack && (mb->spe == 0 || !rn2(3 * abs(mb->spe)))) {
				pline("The %s is insightful.", verb);
				/* pre-damage status */
				probe_monster(mdef);
			}
			break;
	}
	/* stun if that was selected and a worse effect didn't occur */
	if (do_stun) {
		if (youdefend)
			make_stunned((HStun + 3), false);
		else
			mdef->mstun = 1;
		/* avoid extra stun message below if we used mb_verb["stun"] above */
		if (attack_indx == MB_INDEX_STUN) do_stun = false;
	}
	/* lastly, all this magic can be confusing... */
	do_confuse = !rn2(12);
	if (do_confuse) {
		if (youdefend)
			make_confused(HConfusion + 4, false);
		else
			mdef->mconf = 1;
	}

	if (youattack || youdefend || vis) {
		upstart(hittee); /* capitalize */
		if (resisted) {
			pline("%s resist%s!", hittee, youdefend ? "" : "s");
			shieldeff(youdefend ? u.ux : mdef->mx,
				  youdefend ? u.uy : mdef->my);
		}
		if ((do_stun || do_confuse) && flags.verbose) {
			char buf[BUFSZ];

			buf[0] = '\0';
			if (do_stun) strcat(buf, "stunned");
			if (do_stun && do_confuse) strcat(buf, " and ");
			if (do_confuse) strcat(buf, "confused");
			pline("%s %s %s%c", hittee, youdefend ? "are" : "is",
			      buf, (do_stun && do_confuse) ? '!' : '.');
		}
	}

	return result;
}

/* Function used when someone attacks someone else with an artifact
 * weapon.  Only adds the special (artifact) damage, and returns a 1 if it
 * did something special (in which case the caller won't print the normal
 * hit message).  This should be called once upon every artifact attack;
 * dmgval() no longer takes artifact bonuses into account.  Possible
 * extension: change the killer so that when an orc kills you with
 * Stormbringer it's "killed by Stormbringer" instead of "killed by an orc".
 */
bool artifact_hit(struct monst *magr, struct monst *mdef, struct obj *otmp, int *dmgptr, int dieroll /* needed for Magicbane and vorpal blades */) {
	const bool youattack = (magr == &youmonst);
	const bool youdefend = (mdef == &youmonst);
	const bool vis = (!youattack && magr && cansee(magr->mx, magr->my)) || (!youdefend && cansee(mdef->mx, mdef->my)) || (youattack && u.uswallow && mdef == u.ustuck && !Blind);
	const char *wepdesc;
	static const char you[] = "you";
	char hittee[BUFSIZ];
	bool special_applies;

	strcpy(hittee, youdefend ? you : mon_nam(mdef));

	/* The following takes care of most of the damage, but not all--
	 * the exception being for level draining, which is specially
	 * handled.  Messages are done in this function, however.
	 */
	*dmgptr += spec_dbon(otmp, mdef, *dmgptr);

	if (spec_dbon_applies) {
		special_applies = true;
	} else {
		const struct artifact *weap = get_artifact(otmp);
		special_applies = weap && spec_applies(weap, mdef);
	}

	if (youattack && youdefend) {
		impossible("attacking yourself with weapon?");
		return false;
	}

	const bool realizes_damage = (youdefend || vis ||
				      /* feel the effect even if not seen */
				      (youattack && mdef == u.ustuck));

	/* the four basic attacks: fire, cold, shock and missiles */
	if (attacks(AD_FIRE, otmp)) {
		if (realizes_damage)
			pline("The fiery blade %s %s%c",
			      !spec_dbon_applies ? "hits" :
						   (mdef->data == &mons[PM_WATER_ELEMENTAL]) ?
						   "vaporizes part of" :
						   "burns",
			      hittee, !spec_dbon_applies ? '.' : '!');
		if (!rn2(4)) destroy_mitem(mdef, POTION_CLASS, AD_FIRE);
		if (!rn2(4)) destroy_mitem(mdef, SCROLL_CLASS, AD_FIRE);
		if (!rn2(7)) destroy_mitem(mdef, SPBOOK_CLASS, AD_FIRE);
		if (youdefend && Slimed) burn_away_slime();
		return realizes_damage;
	}
	if (attacks(AD_COLD, otmp)) {
		if (realizes_damage)
			pline("The ice-cold blade %s %s%c",
			      !spec_dbon_applies ? "hits" : "freezes",
			      hittee, !spec_dbon_applies ? '.' : '!');
		if (!rn2(4)) destroy_mitem(mdef, POTION_CLASS, AD_COLD);
		return realizes_damage;
	}
	if (attacks(AD_ELEC, otmp)) {
		if (realizes_damage)
			pline("The massive hammer hits%s %s%c",
			      !spec_dbon_applies ? "" : "!  Lightning strikes",
			      hittee, !spec_dbon_applies ? '.' : '!');
		if (!rn2(5)) destroy_mitem(mdef, RING_CLASS, AD_ELEC);
		if (!rn2(5)) destroy_mitem(mdef, WAND_CLASS, AD_ELEC);
		return realizes_damage;
	}
	if (attacks(AD_MAGM, otmp)) {
		if (realizes_damage)
			pline("The imaginary widget hits%s %s%c",
			      !spec_dbon_applies ? "" :
						   "!  A hail of magic missiles strikes",
			      hittee, !spec_dbon_applies ? '.' : '!');
		return realizes_damage;
	}

	if (attacks(AD_STUN, otmp) && dieroll <= MB_MAX_DIEROLL) {
		/* Magicbane's special attacks (possibly modifies hittee[]) */
		return Mb_hit(magr, mdef, otmp, dmgptr, dieroll, vis, hittee);
	}

	if ((!spec_dbon_applies && !spec_ability(otmp, SPFX_BEHEAD)) ||
	    !special_applies) {
		/* since damage bonus didn't apply, nothing more to do;
		   no further attacks have side-effects on inventory */
		/* [ALI] The Tsurugi of Muramasa has no damage bonus but
		   is handled below so avoid early exit if SPFX_BEHEAD set
		   and the defender is vulnerable */
		return false;
	}

	/* STEPHEN WHITE'S NEW CODE */
	if (otmp->oartifact == ART_SERPENT_S_TONGUE) {
		otmp->dknown = true;
		pline("The twisted blade poisons %s!",
		      youdefend ? "you" : mon_nam(mdef));
		if (youdefend ? Poison_resistance : resists_poison(mdef)) {
			if (youdefend)
				pline("You are not affected by the poison.");
			else
				pline("%s seems unaffected by the poison.", Monnam(mdef));
			return true;
		}
		switch (rnd(10)) {
			case 1:
			case 2:
			case 3:
			case 4:
				*dmgptr += d(1, 6) + 2;
				break;
			case 5:
			case 6:
			case 7:
				*dmgptr += d(2, 6) + 4;
				break;
			case 8:
			case 9:
				*dmgptr += d(3, 6) + 6;
				break;
			case 10:
				pline("The poison was deadly...");
				*dmgptr = 2 * (youdefend ? Upolyd ? u.mh : u.uhp : mdef->mhp) + FATAL_DAMAGE_MODIFIER;
				break;
		}
		return true;
	}

	if (otmp->oartifact == ART_DOOMBLADE && dieroll < 6) {
		if (youattack)
			pline("You plunge the Doomblade deeply into %s!",
			      mon_nam(mdef));
		else
			pline("%s plunges the Doomblade deeply into %s!",
			      Monnam(magr), hittee);
		*dmgptr += rnd(4) * 5;
		return true;
	}
	/* END OF STEPHEN WHITE'S NEW CODE */

#if 0
	if (otmp->oartifact == ART_SCALPEL && dieroll < 5) {
		/* faster than a speeding bullet is the Gray Mouser... */
		pline("There is a flurry of blows!");
		/* I suppose this could theoretically continue forever... */
		while (dieroll < 5) {
			*dmgptr += rnd(8) + 1 + otmp->spe;
			dieroll = rn2(11);
		}
	}
	if (otmp->oartifact == ART_HEARTSEEKER && dieroll < 3) {
		/* this weapon just sounds nasty... yuck... */
		if (!youdefend) {
			pline("You plunge Heartseeker into %s!",mon_nam(mdef));
		} else {
			pline("%s plunges Heartseeker into you!",mon_nam(mdef));
		}
		*dmgptr += rnd(6)+rnd(6)+rnd(6)+rnd(6)+4;
	}
#endif

	/* We really want "on a natural 20" but Nethack does it in */
	/* reverse from AD&D. */
	if (spec_ability(otmp, SPFX_BEHEAD)) {
		if (otmp->oartifact == ART_TSURUGI_OF_MURAMASA && dieroll < 3) {
			wepdesc = "The razor-sharp blade";
			/* not really beheading, but so close, why add another SPFX */
			if (youattack && u.uswallow && mdef == u.ustuck) {
				pline("You slice %s wide open!", mon_nam(mdef));
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				return true;
			}
			if (!youdefend) {
				/* allow normal cutworm() call to add extra damage */
				if (notonhead)
					return false;

				if (bigmonst(mdef->data)) {
					if (youattack)
						pline("You slice deeply into %s!",
						      mon_nam(mdef));
					else if (vis)
						pline("%s cuts deeply into %s!",
						      Monnam(magr), hittee);
					*dmgptr *= 2;
					return true;
				}
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				pline("%s cuts %s in half!", wepdesc, mon_nam(mdef));
				otmp->dknown = true;
				return true;
			} else {
				/* Invulnerable player won't be bisected */
				if (bigmonst(youmonst.data) || Invulnerable) {
					pline("%s cuts deeply into you!",
					      magr ? Monnam(magr) : wepdesc);
					*dmgptr *= 2;
					return true;
				}

				/* Players with negative AC's take less damage instead
				 * of just not getting hit.  We must add a large enough
				 * value to the damage so that this reduction in
				 * damage does not prevent death.
				 */
				*dmgptr = 2 * (Upolyd ? u.mh : u.uhp) + FATAL_DAMAGE_MODIFIER;
				pline("%s cuts you in half!", wepdesc);
				otmp->dknown = true;
				return true;
			}
		} else if ((dieroll < 3 || otmp->oartifact == ART_VORPAL_BLADE) &&
			   mdef->data == &mons[PM_JABBERWOCK]) {
			static const char *const behead_msg[2] = {
				"%s beheads %s!",
				"%s decapitates %s!"};

			if (youattack && u.uswallow && mdef == u.ustuck)
				return false;
			wepdesc = artilist[otmp->oartifact].name;
			if (!youdefend) {
				if (!has_head(mdef->data) || notonhead || u.uswallow) {
					if (youattack)
						pline("Somehow, you miss %s wildly.",
						      mon_nam(mdef));
					else if (vis)
						pline("Somehow, %s misses wildly.",
						      mon_nam(magr));
					*dmgptr = 0;
					return youattack || vis;
				}
				if (noncorporeal(mdef->data) || amorphous(mdef->data)) {
					pline("%s slices through %s %s.", wepdesc,
					      s_suffix(mon_nam(mdef)),
					      mbodypart(mdef, NECK));
					return true;
				}
				*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				pline(behead_msg[rn2(SIZE(behead_msg))],
				      wepdesc, mon_nam(mdef));
				otmp->dknown = true;
				return true;
			} else {
				if (!has_head(youmonst.data)) {
					pline("Somehow, %s misses you wildly.",
					      magr ? mon_nam(magr) : wepdesc);
					*dmgptr = 0;
					return true;
				}
				if (noncorporeal(youmonst.data) || amorphous(youmonst.data)) {
					pline("%s slices through your %s.",
					      wepdesc, body_part(NECK));
					return true;
				}
				*dmgptr = 2 * (Upolyd ? u.mh : u.uhp) + FATAL_DAMAGE_MODIFIER;
				if (Invulnerable) {
					pline("%s slices into your %s.",
					      wepdesc, body_part(NECK));
					return true;
				}
				pline(behead_msg[rn2(SIZE(behead_msg))],
				      wepdesc, "you");
				otmp->dknown = true;
				/* Should amulets fall off? */
				return true;
			}
		}
	}
	if (spec_ability(otmp, SPFX_DRLI)) {
		if (!youdefend) {
			if (!resists_drli(mdef)) {
				if (vis) {
					if (otmp->oartifact == ART_STORMBRINGER)
						pline("The %s blade draws the life from %s!",
						      hcolor(NH_BLACK),
						      mon_nam(mdef));
#if 0 /* OBSOLETE */
					else if(otmp->oartifact == ART_TENTACLE_STAFF)
						pline("The writhing tentacles draw the life from %s!",
						      mon_nam(mdef));
#endif
					else
						pline("%s draws the life from %s!",
						      The(distant_name(otmp, xname)),
						      mon_nam(mdef));
				}
				if (mdef->m_lev == 0) {
					*dmgptr = 2 * mdef->mhp + FATAL_DAMAGE_MODIFIER;
				} else {
					int drain = monhp_per_lvl(mdef);
					*dmgptr += drain;
					mdef->mhpmax -= drain;
					mdef->m_lev--;
					drain /= 2;
					if (drain) healup(drain, 0, false, false);
				}
				return vis;
			}
		} else if (!Drain_resistance) { /* youdefend */
			int oldhpmax = u.uhpmax;

			if (Blind)
				pline("You feel an %s drain your life!",
				      otmp->oartifact == ART_STORMBRINGER ?
					      "unholy blade" :
					      "object");
			else if (otmp->oartifact == ART_STORMBRINGER)
				pline("The %s blade drains your life!",
				      hcolor(NH_BLACK));
			else
				pline("%s drains your life!",
				      The(distant_name(otmp, xname)));
			losexp("life drainage", false);
			if (magr && magr->mhp < magr->mhpmax) {
				magr->mhp += (oldhpmax - u.uhpmax) / 2;
				if (magr->mhp > magr->mhpmax) magr->mhp = magr->mhpmax;
			}
			return true;
		}
	}
	/* WAC -- 1/6 chance of cancellation with foobane weapons */
	if (otmp->oartifact == ART_GIANTKILLER ||
	    otmp->oartifact == ART_ORCRIST ||
	    otmp->oartifact == ART_DRAGONBANE ||
	    otmp->oartifact == ART_DEMONBANE ||
	    otmp->oartifact == ART_WEREBANE ||
	    otmp->oartifact == ART_TROLLSBANE ||
	    otmp->oartifact == ART_THIEFBANE ||
	    otmp->oartifact == ART_OGRESMASHER ||
	    otmp->oartifact == ART_ELFRIST) {
		if (dieroll < 4) {
			if (realizes_damage) {
				pline("%s %s!", The(distant_name(otmp, xname)), Blind ? "roars deafeningly" : "shines brilliantly");
				pline("It strikes %s!", hittee);
			}
			cancel_monst(mdef, otmp, youattack, true, magr == mdef);
			return true;
		}
	}
	return false;
}

static const char recharge_type[] = {ALLOW_COUNT, ALL_CLASSES, 0};
static const char invoke_types[] = {ALL_CLASSES, 0};
/* #invoke: an "ugly check" filters out most objects */

int doinvoke(void) {
	struct obj *obj;

	obj = getobj(invoke_types, "invoke");
	if (!obj) return 0;
	if (obj->oartifact && !touch_artifact(obj, &youmonst)) return 1;
	return arti_invoke(obj);
}

static int arti_invoke(struct obj *obj) {
	const struct artifact *oart = get_artifact(obj);
	struct monst *mtmp;
	struct permonst *pm;

	int summon_loop;
	int unseen;
	/*
	   int kill_loop;
	   */

	if (!oart || !oart->inv_prop) {
		if (obj->otyp == CRYSTAL_BALL)
			use_crystal_ball(obj);
		else
			pline("%s", "Nothing happens.");
		return 1;
	}

	if (oart->inv_prop > LAST_PROP) {
		/* It's a special power, not "just" a property */
		if (obj->age > monstermoves) {
			/* the artifact is tired :-) */
			pline("You feel that %s %s ignoring you.",
			      the(xname(obj)), otense(obj, "are"));
			/* and just got more so; patience is essential... */
			obj->age += (long)d(3, 10);
			return 1;
		}
		obj->age = monstermoves + rnz(100);

		switch (oart->inv_prop) {
			case TAMING: {
				struct obj pseudo;

				pseudo = zeroobj; /* neither cursed nor blessed */
				pseudo.otyp = SCR_TAMING;
				seffects(&pseudo);
				break;
			}
			case HEALING: {
				int healamt = (u.uhpmax + 1 - u.uhp) / 2;
				long creamed = (long)u.ucreamed;

				if (Upolyd) healamt = (u.mhmax + 1 - u.mh) / 2;
				if (healamt || Sick || Slimed || Blinded > creamed)
					pline("You feel better.");
				else
					goto nothing_special;
				if (healamt > 0) {
					if (Upolyd)
						u.mh += healamt;
					else
						u.uhp += healamt;
				}
				if (Sick) make_sick(0L, NULL, false, SICK_ALL);
				if (Slimed) make_slimed(0, NULL);
				if (Blinded > creamed) make_blinded(creamed, false);
				context.botl = 1;
				break;
			}
			case ENERGY_BOOST: {
				int epboost = (u.uenmax + 1 - u.uen) / 2;
				if (epboost > 120)
					epboost = 120; /* arbitrary */
				else if (epboost < 12)
					epboost = u.uenmax - u.uen;
				if (epboost) {
					pline("You feel re-energized.");
					u.uen += epboost;
					context.botl = 1;
				} else
					goto nothing_special;
				break;
			}
			case UNTRAP: {
				if (!untrap(true)) {
					obj->age = 0; /* don't charge for changing their mind */
					return 0;
				}
				break;
			}
			case CHARGE_OBJ: {
				struct obj *otmp = getobj(recharge_type, "charge");

				if (!otmp) {
					obj->age = 0;
					return 0;
				}
				bool b_effect = obj->blessed &&
						(Role_switch == oart->role || !oart->role);
				recharge(otmp, b_effect ? 1 : obj->cursed ? -1 : 0);
				update_inventory();
				break;
			}
			case LEV_TELE:
				level_tele();
				break;
			case LIGHT_AREA:
				if (!Blind)
					pline("%s shines brightly for an instant!", The(xname(obj)));
				else
					pline("%s grows warm for a second!", The(xname(obj)));
				litroom(true, obj);
				vision_recalc(0);
				if (is_undead(youmonst.data)) {
					pline("You burn in the radiance!");
					/* This is ground zero.  Not good news ... */
					u.uhp /= 100;
					if (u.uhp < 1) {
						u.uhp = 0;
						killer.format = KILLED_BY;
						killer.name = nhsdupz("the Holy Spear of Light");
						done(DIED);
					}
				}
				/* Undead and Demonics can't stand the light */
				unseen = 0;
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
					if (DEADMONSTER(mtmp)) continue;
					if (distu(mtmp->mx, mtmp->my) > 9 * 9) continue;
					if (couldsee(mtmp->mx, mtmp->my) &&
					    (is_undead(mtmp->data) || is_demon(mtmp->data)) &&
					    !resist(mtmp, '\0', 0, TELL)) {
						if (canseemon(mtmp))
							pline("%s burns in the radiance!", Monnam(mtmp));
						else
							unseen++;
						/* damage depends on distance, divisor ranges from 10 to 2 */
						mtmp->mhp /= (10 - (distu(mtmp->mx, mtmp->my) / 10));
						if (mtmp->mhp < 1) mtmp->mhp = 1;
					}
				}
				if (unseen)
					You_hearf("%s of intense pain!", unseen > 1 ? "cries" : "a cry");
				break;
			case DEATH_GAZE:
				if (u.uluck < -9) {
					pline("The Eye turns on you!");
					u.uhp = 0;
					killer.format = KILLED_BY;
					killer.name = nhsdupz("the Eye of the Beholder");
					done(DIED);
				}
				pline("The Eye looks around with its icy gaze!");
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
					if (DEADMONSTER(mtmp)) continue;
					/* The eye is never blind ... */
					if (couldsee(mtmp->mx, mtmp->my) && !is_undead(mtmp->data)) {
						pline("%s screams in agony!", Monnam(mtmp));
						mtmp->mhp /= 3;
						if (mtmp->mhp < 1) mtmp->mhp = 1;
					}
				}
				/* Tsk,tsk.. */
				adjalign(-3);
				u.uluck -= 3;
				break;
			case SUMMON_UNDEAD:
				if (u.uluck < -9) {
					u.uhp -= rn2(20) + 5;
					pline("The Hand claws you with its icy nails!");
					if (u.uhp <= 0) {
						killer.format = KILLED_BY;
						killer.name = nhsdupz("the Hand of Vecna");
						done(DIED);
					}
				}
				summon_loop = rn2(4) + 4;
				pline("Creatures from the grave surround you!");
				do {
					switch (rn2(6) + 1) {
						case 1:
							pm = mkclass(S_VAMPIRE, 0);
							break;
						case 2:
						case 3:
							pm = mkclass(S_ZOMBIE, 0);
							break;
						case 4:
							pm = mkclass(S_MUMMY, 0);
							break;
						case 5:
							pm = mkclass(S_GHOST, 0);
							break;
						default:
							pm = mkclass(S_WRAITH, 0);
							break;
					}
					mtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS);
					if (tamedog(mtmp, NULL))
						mtmp->mtame = 30;
					summon_loop--;
				} while (summon_loop);
				/* Tsk,tsk.. */
				adjalign(-3);
				u.uluck -= 3;
				break;
			case PROT_POLY:
				pline("You feel more observant.");
				rescham();
				break;
			case SUMMON_FIRE_ELEMENTAL:
				pm = &mons[PM_FIRE_ELEMENTAL];
				mtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS);

				pline("You summon an elemental.");

				if (tamedog(mtmp, NULL))
					mtmp->mtame = 30;
				break;
			case SUMMON_WATER_ELEMENTAL:
				pm = &mons[PM_WATER_ELEMENTAL];
				mtmp = makemon(pm, u.ux, u.uy, NO_MM_FLAGS);

				pline("You summon an elemental.");

				if (tamedog(mtmp, NULL))
					mtmp->mtame = 30;
				break;
			case OBJ_DETECTION:
				object_detect(obj, 0);
				break;
			case CREATE_PORTAL: {
				if (Is_blackmarket(&u.uz) && *u.ushops) {
					pline("You feel very disoriented for a moment.");
					break;
				}

				int i, num_ok_dungeons, last_ok_dungeon = 0;
				d_level newlev;
				extern int n_dgns; /* from dungeon.c */
				winid tmpwin = create_nhwindow(NHW_MENU);
				anything any;

				any.a_void = 0; /* set all bits to zero */
				start_menu(tmpwin);
				/* use index+1 (cant use 0) as identifier */
				for (i = num_ok_dungeons = 0; i < n_dgns; i++) {
					if (!dungeons[i].dunlev_ureached) continue;
					any.a_int = i + 1;
					add_menu(tmpwin, NO_GLYPH, &any, 0, 0, ATR_NONE,
						 dungeons[i].dname, MENU_UNSELECTED);
					num_ok_dungeons++;
					last_ok_dungeon = i;
				}
				end_menu(tmpwin, "Open a portal to which dungeon?");
				if (num_ok_dungeons > 1) {
					/* more than one entry; display menu for choices */
					menu_item *selected;
					int n;

					n = select_menu(tmpwin, PICK_ONE, &selected);
					if (n <= 0) {
						destroy_nhwindow(tmpwin);
						goto nothing_special;
					}
					i = selected[0].item.a_int - 1;
					free(selected);
				} else
					i = last_ok_dungeon; /* also first & only OK dungeon */
				destroy_nhwindow(tmpwin);

				/*
			 * i is now index into dungeon structure for the new dungeon.
			 * Find the closest level in the given dungeon, open
			 * a use-once portal to that dungeon and go there.
			 * The closest level is either the entry or dunlev_ureached.
			 */
				newlev.dnum = i;
				if (dungeons[i].depth_start >= depth(&u.uz))
					newlev.dlevel = dungeons[i].entry_lev;
				else
					newlev.dlevel = dungeons[i].dunlev_ureached;
				if (u.uhave.amulet || In_endgame(&u.uz) || In_endgame(&newlev) ||
				    newlev.dnum == u.uz.dnum) {
					pline("You feel very disoriented for a moment.");
				} else {
					if (!Blind)
						pline("You are surrounded by a shimmering sphere!");
					else
						pline("You feel weightless for a moment.");
					goto_level(&newlev, false, false, false);
				}
				break;
			}
			case ENLIGHTENING:
				enlightenment(0);
				break;
			case CREATE_AMMO: {
				struct obj *otmp = mksobj(ARROW, true, false);

				if (!otmp) goto nothing_special;
				otmp->blessed = obj->blessed;
				otmp->cursed = obj->cursed;
				otmp->bknown = obj->bknown;
				if (obj->blessed) {
					if (otmp->spe < 0) otmp->spe = 0;
					otmp->quan += rnd(10);
				} else if (obj->cursed) {
					if (otmp->spe > 0) otmp->spe = 0;
				} else
					otmp->quan += rnd(5);
				otmp->owt = weight(otmp);
				otmp = hold_another_object(otmp, "Suddenly %s out.",
							   aobjnam(otmp, "fall"), NULL);
				break;
			}
		}
	} else {
		long eprop = (u.uprops[oart->inv_prop].extrinsic ^= W_ARTI),
		     iprop = u.uprops[oart->inv_prop].intrinsic;
		const bool on = eprop & W_ARTI; /* true if invoked prop just set */

		if (on && obj->age > monstermoves) {
			/* the artifact is tired :-) */
			u.uprops[oart->inv_prop].extrinsic ^= W_ARTI;
			pline("You feel that %s %s ignoring you.",
			      the(xname(obj)), otense(obj, "are"));
			/* can't just keep repeatedly trying */
			obj->age += (long)d(3, 10);
			return 1;
		} else if (!on) {
			/* when turning off property, determine downtime */
			/* arbitrary for now until we can tune this -dlc */
			obj->age = monstermoves + rnz(100);
		}

		if ((eprop & ~W_ARTI) || iprop) {
nothing_special:
			/* you had the property from some other source too */
			if (carried(obj))
				pline("You feel a surge of power, but nothing seems to happen.");
			return 1;
		}
		switch (oart->inv_prop) {
			case CONFLICT:
				if (on)
					pline("You feel like a rabble-rouser.");
				else
					pline("You feel the tension decrease around you.");
				break;
			case LEVITATION:
				if (on) {
					float_up();
					spoteffects(false);
				} else
					float_down(I_SPECIAL | TIMEOUT, W_ARTI);
				break;
			case INVIS:
				if (BInvis || Blind) goto nothing_special;
				newsym(u.ux, u.uy);
				if (on)
					pline("Your body takes on a %s transparency...",
					      Hallucination ? "normal" : "strange");
				else
					pline("Your body seems to unfade...");
				break;
		}
	}

	return 1;
}

/*
 * Artifact is dipped into water
 * -1 not handled here (not used up here)
 *  0 no effect but used up
 *  else return
 *  AD_FIRE, etc.
 *  Note caller should handle what happens to the medium in these cases.
 *      This only prints messages about the actual artifact.
 */

int artifact_wet(struct obj *obj, bool silent) {
	if (!obj->oartifact) return -1;
	switch (artilist[(int)(obj)->oartifact].attk.adtyp) {
		case AD_FIRE:
			if (!silent) {
				pline("A cloud of steam rises.");
				pline("%s is untouched.", The(xname(obj)));
			}
			return AD_FIRE;
		case AD_COLD:
			if (!silent) {
				pline("Icicles form and fall from the freezing %s.",
				      the(xname(obj)));
			}
			return AD_COLD;
		case AD_ELEC:
			if (!silent) {
				pline("The humid air crackles with electricity from %s.",
				      the(xname(obj)));
			}
			return AD_ELEC;
		case AD_DRLI:
			if (!silent) {
				pline("%s absorbs the water!", The(xname(obj)));
			}
			return AD_DRLI;
		default:
			break;
	}
	return -1;
}

/* WAC return true if artifact is always lit */
bool artifact_light(struct obj *obj) {
	return get_artifact(obj) && (obj->oartifact == ART_SUNSWORD ||
				     obj->oartifact == ART_HOLY_SPEAR_OF_LIGHT ||
				     obj->oartifact == ART_CANDLE_OF_ETERNAL_FLAME);
}

/* KMH -- Talking artifacts are finally implemented */
void arti_speak(struct obj *obj) {
	const struct artifact *oart = get_artifact(obj);
	const char *line;
	char buf[BUFSZ];

	/* Is this a speaking artifact? */
	if (!oart || !(oart->spfx & SPFX_SPEAK))
		return;

	line = getrumor(bcsign(obj), buf, true);
	if (!*line)
		line = "Slash'EM rumors file closed for renovation.";
	pline("%s:", Tobjnam(obj, "whisper"));
	verbalize("%s", line);
}

bool artifact_has_invprop(struct obj *otmp, uchar inv_prop) {
	const struct artifact *arti = get_artifact(otmp);

	return arti && (arti->inv_prop == inv_prop);
}

/* Return the price sold to the hero of a given artifact or unique item */
long arti_cost(struct obj *otmp) {
	if (!otmp->oartifact)
		return objects[otmp->otyp].oc_cost;
	else if (artilist[otmp->oartifact].cost)
		return artilist[otmp->oartifact].cost;
	else
		return 100L * objects[otmp->otyp].oc_cost;
}
/*artifact.c*/
