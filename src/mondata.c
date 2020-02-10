/*	SCCS Id: @(#)mondata.c	3.4	2003/06/02	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "eshk.h"
#include "epri.h"

/*	These routines provide basic data for any type of monster. */

void set_mon_data(struct monst *mon, struct permonst *ptr, int flag) {
	mon->data = ptr;
	if (flag == -1) return; /* "don't care" */

	if (flag == 1)
		mon->mintrinsics |= (ptr->mresists & MR_TYPEMASK);
	else
		mon->mintrinsics = (ptr->mresists & MR_TYPEMASK);
	return;
}

struct attack *attacktype_fordmg(struct permonst *ptr, int atyp, int dtyp) {
	struct attack *a;

	for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
		if (a->aatyp == atyp && (dtyp == AD_ANY || a->adtyp == dtyp))
			return a;

	return NULL;
}

boolean attacktype(struct permonst *ptr, int atyp) {
	return attacktype_fordmg(ptr, atyp, AD_ANY) ? true : false;
}

boolean poly_when_stoned(struct permonst *ptr) {
	return is_golem(ptr) && ptr != &mons[PM_STONE_GOLEM] &&
	       !(mvitals[PM_STONE_GOLEM].mvflags & G_GENOD);
	/* allow G_EXTINCT */
}

/* returns true if monster is drain-life resistant */
boolean resists_drli(struct monst *mon) {
	struct permonst *ptr = mon->data;
	struct obj *wep = ((mon == &youmonst) ? uwep : MON_WEP(mon));

	return is_undead(ptr) || is_demon(ptr) || is_were(ptr) ||
	       ptr == &mons[PM_DEATH] || is_golem(ptr) ||
	       resists_drain(mon) || is_vampshifter(mon) ||
	       (wep && wep->oartifact && defends(AD_DRLI, wep));
}

/* true if monster is magic-missile resistant */
boolean resists_magm(struct monst *mon) {
	struct permonst *ptr = mon->data;
	struct obj *o;

	/* as of 3.2.0:  gray dragons, Angels, Oracle, Yeenoghu */
	if (dmgtype(ptr, AD_MAGM) || ptr == &mons[PM_BABY_GRAY_DRAGON] ||
	    dmgtype(ptr, AD_RBRE)) /* Chromatic Dragon */
		return true;
	/* check for magic resistance granted by wielded weapon */
	o = (mon == &youmonst) ? uwep : MON_WEP(mon);
	if (o && o->oartifact && defends(AD_MAGM, o))
		return true;
	/* check for magic resistance granted by worn or carried items */
	o = (mon == &youmonst) ? invent : mon->minvent;
	for (; o; o = o->nobj)
		if ((o->owornmask && objects[o->otyp].oc_oprop == ANTIMAGIC) ||
		    (o->oartifact && protects(AD_MAGM, o)))
			return true;
	return false;
}

/* true if monster is resistant to light-induced blindness */
boolean resists_blnd(struct monst *mon) {
	struct permonst *ptr = mon->data;
	boolean is_you = (mon == &youmonst);
	struct obj *o;

	if (is_you ? (Blind || u.usleep) :
		     (mon->mblinded || !mon->mcansee || !haseyes(ptr) ||
		      /* BUG: temporary sleep sets mfrozen, but since
	                     paralysis does too, we can't check it */
		      mon->msleeping))
		return true;
	/* yellow light, Archon; !dust vortex, !cobra, !raven */
	if (dmgtype_fromattack(ptr, AD_BLND, AT_EXPL) ||
	    dmgtype_fromattack(ptr, AD_BLND, AT_GAZE))
		return true;
	o = is_you ? uwep : MON_WEP(mon);
	if (o && o->oartifact && defends(AD_BLND, o))
		return true;
	o = is_you ? invent : mon->minvent;
	for (; o; o = o->nobj)
		if ((o->owornmask && objects[o->otyp].oc_oprop == BLINDED) ||
		    (o->oartifact && protects(AD_BLND, o)))
			return true;
	return false;
}

