/*	SCCS Id: @(#)makemon.c	3.4	2003/09/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "epri.h"
#include "emin.h"
#include "edog.h"
#include <ctype.h>

static struct monst zeromonst;

/* this assumes that a human quest leader or nemesis is an archetype
   of the corresponding role; that isn't so for some roles (tourist
   for instance) but is for the priests and monks we use it for... */
#define quest_mon_represents_role(mptr, role_pm)      \
	(mptr->mlet == S_HUMAN && Role_if(role_pm) && \
	 (mptr->msound == MS_LEADER || mptr->msound == MS_NEMESIS))

static boolean uncommon(int);
static int align_shift(struct permonst *);
static boolean wrong_elem_type(struct permonst *);
static void m_initgrp(struct monst *, int, int, int);
static void m_initthrow(struct monst *, int, int);
static void m_initweap(struct monst *);
static void m_initweap_normal(struct monst *);
static void m_initinv(struct monst *);

#define m_initsgrp(mtmp, x, y)	m_initgrp(mtmp, x, y, 3)
#define m_initlgrp(mtmp, x, y)	m_initgrp(mtmp, x, y, 10)
#define m_initvlgrp(mtmp, x, y) m_initgrp(mtmp, x, y, 20)
#define toostrong(monindx, lev) (mstrength(&mons[monindx]) > lev)
#define tooweak(monindx, lev)	(mstrength(&mons[monindx]) < lev)

boolean is_home_elemental(struct permonst *ptr) {
	if (ptr->mlet == S_ELEMENTAL)
		switch (monsndx(ptr)) {
			case PM_AIR_ELEMENTAL:
				return Is_airlevel(&u.uz);
			case PM_FIRE_ELEMENTAL:
				return Is_firelevel(&u.uz);
			case PM_EARTH_ELEMENTAL:
				return Is_earthlevel(&u.uz);
			case PM_WATER_ELEMENTAL:
				return Is_waterlevel(&u.uz);
		}
	return false;
}

// Return true if the given monster cannot exist on this elemental level
static boolean wrong_elem_type(struct permonst *ptr) {
	if (ptr->mlet == S_ELEMENTAL) {
		return !is_home_elemental(ptr);
	} else if (Is_earthlevel(&u.uz)) {
		/* no restrictions? */
	} else if (Is_waterlevel(&u.uz)) {
		/* just monsters that can swim */
		if (!is_swimmer(ptr)) return true;
	} else if (Is_firelevel(&u.uz)) {
		if (!pm_resistance(ptr, MR_FIRE)) return true;
	} else if (Is_airlevel(&u.uz)) {
		if (!(is_flyer(ptr) && ptr->mlet != S_TRAPPER) && !is_floater(ptr) && !amorphous(ptr) && !noncorporeal(ptr) && !is_whirly(ptr))
			return true;
	}
	return false;
}

// make a group just like mtmp
static void m_initgrp(struct monst *mtmp, int x, int y, int n) {
	coord mm;
	int cnt = rnd(n);
	struct monst *mon;

	/* Tuning: cut down on swarming at low character levels [mrs] */
	if (u.ulevel < 5) cnt /= 2;

	if (!cnt) cnt++;

	mm.x = x;
	mm.y = y;
	while (cnt--) {
		if (peace_minded(mtmp->data)) continue;
		/* Don't create groups of peaceful monsters since they'll get
		 * in our way.  If the monster has a percentage chance so some
		 * are peaceful and some are not, the result will just be a
		 * smaller group.
		 */
		if (enexto(&mm, mm.x, mm.y, mtmp->data)) {
			mon = makemon(mtmp->data, mm.x, mm.y, NO_MM_FLAGS);
			mon->mpeaceful = false;
			mon->mavenge = 0;
			set_malign(mon);
			/* Undo the second peace_minded() check in makemon(); if the
			 * monster turned out to be peaceful the first time we
			 * didn't create it at all; we don't want a second check.
			 */
		}
	}
}

static void m_initthrow(struct monst *mtmp, int otyp, int oquan) {
	struct obj *otmp;

	otmp = mksobj(otyp, true, false);
	otmp->quan = rn1(oquan, 3);
	otmp->owt = weight(otmp);
	if (otyp == ORCISH_ARROW) otmp->opoisoned = true;
	mpickobj(mtmp, otmp);
}

static void m_initweap_normal(struct monst *mtmp) {
	struct permonst *ptr = mtmp->data;
	int bias;

	bias = is_lord(ptr) + is_prince(ptr) * 2 + extra_nasty(ptr);
	switch (rnd(14 - (2 * bias))) {
		case 1:
			if (strongmonst(ptr))
				mongets(mtmp, BATTLE_AXE);
			else
				m_initthrow(mtmp, DART, 12);
			break;
		case 2:
			if (strongmonst(ptr)) {
				mongets(mtmp, TWO_HANDED_SWORD);
			} else {
				mongets(mtmp, CROSSBOW);
				m_initthrow(mtmp, CROSSBOW_BOLT, 12);
			}
			break;
		case 3:
			mongets(mtmp, BOW);
			m_initthrow(mtmp, ARROW, 12);
			break;
		case 4:
			if (strongmonst(ptr))
				mongets(mtmp, LONG_SWORD);
			else
				m_initthrow(mtmp, DAGGER, 3);
			break;
		case 5:
			if (strongmonst(ptr))
				mongets(mtmp, LUCERN_HAMMER);
			else
				mongets(mtmp, AKLYS);
			break;
		/* [Tom] added some more */
		case 6:
			if (strongmonst(ptr))
				mongets(mtmp, AXE);
			else
				mongets(mtmp, SHORT_SWORD);
			break;
		case 7:
			if (strongmonst(ptr))
				mongets(mtmp, MACE);
			else
				mongets(mtmp, CLUB);
			break;
		default:
			break;
	}

	return;
}

