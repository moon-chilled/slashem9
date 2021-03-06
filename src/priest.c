/*	SCCS Id: @(#)priest.c	3.4	2002/11/06	*/
/* Copyright (c) Izchak Miller, Steve Linhart, 1989.		  */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "mfndpos.h"

/* this matches the categorizations shown by enlightenment */
#define ALGN_SINNED (-4) /* worse than strayed */

static boolean histemple_at(struct monst *, xchar, xchar);
static boolean has_shrine(struct monst *);

void newepri(struct monst *mtmp) {
	if (!mtmp->mextra) mtmp->mextra = newmextra();
	if (!EPRI(mtmp)) {
		EPRI(mtmp) = new(struct epri);
	}
}

void free_epri(struct monst *mtmp) {
       if (mtmp->mextra && EPRI(mtmp)) {
               free(EPRI(mtmp));
               EPRI(mtmp) = NULL;
       }
       mtmp->ispriest = false;
}

/*
 * Move for priests and shopkeepers.  Called from shk_move() and pri_move().
 * Valid returns are  1: moved  0: didn't  -1: let m_move do it  -2: died.
 */
int move_special(struct monst *mtmp, boolean in_his_shop, schar appr, boolean uondoor, boolean avoid, xchar omx, xchar omy, xchar gx, xchar gy) {
	xchar nx, ny, nix, niy;
	schar i;
	schar chcnt, cnt;
	coord poss[9];
	long info[9];
	long allowflags;
	struct obj *ib = NULL;

	if (omx == gx && omy == gy)
		return 0;
	if (mtmp->mconf) {
		avoid = false;
		appr = 0;
	}

	nix = omx;
	niy = omy;
	if (mtmp->isshk)
		allowflags = ALLOW_SSM;
	else
		allowflags = ALLOW_SSM | ALLOW_SANCT;
	if (passes_walls(mtmp->data)) allowflags |= (ALLOW_ROCK | ALLOW_WALL);
	if (throws_rocks(mtmp->data)) allowflags |= ALLOW_ROCK;
	if (tunnels(mtmp->data)) allowflags |= ALLOW_DIG;
	if (!nohands(mtmp->data) && !verysmall(mtmp->data)) {
		allowflags |= OPENDOOR;
		if (m_carrying(mtmp, SKELETON_KEY)) allowflags |= BUSTDOOR;
	}
	if (is_giant(mtmp->data)) allowflags |= BUSTDOOR;
	cnt = mfndpos(mtmp, poss, info, allowflags);

	if (mtmp->isshk && avoid && uondoor) { /* perhaps we cannot avoid him */
		for (i = 0; i < cnt; i++)
			if (!(info[i] & NOTONL)) goto pick_move;
		avoid = false;
	}

#define GDIST(x, y) (dist2(x, y, gx, gy))
pick_move:
	chcnt = 0;
	for (i = 0; i < cnt; i++) {
		nx = poss[i].x;
		ny = poss[i].y;
		if (levl[nx][ny].typ == ROOM ||
		    (mtmp->ispriest &&
		     levl[nx][ny].typ == ALTAR) ||
		    (mtmp->isshk &&
		     (!in_his_shop || ESHK(mtmp)->following))) {
			if (avoid && (info[i] & NOTONL))
				continue;
			if ((!appr && !rn2(++chcnt)) ||
			    (appr && GDIST(nx, ny) < GDIST(nix, niy))) {
				nix = nx;
				niy = ny;
			}
		}
	}
	if (mtmp->ispriest && avoid &&
	    nix == omx && niy == omy && onlineu(omx, omy)) {
		/* might as well move closer as long it's going to stay
		 * lined up */
		avoid = false;
		goto pick_move;
	}

	if (nix != omx || niy != omy) {
		remove_monster(omx, omy);
		place_monster(mtmp, nix, niy);
		newsym(nix, niy);
		if (mtmp->isshk && !in_his_shop && inhishop(mtmp))
			check_special_room(false);
		if (ib) {
			if (cansee(mtmp->mx, mtmp->my))
				pline("%s picks up %s.", Monnam(mtmp),
				      distant_name(ib, doname));
			obj_extract_self(ib);
			mpickobj(mtmp, ib);
		}
		return 1;
	}
	return 0;
}