/* true iff monster can be blinded by the given attack */
/* Note: may return true when mdef is blind (e.g. new cream-pie attack) */
// magr == NULL -> no specific aggressor
// obj when aatyp == AT_WEAP, AT_SPIT */
boolean can_blnd(struct monst *magr, struct monst *mdef, uchar aatyp, struct obj *obj) {
	boolean is_you = (mdef == &youmonst);
	boolean check_visor = false;
	struct obj *o;
	const char *s;

	/* no eyes protect against all attacks for now */
	if (!haseyes(mdef->data))
		return false;

	switch (aatyp) {
		case AT_EXPL:
		case AT_BOOM:
		case AT_GAZE:
		case AT_MAGC:
		case AT_BREA: /* assumed to be lightning */
			/* light-based attacks may be cancelled or resisted */
			if (magr && magr->mcan)
				return false;
			return !resists_blnd(mdef);

		case AT_WEAP:
		case AT_SPIT:
		case AT_NONE:
			/* an object is used (thrown/spit/other) */
			if (obj && (obj->otyp == CREAM_PIE)) {
				if (is_you && Blindfolded)
					return false;
			} else if (obj && (obj->otyp == BLINDING_VENOM)) {
				/* all ublindf, including LENSES, protect, cream-pies too */
				if (is_you && (ublindf || u.ucreamed))
					return false;
				check_visor = true;
			} else if (obj && (obj->otyp == POT_BLINDNESS)) {
				return true; /* no defense */
			} else
				return false; /* other objects cannot cause blindness yet */
			if ((magr == &youmonst) && u.uswallow)
				return false; /* can't affect eyes while inside monster */
			break;

		case AT_ENGL:
			if (is_you && (Blindfolded || u.usleep || u.ucreamed))
				return false;
			if (!is_you && mdef->msleeping)
				return false;
			break;

		case AT_CLAW:
			/* e.g. raven: all ublindf, including LENSES, protect */
			if (is_you && ublindf)
				return false;
			if ((magr == &youmonst) && u.uswallow)
				return false; /* can't affect eyes while inside monster */
			check_visor = true;
			break;

		case AT_TUCH:
		case AT_STNG:
			/* some physical, blind-inducing attacks can be cancelled */
			if (magr && magr->mcan)
				return false;
			break;

		default:
			break;
	}

	/* check if wearing a visor (only checked if visor might help) */
	if (check_visor) {
		o = (mdef == &youmonst) ? invent : mdef->minvent;
		for (; o; o = o->nobj)
			if ((o->owornmask & W_ARMH) &&
			    (s = OBJ_DESCR(objects[o->otyp])) != NULL &&
			    !strcmp(s, "visored helmet"))
				return false;
	}

	return true;
}

/* returns true if monster can attack at range */
boolean ranged_attk(struct permonst *ptr) {
	int i, atyp;
	long atk_mask = (1L << AT_BREA) | (1L << AT_SPIT) | (1L << AT_GAZE);

	/* was: (attacktype(ptr, AT_BREA) || attacktype(ptr, AT_WEAP) ||
		attacktype(ptr, AT_SPIT) || attacktype(ptr, AT_GAZE) ||
		attacktype(ptr, AT_MAGC));
	   but that's too slow -dlc
	 */
	for (i = 0; i < NATTK; i++) {
		atyp = ptr->mattk[i].aatyp;
		if (atyp >= AT_WEAP) return true;
		/* assert(atyp < 32); */
		if ((atk_mask & (1L << atyp)) != 0L) return true;
	}

	return false;
}

/* true iff the type of monster pass through iron bars */
boolean passes_bars(struct permonst *mptr) {
	return passes_walls(mptr) || amorphous(mptr) ||
	       is_whirly(mptr) || verysmall(mptr) ||
	       (slithy(mptr) && !bigmonst(mptr));
}

// returns true if the monster is capable of blowing (through e.g. a musical instrument)
bool can_blow(struct monst *mtmp) {
	if ((is_silent(mtmp->data) || mtmp->data->msound == MS_BUZZ) &&
	    (breathless(mtmp->data) || verysmall(mtmp->data) || !has_head(mtmp->data) || mtmp->data->mlet == S_EEL))
		return false;
	if ((mtmp == &youmonst) && Strangled) return false;

	return true;
}

/* returns true if monster can track well */
boolean can_track(struct permonst *ptr) {
	if (uwep && uwep->oartifact == ART_EXCALIBUR)
		return true;
	else
		return haseyes(ptr);
}

/* creature will slide out of armor */
boolean sliparm(struct permonst *ptr) {
	return is_whirly(ptr) || ptr->msize <= MZ_SMALL || noncorporeal(ptr);
}

