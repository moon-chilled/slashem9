/*	SCCS Id: @(#)objects.c	3.4	2002/07/31	*/
/* Copyright (c) Mike Threepoint, 1989.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifdef ONAMES_H
#define OBJECT(o_id, ...) o_id
#else
struct monst {
	struct monst *dummy;
}; /* lint: struct obj's union */

#include "config.h"
#include "obj.h"
#include "objclass.h"
#include "prop.h"
#include "skills.h"
#include "skills.h"
#include "color.h"

/* objects have symbols: ) [ = " ( % ! ? + / $ * ` 0 _ . */

/*
 *	Note:  OBJ() and BITS() macros are used to avoid exceeding argument
 *	limits imposed by some compilers.  The ctnr field of BITS currently
 *	does not map into struct objclass, and is ignored in the expansion.
 *	The 0 in the expansion corresponds to oc_pre_discovered, which is
 *	set at run-time during role-specific character initialization.
 */

#define BITS(nmkn, mrg, uskn, ctnr, mgc, chrg, uniq, nwsh, big, tuf, dir, sub, mtrl) \
	nmkn, mrg, uskn, 0, mgc, chrg, uniq, nwsh, big, tuf, dir, mtrl, sub /* SCO ODT 1.1 cpp fodder */
#define OBJECT(o_id, name, desc, bits, prp, sym, prob, dly, wt, cost, sdam, ldam, oc1, oc2, nut, color) \
	{ name, desc, 0, 0, NULL, bits, prp, sym, dly, color, prob, wt, cost, sdam, ldam, oc1, oc2, nut }
#define HARDGEM(n) (n >= 8)
#endif

#ifdef ONAMES_H
enum {
#else
struct objclass objects[] = {
#endif
	/* dummy object[0] -- description [2nd arg] *must* be NULL */
	OBJECT(STRANGE_OBJECT, "strange object", NULL, BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
	       0, ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),

/* weapons ... */
#define WEAPON(o_id, name, app, kn, mg, bi, prob, wt, cost, sdam, ldam, hitbon, typ, sub, metal, color) \
	OBJECT(                                                                                         \
		o_id, name, app, BITS(kn, mg, 1, 0, 0, 1, 0, 0, bi, 0, typ, sub, metal), 0,             \
		WEAPON_CLASS, prob, 0,                                                                  \
		wt, cost, sdam, ldam, hitbon, 0, wt, color)
#define PROJECTILE(o_id, name, app, kn, prob, wt, cost, sdam, ldam, hitbon, metal, sub, color) \
	OBJECT(                                                                                \
		o_id, name, app, BITS(kn, 1, 1, 0, 0, 1, 0, 0, 0, 0, PIERCE, sub, metal), 0,   \
		WEAPON_CLASS, prob, 0,                                                         \
		wt, cost, sdam, ldam, hitbon, WP_GENERIC, wt, color)
#define BOW(o_id, name, app, kn, bi, prob, wt, cost, hitbon, metal, sub, color)          \
	OBJECT(                                                                          \
		o_id, name, app, BITS(kn, 0, 1, 0, 0, 1, 0, 0, bi, 0, 0, sub, metal), 0, \
		WEAPON_CLASS, prob, 0,                                                   \
		wt, cost, 0, 0, hitbon, WP_GENERIC, wt, color)
#define BULLET(o_id, name, app, kn, prob, wt, cost, sdam, ldam, hitbon, ammotyp, typ, metal, sub, color) \
	OBJECT(                                                                                          \
		o_id, name, app, BITS(kn, 1, 1, 0, 0, 1, 0, 0, 0, 0, typ, sub, metal), 0,                \
		WEAPON_CLASS, prob, 0,                                                                   \
		wt, cost, sdam, ldam, hitbon, ammotyp, wt, color)
#define GUN(o_id, name, app, kn, bi, prob, wt, cost, range, rof, hitbon, ammotyp, metal, sub, color) \
	OBJECT(                                                                                      \
		o_id, name, app, BITS(kn, 0, 1, 0, 0, 1, 0, 0, bi, 0, 0, sub, metal), 0,             \
		WEAPON_CLASS, prob, 0,                                                               \
		wt, cost, range, rof, hitbon, ammotyp, wt, color)

/* Note: for weapons that don't do an even die of damage (ex. 2-7 or 3-18)
	 * the extra damage is added on in weapon.c, not here! */

#define P PIERCE
#define S SLASH
#define B WHACK
#define E EXPLOSION

	/* Daggers */
	WEAPON(ORCISH_DAGGER, "orcish dagger", "crude dagger",
	       0, 1, 0, 10, 10, 4, 3, 3, 2, P, P_DAGGER, IRON, CLR_BLACK),
	WEAPON(DAGGER, "dagger", NULL,
	       1, 1, 0, 25, 10, 4, 4, 3, 2, P, P_DAGGER, IRON, HI_METAL),
	WEAPON(ATHAME, "athame", NULL,
	       1, 1, 0, 0, 10, 4, 4, 3, 2, S, P_DAGGER, IRON, HI_METAL),
	WEAPON(SILVER_DAGGER, "silver dagger", NULL,
	       1, 1, 0, 2, 12, 40, 4, 3, 2, P, P_DAGGER, SILVER, HI_SILVER),
	/* STEPHEN WHITE'S NEW CODE */
	/* WAC silver dagger now pierces, to be same as other daggers
	allows it to be thrown without penalty as well*/
	WEAPON(ELVEN_DAGGER, "elven dagger", "runed dagger",
	       0, 1, 0, 8, 10, 4, 5, 3, 2, P, P_DAGGER, WOOD, HI_METAL),
	WEAPON(DARK_ELVEN_DAGGER, "dark elven dagger", "black runed dagger",
	       0, 1, 0, 0, 10, 4, 5, 3, 2, P, P_DAGGER, WOOD, CLR_BLACK),
	WEAPON(WOODEN_STAKE, "wooden stake", NULL,
	       1, 0, 0, 0, 20, 50, 6, 6, 0, P, P_DAGGER, WOOD, HI_WOOD),
	/* STEPHEN WHITE'S NEW CODE */
	/* Base for artifact (Stake of Van Helsing) */
	WEAPON(GREAT_DAGGER, "great dagger", NULL,
	       1, 0, 0, 0, 20, 500, 6, 7, 2, P, P_DAGGER, METAL, CLR_BLACK),
	/* STEPHEN WHITE'S NEW CODE */
	/* for necromancer artifact... */

	/* Knives */
	WEAPON(WORM_TOOTH, "worm tooth", NULL,
	       1, 0, 0, 0, 20, 2, 2, 2, 0, 0, P_KNIFE, 0, CLR_WHITE),
	WEAPON(KNIFE, "knife", NULL,
	       1, 1, 0, 15, 5, 4, 3, 2, 0, P | S, P_KNIFE, IRON, HI_METAL),
	WEAPON(STILETTO, "stiletto", NULL,
	       1, 1, 0, 5, 5, 4, 3, 2, 0, P | S, P_KNIFE, IRON, HI_METAL),
	WEAPON(SCALPEL, "scalpel", NULL,
	       1, 1, 0, 0, 5, 4, 3, 3, 2, S, P_KNIFE, METAL, HI_METAL),
	WEAPON(CRYSKNIFE, "crysknife", NULL,
	       1, 0, 0, 0, 20, 100, 20, 30, 3, P, P_KNIFE, MINERAL, CLR_WHITE),
	/* [Tom] increased crysknife damage from d10/d10 */
	/* to d20/d30 (otherwise, it's useless to make them...) */

	/* Axes */
	WEAPON(AXE, "axe", NULL,
	       1, 0, 0, 35, 60, 8, 6, 4, 0, S, P_AXE, IRON, HI_METAL),
	WEAPON(BATTLE_AXE, "battle-axe", "double-headed axe", /* "double-bitted" ? */
	       0, 0, 1, 10, 120, 40, 8, 6, 0, S, P_AXE, IRON, HI_METAL),

	/* Pick-axes */
	/* (also weptool pick-axe) */
	WEAPON(DWARVISH_MATTOCK, "dwarvish mattock", "broad pick",
	       0, 0, 1, 13, 120, 50, 12, 8, -1, B, P_PICK_AXE, IRON, HI_METAL),

	/* Short swords */
	WEAPON(ORCISH_SHORT_SWORD, "orcish short sword", "crude short sword",
	       0, 0, 0, 3, 30, 10, 5, 8, 0, P, P_SHORT_SWORD, IRON, CLR_BLACK),
	WEAPON(SHORT_SWORD, "short sword", NULL,
	       1, 0, 0, 8, 30, 10, 6, 8, 0, P, P_SHORT_SWORD, IRON, HI_METAL),
	WEAPON(SILVER_SHORT_SWORD, "silver short sword", NULL,
	       1, 0, 0, 2, 36, 50, 6, 8, 0, P, P_SHORT_SWORD, SILVER, HI_SILVER),
	/* STEPHEN WHITE'S NEW CODE */
	WEAPON(DWARVISH_SHORT_SWORD, "dwarvish short sword", "broad short sword",
	       0, 0, 0, 2, 30, 10, 7, 8, 0, P, P_SHORT_SWORD, IRON, HI_METAL),
	WEAPON(ELVEN_SHORT_SWORD, "elven short sword", "runed short sword",
	       0, 0, 0, 2, 30, 10, 8, 8, 0, P, P_SHORT_SWORD, WOOD, HI_METAL),
	WEAPON(DARK_ELVEN_SHORT_SWORD, "dark elven short sword", "black runed short sword",
	       0, 0, 0, 2, 30, 10, 8, 8, 0, P, P_SHORT_SWORD, WOOD, CLR_BLACK),

	/* Broadswords */
	WEAPON(BROADSWORD, "broadsword", NULL,
	       1, 0, 0, 20, 70, 10, 4, 6, 0, S, P_BROAD_SWORD, IRON, HI_METAL),
	/* +d4 small, +1 large */
	WEAPON(RUNESWORD, "runesword", "runed broadsword",
	       0, 0, 0, 0, 40, 300, 4, 6, 0, S, P_BROAD_SWORD, IRON, CLR_BLACK),
	/* +d4 small, +1 large; base for artifact (Stormbringer) */
	/* +5d2 +d8 from level drain */
	WEAPON(ELVEN_BROADSWORD, "elven broadsword", "runed broadsword",
	       0, 0, 0, 4, 70, 10, 6, 6, 0, S, P_BROAD_SWORD, WOOD, HI_METAL),
	/* +d4 small, +1 large */

	/* Long swords */
	WEAPON(LONG_SWORD, "long sword", NULL,
	       1, 0, 0, 50, 40, 15, 8, 12, 0, S, P_LONG_SWORD, IRON, HI_METAL),
	WEAPON(SILVER_LONG_SWORD, "silver long sword", NULL,
	       1, 0, 0, 2, 48, 75, 8, 12, 0, S, P_LONG_SWORD, SILVER, HI_SILVER),
	/* STEPHEN WHITE'S NEW CODE */
	WEAPON(KATANA, "katana", "samurai sword",
	       0, 0, 0, 4, 40, 80, 10, 12, 1, S, P_LONG_SWORD, IRON, HI_METAL),

	/* Two-handed swords */
	WEAPON(TWO_HANDED_SWORD, "two-handed sword", NULL,
	       1, 0, 1, 25, 150, 50, 12, 6, 0, S, P_TWO_HANDED_SWORD, IRON, HI_METAL),
	/* +2d6 large */
	WEAPON(TSURUGI, "tsurugi", "long samurai sword",
	       0, 0, 1, 0, 60, 500, 16, 8, 2, S, P_TWO_HANDED_SWORD, METAL, HI_METAL),
	/* +2d6 large; base for artifact (T of Muramasa) */

	/* Scimitars */
	WEAPON(SCIMITAR, "scimitar", "curved sword",
	       0, 0, 0, 15, 40, 15, 8, 8, 0, S, P_SCIMITAR, IRON, HI_METAL),

	/* Sabers */
	WEAPON(RAPIER, "rapier", NULL,
	       1, 0, 0, 0, 30, 40, 6, 8, 0, P, P_SABER, METAL, CLR_BLACK),
	/* STEPHEN WHITE'S NEW CODE */
	/* Base for artifact (Scalpel) */
	WEAPON(SILVER_SABER, "silver saber", NULL,
	       1, 0, 0, 27, 40, 75, 8, 8, 0, S, P_SABER, SILVER, HI_SILVER),

	/* Clubs */
	WEAPON(CLUB, "club", NULL,
	       1, 0, 0, 22, 30, 3, 6, 3, 0, B, P_CLUB, WOOD, HI_WOOD),
	WEAPON(AKLYS, "aklys", "thonged club",
	       0, 0, 0, 16, 15, 4, 6, 3, 0, B, P_CLUB, IRON, HI_METAL),
	WEAPON(BASEBALL_BAT, "baseball bat", NULL,
	       1, 0, 0, 0, 40, 50, 8, 6, 0, B, P_CLUB, WOOD, HI_WOOD),
	/* STEPHEN WHITE'S NEW CODE */
	/* Base for artifact */

	/* Paddles */
	/* Good to-hit and small damage, but low large damage */
	WEAPON(FLY_SWATTER, "fly swatter", NULL,
	       1, 0, 0, 2, 10, 3, 10, 2, 2, B, P_PADDLE, PLASTIC, CLR_GREEN),

	/* Maces */
	WEAPON(SILVER_MACE, "silver mace", NULL,
	       1, 0, 0, 12, 36, 65, 6, 6, 0, B, P_MACE, SILVER, HI_SILVER),
	/* STEPHEN WHITE'S NEW CODE */
	WEAPON(MACE, "mace", NULL,
	       1, 0, 0, 40, 30, 5, 6, 6, 0, B, P_MACE, IRON, HI_METAL),
	/* +1 small */