char temple_occupied(char *array) {
	char *ptr;

	for (ptr = array; *ptr; ptr++)
		if (rooms[*ptr - ROOMOFFSET].rtype == TEMPLE)
			return *ptr;
	return '\0';
}

static boolean histemple_at(struct monst *priest, xchar x, xchar y) {
	return (EPRI(priest)->shroom == *in_rooms(x, y, TEMPLE)) &&
	       on_level(&(EPRI(priest)->shrlevel), &u.uz);
}

/*
 * pri_move: return 1: moved  0: didn't  -1: let m_move do it  -2: died
 */
int pri_move(struct monst *priest) {
	xchar gx, gy, omx, omy;
	schar temple;
	boolean avoid = true;

	omx = priest->mx;
	omy = priest->my;

	if (!histemple_at(priest, omx, omy)) return -1;

	temple = EPRI(priest)->shroom;

	gx = EPRI(priest)->shrpos.x;
	gy = EPRI(priest)->shrpos.y;

	gx += rn1(3, -1); /* mill around the altar */
	gy += rn1(3, -1);

	if (!priest->mpeaceful ||
	    (Conflict && !resist(priest, RING_CLASS, 0, 0))) {
		if (monnear(priest, u.ux, u.uy)) {
			if (Displaced)
				pline("Your displaced image doesn't fool %s!",
				      mon_nam(priest));
			mattacku(priest);
			return 0;
		} else if (index(u.urooms, temple)) {
			/* chase player if inside temple & can see him */
			if (priest->mcansee && m_canseeu(priest)) {
				gx = u.ux;
				gy = u.uy;
			}
			avoid = false;
		}
	} else if (Invis)
		avoid = false;

	return move_special(priest, false, true, false, avoid, omx, omy, gx, gy);
}

/* exclusively for mktemple() */
void priestini(d_level *lvl, struct mkroom *sroom, int sx, int sy, boolean sanctum) {
	struct monst *priest;
	struct obj *otmp = NULL;
	int cnt;

	if (MON_AT(sx + 1, sy))
		rloc(m_at(sx + 1, sy), false); /* insurance */

	priest = makemon(&mons[sanctum ? PM_HIGH_PRIEST : PM_ALIGNED_PRIEST], sx + 1, sy, MM_EPRI);
	if (priest) {
		EPRI(priest)->shroom = (sroom - rooms) + ROOMOFFSET;
		EPRI(priest)->shralign = Amask2align(levl[sx][sy].altarmask);
		EPRI(priest)->shrpos.x = sx;
		EPRI(priest)->shrpos.y = sy;
		assign_level(&(EPRI(priest)->shrlevel), lvl);
		priest->mtrapseen = ~0; /* traps are known */
		priest->mpeaceful = 1;
		priest->ispriest = 1;
		priest->isminion = 0;
		priest->msleeping = 0;
		set_malign(priest); /* mpeaceful may have changed */

		/* now his/her goodies... */
		if (sanctum && EPRI(priest)->shralign == A_NONE &&
		    on_level(&sanctum_level, &u.uz)) {
			mongets(priest, AMULET_OF_YENDOR);
		}
		/* 2 to 4 spellbooks */
		for (cnt = rn1(3, 2); cnt > 0; --cnt) {
			mpickobj(priest, mkobj(SPBOOK_CLASS, false));
		}
		/* [ALI] Upgrade existing robe or aquire new */
		if (rn2(2) || (otmp = which_armor(priest, W_ARM)) == 0) {
			struct obj *obj;
			obj = mksobj(rn2(p_coaligned(priest) ? 2 : 5) ?
					     ROBE_OF_PROTECTION :
					     ROBE_OF_POWER,
				     true, false);
			if (p_coaligned(priest))
				uncurse(obj);
			else
				curse(obj);
			mpickobj(priest, obj);
			m_dowear(priest, true);
			if (!(obj->owornmask & W_ARM)) {
				obj_extract_self(obj);
				obfree(obj, NULL);
			} else if (otmp) {
				obj_extract_self(otmp);
				obfree(otmp, NULL);
			}
		}
	}
}