/* creature will break out of armor */
boolean breakarm(struct permonst *ptr) {
	return (bigmonst(ptr) || (ptr->msize > MZ_SMALL && !humanoid(ptr)) ||
		/* special cases of humanoids that cannot wear body armor */
		ptr == &mons[PM_MARILITH] || ptr == &mons[PM_WINGED_GARGOYLE]) &&
	       !sliparm(ptr);
}

/* creature sticks other creatures it hits */
boolean sticks(struct permonst *ptr) {
	return dmgtype(ptr, AD_STCK) || dmgtype(ptr, AD_WRAP) || attacktype(ptr, AT_HUGS);
}

/* number of horns this type of monster has on its head */
int num_horns(struct permonst *ptr) {
	switch (monsndx(ptr)) {
		case PM_LAMB:
		case PM_SHEEP:
		case PM_GOAT:
		case PM_COW:
		case PM_BULL:
		case PM_HORNED_DEVIL: /* ? "more than one" */
		case PM_MINOTAUR:
		case PM_ASMODEUS:
		case PM_BALROG:
			return 2;
		case PM_WHITE_UNICORN:
		case PM_GRAY_UNICORN:
		case PM_BLACK_UNICORN:
		case PM_KI_RIN:
			return 1;
		default:
			break;
	}
	return 0;
}

struct attack *dmgtype_fromattack(struct permonst *ptr, int dtyp, int atyp) {
	struct attack *a;

	for (a = &ptr->mattk[0]; a < &ptr->mattk[NATTK]; a++)
		if (a->adtyp == dtyp && (atyp == AT_ANY || a->aatyp == atyp))
			return a;

	return NULL;
}

boolean dmgtype(struct permonst *ptr, int dtyp) {
	return dmgtype_fromattack(ptr, dtyp, AT_ANY) ? true : false;
}

/* returns the maximum damage a defender can do to the attacker via
 * a passive defense */
int max_passive_dmg(struct monst *mdef, struct monst *magr) {
	int i, dmg = 0, multi = 0;
	uchar adtyp;

	// each attack by magr can result in passive damage
	for(i = 0; i < NATTK; i++) {
		switch (magr->data->mattk[i].aatyp) {
			case AT_CLAW: case AT_BITE: case AT_KICK: case AT_BUTT: case AT_TUCH:
			case AT_STNG: case AT_HUGS: case AT_ENGL: case AT_TENT: case AT_WEAP:
				multi++;
				break;
			default:
				break;
		}
	}


	for (i = 0; i < NATTK; i++)
		if (mdef->data->mattk[i].aatyp == AT_NONE ||
		    mdef->data->mattk[i].aatyp == AT_BOOM) {
			adtyp = mdef->data->mattk[i].adtyp;
			if ((adtyp == AD_ACID && !resists_acid(magr)) ||
			    (adtyp == AD_COLD && !resists_cold(magr)) ||
			    (adtyp == AD_FIRE && !resists_fire(magr)) ||
			    (adtyp == AD_ELEC && !resists_elec(magr)) ||
			    adtyp == AD_PHYS) {
				dmg = mdef->data->mattk[i].damn;
				if (!dmg) dmg = mdef->data->mlevel + 1;
				dmg *= mdef->data->mattk[i].damd;
			} else {
				dmg = 0;
			}

			return dmg * multi;
		}
	return 0;
}