	/* Morning stars */
	WEAPON(MORNING_STAR, "morning star", NULL,
	       1, 0, 0, 12, 120, 10, 4, 6, 0, B, P_MORNING_STAR, IRON, HI_METAL),
	/* +d4 small, +1 large */

	/* Flails */
	WEAPON(FLAIL, "flail", NULL,
	       1, 0, 0, 30, 15, 4, 6, 4, 0, B, P_FLAIL, IRON, HI_METAL),
	/* +1 small, +1d4 large */

	/* Hammers */
	WEAPON(WAR_HAMMER, "war hammer", NULL,
	       1, 0, 0, 25, 50, 5, 4, 4, 0, B, P_HAMMER, IRON, HI_METAL),
	/* +1 small */
	WEAPON(HEAVY_HAMMER, "heavy hammer", NULL,
	       1, 0, 0, 0, 60, 500, 6, 6, 0, B, P_HAMMER, METAL, HI_METAL),
	/* STEPHEN WHITE'S NEW CODE */
	/* Base for artifact */

	/* Quarterstaves */
	WEAPON(QUARTERSTAFF, "quarterstaff", "staff",
	       0, 0, 1, 11, 40, 5, 6, 6, 0, B, P_QUARTERSTAFF, WOOD, HI_WOOD),

	/* Polearms */
	/* (also weptool fishing pole) */
	/* spear-type */
	WEAPON(PARTISAN, "partisan", "vulgar polearm",
	       0, 0, 1, 3, 80, 10, 6, 6, 0, P, P_POLEARMS, IRON, HI_METAL),
	/* +1 large */
	WEAPON(GLAIVE, "glaive", "single-edged polearm",
	       0, 0, 1, 4, 75, 6, 6, 10, 0, S, P_POLEARMS, IRON, HI_METAL),
	WEAPON(SPETUM, "spetum", "forked polearm",
	       0, 0, 1, 3, 50, 5, 6, 6, 0, P, P_POLEARMS, IRON, HI_METAL),
	/* +1 small, +d6 large */
	WEAPON(RANSEUR, "ranseur", "hilted polearm",
	       0, 0, 1, 3, 50, 6, 4, 4, 0, P, P_POLEARMS, IRON, HI_METAL),
	/* +d4 both */
	/* axe-type */
	WEAPON(BARDICHE, "bardiche", "long poleaxe",
	       0, 0, 1, 2, 120, 7, 4, 4, 0, S, P_POLEARMS, IRON, HI_METAL),
	/* +1d4 small, +2d4 large */
	WEAPON(VOULGE, "voulge", "pole cleaver",
	       0, 0, 1, 2, 125, 5, 4, 4, 0, S, P_POLEARMS, IRON, HI_METAL),
	/* +d4 both */
	WEAPON(HALBERD, "halberd", "angled poleaxe",
	       0, 0, 1, 4, 150, 10, 10, 6, 0, P | S, P_POLEARMS, IRON, HI_METAL),
	/* +1d6 large */
	/* curved/hooked */
	WEAPON(FAUCHARD, "fauchard", "pole sickle",
	       0, 0, 1, 3, 60, 5, 6, 8, 0, P | S, P_POLEARMS, IRON, HI_METAL),
	WEAPON(GUISARME, "guisarme", "pruning hook",
	       0, 0, 1, 3, 80, 5, 4, 8, 0, S, P_POLEARMS, IRON, HI_METAL),
	/* +1d4 small */
	WEAPON(BILL_GUISARME, "bill-guisarme", "hooked polearm",
	       0, 0, 1, 2, 120, 7, 4, 10, 0, P | S, P_POLEARMS, IRON, HI_METAL),
	/* +1d4 small */
	/* other */
	WEAPON(LUCERN_HAMMER, "lucern hammer", "pronged polearm",
	       0, 0, 1, 3, 150, 7, 4, 6, 0, B | P, P_POLEARMS, IRON, HI_METAL),
	/* +1d4 small */
	WEAPON(BEC_DE_CORBIN, "bec de corbin", "beaked polearm",
	       0, 0, 1, 2, 100, 8, 8, 6, 0, B | P, P_POLEARMS, IRON, HI_METAL),

	/* Spears */
	WEAPON(ORCISH_SPEAR, "orcish spear", "crude spear",
	       0, 1, 0, 13, 30, 3, 5, 8, 0, P, P_SPEAR, IRON, CLR_BLACK),
	WEAPON(SPEAR, "spear", NULL,
	       1, 1, 0, 50, 30, 3, 6, 8, 0, P, P_SPEAR, IRON, HI_METAL),
	WEAPON(SILVER_SPEAR, "silver spear", NULL,
	       1, 1, 0, 3, 36, 40, 6, 8, 0, P, P_SPEAR, SILVER, HI_SILVER),
	/* STEPHEN WHITE'S NEW CODE */
	WEAPON(ELVEN_SPEAR, "elven spear", "runed spear",
	       0, 1, 0, 10, 30, 3, 7, 8, 0, P, P_SPEAR, WOOD, HI_METAL),
	WEAPON(DWARVISH_SPEAR, "dwarvish spear", "stout spear",
	       0, 1, 0, 12, 35, 3, 8, 8, 0, P, P_SPEAR, IRON, HI_METAL),

	/* Javelins */
	WEAPON(JAVELIN, "javelin", "throwing spear",
	       0, 1, 0, 10, 20, 3, 6, 6, 0, P, P_JAVELIN, IRON, HI_METAL),

	/* Tridents */
	WEAPON(TRIDENT, "trident", NULL,
	       1, 0, 0, 8, 25, 5, 6, 4, 0, P, P_TRIDENT, IRON, HI_METAL),
	/* +1 small, +2d4 large */

	/* Lances */
	WEAPON(LANCE, "lance", NULL,
	       1, 0, 0, 1, 180, 10, 6, 8, 0, P, P_LANCE, IRON, HI_METAL),

