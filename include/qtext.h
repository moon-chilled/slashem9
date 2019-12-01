/*	SCCS Id: @(#)qtext.h	3.4	1997/02/02	*/
/* Copyright (c) Mike Stephenson 1991.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef QTEXT_H
#define QTEXT_H

/*
 *	Quest message defines.	Used in quest.c to trigger off "realistic"
 *	dialogue to the player.
 */
#define QT_FIRSTTIME 1
#define QT_NEXTTIME  2
#define QT_OTHERTIME 3

#define QT_GUARDTALK  5	 /* 5 random things guards say before quest */
#define QT_GUARDTALK2 10 /* 5 random things guards say after quest */

#define QT_FIRSTLEADER 15
#define QT_NEXTLEADER  16
#define QT_OTHERLEADER 17
#define QT_LASTLEADER  18
#define QT_BADLEVEL    19
#define QT_BADALIGN    20
#define QT_ASSIGNQUEST 21

#define QT_ENCOURAGE 25 /* 1-10 random encouragement messages */

#define QT_FIRSTLOCATE 35
#define QT_NEXTLOCATE  36

#define QT_FIRSTGOAL 40
#define QT_NEXTGOAL  41

#define QT_FIRSTNEMESIS 50
#define QT_NEXTNEMESIS	51
#define QT_OTHERNEMESIS 52
#define QT_NEMWANTSIT	53 /* you somehow got the artifact */

#define QT_DISCOURAGE 60 /* 1-10 random maledictive messages */

#define QT_GOTIT 70

#define QT_KILLEDNEM  80
#define QT_OFFEREDIT  81
#define QT_OFFEREDIT2 82

#define QT_POSTHANKS 90
#define QT_HASAMULET 91

/*
 *	Message defines for common text used in maledictions.
 */
#define COMMON_ID "-" /* common message id value */

#define QT_ANGELIC  10
#define QTN_ANGELIC 10

#define QT_DEMONIC  30
#define QTN_DEMONIC 20

#define QT_BANISHED 60

#define QT_GYPSY 100 /* KMH -- Gypsy fortunes */

#define MAX_QT 101

struct qtmsg {
	int msgnum;
	char delivery;
	usize offset, size;
};

struct qtlists {
	struct qtmsg common[MAX_QT];
	struct qtmsg chrole[NUM_ROLES][MAX_QT];
};

#endif /* QTEXT_H */