static void m_initweap(struct monst *mtmp) {
	struct permonst *ptr = mtmp->data;
	int mm = monsndx(ptr);
	struct obj *otmp;

	if (Is_rogue_level(&u.uz)) return;

	/*
	 *	first a few special cases:
	 *
	 *		giants get a boulder to throw sometimes.
	 *		ettins get clubs
	 *		kobolds get darts to throw
	 *		centaurs get some sort of bow & arrows or bolts
	 *		soldiers get all sorts of things.
	 *		kops get clubs & cream pies.
	 */
	switch (ptr->mlet) {
		case S_GIANT:
			if (rn2(2)) mongets(mtmp, (mm != PM_ETTIN) ?
								BOULDER :
								CLUB);
			break;
		case S_HUMAN:
			if (is_mercenary(ptr) || mm == PM_SHOPKEEPER || mm == PM_CHIEF_YEOMAN_WARDER || mm == PM_YEOMAN_WARDER) {
				int w1 = 0, w2 = 0;
				switch (mm) {
					case PM_SOLDIER:
						w1 = rn2(2) ? RIFLE : SUBMACHINE_GUN;
						m_initthrow(mtmp, BULLET, 25);
						m_initthrow(mtmp, BULLET, 25);
						w2 = rn2(2) ? KNIFE : DAGGER;
						mongets(mtmp, FRAG_GRENADE);
						break;
					case PM_WATCHMAN:
						if (!rn2(3)) {
							w1 = rn1(BEC_DE_CORBIN - PARTISAN + 1, PARTISAN);
							w2 = rn2(2) ? DAGGER : KNIFE;
						} else
							w1 = rn2(2) ? SPEAR : SHORT_SWORD;
						break;
					case PM_WATCH_CAPTAIN:
						w1 = rn2(2) ? LONG_SWORD : SILVER_SABER;
						break;
					case PM_LIEUTENANT:
						if (rn2(2)) {
							w1 = HEAVY_MACHINE_GUN;
							m_initthrow(mtmp, BULLET, 50);
							m_initthrow(mtmp, BULLET, 50);
							m_initthrow(mtmp, BULLET, 50);
						} else {
							w1 = SUBMACHINE_GUN;
							m_initthrow(mtmp, BULLET, 30);
							m_initthrow(mtmp, BULLET, 30);
						}
						w2 = rn2(2) ? KNIFE : DAGGER;
						if (rn2(2)) {
							mongets(mtmp, FRAG_GRENADE);
							mongets(mtmp, FRAG_GRENADE);
						} else {
							mongets(mtmp, GAS_GRENADE);
							mongets(mtmp, GAS_GRENADE);
						}
						break;
					case PM_SERGEANT:
						if (rn2(2)) {
							w1 = AUTO_SHOTGUN;
							m_initthrow(mtmp, SHOTGUN_SHELL, 10);
							m_initthrow(mtmp, SHOTGUN_SHELL, 10);
						} else {
							w1 = ASSAULT_RIFLE;
							m_initthrow(mtmp, BULLET, 30);
							m_initthrow(mtmp, BULLET, 30);
						}
						w2 = rn2(2) ? DAGGER : KNIFE;
						if (rn2(2)) {
							m_initthrow(mtmp, FRAG_GRENADE, 5);
						} else {
							m_initthrow(mtmp, GAS_GRENADE, 5);
						}
						if (!rn2(5))
							mongets(mtmp, GRENADE_LAUNCHER);
						break;
					case PM_YEOMAN_WARDER:
						w1 = rn2(2) ? FLAIL : MACE;
						break;
					case PM_CAPTAIN:
						if (rn2(2)) {
							w1 = AUTO_SHOTGUN;
							m_initthrow(mtmp, SHOTGUN_SHELL, 20);
							m_initthrow(mtmp, SHOTGUN_SHELL, 20);
						} else if (rn2(2)) {
							w1 = HEAVY_MACHINE_GUN;
							m_initthrow(mtmp, BULLET, 60);
							m_initthrow(mtmp, BULLET, 60);
							m_initthrow(mtmp, BULLET, 60);
						} else {
							w1 = ASSAULT_RIFLE;
							m_initthrow(mtmp, BULLET, 60);
							m_initthrow(mtmp, BULLET, 60);
						}
						if (rn2(2)) {
							w2 = ROCKET_LAUNCHER;
							m_initthrow(mtmp, ROCKET, 5);
						} else if (rn2(2)) {
							mongets(mtmp, GRENADE_LAUNCHER);
							m_initthrow(mtmp,
								    (rn2(2) ? FRAG_GRENADE : GAS_GRENADE),
								    5);
						} else {
							w2 = rn2(2) ? SILVER_SABER : DAGGER;
						}
						break;
					case PM_CHIEF_YEOMAN_WARDER:
						w1 = rn2(2) ? BROADSWORD : LONG_SWORD;
						break;
					case PM_SHOPKEEPER:
						mongets(mtmp, SHOTGUN);
						m_initthrow(mtmp, SHOTGUN_SHELL, 20);
						m_initthrow(mtmp, SHOTGUN_SHELL, 20);
						m_initthrow(mtmp, SHOTGUN_SHELL, 20);
					/* Fallthrough */
					default:
						if (!rn2(4)) w1 = DAGGER;
						if (!rn2(7)) w2 = SPEAR;
						break;
				}
				if (w1) mongets(mtmp, w1);
				if (!w2 && w1 != DAGGER && !rn2(4)) w2 = KNIFE;
				if (w2) mongets(mtmp, w2);
			} else if (is_elf(ptr)) {
				if (mm == PM_DROW) {
					mongets(mtmp, DARK_ELVEN_MITHRIL_COAT);
					mongets(mtmp, DARK_ELVEN_SHORT_SWORD);
					mongets(mtmp, DARK_ELVEN_BOW);
					m_initthrow(mtmp, DARK_ELVEN_ARROW, 12);
				} else {
					if (rn2(2))
						mongets(mtmp,
							!rn2(4) ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
					if (!rn2(3))
						mongets(mtmp, ELVEN_LEATHER_HELM);
					else if (!rn2(4))
						mongets(mtmp, ELVEN_BOOTS);
					if (!rn2(3)) mongets(mtmp, ELVEN_DAGGER);
					switch (rn2(3)) {
						case 0:
							if (!rn2(4)) mongets(mtmp, ELVEN_SHIELD);
							if (rn2(3)) mongets(mtmp, ELVEN_SHORT_SWORD);
							mongets(mtmp, ELVEN_BOW);
							m_initthrow(mtmp, ELVEN_ARROW, 12);
							break;
						case 1:
							mongets(mtmp, ELVEN_BROADSWORD);
							if (rn2(2)) mongets(mtmp, ELVEN_SHIELD);
							break;
						case 2:
							if (rn2(2)) {
								mongets(mtmp, ELVEN_SPEAR);
								mongets(mtmp, ELVEN_SHIELD);
							}
							break;
					}
					if (mm == PM_ELVENKING) {
						if (rn2(3) || (in_mklev && Is_earthlevel(&u.uz)))
							mongets(mtmp, PICK_AXE);
						if (!rn2(50)) mongets(mtmp, CRYSTAL_BALL);
					}
				} /* normal elves */
			} else	  /* enemy characters! */
				if (mm >= PM_ARCHEOLOGIST && mm <= PM_WIZARD && rn2(4)) {
				switch (mm) {
					case PM_ARCHEOLOGIST:
						mongets(mtmp, BULLWHIP);
						mongets(mtmp, LEATHER_JACKET);
						mongets(mtmp, FEDORA);
						if (rn2(2)) mongets(mtmp, PICK_AXE);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_BARBARIAN:
						mongets(mtmp, BATTLE_AXE);
						if (!rn2(2)) mongets(mtmp, TWO_HANDED_SWORD);
						mongets(mtmp, RING_MAIL);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_CAVEMAN:
					case PM_CAVEWOMAN:
						mongets(mtmp, CLUB);
						if (rn2(3)) {
							mongets(mtmp, BOW);
							m_initthrow(mtmp, ARROW, 18);
						}
						mongets(mtmp, LEATHER_ARMOR);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_DOPPELGANGER:
						mongets(mtmp, SILVER_DAGGER);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_ELF:
						/* gets taken care of later... */
						/*mongets(mtmp, ELVEN_SHORT_SWORD);
					if (rn2(3)) {
					mongets(mtmp, ELVEN_BOW);
					 m_initthrow(mtmp, ELVEN_ARROW, 18);
					}
					if (rn2(3)) mongets(mtmp, ELVEN_CLOAK);
					else mongets(mtmp, ELVEN_MITHRIL_COAT);*/
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_FLAME_MAGE:
						mongets(mtmp, QUARTERSTAFF);
						mongets(mtmp, STUDDED_LEATHER_ARMOR);
						mongets(mtmp, WAN_FIRE);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_HEALER:
						mongets(mtmp, SCALPEL);
						mongets(mtmp, LEATHER_GLOVES);
						mongets(mtmp, WAN_HEALING);
						mongets(mtmp, WAN_SLEEP);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 20));
						break;
					case PM_ICE_MAGE:
						mongets(mtmp, QUARTERSTAFF);
						mongets(mtmp, STUDDED_LEATHER_ARMOR);
						mongets(mtmp, WAN_COLD);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_YEOMAN:
					case PM_KNIGHT:
						mongets(mtmp, LONG_SWORD);
						mongets(mtmp, PLATE_MAIL);
						mongets(mtmp, LARGE_SHIELD);
						mongets(mtmp, HELMET);
						mongets(mtmp, LEATHER_GLOVES);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_MONK:
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 5));
						break;
					case PM_NECROMANCER:
						mongets(mtmp, ATHAME);
						if (!rn2(4)) mongets(mtmp, PICK_AXE);
						mongets(mtmp, rnd_offensive_item(mtmp));
						mongets(mtmp, rnd_defensive_item(mtmp));
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_PRIEST:
					case PM_PRIESTESS:
						mongets(mtmp, MACE);
						mongets(mtmp, rn1(ROBE_OF_WEAKNESS - ROBE + 1, ROBE));
						mongets(mtmp, SMALL_SHIELD);
						if (!rn2(4)) {
							int v, vials;
							vials = rn2(4) + 1;
							for (v = 0; v < vials; v++) {
								otmp = mksobj(POT_WATER, false, false);
								bless(otmp);
								mpickobj(mtmp, otmp);
							}
						}
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_ROGUE:
						mongets(mtmp, SHORT_SWORD);
						mongets(mtmp, LEATHER_ARMOR);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 25));
						break;
					case PM_SAMURAI:
						mongets(mtmp, KATANA);
						if (rn2(2)) mongets(mtmp, SHORT_SWORD);
						if (rn2(3)) {
							mongets(mtmp, YUMI);
							m_initthrow(mtmp, YA, 18);
						}
						mongets(mtmp, SPLINT_MAIL);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_TOURIST:
						m_initthrow(mtmp, DART, 18);
						mongets(mtmp, HAWAIIAN_SHIRT);
						if (rn2(2)) mongets(mtmp, EXPENSIVE_CAMERA);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 20));
						break;
					case PM_UNDEAD_SLAYER:
						mongets(mtmp, SILVER_SPEAR);
						mongets(mtmp, CHAIN_MAIL);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_VALKYRIE:
						mongets(mtmp, LONG_SWORD);
						if (!rn2(3)) m_initthrow(mtmp, DAGGER, 4);
						mongets(mtmp, SMALL_SHIELD);
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					case PM_WIZARD:
						mongets(mtmp, ATHAME);
						mongets(mtmp, rnd_offensive_item(mtmp));
						mongets(mtmp, rnd_offensive_item(mtmp));
						mongets(mtmp, rnd_defensive_item(mtmp));
						mkmonmoney(mtmp, (long)d(mtmp->m_lev, 15));
						break;
					default:
						break;
				}
				if ((int)mtmp->m_lev > rn2(40))
					mongets(mtmp, rnd_offensive_item(mtmp));
				if ((int)mtmp->m_lev > rn2(40))
					mongets(mtmp, rnd_offensive_item(mtmp));
				if ((int)mtmp->m_lev > rn2(40))
					mongets(mtmp, rnd_defensive_item(mtmp));
				if ((int)mtmp->m_lev > rn2(40))
					mongets(mtmp, rnd_defensive_item(mtmp));
				if ((int)mtmp->m_lev > rn2(40))
					mongets(mtmp, rnd_misc_item(mtmp));
				if ((int)mtmp->m_lev > rn2(40))
					mongets(mtmp, rnd_misc_item(mtmp));
			} /* end of other characters */
			  /*break;*/
			else if (ptr->msound == MS_PRIEST ||
				 quest_mon_represents_role(ptr, PM_PRIEST)) {
				otmp = mksobj(MACE, false, false);
				if (otmp) {
					otmp->spe = rnd(3);
					if (!rn2(2)) curse(otmp);
					mpickobj(mtmp, otmp);
				}

				/* MRKR: Dwarves in the Mines sometimes carry torches */

				if (In_mines(&u.uz)) {
					if (!rn2(4)) {
						otmp = mksobj(TORCH, true, false);
						otmp->quan = 1;
						mpickobj(mtmp, otmp);

						/* If this spot is unlit, light the torch */

						if (!levl[mtmp->mx][mtmp->my].lit) {
							begin_burn(otmp, false);
						}
					}
				}
			}
			break;

		case S_ANGEL: if (humanoid(ptr)) {
			int spe2;

			/* create minion stuff; can't use mongets */
			otmp = mksobj(LONG_SWORD, false, false);

			/* maybe make it special */
			if (!rn2(20) || is_lord(ptr))
				otmp = oname(otmp, artiname(
							   rn2(2) ? ART_DEMONBANE : ART_SUNSWORD));
			bless(otmp);
			otmp->oerodeproof = true;
			spe2 = rn2(4);
			otmp->spe = max(otmp->spe, spe2);
			mpickobj(mtmp, otmp);

			otmp = mksobj(!rn2(4) || is_lord(ptr) ?
					      SHIELD_OF_REFLECTION :
					      LARGE_SHIELD,
				      false, false);
			otmp->cursed = false;
			otmp->oerodeproof = true;
			otmp->spe = 0;
			mpickobj(mtmp, otmp);
		} break;
		case S_GNOME:
			switch (mm) {
				case PM_GNOLL:
					if (!rn2(3)) mongets(mtmp, ORCISH_HELM);
					if (!rn2(3)) mongets(mtmp, STUDDED_LEATHER_ARMOR);
					if (!rn2(3)) mongets(mtmp, ORCISH_SHIELD);
					if (!rn2(4)) mongets(mtmp, SPEAR);
					break;

				case PM_GNOLL_WARRIOR:
					if (!rn2(2)) mongets(mtmp, ORCISH_HELM);

					if (!rn2(20))
						mongets(mtmp, ORANGE_DRAGON_SCALE_MAIL);
					else if (rn2(3))
						mongets(mtmp, SCALE_MAIL);
					else
						mongets(mtmp, SPLINT_MAIL);

					if (!rn2(2)) mongets(mtmp, ORCISH_SHIELD);
					if (!rn2(3)) mongets(mtmp, KATANA);
					break;

				case PM_GNOLL_CHIEFTAIN:
					mongets(mtmp, ORCISH_HELM);

					if (!rn2(10))
						mongets(mtmp, BLUE_DRAGON_SCALE_MAIL);
					else
						mongets(mtmp, CRYSTAL_PLATE_MAIL);

					mongets(mtmp, ORCISH_SHIELD);
					mongets(mtmp, KATANA);
					mongets(mtmp, rnd_offensive_item(mtmp));
					break;

				case PM_GNOLL_SHAMAN:
					if (!rn2(10))
						mongets(mtmp, SILVER_DRAGON_SCALE_MAIL);
					else if (rn2(5))
						mongets(mtmp, CRYSTAL_PLATE_MAIL);
					else
						mongets(mtmp, RED_DRAGON_SCALE_MAIL);

					mongets(mtmp, ATHAME);
					m_initthrow(mtmp, SHURIKEN, 12);
					mongets(mtmp, rnd_offensive_item(mtmp));
					mongets(mtmp, rnd_offensive_item(mtmp));
					break;

				default:
					m_initweap_normal(mtmp);
					break;
			}
			break;
		case S_HUMANOID:
			if (is_dwarf(ptr)) {
				if (rn2(7)) mongets(mtmp, DWARVISH_CLOAK);
				if (rn2(7)) mongets(mtmp, IRON_SHOES);
				if (!rn2(4)) {
					mongets(mtmp, DWARVISH_SHORT_SWORD);
					/* note: you can't use a mattock with a shield */
					if (rn2(2))
						mongets(mtmp, DWARVISH_MATTOCK);
					else {
						mongets(mtmp, AXE);
						mongets(mtmp, DWARVISH_ROUNDSHIELD);
					}
					mongets(mtmp, DWARVISH_IRON_HELM);
					if (!rn2(4))
						mongets(mtmp, DWARVISH_MITHRIL_COAT);
				} else {
					mongets(mtmp, !rn2(3) ? PICK_AXE : DAGGER);
				}
			} else if (is_hobbit(ptr)) {
				switch (rn2(3)) {
					case 0:
						mongets(mtmp, DAGGER);
						break;
					case 1:
						mongets(mtmp, ELVEN_DAGGER);
						break;
					case 2:
						mongets(mtmp, SLING);
						/* WAC give them some rocks to throw */
						m_initthrow(mtmp, ROCK, 2);
						break;
				}
				/* WAC add 50% chance of leather */
				if (!rn2(10))
					mongets(mtmp, ELVEN_MITHRIL_COAT);
				else if (!rn2(2))
					mongets(mtmp, LEATHER_ARMOR);
				if (!rn2(10)) mongets(mtmp, DWARVISH_CLOAK);
			} else
				switch (mm) {
					/* Mind flayers get robes */
					case PM_MIND_FLAYER:
						if (!rn2(2)) mongets(mtmp, ROBE);
						break;
					case PM_MASTER_MIND_FLAYER:
						if (!rn2(10))
							mongets(mtmp, ROBE_OF_PROTECTION);
						else if (!rn2(10))
							mongets(mtmp, ROBE_OF_POWER);
						else
							mongets(mtmp, ROBE);
						break;
					case PM_GNOLL:
						if (!rn2(2)) switch (rn2(3)) {
								case 0:
									mongets(mtmp, BARDICHE);
									break;
								case 1:
									mongets(mtmp, VOULGE);
									break;
								case 2:
									mongets(mtmp, HALBERD);
									break;
							}
						if (!rn2(2)) mongets(mtmp, LEATHER_ARMOR);
						break;
					default:
						break;
				}
			break;
		case S_KOP: /* create Keystone Kops with cream pies to
				 * throw. As suggested by KAA.	   [MRS]
				 */
			if (!rn2(4)) m_initthrow(mtmp, CREAM_PIE, 2);
			if (!rn2(3)) mongets(mtmp, (rn2(2)) ? CLUB : RUBBER_HOSE);
			break;
		case S_ORC:
			/* All orcs will get at least an orcish dagger*/
			if (rn2(2)) mongets(mtmp, ORCISH_HELM);
			switch (mm != PM_ORC_CAPTAIN ? mm :
						       rn2(2) ? PM_MORDOR_ORC : PM_URUK_HAI) {
				case PM_MORDOR_ORC:
					if (!rn2(3))
						mongets(mtmp, SCIMITAR);
					else
						mongets(mtmp, ORCISH_DAGGER);
					if (!rn2(3)) mongets(mtmp, ORCISH_SHIELD);
					if (!rn2(3)) mongets(mtmp, KNIFE);
					/* WAC add possible orcish spear */
					if (!rn2(4)) m_initthrow(mtmp, ORCISH_SPEAR, 1);
					if (!rn2(3)) mongets(mtmp, KNIFE);
					if (!rn2(3)) mongets(mtmp, ORCISH_CHAIN_MAIL);
					break;
				case PM_URUK_HAI:
					if (!rn2(3)) mongets(mtmp, ORCISH_CLOAK);
					if (!rn2(3))
						mongets(mtmp, ORCISH_SHORT_SWORD);
					else
						mongets(mtmp, ORCISH_DAGGER);
					if (!rn2(3)) mongets(mtmp, IRON_SHOES);
					if (!rn2(3)) {
						mongets(mtmp, ORCISH_BOW);
						m_initthrow(mtmp, ORCISH_ARROW, 12);
					}
					if (!rn2(3)) mongets(mtmp, URUK_HAI_SHIELD);
					break;
				case PM_GOBLIN:
					if (!rn2(3))
						mongets(mtmp, ORCISH_SHORT_SWORD);
					else
						mongets(mtmp, ORCISH_DAGGER);
					break;
				case PM_HOBGOBLIN:
					if (!rn2(3))
						mongets(mtmp, MORNING_STAR);
					else if (!rn2(3))
						mongets(mtmp, ORCISH_SHORT_SWORD);
					else
						mongets(mtmp, ORCISH_DAGGER);
					break;
				default:
					if (mm != PM_ORC_SHAMAN && rn2(2))
						/*                          mongets(mtmp, (mm == PM_GOBLIN || rn2(2) == 0)*/
						mongets(mtmp, (rn2(2) == 0) ? ORCISH_DAGGER : SCIMITAR);
			}
			break;
		case S_OGRE:
			if (!rn2(mm == PM_OGRE_KING ? 3 : mm == PM_OGRE_LORD ? 6 : 12))
				mongets(mtmp, BATTLE_AXE);
			else
				mongets(mtmp, CLUB);
			break;
		case S_KOBOLD:
			/* WAC gets orcish 1:4, otherwise darts
		        (used to be darts 1:4)
		   gets orcish short sword 1:4, otherwise orcish dagger */
			if (!rn2(4))
				m_initthrow(mtmp, ORCISH_SPEAR, 1);
			else
				m_initthrow(mtmp, DART, 12);
			if (!rn2(4))
				mongets(mtmp, ORCISH_SHORT_SWORD);
			else
				mongets(mtmp, ORCISH_DAGGER);
			break;

		case S_CENTAUR:
			if (rn2(2)) {
				if (ptr == &mons[PM_FOREST_CENTAUR]) {
					mongets(mtmp, BOW);
					m_initthrow(mtmp, ARROW, 12);
				} else {
					mongets(mtmp, CROSSBOW);
					m_initthrow(mtmp, CROSSBOW_BOLT, 12);
				}
			}
			break;
		case S_WRAITH:
			mongets(mtmp, KNIFE);
			mongets(mtmp, LONG_SWORD);
			break;
		case S_ZOMBIE:
			if (!rn2(4)) mongets(mtmp, LEATHER_ARMOR);
			if (!rn2(4))
				mongets(mtmp, (rn2(3) ? KNIFE : SHORT_SWORD));
			break;
		case S_LIZARD:
			if (mm == PM_SALAMANDER)
				mongets(mtmp, (rn2(7) ? SPEAR : rn2(3) ? TRIDENT : STILETTO));
			break;
		case S_TROLL:
			if (!rn2(2)) switch (rn2(4)) {
					case 0:
						mongets(mtmp, RANSEUR);
						break;
					case 1:
						mongets(mtmp, PARTISAN);
						break;
					case 2:
						mongets(mtmp, GLAIVE);
						break;
					case 3:
						mongets(mtmp, SPETUM);
						break;
				}
			break;
		case S_DEMON:
			switch (mm) {
				case PM_BALROG:
					mongets(mtmp, BULLWHIP);
					mongets(mtmp, BROADSWORD);
					break;
				case PM_ORCUS:
					mongets(mtmp, WAN_DEATH); /* the Wand of Orcus */
					break;
				case PM_HORNED_DEVIL:
					mongets(mtmp, rn2(4) ? TRIDENT : BULLWHIP);
					break;
				case PM_DISPATER:
					mongets(mtmp, WAN_STRIKING);
					break;
				case PM_YEENOGHU:
					mongets(mtmp, FLAIL);
					break;
			}
			/* prevent djinnis and mail daemons from leaving objects when
		 * they vanish
		 */
			if (!is_demon(ptr)) break;
		/* fall thru */
		/*
	 *	Now the general case, Some chance of getting some type
	 *	of weapon for "normal" monsters.  Certain special types
	 *	of monsters will get a bonus chance or different selections.
	 */
		default:
			m_initweap_normal(mtmp);
			break;
	}
	/*    if ((int) mtmp->m_lev > rn2(120)) */
	if ((int)mtmp->m_lev > rn2(200))
		mongets(mtmp, rnd_offensive_item(mtmp));
}