	/* Bows (and arrows) */
	/* KMH, balance patch -- damage of launchers restored to d2 big and small */
	BOW(ORCISH_BOW, "orcish bow", "crude bow", 0, 1, 12, 30, 60, 0, WOOD, P_BOW, CLR_BLACK),
	BOW(BOW, "bow", NULL, 1, 1, 24, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
	BOW(ELVEN_BOW, "elven bow", "runed bow", 0, 1, 12, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
	BOW(DARK_ELVEN_BOW, "dark elven bow", "black runed bow", 0, 1, 0, 30, 60, 0, WOOD, P_BOW, CLR_BLACK),
	BOW(YUMI, "yumi", "long bow", 0, 1, 0, 30, 60, 0, WOOD, P_BOW, HI_WOOD),
	PROJECTILE(ORCISH_ARROW, "orcish arrow", "crude arrow",
		   0, 15, 1, 2, 5, 6, 0, IRON, -P_BOW, CLR_BLACK),
	PROJECTILE(ARROW, "arrow", NULL,
		   1, 50, 1, 2, 6, 6, 0, IRON, -P_BOW, HI_METAL),
	PROJECTILE(SILVER_ARROW, "silver arrow", NULL,
		   1, 15, 1, 5, 6, 6, 0, SILVER, -P_BOW, HI_SILVER),
	PROJECTILE(ELVEN_ARROW, "elven arrow", "runed arrow",
		   0, 25, 1, 2, 7, 6, 0, WOOD, -P_BOW, HI_METAL),
	PROJECTILE(DARK_ELVEN_ARROW, "dark elven arrow", "black runed arrow",
		   0, 0, 1, 2, 7, 6, 0, WOOD, -P_BOW, CLR_BLACK),
	PROJECTILE(YA, "ya", "bamboo arrow",
		   0, 10, 1, 4, 7, 7, 1, METAL, -P_BOW, HI_METAL),

	/* Slings */
	BOW(SLING, "sling", NULL, 1, 0, 40, 3, 20, 0, WOOD, P_SLING, HI_LEATHER),

	/* Firearms */
	GUN(PISTOL, "pistol", NULL, 1, 0, 0, 20, 100, 15, 0, 0, WP_BULLET, IRON, P_FIREARM, HI_METAL),
	GUN(SUBMACHINE_GUN, "submachine gun", NULL, 1, 0, 0, 25, 250, 10, 3, -1, WP_BULLET, IRON, P_FIREARM, HI_METAL),
	GUN(HEAVY_MACHINE_GUN, "heavy machine gun", NULL, 1, 1, 0, 500, 2000, 20, 8, -4, WP_BULLET, IRON, P_FIREARM, HI_METAL),
	GUN(RIFLE, "rifle", NULL, 1, 1, 0, 30, 150, 22, -1, 1, WP_BULLET, IRON, P_FIREARM, HI_METAL),
	GUN(ASSAULT_RIFLE, "assault rifle", NULL, 1, 0, 0, 40, 1000, 20, 5, -2, WP_BULLET, IRON, P_FIREARM, HI_METAL),
	GUN(SNIPER_RIFLE, "sniper rifle", NULL, 1, 1, 0, 50, 4000, 25, -3, 4, WP_BULLET, IRON, P_FIREARM, HI_METAL),
	GUN(SHOTGUN, "shotgun", NULL, 1, 0, 0, 35, 200, 3, -1, 3, WP_SHELL, IRON, P_FIREARM, HI_METAL),
	GUN(AUTO_SHOTGUN, "auto shotgun", NULL, 1, 1, 0, 60, 1500, 3, 2, 0, WP_SHELL, IRON, P_FIREARM, HI_METAL),
	GUN(ROCKET_LAUNCHER, "rocket launcher", NULL, 1, 1, 0, 750, 3500, 20, -5, -4, WP_ROCKET, IRON, P_FIREARM, HI_METAL),
	GUN(GRENADE_LAUNCHER, "grenade launcher", NULL, 1, 1, 0, 55, 1500, 6, -3, -3, WP_GRENADE, IRON, P_FIREARM, HI_METAL),
	BULLET(BULLET, "bullet", NULL,
	       1, 0, 1, 5, 20, 30, 0, WP_BULLET, P, IRON, -P_FIREARM, HI_METAL),
	BULLET(SILVER_BULLET, "silver bullet", NULL,
	       1, 0, 1, 15, 20, 30, 0, WP_BULLET, P, SILVER, -P_FIREARM, HI_SILVER),
	BULLET(SHOTGUN_SHELL, "shotgun shell", NULL,
	       1, 0, 1, 10, 30, 45, 0, WP_SHELL, P, IRON, -P_FIREARM, CLR_RED),
	BULLET(ROCKET, "rocket", NULL,
	       1, 0, 200, 450, 45, 60, 0, WP_ROCKET, P | E, IRON, -P_FIREARM, CLR_BLUE),
	BULLET(FRAG_GRENADE, "frag grenade", NULL,
	       1, 0, 25, 350, 0, 0, 0, WP_GRENADE, B | E, IRON, -P_FIREARM, CLR_GREEN),
	BULLET(GAS_GRENADE, "gas grenade", NULL,
	       1, 0, 25, 350, 0, 0, 0, WP_GRENADE, B | E, IRON, -P_FIREARM, CLR_ORANGE),
	BULLET(STICK_OF_DYNAMITE, "stick of dynamite", "red stick",
	       0, 0, 30, 150, 0, 0, 0, WP_GENERIC, B, PLASTIC, P_NONE, CLR_RED),

	/* Crossbows (and bolts) */
	/* Crossbow range is now independant of strength */
	GUN(CROSSBOW, "crossbow", NULL, 0, 1, 45, 50, 40, 12, -1, 0, WP_GENERIC, WOOD, P_CROSSBOW, HI_WOOD),
	PROJECTILE(CROSSBOW_BOLT, "crossbow bolt", NULL,
		   1, 45, 1, 2, 4, 6, 0, IRON, -P_CROSSBOW, HI_METAL),

	/* Darts */
	/* (also weptoool spoon) */
	WEAPON(DART, "dart", NULL,
	       1, 1, 0, 55, 1, 2, 3, 2, 0, P, -P_DART, IRON, HI_METAL),

	/* Shurikens */
	WEAPON(SHURIKEN, "shuriken", "throwing star",
	       0, 1, 0, 35, 1, 5, 8, 6, 2, P, -P_SHURIKEN, IRON, HI_METAL),

	/* Boomerangs */
	WEAPON(BOOMERANG, "boomerang", NULL,
	       1, 1, 0, 15, 5, 20, 9, 9, 0, 0, -P_BOOMERANG, WOOD, HI_WOOD),

	/* Whips */
	WEAPON(BULLWHIP, "bullwhip", NULL,
	       1, 0, 0, 2, 20, 4, 2, 1, 0, 0, P_WHIP, LEATHER, CLR_BROWN),
	WEAPON(RUBBER_HOSE, "rubber hose", NULL,
	       1, 0, 0, 0, 20, 3, 4, 3, 0, B, P_WHIP, PLASTIC, CLR_BROWN),

/* With shuffled appearances... */
#undef P
#undef S
#undef B
#undef E

#undef WEAPON
#undef PROJECTILE
#undef BOW
#undef BULLET
#undef GUN

/* armor ... */
/* IRON denotes ferrous metals, including steel.
	 * Only IRON weapons and armor can rust.
	 * Only COPPER (including brass) corrodes.
	 * Some creatures are vulnerable to SILVER.
	 */
#define ARMOR(o_id, name, desc, kn, mgc, blk, power, prob, delay, wt, cost, ac, can, sub, metal, c) \
	OBJECT(                                                                                     \
		o_id, name, desc, BITS(kn, 0, 1, 0, mgc, 1, 0, 0, blk, 0, 0, sub, metal), power,    \
		ARMOR_CLASS, prob, delay, wt, cost,                                                 \
		0, 0, 10 - ac, can, wt, c)
#define CLOAK(o_id, name, desc, kn, mgc, power, prob, delay, wt, cost, ac, can, metal, c) \
	ARMOR(o_id, name, desc, kn, mgc, 0, power, prob, delay, wt, cost, ac, can, ARM_CLOAK, metal, c)
#define HELM(o_id, name, desc, kn, mgc, power, prob, delay, wt, cost, ac, can, metal, c) \
	ARMOR(o_id, name, desc, kn, mgc, 0, power, prob, delay, wt, cost, ac, can, ARM_HELM, metal, c)
#define GLOVES(o_id, name, desc, kn, mgc, power, prob, delay, wt, cost, ac, can, metal, c) \
	ARMOR(o_id, name, desc, kn, mgc, 0, power, prob, delay, wt, cost, ac, can, ARM_GLOVES, metal, c)
#define SHIELD(o_id, name, desc, kn, mgc, blk, power, prob, delay, wt, cost, ac, can, metal, c) \
	ARMOR(o_id, name, desc, kn, mgc, blk, power, prob, delay, wt, cost, ac, can, ARM_SHIELD, metal, c)
#define BOOTS(o_id, name, desc, kn, mgc, power, prob, delay, wt, cost, ac, can, metal, c) \
	ARMOR(o_id, name, desc, kn, mgc, 0, power, prob, delay, wt, cost, ac, can, ARM_BOOTS, metal, c)

	/* Shirts */
	ARMOR(HAWAIIAN_SHIRT, "Hawaiian shirt", NULL,
	      1, 0, 0, 0, 8, 0, 5, 3, 10, 0, ARM_SHIRT, CLOTH, CLR_MAGENTA),
	ARMOR(T_SHIRT, "T-shirt", NULL,
	      1, 0, 0, 0, 2, 0, 5, 2, 10, 0, ARM_SHIRT, CLOTH, CLR_WHITE),

	/* Suits of armor */
	ARMOR(PLATE_MAIL, "plate mail", NULL,
	      1, 0, 1, 0, 40, 5, 450, 600, 3, 2, ARM_SUIT, IRON, HI_METAL),
	ARMOR(CRYSTAL_PLATE_MAIL, "crystal plate mail", NULL,
	      1, 0, 1, 0, 10, 5, 450, 820, 3, 2, ARM_SUIT, GLASS, CLR_WHITE),
	ARMOR(BRONZE_PLATE_MAIL, "bronze plate mail", NULL,
	      1, 0, 1, 0, 25, 5, 450, 400, 4, 0, ARM_SUIT, COPPER, HI_COPPER),
	ARMOR(SPLINT_MAIL, "splint mail", NULL,
	      1, 0, 1, 0, 65, 5, 400, 80, 4, 1, ARM_SUIT, IRON, HI_METAL),
	ARMOR(BANDED_MAIL, "banded mail", NULL,
	      1, 0, 1, 0, 75, 5, 350, 90, 4, 0, ARM_SUIT, IRON, HI_METAL),
	ARMOR(DWARVISH_MITHRIL_COAT, "dwarvish mithril-coat", NULL,
	      1, 0, 0, 0, 10, 1, 150, 240, 4, 3, ARM_SUIT, MITHRIL, HI_METAL),
	ARMOR(DARK_ELVEN_MITHRIL_COAT, "dark elven mithril-coat", NULL,
	      1, 0, 0, 0, 0, 1, 150, 240, 4, 3, ARM_SUIT, MITHRIL, CLR_BLACK),
	ARMOR(ELVEN_MITHRIL_COAT, "elven mithril-coat", NULL,
	      1, 0, 0, 0, 15, 1, 150, 240, 5, 3, ARM_SUIT, MITHRIL, HI_METAL),
	ARMOR(CHAIN_MAIL, "chain mail", NULL,
	      1, 0, 0, 0, 70, 5, 300, 75, 5, 1, ARM_SUIT, IRON, HI_METAL),
	ARMOR(ORCISH_CHAIN_MAIL, "orcish chain mail", "crude chain mail",
	      0, 0, 0, 0, 20, 5, 300, 75, 6, 1, ARM_SUIT, IRON, CLR_BLACK),
	ARMOR(SCALE_MAIL, "scale mail", NULL,
	      1, 0, 0, 0, 70, 5, 250, 45, 6, 0, ARM_SUIT, IRON, HI_METAL),
	ARMOR(STUDDED_LEATHER_ARMOR, "studded leather armor", NULL,
	      1, 0, 0, 0, 70, 3, 200, 15, 7, 1, ARM_SUIT, LEATHER, HI_LEATHER),
	ARMOR(RING_MAIL, "ring mail", NULL,
	      1, 0, 0, 0, 70, 5, 250, 100, 7, 0, ARM_SUIT, IRON, HI_METAL),
	ARMOR(ORCISH_RING_MAIL, "orcish ring mail", "crude ring mail",
	      0, 0, 0, 0, 20, 5, 250, 80, 8, 1, ARM_SUIT, IRON, CLR_BLACK),
	ARMOR(LEATHER_ARMOR, "leather armor", NULL,
	      1, 0, 0, 0, 75, 3, 150, 5, 8, 0, ARM_SUIT, LEATHER, HI_LEATHER),
	ARMOR(LEATHER_JACKET, "leather jacket", NULL,
	      1, 0, 0, 0, 11, 0, 30, 10, 9, 0, ARM_SUIT, LEATHER, CLR_BLACK),

	/* Robes */
	/* STEPHEN WHITE'S NEW CODE */
	ARMOR(ROBE, "robe", "red robe",
	      0, 0, 0, 0, 1, 1, 40, 25, 9, 0, ARM_SUIT, LEATHER, CLR_RED),
	ARMOR(ROBE_OF_PROTECTION, "robe of protection", "blue robe",
	      0, 1, 0, PROTECTION, 1, 1, 40, 50, 5, 0, ARM_SUIT, LEATHER, CLR_BLUE),
	ARMOR(ROBE_OF_POWER, "robe of power", "orange robe",
	      0, 1, 0, 0, 0, 1, 40, 50, 9, 0, ARM_SUIT, LEATHER, CLR_ORANGE),
	ARMOR(ROBE_OF_WEAKNESS, "robe of weakness", "green robe",
	      0, 1, 0, 0, 1, 1, 40, 50, 9, 0, ARM_SUIT, LEATHER, CLR_GREEN),

/*
	 * Dragon suits
	 * There is code in polyself.c that assumes (1) and (2).
	 * There is code in obj.h, objnam.c, mon.c, read.c that assumes (2).
	 *
	 *	(1) The dragon scale mails and the dragon scales are together.
	 *	(2) That the order of the dragon scale mail and dragon scales is the
	 *	    the same defined in monst.c.
	 */
#define DRGN_ARMR(o_id, name, mgc, power, cost, ac, color) \
	ARMOR(o_id, name, NULL, 1, mgc, 1, power, 0, 5, 50, cost, ac, 0, ARM_SUIT, DRAGON_HIDE, color)
	/* 3.4.1: dragon scale mail reclassified as "magic" since magic is
	   needed to create them */
	DRGN_ARMR(GRAY_DRAGON_SCALE_MAIL, "gray dragon scale mail", 1, ANTIMAGIC, 1200, 1, CLR_GRAY),
	DRGN_ARMR(SILVER_DRAGON_SCALE_MAIL, "silver dragon scale mail", 1, REFLECTING, 1200, 1, SILVER),
	DRGN_ARMR(SHIMMERING_DRAGON_SCALE_MAIL, "shimmering dragon scale mail", 1, DISPLACED, 1200, 1, CLR_CYAN),
	DRGN_ARMR(DEEP_DRAGON_SCALE_MAIL, "deep dragon scale mail", 1, DRAIN_RES, 1200, 1, CLR_MAGENTA),
	DRGN_ARMR(RED_DRAGON_SCALE_MAIL, "red dragon scale mail", 1, FIRE_RES, 900, 1, CLR_RED),
	DRGN_ARMR(WHITE_DRAGON_SCALE_MAIL, "white dragon scale mail", 1, COLD_RES, 900, 1, CLR_WHITE),
	DRGN_ARMR(ORANGE_DRAGON_SCALE_MAIL, "orange dragon scale mail", 1, SLEEP_RES, 900, 1, CLR_ORANGE),
	DRGN_ARMR(BLACK_DRAGON_SCALE_MAIL, "black dragon scale mail", 1, DISINT_RES, 1200, 1, CLR_BLACK),
	DRGN_ARMR(BLUE_DRAGON_SCALE_MAIL, "blue dragon scale mail", 1, SHOCK_RES, 900, 1, CLR_BLUE),
	DRGN_ARMR(GREEN_DRAGON_SCALE_MAIL, "green dragon scale mail", 1, POISON_RES, 900, 1, CLR_GREEN),
	DRGN_ARMR(YELLOW_DRAGON_SCALE_MAIL, "yellow dragon scale mail", 1, ACID_RES, 900, 1, CLR_YELLOW),

	/* For now, only dragons leave these. */
	/* 3.4.1: dragon scales left classified as "non-magic"; they confer
	   magical properties but are produced "naturally" */
	DRGN_ARMR(GRAY_DRAGON_SCALES, "gray dragon scales", 0, ANTIMAGIC, 700, 7, CLR_GRAY),
	DRGN_ARMR(SILVER_DRAGON_SCALES, "silver dragon scales", 0, REFLECTING, 700, 7, SILVER),
	DRGN_ARMR(SHIMMERING_DRAGON_SCALES, "shimmering dragon scales", 0, DISPLACED, 700, 7, CLR_CYAN),
	DRGN_ARMR(DEEP_DRAGON_SCALES, "deep dragon scales", 0, DRAIN_RES, 500, 7, CLR_MAGENTA),
	DRGN_ARMR(RED_DRAGON_SCALES, "red dragon scales", 0, FIRE_RES, 500, 7, CLR_RED),
	DRGN_ARMR(WHITE_DRAGON_SCALES, "white dragon scales", 0, COLD_RES, 500, 7, CLR_WHITE),
	DRGN_ARMR(ORANGE_DRAGON_SCALES, "orange dragon scales", 0, SLEEP_RES, 500, 7, CLR_ORANGE),
	DRGN_ARMR(BLACK_DRAGON_SCALES, "black dragon scales", 0, DISINT_RES, 700, 7, CLR_BLACK),
	DRGN_ARMR(BLUE_DRAGON_SCALES, "blue dragon scales", 0, SHOCK_RES, 500, 7, CLR_BLUE),
	DRGN_ARMR(GREEN_DRAGON_SCALES, "green dragon scales", 0, POISON_RES, 500, 7, CLR_GREEN),
	DRGN_ARMR(YELLOW_DRAGON_SCALES, "yellow dragon scales", 0, ACID_RES, 500, 7, CLR_YELLOW),
#undef DRGN_ARMR

	/* Cloaks */
	/*  'cope' is not a spelling mistake... leave it be */
	CLOAK(MUMMY_WRAPPING, "mummy wrapping", NULL,
	      1, 0, 0, 0, 0, 3, 2, 10, 1, CLOTH, CLR_GRAY),
	CLOAK(ORCISH_CLOAK, "orcish cloak", "coarse mantelet",
	      0, 0, 0, 8, 0, 10, 40, 10, 2, CLOTH, CLR_BLACK),
	CLOAK(DWARVISH_CLOAK, "dwarvish cloak", "hooded cloak",
	      0, 0, 0, 8, 0, 10, 50, 10, 2, CLOTH, HI_CLOTH),
	CLOAK(OILSKIN_CLOAK, "oilskin cloak", "slippery cloak",
	      0, 0, 0, 8, 0, 10, 50, 9, 3, CLOTH, HI_CLOTH),
	CLOAK(ELVEN_CLOAK, "elven cloak", "faded pall",
	      0, 1, STEALTH, 8, 0, 10, 60, 9, 3, CLOTH, CLR_BLACK),
	CLOAK(LAB_COAT, "lab coat", "white coat",
	      0, 1, POISON_RES, 10, 0, 10, 60, 9, 3, CLOTH, CLR_WHITE),
	CLOAK(LEATHER_CLOAK, "leather cloak", NULL,
	      1, 0, 0, 8, 0, 15, 40, 9, 1, LEATHER, CLR_BROWN),
#if 0
	CLOAK(ROBE, "robe", NULL,
	      1, 1,	0,	    3, 0, 15, 50,  8, 3, CLOTH, CLR_RED),
	CLOAK(ALCHEMY_SMOCK, "alchemy smock", "apron",
	      0, 1,	POISON_RES, 9, 0, 10, 50,  9, 1, CLOTH, CLR_WHITE),
#endif
	/* With shuffled appearances... */
	CLOAK(CLOAK_OF_PROTECTION, "cloak of protection", "tattered cape",
	      0, 1, PROTECTION, 9, 0, 10, 50, 7, 3, CLOTH, HI_CLOTH),
	CLOAK(POISONOUS_CLOAK, "poisonous cloak", "dirty rag",
	      0, 1, 0, 5, 0, 10, 40, 10, 3, CLOTH, CLR_GREEN),
	CLOAK(CLOAK_OF_INVISIBILITY, "cloak of invisibility", "opera cloak",
	      0, 1, INVIS, 10, 0, 10, 60, 9, 2, CLOTH, CLR_BRIGHT_MAGENTA),
	CLOAK(CLOAK_OF_MAGIC_RESISTANCE, "cloak of magic resistance", "ornamental cope",
	      0, 1, ANTIMAGIC, 2, 0, 10, 60, 9, 3, CLOTH, CLR_WHITE),
	CLOAK(CLOAK_OF_DISPLACEMENT, "cloak of displacement", "piece of cloth",
	      0, 1, DISPLACED, 10, 0, 10, 50, 9, 2, CLOTH, HI_CLOTH),
	/* Helmets */
	HELM(ELVEN_LEATHER_HELM, "elven leather helm", "leather hat",
	     0, 0, 0, 6, 1, 3, 8, 9, 0, LEATHER, HI_LEATHER),
	HELM(ORCISH_HELM, "orcish helm", "iron skull cap",
	     0, 0, 0, 6, 1, 30, 10, 9, 0, IRON, CLR_BLACK),
	HELM(DWARVISH_IRON_HELM, "dwarvish iron helm", "hard hat",
	     0, 0, 0, 6, 1, 40, 20, 8, 0, IRON, HI_METAL),
	HELM(FEDORA, "fedora", NULL,
	     1, 0, 0, 0, 0, 3, 1, 10, 0, CLOTH, CLR_BROWN),
	HELM(CORNUTHAUM, "cornuthaum", "conical hat",
	     0, 1, CLAIRVOYANT, 3, 1, 4, 80, 10, 2, CLOTH, CLR_BLUE),
	HELM(DUNCE_CAP, "dunce cap", "conical hat",
	     0, 1, 0, 3, 1, 4, 1, 10, 0, CLOTH, CLR_BLUE),
	HELM(DENTED_POT, "dented pot", NULL,
	     1, 0, 0, 2, 0, 10, 8, 9, 0, IRON, CLR_BLACK),
	/* ...with shuffled appearances */
	HELM(HELMET, "helmet", "plumed helmet",
	     0, 0, 0, 10, 1, 30, 10, 9, 0, IRON, HI_METAL),
	HELM(HELM_OF_BRILLIANCE, "helm of brilliance", "etched helmet",
	     0, 1, 0, 6, 1, 50, 50, 9, 0, IRON, CLR_GREEN),
	HELM(HELM_OF_OPPOSITE_ALIGNMENT, "helm of opposite alignment", "crested helmet",
	     0, 1, 0, 6, 1, 50, 50, 9, 0, IRON, HI_METAL),
	HELM(HELM_OF_TELEPATHY, "helm of telepathy", "visored helmet",
	     0, 1, TELEPAT, 2, 1, 50, 50, 9, 0, IRON, HI_METAL),

	/* Gloves */
	/* these have their color but not material shuffled, so the IRON must stay
	 * CLR_BROWN (== HI_LEATHER)
	 */
	GLOVES(LEATHER_GLOVES, "leather gloves", "old gloves",
	       0, 0, 0, 16, 1, 10, 8, 9, 0, LEATHER, HI_LEATHER),
	GLOVES(GAUNTLETS_OF_FUMBLING, "gauntlets of fumbling", "padded gloves",
	       0, 1, FUMBLING, 8, 1, 10, 50, 9, 0, LEATHER, HI_LEATHER),
	GLOVES(GAUNTLETS_OF_POWER, "gauntlets of power", "riding gloves",
	       0, 1, 0, 8, 1, 30, 50, 9, 0, IRON, CLR_BROWN),
	GLOVES(GAUNTLETS_OF_SWIMMING, "gauntlets of swimming", "black gloves",
	       0, 1, SWIMMING, 8, 1, 10, 50, 9, 0, LEATHER, HI_LEATHER),
	GLOVES(GAUNTLETS_OF_DEXTERITY, "gauntlets of dexterity", "fencing gloves",
	       0, 1, 0, 8, 1, 10, 50, 9, 0, LEATHER, HI_LEATHER),

	/* Shields */
	SHIELD(SMALL_SHIELD, "small shield", NULL,
	       1, 0, 0, 0, 6, 0, 30, 3, 9, 0, WOOD, HI_WOOD),
	/* Elven ... orcish shields can't be differentiated by feel */
	SHIELD(ELVEN_SHIELD, "elven shield", "blue and green shield",
	       0, 0, 0, 0, 2, 0, 50, 7, 8, 0, WOOD, CLR_GREEN),
	SHIELD(URUK_HAI_SHIELD, "Uruk-hai shield", "white-handed shield",
	       0, 0, 0, 0, 2, 0, 50, 7, 9, 0, IRON, HI_METAL),
	SHIELD(ORCISH_SHIELD, "orcish shield", "red-eyed shield",
	       0, 0, 0, 0, 2, 0, 50, 7, 9, 0, IRON, CLR_RED),
	SHIELD(LARGE_SHIELD, "large shield", NULL,
	       1, 0, 1, 0, 7, 0, 100, 10, 8, 0, IRON, HI_METAL),
	SHIELD(DWARVISH_ROUNDSHIELD, "dwarvish roundshield", "large round shield",
	       0, 0, 0, 0, 4, 0, 100, 10, 8, 0, IRON, HI_METAL),
	SHIELD(SHIELD_OF_REFLECTION, "shield of reflection", "polished silver shield",
	       0, 1, 0, REFLECTING, 3, 0, 50, 50, 8, 0, SILVER, HI_SILVER),

	/* Boots */
	BOOTS(LOW_BOOTS, "low boots", "walking shoes",
	      0, 0, 0, 25, 2, 10, 8, 9, 0, LEATHER, HI_LEATHER),
	BOOTS(IRON_SHOES, "iron shoes", "hard shoes",
	      0, 0, 0, 7, 2, 50, 16, 8, 0, IRON, HI_METAL),
	BOOTS(HIGH_BOOTS, "high boots", "jackboots",
	      0, 0, 0, 15, 2, 20, 12, 8, 0, LEATHER, HI_LEATHER),
	/* ...with shuffled appearances */
	BOOTS(SPEED_BOOTS, "speed boots", "combat boots",
	      0, 1, FAST, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER),
	/* With shuffled appearances... */
	BOOTS(WATER_WALKING_BOOTS, "water walking boots", "jungle boots",
	      0, 1, WWALKING, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER),
	BOOTS(JUMPING_BOOTS, "jumping boots", "hiking boots",
	      0, 1, JUMPING, 12, 2, 20, 50, 9, 0, LEATHER, HI_LEATHER),
	BOOTS(ELVEN_BOOTS, "elven boots", "mud boots",
	      0, 1, STEALTH, 12, 2, 15, 8, 9, 0, LEATHER, HI_LEATHER),
	BOOTS(KICKING_BOOTS, "kicking boots", "steel boots",
	      0, 1, 0, 12, 2, 15, 8, 9, 0, IRON, CLR_BROWN),
	BOOTS(FUMBLE_BOOTS, "fumble boots", "riding boots",
	      0, 1, FUMBLING, 12, 2, 20, 30, 9, 0, LEATHER, HI_LEATHER),
	BOOTS(LEVITATION_BOOTS, "levitation boots", "snow boots",
	      0, 1, LEVITATION, 12, 2, 15, 30, 9, 0, LEATHER, HI_LEATHER),

#undef HELM
#undef CLOAK
#undef SHIELD
#undef GLOVES
#undef BOOTS
#undef ARMOR

/* rings ... */
/* [Tom] looks like there are no probs to change... */
#define RING(o_id, name, power, stone, cost, mgc, spec, mohs, metal, color)         \
	OBJECT(RIN_##o_id, name, stone,                                             \
	       BITS(0, 0, spec, 0, mgc, spec, 0, 0, 0, HARDGEM(mohs), 0, 0, metal), \
	       power, RING_CLASS, 0, 0, 3, cost, 0, 0, 0, 0, 15, color)
	RING(ADORNMENT, "adornment", ADORNED, "wooden", 100, 1, 1, 2, WOOD, HI_WOOD),
	RING(HUNGER, "hunger", HUNGER, "topaz", 100, 1, 0, 8, GEMSTONE, CLR_CYAN),
	RING(MOOD, "mood", 0, "ridged", 100, 1, 0, 8, IRON, HI_METAL),
	RING(PROTECTION, "protection", PROTECTION, "black onyx", 100, 1, 1, 7, MINERAL, CLR_BLACK),
	RING(PROTECTION_FROM_SHAPE_CHANGERS, "protection from shape changers", PROT_FROM_SHAPE_CHANGERS, "shiny",
	     100, 1, 0, 5, IRON, CLR_BRIGHT_CYAN),
	RING(SLEEPING, "sleeping", SLEEPING, "wedding", 100, 1, 0, 7, GEMSTONE, CLR_WHITE),
	RING(STEALTH, "stealth", STEALTH, "jade", 100, 1, 0, 6, GEMSTONE, CLR_GREEN),
	RING(SUSTAIN_ABILITY, "sustain ability", FIXED_ABIL, "bronze",
	     100, 1, 0, 4, COPPER, HI_COPPER),
	RING(WARNING, "warning", WARNING, "diamond", 100, 1, 0, 10, GEMSTONE, CLR_WHITE),
	RING(AGGRAVATE_MONSTER, "aggravate monster", AGGRAVATE_MONSTER, "sapphire",
	     150, 1, 0, 9, GEMSTONE, CLR_BLUE),
	RING(COLD_RESISTANCE, "cold resistance", COLD_RES, "brass", 150, 1, 0, 4, COPPER, HI_COPPER),
	RING(GAIN_CONSTITUTION, "gain constitution", 0, "opal", 150, 1, 1, 7, MINERAL, HI_MINERAL),
	RING(GAIN_DEXTERITY, "gain dexterity", 0, "obsidian", 150, 1, 1, 7, GEMSTONE, CLR_BLACK),
	RING(GAIN_INTELLIGENCE, "gain intelligence", 0, "plain", 150, 1, 1, 7, MINERAL, HI_MINERAL),
	RING(GAIN_STRENGTH, "gain strength", 0, "granite", 150, 1, 1, 7, MINERAL, HI_MINERAL),
	RING(GAIN_WISDOM, "gain wisdom", 0, "glass", 150, 1, 1, 7, MINERAL, CLR_CYAN),
	RING(INCREASE_ACCURACY, "increase accuracy", 0, "clay", 150, 1, 1, 4, MINERAL, CLR_RED),
	RING(INCREASE_DAMAGE, "increase damage", 0, "coral", 150, 1, 1, 4, MINERAL, CLR_ORANGE),
	RING(SLOW_DIGESTION, "slow digestion", SLOW_DIGESTION, "steel",
	     200, 1, 0, 8, IRON, HI_METAL),
	RING(INVISIBILITY, "invisibility", INVIS, "wire", 150, 1, 0, 5, IRON, HI_METAL),
	RING(POISON_RESISTANCE, "poison resistance", POISON_RES, "pearl",
	     150, 1, 0, 4, IRON, CLR_WHITE),
	RING(SEE_INVISIBLE, "see invisible", SEE_INVIS, "engagement",
	     150, 1, 0, 5, IRON, HI_METAL),
	RING(SHOCK_RESISTANCE, "shock resistance", SHOCK_RES, "copper",
	     150, 1, 0, 3, COPPER, HI_COPPER),
	RING(FIRE_RESISTANCE, "fire resistance", FIRE_RES, "iron", 200, 1, 0, 5, IRON, HI_METAL),
	RING(FREE_ACTION, "free action", FREE_ACTION, "twisted",
	     200, 1, 0, 6, IRON, HI_METAL),
	/*RING(INFRAVISION, "infravision", 0, "zinc",              200, 1, 0, 5, MITHRIL, HI_METAL),*/
	RING(LEVITATION, "levitation", LEVITATION, "agate", 200, 1, 0, 7, GEMSTONE, CLR_RED),
	RING(REGENERATION, "regeneration", REGENERATION, "moonstone",
	     200, 1, 0, 6, MINERAL, HI_MINERAL),
	RING(SEARCHING, "searching", SEARCHING, "tiger eye", 200, 1, 0, 6, GEMSTONE, CLR_BROWN),
	RING(TELEPORTATION, "teleportation", TELEPORT, "silver", 200, 1, 0, 3, SILVER, HI_SILVER),
	RING(CONFLICT, "conflict", CONFLICT, "ruby", 300, 1, 0, 9, GEMSTONE, CLR_RED),
	RING(POLYMORPH, "polymorph", POLYMORPH, "ivory", 300, 1, 0, 4, BONE, CLR_WHITE),
	RING(POLYMORPH_CONTROL, "polymorph control", POLYMORPH_CONTROL, "emerald",
	     300, 1, 0, 8, GEMSTONE, CLR_BRIGHT_GREEN),
	RING(TELEPORT_CONTROL, "teleport control", TELEPORT_CONTROL, "gold",
	     300, 1, 0, 3, GOLD, HI_GOLD),
/* More descriptions: cameo, intaglio */
#undef RING

/* amulets ... - THE Amulet comes last because it is special */
#define AMULET(o_id, name, desc, power, prob)                                           \
	OBJECT(o_id, name, desc, BITS(0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, IRON), power, \
	       AMULET_CLASS, prob, 0, 20, 150, 0, 0, 0, 0, 20, HI_METAL)
	AMULET(AMULET_OF_CHANGE, "amulet of change", "square", 0, 110),
	AMULET(AMULET_OF_DRAIN_RESISTANCE, "amulet of drain resistance", "warped", DRAIN_RES, 60),
	AMULET(AMULET_OF_ESP, "amulet of ESP", "circular", TELEPAT, 140),
	AMULET(AMULET_OF_FLYING, "amulet of flying", "convex", FLYING, 50),
	AMULET(AMULET_OF_LIFE_SAVING, "amulet of life saving", "spherical", LIFESAVED, 60),
	AMULET(AMULET_OF_MAGICAL_BREATHING, "amulet of magical breathing", "octagonal", MAGICAL_BREATHING, 50),
	AMULET(AMULET_OF_REFLECTION, "amulet of reflection", "hexagonal", REFLECTING, 60),
	AMULET(AMULET_OF_RESTFUL_SLEEP, "amulet of restful sleep", "triangular", SLEEPING, 110),
	AMULET(AMULET_OF_STRANGULATION, "amulet of strangulation", "oval", STRANGLED, 110),
	AMULET(AMULET_OF_UNCHANGING, "amulet of unchanging", "concave", UNCHANGING, 50),
	AMULET(AMULET_VERSUS_POISON, "amulet versus poison", "pyramidal", POISON_RES, 140),
	AMULET(AMULET_VERSUS_STONE, "amulet versus stone", "lunate", /*STONE_RES*/ 0, 60),
	OBJECT(FAKE_AMULET_OF_YENDOR, "cheap plastic imitation of the Amulet of Yendor",
	       "Amulet of Yendor", BITS(0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, PLASTIC), 0,
	       AMULET_CLASS, 0, 0, 20, 0, 0, 0, 0, 0, 1, HI_METAL),
	OBJECT(AMULET_OF_YENDOR, "Amulet of Yendor", /* note: description == name */
	       "Amulet of Yendor", BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, MITHRIL), 0,
	       AMULET_CLASS, 0, 0, 20, 30000, 0, 0, 0, 0, 20, HI_METAL),
#undef AMULET

/* tools ... */
/* tools with weapon characteristics come last */
#define TOOL(o_id, name, desc, kn, mrg, mgc, chg, prob, wt, cost, mat, color) \
	OBJECT(o_id, name, desc,                                              \
	       BITS(kn, mrg, chg, 0, mgc, chg, 0, 0, 0, 0, 0, P_NONE, mat),   \
	       0, TOOL_CLASS, prob, 0,                                        \
	       wt, cost, 0, 0, 0, 0, wt, color)
#define CONTAINER(o_id, name, desc, kn, mgc, chg, bi, prob, wt, cost, mat, color) \
	OBJECT(o_id, name, desc,                                                  \
	       BITS(kn, 0, chg, 1, mgc, chg, 0, 0, bi, 0, 0, P_NONE, mat),        \
	       0, TOOL_CLASS, prob, 0,                                            \
	       wt, cost, 0, 0, 0, 0, wt, color)
#define WEPTOOL(o_id, name, desc, kn, mgc, chg, bi, prob, wt, cost, sdam, ldam, hitbon, typ, sub, mat, clr) \
	OBJECT(o_id, name, desc,                                                                            \
	       BITS(kn, 0, 1, chg, mgc, 1, 0, 0, bi, 0, typ, sub, mat),                                     \
	       0, TOOL_CLASS, prob, 0,                                                                      \
	       wt, cost, sdam, ldam, hitbon, 0, wt, clr)
	/* Containers */
	CONTAINER(LARGE_BOX, "large box", NULL, 1, 0, 0, 1, 40, 350, 8, WOOD, HI_WOOD),
	CONTAINER(CHEST, "chest", NULL, 1, 0, 0, 1, 35, 600, 16, WOOD, HI_WOOD),
	CONTAINER(ICE_BOX, "ice box", NULL, 1, 0, 0, 1, 10, 900, 42, PLASTIC, CLR_WHITE),
	CONTAINER(SACK, "sack", "bag", 0, 0, 0, 0, 40, 15, 2, CLOTH, HI_CLOTH),
	CONTAINER(OILSKIN_SACK, "oilskin sack", "bag", 0, 0, 0, 0, 10, 15, 100, CLOTH, HI_CLOTH),
	CONTAINER(BAG_OF_HOLDING, "bag of holding", "bag", 0, 1, 0, 0, 20, 15, 100, CLOTH, HI_CLOTH),
	CONTAINER(BAG_OF_TRICKS, "bag of tricks", "bag", 0, 1, 1, 0, 20, 15, 100, CLOTH, HI_CLOTH),

	/* Unlocking tools */
	TOOL(SKELETON_KEY, "skeleton key", "key", 0, 0, 0, 0, 80, 3, 10, IRON, HI_METAL),
	TOOL(LOCK_PICK, "lock pick", NULL, 1, 0, 0, 0, 60, 4, 20, IRON, HI_METAL),
	TOOL(CREDIT_CARD, "credit card", NULL, 1, 0, 0, 0, 15, 1, 10, PLASTIC, CLR_WHITE),

	/* Light sources */
	/* [Tom] made candles cheaper & more common */
	TOOL(TALLOW_CANDLE, "tallow candle", "candle", 0, 1, 0, 0, 50, 2, 1, WAX, CLR_WHITE),
	TOOL(WAX_CANDLE, "wax candle", "candle", 0, 1, 0, 0, 40, 2, 2, WAX, CLR_WHITE),
	TOOL(MAGIC_CANDLE, "magic candle", "candle", 0, 1, 1, 0, 5, 2, 500, WAX, CLR_WHITE),
	TOOL(OIL_LAMP, "oil lamp", "lamp", 0, 0, 0, 0, 25, 20, 10, COPPER, CLR_YELLOW),
	TOOL(BRASS_LANTERN, "brass lantern", NULL, 1, 0, 0, 0, 15, 30, 12, COPPER, CLR_YELLOW),
	TOOL(MAGIC_LAMP, "magic lamp", "lamp", 0, 0, 1, 0, 10, 20, 1000, COPPER, CLR_YELLOW),

	/* Instruments */
	/* KMH -- made less common */
	TOOL(TIN_WHISTLE, "tin whistle", "whistle", 0, 0, 0, 0, 63, 3, 10, METAL, HI_METAL),
	TOOL(MAGIC_WHISTLE, "magic whistle", "whistle", 0, 0, 1, 0, 25, 3, 10, METAL, HI_METAL),
	/* "If tin whistles are made out of tin, what do they make foghorns out of?" */
	TOOL(WOODEN_FLUTE, "wooden flute", "flute", 0, 0, 0, 0, 2, 5, 12, WOOD, HI_WOOD),
	TOOL(MAGIC_FLUTE, "magic flute", "flute", 0, 0, 1, 1, 1, 5, 36, WOOD, HI_WOOD),
	TOOL(TOOLED_HORN, "tooled horn", "horn", 0, 0, 0, 0, 2, 18, 15, BONE, CLR_WHITE),
	TOOL(FROST_HORN, "frost horn", "horn", 0, 0, 1, 1, 1, 18, 50, BONE, CLR_WHITE),
	TOOL(FIRE_HORN, "fire horn", "horn", 0, 0, 1, 1, 1, 18, 50, BONE, CLR_WHITE),
	TOOL(HORN_OF_PLENTY, "horn of plenty", "horn", 0, 0, 1, 1, 1, 18, 50, BONE, CLR_WHITE),
	TOOL(WOODEN_HARP, "wooden harp", "harp", 0, 0, 0, 0, 2, 30, 50, WOOD, HI_WOOD),
	TOOL(MAGIC_HARP, "magic harp", "harp", 0, 0, 1, 1, 1, 30, 50, WOOD, HI_WOOD),
	TOOL(BELL, "bell", NULL, 1, 0, 0, 0, 1, 30, 50, COPPER, HI_COPPER),
	TOOL(BUGLE, "bugle", NULL, 1, 0, 0, 0, 2, 10, 15, COPPER, HI_COPPER),
	TOOL(LEATHER_DRUM, "leather drum", "drum", 0, 0, 0, 0, 2, 25, 25, LEATHER, HI_LEATHER),
	TOOL(DRUM_OF_EARTHQUAKE, "drum of earthquake", "drum",
	     0, 0, 1, 1, 1, 25, 25, LEATHER, HI_LEATHER),

	/* Traps */
	TOOL(LAND_MINE, "land mine", NULL, 1, 0, 0, 0, 0, 300, 180, IRON, CLR_RED),
	TOOL(BEARTRAP, "beartrap", NULL, 1, 0, 0, 0, 0, 200, 60, IRON, HI_METAL),

	/* Weapon-tools */
	/* Added by Tsanth, in homage to Final Fantasy 2 */
	/* KMH -- Not randomly generated (no damage!) */
	WEPTOOL(SPOON, "spoon", NULL,
		1, 0, 0, 0, 0, 1, 5000, 0, 0, 0, WHACK, -P_DART, PLATINUM, HI_METAL),
	WEPTOOL(PICK_AXE, "pick-axe", NULL,
		1, 0, 0, 0, 17, 80, 50, 6, 3, 0, WHACK, P_PICK_AXE, IRON, HI_METAL),
	WEPTOOL(FISHING_POLE, "fishing pole", NULL,
		1, 0, 0, 0, 5, 30, 50, 2, 6, 0, WHACK, P_POLEARMS, METAL, HI_METAL),
	WEPTOOL(GRAPPLING_HOOK, "grappling hook", "iron hook",
		0, 0, 0, 0, 5, 30, 50, 2, 6, 0, WHACK, P_FLAIL, IRON, HI_METAL),
	/* 3.4.1: unicorn horn left classified as "magic" */
	WEPTOOL(UNICORN_HORN, "unicorn horn", NULL,
		1, 1, 0, 1, 0, 20, 100, 12, 12, 0, PIERCE, P_UNICORN_HORN, BONE, CLR_WHITE),
	/* WEPTOOL("torch", NULL,
		   1, 0, 0,  0,  25, 8, 5, 2, WHACK, P_CLUB, WOOD, HI_WOOD), */

	OBJECT(TORCH, "torch", NULL,
	       BITS(1, 1, 1, 0, 0, 1, 0, 0, 0, 0, WHACK, P_CLUB, WOOD),
	       0, TOOL_CLASS, 25, 0,
	       20, 8, 2, 5, WHACK, 0, 20, HI_WOOD),

	/* [WAC]
	 * Lightsabers are -3 to hit
	 * Double lightsaber is -4 to hit (only red)
	 * DMG is increased: 10.5/15.5
	 * green :9 + d3, 13 + d5
	 * blue : 8 + d5, 12 + d7
	 * red :  6 + d9, 10 + d11
	 * red double: 6 + d9 + d9, 10 + d11 + d11  (15/21) in double mode
	 */
	WEPTOOL(GREEN_LIGHTSABER, "green lightsaber", "lightsaber",
		0, 0, 1, 0, 1, 60, 500, 3, 5, -3, SLASH, P_LIGHTSABER, PLASTIC, HI_METAL),
	WEPTOOL(BLUE_LIGHTSABER, "blue lightsaber", "lightsaber",
		0, 0, 1, 0, 1, 60, 500, 5, 7, -3, SLASH, P_LIGHTSABER, PLATINUM, HI_METAL),
	WEPTOOL(RED_LIGHTSABER, "red lightsaber", "lightsaber",
		0, 0, 1, 0, 1, 60, 500, 9, 11, -3, SLASH, P_LIGHTSABER, PLATINUM, HI_METAL),
	WEPTOOL(RED_DOUBLE_LIGHTSABER, "red double lightsaber", "double lightsaber",
		0, 0, 1, 1, 0, 60, 1000, 9, 11, -4, SLASH, P_LIGHTSABER, PLATINUM, HI_METAL),

	// Other tools
	TOOL(EXPENSIVE_CAMERA, "expensive camera", NULL, 1, 0, 0, 1, 10, 12, 200, PLASTIC, CLR_BLACK),
	TOOL(MIRROR, "mirror", "looking glass", 0, 0, 0, 0, 40, 13, 10, GLASS, HI_SILVER),
	TOOL(CRYSTAL_BALL, "crystal ball", "glass orb", 0, 0, 1, 1, 10, 150, 60, GLASS, HI_GLASS),
#if 0
	/* STEPHEN WHITE'S NEW CODE */
	/* KMH -- removed because there's potential for abuse */
	TOOL(ORB_OF_ENCHANTMENT, "orb of enchantment", "glass orb",
	     0, 0, 1, 1,   5, 75, 750, GLASS, HI_GLASS),
	TOOL(ORB_OF_CHARGING, "orb of charging", "glass orb",
	     0, 0, 1, 1,   5, 75, 750, GLASS, HI_GLASS),
	TOOL(ORB_OF_DESTRUCTION, "orb of destruction", "glass orb",
	     0, 0, 0, 0,   5, 75, 750, GLASS, HI_GLASS),
#endif
	TOOL(LENSES, "lenses", NULL, 1, 0, 0, 0, 5, 3, 80, GLASS, HI_GLASS),
	TOOL(BLINDFOLD, "blindfold", NULL, 1, 0, 0, 0, 50, 2, 20, CLOTH, CLR_BLACK),
	TOOL(TOWEL, "towel", NULL, 1, 0, 0, 0, 50, 2, 50, CLOTH, CLR_MAGENTA),
	TOOL(SADDLE, "saddle", NULL, 1, 0, 0, 0, 5, 100, 150, LEATHER, HI_LEATHER),
	TOOL(LEASH, "leash", NULL, 1, 0, 0, 0, 65, 12, 20, LEATHER, HI_LEATHER),
	TOOL(STETHOSCOPE, "stethoscope", NULL, 1, 0, 0, 0, 25, 4, 75, IRON, HI_METAL),
	TOOL(TINNING_KIT, "tinning kit", NULL, 1, 0, 0, 1, 15, 75, 30, IRON, HI_METAL),
	CONTAINER(MEDICAL_KIT, "medical kit", "leather bag",
		  0, 0, 0, 0, 10, 25, 500, LEATHER, HI_LEATHER),
	TOOL(TIN_OPENER, "tin opener", NULL, 1, 0, 0, 0, 25, 4, 30, IRON, HI_METAL),
	TOOL(CAN_OF_GREASE, "can of grease", NULL, 1, 0, 0, 1, 15, 15, 20, IRON, HI_METAL),
	TOOL(FIGURINE, "figurine", NULL, 1, 0, 1, 0, 25, 50, 80, MINERAL, HI_MINERAL),
	TOOL(MAGIC_MARKER, "magic marker", NULL, 1, 0, 1, 1, 15, 2, 50, PLASTIC, CLR_RED),

	/* Two pseudo tools. These can never exist outside of medical kits. */
	OBJECT(BANDAGE, "bandage", NULL,
	       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, CLOTH), 0,
	       TOOL_CLASS, 0, 0, 1, 1, 0, 0, 0, 0, 0, CLR_WHITE),
	OBJECT(PHIAL, "phial", NULL,
	       BITS(1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, GLASS), 0,
	       TOOL_CLASS, 0, 0, 2, 1, 0, 0, 0, 0, 1, HI_GLASS),