/* get a monster's alignment type without caller needing EPRI & EMIN */
aligntyp mon_aligntyp(struct monst *mon) {
	aligntyp algn = mon->ispriest ? EPRI(mon)->shralign :
			mon->isminion ? EMIN(mon)->min_align :
			mon->data->maligntyp;

	if (algn == A_NONE)
		return A_NONE; /* negative but differs from chaotic */

	return (algn > 0) ? A_LAWFUL :
	       (algn < 0) ? A_CHAOTIC :
	       A_NEUTRAL;
}


/*
 * Specially aligned monsters are named specially.
 *	- aligned priests with ispriest and high priests have shrines
 *		they retain ispriest and epri when polymorphed
 *	- aligned priests without ispriest and Angels are roamers
 *		they have isminion set and access epri as emin
 *	- minions do not have ispriest but have isminion and emin
 *	- caller needs to inhibit Hallucination if it wants to force
 *		the true name even when under that influence
 */
char *priestname(struct monst *mon, char *pname) {
	bool aligned_priest = mon->data == &mons[PM_ALIGNED_PRIEST],
	     high_priest = mon->data == &mons[PM_HIGH_PRIEST];

	const char *what = Hallucination ? rndmonnam() : mon->data->mname;

	if (!mon->ispriest && !mon->isminion) {
		impossible("Attempted to priestname() a non-priest");
		return strcpy(pname, what);
	}

	strcpy(pname, "the ");
	if (mon->minvis) strcat(pname, "invisible ");
	if (mon->isminion && EMIN(mon)->renegade)
		strcat(pname, "renegade ");

	if (mon->ispriest || aligned_priest) {  /* high_priest implies ispriest */
		if (!aligned_priest && !high_priest) {
			;   /* polymorphed priest; use ``what'' as is */
		} else {
			if (high_priest)
				strcat(pname, "high ");
			if (Hallucination)
				what = "poohbah";
			else if (mon->female)
				what = "priestess";
			else
				what = "priest";
		}
	} else {
		if (mon->mtame) {
			strcat(pname, "guardian ");
		}
	}

	strcat(pname, what);
	strcat(pname, " of ");
	strcat(pname, halu_gname(mon_aligntyp(mon)));
	return pname;
}

boolean p_coaligned(struct monst *priest) {
	return u.ualign.type == ((int)EPRI(priest)->shralign);
}

static boolean has_shrine(struct monst *pri) {
	struct rm *lev;

	if (!pri)
		return false;
	lev = &levl[EPRI(pri)->shrpos.x][EPRI(pri)->shrpos.y];
	if (!IS_ALTAR(lev->typ) || !(lev->altarmask & AM_SHRINE))
		return false;
	return EPRI(pri)->shralign == Amask2align(lev->altarmask & ~AM_SHRINE);
}

struct monst *
findpriest(char roomno) {
	struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if (mtmp->ispriest && (EPRI(mtmp)->shroom == roomno) &&
		    histemple_at(mtmp, mtmp->mx, mtmp->my))
			return mtmp;
	}
	return NULL;
}