/*
 *   Makes up money for monster's inventory.
 *   This will change with silver & copper coins
 */
void mkmonmoney(struct monst *mtmp, long amount) {
	struct obj *gold = mksobj(GOLD_PIECE, false, false);
	gold->quan = amount;
	add_to_minv(mtmp, gold);
}

static void m_initinv(struct monst *mtmp) {
	int cnt;
	struct obj *otmp;
	struct permonst *ptr = mtmp->data;
	/* 	char *opera_cloak = "opera cloak";*/
	int i;

	if (Is_rogue_level(&u.uz)) return;

	/*
	 *	Soldiers get armour & rations - armour approximates their ac.
	 *	Nymphs may get mirror or potion of object detection.
	 */
	switch (ptr->mlet) {
		case S_HUMAN:
			if (is_mercenary(ptr) || ptr == &mons[PM_CHIEF_YEOMAN_WARDER] || ptr == &mons[PM_YEOMAN_WARDER]) {
				int mac;

				switch (monsndx(ptr)) {
					case PM_GUARD:
						mac = -1;
						break;
					case PM_SOLDIER:
						mac = 3;
						break;
					case PM_SERGEANT:
						mac = 0;
						break;
					case PM_LIEUTENANT:
						mac = -2;
						break;
					case PM_CAPTAIN:
						mac = -3;
						break;
					case PM_YEOMAN_WARDER:
					case PM_WATCHMAN:
						mac = 3;
						break;
					case PM_CHIEF_YEOMAN_WARDER:
						mongets(mtmp, TALLOW_CANDLE);
					//fallthru
					case PM_WATCH_CAPTAIN:
						mac = -2;
						break;
					default:
						impossible("odd mercenary %d?", monsndx(ptr));
						mac = 0;
						break;
				}

				if (mac < -1 && rn2(5))
					mac += 7 + mongets(mtmp, (rn2(5)) ?
									 PLATE_MAIL :
									 CRYSTAL_PLATE_MAIL);
				else if (mac < 3 && rn2(5))
					mac += 6 + mongets(mtmp, (rn2(3)) ?
									 SPLINT_MAIL :
									 BANDED_MAIL);
				else if (rn2(5))
					mac += 3 + mongets(mtmp, (rn2(3)) ?
									 RING_MAIL :
									 STUDDED_LEATHER_ARMOR);
				else
					mac += 2 + mongets(mtmp, LEATHER_ARMOR);

				if (mac < 10 && rn2(3))
					mac += 1 + mongets(mtmp, HELMET);
				else if (mac < 10 && rn2(2))
					mac += 1 + mongets(mtmp, DENTED_POT);
				if (mac < 10 && rn2(3))
					mac += 1 + mongets(mtmp, SMALL_SHIELD);
				else if (mac < 10 && rn2(2))
					mac += 2 + mongets(mtmp, LARGE_SHIELD);
				if (mac < 10 && rn2(3))
					mac += 1 + mongets(mtmp, LOW_BOOTS);
				else if (mac < 10 && rn2(2))
					mac += 2 + mongets(mtmp, HIGH_BOOTS);
				if (mac < 10 && rn2(3))
					mac += 1 + mongets(mtmp, LEATHER_GLOVES);
				else if (mac < 10 && rn2(2))
					mac += 1 + mongets(mtmp, LEATHER_CLOAK);

				if (ptr != &mons[PM_GUARD] &&
				    ptr != &mons[PM_WATCHMAN] &&
				    ptr != &mons[PM_WATCH_CAPTAIN]) {
					if (!rn2(3)) mongets(mtmp, K_RATION);
					if (!rn2(2)) mongets(mtmp, C_RATION);
					if (ptr != &mons[PM_SOLDIER] && !rn2(3))
						mongets(mtmp, BUGLE);
				} else if (ptr == &mons[PM_WATCHMAN] && rn2(3))
					mongets(mtmp, TIN_WHISTLE);
			} else if (ptr == &mons[PM_SHOPKEEPER]) {
				mongets(mtmp, SKELETON_KEY);
				/* STEPHEN WHITE'S NEW CODE
				 *
				 * "Were here to pump *clap* YOU up!"  -Hans and Frans
				 *                                      Saterday Night Live
				 */
				switch (rn2(4)) {
					case 0: mongets(mtmp, WAN_MAGIC_MISSILE); //fallthru
					case 1: mongets(mtmp, POT_EXTRA_HEALING); //fallthru
					case 2: mongets(mtmp, POT_HEALING); //fallthru
					case 3: mongets(mtmp, WAN_STRIKING);
				}
				switch (rnd(4)) {
					case 1: mongets(mtmp, POT_HEALING); //fallthru
					case 2: mongets(mtmp, POT_EXTRA_HEALING); //fallthru
					case 3: mongets(mtmp, SCR_TELEPORTATION); //fallthru
					case 4: mongets(mtmp, WAN_TELEPORTATION);
				}
			} else if (ptr->msound == MS_PRIEST ||
				   quest_mon_represents_role(ptr, PM_PRIEST)) {
				mongets(mtmp,
					rn2(7) ? rn1(ROBE_OF_WEAKNESS - ROBE + 1, ROBE) :
						 rn2(3) ? CLOAK_OF_PROTECTION :
							  CLOAK_OF_MAGIC_RESISTANCE);
				mongets(mtmp, SMALL_SHIELD);
				mkmonmoney(mtmp, (long)rn1(10, 20));
			} else if (quest_mon_represents_role(ptr, PM_MONK)) {
				mongets(mtmp, rn2(11) ? ROBE :
							CLOAK_OF_MAGIC_RESISTANCE);
			}
			break;
		case S_NYMPH:
			if (!rn2(2)) mongets(mtmp, MIRROR);
			if (!rn2(2)) mongets(mtmp, POT_OBJECT_DETECTION);
			break;
		case S_GIANT:
			if (ptr == &mons[PM_MINOTAUR]) {
				if (!rn2(3) || (in_mklev && Is_earthlevel(&u.uz)))
					mongets(mtmp, WAN_DIGGING);
			} else if (is_giant(ptr)) {
				for (cnt = rn2((int)(mtmp->m_lev / 2)); cnt; cnt--) {
					otmp = mksobj(rnd_class(DILITHIUM_CRYSTAL, LUCKSTONE - 1),
						      false, false);
					otmp->quan = (long)rn1(2, 3);
					otmp->owt = weight(otmp);
					mpickobj(mtmp, otmp);
				}
			}
			break;
		case S_WRAITH:
			if (!rn2(2)) mongets(mtmp, ROBE);
			if (ptr == &mons[PM_NAZGUL]) {
				otmp = mksobj(RIN_INVISIBILITY, false, false);
				curse(otmp);
				mpickobj(mtmp, otmp);
			}
			break;
		case S_LICH:
			if (ptr == &mons[PM_MASTER_LICH] && !rn2(13))
				mongets(mtmp, (rn2(7) ? ATHAME : WAN_NOTHING));
			else if (ptr == &mons[PM_ARCH_LICH] && !rn2(3)) {
				otmp = mksobj(rn2(3) ? ATHAME : QUARTERSTAFF,
					      true, rn2(13) ? false : true);
				if (otmp->spe < 2) otmp->spe = rnd(3);
				if (!rn2(4)) otmp->oerodeproof = 1;
				mpickobj(mtmp, otmp);
			}
			break;
		case S_MUMMY:
			if (rn2(7)) mongets(mtmp, MUMMY_WRAPPING);
			break;
		case S_QUANTMECH:
			if (monsndx(ptr) == PM_QUANTUM_MECHANIC && !rn2(20)) {
				otmp = mksobj(LARGE_BOX, false, false);
				otmp->spe = 1; /* flag for special box */
				otmp->owt = weight(otmp);
				mpickobj(mtmp, otmp);
			}
			if (monsndx(ptr) == PM_DOCTOR_FRANKENSTEIN) {
				mongets(mtmp, LAB_COAT);
				mongets(mtmp, WAN_POLYMORPH);
				mongets(mtmp, SPE_POLYMORPH);
			}
			break;
		case S_LEPRECHAUN:
			mkmonmoney(mtmp, (long)d(level_difficulty(), 30));
			break;
		case S_ELEMENTAL:
			/*            if(ptr == &mons[PM_WATER_WEIRD]){
				otmp = mksobj(WAN_WISHING,true,false);
				otmp->spe=3;
				otmp->blessed=0;
				mpickobj(mtmp, otmp);
			}*/
			break;
		case S_VAMPIRE:
			/* [Lethe] Star and fire vampires don't get this stuff */
			if (ptr == &mons[PM_STAR_VAMPIRE] ||
			    ptr == &mons[PM_FIRE_VAMPIRE])
				break;
			/* Get opera cloak */
			/*	    	otmp = readobjnam(opera_cloak);
				if (otmp && otmp != &zeroobj) mpickobj(mtmp, otmp);*/
			for (i = STRANGE_OBJECT; i < NUM_OBJECTS; i++) {
				const char *zn;
				if ((zn = OBJ_DESCR(objects[i])) && !strcmpi(zn, "opera cloak")) {
					if (!OBJ_NAME(objects[i])) i = STRANGE_OBJECT;
					break;
				}
			}
			if (i != NUM_OBJECTS) mongets(mtmp, i);
			if (rn2(2)) {
				if ((int)mtmp->m_lev > rn2(30))
					mongets(mtmp, POT_VAMPIRE_BLOOD);
				else
					mongets(mtmp, POT_BLOOD);
			}
			break;
		case S_DEMON:
			/* moved here from m_initweap() because these don't
		   have AT_WEAP so m_initweap() is not called for them */
			if (ptr == &mons[PM_ICE_DEVIL] && !rn2(4)) {
				mongets(mtmp, SPEAR);
				/* [DS] Cthulhu isn't fully integrated yet, and he won't be
			 *      until Moloch's Sanctum is rearranged */
			} else if (ptr == &mons[PM_CTHULHU]) {
				mongets(mtmp, AMULET_OF_YENDOR);
				mongets(mtmp, WAN_DEATH);
				mongets(mtmp, POT_FULL_HEALING);
			} else if (ptr == &mons[PM_ASMODEUS]) {
				mongets(mtmp, WAN_COLD);
				mongets(mtmp, WAN_FIRE);
			}
			break;
		default:
			break;
	}

	/* ordinary soldiers rarely have access to magic (or gold :-) */
	if (ptr == &mons[PM_SOLDIER] && rn2(15)) return;

	if ((int)mtmp->m_lev > rn2(200))
		mongets(mtmp, rnd_defensive_item(mtmp));
	if ((int)mtmp->m_lev > rn2(200))
		mongets(mtmp, rnd_misc_item(mtmp));
	if (likes_gold(ptr) && !findgold(mtmp->minvent) && !rn2(5))
		mkmonmoney(mtmp, (long)d(level_difficulty(), mtmp->minvent ? 5 : 10));
}