	/* Two special unique artifact "tools" */
	OBJECT(CANDELABRUM_OF_INVOCATION, "Candelabrum of Invocation", "candelabrum",
	       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, GOLD), 0,
	       TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 200, HI_GOLD),
	OBJECT(BELL_OF_OPENING, "Bell of Opening", "silver bell",
	       BITS(0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, SILVER), 0,
	       TOOL_CLASS, 0, 0, 10, 5000, 0, 0, 0, 0, 50, HI_SILVER),
#undef TOOL
#undef CONTAINER
#undef WEPTOOL

/* Comestibles ... */
#define FOOD(o_id, name, prob, delay, wt, unk, tin, nutrition, color)                \
	OBJECT(o_id, name, NULL, BITS(1, 1, unk, 0, 0, 0, 0, 0, 0, 0, 0, 0, tin), 0, \
	       FOOD_CLASS, prob, delay,                                              \
	       wt, nutrition / 20 + 5, 0, 0, 0, 0, nutrition, color)
	/* all types of food (except tins & corpses) must have a delay of at least 1. */
	/* delay on corpses is computed and is weight dependant */
	/* dog eats foods 0-4 but prefers tripe rations above all others */
	/* fortune cookies can be read */
	/* carrots improve your vision */
	/* +0 tins contain monster meat */
	/* +1 tins (of spinach) make you stronger (like Popeye) */
	/* food CORPSE is a cadaver of some type */
	/* meatballs/sticks/rings are only created from objects via stone to flesh */