/* called from check_special_room() when the player enters the temple room */
void intemple(int roomno) {
	struct monst *priest = findpriest((char)roomno);
	boolean tended = (priest != NULL);
	boolean shrined, sanctum, can_speak;
	const char *msg1, *msg2;
	char buf[BUFSZ];

	if (!temple_occupied(u.urooms0)) {
		if (tended) {
			shrined = has_shrine(priest);
			sanctum = (priest->data == &mons[PM_HIGH_PRIEST] &&
				   (Is_sanctum(&u.uz) || In_endgame(&u.uz)));
			can_speak = (priest->mcanmove && !priest->msleeping &&
				     !Deaf);
			if (can_speak) {
				unsigned save_priest = priest->ispriest;
				/* don't reveal the altar's owner upon temple entry in
				   the endgame; for the Sanctum, the next message names
				   Moloch so suppress the "of Moloch" for him here too */
				if (sanctum && !Hallucination) priest->ispriest = 0;
				pline("%s intones:",
				      canseemon(priest) ? Monnam(priest) : "A nearby voice");
				priest->ispriest = save_priest;
			}
			msg2 = 0;
			if (sanctum && Is_sanctum(&u.uz)) {
				if (priest->mpeaceful) {
					msg1 = "Infidel, you have entered Moloch's Sanctum!";
					msg2 = "Be gone!";
					priest->mpeaceful = 0;
					set_malign(priest);
				} else
					msg1 = "You desecrate this place by your presence!";
			} else {
				sprintf(buf, "Pilgrim, you enter a %s place!",
					!shrined ? "desecrated" : "sacred");
				msg1 = buf;
			}
			if (can_speak) {
				verbalizes(msg1);
				if (msg2) verbalizes(msg2);
			}
			if (!sanctum) {
				/* !tended -> !shrined */
				if (!shrined || !p_coaligned(priest) ||
				    u.ualign.record <= ALGN_SINNED)
					pline("You have a%s forbidding feeling...",
					      (!shrined) ? "" : " strange");
				else
					pline("You experience a strange sense of peace.");
			}
		} else {
			switch (rn2(3)) {
				case 0:
					pline("You have an eerie feeling...");
					break;
				case 1:
					pline("You feel like you are being watched.");
					break;
				default:
					pline("A shiver runs down your %s.",
					      body_part(SPINE));
					break;
			}
			if (!rn2(5)) {
				struct monst *mtmp;

				if (!(mtmp = makemon(&mons[PM_GHOST], u.ux, u.uy, NO_MM_FLAGS)))
					return;
				if (!Blind || sensemon(mtmp))
					pline("An enormous ghost appears next to you!");
				else
					pline("You sense a presence close by!");
				mtmp->mpeaceful = 0;
				set_malign(mtmp);
				if (flags.verbose)
					pline("You are frightened to death, and unable to move.");
				nomul(-3);
				nomovemsg = "You regain your composure.";
			}
		}
	}
}