// Note: for long worms, always call cutworm (cutworm calls clone_mon) */
struct monst *clone_mon(struct monst *mon, xchar x, xchar y /* clone's preferred location or 0 (near mon) */) {
	coord mm;
	struct monst *m2;

	/* may be too weak or have been extinguished for population control */
	if (mon->mhp <= 1 || (mvitals[monsndx(mon->data)].mvflags & G_EXTINCT))
		return NULL;

	if (x == 0) {
		mm.x = mon->mx;
		mm.y = mon->my;
		if (!enexto(&mm, mm.x, mm.y, mon->data) || MON_AT(mm.x, mm.y))
			return NULL;
	} else if (!isok(x, y)) {
		return NULL; /* paranoia */
	} else {
		mm.x = x;
		mm.y = y;
		if (MON_AT(mm.x, mm.y)) {
			if (!enexto(&mm, mm.x, mm.y, mon->data) || MON_AT(mm.x, mm.y))
				return NULL;
		}
	}
	m2 = newmonst(0);
	*m2 = *mon; /* copy condition of old monster */
	m2->nmon = fmon;
	fmon = m2;
	m2->m_id = context.ident++;
	if (!m2->m_id) m2->m_id = context.ident++; /* ident overflowed */
	m2->mx = mm.x;
	m2->my = mm.y;

	m2->minvent = NULL; /* objects don't clone */
	m2->mleashed = false;

	/* Max HP the same, but current HP halved for both.  The caller
	 * might want to override this by halving the max HP also.
	 * When current HP is odd, the original keeps the extra point.
	 */
	m2->mhpmax = mon->mhpmax;
	m2->mhp = mon->mhp / 2;
	mon->mhp -= m2->mhp;

	/* Same for the power */
	m2->m_enmax = mon->m_enmax;
	m2->m_en = mon->m_en /= 2;

	/* since shopkeepers and guards will only be cloned if they've been
	 * polymorphed away from their original forms, the clone doesn't have
	 * room for the extra information.  we also don't want two shopkeepers
	 * around for the same shop.
	 */
	if (mon->isshk) m2->isshk = false;
	if (mon->isgd) m2->isgd = false;
	if (mon->ispriest) m2->ispriest = false;
	if (mon->isgyp) m2->isgyp = false;
	m2->mxlth = 0;
	place_monster(m2, m2->mx, m2->my);
	if (emits_light(m2->data))
		new_light_source(m2->mx, m2->my, emits_light(m2->data),
				 LS_MONSTER, monst_to_any(m2));
	if (m2->mnamelth) {
		m2->mnamelth = 0; /* or it won't get allocated */
		m2 = christen_monst(m2, NAME(mon));
	} else if (mon->isshk) {
		m2 = christen_monst(m2, shkname(mon));
	}

	/* not all clones caused by player are tame or peaceful */
	if (!context.mon_moving) {
		if (mon->mtame)
			m2->mtame = rn2(max(2 + u.uluck, 2)) ? mon->mtame : 0;
		else if (mon->mpeaceful)
			m2->mpeaceful = rn2(max(2 + u.uluck, 2)) ? 1 : 0;
	}

	newsym(m2->mx, m2->my); /* display the new monster */
	if (m2->mtame) {
		struct monst *m3;

		if (mon->isminion) {
			m3 = newmonst(sizeof(struct epri) + mon->mnamelth);
			*m3 = *m2;
			m3->mxlth = sizeof(struct epri);
			if (m2->mnamelth) strcpy(NAME(m3), NAME(m2));
			*(EPRI(m3)) = *(EPRI(mon));
			replmon(m2, m3);
			m2 = m3;
		} else {
			/* because m2 is a copy of mon it is tame but not init'ed.
			 * however, tamedog will not re-tame a tame dog, so m2
			 * must be made non-tame to get initialized properly.
			 */
			m2->mtame = 0;
			if ((m3 = tamedog(m2, NULL)) != 0) {
				m2 = m3;
				*(EDOG(m2)) = *(EDOG(mon));
			}
		}
	}
	set_malign(m2);

	return m2;
}

/*
 * Propagate a species
 *
 * Once a certain number of monsters are created, don't create any more
 * at random (i.e. make them extinct).  The previous (3.2) behavior was
 * to do this when a certain number had _died_, which didn't make
 * much sense.
 *
 * Returns false propagation unsuccessful
 *         true  propagation successful
 */
