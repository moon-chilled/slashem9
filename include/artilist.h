/*	SCCS Id: @(#)artilist.h 3.4	2003/02/12	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef ARTILIST_H
#define ARTILIST_H

#include "config.h" /* WAC for blackmarket,  spoon */
#include "artifact.h"
#include "onames.h"

/* in artifact.c, set up the actual artifact list structure */
#define     NO_ATTK	{0,0,0,0}		/* no attack */
#define     NO_DFNS	{0,0,0,0}		/* no defense */
#define     NO_CARY	{0,0,0,0}		/* no carry effects */
#define     DFNS(c)	{0,c,0,0}
#define     CARY(c)	{0,c,0,0}
#define     PHYS(a,b)	{0,AD_PHYS,a,b}		/* physical */
#define     DRLI(a,b)	{0,AD_DRLI,a,b}		/* life drain */
#define     COLD(a,b)	{0,AD_COLD,a,b}
#define     FIRE(a,b)	{0,AD_FIRE,a,b}
#define     ELEC(a,b)	{0,AD_ELEC,a,b}		/* electrical shock */
#define     STUN(a,b)	{0,AD_STUN,a,b}		/* magical attack */


#ifdef ARTILIST_PASS2
# define A(art_id,nam,typ,s1,s2,mt,atk,dfn,cry,inv,al,cl,rac,cost) art_id

enum {
#else
# define A(art_id,nam,typ,s1,s2,mt,atk,dfn,cry,inv,al,cl,rac,cost) \
  { typ, nam, s1, s2, mt, atk, dfn, cry, inv, al, cl, rac, cost, 0 }

static struct artifact artilist[] = {
#endif

	/* Artifact cost rationale:
	 * 1.  The more useful the artifact, the better its cost.
	 * 2.  Quest artifacts are highly valued.
	 * 3.  Chaotic artifacts are inflated due to scarcity (and balance).
	 */


	/* [Tom] rearranged by alignment, so when people ask... */
	/* KMH -- Further arranged:
	 * 1.  By alignment (but quest artifacts last).
	 * 2.  By weapon class (skill).
	 */

	/*  dummy element #0, so that all interesting indices are non-zero */
	A(_DUMMY_ART, "",				STRANGE_OBJECT,
	  0, 0, 0, NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0L ),

	/*** Lawful artifacts ***/
	A(ART_FIREWALL, "Firewall",                  ATHAME,
	  (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	  FIRE(4,4),      FIRE(0,0),      NO_CARY,        0, A_LAWFUL, PM_FLAME_MAGE, NON_PM, 400L ),

	/*
	 *	The combination of SPFX_WARN and M2_something on an artifact
	 *	will trigger EWarn_of_mon for all monsters that have the appropriate
	 *	M2_something flags.  In Sting's case it will trigger EWarn_of_mon
	 *	for M2_ORC monsters.
	 */
	A(ART_STING, "Sting",			ELVEN_DAGGER,
	  (SPFX_WARN|SPFX_DCLAS), 0, M2_ORC,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_LAWFUL, NON_PM, PM_ELF, 800L ),

	A(ART_GIANTKILLER, "Giantkiller",                AXE,
	  (SPFX_RESTR|SPFX_DFLAG2), 0, M2_GIANT,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_NEUTRAL, NON_PM, NON_PM, 800L ),

	A(ART_Quick_BLADE, "Quick Blade",                ELVEN_SHORT_SWORD,
	  SPFX_RESTR, 0, 0,
	  PHYS(9,2),      NO_DFNS,        NO_CARY,        0, A_LAWFUL, NON_PM, NON_PM, 1000L ),

	A(ART_ORCRIST, "Orcrist",                    ELVEN_BROADSWORD,
	  SPFX_DFLAG2, 0, M2_ORC,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_LAWFUL, NON_PM, PM_ELF, 2000L ),

	A(ART_DRAGONBANE, "Dragonbane",			BROADSWORD,
	  (SPFX_RESTR|SPFX_DCLAS), 0, S_DRAGON,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 500L ),

	A(ART_EXCALIBUR, "Excalibur",                  LONG_SWORD,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_SEEK|SPFX_DEFN|SPFX_INTEL|SPFX_SEARCH),0,0,
	  PHYS(5,10),	DRLI(0,0),	NO_CARY,	0, A_LAWFUL, PM_KNIGHT, NON_PM, 4000L ),