	/* Meat */
	FOOD(TRIPE_RATION, "tripe ration", 142, 2, 10, 0, FLESH, 200, CLR_BROWN),
	FOOD(CORPSE, "corpse", 0, 1, 0, 0, FLESH, 0, CLR_BROWN),
	FOOD(EGG, "egg", 80, 1, 1, 1, FLESH, 80, CLR_WHITE),
	FOOD(MEATBALL, "meatball", 0, 1, 1, 0, FLESH, 5, CLR_BROWN),
	FOOD(MEAT_STICK, "meat stick", 0, 1, 1, 0, FLESH, 5, CLR_BROWN),
	FOOD(HUGE_CHUNK_OF_MEAT, "huge chunk of meat", 0, 20, 400, 0, FLESH, 2000, CLR_BROWN),
	/* special case because it's not mergable */
	OBJECT(MEAT_RING, "meat ring", NULL,
	       BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, FLESH),
	       0, FOOD_CLASS, 0, 1, 5, 1, 0, 0, 0, 0, 5, CLR_BROWN),

	/* Body parts.... eeeww */
	FOOD(EYEBALL, "eyeball", 0, 1, 0, 0, FLESH, 10, CLR_WHITE),
	FOOD(SEVERED_HAND, "severed hand", 0, 1, 0, 0, FLESH, 40, CLR_BROWN),

	/* Fruits & veggies */
	FOOD(KELP_FROND, "kelp frond", 0, 1, 1, 0, VEGGY, 30, CLR_GREEN),
	FOOD(EUCALYPTUS_LEAF, "eucalyptus leaf", 4, 1, 1, 0, VEGGY, 30, CLR_GREEN),
	FOOD(CLOVE_OF_GARLIC, "clove of garlic", 7, 1, 1, 0, VEGGY, 40, CLR_WHITE),
	FOOD(SPRIG_OF_WOLFSBANE, "sprig of wolfsbane", 7, 1, 1, 0, VEGGY, 40, CLR_GREEN),
	FOOD(APPLE, "apple", 13, 1, 2, 0, VEGGY, 50, CLR_RED),
	FOOD(CARROT, "carrot", 15, 1, 2, 0, VEGGY, 50, CLR_ORANGE),
	FOOD(PEAR, "pear", 9, 1, 2, 0, VEGGY, 50, CLR_BRIGHT_GREEN),
	FOOD(ASIAN_PEAR, "asian pear", 1, 1, 2, 0, VEGGY, 75, CLR_BRIGHT_GREEN),
	FOOD(BANANA, "banana", 10, 1, 2, 0, VEGGY, 80, CLR_YELLOW),
	FOOD(ORANGE, "orange", 10, 1, 2, 0, VEGGY, 80, CLR_ORANGE),
	FOOD(MUSHROOM, "mushroom", 1, 1, 5, 0, VEGGY, 90, CLR_BLACK),
	FOOD(MELON, "melon", 9, 1, 5, 0, VEGGY, 100, CLR_BRIGHT_GREEN),
	FOOD(SLIME_MOLD, "slime mold", 75, 1, 5, 0, VEGGY, 250, HI_ORGANIC),

	/* People food */
	FOOD(LUMP_OF_ROYAL_JELLY, "lump of royal jelly", 1, 1, 2, 0, VEGGY, 200, CLR_YELLOW),
	FOOD(CREAM_PIE, "cream pie", 10, 1, 10, 0, VEGGY, 100, CLR_WHITE),
	FOOD(SANDWICH, "sandwich", 10, 1, 10, 0, FLESH, 100, CLR_WHITE),
	FOOD(CANDY_BAR, "candy bar", 13, 1, 2, 0, VEGGY, 100, CLR_BROWN),
	FOOD(FORTUNE_COOKIE, "fortune cookie", 55, 1, 1, 0, VEGGY, 40, CLR_YELLOW),
	FOOD(PANCAKE, "pancake", 14, 2, 2, 0, VEGGY, 200, CLR_YELLOW),
	FOOD(TORTILLA, "tortilla", 1, 2, 2, 0, VEGGY, 80, CLR_WHITE),
	/* [Tom] more food.... taken off pancake (25) */
	FOOD(CHEESE, "cheese", 10, 2, 2, 0, FLESH, 250, CLR_YELLOW),
	FOOD(PILL, "pill", 1, 1, 1, 0, VEGGY, 0, CLR_BRIGHT_MAGENTA),
	FOOD(HOLY_WAFER, "holy wafer", 7, 1, 1, 0, VEGGY, 150, CLR_WHITE),
	FOOD(LEMBAS_WAFER, "lembas wafer", 20, 2, 5, 0, VEGGY, 800, CLR_WHITE),
	FOOD(CRAM_RATION, "cram ration", 20, 3, 15, 0, VEGGY, 600, HI_ORGANIC),
	FOOD(FOOD_RATION, "food ration", 380, 5, 20, 0, VEGGY, 800, HI_ORGANIC),
	FOOD(K_RATION, "K-ration", 0, 1, 10, 0, VEGGY, 400, HI_ORGANIC),
	FOOD(C_RATION, "C-ration", 0, 1, 10, 0, VEGGY, 300, HI_ORGANIC),
	FOOD(TIN, "tin", 75, 0, 10, 1, METAL, 0, HI_METAL),