boolean propagate(int mndx, boolean tally, boolean ghostly) {
	boolean result;
	uchar lim = mbirth_limit(mndx);
	boolean gone = (mvitals[mndx].mvflags & G_GONE); /* genocided or extinct */

	result = (((int)mvitals[mndx].born < lim) && !gone) ? true : false;

	/* if it's unique, don't ever make it again */
	if (mons[mndx].geno & G_UNIQ) mvitals[mndx].mvflags |= G_EXTINCT;

	if (mvitals[mndx].born < 255 && tally && (!ghostly || (ghostly && result)))
		mvitals[mndx].born++;
	if ((int)mvitals[mndx].born >= lim && !(mons[mndx].geno & G_NOGEN) &&
	    !(mvitals[mndx].mvflags & G_EXTINCT)) {
#ifdef DEBUG
		if (wizard) pline("Automatically extinguished %s.",
				  makeplural(mons[mndx].mname));
#endif
		mvitals[mndx].mvflags |= G_EXTINCT;
		reset_rndmonst(mndx);
	}
	return result;
}

/* set up a new monster's initial level and hit points;
   used by newcham() as well as by makemon() */
void newmonhp(struct monst *mon, int mndx) {
	struct permonst *ptr = &mons[mndx];

	mon->m_lev = adj_lev(ptr);

	if (ptr >= &mons[PM_ARCHEOLOGIST] && ptr <= &mons[PM_WIZARD]) {
		/* enemy characters are of varying level */
		int base_you, base_lev;
		base_you = (u.ulevel / 2) + 1;
		base_lev = level_difficulty() + 1;
		if (base_you < 1) base_you = 1;
		if (base_lev < 1) base_lev = 1;
		mon->m_lev = (1 + rn2(base_you) + rn2(base_lev) / 2) + 1;
	}

	if (is_golem(ptr)) {
		mon->mhpmax = mon->mhp = golemhp(mndx);
	} else if (is_rider(ptr)) {
		// We want low HP, but a high mlevel so they can attack well
		mon->mhpmax = mon->mhp = d(10, 8) + 20;
	} else if (ptr->mlevel > 49) {
		/* "special" fixed hp monster
		 * the hit points are encoded in the mlevel in a somewhat strange
		 * way to fit in the 50..127 positive range of a signed character
		 * above the 1..49 that indicate "normal" monster levels */
		mon->mhpmax = mon->mhp = 2 * (ptr->mlevel - 6);
		mon->m_lev = mon->mhp / 4; /* approximation */
	} else if (ptr->mlet == S_DRAGON && mndx >= PM_GRAY_DRAGON) {
		// adult dragons
		mon->mhpmax = mon->mhp = (In_endgame(&u.uz) ?  (8 * mon->m_lev) :
					  (4 * mon->m_lev + d(mon->m_lev, 4)));
	} else if (!mon->m_lev) {
		mon->mhpmax = mon->mhp = rnd(4);
	} else {
		mon->mhpmax = mon->mhp = d(mon->m_lev, 8);

		if (is_home_elemental(ptr)) mon->mhpmax = (mon->mhp *= 3);
		else mon->mhpmax = mon->mhp = d(mon->m_lev, 8) + (mon->m_lev * rnd(2));
	}
}

/*
 * called with [x,y] = coordinates;
 *	[0,0] means anyplace
 *	[u.ux,u.uy] means: near player (if !in_mklev)
 *
 *	In case we make a monster group, only return the one at [x,y].
 */
struct monst *makemon(struct permonst *ptr, int x, int y, int mmflags) {
	struct monst *mtmp;
	int mndx, mcham, ct, mitem, xlth;
	boolean anymon = (!ptr);
	boolean byyou = (x == u.ux && y == u.uy);
	boolean allow_minvent = ((mmflags & NO_MINVENT) == 0);
	boolean countbirth = ((mmflags & MM_NOCOUNTBIRTH) == 0);
	unsigned gpflags = (mmflags & MM_IGNOREWATER) ? MM_IGNOREWATER : 0;

	/* if caller wants random location, do it here */
	if (x == 0 && y == 0) {
		int tryct = 0; /* careful with bigrooms */
		struct monst fakemon;

		fakemon.data = ptr; /* set up for goodpos */
		do {
			x = rn1(COLNO - 3, 2);
			y = rn2(ROWNO);
		} while (!goodpos(x, y, ptr ? &fakemon : NULL, gpflags) ||
			 (!in_mklev && tryct++ < 50 && cansee(x, y)));
	} else if (byyou && !in_mklev) {
		coord bypos;

		if (enexto_core(&bypos, u.ux, u.uy, ptr, gpflags)) {
			x = bypos.x;
			y = bypos.y;
		} else
			return NULL;
	}

	/* Does monster already exist at the position? */
	if (MON_AT(x, y)) {
		if ((mmflags & MM_ADJACENTOK) != 0) {
			coord bypos;
			if (enexto_core(&bypos, x, y, ptr, gpflags)) {
				x = bypos.x;
				y = bypos.y;
			} else
				return NULL;
		} else
			return NULL;
	}

	if (ptr) {
		mndx = monsndx(ptr);
		/* if you are to make a specific monster and it has
		   already been genocided, return */
		if (mvitals[mndx].mvflags & G_GENOD) return NULL;
#ifdef DEBUG
		if (wizard && (mvitals[mndx].mvflags & G_EXTINCT))
			pline("Explicitly creating extinct monster %s.",
			      mons[mndx].mname);
#endif
	} else {
		/* make a random (common) monster that can survive here.
		 * (the special levels ask for random monsters at specific
		 * positions, causing mass drowning on the medusa level,
		 * for instance.)
		 */
		int tryct = 0; /* maybe there are no good choices */
		struct monst fakemon;
		do {
			if (!(ptr = rndmonst())) {
#ifdef DEBUG
				pline("Warning: no monster.");
#endif
				return NULL; /* no more monsters! */
			}
			fakemon.data = ptr; /* set up for goodpos */
		} while (!goodpos(x, y, &fakemon, gpflags) && tryct++ < 50);
		mndx = monsndx(ptr);
	}
	propagate(mndx, countbirth, false);
	xlth = ptr->pxlth;
	if (mmflags & MM_EDOG)
		xlth += sizeof(struct edog);
	else if (mmflags & MM_EMIN)
		xlth += sizeof(struct emin);
	mtmp = newmonst(xlth);
	*mtmp = zeromonst; /* clear all entries in structure */
	memset(mtmp->mextra, 0, xlth);
	mtmp->nmon = fmon;
	fmon = mtmp;
	mtmp->m_id = context.ident++;
	if (!mtmp->m_id) mtmp->m_id = context.ident++; /* ident overflowed */
	set_mon_data(mtmp, ptr, 0);
	if (mtmp->data->msound == MS_LEADER)
		quest_status.leader_m_id = mtmp->m_id;
	mtmp->mxlth = xlth;
	mtmp->mnum = mndx;

	/* WAC set oldmonnm */
	mtmp->oldmonnm = monsndx(ptr);

	// set up level and hit points
	newmonhp(mtmp, mndx);

	/* Assign power */
	if (mindless(ptr)) {
		mtmp->m_enmax = mtmp->m_en = 0;
	} else {
		/* This is actually quite similar to hit dice,
		 * but with more randomness
		 */
		mtmp->m_enmax = mtmp->m_en =
			d((int)mtmp->m_lev * 2, 4) + (mtmp->m_lev * rnd(2));
	}
	if (is_female(ptr))
		mtmp->female = true;
	else if (is_male(ptr))
		mtmp->female = false;
	else
		mtmp->female = rn2(2); /* ignored for neuters */

	if (In_sokoban(&u.uz) && !mindless(ptr)) /* know about traps here */
		mtmp->mtrapseen = (1L << (PIT - 1)) | (1L << (HOLE - 1));
	if (ptr->msound == MS_LEADER) /* leader knows about portal */
		mtmp->mtrapseen |= (1L << (MAGIC_PORTAL - 1));

	place_monster(mtmp, x, y);
	mtmp->mcansee = mtmp->mcanmove = true;
	mtmp->mpeaceful = (mmflags & MM_ANGRY) ? false : peace_minded(ptr);
	mtmp->mtraitor = false;

	switch (ptr->mlet) {
		case S_MIMIC:
			set_mimic_sym(mtmp);
			break;
		case S_SPIDER:
		case S_SNAKE:
			if (in_mklev)
				if (x && y)
					mkobj_at(0, x, y, true);
			if (hides_under(ptr) && OBJ_AT(x, y))
				mtmp->mundetected = true;
			break;
		case S_LIGHT:
		case S_ELEMENTAL:
			if (mndx == PM_STALKER || mndx == PM_BLACK_LIGHT) {
				mtmp->perminvis = true;
				mtmp->minvis = true;
			}
			break;
		case S_EEL:
			if (is_pool(x, y))
				mtmp->mundetected = true;
			break;
		case S_LEPRECHAUN:
			mtmp->msleeping = 1;
			break;
		case S_JABBERWOCK:
		case S_NYMPH:
			if (rn2(5) && !u.uhave.amulet) mtmp->msleeping = 1;
			if (mndx == PM_PIXIE) {
				/*  			    mtmp->perminvis = true;*/
				mtmp->minvis = true;
			}
			break;
		case S_ORC:
			if (Race_if(PM_ELF)) mtmp->mpeaceful = false;
			break;
		case S_UNICORN:
			if (is_unicorn(ptr) &&
			    sgn(u.ualign.type) == sgn(ptr->maligntyp))
				mtmp->mpeaceful = true;
			break;
		case S_BAT:
			if (Inhell && is_bat(ptr))
				mon_adjust_speed(mtmp, 2, NULL);
			break;
		case S_VAMPIRE:
			/* [DS] Star vampires are invisible until they feed */
			if (mndx == PM_STAR_VAMPIRE) {
				mtmp->perminvis = true;
				mtmp->minvis = true;
			}
			break;
	}
	if ((ct = emits_light(mtmp->data)) > 0)
		new_light_source(mtmp->mx, mtmp->my, ct,
				 LS_MONSTER, monst_to_any(mtmp));

	mitem = 0; /* extra inventory item for this monster */

	if (mndx == PM_VLAD_THE_IMPALER)
		mitem = CANDELABRUM_OF_INVOCATION;

