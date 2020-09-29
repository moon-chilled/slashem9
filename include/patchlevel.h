/*	SCCS Id: @(#)patchlevel.h	3.4	2003/12/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/*The name of the compiled game- should be same as stuff in makefile*/
/*for makedefs*/
/* KMH -- Made it mixed case, from which upper & lower case versions are made */
#define DEF_GAME_NAME "SlashEM9"

/* Version */
#define VERSION_NUM 8
/* EDITLEVEL is updated for each release. */
#define VERSION_EDITLEVEL  0

#define COPYRIGHT_BANNER_A "This is SuperLotsoAddedStuffHack-Extended Magic Nine 2017-2020"
#define COPYRIGHT_BANNER_B "SuperLotsoAddedStuffHack-Extended Magic 1997-2007"
#define COPYRIGHT_BANNER_C "NetHack, Copyright 1985-2003 Stichting Mathematisch Centrum, M. Stephenson."
#define COPYRIGHT_BANNER_D "See license for details.  Bug reports to Moonchild."

#define VERSION_NUMBER \
	((VERSION_NUM << 16) | VERSION_EDITLEVEL)

#define VERSION_SANITY \
	((sizeof(struct flag) << 24) | (sizeof(struct obj) << 16) | (sizeof(struct monst) << 8) | (sizeof(struct you)))

/*patchlevel.h*/