	A(ART_SUNSWORD, "Sunsword",                   LONG_SWORD,
	  (SPFX_RESTR|SPFX_DFLAG2), 0, M2_UNDEAD,
	  PHYS(5,0),	DFNS(AD_BLND),	NO_CARY,	0, A_LAWFUL, NON_PM, NON_PM, 1500L ),

	/*
	 *	Ah, never shall I forget the cry,
	 *		or the shriek that shrieked he,
	 *	As I gnashed my teeth, and from my sheath
	 *		I drew my Snickersnee!
	 *			--Koko, Lord high executioner of Titipu
	 *			  (From Sir W.S. Gilbert's "The Mikado")
	 */
	A(ART_SNICKERSNEE, "Snickersnee",                KATANA,
	  SPFX_RESTR, 0, 0,
	  PHYS(0,8),	NO_DFNS,	NO_CARY,	0, A_LAWFUL, PM_SAMURAI, NON_PM, 1200L ),

	/* KMH -- Renamed from Holy Sword of Law (Stephen White)
	 * This is an actual sword used in British coronations!
	 */
	A(ART_SWORD_OF_JUSTICE, "Sword of Justice",           LONG_SWORD,
	  (SPFX_RESTR|SPFX_DALIGN), 0, 0,
	  PHYS(5,12),     NO_DFNS,        NO_CARY,        0, A_LAWFUL, PM_YEOMAN, NON_PM, 1500L ),

	A(ART_DEMONBANE, "Demonbane",			LONG_SWORD,
	  (SPFX_RESTR|SPFX_DFLAG2), 0, M2_DEMON,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_LAWFUL, NON_PM, NON_PM, 2500L ),

	A(ART_WEREBANE, "Werebane",			SILVER_SABER,
	  (SPFX_RESTR|SPFX_DFLAG2), 0, M2_WERE,
	  PHYS(5,0),	DFNS(AD_WERE),	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 1500L ),

	A(ART_GRAYSWANDIR, "Grayswandir",		SILVER_SABER,
	  (SPFX_RESTR|SPFX_HALRES), 0, 0,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_LAWFUL, NON_PM, NON_PM, 8000L ),

	A(ART_SKULLCRUSHER, "Skullcrusher",               CLUB,
	  SPFX_RESTR, 0, 0,
	  PHYS(3,10),     NO_DFNS,        NO_CARY,        0, A_LAWFUL, PM_CAVEMAN, NON_PM, 300L ),

	A(ART_TROLLSBANE, "Trollsbane",                 MORNING_STAR,
	  (SPFX_RESTR|SPFX_DCLAS), 0, S_TROLL,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 200L ),

	A(ART_OGRESMASHER, "Ogresmasher",		WAR_HAMMER,
	  (SPFX_RESTR|SPFX_DCLAS), 0, S_OGRE,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 200L ),

	A(ART_REAPER, "Reaper",                     HALBERD,
	  SPFX_RESTR, 0, 0,
	  PHYS(5,20),      NO_DFNS,        NO_CARY,        0, A_LAWFUL, PM_YEOMAN, NON_PM, 1000L ),

	A(ART_HOLY_SPEAR_OF_LIGHT, "Holy Spear of Light",        SILVER_SPEAR,
	  (SPFX_RESTR|SPFX_INTEL|SPFX_DFLAG2), 0, M2_UNDEAD,
	  PHYS(5,10),      NO_DFNS,  NO_CARY,     LIGHT_AREA, A_LAWFUL, NON_PM, NON_PM, 4000L ),