	mtmp->cham = CHAM_ORDINARY;
	if ((mcham = pm_to_cham(mndx)) != CHAM_ORDINARY) {
		/* If you're protected with a ring, don't create
		 * any shape-changing chameleons -dgk
		 */
		if (Protection_from_shape_changers) {
			; // stuck in its natural form (CHAM_ORDINARY)
		} else {
			/* General shapechangers start out with random form
			 * (this explicitly picks something from the normal
			 * selection for current difficulty level rather
			 * than from among shapechanger's preferred forms).
			 * Vampires are the exception.
			 */

			struct permonst *tmpcham = rndmonst();

			mtmp->cham = mcham;

			if (is_vampshifter(mtmp)) {
				int chamidx = select_newcham_form(mtmp);
				if (chamidx != NON_PM)
					tmpcham = &mons[chamidx];
			}

			if (mtmp->cham != PM_VLAD_THE_IMPALER)
				mon_spec_poly(mtmp, tmpcham, 0L, false, false, false, false);
		}
	} else if (mndx == PM_WIZARD_OF_YENDOR) {
		mtmp->iswiz = true;
		context.no_of_wizards++;
		if (context.no_of_wizards == 1 && Is_earthlevel(&u.uz))
			mitem = SPE_DIG;
	} else if (mndx == PM_GHOST && !(mmflags & MM_NONAME)) {
			mtmp = christen_monst(mtmp, rndghostname());
	} else if (mndx == PM_NIGHTMARE) {
		struct obj *otmp;

		otmp = oname(mksobj(SKELETON_KEY, true, false),
			     artiname(ART_KEY_OF_LAW));
		if (otmp) {
			otmp->blessed = otmp->cursed = 0;
			mpickobj(mtmp, otmp);
		}
	} else if (mndx == PM_BEHOLDER) {
		struct obj *otmp;

		otmp = oname(mksobj(SKELETON_KEY, true, false),
			     artiname(ART_KEY_OF_NEUTRALITY));
		if (otmp) {
			otmp->blessed = otmp->cursed = 0;
			mpickobj(mtmp, otmp);
		}
	} else if (mndx == PM_VECNA) {
		struct obj *otmp;

		otmp = oname(mksobj(SKELETON_KEY, true, false),
			     artiname(ART_KEY_OF_CHAOS));
		if (otmp) {
			otmp->blessed = otmp->cursed = 0;
			mpickobj(mtmp, otmp);
		}
	} else if (mndx == PM_GYPSY) {
		/* KMH -- Gypsies are randomly generated; initialize them here */
		gypsy_init(mtmp);
	} else if (mndx == PM_CROESUS) {
		mitem = TWO_HANDED_SWORD;
	} else if (ptr->msound == MS_NEMESIS) {
		mitem = BELL_OF_OPENING;
	} else if (mndx == PM_PESTILENCE) {
		mitem = POT_SICKNESS;
	}
	if (mitem && allow_minvent) mongets(mtmp, mitem);

	if (in_mklev) {
		if (((is_ndemon(ptr)) ||
		     (mndx == PM_WUMPUS) ||
		     (mndx == PM_LONG_WORM) ||
		     (mndx == PM_GIANT_EEL)) &&
		    !u.uhave.amulet && rn2(5))
			mtmp->msleeping = 1;
	} else {
		if (byyou) {
			newsym(mtmp->mx, mtmp->my);
			set_apparxy(mtmp);
		}
	}
	if (is_dprince(ptr) && ptr->msound == MS_BRIBE) {
		mtmp->mpeaceful = mtmp->minvis = mtmp->perminvis = 1;
		mtmp->mavenge = 0;
		if (uwep && uwep->oartifact == ART_EXCALIBUR) {
			mtmp->mpeaceful = false;
			mtmp->mtame = 0;
		}
	}
	if (mndx == PM_LONG_WORM && (mtmp->wormno = get_wormno()) != 0) {
		/* we can now create worms with tails - 11/91 */
		initworm(mtmp, rn2(5));
		if (count_wsegs(mtmp)) place_worm_tail_randomly(mtmp, x, y);
	}
	set_malign(mtmp); /* having finished peaceful changes */
	if (anymon) {
		if ((ptr->geno & G_SGROUP) && rn2(2)) {
			m_initsgrp(mtmp, mtmp->mx, mtmp->my);
		} else if (ptr->geno & G_LGROUP) {
			if (rn2(3))
				m_initlgrp(mtmp, mtmp->mx, mtmp->my);
			else
				m_initsgrp(mtmp, mtmp->mx, mtmp->my);
		} else if (ptr->geno & G_VLGROUP) {
			if (rn2(3))
				m_initvlgrp(mtmp, mtmp->mx, mtmp->my);
			else if (rn2(3))
				m_initlgrp(mtmp, mtmp->mx, mtmp->my);
			else
				m_initsgrp(mtmp, mtmp->mx, mtmp->my);
		}
	}

	if (allow_minvent) {
		if (is_armed(ptr))
			m_initweap(mtmp); /* equip with weapons / armor */
		m_initinv(mtmp);	  /* add on a few special items incl. more armor */
		m_dowear(mtmp, true);
	} else {
		if (mtmp->minvent) discard_minvent(mtmp);
		mtmp->minvent = NULL; /* caller expects this */
		mtmp->minvent = NULL; /* caller expects this */
	}
	if ((ptr->mflags3 & M3_WAITMASK) && !(mmflags & MM_NOWAIT)) {
		if (ptr->mflags3 & M3_WAITFORU)
			mtmp->mstrategy |= STRAT_WAITFORU;
		if (ptr->mflags3 & M3_CLOSE)
			mtmp->mstrategy |= STRAT_CLOSE;
	}
	if (!in_mklev)
		newsym(mtmp->mx, mtmp->my); /* make sure the mon shows up */

	return mtmp;
}

int mbirth_limit(int mndx) {
	/* assert(MAXMONNO < 255); */
	return mndx == PM_NAZGUL ? 9 : mndx == PM_ERINYS ? 3 : MAXMONNO;
}

/* used for wand/scroll/spell of create monster */
/* returns true iff you know monsters have been created */

// usually null; used for confused reading
boolean create_critters(int cnt, struct permonst *mptr) {
	coord c;
	int x, y;
	struct monst *mon;
	boolean known = false;
	boolean ask = wizard;

	while (cnt--) {
		if (ask) {
			if (create_particular()) {
				known = true;
				continue;
			} else
				ask = false; /* ESC will shut off prompting */
		}
		x = u.ux, y = u.uy;
		/* if in water, try to encourage an aquatic monster
		   by finding and then specifying another wet location */
		if (!mptr && u.uinwater && enexto(&c, x, y, &mons[PM_GIANT_EEL]))
			x = c.x, y = c.y;

		mon = makemon(mptr, x, y, NO_MM_FLAGS);
		if (mon && canspotmon(mon)) known = true;
	}
	return known;
}

static boolean uncommon(int mndx) {
	if (mons[mndx].geno & (G_NOGEN | G_UNIQ)) return true;
	if (mvitals[mndx].mvflags & G_GONE) return true;
	if (Inhell)
		return mons[mndx].maligntyp > A_NEUTRAL;
	else
		return (mons[mndx].geno & G_HELL) != 0;
}

/*
 *	shift the probability of a monster's generation by
 *	comparing the dungeon alignment and monster alignment.
 *	return an integer in the range of 0-5.
 */
static int align_shift(struct permonst *ptr) {
	static long oldmoves = 0L; /* != 1, starting value of moves */
	static s_level *lev;
	int alshift;

	if (oldmoves != moves) {
		lev = Is_special(&u.uz);
		oldmoves = moves;
	}
	switch ((lev) ? lev->flags.align : dungeons[u.uz.dnum].flags.align) {
		default: /* just in case */
		case AM_NONE:
			alshift = 0;
			break;
		case AM_LAWFUL:
			alshift = (ptr->maligntyp + 20) / (2 * ALIGNWEIGHT);
			break;
		case AM_NEUTRAL:
			alshift = (20 - abs(ptr->maligntyp)) / ALIGNWEIGHT;
			break;
		case AM_CHAOTIC:
			alshift = (-(ptr->maligntyp - 20)) / (2 * ALIGNWEIGHT);
			break;
	}
	return alshift;
}

static struct {
	int choice_count;
	char mchoices[SPECIAL_PM]; /* value range is 0..127 */
} rndmonst_state = {-1, {0}};

/* select a random monster type */
struct permonst *rndmonst(void) {
	struct permonst *ptr;
	int mndx, ct;

	/* [Tom] this was locking up priest quest... who knows why? */
	/* fixed it! no 'W' class monsters with corpses! oops! */
	/*    if(u.uz.dnum == quest_dnum && (ptr = qt_montype())) return ptr; */
	if (u.uz.dnum == quest_dnum) {
		if ((ptr = qt_montype())) {
			return ptr;
		}
	}

	/* KMH -- February 2 is Groundhog Day! */
	if (Is_oracle_level(&u.uz) && (!!flags.groundhogday ^ !rn2(20)))
		return &mons[PM_WOODCHUCK];

	/*      if (u.uz.dnum == quest_dnum && rn2(7) && (ptr = qt_montype()) != 0)
		    return ptr; */

	if (rndmonst_state.choice_count < 0) { /* need to recalculate */
		int zlevel, minmlev, maxmlev;
		boolean elemlevel;
		boolean upper;
		rndmonst_state.choice_count = 0;
		/* look for first common monster */
		for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++) {
			if (!uncommon(mndx)) break;
			rndmonst_state.mchoices[mndx] = 0;
		}
		if (mndx == SPECIAL_PM) {
			/* evidently they've all been exterminated */
#ifdef DEBUG
			pline("rndmonst: no common mons!");
#endif
			return NULL;
		} /* else `mndx' now ready for use below */
		zlevel = level_difficulty();
		/* determine the level of the weakest monster to make. */
		minmlev = zlevel / 6;
		/* determine the level of the strongest monster to make. */
		maxmlev = (zlevel + u.ulevel + 1) >> 1;
		upper = Is_rogue_level(&u.uz);
		elemlevel = In_endgame(&u.uz) && !Is_astralevel(&u.uz);

		/*
		 *	Find out how many monsters exist in the range we have selected.
		 */

	loopback:
		/* (`mndx' initialized above) */
		for (; mndx < SPECIAL_PM; mndx++) {
			ptr = &mons[mndx];
			rndmonst_state.mchoices[mndx] = 0;
			if (tooweak(mndx, minmlev) || toostrong(mndx, maxmlev))
				continue;
			if (upper && !isupper((int)def_monsyms[(int)(ptr->mlet)])) continue;
			if (elemlevel && wrong_elem_type(ptr)) continue;
			if (uncommon(mndx)) continue;
			if (Inhell && (ptr->geno & G_NOHELL)) continue;
			ct = (int)(ptr->geno & G_FREQ) + align_shift(ptr);
			if (ct < 0 || ct > 127)
				panic("rndmonst: bad count [#%d: %d]", mndx, ct);
			rndmonst_state.choice_count += ct;
			rndmonst_state.mchoices[mndx] = (char)ct;
		}
		/*
		 *	    Possible modification:  if choice_count is "too low",
		 *	    expand minmlev..maxmlev range and try again.
		 */
	} /* choice_count+mchoices[] recalc */

	if (rndmonst_state.choice_count <= 0) {
		/* maybe no common mons left, or all are too weak or too strong */
#ifdef DEBUG
		Norep("rndmonst: choice_count=%d", rndmonst_state.choice_count);
#endif
		return NULL;
	}

	/*
	 *	Now, select a monster at random.
	 */
	ct = rnd(rndmonst_state.choice_count);
	for (mndx = LOW_PM; mndx < SPECIAL_PM; mndx++)
		if ((ct -= (int)rndmonst_state.mchoices[mndx]) <= 0) break;

	if (mndx == SPECIAL_PM || uncommon(mndx)) { /* shouldn't happen */
		impossible("rndmonst: bad `mndx' [#%d]", mndx);
		return NULL;
	}
	return &mons[mndx];

	/*
	 * [Tom] Your rival adventurers are a special case:
	 *       - They have varying levels, and thus can always appear
	 *       - But they need to be sorta rare or else they're a large
	 *         percentage of the dungeon inhabitants.
	 */
	if ((monsndx(ptr - 1) >= PM_ARCHEOLOGIST) &&
	    (monsndx(ptr - 1) <= PM_WIZARD) && (rn2(4))) goto loopback;
}

/* called when you change level (experience or dungeon depth) or when
   monster species can no longer be created (genocide or extinction) */