/* determine whether two monster types are from the same species */
bool same_race(struct permonst *pm1, struct permonst *pm2) {
	char let1 = pm1->mlet, let2 = pm2->mlet;

	if (pm1 == pm2)		return true;	/* exact match */
	/* player races have their own predicates */
	if (is_human(pm1))	return is_human(pm2);
	if (is_elf(pm1))	return is_elf(pm2);
	if (is_dwarf(pm1))	return is_dwarf(pm2);
	if (is_gnome(pm1))	return is_gnome(pm2);
	if (is_orc(pm1))	return is_orc(pm2);
	if (is_vampire(pm1))	return is_vampire(pm2);
	/* other creatures are less precise */
	if (is_giant(pm1))	return is_giant(pm2);   /* open to quibbling here */
	if (is_golem(pm1))	return is_golem(pm2);   /* even moreso... */
	if (is_mind_flayer(pm1))return is_mind_flayer(pm2);
	if (let1 == S_KOBOLD ||
			pm1 == &mons[PM_KOBOLD_ZOMBIE] ||
			pm1 == &mons[PM_KOBOLD_MUMMY])
		return (let2 == S_KOBOLD ||
				pm2 == &mons[PM_KOBOLD_ZOMBIE] ||
				pm2 == &mons[PM_KOBOLD_MUMMY]);
	if (let1 == S_OGRE)	return (let2 == S_OGRE);
	if (let1 == S_NYMPH)	return (let2 == S_NYMPH);
	if (let1 == S_CENTAUR)	return (let2 == S_CENTAUR);
	if (is_unicorn(pm1))	return is_unicorn(pm2);
	if (let1 == S_DRAGON)	return (let2 == S_DRAGON);
	if (let1 == S_NAGA)	return (let2 == S_NAGA);
	/* other critters get steadily messier */
	if (is_rider(pm1))	return is_rider(pm2);   /* debatable */
	if (is_minion(pm1))	return is_minion(pm2); /* [needs work?] */
	/* tengu don't match imps (first test handled case of both being tengu) */
	if (pm1 == &mons[PM_TENGU] || pm2 == &mons[PM_TENGU]) return false;

	if (let1 == S_IMP)	return (let2 == S_IMP);
	/* and minor demons (imps) don't match major demons */
	else if (let2 == S_IMP)	return false;

	if (is_demon(pm1))	return is_demon(pm2);
	if (is_undead(pm1)) {
		if (let1 == S_ZOMBIE) 	return (let2 == S_ZOMBIE);
		if (let1 == S_MUMMY) 	return (let2 == S_MUMMY);
		if (let1 == S_VAMPIRE) 	return (let2 == S_VAMPIRE);
		if (let1 == S_LICH) 	return (let2 == S_LICH);
		if (let1 == S_WRAITH) 	return (let2 == S_WRAITH);
		if (let1 == S_GHOST) 	return (let2 == S_GHOST);
	} else if (is_undead(pm2)) return false;

	/* check for monsters--mainly animals--which grow into more mature forms */
	if (let1 == let2) {
		int m1 = monsndx(pm1), m2 = monsndx(pm2), prv, nxt;

		/* we know m1 != m2 (very first check above); test all smaller
		   forms of m1 against m2, then all larger ones; don't need to
		   make the corresponding tests for variants of m2 against m1 */
		for (prv = m1, nxt = big_to_little(m1); nxt != prv;
				prv = nxt, nxt = big_to_little(nxt)) if (nxt == m2) return true;
		for (prv = m1, nxt = little_to_big(m1); nxt != prv;
				prv = nxt, nxt = little_to_big(nxt)) if (nxt == m2) return true;
	}

	/* not caught by little/big handling */
	if (pm1 == &mons[PM_GARGOYLE] || pm1 == &mons[PM_WINGED_GARGOYLE])
		return (pm2 == &mons[PM_GARGOYLE] || pm2 == &mons[PM_WINGED_GARGOYLE]);

	if (pm1 == &mons[PM_KILLER_BEE] || pm1 == &mons[PM_QUEEN_BEE])
		return (pm2 == &mons[PM_KILLER_BEE] || pm2 == &mons[PM_QUEEN_BEE]);

	if (is_longworm(pm1)) return is_longworm(pm2);     /* handles tail */

	/* [currently there's no reason to bother matching up
	   assorted bugs and blobs with their closest variants] */
	/* didn't match */
	return false;
}

// return an index into the mons array
int monsndx(struct permonst *ptr) {
	int i;

	if (ptr == &upermonst) return PM_PLAYERMON;

	i = (int)(ptr - &mons[0]);
	if (i < LOW_PM || i >= NUMMONS) {
		/* ought to switch this to use `fmt_ptr' */
		panic("monsndx - could not index monster (%d)",
		      i);
		return NON_PM; /* will not get here */
	}

	return i;
}