	/*** Neutral artifacts ***/
	A(ART_MAGICBANE, "Magicbane",                  ATHAME,
	  (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	  STUN(3,4),	DFNS(AD_MAGM),	NO_CARY,	0, A_NEUTRAL, PM_WIZARD, NON_PM, 3500L ),

	A(ART_LUCKBLADE, "Luckblade",                  SHORT_SWORD,
	  (SPFX_RESTR|SPFX_LUCK), 0, 0,
	  PHYS(5,5),      NO_DFNS,        NO_CARY,        0, A_NEUTRAL, NON_PM, PM_GNOME, 1000L ),

	A(ART_SWORD_OF_BALANCE, "Sword of Balance",           SILVER_SHORT_SWORD,
	  (SPFX_RESTR|SPFX_DALIGN), 0, 0,
	  PHYS(2,5),      NO_DFNS,        NO_CARY,        0, A_NEUTRAL, NON_PM, NON_PM, 5000L ),

	A(ART_FROST_BRAND, "Frost Brand",                LONG_SWORD,
	  (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	  COLD(5,0),	COLD(0,0),	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 3000L ),

	A(ART_FIRE_BRAND, "Fire Brand",                 LONG_SWORD,
	  (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	  FIRE(5,0),	FIRE(0,0),	NO_CARY,	0, A_NONE, NON_PM, NON_PM, 3000L ),

	/*
	 *	Two problems:  1) doesn't let trolls regenerate heads,
	 *	2) doesn't give unusual message for 2-headed monsters (but
	 *	allowing those at all causes more problems than worth the effort).
	 */
	A(ART_VORPAL_BLADE, "Vorpal Blade",		LONG_SWORD,
	  (SPFX_RESTR|SPFX_BEHEAD), 0, 0,
	  PHYS(5,1),	NO_DFNS,	NO_CARY,	0, A_NEUTRAL, NON_PM, NON_PM, 4000L ),

	A(ART_DISRUPTER, "Disrupter",                  MACE,
	  (SPFX_RESTR|SPFX_DFLAG2), 0, M2_UNDEAD,
	  PHYS(5,30),     NO_DFNS,        NO_CARY,        0, A_NEUTRAL, PM_PRIEST, NON_PM, 500L ),

	/*
	 *	Mjollnir will return to the hand of a Valkyrie when thrown
	 *	if the wielder is a Valkyrie with strength of 25 or more.
	 */
	A(ART_MJOLLNIR, "Mjollnir",                   HEAVY_HAMMER,           /* Mjo:llnir */
	  (SPFX_RESTR|SPFX_ATTK),  0, 0,
	  ELEC(5,24),	NO_DFNS,	NO_CARY,	0, A_NEUTRAL, PM_VALKYRIE, NON_PM, 4000L ),

	/* STEPHEN WHITE'S NEW CODE */
	A(ART_GAUNTLETS_OF_DEFENSE, "Gauntlets of Defense",    GAUNTLETS_OF_DEXTERITY,
	  SPFX_RESTR, SPFX_HPHDAM, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,    INVIS, A_NEUTRAL, PM_MONK, NON_PM, 5000L ),

	A(ART_MIRRORBRIGHT, "Mirrorbright",               SHIELD_OF_REFLECTION,
	  (SPFX_RESTR|SPFX_HALRES|SPFX_REFLECT), 0, 0,
	  NO_ATTK,      NO_DFNS,        NO_CARY,        0, A_NEUTRAL, PM_HEALER, NON_PM, 5000L ),

	A(ART_DELUDER, "Deluder",               CLOAK_OF_DISPLACEMENT,
	  (SPFX_RESTR|SPFX_STLTH|SPFX_LUCK), 0, 0,
	  NO_ATTK,      NO_DFNS,        NO_CARY,        0, A_NEUTRAL, PM_WIZARD, NON_PM, 5000L ),

	A(ART_WHISPERFEET, "Whisperfeet",               SPEED_BOOTS,
	  (SPFX_RESTR|SPFX_STLTH|SPFX_LUCK), 0, 0,
	  NO_ATTK,      NO_DFNS,        NO_CARY,        0, A_NEUTRAL, PM_TOURIST, NON_PM, 5000L ),

	/*** Chaotic artifacts ***/
	A(ART_GRIMTOOTH, "Grimtooth",                  ORCISH_DAGGER,
	  SPFX_RESTR, 0, 0,
	  PHYS(2,6),	NO_DFNS,	NO_CARY,	0, A_CHAOTIC, NON_PM, PM_ORC, 300L ),

	A(ART_DEEP_FREEZE, "Deep Freeze",                ATHAME,
	  (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN), 0, 0,
	  COLD(5,5),      COLD(0,0),      NO_CARY,        0, A_CHAOTIC, PM_ICE_MAGE, NON_PM, 400L ),


	A(ART_SERPENT_S_TONGUE, "Serpent's Tongue",            DAGGER,
	  SPFX_RESTR, 0, 0,
	  PHYS(2,0),      NO_DFNS,        NO_CARY,        0, A_CHAOTIC, PM_NECROMANCER, NON_PM, 400L ),
	/* See artifact.c for special poison damage */

	A(ART_CLEAVER, "Cleaver",                    BATTLE_AXE,
	  SPFX_RESTR, 0, 0,
	  PHYS(3,6),	NO_DFNS,	NO_CARY,	0, A_NEUTRAL, PM_BARBARIAN, NON_PM, 1500L ),

	A(ART_DOOMBLADE, "Doomblade",                  ORCISH_SHORT_SWORD,
	  SPFX_RESTR, 0, 0,
	  PHYS(0,10),     NO_DFNS,        NO_CARY,        0, A_CHAOTIC, PM_HUMAN_WEREWOLF, NON_PM, 1000L ),

	/*
	 *	Stormbringer only has a 2 because it can drain a level,
	 *	providing 8 more.
	 */
	A(ART_STORMBRINGER, "Stormbringer",               RUNESWORD,
	  (SPFX_RESTR|SPFX_ATTK|SPFX_DEFN|SPFX_INTEL|SPFX_DRLI), 0, 0,
	  DRLI(5,2),	DRLI(0,0),	NO_CARY,	0, A_CHAOTIC, NON_PM, NON_PM, 8000L ),

	A(ART_THIEFBANE, "Thiefbane",                  LONG_SWORD,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_BEHEAD|SPFX_DCLAS|SPFX_DRLI), 0, S_HUMAN,
	  DRLI(5,1),      NO_DFNS,        NO_CARY,        0, A_CHAOTIC, NON_PM, NON_PM, 1500L ),

	A(ART_DEATHSWORD, "Deathsword",                   TWO_HANDED_SWORD,
	  (SPFX_RESTR|SPFX_DFLAG2), 0, M2_HUMAN,
	  PHYS(5,14),      NO_DFNS,        NO_CARY,        0, A_CHAOTIC, PM_BARBARIAN, NON_PM, 5000L ),

	A(ART_BAT_FROM_HELL, "Bat from Hell",                BASEBALL_BAT,
	  (SPFX_RESTR), 0, 0,
	  PHYS(3,20),      NO_DFNS,        NO_CARY,        0, A_CHAOTIC, PM_ROGUE, NON_PM, 5000L ),

	A(ART_ELFRIST, "Elfrist",                    ORCISH_SPEAR,
	  SPFX_DFLAG2, 0, M2_ELF,
	  PHYS(5,15),     NO_DFNS,        NO_CARY,        0, A_CHAOTIC, PM_HUMAN_WEREWOLF, PM_ORC, 300L ),

	A(ART_PLAGUE, "Plague", DARK_ELVEN_BOW,	/* KMH */
	  (SPFX_RESTR|SPFX_DEFN), 0, 0,
	  PHYS(5,7),        DFNS(AD_DRST),  NO_CARY,        0, A_CHAOTIC, PM_DROW, NON_PM, 6000L ),
	/* Auto-poison code in dothrow.c */

	A(ART_HELLFIRE, "Hellfire", CROSSBOW,	/* WAC */
	  (SPFX_RESTR|SPFX_DEFN), 0, 0,
	  PHYS(5,7),        FIRE(0,0),  NO_CARY,        0, A_CHAOTIC, NON_PM, NON_PM, 4000L ),
	/* Auto-explode code in dothrow.c, uhitm.c */

	A(ART_HOUCHOU, "Houchou",                SPOON,
	  (SPFX_RESTR), 0, 0,
	  NO_ATTK,      NO_DFNS,        NO_CARY,        0, A_CHAOTIC, NON_PM, NON_PM, 50000L ),


	/*** Special Artifacts ***/

	/* KMH -- made it a bag of holding */
	A(ART_WALLET_OF_PERSEUS, "Wallet of Perseus",       BAG_OF_HOLDING,
	  (SPFX_RESTR), 0, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,
	  0,      A_NONE, NON_PM, NON_PM, 10000L ),

	A(ART_NIGHTHORN, "Nighthorn", UNICORN_HORN,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_REFLECT), 0, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,
	  0,      A_LAWFUL, NON_PM, NON_PM, 10000L ),

	A(ART_KEY_OF_LAW, "The Key of Law", SKELETON_KEY,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), 0, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,
	  0,      A_LAWFUL, NON_PM, NON_PM, 1000L ),