void reset_rndmonst(int mndx /* particular species that can no longer be created */) {
	/* cached selection info is out of date */
	if (mndx == NON_PM) {
		rndmonst_state.choice_count = -1; /* full recalc needed */
	} else if (mndx < SPECIAL_PM) {
		rndmonst_state.choice_count -= rndmonst_state.mchoices[mndx];
		rndmonst_state.mchoices[mndx] = 0;
	} /* note: safe to ignore extinction of unique monsters */
}

/*	The routine below is used to make one of the multiple types
 *	of a given monster class.  The second parameter specifies a
 *	special casing bit mask to allow the normal genesis
 *	masks to be deactivated.  Returns 0 if no monsters
 *	in that class can be made.
 */

struct permonst *mkclass(char class, int spc) {
	int first;

	first = pm_mkclass(class, spc);

	if (first == -1)
		return NULL;

	return &mons[first];
}

/* Called by mkclass() - returns the pm of the monster
 * Returns -1 (PM_PLAYERMON) if can't find monster of the class
 *
 * spc may have G_UNIQ and/or G_NOGEN set to allow monsters of
 * this type (otherwise they will be ignored). It may also have
 * MKC_ULIMIT set to place an upper limit on the difficulty of
 * the monster returned.
 */
int pm_mkclass(char class, int spc) {
	int first, last, num = 0;
	int maxmlev, mask = (G_NOGEN | G_UNIQ) & ~spc;

	maxmlev = level_difficulty() >> 1;
	if (class < 1 || class >= MAXMCLASSES) {
		impossible("mkclass called with bad class!");
		return -1;
	}
	/*	Assumption #1:	monsters of a given class are contiguous in the
	 *			mons[] array.
	 */
	for (first = LOW_PM; first < SPECIAL_PM; first++)
		if (mons[first].mlet == class) break;
	if (first == SPECIAL_PM) return -1;

	for (last = first;
	     last < SPECIAL_PM && mons[last].mlet == class;
	     last++)
		if (!(mvitals[last].mvflags & G_GONE) && !(mons[last].geno & mask) && !is_placeholder(&mons[last])) {
			/* consider it */
			if (spc & MKC_ULIMIT && toostrong(last, 4 * maxmlev)) break;
			if (num && toostrong(last, maxmlev) &&
			    mstrength(&mons[last]) != mstrength(&mons[last - 1]) && rn2(2)) break;
			num += mons[last].geno & G_FREQ;
		}

	if (!num) return -1;

	/*	Assumption #2:	monsters of a given class are presented in ascending
	 *			order of strength.
	 */
	for (num = rnd(num); num > 0; first++)
		if (!(mvitals[first].mvflags & G_GONE) && !(mons[first].geno & mask) && !is_placeholder(&mons[first])) {
			/* skew towards lower value monsters at lower exp. levels */
			num -= mons[first].geno & G_FREQ;
			if (num && adj_lev(&mons[first]) > (u.ulevel * 2)) {
				/* but not when multiple monsters are same level */
				if (mons[first].mlevel != mons[first + 1].mlevel)
					num--;
			}
		}
	first--; /* correct an off-by-one error */

	return first;
}

// adjust strength of monsters based on u.uz and u.ulevel
int adj_lev(struct permonst *ptr) {
	int tmp, tmp2;

	if (ptr == &mons[PM_WIZARD_OF_YENDOR]) {
		/* does not depend on other strengths, but does get stronger
		 * every time he is killed
		 */
		tmp = ptr->mlevel + mvitals[PM_WIZARD_OF_YENDOR].died;
		if (tmp > 49) tmp = 49;
		return tmp;
	}

	if ((tmp = ptr->mlevel) > 49) return 50; /* "special" demons/devils */
	tmp2 = (level_difficulty() - tmp);
	if (tmp2 < 0)
		tmp--; /* if mlevel > u.uz decrement tmp */
	else
		tmp += (tmp2 / 5); /* else increment 1 per five diff */

	tmp2 = (u.ulevel - ptr->mlevel); /* adjust vs. the player */
	if (tmp2 > 0) tmp += (tmp2 / 4); /* level as well */

	tmp2 = (3 * ((int)ptr->mlevel)) / 2;		  /* crude upper limit */
	if (tmp2 > 49) tmp2 = 49;			  /* hard upper limit */
	return (tmp > tmp2) ? tmp2 : (tmp > 0 ? tmp : 0); /* 0 lower limit */
}

// `mtmp' might "grow up" into a bigger version
struct permonst *grow_up(struct monst *mtmp, struct monst *victim) {
	int oldtype, newtype, max_increase, cur_increase,
		lev_limit, hp_threshold;
	struct permonst *ptr = mtmp->data;

	/* monster died after killing enemy but before calling this function */
	/* currently possible if killing a gas spore */
	if (mtmp->mhp <= 0)
		return NULL;

	if (mtmp->oldmonnm != monsndx(ptr))
		return ptr; /* No effect if polymorphed */

	/* note:  none of the monsters with special hit point calculations
	   have both little and big forms */
	oldtype = monsndx(ptr);
	newtype = little_to_big(oldtype);
	if (newtype == PM_PRIEST && mtmp->female) newtype = PM_PRIESTESS;

	/* growth limits differ depending on method of advancement */
	if (victim) { /* killed a monster */
		/*
		 * The HP threshold is the maximum number of hit points for the
		 * current level; once exceeded, a level will be gained.
		 * Possible bug: if somehow the hit points are already higher
		 * than that, monster will gain a level without any increase in HP.
		 */
		hp_threshold = mtmp->m_lev * 8; /* normal limit */
		if (!mtmp->m_lev)
			hp_threshold = 4;
		else if (is_golem(ptr)) /* strange creatures */
			hp_threshold = ((mtmp->mhpmax / 10) + 1) * 10 - 1;
		else if (is_home_elemental(ptr))
			hp_threshold *= 3;
		lev_limit = 3 * (int)ptr->mlevel / 2; /* same as adj_lev() */
		/* If they can grow up, be sure the level is high enough for that */
		if (oldtype != newtype && mons[newtype].mlevel > lev_limit)
			lev_limit = (int)mons[newtype].mlevel;
		/* number of hit points to gain; unlike for the player, we put
		   the limit at the bottom of the next level rather than the top */
		max_increase = rnd((int)victim->m_lev + 1);
		if (mtmp->mhpmax + max_increase > hp_threshold + 1)
			max_increase = max((hp_threshold + 1) - mtmp->mhpmax, 0);
		cur_increase = (max_increase > 1) ? rn2(max_increase) : 0;
	} else {
		/* a gain level potion or wraith corpse; always go up a level
		   unless already at maximum (49 is hard upper limit except
		   for demon lords, who start at 50 and can't go any higher) */
		max_increase = cur_increase = rnd(8) + rnd(2);
		hp_threshold = 0; /* smaller than `mhpmax + max_increase' */
		lev_limit = 50;	  /* recalc below */
	}

	mtmp->mhpmax += max_increase;
	mtmp->mhp += cur_increase;

	mtmp->m_enmax += max_increase;
	mtmp->m_en += cur_increase;

	if (mtmp->mhpmax <= hp_threshold)
		return ptr; /* doesn't gain a level */

	/* Allow to grow up even if grown up form would normally be
	 * out of range */
	if (lev_limit < mons[newtype].mlevel)
		lev_limit = mons[newtype].mlevel;

	if (is_mplayer(ptr))
		lev_limit = 30; /* same as player */
	else if (lev_limit < 5)
		lev_limit = 5; /* arbitrary */
	else if (lev_limit > 49)
		lev_limit = (ptr->mlevel > 49 ? 50 : 49);

	if ((int)++mtmp->m_lev >= mons[newtype].mlevel && newtype != oldtype) {
		ptr = &mons[newtype];
		if (mvitals[newtype].mvflags & G_GENOD) { /* allow G_EXTINCT */
			if (sensemon(mtmp))
				pline("As %s grows up into %s, %s %s!", mon_nam(mtmp),
				      an(ptr->mname), mhe(mtmp),
				      nonliving(ptr) ? "expires" : "dies");
			set_mon_data(mtmp, ptr, -1); /* keep mvitals[] accurate */
			mondied(mtmp);
			return NULL;
		}
		set_mon_data(mtmp, ptr, 1);   /* preserve intrinsics */
		newsym(mtmp->mx, mtmp->my);   /* color may change */
		lev_limit = (int)mtmp->m_lev; /* never undo increment */
	}
	/* sanity checks */
	if ((int)mtmp->m_lev > lev_limit) {
		mtmp->m_lev--; /* undo increment */
		/* HP might have been allowed to grow when it shouldn't */
		if (mtmp->mhpmax == hp_threshold + 1) mtmp->mhpmax--;
	}
	if (mtmp->mhpmax > 50 * 8) mtmp->mhpmax = 50 * 8; /* absolute limit */
	if (mtmp->mhp > mtmp->mhpmax) mtmp->mhp = mtmp->mhpmax;

	if (mtmp->m_enmax > 50 * 8) mtmp->m_enmax = 50 * 8; /* absolute limit */
	if (mtmp->m_en > mtmp->m_enmax) mtmp->m_en = mtmp->m_enmax;

	if (mtmp->oldmonnm != monsndx(ptr)) mtmp->oldmonnm = monsndx(ptr);
	return ptr;
}

int mongets(struct monst *mtmp, int otyp) {
	struct obj *otmp;
	int spe;

	if (!otyp) return 0;
	otmp = mksobj(otyp, true, false);
	if (otmp) {
		if (mtmp->data->mlet == S_DEMON) {
			/* demons never get blessed objects */
			if (otmp->blessed) curse(otmp);
		} else if (is_lminion(mtmp)) {
			/* lawful minions don't get cursed, bad, or rusting objects */
			otmp->cursed = false;
			if (otmp->spe < 0) otmp->spe = 0;
			otmp->oerodeproof = true;
		} else if (In_endgame(&u.uz) && is_mplayer(mtmp->data) && is_sword(otmp)) {
			otmp->spe = (3 + rn2(3));
		}
		/*
		 *      This seems to be covered under mkobj.c ...
		 *
		 *          * STEPHEN WHITE'S NEW CODE *
		 *
		 *          if ((otmp->otyp == ORCISH_DAGGER      && !rn2(8))  ||
		 *              (otmp->otyp == ORCISH_SPEAR       && !rn2(10)) ||
		 *              (otmp->otyp == ORCISH_SHORT_SWORD && !rn2(12)))
		 *                      otmp->opoisoned = true;
		 *
		 *           * It could be alread immune to rust ... *
		 *          if (!otmp->oerodeproof && !is_rustprone(otmp) &&
		 *              ((otmp->otyp >= SPEAR && otmp->otyp <= BULLWHIP) ||
		 *               (otmp->otyp >= ELVEN_LEATHER_HELM &&
		 *                otmp->otyp <= LEVITATION_BOOTS))) {
		 *                   if (!rn2(10-otmp->spe)) otmp->oerodeproof = true;
		 *              else if (!rn2(10+otmp->spe)) otmp->oeroded = rn2(3);
		 *          }
		 */

		if (otmp->otyp == CANDELABRUM_OF_INVOCATION) {
			otmp->spe = 0;
			otmp->age = 0L;
			otmp->lamplit = false;
			otmp->blessed = otmp->cursed = false;
		} else if (otmp->otyp == BELL_OF_OPENING) {
			otmp->blessed = otmp->cursed = false;
		} else if (otmp->otyp == SPE_BOOK_OF_THE_DEAD) {
			otmp->blessed = false;
			otmp->cursed = true;
		}

		/* leaders don't tolerate inferior quality battle gear */
		if (is_prince(mtmp->data)) {
			if (otmp->oclass == WEAPON_CLASS && otmp->spe < 1)
				otmp->spe = 1;
			else if (otmp->oclass == ARMOR_CLASS && otmp->spe < 0)
				otmp->spe = 0;
		}

		spe = otmp->spe;
		mpickobj(mtmp, otmp); /* might free otmp */
		return spe;
	} else
		return 0;
}