int name_to_mon(const char *in_str) {
	/* Be careful.  We must check the entire string in case it was
	 * something such as "ettin zombie corpse".  The calling routine
	 * doesn't know about the "corpse" until the monster name has
	 * already been taken off the front, so we have to be able to
	 * read the name with extraneous stuff such as "corpse" stuck on
	 * the end.
	 * This causes a problem for names which prefix other names such
	 * as "ettin" on "ettin zombie".  In this case we want the _longest_
	 * name which exists.
	 * This also permits plurals created by adding suffixes such as 's'
	 * or 'es'.  Other plurals must still be handled explicitly.
	 */
	int i;
	int mntmp = NON_PM;
	char *s, *str, *term;
	char buf[BUFSZ];
	int len, slen;

	str = strcpy(buf, in_str);

	if (!strncmp(str, "a ", 2))
		str += 2;
	else if (!strncmp(str, "an ", 3))
		str += 3;

	slen = strlen(str);
	term = str + slen;

	if ((s = strstri(str, "vortices")) != 0)
		strcpy(s + 4, "ex");
	/* be careful with "ies"; "priest", "zombies" */
	else if (slen > 3 && !strcmpi(term - 3, "ies") &&
		 (slen < 7 || strcmpi(term - 7, "zombies")))
		strcpy(term - 3, "y");
	/* luckily no monster names end in fe or ve with ves plurals */
	else if (slen > 3 && !strcmpi(term - 3, "ves"))
		strcpy(term - 3, "f");

	slen = strlen(str); /* length possibly needs recomputing */

	{
		static const struct alt_spl {
			const char *name;
			short pm_val;
		} names[] = {
			/* Alternate spellings */
			{"grey dragon", PM_GRAY_DRAGON},
			{"baby grey dragon", PM_BABY_GRAY_DRAGON},
			{"grey unicorn", PM_GRAY_UNICORN},
			{"grey ooze", PM_GRAY_OOZE},
			{"gray-elf", PM_GREY_ELF},
			{"mindflayer", PM_MIND_FLAYER},
			{"master mindflayer", PM_MASTER_MIND_FLAYER},
			/* Hyphenated names */
			{"ki rin", PM_KI_RIN},
			{"uruk hai", PM_URUK_HAI},
			{"orc captain", PM_ORC_CAPTAIN},
			{"woodland elf", PM_WOODLAND_ELF},
			{"green elf", PM_GREEN_ELF},
			{"grey elf", PM_GREY_ELF},
			{"gray elf", PM_GREY_ELF},
			{"elf lord", PM_ELF_LORD},
#if 0 /* OBSOLETE */
			{ "high elf",		PM_HIGH_ELF },
#endif
			{"olog hai", PM_OLOG_HAI},
			{"arch lich", PM_ARCH_LICH},
			/* Some irregular plurals */
			{"incubi", PM_INCUBUS},
			{"succubi", PM_SUCCUBUS},
			{"violet fungi", PM_VIOLET_FUNGUS},
			{"homunculi", PM_HOMUNCULUS},
			{"baluchitheria", PM_BALUCHITHERIUM},
			{"lurkers above", PM_LURKER_ABOVE},
			{"cavemen", PM_CAVEMAN},
			{"cavewomen", PM_CAVEWOMAN},
#ifndef ZOUTHERN
		/*		{ "zruties",            PM_ZRUTY },*/
#endif
			{"djinn", PM_DJINNI},
			{"mumakil", PM_MUMAK},
			{"erinyes", PM_ERINYS},
			{"giant lice", PM_GIANT_LOUSE}, /* RJ */
			/* falsely caught by -ves check above */
			{"master of thief", PM_MASTER_OF_THIEVES},
			/* end of list */
			{0, 0}
		};
		const struct alt_spl *namep;

		for (namep = names; namep->name; namep++)
			if (!strncmpi(str, namep->name, (int)strlen(namep->name)))
				return namep->pm_val;
	}

	for (len = 0, i = LOW_PM; i < NUMMONS; i++) {
		int m_i_len = strlen(mons[i].mname);
		if (m_i_len > len && !strncmpi(mons[i].mname, str, m_i_len)) {
			if (m_i_len == slen)
				return i; /* exact match */
			else if (slen > m_i_len &&
				 (str[m_i_len] == ' ' ||
				  !strcmpi(&str[m_i_len], "s") ||
				  !strncmpi(&str[m_i_len], "s ", 2) ||
				  !strcmpi(&str[m_i_len], "'") ||
				  !strncmpi(&str[m_i_len], "' ", 2) ||
				  !strcmpi(&str[m_i_len], "'s") ||
				  !strncmpi(&str[m_i_len], "'s ", 3) ||
				  !strcmpi(&str[m_i_len], "es") ||
				  !strncmpi(&str[m_i_len], "es ", 3))) {
				mntmp = i;
				len = m_i_len;
			}
		}
	}
	if (mntmp == NON_PM) mntmp = title_to_mon(str, NULL, NULL);
	return mntmp;
}