	A(ART_EYE_OF_THE_BEHOLDER, "The Eye of the Beholder", EYEBALL,
	  (SPFX_NOGEN|SPFX_RESTR), 0, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,
	  DEATH_GAZE,     A_NEUTRAL, NON_PM, NON_PM, 500L ),

	A(ART_KEY_OF_NEUTRALITY, "The Key of Neutrality", SKELETON_KEY,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), 0, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,
	  0,      A_NEUTRAL, NON_PM, NON_PM, 1000L ),

	A(ART_HAND_OF_VECNA, "The Hand of Vecna",       SEVERED_HAND,
	  (SPFX_NOGEN|SPFX_RESTR), (SPFX_REGEN|SPFX_HPHDAM), 0,
	  NO_ATTK,        DRLI(0,0),      CARY(AD_COLD),
	  SUMMON_UNDEAD,          A_CHAOTIC, NON_PM, NON_PM, 700L ),

	A(ART_KEY_OF_CHAOS, "The Key of Chaos", SKELETON_KEY,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), 0, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,
	  0,      A_CHAOTIC, NON_PM, NON_PM, 1000L ),


	/*** The artifacts for the quest dungeon, all self-willed ***/

	A(ART_ORB_OF_DETECTION, "The Orb of Detection",	CRYSTAL_BALL,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_ESP|SPFX_HSPDAM), 0,
	  NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	  INVIS,		A_LAWFUL, PM_ARCHEOLOGIST, NON_PM, 2500L ),

	A(ART_HEART_OF_AHRIMAN, "The Heart of Ahriman",	LUCKSTONE,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), SPFX_STLTH, 0,
	  /* this stone does double damage if used as a projectile weapon */
	  PHYS(5,0),	NO_DFNS,	NO_CARY,
	  LEVITATION,	A_NEUTRAL, PM_BARBARIAN, NON_PM, 2500L ),

	A(ART_SCEPTRE_OF_MIGHT, "The Sceptre of Might",	MACE,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DALIGN), 0, 0,
	  PHYS(3,5),      NO_DFNS,        CARY(AD_MAGM),
	  CONFLICT,	A_LAWFUL, PM_CAVEMAN, NON_PM, 2500L ),