void priest_talk(struct monst *priest) {
	boolean coaligned = p_coaligned(priest);
	boolean strayed = (u.ualign.record < 0);

	/* KMH, conduct */
	u.uconduct.gnostic++;

	if (priest->mflee || (!priest->ispriest && coaligned && strayed)) {
		pline("%s doesn't want anything to do with you!",
		      Monnam(priest));
		priest->mpeaceful = 0;
		return;
	}

	/* priests don't chat unless peaceful and in their own temple */
	if (!histemple_at(priest, priest->mx, priest->my) ||
	    !priest->mpeaceful || !priest->mcanmove || priest->msleeping) {
		static const char *const cranky_msg[] = {
			"Thou wouldst have words, eh?  I'll give thee a word or two!",
			"Talk?  Here is what I have to say!",
			"Pilgrim, I would speak no longer with thee."};

		if (!priest->mcanmove || priest->msleeping) {
			pline("%s breaks out of %s reverie!",
			      Monnam(priest), mhis(priest));
			priest->mfrozen = priest->msleeping = 0;
			priest->mcanmove = 1;
		}
		priest->mpeaceful = 0;
		verbalizes(cranky_msg[rn2(SIZE(cranky_msg))]);
		return;
	}

	/* you desecrated the temple and now you want to chat? */
	if (priest->mpeaceful && *in_rooms(priest->mx, priest->my, TEMPLE) &&
	    !has_shrine(priest)) {
		verbalize("Begone!  Thou desecratest this holy place with thy presence.");
		priest->mpeaceful = 0;
		return;
	}
	if (!money_cnt(invent)) {
		if (coaligned && !strayed) {
			long pmoney = money_cnt(priest->minvent);
			if (pmoney > 0L) {
				/* Note: two bits is actually 25 cents.  Hmm. */
				pline("%s gives you %s for an ale.", Monnam(priest),
				      (pmoney == 1L) ? "one bit" : "two bits");
				money2u(priest, pmoney > 1L ? 2 : 1);
			} else {
				pline("%s preaches the virtues of poverty.", Monnam(priest));
			}
			exercise(A_WIS, true);
		} else {
			pline("%s is not interested.", Monnam(priest));
		}
		return;
	} else {
		long offer;

		pline("%s asks you for a contribution for the temple.",
		      Monnam(priest));
		if ((offer = bribe(priest)) == 0) {
			verbalize("Thou shalt regret thine action!");
			if (coaligned) adjalign(-1);
		} else if (offer < (u.ulevel * 200)) {
			if (money_cnt(invent) > (offer * 2L))
				verbalize("Cheapskate.");
			else {
				verbalize("I thank thee for thy contribution.");
				/*  give player some token  */
				exercise(A_WIS, true);
			}
		} else if (offer < (u.ulevel * 400)) {
			verbalize("Thou art indeed a pious individual.");
			if (money_cnt(invent) < (offer * 2L)) {
				if (coaligned && u.ualign.record <= ALGN_SINNED)
					adjalign(1);
				verbalize("I bestow upon thee a blessing.");
				/* KMH, intrinsic patch */
				incr_itimeout(&HClairvoyant, rn1(500, 500));
			}
		} else if (offer < (u.ulevel * 600) &&
			   u.ublessed < 20 &&
			   (u.ublessed < 9 || !rn2(u.ublessed))) {
			verbalize("Thy devotion has been rewarded.");
			if (!(HProtection & INTRINSIC)) {
				HProtection |= FROMOUTSIDE;
				if (!u.ublessed) u.ublessed = rn1(3, 2);
			} else
				u.ublessed++;
		} else {
			verbalize("Thy selfless generosity is deeply appreciated.");
			if (money_cnt(invent) < (offer * 2L) && coaligned) {
				if (strayed && (moves - u.ucleansed) > 5000L) {
					u.ualign.record = 0; /* cleanse thee */
					u.ucleansed = moves;
				} else {
					adjalign(2);
				}
			}
		}
	}
}

struct monst *mk_roamer(struct permonst *ptr, aligntyp alignment, xchar x, xchar y, boolean peaceful) {
	struct monst *roamer;
	boolean coaligned = (u.ualign.type == alignment);

	/* Angels have the emin extension; aligned priests have the epri
	   extension, we access it as if it were emin */
	if (ptr != &mons[PM_ALIGNED_PRIEST] && ptr != &mons[PM_ANGEL])
		return NULL;

	if (MON_AT(x, y)) rloc(m_at(x, y), false); /* insurance */

	if (!(roamer = makemon(ptr, x, y, MM_ADJACENTOK | MM_EMIN)))
		return NULL;

	EMIN(roamer)->min_align = alignment;
	EMIN(roamer)->renegade = (coaligned && !peaceful);
	roamer->ispriest = 0;
	roamer->isminion = 1;

	roamer->mtrapseen = ~0;	 /* traps are known */
	roamer->mpeaceful = peaceful;
	roamer->msleeping = 0;
	set_malign(roamer); /* peaceful may have changed */

	/* MORE TO COME */
	return roamer;
}

void reset_hostility(struct monst *roamer) {
	if (!roamer->isminion) return;
	if (roamer->data != &mons[PM_ALIGNED_PRIEST] && roamer->data != &mons[PM_ANGEL]) return;

	if (EMIN(roamer)->min_align != u.ualign.type) {
		roamer->mpeaceful = false;
		roamer->mtame = 0;
		set_malign(roamer);
	}
	newsym(roamer->mx, roamer->my);
}