int golemhp(int type) {
	switch (type) {
		case PM_PAPER_GOLEM:
			return 36;
		case PM_STRAW_GOLEM:
			return 40;
		case PM_GARGOYLE:
			return 46;
		case PM_ROPE_GOLEM:
			return 60;
		case PM_LEATHER_GOLEM:
			return 80;
		case PM_GOLD_GOLEM:
			return 80;
		case PM_WOOD_GOLEM:
			return 100;
		case PM_FLESH_GOLEM:
			return 120;
		case PM_STATUE_GARGOYLE:
			return 140;
		case PM_CLAY_GOLEM:
			return 150;
		case PM_STONE_GOLEM:
			return 180;
		case PM_GLASS_GOLEM:
			return 140;
		case PM_IRON_GOLEM:
			return 240;
		case PM_RUBY_GOLEM:
			return 250;
		case PM_DIAMOND_GOLEM:
			return 270;
		case PM_SAPPHIRE_GOLEM:
			return 280;
		case PM_STEEL_GOLEM:
			return 290;
		case PM_CRYSTAL_GOLEM:
			return 300;
		case PM_FRANKENSTEIN_S_MONSTER:
			return 400;
		case PM_WAX_GOLEM:
			return 40;
		case PM_PLASTIC_GOLEM:
			return 60;
		default:
			return 0;
	}
}

/*
 *	Alignment vs. yours determines monster's attitude to you.
 *	( some "animal" types are co-aligned, but also hungry )
 */
boolean peace_minded(struct permonst *ptr) {
	aligntyp mal = ptr->maligntyp, ual = u.ualign.type;

	if (always_peaceful(ptr)) return true;

	if (always_hostile(ptr)) return false;
	if (ptr->msound == MS_LEADER || ptr->msound == MS_GUARDIAN)
		return true;
	if (ptr->msound == MS_NEMESIS) return false;

	if (is_elf(ptr) && is_elf(youmonst.data)) {
		/* Light and dark elves are always hostile to each other.
		 * Suggested by Dr. Eva R. Myers.
		 */
		if ((ual > A_NEUTRAL && mal < A_NEUTRAL) ||
		    (ual < A_NEUTRAL && mal > A_NEUTRAL))
			return false;
	}

	if (race_peaceful(ptr)) return true;
	if (race_hostile(ptr)) return false;

	/* the monster is hostile if its alignment is different from the
	 * player's */
	if (sgn(mal) != sgn(ual)) return false;

	/* Negative monster hostile to player with Amulet. */
	if (mal < A_NEUTRAL && u.uhave.amulet) return false;

	/* minions are hostile to players that have strayed at all */
	if (is_minion(ptr)) return u.ualign.record >= 0;

	/* Last case:  a chance of a co-aligned monster being
	 * hostile.  This chance is greater if the player has strayed
	 * (u.ualign.record negative) or the monster is not strongly aligned.
	 */
	return ((boolean)(!!rn2(16 + (u.ualign.record < -15 ? -15 : u.ualign.record)) &&
			  !!rn2(2 + abs(mal))));
}

/* Set malign to have the proper effect on player alignment if monster is
 * killed.  Negative numbers mean it's bad to kill this monster; positive
 * numbers mean it's good.  Since there are more hostile monsters than
 * peaceful monsters, the penalty for killing a peaceful monster should be
 * greater than the bonus for killing a hostile monster to maintain balance.
 * Rules:
 *   it's bad to kill peaceful monsters, potentially worse to kill always-
 *	peaceful monsters
 *   it's never bad to kill a hostile monster, although it may not be good
 */
void set_malign(struct monst *mtmp) {
	schar mal = mtmp->data->maligntyp;
	boolean coaligned;

	if (mtmp->ispriest || mtmp->isminion) {
		/* some monsters have individual alignments; check them */
		if (mtmp->ispriest)
			mal = EPRI(mtmp)->shralign;
		else if (mtmp->isminion)
			mal = EMIN(mtmp)->min_align;
		/* unless alignment is none, set mal to -5,0,5 */
		/* (see align.h for valid aligntyp values)     */
		if (mal != A_NONE)
			mal *= 5;
		/* make priests of Moloch hostile */
		if (mal == A_NONE) mtmp->mpeaceful = 0;
	}

	coaligned = (sgn(mal) == sgn(u.ualign.type));
	if (mtmp->data->msound == MS_LEADER) {
		mtmp->malign = -20;
	} else if (mal == A_NONE) {
		if (mtmp->mpeaceful)
			mtmp->malign = 0;
		else
			mtmp->malign = 20; /* really hostile */
	} else if (always_peaceful(mtmp->data)) {
		int absmal = abs(mal);
		if (mtmp->mpeaceful)
			mtmp->malign = -3 * max(5, absmal);
		else
			mtmp->malign = 3 * max(5, absmal); /* renegade */
	} else if (always_hostile(mtmp->data)) {
		int absmal = abs(mal);
		if (coaligned)
			mtmp->malign = 0;
		else
			mtmp->malign = max(5, absmal);
	} else if (coaligned) {
		int absmal = abs(mal);
		if (mtmp->mpeaceful)
			mtmp->malign = -3 * max(3, absmal);
		else /* renegade */
			mtmp->malign = max(3, absmal);
	} else /* not coaligned and therefore hostile */
		mtmp->malign = abs(mal);
}

static char syms[] = {
	MAXOCLASSES,
	MAXOCLASSES + 1,
	RING_CLASS,
	WAND_CLASS,
	WEAPON_CLASS,
	FOOD_CLASS,
	COIN_CLASS,
	SCROLL_CLASS,
	POTION_CLASS,
	ARMOR_CLASS,
	AMULET_CLASS,
	TOOL_CLASS,
	ROCK_CLASS,
	GEM_CLASS,
	SPBOOK_CLASS,
	S_MIMIC_DEF,
	S_MIMIC_DEF,
	S_MIMIC_DEF,
};

// KAA, modified by ERS
void set_mimic_sym(struct monst *mtmp) {
	int typ, roomno, rt;
	unsigned appear, ap_type;
	int s_sym;
	struct obj *otmp;
	int mx, my;

	if (!mtmp) return;
	mx = mtmp->mx;
	my = mtmp->my;
	typ = levl[mx][my].typ;
	/* only valid for INSIDE of room */
	roomno = levl[mx][my].roomno - ROOMOFFSET;
	if (roomno >= 0)
		rt = rooms[roomno].rtype;
#ifdef SPECIALIZATION
	else if (IS_ROOM(typ))
		rt = OROOM, roomno = 0;
#endif
	else
		rt = 0; /* roomno < 0 case for GCC_WARN */

	if (OBJ_AT(mx, my)) {
		ap_type = M_AP_OBJECT;
		appear = level.objects[mx][my]->otyp;
	} else if (IS_DOOR(typ) || IS_WALL(typ) ||
		   typ == SDOOR || typ == SCORR) {
		ap_type = M_AP_FURNITURE;
		/*
		 *  If there is a wall to the left that connects to this
		 *  location, then the mimic mimics a horizontal closed door.
		 *  This does not allow doors to be in corners of rooms.
		 *  Since rogue has no closed doors, mimics on the rogue level
		 *  imitate a wall, since that's what locking/wizard lock does.
		 */
		if (mx != 0 &&
		    (levl[mx - 1][my].typ == HWALL ||
		     levl[mx - 1][my].typ == TLCORNER ||
		     levl[mx - 1][my].typ == TRWALL ||
		     levl[mx - 1][my].typ == BLCORNER ||
		     levl[mx - 1][my].typ == TDWALL ||
		     levl[mx - 1][my].typ == CROSSWALL ||
		     levl[mx - 1][my].typ == TUWALL))
			appear = Is_rogue_level(&u.uz) ? S_hwall : S_hcdoor;
		else
			appear = Is_rogue_level(&u.uz) ? S_vwall : S_vcdoor;

		if (!mtmp->minvis || See_invisible)
			block_point(mx, my); /* vision */
	} else if (level.flags.is_maze_lev && rn2(2)) {
		ap_type = M_AP_OBJECT;
		appear = STATUE;
	} else if (roomno < 0) {
		ap_type = M_AP_OBJECT;
		appear = BOULDER;
		if (!mtmp->minvis || See_invisible)
			block_point(mx, my); /* vision */
	} else if (rt == ZOO || rt == VAULT) {
		ap_type = M_AP_OBJECT;
		appear = GOLD_PIECE;
	} else if (rt == DELPHI) {
		if (rn2(2)) {
			ap_type = M_AP_OBJECT;
			appear = STATUE;
		} else {
			ap_type = M_AP_FURNITURE;
			appear = S_fountain;
		}
	} else if (rt == TEMPLE) {
		ap_type = M_AP_FURNITURE;
		appear = S_altar;
		/*
		 * We won't bother with beehives, morgues, barracks, throne rooms
		 * since they shouldn't contain too many mimics anyway...
		 */
	} else if (rt >= SHOPBASE) {
		s_sym = get_shop_item(rt - SHOPBASE);
		if (s_sym < 0) {
			ap_type = M_AP_OBJECT;
			appear = -s_sym;
		} else {
			if (s_sym == RANDOM_CLASS)
				s_sym = syms[rn2((int)sizeof(syms) - 2) + 2];
			goto assign_sym;
		}
	} else {
		s_sym = syms[rn2((int)sizeof(syms))];
	assign_sym:
		if (s_sym >= MAXOCLASSES) {
			ap_type = M_AP_FURNITURE;
			appear = s_sym == MAXOCLASSES ? S_upstair : S_dnstair;
		} else if (s_sym == COIN_CLASS) {
			ap_type = M_AP_OBJECT;
			appear = GOLD_PIECE;
		} else {
			ap_type = M_AP_OBJECT;
			if (s_sym == S_MIMIC_DEF) {
				appear = STRANGE_OBJECT;
			} else {
				otmp = mkobj((char)s_sym, false);
				appear = otmp->otyp;
				/* make sure container contents are free'ed */
				if (Has_contents(otmp))
					delete_contents(otmp);
				obfree(otmp, NULL);
			}
		}
	}
	mtmp->m_ap_type = ap_type;
	mtmp->mappearance = appear;
}

// release a monster from a bag of tricks
void bagotricks(struct obj *bag) {
	if (!bag || bag->otyp != BAG_OF_TRICKS) {
		impossible("bad bag o' tricks");
	} else if (bag->spe < 1) {
		pline("%s", "Nothing happens.");
	} else {
		boolean gotone = false;
		int cnt = 1;

		consume_obj_charge(bag, true);

		if (!rn2(23)) cnt += rn1(7, 1);
		while (cnt-- > 0) {
			if (makemon(NULL, u.ux, u.uy, NO_MM_FLAGS))
				gotone = true;
		}
		if (gotone) makeknown(BAG_OF_TRICKS);
	}
}

/*makemon.c*/