#undef FOOD

/* potions ... */
#define POTION(o_id, name, desc, mgc, power, prob, cost, color)                                  \
	OBJECT(POT_##o_id, name, desc, BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, 0, GLASS), power, \
	       POTION_CLASS, prob, 0, 20, cost, 0, 0, 0, 0, 10, color)
	POTION(BOOZE, "booze", "brown", 0, 0, 40, 50, CLR_BROWN),
	POTION(FRUIT_JUICE, "fruit juice", "dark", 0, 0, 40, 50, CLR_BLACK),
	POTION(SEE_INVISIBLE, "see invisible", "magenta", 1, SEE_INVIS, 38, 50, CLR_MAGENTA),
	POTION(SICKNESS, "sickness", "fizzy", 0, 0, 40, 50, CLR_CYAN),
	POTION(SLEEPING, "sleeping", "effervescent", 1, 0, 40, 100, CLR_GRAY),
	POTION(CLAIRVOYANCE, "clairvoyance", "luminescent", 1, 0, 20, 100, CLR_WHITE),
	POTION(CONFUSION, "confusion", "orange", 1, CONFUSION, 40, 100, CLR_ORANGE),
	POTION(HALLUCINATION, "hallucination", "sky blue", 1, HALLUC, 40, 100, CLR_CYAN),
	POTION(HEALING, "healing", "purple-red", 1, 0, 55, 100, CLR_MAGENTA),
	POTION(EXTRA_HEALING, "extra healing", "puce", 1, 0, 45, 100, CLR_RED),
	POTION(RESTORE_ABILITY, "restore ability", "pink", 1, 0, 40, 100, CLR_BRIGHT_MAGENTA),
	POTION(BLINDNESS, "blindness", "yellow", 1, BLINDED, 38, 150, CLR_YELLOW),
	POTION(ESP, "ESP", "muddy", 1, TELEPAT, 20, 150, CLR_BROWN),
	POTION(GAIN_ENERGY, "gain energy", "cloudy", 1, 0, 40, 150, CLR_WHITE),
	POTION(INVISIBILITY, "invisibility", "brilliant blue", 1, INVIS, 40, 150, CLR_BRIGHT_BLUE),
	POTION(MONSTER_DETECTION, "monster detection", "bubbly", 1, 0, 38, 150, CLR_WHITE),
	POTION(OBJECT_DETECTION, "object detection", "smoky", 1, 0, 38, 150, CLR_GRAY),
	POTION(ENLIGHTENMENT, "enlightenment", "swirly", 1, 0, 20, 200, CLR_BROWN),
	POTION(FULL_HEALING, "full healing", "black", 1, 0, 20, 200, CLR_BLACK),
	POTION(LEVITATION, "levitation", "cyan", 1, LEVITATION, 38, 200, CLR_CYAN),
	POTION(POLYMORPH, "polymorph", "golden", 1, 0, 10, 200, CLR_YELLOW),
	POTION(SPEED, "speed", "dark green", 1, FAST, 38, 200, CLR_GREEN),
	POTION(ACID, "acid", "white", 0, 0, 20, 250, CLR_WHITE),
	POTION(OIL, "oil", "murky", 0, 0, 30, 250, CLR_BROWN),
	POTION(GAIN_ABILITY, "gain ability", "ruby", 1, 0, 38, 300, CLR_RED),
	POTION(GAIN_LEVEL, "gain level", "milky", 1, 0, 20, 300, CLR_WHITE),
	POTION(INVULNERABILITY, "invulnerability", "icy", 1, 0, 5, 300, CLR_BRIGHT_BLUE),
	POTION(PARALYSIS, "paralysis", "emerald", 1, 0, 38, 300, CLR_BRIGHT_GREEN),
	POTION(WATER, "water", "clear", 0, 0, 55, 100, CLR_CYAN),
	POTION(BLOOD, "blood", "blood-red", 0, 0, 0, 50, CLR_RED),
	POTION(VAMPIRE_BLOOD, "vampire blood", "blood-red", 1, 0, 0, 350, CLR_RED),
	POTION(AMNESIA, "amnesia", "sparkling", 1, 0, 16, 100, CLR_CYAN),
#undef POTION

/* scrolls ... */
#define SCROLL(o_id, name, text, sub, mgc, prob, cost)                                         \
	OBJECT(SCR_##o_id, name, text, BITS(0, 1, 0, 0, mgc, 0, 0, 0, 0, 0, 0, sub, PAPER), 0, \
	       SCROLL_CLASS, prob, 0, 5, cost, 0, 0, 0, 0, 6, HI_PAPER)
	/* Attack */
	SCROLL(CREATE_MONSTER, "create monster", "LEP GEX VEN ZEA", P_ATTACK_SPELL, 1, 45, 200),
	/* Enchantment */
	SCROLL(TAMING, "taming", "PRIRUTSENIE", P_ENCHANTMENT_SPELL, 1, 15, 200),
	/* Divination */
	SCROLL(LIGHT, "light", "VERR YED HORRE", P_DIVINATION_SPELL, 1, 90, 50),
	SCROLL(FOOD_DETECTION, "food detection", "YUM YUM", P_DIVINATION_SPELL, 1, 25, 100),
	SCROLL(GOLD_DETECTION, "gold detection", "THARR", P_DIVINATION_SPELL, 1, 33, 100),
	SCROLL(IDENTIFY, "identify", "KERNOD WEL", P_DIVINATION_SPELL, 1, 185, 20),
	SCROLL(MAGIC_MAPPING, "magic mapping", "ELAM EBOW", P_DIVINATION_SPELL, 1, 45, 100),
	/* Enchantment */
	SCROLL(CONFUSE_MONSTER, "confuse monster", "NR 9", P_ENCHANTMENT_SPELL, 1, 43, 100),
	SCROLL(SCARE_MONSTER, "scare monster", "XIXAXA XOXAXA XUXAXA", P_ENCHANTMENT_SPELL, 1, 35, 100),
	SCROLL(ENCHANT_WEAPON, "enchant weapon", "DAIYEN FOOELS", P_ENCHANTMENT_SPELL, 1, 80, 60),
	SCROLL(ENCHANT_ARMOR, "enchant armor", "ZELGO MER", P_ENCHANTMENT_SPELL, 1, 63, 80),
	/* Protection */
	SCROLL(REMOVE_CURSE, "remove curse", "PRATYAVAYAH", P_PROTECTION_SPELL, 1, 65, 80),
	/* Body */
	SCROLL(TELEPORTATION, "teleportation", "VENZAR BORGAVVE", P_BODY_SPELL, 1, 55, 100),
	/* Matter */
	SCROLL(FIRE, "fire", "ANDOVA BEGARIN", P_MATTER_SPELL, 1, 33, 100),
	SCROLL(EARTH, "earth", "KIRJE", P_MATTER_SPELL, 1, 20, 200),

	SCROLL(DESTROY_ARMOR, "destroy armor", "JUYED AWK YACC", P_NONE, 1, 45, 100),
	SCROLL(AMNESIA, "amnesia", "DUAM XNAHT", P_NONE, 1, 35, 200),
	SCROLL(CHARGING, "charging", "HACKEM MUCHE", P_NONE, 1, 15, 300),
	SCROLL(GENOCIDE, "genocide", "ELBIB YLOH", P_NONE, 1, 15, 300),
	SCROLL(PUNISHMENT, "punishment", "VE FORBRYDERNE", P_NONE, 1, 15, 300),
	SCROLL(STINKING_CLOUD, "stinking cloud", "VELOX NEB", P_NONE, 1, 15, 300),
	SCROLL(_OBJECT_DUMMY1, NULL, "FOOBIE BLETCH", P_NONE, 1, 0, 100),
	SCROLL(_OBJECT_DUMMY2, NULL, "TEMOV", P_NONE, 1, 0, 100),
	SCROLL(_OBJECT_DUMMY3, NULL, "GARVEN DEH", P_NONE, 1, 0, 100),
	SCROLL(_OBJECT_DUMMY4, NULL, "READ ME", P_NONE, 1, 0, 100),