/* if mon is non-null, <mx,my> overrides <x,y> */
boolean in_your_sanctuary(struct monst *mon, xchar x, xchar y) {
	char roomno;
	struct monst *priest;

	if (mon) {
		if (is_minion(mon->data) || is_rider(mon->data)) return false;
		x = mon->mx, y = mon->my;
	}
	if (u.ualign.record <= ALGN_SINNED) /* sinned or worse */
		return false;
	if ((roomno = temple_occupied(u.urooms)) == 0 ||
	    roomno != *in_rooms(x, y, TEMPLE))
		return false;
	if ((priest = findpriest(roomno)) == 0)
		return false;
	return has_shrine(priest) &&
	       p_coaligned(priest) &&
	       priest->mpeaceful;
}

/* when attacking "priest" in his temple */
void ghod_hitsu(struct monst *priest) {
	int x, y, ax, ay, roomno = (int)temple_occupied(u.urooms);
	int x1, y1, x2, y2, n;
	coord poss[4];
	int stpx = sgn(u.ux - priest->mx), stpy = sgn(u.uy - priest->my);
	/* gods avoid hitting the temple priest */
	struct mkroom *troom;

	if (!roomno || !has_shrine(priest))
		return;

	ax = x = EPRI(priest)->shrpos.x;
	ay = y = EPRI(priest)->shrpos.y;
	troom = &rooms[roomno - ROOMOFFSET];

	/*
	 * Determine the source of the lightning bolt according to the
	 * following rules:
	 *	1. The source cannot be directly under the player
	 *	2. Don't zap through the temple priest
	 *	3. First choice of source is the altar itself
	 *	4. Otherwise use a wall, prefering orthogonal to diagonal paths
	 *	5. Choose randomly from equally preferred sources
	 * Note that if the hero is not standing on either the altar or
	 * a door then (u.ux, u.uy) may be counted as a possible source which
	 * is later rejected by linedup() letting the hero off the hook.
	 */
	if ((u.ux == x && u.uy == y) || !linedup(u.ux, u.uy, x, y) ||
	    (stpx == sgn(tbx) && stpy == sgn(tby))) {
		if (IS_DOOR(levl[u.ux][u.uy].typ)) {
			if (u.ux == troom->lx - 1) {
				if (stpx != sgn(u.ux - troom->hx) || stpy != 0) {
					x = troom->hx;
					y = u.uy;
				} else {
					/* Diagonal required */
					x1 = u.ux + u.uy - troom->ly;
					y1 = troom->ly;
					x2 = u.ux + troom->hy - u.uy;
					y2 = troom->hy;
					if (x1 > troom->hx && x2 > troom->hx)
						return;
					else if ((x2 > troom->hx || x1 <= troom->hx) && !rn2(2)) {
						x = x1;
						y = y1;
					} else {
						x = x2;
						y = y2;
					}
				}
			} else if (u.ux == troom->hx + 1) {
				if (stpx != sgn(u.ux - troom->lx) || stpy != 0) {
					x = troom->lx;
					y = u.uy;
				} else {
					/* Diagonal required */
					x1 = u.ux - (u.uy - troom->ly);
					y1 = troom->ly;
					x2 = u.ux - (troom->hy - u.uy);
					y2 = troom->hy;
					if (x1 < troom->lx && x2 < troom->lx)
						return;
					else if ((x2 < troom->lx || x1 >= troom->lx) && !rn2(2)) {
						x = x1;
						y = y1;
					} else {
						x = x2;
						y = y2;
					}
				}
			} else if (u.uy == troom->ly - 1) {
				if (stpx != 0 || stpy != sgn(u.uy - troom->hy)) {
					x = u.ux;
					y = troom->hy;
				} else {
					/* Diagonal required */
					x1 = troom->lx;
					y1 = u.uy + u.ux - troom->lx;
					x2 = troom->hx;
					y2 = u.uy + troom->hx - u.ux;
					if (y1 > troom->hy && y2 > troom->hy)
						return;
					else if ((y2 > troom->hy || y1 <= troom->hy) && !rn2(2)) {
						x = x1;
						y = y1;
					} else {
						x = x2;
						y = y2;
					}
				}
			} else if (u.uy == troom->hy + 1) {
				if (stpx != 0 || stpy != sgn(u.uy - troom->ly)) {
					x = u.ux;
					y = troom->ly;
				} else {
					/* Diagonal required */
					x1 = troom->lx;
					y1 = u.uy - (u.ux - troom->lx);
					x2 = troom->hx;
					y2 = u.uy - (troom->hx - u.ux);
					if (y1 < troom->ly && y2 < troom->ly)
						return;
					else if ((y2 < troom->ly || y1 >= troom->ly) && !rn2(2)) {
						x = x1;
						y = y1;
					} else {
						x = x2;
						y = y2;
					}
				}
			}
		} else {
			/* Calculate the possible orthogonal paths */
			n = 0;
			if (stpx != 0 || stpy != sgn(u.uy - troom->ly)) {
				poss[n].x = u.ux;
				poss[n++].y = troom->ly;
			}
			if (stpx != 0 || stpy != sgn(u.uy - troom->hy)) {
				poss[n].x = u.ux;
				poss[n++].y = troom->hy;
			}
			if (stpx != sgn(u.ux - troom->lx) || stpy != 0) {
				poss[n].x = troom->lx;
				poss[n++].y = u.uy;
			}
			if (stpx != sgn(u.ux - troom->hx) || stpy != 0) {
				poss[n].x = troom->hx;
				poss[n++].y = u.uy;
			}
			if (n) {
				n = rn2(n);
				x = poss[n].x;
				y = poss[n].y;
			} else {
				impossible("Omnipresent priest?");
				return;
			}
		}
		if (!linedup(u.ux, u.uy, x, y))
			return;
	}

	switch (rn2(3)) {
		case 0:
			pline("%s roars in anger:  \"Thou shalt suffer!\"",
			      a_gname_at(ax, ay));
			break;
		case 1:
			pline("%s voice booms:  \"How darest thou harm my servant!\"",
			      s_suffix(a_gname_at(ax, ay)));
			break;
		default:
			pline("%s roars:  \"Thou dost profane my shrine!\"",
			      a_gname_at(ax, ay));
			break;
	}

	buzz(-10 - (AD_ELEC - 1), 6, x, y, sgn(tbx), sgn(tby)); /* bolt of lightning */
	exercise(A_WIS, false);
}