/* returns 3 values (0=male, 1=female, 2=none) */
int gender(struct monst *mtmp) {
	if (is_neuter(mtmp->data)) return 2;
	return mtmp->female;
}

/* Like gender(), but lower animals and such are still "it". */
/* This is the one we want to use when printing messages. */
int pronoun_gender(struct monst *mtmp) {
	if (!mtmp->isshk && (is_neuter(mtmp->data) || !canspotmon(mtmp))) return 2;
	return (humanoid(mtmp->data) || (mtmp->data->geno & G_UNIQ) ||
		type_is_pname(mtmp->data)) ?
		       (int)mtmp->female :
		       2;
}

/* used for nearby monsters when you go to another level */
boolean levl_follower(struct monst *mtmp) {
	/* monsters with the Amulet--even pets--won't follow across levels */
	if (mon_has_amulet(mtmp)) return false;

	/* some monsters will follow even while intending to flee from you */
	if (mtmp->mtame || mtmp->iswiz || is_fshk(mtmp)) return true;

	/* stalking types follow, but won't when fleeing unless you hold
	   the Amulet */
	return (mtmp->data->mflags2 & M2_STALK) && (!mtmp->mflee || u.uhave.amulet);
}

static const short grownups[][2] = {
	{PM_LITTLE_DOG, PM_DOG},
	{PM_DOG, PM_LARGE_DOG},
	{PM_HELL_HOUND_PUP, PM_HELL_HOUND},
	{PM_KITTEN, PM_HOUSECAT},
	{PM_HOUSECAT, PM_LARGE_CAT},
	{PM_KOBOLD, PM_LARGE_KOBOLD},
	{PM_LARGE_KOBOLD, PM_KOBOLD_LORD},
	{PM_GNOME, PM_GNOME_LORD},
	{PM_GNOME_LORD, PM_GNOME_WARRIOR},
	{PM_DWARF, PM_DWARF_LORD},
	{PM_DWARF_LORD, PM_DWARF_KING},
	{PM_MIND_FLAYER, PM_MASTER_MIND_FLAYER},
	{PM_ORC, PM_ORC_CAPTAIN},
	{PM_HILL_ORC, PM_ORC_CAPTAIN},
	{PM_MORDOR_ORC, PM_ORC_CAPTAIN},
	{PM_URUK_HAI, PM_ORC_CAPTAIN},
	{PM_SEWER_RAT, PM_GIANT_RAT},
	{PM_CAVE_SPIDER, PM_GIANT_SPIDER},
	{PM_OGRE, PM_OGRE_LORD},
	{PM_OGRE_LORD, PM_OGRE_KING},
	{PM_ELF, PM_ELF_LORD},
	{PM_WOODLAND_ELF, PM_ELF_LORD},
	{PM_GREEN_ELF, PM_ELF_LORD},
	{PM_GREY_ELF, PM_ELF_LORD},
	{PM_ELF_LORD, PM_ELVENKING},
	{PM_LICH, PM_DEMILICH},
	{PM_DEMILICH, PM_MASTER_LICH},
	{PM_MASTER_LICH, PM_ARCH_LICH},
	{PM_VAMPIRE, PM_VAMPIRE_LORD},
	{PM_VAMPIRE_LORD, PM_VAMPIRE_MAGE},
	{PM_BAT, PM_GIANT_BAT},
	{PM_CHICKATRICE, PM_COCKATRICE},
	{PM_BABY_GRAY_DRAGON, PM_GRAY_DRAGON},
	{PM_BABY_RED_DRAGON, PM_RED_DRAGON},
	{PM_BABY_SILVER_DRAGON, PM_SILVER_DRAGON},
	{PM_BABY_DEEP_DRAGON, PM_DEEP_DRAGON},
	{PM_BABY_SHIMMERING_DRAGON, PM_SHIMMERING_DRAGON},
	{PM_BABY_WHITE_DRAGON, PM_WHITE_DRAGON},
	{PM_BABY_ORANGE_DRAGON, PM_ORANGE_DRAGON},
	{PM_BABY_BLACK_DRAGON, PM_BLACK_DRAGON},
	{PM_BABY_BLUE_DRAGON, PM_BLUE_DRAGON},
	{PM_BABY_GREEN_DRAGON, PM_GREEN_DRAGON},
	{PM_BABY_YELLOW_DRAGON, PM_YELLOW_DRAGON},
	{PM_RED_NAGA_HATCHLING, PM_RED_NAGA},
	{PM_BLACK_NAGA_HATCHLING, PM_BLACK_NAGA},
	{PM_GOLDEN_NAGA_HATCHLING, PM_GOLDEN_NAGA},
	{PM_GUARDIAN_NAGA_HATCHLING, PM_GUARDIAN_NAGA},
	{PM_SMALL_MIMIC, PM_LARGE_MIMIC},
	{PM_LARGE_MIMIC, PM_GIANT_MIMIC},
	{PM_BABY_LONG_WORM, PM_LONG_WORM},
	{PM_BABY_PURPLE_WORM, PM_PURPLE_WORM},
	{PM_BABY_CROCODILE, PM_CROCODILE},
	{PM_SOLDIER, PM_SERGEANT},
	{PM_SERGEANT, PM_LIEUTENANT},
	{PM_LIEUTENANT, PM_CAPTAIN},
	{PM_WATCHMAN, PM_WATCH_CAPTAIN},
	{PM_ALIGNED_PRIEST, PM_HIGH_PRIEST},
	{PM_STUDENT, PM_ARCHEOLOGIST},
	{PM_ATTENDANT, PM_HEALER},
	{PM_PAGE, PM_KNIGHT},
	{PM_ACOLYTE, PM_PRIEST},
	{PM_APPRENTICE, PM_WIZARD},
	{PM_MANES, PM_LEMURE},
	{PM_KEYSTONE_KOP, PM_KOP_SERGEANT},
	{PM_KOP_SERGEANT, PM_KOP_LIEUTENANT},
	{PM_KOP_LIEUTENANT, PM_KOP_KAPTAIN},

	/* WAC -- added killer coin piles */
	{PM_PILE_OF_KILLER_COINS, PM_LARGE_PILE_OF_KILLER_COINS},
	{PM_LARGE_PILE_OF_KILLER_COINS, PM_HUGE_PILE_OF_KILLER_COINS},
	/* KMH -- added more sequences */
	{PM_DINGO_PUPPY, PM_DINGO},
	{PM_DINGO, PM_LARGE_DINGO},
	{PM_PONY, PM_HORSE},
	{PM_HORSE, PM_WARHORSE},
	{PM_LARVA, PM_MAGGOT},
	{PM_MAGGOT, PM_DUNG_WORM},
	{PM_WINTER_WOLF_CUB, PM_WINTER_WOLF},
	{PM_GIANT_TICK, PM_GIANT_FLEA},
	{PM_GIANT_FLEA, PM_GIANT_LOUSE}, /* RJ */
	/* DS -- growing up, Lethe style */
	{PM_DEEP_ONE, PM_DEEPER_ONE},
	{PM_DEEPER_ONE, PM_DEEPEST_ONE},
	{PM_LAMB, PM_SHEEP},
	{PM_SHOGGOTH, PM_GIANT_SHOGGOTH},
	{PM_GNOLL, PM_GNOLL_WARRIOR},
	{PM_GNOLL_WARRIOR, PM_GNOLL_CHIEFTAIN},
	{PM_MIGO_DRONE, PM_MIGO_WARRIOR},

	{NON_PM, NON_PM}};