/* these must come last because they have special descriptions */
#ifdef MAIL
	SCROLL(MAIL, "mail", "stamped", P_NONE, 0, 0, 0),
#endif
	SCROLL(BLANK_PAPER, "blank paper", "unlabeled", P_NONE, 0, 28, 60),
#undef SCROLL

/* spell books ... */
#define SPELL(o_id, name, desc, sub, prob, delay, level, mgc, dir, color)                        \
	OBJECT(SPE_##o_id, name, desc, BITS(0, 0, 1, 0, mgc, 1, 0, 0, 0, 0, dir, sub, PAPER), 0, \
	       SPBOOK_CLASS, prob, delay,                                                        \
	       50, level * 100, 0, 0, 0, level, 20, color)
#ifdef ONAMES_H
	FIRST_SPELL,
#endif
	/* Attack spells */
	SPELL(FORCE_BOLT = FIRST_SPELL, "force bolt", "red", P_ATTACK_SPELL, 25, 2, 1, 1, IMMEDIATE, CLR_RED),
	SPELL(CREATE_MONSTER, "create monster", "turquoise", P_ATTACK_SPELL, 25, 3, 2, 1, NODIR, CLR_BRIGHT_CYAN),
	SPELL(DRAIN_LIFE, "drain life", "velvet", P_ATTACK_SPELL, 10, 4, 3, 1, IMMEDIATE, CLR_MAGENTA),
	/* NEEDS TILE */ /* WAC -- probs from force bolt and extra healing */
	SPELL(COMMAND_UNDEAD, "command undead", "dark", P_ATTACK_SPELL, 10, 7, 5, 1, IMMEDIATE, CLR_BLACK),
	SPELL(SUMMON_UNDEAD, "summon undead", "black", P_ATTACK_SPELL, 10, 7, 5, 1, IMMEDIATE, CLR_BLACK),
	/* Healing spells */
	SPELL(STONE_TO_FLESH, "stone to flesh", "thick", P_HEALING_SPELL, 15, 1, 3, 1, IMMEDIATE, HI_PAPER),
	SPELL(HEALING, "healing", "white", P_HEALING_SPELL, 30, 2, 1, 1, IMMEDIATE, CLR_WHITE),
	SPELL(CURE_BLINDNESS, "cure blindness", "yellow", P_HEALING_SPELL, 20, 2, 2, 1, IMMEDIATE, CLR_YELLOW),
	SPELL(CURE_SICKNESS, "cure sickness", "indigo", P_HEALING_SPELL, 20, 3, 3, 1, NODIR, CLR_BLUE),
	SPELL(EXTRA_HEALING, "extra healing", "plaid", P_HEALING_SPELL, 15, 5, 3, 1, IMMEDIATE, CLR_GREEN),
	SPELL(RESTORE_ABILITY, "restore ability", "light brown", P_HEALING_SPELL, 15, 5, 4, 1, NODIR, CLR_BROWN),
	SPELL(CREATE_FAMILIAR, "create familiar", "glittering", P_HEALING_SPELL, 10, 7, 6, 1, NODIR, CLR_WHITE),
	/* Divination spells */
	SPELL(LIGHT, "light", "cloth", P_DIVINATION_SPELL, 30, 1, 1, 1, NODIR, HI_CLOTH),
	SPELL(DETECT_MONSTERS, "detect monsters", "leather", P_DIVINATION_SPELL, 20, 1, 1, 1, NODIR, HI_LEATHER),
	SPELL(DETECT_FOOD, "detect food", "cyan", P_DIVINATION_SPELL, 15, 3, 2, 1, NODIR, CLR_CYAN),
	SPELL(CLAIRVOYANCE, "clairvoyance", "dark blue", P_DIVINATION_SPELL, 15, 3, 3, 1, NODIR, CLR_BLUE),
	SPELL(DETECT_UNSEEN, "detect unseen", "violet", P_DIVINATION_SPELL, 15, 4, 3, 1, NODIR, CLR_MAGENTA),
	SPELL(IDENTIFY, "identify", "bronze", P_DIVINATION_SPELL, 30, 8, 5, 1, NODIR, HI_COPPER),
	SPELL(DETECT_TREASURE, "detect treasure", "gray", P_DIVINATION_SPELL, 25, 5, 4, 1, NODIR, CLR_GRAY),
	SPELL(MAGIC_MAPPING, "magic mapping", "dusty", P_DIVINATION_SPELL, 15, 7, 5, 1, NODIR, HI_PAPER),
	/* Enchantment spells */
	SPELL(CONFUSE_MONSTER, "confuse monster", "orange", P_ENCHANTMENT_SPELL, 25, 2, 2, 1, IMMEDIATE, CLR_ORANGE),
	SPELL(SLOW_MONSTER, "slow monster", "light green", P_ENCHANTMENT_SPELL, 25, 2, 2, 1, IMMEDIATE, CLR_BRIGHT_GREEN),
	SPELL(CAUSE_FEAR, "cause fear", "light blue", P_ENCHANTMENT_SPELL, 20, 3, 3, 1, NODIR, CLR_BRIGHT_BLUE),
	SPELL(CHARM_MONSTER, "charm monster", "magenta", P_ENCHANTMENT_SPELL, 20, 3, 3, 1, IMMEDIATE, CLR_MAGENTA),
	SPELL(ENCHANT_WEAPON, "enchant weapon", "dull", P_ENCHANTMENT_SPELL, 5, 8, 7, 1, NODIR, CLR_WHITE),
	SPELL(ENCHANT_ARMOR, "enchant armor", "thin", P_ENCHANTMENT_SPELL, 5, 8, 7, 1, NODIR, CLR_WHITE),
	/* Protection spells */
	SPELL(PROTECTION, "protection", "wide", P_PROTECTION_SPELL, 5, 3, 1, 1, NODIR, HI_PAPER),
	SPELL(RESIST_POISON, "resist poison", "big", P_PROTECTION_SPELL, 20, 2, 1, 1, NODIR, CLR_GRAY),
	SPELL(RESIST_SLEEP, "resist sleep", "fuzzy", P_PROTECTION_SPELL, 20, 2, 1, 1, NODIR, CLR_BROWN),
	SPELL(ENDURE_COLD, "endure cold", "deep", P_PROTECTION_SPELL, 15, 3, 2, 1, NODIR, HI_PAPER),
	SPELL(ENDURE_HEAT, "endure heat", "spotted", P_PROTECTION_SPELL, 15, 3, 2, 1, NODIR, HI_PAPER),
	SPELL(INSULATE, "insulate", "long", P_PROTECTION_SPELL, 15, 3, 2, 1, NODIR, HI_PAPER),
	SPELL(REMOVE_CURSE, "remove curse", "wrinkled", P_PROTECTION_SPELL, 25, 5, 5, 1, NODIR, HI_PAPER),
	SPELL(TURN_UNDEAD, "turn undead", "copper", P_PROTECTION_SPELL, 15, 8, 6, 1, IMMEDIATE, HI_COPPER),
	/* Body spells */
	SPELL(JUMPING, "jumping", "torn", P_BODY_SPELL, 15, 3, 1, 1, IMMEDIATE, HI_PAPER),
	SPELL(HASTE_SELF, "haste self", "purple", P_BODY_SPELL, 20, 4, 3, 1, NODIR, CLR_MAGENTA),
	SPELL(ENLIGHTEN, "enlighten", "faded", P_BODY_SPELL, 15, 5, 4, 1, NODIR, CLR_GRAY),
	SPELL(INVISIBILITY, "invisibility", "dark brown", P_BODY_SPELL, 20, 5, 4, 1, NODIR, CLR_BROWN),
	SPELL(LEVITATION, "levitation", "tan", P_BODY_SPELL, 15, 4, 4, 1, NODIR, CLR_BROWN),
	SPELL(TELEPORT_AWAY, "teleport away", "gold", P_BODY_SPELL, 15, 6, 6, 1, IMMEDIATE, HI_GOLD),
	SPELL(PASSWALL, "passwall", "ochre", P_BODY_SPELL, 5, 7, 6, 1, NODIR, CLR_YELLOW),
	SPELL(POLYMORPH, "polymorph", "silver", P_BODY_SPELL, 15, 8, 6, 1, IMMEDIATE, HI_SILVER),
	/* Matter spells */
	SPELL(KNOCK, "knock", "pink", P_MATTER_SPELL, 25, 1, 1, 1, IMMEDIATE, CLR_BRIGHT_MAGENTA),
	SPELL(FLAME_SPHERE, "flame sphere", "canvas", P_MATTER_SPELL, 20, 2, 1, 1, NODIR, CLR_BROWN),
	SPELL(FREEZE_SPHERE, "freeze sphere", "hardcover", P_MATTER_SPELL, 20, 2, 1, 1, NODIR, CLR_BROWN),
	SPELL(WIZARD_LOCK, "wizard lock", "dark green", P_MATTER_SPELL, 30, 3, 2, 1, IMMEDIATE, CLR_GREEN),
	SPELL(DIG, "dig", "parchment", P_MATTER_SPELL, 20, 6, 5, 1, RAY, HI_PAPER),
	SPELL(CANCELLATION, "cancellation", "shining", P_MATTER_SPELL, 15, 8, 7, 1, IMMEDIATE, CLR_WHITE),
	/* Ray type spells are all here.  Kludge for zap.c */
	SPELL(MAGIC_MISSILE, "magic missile", "vellum", P_ATTACK_SPELL, 40, 3, 2, 1, RAY, HI_PAPER),
	SPELL(FIREBALL, "fireball", "ragged", P_MATTER_SPELL, 15, 6, 4, 1, RAY, HI_PAPER),
	SPELL(CONE_OF_COLD, "cone of cold", "dog eared", P_MATTER_SPELL, 15, 8, 5, 1, RAY, HI_PAPER),
	SPELL(SLEEP, "sleep", "mottled", P_ENCHANTMENT_SPELL, 35, 1, 1, 1, RAY, HI_PAPER),
	SPELL(FINGER_OF_DEATH, "finger of death", "stained", P_ATTACK_SPELL, 5, 10, 7, 1, RAY, HI_PAPER),
	SPELL(LIGHTNING, "lightning", "rainbow", P_MATTER_SPELL, 10, 7, 4, 1, RAY, HI_PAPER),
	SPELL(POISON_BLAST, "poison blast", "tattered", P_ATTACK_SPELL, 5, 7, 4, 1, RAY, HI_PAPER),
	SPELL(ACID_STREAM, "acid stream", "colorful", P_MATTER_SPELL, 5, 7, 4, 1, RAY, HI_PAPER),
	/* Description placeholders and special spellbooks */
	SPELL(_DUMMY_SPELL_START, NULL, "tartan", P_NONE, 0, 0, 0, 1, 0, CLR_RED),
	SPELL(_OBJECT_DUMMY6, NULL, "stylish", P_NONE, 0, 0, 0, 1, 0, HI_PAPER),
	SPELL(_OBJECT_DUMMY7, NULL, "psychedelic", P_NONE, 0, 0, 0, 1, 0, CLR_BRIGHT_MAGENTA),
	SPELL(_OBJECT_DUMMY8, NULL, "spiral-bound", P_NONE, 0, 0, 0, 1, 0, HI_PAPER),
	SPELL(_OBJECT_DUMMY9, NULL, "left-handed", P_NONE, 0, 0, 0, 1, 0, HI_PAPER),
	SPELL(_DUMMY_SPELL_END, NULL, "stapled", P_NONE, 0, 0, 0, 1, 0, HI_PAPER),
	SPELL(BLANK_PAPER, "blank paper", "plain", P_NONE, 20, 0, 0, 0, 0, HI_PAPER),
	/* ...Blank spellbook must come last because it retains its description */
	OBJECT(SPE_BOOK_OF_THE_DEAD, "Book of the Dead", "papyrus",
	       BITS(0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, PAPER), 0,
	       SPBOOK_CLASS, 0, 0, 20, 10000, 0, 0, 0, 7, 20, HI_PAPER),
/* ...A special, one of a kind, spellbook */
#ifdef ONAMES_H
	LAST_SPELL = SPE_BOOK_OF_THE_DEAD,
#endif
#undef SPELL

