/*	SCCS Id: @(#)patchlevel.h	3.4	2003/12/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*The name of the compiled game- should be same as stuff in makefile*/
/*for makedefs*/
/* KMH -- Made it mixed case, from which upper & lower case versions are made */
#define DEF_GAME_NAME   "SlashEM-Next"

/* Version */
#define VERSION_MAJOR   0
#define VERSION_MINOR   0
/*
 * PATCHLEVEL is updated for each release.
 */
#define PATCHLEVEL      0
#define EDITLEVEL	999
#define FIXLEVEL        0

#define COPYRIGHT_BANNER_A "This is SuperLotsoAddedStuffHack-Extended Magic-Next 2017-2019"
#define COPYRIGHT_BANNER_B "SuperLotsoAddedStuffHack-Extended Magic 1997-2007"
#define COPYRIGHT_BANNER_C "NetHack, Copyright 1985-2003 Stichting Mathematisch Centrum, M. Stephenson."
#define COPYRIGHT_BANNER_D "See license for details.  Bug reports to Moonchild."


#define VERSION_NUMBER \
	 ((VERSION_MAJOR << 24) \
	| (VERSION_MINOR << 16) \
	| (PATCHLEVEL    << 8) \
	| EDITLEVEL)

#define VERSION_SANITY \
	  ((sizeof(struct flag)  << 24) \
	 | (sizeof(struct obj)   << 17) \
	 | (sizeof(struct monst) << 10) \
	 | (sizeof(struct you)))

/*patchlevel.h*/