#if 0	/* OBSOLETE */
	A(ART_PALANTIR_OF_WESTERNESSE, "The Palantir of Westernesse",	CRYSTAL_BALL,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL),
	  (SPFX_ESP|SPFX_REGEN|SPFX_HSPDAM), 0,
	  NO_ATTK,	NO_DFNS,	NO_CARY,
	  TAMING,		A_CHAOTIC, NON_PM, PM_ELF, 8000L ),
#endif

	/* STEPHEN WHITE'S NEW CODE */
	A(ART_CANDLE_OF_ETERNAL_FLAME, "The Candle of Eternal Flame",        MAGIC_CANDLE,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_WARN|SPFX_TCTRL), 0,
	  NO_ATTK,        NO_DFNS,        CARY(AD_COLD),
	  SUMMON_FIRE_ELEMENTAL,         A_NEUTRAL, PM_FLAME_MAGE, NON_PM, 50000L ),

	A(ART_STAFF_OF_AESCULAPIUS, "The Staff of Aesculapius",	QUARTERSTAFF,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_INTEL|SPFX_DRLI|SPFX_REGEN), 0,0,
	  DRLI(3,0),      NO_DFNS,        NO_CARY,
	  HEALING,	A_NEUTRAL, PM_HEALER, NON_PM, 5000L ),
	/* STEPHEN WHITE'S NEW CODE */
	A(ART_STORM_WHISTLE, "The Storm Whistle",          MAGIC_WHISTLE,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_WARN|SPFX_TCTRL), 0,
	  NO_ATTK,        NO_DFNS,        CARY(AD_FIRE),
	  SUMMON_WATER_ELEMENTAL,         A_LAWFUL, PM_ICE_MAGE, NON_PM, 1000L ),

	A(ART_MAGIC_MIRROR_OF_MERLIN, "The Magic Mirror of Merlin", MIRROR,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK), SPFX_ESP, 0,
	  NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	  0,		A_LAWFUL, PM_KNIGHT, NON_PM, 1500L ),

	A(ART_EYES_OF_THE_OVERWORLD, "The Eyes of the Overworld",	LENSES,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_XRAY), 0, 0,
	  NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	  ENLIGHTENING,	A_NEUTRAL,	 PM_MONK, NON_PM, 2500L ),

	A(ART_GREAT_DAGGER_OF_GLAURGNAA, "The Great Dagger of Glaurgnaa",       GREAT_DAGGER,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_ATTK|SPFX_INTEL|SPFX_DRLI|SPFX_DALIGN), 0, 0,
	  DRLI(8,4),      NO_DFNS,        CARY(AD_MAGM),
	  ENERGY_BOOST,   A_CHAOTIC, PM_NECROMANCER, NON_PM, 50000L ),

	A(ART_MITRE_OF_HOLINESS, "The Mitre of Holiness",	HELM_OF_BRILLIANCE,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_DCLAS|SPFX_INTEL), 0, M2_UNDEAD,
	  NO_ATTK,	NO_DFNS,	CARY(AD_FIRE),
	  ENERGY_BOOST,	A_LAWFUL, PM_PRIEST, NON_PM, 2000L ),

	A(ART_LONGBOW_OF_DIANA, "The Longbow of Diana", BOW,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_REFLECT), SPFX_ESP, 0,
	  PHYS(5,0),	NO_DFNS,	NO_CARY,
	  CREATE_AMMO, A_CHAOTIC, PM_RANGER, NON_PM, 4000L ),

	A(ART_MASTER_KEY_OF_THIEVERY, "The Master Key of Thievery", SKELETON_KEY,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_SPEAK),
	  (SPFX_WARN|SPFX_TCTRL|SPFX_HPHDAM), 0,
	  NO_ATTK,	NO_DFNS,	NO_CARY,
	  UNTRAP,		A_CHAOTIC, PM_ROGUE, NON_PM, 3500L ),

	A(ART_TSURUGI_OF_MURAMASA, "The Tsurugi of Muramasa",	TSURUGI,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_BEHEAD|SPFX_LUCK), 0, 0,
	  NO_ATTK,        NO_DFNS,        NO_CARY,
	  0,		A_LAWFUL, PM_SAMURAI, NON_PM, 4500L ),

	A(ART_YENDORIAN_EXPRESS_CARD, "The Platinum Yendorian Express Card", CREDIT_CARD,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_DEFN),
	  (SPFX_ESP|SPFX_HSPDAM), 0,
	  NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	  CHARGE_OBJ,	A_NEUTRAL, PM_TOURIST, NON_PM, 7000L ),

	/* KMH -- More effective against normal monsters
	 * Was +10 to-hit, +d20 damage only versus vampires
	 */
	A(ART_STAKE_OF_VAN_HELSING, "The Stake of Van Helsing",        WOODEN_STAKE,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), 0, 0,
	  PHYS(5,12),    NO_DFNS,        CARY(AD_MAGM),
	  0,              A_LAWFUL, PM_UNDEAD_SLAYER, NON_PM, 5000L ),

	A(ART_ORB_OF_FATE, "The Orb of Fate",		CRYSTAL_BALL,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL|SPFX_LUCK),
	  (SPFX_WARN|SPFX_HSPDAM|SPFX_HPHDAM), 0,
	  NO_ATTK,	NO_DFNS,	NO_CARY,
	  LEV_TELE,	A_NEUTRAL, PM_VALKYRIE, NON_PM, 3500L ),

	A(ART_EYE_OF_THE_AETHIOPICA, "The Eye of the Aethiopica",	AMULET_OF_ESP,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_EREGEN|SPFX_HSPDAM), 0,
	  NO_ATTK,	NO_DFNS,	CARY(AD_MAGM),
	  CREATE_PORTAL,	A_NEUTRAL, PM_WIZARD, NON_PM, 4000L ),

	A(ART_CROWN_OF_SAINT_EDWARD, "The Crown of Saint Edward", HELM_OF_TELEPATHY,
	  (SPFX_NOGEN|SPFX_RESTR|SPFX_INTEL), (SPFX_HSPDAM), 0,
	  NO_ATTK,        NO_DFNS,        CARY(AD_MAGM),
	  0,  A_LAWFUL, PM_YEOMAN, NON_PM, 5000L ),

	/*
	 *  terminator; otyp must be zero
	 */
	A(NROFARTIFACTS = ART_CROWN_OF_SAINT_EDWARD, NULL, 0, 0, 0, 0, NO_ATTK, NO_DFNS, NO_CARY, 0, A_NONE, NON_PM, NON_PM, 0L )

};	/* artilist[] (or artifact_names[]) */

#undef A
#undef NO_ATTK
#undef NO_DFNS
#undef DFNS
#undef PHYS
#undef DRLI
#undef COLD
#undef FIRE
#undef ELEC
#undef STUN

#ifndef ARTILIST_PASS2
# undef ARTILIST_H // to force inclusion

# define ARTILIST_PASS2
# include "artilist.h"
# define ARTILIST_H
#endif

#endif //ARTILIST_H

/*artilist.h*/