int little_to_big(int montype) {
	int i;

	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if (montype == grownups[i][0]) return grownups[i][1];
	return montype;
}

int big_to_little(int montype) {
	int i;

	for (i = 0; grownups[i][0] >= LOW_PM; i++)
		if (montype == grownups[i][1]) return grownups[i][0];
	return montype;
}

/*
 * Return the permonst ptr for the race of the monster.
 * Returns correct pointer for non-polymorphed and polymorphed
 * player.  It does not return a pointer to player role character.
 */
const struct permonst *raceptr(struct monst *mtmp) {
	if (mtmp == &youmonst && !Upolyd)
		return &mons[urace.malenum];
	else
		return mtmp->data;
}

static const char *levitate[4] = {"float", "Float", "wobble", "Wobble"};
static const char *flys[4] = {"fly", "Fly", "flutter", "Flutter"};
static const char *flyl[4] = {"fly", "Fly", "stagger", "Stagger"};
static const char *slither[4] = {"slither", "Slither", "falter", "Falter"};
static const char *ooze[4] = {"ooze", "Ooze", "tremble", "Tremble"};
static const char *immobile[4] = {"wiggle", "Wiggle", "pulsate", "Pulsate"};
static const char *crawl[4] = {"crawl", "Crawl", "falter", "Falter"};