/* wands ... */
#define WAND(o_id, name, typ, prob, cost, mgc, dir, metal, color)                             \
	OBJECT(WAN_##o_id, name, typ, BITS(0, 0, 1, 0, mgc, 1, 0, 0, 0, 0, dir, 0, metal), 0, \
	       WAND_CLASS, prob, 0, 7, cost, 0, 0, 0, 0, 30, color)
	WAND(LIGHT, "light", "glass", 50, 100, 1, NODIR, GLASS, HI_GLASS),
	WAND(NOTHING, "nothing", "oak", 20, 100, 0, IMMEDIATE, WOOD, HI_WOOD),
	WAND(ENLIGHTENMENT, "enlightenment", "crystal", 30, 150, 1, NODIR, GLASS, HI_GLASS),
	WAND(HEALING, "healing", "bamboo", 60, 150, 1, IMMEDIATE, WOOD, CLR_YELLOW),
	WAND(LOCKING, "locking", "aluminum", 25, 150, 1, IMMEDIATE, METAL, HI_METAL),
	WAND(MAKE_INVISIBLE, "make invisible", "marble", 45, 150, 1, IMMEDIATE, MINERAL, HI_MINERAL),
	WAND(OPENING, "opening", "zinc", 25, 150, 1, IMMEDIATE, METAL, HI_METAL),
	WAND(PROBING, "probing", "uranium", 30, 150, 1, IMMEDIATE, METAL, HI_METAL),
	WAND(SECRET_DOOR_DETECTION, "secret door detection", "balsa",
	     30, 150, 1, NODIR, WOOD, HI_WOOD),
	WAND(SLOW_MONSTER, "slow monster", "tin", 45, 150, 1, IMMEDIATE, METAL, HI_METAL),
	WAND(SPEED_MONSTER, "speed monster", "brass", 45, 150, 1, IMMEDIATE, COPPER, HI_COPPER),
	WAND(STRIKING, "striking", "ebony", 65, 150, 1, IMMEDIATE, WOOD, HI_WOOD),
	WAND(UNDEAD_TURNING, "undead turning", "copper", 45, 150, 1, IMMEDIATE, COPPER, HI_COPPER),
	WAND(DRAINING, "draining", "ceramic", 15, 175, 1, IMMEDIATE, MINERAL, HI_MINERAL),
	/* KMH -- 15/1000 probability from light */
	WAND(CANCELLATION, "cancellation", "platinum", 45, 200, 1, IMMEDIATE, PLATINUM, CLR_WHITE),
	WAND(CREATE_MONSTER, "create monster", "maple", 35, 200, 1, NODIR, WOOD, HI_WOOD),
	WAND(FEAR, "fear", "rusty", 25, 200, 1, IMMEDIATE, IRON, CLR_RED),
	WAND(POLYMORPH, "polymorph", "silver", 45, 200, 1, IMMEDIATE, SILVER, HI_SILVER),
	WAND(TELEPORTATION, "teleportation", "iridium", 45, 200, 1, IMMEDIATE, METAL, CLR_BRIGHT_CYAN),
	WAND(CREATE_HORDE, "create horde", "black", 5, 300, 1, NODIR, PLASTIC, CLR_BLACK),
	WAND(EXTRA_HEALING, "extra healing", "bronze", 30, 300, 1, IMMEDIATE, COPPER, CLR_YELLOW),
	WAND(WISHING, "wishing", "pine", 5, 500, 1, NODIR, WOOD, HI_WOOD),
	/* Ray wands have to come last, and in this order. */
	/* This is extremely kludgy, but that's what zap.c expects. */
	WAND(DIGGING, "digging", "iron", 50, 150, 1, RAY, IRON, HI_METAL),
	WAND(MAGIC_MISSILE, "magic missile", "steel", 50, 150, 1, RAY, IRON, HI_METAL),
	WAND(FIRE, "fire", "hexagonal", 25, 175, 1, RAY, IRON, HI_METAL),
	WAND(COLD, "cold", "short", 30, 175, 1, RAY, IRON, HI_METAL),
	WAND(SLEEP, "sleep", "runed", 50, 175, 1, RAY, IRON, HI_METAL),
	WAND(DEATH, "death", "long", 5, 500, 1, RAY, IRON, HI_METAL),
	WAND(LIGHTNING, "lightning", "curved", 20, 175, 1, RAY, IRON, HI_METAL),
	WAND(FIREBALL, "fireball", "octagonal", 5, 300, 1, RAY, IRON, HI_METAL),
	WAND(_OBJECT_DUMMY11, NULL, "forked", 0, 150, 1, 0, WOOD, HI_WOOD),
	WAND(_OBJECT_DUMMY12, NULL, "spiked", 0, 150, 1, 0, IRON, HI_METAL),
	WAND(_OBJECT_DUMMY13, NULL, "jeweled", 0, 150, 1, 0, IRON, HI_MINERAL),
#undef WAND

/* coins ... - so far, gold is all there is */
#define COIN(o_id, name, prob, metal, worth)                                              \
	OBJECT(o_id, name, NULL, BITS(0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, metal), 0, \
	       COIN_CLASS, prob, 0, 1, worth, 0, 0, 0, 0, 0, HI_GOLD)
	COIN(GOLD_PIECE, "gold piece", 1000, GOLD, 1),
#undef COIN

/* gems ... - includes stones and rocks but not boulders */
#define GEM(o_id, name, desc, prob, wt, gval, nutr, mohs, glass, color)               \
	OBJECT(o_id, name, desc,                                                      \
	       BITS(0, 1, 0, 0, 0, 0, 0, 0, 0, HARDGEM(mohs), 0, -P_SLING, glass), 0, \
	       GEM_CLASS, prob, 0, 1, gval, 3, 3, 0, 0, nutr, color)
#define ROCK(o_id, name, desc, kn, prob, wt, gval, sdam, ldam, mgc, nutr, mohs, glass, color) \
	OBJECT(o_id, name, desc,                                                              \
	       BITS(kn, 1, 0, 0, mgc, 0, 0, 0, 0, HARDGEM(mohs), 0, -P_SLING, glass), 0,      \
	       GEM_CLASS, prob, 0, wt, gval, sdam, ldam, 0, 0, nutr, color)
	GEM(DILITHIUM_CRYSTAL, "dilithium crystal", "white", 2, 1, 4500, 15, 5, GEMSTONE, CLR_WHITE),
	GEM(DIAMOND, "diamond", "white", 3, 1, 4000, 15, 10, GEMSTONE, CLR_WHITE),
	GEM(RUBY, "ruby", "red", 4, 1, 3500, 15, 9, GEMSTONE, CLR_RED),
	GEM(JACINTH, "jacinth", "orange", 3, 1, 3250, 15, 9, GEMSTONE, CLR_ORANGE),
	GEM(SAPPHIRE, "sapphire", "blue", 4, 1, 3000, 15, 9, GEMSTONE, CLR_BLUE),
	GEM(BLACK_OPAL, "black opal", "black", 3, 1, 2500, 15, 8, GEMSTONE, CLR_BLACK),
	GEM(EMERALD, "emerald", "green", 5, 1, 2500, 15, 8, GEMSTONE, CLR_GREEN),
	GEM(TURQUOISE, "turquoise", "green", 6, 1, 2000, 15, 6, GEMSTONE, CLR_GREEN),
	GEM(CITRINE, "citrine", "yellow", 4, 1, 1500, 15, 6, GEMSTONE, CLR_YELLOW),
	GEM(AQUAMARINE, "aquamarine", "green", 6, 1, 1500, 15, 8, GEMSTONE, CLR_GREEN),
	GEM(AMBER, "amber", "yellowish brown", 8, 1, 1000, 15, 2, GEMSTONE, CLR_BROWN),
	GEM(TOPAZ, "topaz", "yellowish brown", 10, 1, 900, 15, 8, GEMSTONE, CLR_BROWN),
	GEM(JET, "jet", "black", 6, 1, 850, 15, 7, GEMSTONE, CLR_BLACK),
	GEM(OPAL, "opal", "white", 12, 1, 800, 15, 6, GEMSTONE, CLR_WHITE),
	GEM(CHRYSOBERYL, "chrysoberyl", "yellow", 8, 1, 700, 15, 5, GEMSTONE, CLR_YELLOW),
	GEM(GARNET, "garnet", "red", 12, 1, 700, 15, 7, GEMSTONE, CLR_RED),
	GEM(AMETHYST, "amethyst", "violet", 14, 1, 600, 15, 7, GEMSTONE, CLR_MAGENTA),
	GEM(JASPER, "jasper", "red", 15, 1, 500, 15, 7, GEMSTONE, CLR_RED),
	GEM(FLUORITE, "fluorite", "violet", 15, 1, 400, 15, 4, GEMSTONE, CLR_MAGENTA),
	GEM(OBSIDIAN, "obsidian", "black", 9, 1, 200, 15, 6, GEMSTONE, CLR_BLACK),
	GEM(AGATE, "agate", "orange", 12, 1, 200, 15, 6, GEMSTONE, CLR_ORANGE),
	GEM(JADE, "jade", "green", 10, 1, 300, 15, 6, GEMSTONE, CLR_GREEN),
#ifdef ONAMES_H
	LAST_GEM = JADE,
#endif
	GEM(WORTHLESS_PIECE_OF_WHITE_GLASS, "worthless piece of white glass", "white", 76, 1, 0, 6, 5, GLASS, CLR_WHITE),
	GEM(WORTHLESS_PIECE_OF_BLUE_GLASS, "worthless piece of blue glass", "blue", 76, 1, 0, 6, 5, GLASS, CLR_BLUE),
	GEM(WORTHLESS_PIECE_OF_RED_GLASS, "worthless piece of red glass", "red", 76, 1, 0, 6, 5, GLASS, CLR_RED),
	GEM(WORTHLESS_PIECE_OF_YELLOWISH_BROWN_GLASS, "worthless piece of yellowish brown glass", "yellowish brown",
	    76, 1, 0, 6, 5, GLASS, CLR_BROWN),
	GEM(WORTHLESS_PIECE_OF_ORANGE_GLASS, "worthless piece of orange glass", "orange", 76, 1, 0, 6, 5, GLASS, CLR_ORANGE),
	GEM(WORTHLESS_PIECE_OF_YELLOW_GLASS, "worthless piece of yellow glass", "yellow", 76, 1, 0, 6, 5, GLASS, CLR_YELLOW),
	GEM(WORTHLESS_PIECE_OF_BLACK_GLASS, "worthless piece of black glass", "black", 76, 1, 0, 6, 5, GLASS, CLR_BLACK),
	GEM(WORTHLESS_PIECE_OF_GREEN_GLASS, "worthless piece of green glass", "green", 76, 1, 0, 6, 5, GLASS, CLR_GREEN),
	GEM(WORTHLESS_PIECE_OF_VIOLET_GLASS, "worthless piece of violet glass", "violet", 76, 1, 0, 6, 5, GLASS, CLR_MAGENTA),

	/* Placement note: there is a wishable subrange for
	 * "gray stones" in the o_ranges[] array in objnam.c
	 * that is currently everything between luckstones and flint (inclusive).
	 */
	ROCK(LUCKSTONE, "luckstone", "gray", 0, 10, 10, 60, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY),
	ROCK(HEALTHSTONE, "healthstone", "gray", 0, 8, 10, 60, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY),
	ROCK(LOADSTONE, "loadstone", "gray", 0, 10, 500, 1, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY),
	ROCK(TOUCHSTONE, "touchstone", "gray", 0, 4, 10, 45, 3, 3, 1, 10, 6, MINERAL, CLR_GRAY),
	ROCK(WHETSTONE, "whetstone", "gray", 0, 3, 10, 45, 3, 3, 1, 10, 7, MINERAL, CLR_GRAY),
	ROCK(FLINT, "flint", "gray", 0, 10, 10, 1, 6, 6, 0, 10, 7, MINERAL, CLR_GRAY),
	ROCK(ROCK, "rock", NULL, 1, 100, 10, 0, 3, 3, 0, 10, 7, MINERAL, CLR_GRAY),
#undef GEM
#undef ROCK

	/* miscellaneous ... */
	/* Note: boulders and rocks are not normally created at random; the
	 * probabilities only come into effect when you try to polymorph them.
	 * Boulders weigh more than MAX_CARR_CAP; statues use corpsenm to take
	 * on a specific type and may act as containers (both affect weight).
	 */
	OBJECT(BOULDER, "boulder", NULL, BITS(1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, P_NONE, MINERAL), 0,
	       ROCK_CLASS, 100, 0, 6000, 0, 20, 20, 0, 0, 2000, HI_MINERAL),
	OBJECT(STATUE, "statue", NULL, BITS(1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, P_NONE, MINERAL), 0,
	       ROCK_CLASS, 900, 0, 2500, 0, 20, 20, 0, 0, 2500, CLR_WHITE),

	OBJECT(HEAVY_IRON_BALL, "heavy iron ball", NULL, BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
	       BALL_CLASS, 1000, 0, 480, 10, 25, 25, 0, 0, 200, HI_METAL),
	/* +d4 when "very heavy" */
	OBJECT(IRON_CHAIN, "iron chain", NULL, BITS(1, 0, 0, 0, 0, 0, 0, 0, 0, 0, WHACK, P_NONE, IRON), 0,
	       CHAIN_CLASS, 1000, 0, 120, 0, 4, 4, 0, 0, 200, HI_METAL),
	/* +1 both l & s */

	OBJECT(BLINDING_VENOM, "blinding venom", "splash of venom",
	       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
	       VENOM_CLASS, 500, 0, 1, 0, 0, 0, 0, 0, 0, HI_ORGANIC),
	OBJECT(ACID_VENOM, "acid venom", "splash of venom",
	       BITS(0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, P_NONE, LIQUID), 0,
	       VENOM_CLASS, 500, 0, 1, 0, 6, 6, 0, 0, 0, HI_ORGANIC),
	/* +d6 small or large */

	/* fencepost, the deadly Array Terminator -- name [1st arg] *must* be NULL */
	OBJECT(NUM_OBJECTS, NULL, NULL, BITS(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, P_NONE, 0), 0,
	       ILLOBJ_CLASS, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)
}; /* objects[] */

#ifndef ONAMES_H
/* dummy routine used to force linkage */
void objects_init() {
	return;
}
#endif

/*objects.c*/
