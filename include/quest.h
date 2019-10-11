/*	SCCS Id: @(#)quest.h	3.4	1992/11/15	*/
/* Copyright (c) Mike Stephenson 1991.				  */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef QUEST_H
#define QUEST_H

struct q_score {			/* Quest "scorecard" */
	bool first_start;	/* only set the first time */
	bool met_leader;		/* has met the leader */
	Bitfield(not_ready,3);		/* rejected due to alignment, etc. */
	bool pissed_off;		/* got the leader angry */
	bool got_quest;		/* got the quest assignment */

	bool first_locate;	/* only set the first time */
	bool met_intermed;	/* used if the locate is a person. */
	bool got_final;		/* got the final quest assignment */

	Bitfield(made_goal,3);		/* # of times on goal level */
	bool met_nemesis;	/* has met the nemesis before */
	bool killed_nemesis;	/* set when the nemesis is killed */
	bool in_battle;		/* set when nemesis fighting you */

	bool cheater;		/* set if cheating detected */
	bool touched_artifact;	/* for a special message */
	bool offered_artifact;	/* offered to leader */
	bool got_thanks;		/* final message from leader */

	/* keep track of leader presence/absence even if leader is
	   polymorphed, raised from dead, etc */
	bool leader_is_dead;
	unsigned leader_m_id;
};

/* KMH, balance patch -- These were 7, 20, 14 for NetHack and 1, 20, 6 for Slash */
#define MAX_QUEST_TRIES  7	/* exceed this and you "fail" */
#define MIN_QUEST_ALIGN 20	/* at least this align.record to start */
/* note: align 20 matches "pious" as reported by enlightenment (cmd.c) */
#define MIN_QUEST_LEVEL	10	/* at least this u.ulevel to start */
/* note: exp.lev. 14 is threshold level for 5th rank (class title, role.c) */

#endif /* QUEST_H */