const char *
locomotion(const struct permonst *ptr, const char *def) {
	int capitalize = (*def == highc(*def));

	return is_floater(ptr) ? levitate[capitalize] :
				 (is_flyer(ptr) && ptr->msize <= MZ_SMALL) ? flys[capitalize] :
									     (is_flyer(ptr) && ptr->msize > MZ_SMALL) ? flyl[capitalize] :
															slithy(ptr) ? slither[capitalize] :
																      amorphous(ptr) ? ooze[capitalize] :
																		       !ptr->mmove ? immobile[capitalize] :
																				     nolimbs(ptr) ? crawl[capitalize] :
																						    def;
}

const char *stagger(const struct permonst *ptr, const char *def) {
	int capitalize = 2 + (*def == highc(*def));

	return is_floater(ptr) ? levitate[capitalize] :
				 (is_flyer(ptr) && ptr->msize <= MZ_SMALL) ? flys[capitalize] :
									     (is_flyer(ptr) && ptr->msize > MZ_SMALL) ? flyl[capitalize] :
															slithy(ptr) ? slither[capitalize] :
																      amorphous(ptr) ? ooze[capitalize] :
																		       !ptr->mmove ? immobile[capitalize] :
																				     nolimbs(ptr) ? crawl[capitalize] :
																						    def;
}

/* return a phrase describing the effect of fire attack on a type of monster */
const char *on_fire(struct permonst *mptr, struct attack *mattk) {
	const char *what;

	switch (monsndx(mptr)) {
		case PM_FLAMING_SPHERE:
		case PM_FIRE_VORTEX:
		case PM_FIRE_ELEMENTAL:
		case PM_SALAMANDER:
			what = "already on fire";
			break;
		case PM_WATER_ELEMENTAL:
		case PM_FOG_CLOUD:
		case PM_STEAM_VORTEX:
			what = "boiling";
			break;
		case PM_ICE_VORTEX:
		case PM_GLASS_GOLEM:
			what = "melting";
			break;
		case PM_STONE_GOLEM:
		case PM_CLAY_GOLEM:
		case PM_GOLD_GOLEM:
		case PM_AIR_ELEMENTAL:
		case PM_EARTH_ELEMENTAL:
		case PM_DUST_VORTEX:
		case PM_ENERGY_VORTEX:
			what = "heating up";
			break;
		default:
			what = (mattk->aatyp == AT_HUGS) ? "being roasted" : "on fire";
			break;
	}
	return what;
}

/*
 * Returns:
 *     True if monster is presumed to have a sense of smell.
 *     False if monster definitely does not have a sense of smell.
 *
 *     Do not base this on presence of a head or nose, since many
 *     creatures sense smells other ways (feelers, forked-tongues, etc.)
 *     We're assuming all insects can smell at a distance too.
 */
bool olfaction(struct permonst *mdat) {
	if (mdat && (is_golem(mdat) ||
		     mdat->mlet == S_EYE || /* spheres  */
		     mdat->mlet == S_JELLY ||
		     mdat->mlet == S_PUDDING ||
		     mdat->mlet == S_BLOB ||
		     mdat->mlet == S_VORTEX ||
		     mdat->mlet == S_ELEMENTAL ||
		     mdat->mlet == S_FUNGUS || /* mushrooms and fungi */
		     mdat->mlet == S_LIGHT ||
		     mdat->mlet == S_BAD_FOOD ||
		     mdat->mlet == S_BAD_COINS))
		return false;
	return true;
}

bool is_vampshifter(struct monst *mon) {
	if (mon == &youmonst && mon->cham == u.umonster) {
		return is_vampire(&upermonst);
	} else {
		return is_vampire(&mons[mon->cham]);
	}
}

/*mondata.c*/