void angry_priest() {
	struct monst *priest;
	struct rm *lev;

	if ((priest = findpriest(temple_occupied(u.urooms))) != 0) {
		wakeup(priest);
		/*
		 * If the altar has been destroyed or converted, let the
		 * priest run loose.
		 * (When it's just a conversion and there happens to be
		 *	a fresh corpse nearby, the priest ought to have an
		 *	opportunity to try converting it back; maybe someday...)
		 */
		lev = &levl[EPRI(priest)->shrpos.x][EPRI(priest)->shrpos.y];
		if (!IS_ALTAR(lev->typ) ||
		    ((aligntyp)Amask2align(lev->altarmask & AM_MASK) !=
		     EPRI(priest)->shralign)) {
			if (EPRI(priest)) {
				if (!EMIN(priest)) newemin(priest);
				priest->ispriest = 0;	// roamer
							// but still aligned
				priest->isminion = 1;
				EMIN(priest)->min_align = EPRI(priest)->shralign;
			}

			/* this used to overload EPRI's shroom field, which was then clobbered
			 * but not since adding the separate mextra structure */
			EMIN(priest)->renegade = 0;
		}
	}
}

/*
 * When saving bones, find priests that aren't on their shrine level,
 * and remove them.   This avoids big problems when restoring bones.
 * [Perhaps we could convert them into roamers instead?]
 */
void clearpriests(void) {
	for (struct monst *mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
		if (DEADMONSTER(mtmp)) continue;
		if (mtmp->ispriest && !on_level(&(EPRI(mtmp)->shrlevel), &u.uz))
			mongone(mtmp);
	}
}

/* munge priest-specific structure when restoring -dlc */
void restpriest(struct monst *mtmp, boolean ghostly) {
	if (u.uz.dlevel) {
		if (ghostly)
			assign_level(&(EPRI(mtmp)->shrlevel), &u.uz);
	}
}

/*priest.c*/
