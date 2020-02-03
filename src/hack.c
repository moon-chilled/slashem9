/*	SCCS Id: @(#)hack.c	3.4	2003/04/30	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

static void maybe_wail(void);
static int moverock(void);
static int still_chewing(xchar, xchar);
static void dosinkfall(void);
static boolean findtravelpath(boolean);
static boolean monstinroom(struct permonst *, int);

static void move_update(boolean);

#define IS_SHOP(x) (rooms[x].rtype >= SHOPBASE)

//FIXME: this is currently used for destroying LS_TEMPs.  Which take the form of
//an int.  But del_light_source uses uint.  I doubt they get big enough that
//actually an issue but...
anything int_to_any(int i) {
	anything ret = {0};
	ret.a_int = i;
	return ret;
}
anything uint_to_any(uint ui) {
	anything ret = {0};
	ret.a_uint = ui;
	return ret;
}

anything monst_to_any(struct monst *mtmp) {
	anything ret = {0};
	ret.a_monst = mtmp;
	return ret;
}

anything obj_to_any(struct obj *obj) {
	anything ret = {0};
	ret.a_obj = obj;
	return ret;
}

anything long_to_any(long l) {
	anything ret = {0};
	ret.a_long = l;
	return ret;
}

// guaranteed to return a valid coord
void rndmappos(xchar *x, xchar *y) {
	if (*x >= COLNO)
		*x = COLNO;
	else if (*x == -1)
		*x = rn2(COLNO - 1) + 1;
	else if (*x < 1)
		*x = 1;

	if (*y >= ROWNO)
		*y = ROWNO;
	else if (*y == -1)
		*y = rn2(ROWNO);
	else if (*y < 0)
		*y = 0;
}

#define HERB_GROWTH_LIMIT 3 /* to limit excessive farming */

static const struct herb_info {
	int herb;
	boolean in_water;
} herb_info[] = {
	{SPRIG_OF_WOLFSBANE, false},
	{CLOVE_OF_GARLIC, false},
	{CARROT, false},
	{KELP_FROND, true}};

long count_herbs_at(xchar x, xchar y, boolean watery) {
	int dd;
	long count = 0;

	if (isok(x, y)) {
		for (dd = 0; dd < SIZE(herb_info); dd++) {
			if (watery == herb_info[dd].in_water) {
				struct obj *otmp = sobj_at(herb_info[dd].herb, x, y);
				if (otmp)
					count += otmp->quan;
			}
		}
	}
	return count;
}

/* returns true if a herb can grow at (x,y) */
boolean herb_can_grow_at(xchar x, xchar y, boolean watery) {
	struct rm *lev = &levl[x][y];
	if (inside_shop(x, y)) return false;
	if (watery)
		return (IS_POOL(lev->typ) &&
			((count_herbs_at(x, y, watery)) < HERB_GROWTH_LIMIT));
	return (lev->lit && (lev->typ == ROOM || lev->typ == CORR || (IS_DOOR(lev->typ) && ((lev->doormask == D_NODOOR) || (lev->doormask == D_ISOPEN) || (lev->doormask == D_BROKEN)))) &&
		(count_herbs_at(x, y, watery) < HERB_GROWTH_LIMIT));
}

/* grow herbs in water. return true if did something. */
boolean grow_water_herbs(int herb, xchar x, xchar y) {
	struct obj *otmp;

	rndmappos(&x, &y);
	otmp = sobj_at(herb, x, y);
	if (otmp && herb_can_grow_at(x, y, true)) {
		otmp->quan++;
		otmp->owt = weight(otmp);
		return true;
		/* There's no need to start growing these on the neighboring
		 * mapgrids, as they move around (see water_current())
		 */
	}
	return false;
}

/* grow herb on ground at (x,y), or maybe spread out.
   return true if did something. */
boolean grow_herbs(int herb, xchar x, xchar y, boolean showmsg, boolean update) {
	struct obj *otmp;

	rndmappos(&x, &y);
	otmp = sobj_at(herb, x, y);
	if (otmp && herb_can_grow_at(x, y, false)) {
		if (otmp->quan <= rn2(HERB_GROWTH_LIMIT)) {
			otmp->quan++;
			otmp->owt = weight(otmp);
			return true;
		} else {
			int dd, dofs = rn2(8);
			/* check surroundings, maybe grow there? */
			for (dd = 0; dd < 8; dd++) {
				coord pos;

				dtoxy(&pos, (dd + dofs) % 8);
				pos.x += x;
				pos.y += y;
				if (isok(pos.x, pos.y) && herb_can_grow_at(pos.x, pos.y, false)) {
					otmp = sobj_at(herb, pos.x, pos.y);
					if (otmp) {
						if (otmp->quan <= rn2(HERB_GROWTH_LIMIT)) {
							otmp->quan++;
							otmp->owt = weight(otmp);
							return true;
						}
					} else {
						otmp = mksobj(herb, true, false);
						otmp->quan = 1;
						otmp->owt = weight(otmp);
						place_object(otmp, pos.x, pos.y);
						if (update) newsym(pos.x, pos.y);
						if (cansee(pos.x, pos.y)) {
							if (showmsg && flags.verbose) {
								const char *what;
								if (herb == CLOVE_OF_GARLIC)
									what = "some garlic";
								else
									what = an(xname(otmp));
								Norep("Suddenly you notice %s growing on the %s.",
								      what, surface(pos.x, pos.y));
							}
						}
						return true;
					}
				}
			}
		}
	}
	return false;
}

/* moves topmost object in water at (x,y) to dir.
   return true if did something. */
boolean water_current(xchar x, xchar y, int dir, unsigned waterforce /* strength of the water current */, boolean showmsg, boolean update) {
	struct obj *otmp;
	coord pos;

	rndmappos(&x, &y);
	dtoxy(&pos, dir);
	pos.x += x;
	pos.y += y;
	if (isok(pos.x, pos.y) && IS_POOL(levl[x][y].typ) &&
	    IS_POOL(levl[pos.x][pos.y].typ)) {
		otmp = level.objects[x][y];
		if (otmp && otmp->where == OBJ_FLOOR) {
			if (otmp->quan > 1)
				otmp = splitobj(otmp, otmp->quan - 1);
			if (otmp->owt <= waterforce) {
				if (showmsg && Underwater &&
				    (cansee(pos.x, pos.y) || cansee(x, y))) {
					Norep("%s floats%s in%s the murky water.",
					      An(xname(otmp)),
					      (cansee(x, y) && cansee(pos.x, pos.y)) ? "" :
										       (cansee(x, y) ? " away from you" : " towards you"),
					      flags.verbose ? " the currents of" : "");
				}
				obj_extract_self(otmp);
				place_object(otmp, pos.x, pos.y);
				stackobj(otmp);
				if (update) {
					newsym(x, y);
					newsym(pos.x, pos.y);
				}
				return true;
			} else /* the object didn't move, put it back */
				stackobj(otmp);
		}
	}
	return false;
}

/* a tree at (x,y) spontaneously drops a ripe fruit */
boolean drop_ripe_treefruit(xchar x, xchar y, boolean showmsg, boolean update) {
	struct rm *lev;

	rndmappos(&x, &y);
	lev = &levl[x][y];
	if (IS_TREE(lev->typ) && !(lev->looted & TREE_LOOTED) && may_dig(x, y)) {
		coord pos;
		int dir, dofs = rn2(8);
		for (dir = 0; dir < 8; dir++) {
			dtoxy(&pos, (dir + dofs) % 8);
			pos.x += x;
			pos.y += y;
			if (!isok(pos.x, pos.y)) return false;
			lev = &levl[pos.x][pos.y];
			if (SPACE_POS(lev->typ) || IS_POOL(lev->typ)) {
				struct obj *otmp;
				otmp = rnd_treefruit_at(pos.x, pos.y);
				if (otmp) {
					otmp->quan = 1;
					otmp->owt = weight(otmp);
					obj_extract_self(otmp);
					if (showmsg) {
						if ((cansee(pos.x, pos.y) || cansee(x, y))) {
							Norep("%s falls from %s%s.",
							      cansee(pos.x, pos.y) ? An(xname(otmp)) : "Something",
							      cansee(x, y) ? "the tree" : "somewhere",
							      (cansee(x, y) && IS_POOL(lev->typ)) ?
								      " into the water" :
								      "");
						} else if (distu(pos.x, pos.y) < 9 &&
							   otmp->otyp != EUCALYPTUS_LEAF) {
							/* a leaf is too light to cause any sound */
							You_hearf("a %s!",
								  (IS_POOL(lev->typ) || IS_FOUNTAIN(lev->typ)) ?
									  "plop" :
									  "splut"); /* rainforesty sounds */
						}
					}
					place_object(otmp, pos.x, pos.y);
					stackobj(otmp);
					if (rn2(6)) levl[x][y].looted |= TREE_LOOTED;
					if (update) newsym(pos.x, pos.y);
					return true;
				}
			}
		}
	}
	return false;
}

/* Tree at (x,y) seeds. returns true if a new tree was created.
 * Creates a kind of forest, with (hopefully) most places available.
 */
boolean seed_tree(xchar x, xchar y) {
	coord pos, pos2;
	struct rm *lev;

	rndmappos(&x, &y);
	if (IS_TREE(levl[x][y].typ) && may_dig(x, y)) {
		int dir = rn2(8);
		dtoxy(&pos, dir);
		pos.x += x;
		pos.y += y;
		if (!rn2(3)) {
			dtoxy(&pos2, (dir + rn2(2)) % 8);
			pos.x += pos2.x;
			pos.y += pos2.y;
		}
		if (!isok(pos.x, pos.y)) return false;
		lev = &levl[pos.x][pos.y];
		if (lev->lit && !cansee(pos.x, pos.y) && !inside_shop(pos.x, pos.y) &&
		    (lev->typ == ROOM || lev->typ == CORR) &&
		    !(u.ux == pos.x && u.uy == pos.y) && !m_at(pos.x, pos.y) &&
		    !t_at(pos.x, pos.y) && !OBJ_AT(pos.x, pos.y)) {
			int nogrow = 0;
			int dx, dy;
			for (dx = pos.x - 1; dx <= pos.x + 1; dx++) {
				for (dy = pos.y - 1; dy <= pos.y + 1; dy++) {
					if (!isok(dx, dy) ||
					    (isok(dx, dy) && !SPACE_POS(levl[dx][dy].typ)))
						nogrow++;
				}
			}
			if (nogrow < 3) {
				lev->typ = TREE;
				lev->looted &= ~TREE_LOOTED;
				block_point(pos.x, pos.y);
				return true;
			}
		}
	}
	return false;
}

void dgn_growths(boolean showmsg /* show messages */, boolean update /* do newsym() */) {
	int herbnum = rn2(SIZE(herb_info));
	seed_tree(-1, -1);
	if (herb_info[herbnum].in_water)
		grow_water_herbs(herb_info[herbnum].herb, -1, -1);
	else
		grow_herbs(herb_info[herbnum].herb, -1, -1, showmsg, update);
	if (!rn2(30))
		drop_ripe_treefruit(-1, -1, showmsg, update);
	water_current(-1, -1, rn2(8),
		      Is_waterlevel(&u.uz) ? 200 : 25, showmsg, update);
}

/* catch up with growths when returning to a previously visited level */
void catchup_dgn_growths(int mvs) {
	if (mvs < 0)
		mvs = 0;
	else if (mvs > LARGEST_INT)
		mvs = LARGEST_INT;
	while (mvs-- > 0)
		dgn_growths(false, false);
}

boolean revive_nasty(int x, int y, const char *msg) {
	struct obj *otmp, *otmp2;
	struct monst *mtmp;
	coord cc;
	boolean revived = false;

	for (otmp = level.objects[x][y]; otmp; otmp = otmp2) {
		otmp2 = otmp->nexthere;
		if (otmp->otyp == CORPSE &&
		    (is_rider(&mons[otmp->corpsenm]) ||
		     otmp->corpsenm == PM_WIZARD_OF_YENDOR)) {
			/* move any living monster already at that location */
			if ((mtmp = m_at(x, y)) && enexto(&cc, x, y, mtmp->data))
				rloc_to(mtmp, cc.x, cc.y);
			if (msg) Norep("%s", msg);
			revived = revive_corpse(otmp, false);
		}
	}

	/* this location might not be safe, if not, move revived monster */
	if (revived) {
		mtmp = m_at(x, y);
		if (mtmp && !goodpos(x, y, mtmp, 0) &&
		    enexto(&cc, x, y, mtmp->data)) {
			rloc_to(mtmp, cc.x, cc.y);
		}
		/* else impossible? */
	}

	return revived;
}

static int moverock(void) {
	xchar rx, ry, sx, sy;
	struct obj *otmp;
	struct trap *ttmp;
	struct monst *mtmp;

	sx = u.ux + u.dx, sy = u.uy + u.dy; /* boulder starting position */
	while ((otmp = sobj_at(BOULDER, sx, sy)) != 0) {
		/* make sure that this boulder is visible as the top object */
		if (otmp != level.objects[sx][sy]) movobj(otmp, sx, sy);

		rx = u.ux + 2 * u.dx; /* boulder destination position */
		ry = u.uy + 2 * u.dy;
		nomul(0);
		if (Levitation || Is_airlevel(&u.uz)) {
			if (Blind) feel_location(sx, sy);
			pline("You don't have enough leverage to push %s.", the(xname(otmp)));
			/* Give them a chance to climb over it? */
			return -1;
		}
		if (verysmall(youmonst.data) && !u.usteed) {
			if (Blind) feel_location(sx, sy);
			pline("You're too small to push that %s.", xname(otmp));
			goto cannot_push;
		}
		if (isok(rx, ry) && !IS_ROCK(levl[rx][ry].typ) &&
		    levl[rx][ry].typ != IRONBARS &&
		    (!IS_DOOR(levl[rx][ry].typ) || !(u.dx && u.dy) || (!Is_rogue_level(&u.uz) && (levl[rx][ry].doormask & ~D_BROKEN) == D_NODOOR)) &&
		    !sobj_at(BOULDER, rx, ry)) {
			ttmp = t_at(rx, ry);
			mtmp = m_at(rx, ry);

			/* KMH -- Sokoban doesn't let you push boulders diagonally */
			if (In_sokoban(&u.uz) && u.dx && u.dy) {
				if (Blind) feel_location(sx, sy);
				pline("%s won't roll diagonally on this %s.",
				      The(xname(otmp)), surface(sx, sy));
				goto cannot_push;
			}

			if (revive_nasty(rx, ry, "You sense movement on the other side."))
				return -1;

			if (mtmp && !noncorporeal(mtmp->data) &&
			    (!mtmp->mtrapped ||
			     !(ttmp && is_pitlike(ttmp->ttyp)))) {
				if (Blind) feel_location(sx, sy);
				if (canspotmon(mtmp)) {
					boolean by_name = (mtmp->data->geno & G_UNIQ ||
							   mtmp->isshk || mtmp->mnamelth);
					if (by_name && !Hallucination)
						pline("%s is on the other side.", Monnam(mtmp));
					else
						pline("There's %s on the other side.", a_monnam(mtmp));
				} else {
					You_hearf("a monster behind %s.", the(xname(otmp)));
					map_invisible(rx, ry);
				}
				if (flags.verbose)
					pline("Perhaps that's why %s cannot move it.",
					      u.usteed ? y_monnam(u.usteed) : "you");
				goto cannot_push;
			}

			if (ttmp)
				switch (ttmp->ttyp) {
					case LANDMINE:
						if (rn2(10)) {
							obj_extract_self(otmp);
							place_object(otmp, rx, ry);
							unblock_point(sx, sy);
							newsym(sx, sy);
							pline("KAABLAMM!!!  %s %s land mine.",
							      Tobjnam(otmp, "trigger"),
							      ttmp->madeby_u ? "your" : "a");
							blow_up_landmine(ttmp);
							/* if the boulder remains, it should fill the pit */
							fill_pit(u.ux, u.uy);
							if (cansee(rx, ry)) newsym(rx, ry);
							continue;
						}
						break;
					case SPIKED_PIT:
					case PIT:
						obj_extract_self(otmp);
						/* vision kludge to get messages right;
					   the pit will temporarily be seen even
					   if this is one among multiple boulders */
						if (!Blind) viz_array[ry][rx] |= IN_SIGHT;
						if (!flooreffects(otmp, rx, ry, "fall")) {
							place_object(otmp, rx, ry);
						}
						if (mtmp && !Blind) newsym(rx, ry);
						continue;
					case HOLE:
					case TRAPDOOR:
						if (Blind)
							pline("Kerplunk!  You no longer feel %s.",
							      the(xname(otmp)));
						else
							pline("%s%s and %s a %s in the %s!",
							      Tobjnam(otmp,
								      (ttmp->ttyp == TRAPDOOR) ? "trigger" : "fall"),
							      (ttmp->ttyp == TRAPDOOR) ? nul : " into",
							      otense(otmp, "plug"),
							      (ttmp->ttyp == TRAPDOOR) ? "trap door" : "hole",
							      surface(rx, ry));
						deltrap(ttmp);
						delobj(otmp);
						bury_objs(rx, ry);
						levl[rx][ry].wall_info &= ~W_NONDIGGABLE;
						levl[rx][ry].candig = true;
						if (cansee(rx, ry)) newsym(rx, ry);
						continue;
					case LEVEL_TELEP:
					case TELEP_TRAP:
						if (u.usteed)
							pline("%s pushes %s and suddenly it disappears!",
							      upstart(y_monnam(u.usteed)), the(xname(otmp)));
						else
							pline("You push %s and suddenly it disappears!", the(xname(otmp)));

						if (ttmp->ttyp == TELEP_TRAP)
							rloco(otmp);
						else {
							int newlev = random_teleport_level();
							d_level dest;

							if (newlev == depth(&u.uz) || In_endgame(&u.uz))
								continue;
							obj_extract_self(otmp);
							add_to_migration(otmp);
							get_level(&dest, newlev);
							otmp->ox = dest.dnum;
							otmp->oy = dest.dlevel;
							otmp->owornmask = (long)MIGR_RANDOM;
						}
						seetrap(ttmp);
						continue;
				}
			if (closed_door(rx, ry))
				goto nopushmsg;
			if (boulder_hits_pool(otmp, rx, ry, true))
				continue;
			/*
			 * Re-link at top of fobj chain so that pile order is preserved
			 * when level is restored.
			 */
			if (otmp != fobj) {
				remove_object(otmp);
				place_object(otmp, otmp->ox, otmp->oy);
			}

			{
				/* note: reset to zero after save/restore cycle */
				static long lastmovetime;
				if (!u.usteed) {
					if (moves > lastmovetime + 2 || moves < lastmovetime)
						pline("With %s effort you move %s.",
						      throws_rocks(youmonst.data) ? "little" : "great",
						      the(xname(otmp)));
					exercise(A_STR, true);
				} else
					pline("%s moves %s.",
					      upstart(y_monnam(u.usteed)), the(xname(otmp)));
				lastmovetime = moves;
			}

			/* Move the boulder *after* the message. */
			if (memory_is_invisible(rx, ry))
				unmap_object(rx, ry);
			movobj(otmp, rx, ry); /* does newsym(rx,ry) */
			if (Blind) {
				feel_location(rx, ry);
				feel_location(sx, sy);
			} else {
				newsym(sx, sy);
			}
		} else {
		nopushmsg:
			if (u.usteed)
				pline("%s tries to move %s, but cannot.",
				      upstart(y_monnam(u.usteed)), the(xname(otmp)));
			else
				pline("You try to move %s, but in vain.", the(xname(otmp)));
			if (Blind) feel_location(sx, sy);
		cannot_push:
			if (throws_rocks(youmonst.data)) {
				if (u.usteed && P_SKILL(P_RIDING) < P_BASIC) {
					pline("You aren't skilled enough to %s %s from %s.",
					      (flags.pickup && !In_sokoban(&u.uz)) ? "pick up" : "push aside",
					      the(xname(otmp)), y_monnam(u.usteed));
				} else {
					pline("However, you can easily %s.",
					      (flags.pickup && !In_sokoban(&u.uz)) ? "pick it up" : "push it aside");
					if (In_sokoban(&u.uz))
						change_luck(-1); /* Sokoban guilt */
					break;
				}
				break;
			}

			if (!u.usteed &&
			    (((!invent || inv_weight() <= -850) &&
			      (!u.dx || !u.dy || (IS_ROCK(levl[u.ux][sy].typ) && IS_ROCK(levl[sx][u.uy].typ)))) ||
			     verysmall(youmonst.data))) {
				pline("However, you can squeeze yourself into a small opening.");
				if (In_sokoban(&u.uz))
					change_luck(-1); /* Sokoban guilt */
				break;
			} else
				return -1;
		}
	}
	return 0;
}

/*
 *  still_chewing()
 *
 *  Chew on a wall, door, or boulder.  Returns true if still eating, false
 *  when done.
 */
static int still_chewing(xchar x, xchar y) {
	struct rm *lev = &levl[x][y];
	struct obj *boulder = sobj_at(BOULDER, x, y);
	const char *digtxt = NULL, *dmgtxt = NULL;

	if (context.digging.down) /* not continuing previous dig (w/ pick-axe) */
		memset(&context.digging, 0, sizeof context.digging);

	if (!boulder && IS_ROCK(lev->typ) && !may_dig(x, y)) {
		pline("You hurt your teeth on the %s.",
		      IS_TREE(lev->typ) ? "tree" : "hard stone");
		nomul(0);
		return 1;
	} else if (context.digging.pos.x != x || context.digging.pos.y != y ||
		   !on_level(&context.digging.level, &u.uz)) {
		context.digging.down = false;
		context.digging.chew = true;
		context.digging.warned = false;
		context.digging.pos.x = x;
		context.digging.pos.y = y;
		assign_level(&context.digging.level, &u.uz);
		/* solid rock takes more work & time to dig through */
		context.digging.effort =
			(IS_ROCK(lev->typ) && !IS_TREE(lev->typ) ? 30 : 60) + u.udaminc;
		pline("You start chewing %s %s.",
		      (boulder || IS_TREE(lev->typ)) ? "on a" : "a hole in the",
		      boulder ? "boulder" :
				IS_TREE(lev->typ) ? "tree" : IS_ROCK(lev->typ) ? "rock" : "door");
		watch_dig(NULL, x, y, false);
		return 1;
	} else if ((context.digging.effort += (30 + u.udaminc)) <= 100) {
		if (flags.verbose)
			pline("You %s chewing on the %s.",
			      context.digging.chew ? "continue" : "begin",
			      boulder ? "boulder" :
					IS_TREE(lev->typ) ? "tree" :
							    IS_ROCK(lev->typ) ? "rock" : "door");
		context.digging.chew = true;
		watch_dig(NULL, x, y, false);
		return 1;
	}

	/* Okay, you've chewed through something */
	u.uconduct.food++;
	u.uhunger += rnd(20);

	if (boulder) {
		delobj(boulder);	       /* boulder goes bye-bye */
		pline("You eat the boulder."); /* yum */

		/*
		 *  The location could still block because of
		 *	1. More than one boulder
		 *	2. Boulder stuck in a wall/stone/door.
		 *
		 *  [perhaps use does_block() below (from vision.c)]
		 */
		if (IS_ROCK(lev->typ) || closed_door(x, y) || sobj_at(BOULDER, x, y)) {
			block_point(x, y); /* delobj will unblock the point */
			/* reset dig state */
			memset(&context.digging, 0, sizeof context.digging);
			return 1;
		}

	} else if (IS_WALL(lev->typ)) {
		if (*in_rooms(x, y, SHOPBASE)) {
			add_damage(x, y, 10L * ACURRSTR);
			dmgtxt = "damage";
		}
		digtxt = "chew a hole in the wall.";
		if (level.flags.is_maze_lev) {
			lev->typ = ROOM;
		} else if (level.flags.is_cavernous_lev && !in_town(x, y)) {
			lev->typ = CORR;
		} else {
			lev->typ = DOOR;
			lev->doormask = D_NODOOR;
		}
	} else if (IS_TREE(lev->typ)) {
		digtxt = "chew through the tree.";
		lev->typ = ROOM;
	} else if (lev->typ == SDOOR) {
		if (lev->doormask & D_TRAPPED) {
			lev->doormask = D_NODOOR;
			b_trapped("secret door", 0);
		} else {
			digtxt = "chew through the secret door.";
			lev->doormask = D_BROKEN;
		}
		lev->typ = DOOR;

	} else if (IS_DOOR(lev->typ)) {
		if (*in_rooms(x, y, SHOPBASE)) {
			add_damage(x, y, 400L);
			dmgtxt = "break";
		}
		if (lev->doormask & D_TRAPPED) {
			lev->doormask = D_NODOOR;
			b_trapped("door", 0);
		} else {
			digtxt = "chew through the door.";
			lev->doormask = D_BROKEN;
		}

	} else { /* STONE or SCORR */
		digtxt = "chew a passage through the rock.";
		lev->typ = CORR;
	}

	unblock_point(x, y); /* vision */
	newsym(x, y);
	if (digtxt) pline("You %s", digtxt); /* after newsym */
	if (dmgtxt) pay_for_damage(dmgtxt, false);
	memset(&context.digging, 0, sizeof context.digging);
	return 0;
}

void movobj(struct obj *obj, xchar ox, xchar oy) {
	/* optimize by leaving on the fobj chain? */
	remove_object(obj);
	newsym(obj->ox, obj->oy);
	place_object(obj, ox, oy);
	newsym(ox, oy);
}

static void dosinkfall(void) {
	struct obj *obj;

	if (is_floater(youmonst.data) || (HLevitation & FROMOUTSIDE)) {
		pline("You wobble unsteadily for a moment.");
	} else {
		long save_ELev = ELevitation, save_HLev = HLevitation;

		/* fake removal of levitation in advance so that final
		   disclosure will be right in case this turns out to
		   be fatal; fortunately the fact that rings and boots
		   are really still worn has no effect on bones data */
		ELevitation = HLevitation = 0L;
		pline("You crash to the floor!");
		losehp(Maybe_Half_Phys(rn1(8, 25 - ACURR(A_CON))), "fell onto a sink", NO_KILLER_PREFIX);
		exercise(A_DEX, false);
		selftouch("Falling, you");
		for (obj = level.objects[u.ux][u.uy]; obj; obj = obj->nexthere)
			if (obj->oclass == WEAPON_CLASS || is_weptool(obj)) {
				pline("You fell on %s.", doname(obj));
				losehp(Maybe_Half_Phys(rnd(3)), "fell onto a sink", NO_KILLER_PREFIX);
				exercise(A_CON, false);
			}
		ELevitation = save_ELev;
		HLevitation = save_HLev;
	}

	ELevitation &= ~W_ARTI;
	HLevitation &= ~(I_SPECIAL | TIMEOUT);
	HLevitation++;
	if (uleft && uleft->otyp == RIN_LEVITATION) {
		obj = uleft;
		Ring_off(obj);
		off_msg(obj);
	}
	if (uright && uright->otyp == RIN_LEVITATION) {
		obj = uright;
		Ring_off(obj);
		off_msg(obj);
	}
	if (uarmf && uarmf->otyp == LEVITATION_BOOTS) {
		obj = uarmf;
		Boots_off();
		off_msg(obj);
	}
	HLevitation--;
}

// intended to be called only on ROCKs
boolean may_dig(xchar x, xchar y) {
	return !(IS_STWALL(levl[x][y].typ) &&
		 (levl[x][y].wall_info & W_NONDIGGABLE));
}

boolean may_passwall(xchar x, xchar y) {
	return !(IS_STWALL(levl[x][y].typ) &&
		 (levl[x][y].wall_info & W_NONPASSWALL));
}

/* [ALI] Changed to take monst * as argument to support passwall property */
boolean bad_rock(struct monst *mon, xchar x, xchar y) {
	struct permonst *mdat = mon->data;
	boolean passwall = mon == &youmonst ? Passes_walls : passes_walls(mdat);
	return ((boolean)((In_sokoban(&u.uz) && sobj_at(BOULDER, x, y)) ||
			  (IS_ROCK(levl[x][y].typ) && (!tunnels(mdat) || needspick(mdat) || !may_dig(x, y)) && !(passwall && may_passwall(x, y)))));
}

boolean invocation_pos(xchar x, xchar y) {
	return Invocation_lev(&u.uz) && x == inv_pos.x && y == inv_pos.y;
}

/* For my clever ending messages... */
int Instant_Death = 0;
int Quick_Death = 0;
int Nibble_Death = 0;
int last_hit = 0;
int second_last_hit = 0;
int third_last_hit = 0;

/* For those tough guys who get carried away... */
int repeat_hit = 0;

/* return true if (dx,dy) is an OK place to move
 * mode is one of DO_MOVE, TEST_MOVE or TEST_TRAV
 */
boolean test_move(int ux, int uy, int dx, int dy, int mode) {
	int x = ux + dx;
	int y = uy + dy;
	struct rm *tmpr = &levl[x][y];
	struct rm *ust;

	/*
	 *  Check for physical obstacles.  First, the place we are going.
	 */
	if (IS_ROCK(tmpr->typ) || tmpr->typ == IRONBARS) {
		if (Blind && mode == DO_MOVE) feel_location(x, y);
		if (tmpr->typ == IRONBARS) {
			if (!(Passes_walls || passes_bars(youmonst.data)))
				return false;
			else if (In_sokoban(&u.uz)) {
				if (mode == DO_MOVE)
					pline("The Sokoban bars resist your ability.");
				return false;
			}
		} else if (Passes_walls && may_passwall(x, y)) {
			; /* do nothing */
		} else if (tunnels(youmonst.data) && !needspick(youmonst.data)) {
			/* Eat the rock. */
			if (mode == DO_MOVE && still_chewing(x, y)) return false;
		} else if (flags.autodig && !context.run && !context.nopick &&
			   uwep && is_pick(uwep)) {
			/* MRKR: Automatic digging when wielding the appropriate tool */
			if (mode == DO_MOVE)
				use_pick_axe2(uwep);
			return false;
		} else {
			if (mode == DO_MOVE) {
				if (Is_stronghold(&u.uz) && is_db_wall(x, y))
					pline("The drawbridge is up!");
				if (Passes_walls && !may_passwall(x, y) && In_sokoban(&u.uz))
					pline("The Sokoban walls resist your ability.");
			}
			return false;
		}
	} else if (IS_DOOR(tmpr->typ)) {
		if (closed_door(x, y)) {
			if (Blind && mode == DO_MOVE) feel_location(x, y);
			/* ALI - artifact doors */
			if (artifact_door(x, y)) {
				if (mode == DO_MOVE) {
					if (amorphous(youmonst.data))
						pline("You try to ooze under the door, but the gap is too small.");
					else if (tunnels(youmonst.data) && !needspick(youmonst.data))
						pline("You hurt your teeth on the re-enforced door.");
					else if (x == u.ux || y == u.uy) {
						if (Blind || Stunned || ACURR(A_DEX) < 10 || Fumbling) {
							pline("Ouch!  You bump into a heavy door.");
							exercise(A_DEX, false);
						} else
							pline("That door is closed.");
					}
				}
				return false;
			} else if (Passes_walls)
				; /* do nothing */
			else if (can_ooze(&youmonst)) {
				if (mode == DO_MOVE) pline("You ooze under the door.");
			} else if (tunnels(youmonst.data) && !needspick(youmonst.data)) {
				/* Eat the door. */
				if (mode == DO_MOVE && still_chewing(x, y)) return false;
			} else {
				if (mode == DO_MOVE) {
					if (amorphous(youmonst.data))
						pline("You try to ooze under the door, but can't squeeze your possessions through.");
					else if (x == ux || y == uy) {
						if (Blind || Stunned || ACURR(A_DEX) < 10 || Fumbling) {
							if (u.usteed) {
								pline("You can't lead %s through that closed door.",
								      y_monnam(u.usteed));
							} else {
								pline("Ouch!  You bump into a door.");
								exercise(A_DEX, false);
							}
						} else
							pline("That door is closed.");
					}
				} else if (mode == TEST_TRAV)
					goto testdiag;
				return false;
			}
		} else {
		testdiag:
			if (dx && dy && !Passes_walls && ((tmpr->doormask & ~D_BROKEN) || Is_rogue_level(&u.uz) || block_door(x, y))) {
				/* Diagonal moves into a door are not allowed. */
				if (Blind && mode == DO_MOVE)
					feel_location(x, y);
				return false;
			}
		}
	}
	if (dx && dy && bad_rock(&youmonst, ux, y) && bad_rock(&youmonst, x, uy)) {
		/* Move at a diagonal. */
		if (In_sokoban(&u.uz)) {
			if (mode == DO_MOVE)
				pline("You cannot pass that way.");
			return false;
		}
		if (bigmonst(youmonst.data)) {
			if (mode == DO_MOVE)
				pline("Your body is too large to fit through.");
			return false;
		}
		if (invent && (inv_weight() + weight_cap() > 600)) {
			if (mode == DO_MOVE)
				pline("You are carrying too much to get through.");
			return false;
		}
	}
	/* Pick travel path that does not require crossing a trap.
	 * Avoid water and lava using the usual running rules.
	 * (but not u.ux/u.uy because findtravelpath walks toward u.ux/u.uy) */
	if (context.run == 8 && mode != DO_MOVE && (x != u.ux || y != u.uy)) {
		struct trap *t = t_at(x, y);

		if ((t && t->tseen) ||
		    (!Levitation && !Flying &&
		     !is_clinger(youmonst.data) &&
		     (is_pool(x, y) || is_lava(x, y)) && levl[x][y].seenv))
			return false;
	}

	ust = &levl[ux][uy];

	/* Now see if other things block our way . . */
	if (dx && dy && !Passes_walls && (IS_DOOR(ust->typ) && ((ust->doormask & ~D_BROKEN) || Is_rogue_level(&u.uz) || block_entry(x, y)))) {
		/* Can't move at a diagonal out of a doorway with door. */
		return false;
	}

	if (sobj_at(BOULDER, x, y) && (In_sokoban(&u.uz) || !Passes_walls)) {
		if (!(Blind || Hallucination) && (context.run >= 2) && mode != TEST_TRAV)
			return false;
		if (mode == DO_MOVE) {
			/* tunneling monsters will chew before pushing */
			if (tunnels(youmonst.data) && !needspick(youmonst.data) &&
			    !In_sokoban(&u.uz)) {
				if (still_chewing(x, y)) return false;
			} else if (moverock() < 0)
				return false;
		} else if (mode == TEST_TRAV) {
			struct obj *obj;

			/* don't pick two boulders in a row, unless there's a way thru */
			if (sobj_at(BOULDER, ux, uy) && !In_sokoban(&u.uz)) {
				if (!Passes_walls &&
				    !(tunnels(youmonst.data) && !needspick(youmonst.data)) &&
				    !carrying(PICK_AXE) && !carrying(DWARVISH_MATTOCK) &&
				    !((obj = carrying(WAN_DIGGING)) &&
				      !objects[obj->otyp].oc_name_known))
					return false;
			}
		}
		/* assume you'll be able to push it when you get there... */
	}

	/* OK, it is a legal place to move. */
	return true;
}

/*
 * Find a path from the destination (u.tx,u.ty) back to (u.ux,u.uy).
 * A shortest path is returned.  If guess is true, consider various
 * inaccessible locations as valid intermediate path points.
 * Returns true if a path was found.
 */
static boolean findtravelpath(boolean guess) {
	/* if travel to adjacent, reachable location, use normal movement rules */
	if (!guess && context.travel1 && distmin(u.ux, u.uy, u.tx, u.ty) == 1) {
		context.run = 0;
		if (test_move(u.ux, u.uy, u.tx - u.ux, u.ty - u.uy, TEST_MOVE)) {
			u.dx = u.tx - u.ux;
			u.dy = u.ty - u.uy;
			nomul(0);
			iflags.travelcc.x = iflags.travelcc.y = -1;
			return true;
		}
		context.run = 8;
	}
	if (u.tx != u.ux || u.ty != u.uy) {
		xchar travel[COLNO][ROWNO];
		xchar travelstepx[2][COLNO * ROWNO];
		xchar travelstepy[2][COLNO * ROWNO];
		xchar tx, ty, ux, uy;
		int n = 1;	/* max offset in travelsteps */
		int set = 0;	/* two sets current and previous */
		int radius = 1; /* search radius */
		int i;

		/* If guessing, first find an "obvious" goal location.  The obvious
		 * goal is the position the player knows of, or might figure out
		 * (couldsee) that is closest to the target on a straight path.
		 */
		if (guess) {
			tx = u.ux;
			ty = u.uy;
			ux = u.tx;
			uy = u.ty;
		} else {
			tx = u.tx;
			ty = u.ty;
			ux = u.ux;
			uy = u.uy;
		}

	noguess:
		memset((void *)travel, 0, sizeof(travel));
		travelstepx[0][0] = tx;
		travelstepy[0][0] = ty;

		while (n != 0) {
			int nn = 0;

			for (i = 0; i < n; i++) {
				int dir;
				int x = travelstepx[set][i];
				int y = travelstepy[set][i];
				static int ordered[] = {0, 2, 4, 6, 1, 3, 5, 7};
				/* no diagonal movement for grid bugs */
				int dirmax = u.umonnum == PM_GRID_BUG ? 4 : 8;

				for (dir = 0; dir < dirmax; ++dir) {
					int nx = x + xdir[ordered[dir]];
					int ny = y + ydir[ordered[dir]];

					if (!isok(nx, ny)) continue;
					if ((!Passes_walls && !can_ooze(&youmonst) &&
					     closed_door(x, y)) ||
					    sobj_at(BOULDER, x, y)) {
						/* closed doors and boulders usually
						 * cause a delay, so prefer another path */
						if (travel[x][y] > radius - 3) {
							travelstepx[1 - set][nn] = x;
							travelstepy[1 - set][nn] = y;
							/* don't change travel matrix! */
							nn++;
							continue;
						}
					}
					if (test_move(x, y, nx - x, ny - y, TEST_TRAV) &&
					    (levl[nx][ny].seenv || (!Blind && couldsee(nx, ny)))) {
						if (nx == ux && ny == uy) {
							if (!guess) {
								u.dx = x - ux;
								u.dy = y - uy;
								if (x == u.tx && y == u.ty) {
									nomul(0);
									/* reset run so domove run checks work */
									context.run = 8;
									iflags.travelcc.x = iflags.travelcc.y = -1;
								}
								return true;
							}
						} else if (!travel[nx][ny]) {
							travelstepx[1 - set][nn] = nx;
							travelstepy[1 - set][nn] = ny;
							travel[nx][ny] = radius;
							nn++;
						}
					}
				}
			}

			n = nn;
			set = 1 - set;
			radius++;
		}

		/* if guessing, find best location in travel matrix and go there */
		if (guess) {
			int px = tx, py = ty; /* pick location */
			int dist, nxtdist, d2, nd2;

			dist = distmin(ux, uy, tx, ty);
			d2 = dist2(ux, uy, tx, ty);
			for (tx = 1; tx < COLNO; ++tx)
				for (ty = 0; ty < ROWNO; ++ty)
					if (travel[tx][ty]) {
						nxtdist = distmin(ux, uy, tx, ty);
						if (nxtdist == dist && couldsee(tx, ty)) {
							nd2 = dist2(ux, uy, tx, ty);
							if (nd2 < d2) {
								/* prefer non-zigzag path */
								px = tx;
								py = ty;
								d2 = nd2;
							}
						} else if (nxtdist < dist && couldsee(tx, ty)) {
							px = tx;
							py = ty;
							dist = nxtdist;
							d2 = dist2(ux, uy, tx, ty);
						}
					}

			if (px == u.ux && py == u.uy) {
				/* no guesses, just go in the general direction */
				u.dx = sgn(u.tx - u.ux);
				u.dy = sgn(u.ty - u.uy);
				if (test_move(u.ux, u.uy, u.dx, u.dy, TEST_MOVE))
					return true;
				goto found;
			}
			tx = px;
			ty = py;
			ux = u.ux;
			uy = u.uy;
			set = 0;
			n = radius = 1;
			guess = false;
			goto noguess;
		}
		return false;
	}

found:
	u.dx = 0;
	u.dy = 0;
	nomul(0);
	return false;
}

void domove(void) {
	struct monst *mtmp;
	struct rm *tmpr;
	xchar x, y;
	struct trap *trap;
	int wtcap;
	boolean on_ice;
	xchar chainx, chainy, ballx, bally; /* ball&chain new positions */
	int bc_control;			    /* control for ball&chain */
	boolean cause_delay = false;	    /* dragging ball will skip a move */
	const char *predicament;
	boolean displacer = false; /* defender attempts to displace you */

	u_wipe_engr(rnd(5));

	if (context.travel) {
		if (!findtravelpath(false))
			findtravelpath(true);
		context.travel1 = 0;
	}

	if (((wtcap = near_capacity()) >= OVERLOADED || (wtcap > SLT_ENCUMBER &&
							 (Upolyd ? (u.mh < 5 && u.mh != u.mhmax) : (u.uhp < 10 && u.uhp != u.uhpmax)))) &&
	    !Is_airlevel(&u.uz)) {
		if (wtcap < OVERLOADED) {
			pline("You don't have enough stamina to move.");
			exercise(A_CON, false);
		} else
			pline("You collapse under your load.");
		nomul(0);
		return;
	}
	if (u.uswallow) {
		u.dx = u.dy = 0;
		u.ux = x = u.ustuck->mx;
		u.uy = y = u.ustuck->my;
		mtmp = u.ustuck;
	} else {
		if (Is_airlevel(&u.uz) && rn2(4) &&
		    !Levitation && !Flying) {
			switch (rn2(3)) {
				case 0:
					pline("You tumble in place.");
					exercise(A_DEX, false);
					break;
				case 1:
					pline("You can't control your movements very well.");
					break;
				case 2:
					pline("It's hard to walk in thin air.");
					exercise(A_DEX, true);
					break;
			}
			return;
		}

		/* check slippery ice */
		on_ice = !Levitation && is_ice(u.ux, u.uy);
		if (on_ice) {
			static int skates = 0;
			if (!skates) skates = find_skates();
			if ((uarmf && uarmf->otyp == skates) || resists_cold(&youmonst) || Flying || is_floater(youmonst.data) || is_clinger(youmonst.data) || is_whirly(youmonst.data))
				on_ice = false;
			else if (!rn2(Cold_resistance ? 3 : 2)) {
				HFumbling |= FROMOUTSIDE;
				HFumbling &= ~TIMEOUT;
				HFumbling += 1; /* slip on next move */
			}
		}
		if (!on_ice && (HFumbling & FROMOUTSIDE))
			HFumbling &= ~FROMOUTSIDE;

		x = u.ux + u.dx;
		y = u.uy + u.dy;
		/* Check if your steed can move */
		if (u.usteed && (!u.usteed->mcanmove || u.usteed->msleeping)) {
			pline("Your steed doesn't respond!");
			nomul(0);
			return;
		}
		if (Stunned || (Confusion && !rn2(5)) || (u.usteed && u.usteed->mconf)) {
			int tries = 0;

			do {
				if (tries++ > 50) {
					nomul(0);
					return;
				}
				confdir();
				x = u.ux + u.dx;
				y = u.uy + u.dy;
			} while (!isok(x, y) || bad_rock(&youmonst, x, y));
		}
		/* turbulence might alter your actual destination */
		if (u.uinwater) {
			water_friction();
			if (!u.dx && !u.dy) {
				nomul(0);
				return;
			}
			x = u.ux + u.dx;
			y = u.uy + u.dy;
		}
		if (!isok(x, y)) {
			nomul(0);
			return;
		}
		if (((trap = t_at(x, y)) && trap->tseen) ||
		    (Blind && !Levitation && !Flying &&
		     !is_clinger(youmonst.data) &&
		     (is_pool(x, y) || is_lava(x, y)) && levl[x][y].seenv)) {
			if (context.run >= 2) {
				nomul(0);
				context.move = 0;
				return;
			} else
				nomul(0);
		}

		if (u.ustuck && (x != u.ustuck->mx || y != u.ustuck->my)) {
			if (distu(u.ustuck->mx, u.ustuck->my) > 2) {
				/* perhaps it fled (or was teleported or ... ) */
				setustuck(0);
			} else if (sticks(youmonst.data)) {
				/* When polymorphed into a sticking monster,
				 * u.ustuck means it's stuck to you, not you to it.
				 */
				pline("You release %s.", mon_nam(u.ustuck));
				setustuck(0);
			} else {
				/* If holder is asleep or paralyzed:
				 *	37.5% chance of getting away,
				 *	12.5% chance of waking/releasing it;
				 * otherwise:
				 *	 7.5% chance of getting away.
				 * [strength ought to be a factor]
				 * If holder is tame and there is no conflict,
				 * guaranteed escape.
				 */
				switch (rn2(!u.ustuck->mcanmove ? 8 : 40)) {
					case 0:
					case 1:
					case 2:
					pull_free:
						pline("You pull free from %s.", mon_nam(u.ustuck));
						setustuck(0);
						break;
					case 3:
						if (!u.ustuck->mcanmove) {
							/* it's free to move on next turn */
							u.ustuck->mfrozen = 1;
							u.ustuck->msleeping = 0;
						}
					/*FALLTHRU*/
					default:
						if (u.ustuck->mtame &&
						    !Conflict && !u.ustuck->mconf)
							goto pull_free;
						pline("You cannot escape from %s!", mon_nam(u.ustuck));
						nomul(0);
						return;
				}
			}
		}

		mtmp = m_at(x, y);
		if (mtmp) {
			/* Don't attack if you're running, and can see it */
			/* We should never get here if forcefight */
			if (context.run &&
			    ((!Blind && mon_visible(mtmp) &&
			      ((mtmp->m_ap_type != M_AP_FURNITURE &&
				mtmp->m_ap_type != M_AP_OBJECT) ||
			       Protection_from_shape_changers)) ||
			     sensemon(mtmp))) {
				nomul(0);
				context.move = 0;
				return;
			}
		}
	}
	u.ux0 = u.ux;
	u.uy0 = u.uy;
	bhitpos.x = x;
	bhitpos.y = y;
	tmpr = &levl[x][y];
	/* attack monster */
	if (mtmp) {
		nomul(0);
		/* only attack if we know it's there */
		/* or if we used the 'F' command to fight blindly */
		/* or if it hides_under, in which case we call attack() to print
		 * the Wait! message.
		 * This is different from ceiling hiders, who aren't handled in
		 * attack().
		 */

		/* If they used a 'm' command, trying to move onto a monster
		 * prints the below message and wastes a turn.  The exception is
		 * if the monster is unseen and the player doesn't remember an
		 * invisible monster--then, we fall through to attack() and
		 * attack_check(), which still wastes a turn, but prints a
		 * different message and makes the player remember the monster.		     */
		if (context.nopick &&
		    (canspotmon(mtmp) || memory_is_invisible(x, y))) {
			if (mtmp->m_ap_type && !Protection_from_shape_changers && !sensemon(mtmp))
				stumble_onto_mimic(mtmp);
			else if (mtmp->mpeaceful && !Hallucination)
				pline("Pardon me, %s.", m_monnam(mtmp));
			else
				pline("You move right into %s.", mon_nam(mtmp));
			return;
		}
		if (context.forcefight || !mtmp->mundetected || sensemon(mtmp) ||
		    ((hides_under(mtmp->data) || mtmp->data->mlet == S_EEL) &&
		     !is_safepet(mtmp))) {
			gethungry();
			if (wtcap >= HVY_ENCUMBER && moves % 3) {
				if (Upolyd && u.mh > 1) {
					u.mh--;
				} else if (!Upolyd && u.uhp > 1) {
					u.uhp--;
				} else {
					pline("You pass out from exertion!");
					exercise(A_CON, false);
					fall_asleep(-10, false);
				}
			}
			if (multi < 0) return; /* we just fainted */
			/* new displacer beast thingie -- by [Tom] */
			/* sometimes, instead of attacking, you displace it. */
			/* Good joke, huh? */
			if (mtmp->data == &mons[PM_DISPLACER_BEAST] && !rn2(2))
				displacer = true;
			else
				/* try to attack; note that it might evade */
				/* also, we don't attack tame when _safepet_ */
				if (attack(mtmp))
				return;
		}
	}
	/* specifying 'F' with no monster wastes a turn */
	if (context.forcefight ||
	    /* remembered an 'I' && didn't use a move command */
	    (memory_is_invisible(x, y) && !context.nopick)) {
		boolean expl = (Upolyd && attacktype(youmonst.data, AT_EXPL));
		char buf[BUFSZ];
		sprintf(buf, "a vacant spot on the %s", surface(x, y));
		pline("You %s %s.",
		      expl ? "explode at" : "attack",
		      !Underwater ? "thin air" :
				    is_pool(x, y) ? "empty water" : buf);
		unmap_object(x, y); /* known empty -- remove 'I' if present */
		newsym(x, y);
		nomul(0);
		if (expl) {
			u.mh = -1; /* dead in the current form */
			rehumanize();
		}
		return;
	}
	if (memory_is_invisible(x, y)) {
		unmap_object(x, y);
		newsym(x, y);
	}
	/* not attacking an animal, so we try to move */
	if (!displacer) {
		if (u.usteed && !u.usteed->mcanmove && (u.dx || u.dy)) {
			pline("%s won't move!", upstart(y_monnam(u.usteed)));
			nomul(0);
			return;
		} else if (!youmonst.data->mmove) {
			pline("You are rooted %s.",
			      Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) ?
				      "in place" :
				      "to the ground");
			nomul(0);
			return;
		}
		if (u.utrap) {
			if (u.utraptype == TT_PIT) {
				if (Passes_walls) {
					// were marked as trapped so they could pick stuff up from pit
					pline("You ascend from the pit.");
					u.utrap = 0;
					fill_pit(u.ux, u.uy);
					vision_full_recalc = true;
					goto maybemove;
				} else if (!rn2(2) && sobj_at(BOULDER, u.ux, u.uy)) {
					pline("Your %s gets stuck in a crevice.", body_part(LEG));
					display_nhwindow(WIN_MESSAGE, false);
					clear_nhwindow(WIN_MESSAGE);
					pline("You free your %s.", body_part(LEG));
				} else if (Flying && !In_sokoban(&u.uz)) {
					/* eg fell in pit, poly'd to a flying monster */
					pline("You fly from the pit.");
					u.utrap = 0;
					fill_pit(u.ux, u.uy);
					vision_full_recalc = 1; /* vision limits change */
				} else if (!(--u.utrap)) {
					pline("You %s to the edge of the pit.",
					      (In_sokoban(&u.uz) && Levitation) ?
						      "struggle against the air currents and float" :
						      u.usteed ? "ride" :
								 "crawl");
					fill_pit(u.ux, u.uy);
					vision_full_recalc = 1; /* vision limits change */
				} else if (flags.verbose) {
					if (u.usteed)
						Norep("%s is still in a pit.",
						      upstart(y_monnam(u.usteed)));
					else
						Norep((Hallucination && !rn2(5)) ?
							      "You've fallen, and you can't get up." :
							      "You are still in a pit.");
				}
			} else if (u.utraptype == TT_LAVA) {
				if (flags.verbose) {
					predicament = "stuck in the lava";
					if (u.usteed)
						Norep("%s is %s.", upstart(y_monnam(u.usteed)),
						      predicament);
					else
						Norep("You are %s.", predicament);
				}
				if (!is_lava(x, y)) {
					u.utrap--;
					if ((u.utrap & 0xff) == 0) {
						if (u.usteed)
							pline("You lead %s to the edge of the lava.",
							      y_monnam(u.usteed));
						else
							pline("You pull yourself to the edge of the lava.");
						u.utrap = 0;
					}
				}
				u.umoved = true;
			} else if (u.utraptype == TT_WEB) {
				if (uwep && uwep->oartifact == ART_STING) {
					u.utrap = 0;
					pline("Sting cuts through the web!");
					return;
				}
				if (--u.utrap) {
					if (flags.verbose) {
						predicament = "stuck to the web";
						if (u.usteed)
							Norep("%s is %s.", upstart(y_monnam(u.usteed)),
							      predicament);
						else
							Norep("You are %s.", predicament);
					}
				} else {
					if (u.usteed)
						pline("%s breaks out of the web.",
						      upstart(y_monnam(u.usteed)));
					else
						pline("You disentangle yourself.");
				}
			} else if (u.utraptype == TT_INFLOOR) {
				if (--u.utrap) {
					if (flags.verbose) {
						predicament = "stuck in the";
						if (u.usteed)
							Norep("%s is %s %s.",
							      upstart(y_monnam(u.usteed)),
							      predicament, surface(u.ux, u.uy));
						else
							Norep("You are %s %s.", predicament, surface(u.ux, u.uy));
					}
				} else {
					if (u.usteed)
						pline("%s finally wiggles free.",
						      upstart(y_monnam(u.usteed)));
					else
						pline("You finally wiggle free.");
				}
			} else {
				if (flags.verbose) {
					predicament = "caught in a bear trap";
					if (u.usteed)
						Norep("%s is %s.", upstart(y_monnam(u.usteed)),
						      predicament);
					else
						Norep("You are %s.", predicament);
				}
				if ((u.dx && u.dy) || !rn2(5)) u.utrap--;
			}
			return;
		}
maybemove:

		if (!test_move(u.ux, u.uy, x - u.ux, y - u.uy, DO_MOVE)) {
			context.move = 0;
			nomul(0);
			return;
		}

	} else if (!test_move(u.ux, u.uy, x - u.ux, y - u.uy, TEST_MOVE)) {
		/*
		 * If a monster attempted to displace us but failed
		 * then we are entitled to our normal attack.
		 */
		if (!attack(mtmp)) {
			context.move = 0;
			nomul(0);
		}
		return;
	}

	/* Move ball and chain.  */
	if (Punished)
		if (!drag_ball(x, y, &bc_control, &ballx, &bally, &chainx, &chainy,
			       &cause_delay, true))
			return;

	/* Check regions entering/leaving */
	if (!in_out_region(x, y)) {
		/* [ALI] This can't happen at present, but if it did we would
		 * also need to worry about the call to drag_ball above.
		 */
		//if (displacer) attack(mtmp);
		return;
	}

	/* now move the hero */
	mtmp = m_at(x, y);
	u.ux += u.dx;
	u.uy += u.dy;

	/* Move your steed, too */
	if (u.usteed) {
		u.usteed->mx = u.ux;
		u.usteed->my = u.uy;
		exercise_steed();
	}

	if (displacer) {
		char pnambuf[BUFSZ];

		u.utrap = 0; /* A lucky escape */
		/* save its current description in case of polymorph */
		strcpy(pnambuf, mon_nam(mtmp));
		remove_monster(x, y);
		place_monster(mtmp, u.ux0, u.uy0);
		/* check for displacing it into pools and traps */
		switch (minliquid(mtmp) ? 2 : mintrap(mtmp)) {
			case 0:
				pline("You displaced %s.", pnambuf);
				break;
			case 1:
			case 3:
				break;
			case 2:
				u.uconduct.killer++;
				break;
		}
	}

	/*
	 * if safepet at destination then move the pet to the hero's
	 * previous location using the same conditions as in attack().
	 * there are special extenuating circumstances:
	 * (1) if the pet dies then your god angers,
	 * (2) if the pet gets trapped then your god may disapprove,
	 * (3) if the pet was already trapped and you attempt to free it
	 * not only do you encounter the trap but you may frighten your
	 * pet causing it to go wild!  moral: don't abuse this privilege.
	 *
	 * Ceiling-hiding pets are skipped by this section of code, to
	 * be caught by the normal falling-monster code.
	 */
	if (is_safepet(mtmp) && !(is_hider(mtmp->data) && mtmp->mundetected)) {
		/* if trapped, there's a chance the pet goes wild */
		if (mtmp->mtrapped) {
			if (!rn2(mtmp->mtame)) {
				mtmp->mtame = mtmp->mpeaceful = mtmp->msleeping = 0;
				if (mtmp->mleashed) m_unleash(mtmp, true);
				growl(mtmp);
			} else {
				yelp(mtmp);
			}
		}
		mtmp->mundetected = 0;
		if (mtmp->m_ap_type)
			seemimic(mtmp);
		else if (!mtmp->mtame)
			newsym(mtmp->mx, mtmp->my);

		if (mtmp->mtrapped &&
		    (trap = t_at(mtmp->mx, mtmp->my)) != 0 &&
		    is_pitlike(trap->ttyp) &&
		    sobj_at(BOULDER, trap->tx, trap->ty)) {
			/* can't swap places with pet pinned in a pit by a boulder */
			u.ux = u.ux0, u.uy = u.uy0; /* didn't move after all */
		} else if (u.ux0 != x && u.uy0 != y &&
			   bad_rock(mtmp, x, u.uy0) &&
			   bad_rock(mtmp, u.ux0, y) &&
			   (bigmonst(mtmp->data) || (curr_mon_load(mtmp) > 600))) {
			/* can't swap places when pet won't fit thru the opening */
			u.ux = u.ux0, u.uy = u.uy0; /* didn't move after all */
			pline("You stop.  %s won't fit through.", upstart(y_monnam(mtmp)));
		} else {
			char pnambuf[BUFSZ];

			/* save its current description in case of polymorph */
			strcpy(pnambuf, y_monnam(mtmp));
			mtmp->mtrapped = 0;
			remove_monster(x, y);
			place_monster(mtmp, u.ux0, u.uy0);

			/* check for displacing it into pools and traps */
			switch (minliquid(mtmp) ? 2 : mintrap(mtmp)) {
				case 0:
					pline("You %s %s.", mtmp->mtame ? "displaced" : "frightened",
					      pnambuf);
					break;
				case 1: /* trapped */
				case 3: /* changed levels */
					/* there's already been a trap message, reinforce it */
					abuse_dog(mtmp);
					adjalign(-3);
					break;
				case 2:
					/* drowned or died...
					 * you killed your pet by direct action, so get experience
					 * and possibly penalties;
					 * we want the level gain message, if it happens, to occur
					 * before the guilt message below
					 */

					{
						/* minliquid() and mintrap() call mondead() rather than
						 * killed() so we duplicate some of the latter here */
						int tmp, mndx;

						u.uconduct.killer++;
						mndx = monsndx(mtmp->data);
						tmp = experience(mtmp, mvitals[mndx].died + 1);
						more_experienced(tmp, 0);
						newexplevel();	// will decide if you go up
					}

					/* That's no way to treat a pet!  Your god gets angry.
					 *
					 * [This has always been pretty iffy.  Why does your
					 * patron deity care at all, let alone enough to get mad?]
					 */
					if (rn2(4)) {
						pline("You feel guilty about losing your pet like this.");
						u.ugangr++;
						adjalign(-15);
					}

					break;
				default:
					pline("that's strange, unknown mintrap result!");
					break;
			}
		}
	}

	reset_occupations();
	if (context.run) {
		if (context.run < 8)
			if (IS_DOOR(tmpr->typ) || IS_ROCK(tmpr->typ) ||
			    IS_FURNITURE(tmpr->typ))
				nomul(0);
	}

	if (hides_under(youmonst.data))
		u.uundetected = OBJ_AT(u.ux, u.uy);
	else if (youmonst.data->mlet == S_EEL)
		u.uundetected = is_pool(u.ux, u.uy) && !Is_waterlevel(&u.uz);
	else if (u.dx || u.dy)
		u.uundetected = 0;

	/*
	 * Mimics (or whatever) become noticeable if they move and are
	 * imitating something that doesn't move.  We could extend this
	 * to non-moving monsters...
	 */
	if ((u.dx || u.dy) && (youmonst.m_ap_type == M_AP_OBJECT || youmonst.m_ap_type == M_AP_FURNITURE))
		youmonst.m_ap_type = M_AP_NOTHING;

	check_leash(&youmonst, u.ux0, u.uy0, false);

	if (u.ux0 != u.ux || u.uy0 != u.uy) {
		u.umoved = true;
		/* Clean old position -- vision_recalc() will print our new one. */
		newsym(u.ux0, u.uy0);
		/* Since the hero has moved, adjust what can be seen/unseen. */
		vision_recalc(1); /* Do the work now in the recover time. */
		invocation_message();
	}

	if (Punished) /* put back ball and chain */
		move_bc(0, bc_control, ballx, bally, chainx, chainy);

	spoteffects(true);

	/* delay next move because of ball dragging */
	/* must come after we finished picking up, in spoteffects() */
	if (cause_delay) {
		nomul(-2);
		nomovemsg = "";
	}

	if (context.run && iflags.runmode != RUN_TPORT) {
		/* display every step or every 7th step depending upon mode */
		if (iflags.runmode != RUN_LEAP || !(moves % 7L)) {
			if (flags.time) context.botl = 1;
			curs_on_u();
			delay_output();
			if (iflags.runmode == RUN_CRAWL) {
				delay_output();
				delay_output();
				delay_output();
				delay_output();
			}
		}
	}
}

void invocation_message(void) {
	/* a special clue-msg when on the Invocation position */
	if (invocation_pos(u.ux, u.uy) && !On_stairs(u.ux, u.uy)) {
		char buf[BUFSZ];
		struct obj *otmp = carrying(CANDELABRUM_OF_INVOCATION);

		nomul(0); /* stop running or travelling */
		if (Hallucination)
			pline("You're picking up good vibrations!");
		else {
			if (u.usteed)
				sprintf(buf, "beneath %s", y_monnam(u.usteed));
			else if (Levitation || Flying)
				strcpy(buf, "beneath you");
			else
				sprintf(buf, "under your %s", makeplural(body_part(FOOT)));

			pline("You feel a strange vibration %s.", buf);
		}
		if (otmp && otmp->spe == 7 && otmp->lamplit)
			pline("%s %s!", The(xname(otmp)),
			      Blind ? "throbs palpably" : "glows with a strange light");
	}
}

void spoteffects(boolean pick) {
	struct monst *mtmp;

	if (u.uinwater) {
		int was_underwater;

		if (!is_pool(u.ux, u.uy)) {
			if (Is_waterlevel(&u.uz))
				pline("You pop into an air bubble.");
			else if (is_lava(u.ux, u.uy))
				pline("You leave the water..."); /* oops! */
			else
				pline("You are on solid %s again.",
				      is_ice(u.ux, u.uy) ? "ice" : "land");
		} else if (Is_waterlevel(&u.uz))
			goto stillinwater;
		else if (Levitation)
			pline("You pop out of the water like a cork!");
		/* KMH, balance patch -- new intrinsic */
		else if (Flying)
			pline("You fly out of the water.");
		else if (Wwalking)
			pline("You slowly rise above the surface.");
		/*              else if (Swimming)
					pline("You paddle up to the surface.");*/
		else
			goto stillinwater;
		was_underwater = Underwater && !Is_waterlevel(&u.uz);
		u.uinwater = 0;	      /* leave the water */
		if (was_underwater) { /* restore vision */
			docrt();
			vision_full_recalc = 1;
		}
	}
stillinwater:;
	if (!Levitation && !u.ustuck && !Flying) {
		/* limit recursive calls through teleds() */
		if (is_pool(u.ux, u.uy) || is_lava(u.ux, u.uy)) {
			if (u.usteed && !is_flyer(u.usteed->data) &&
			    !is_floater(u.usteed->data) &&
			    !is_clinger(u.usteed->data)) {
				dismount_steed(Underwater ?
						       DISMOUNT_FELL :
						       DISMOUNT_GENERIC);
				/* dismount_steed() -> float_down() -> pickup() */
				if (!Is_airlevel(&u.uz) && !Is_waterlevel(&u.uz))
					pick = false;
			} else if (is_lava(u.ux, u.uy)) {
				if (lava_effects()) return;
			} else if (!Wwalking && drown()) {
				return;
			}
		}
	}
	check_special_room(false);

	if (IS_SINK(levl[u.ux][u.uy].typ) && Levitation)
		dosinkfall();

	if (!in_steed_dismounting) { /* if dismounting, we'll check again later */
		struct trap *trap = t_at(u.ux, u.uy);
		boolean pit;
		pit = trap && is_pitlike(trap->ttyp);
		if (trap && pit)
			dotrap(trap, 0); /* fall into pit */
		if (pick) pickup(1);
		if (trap && !pit)
			dotrap(trap, 0); /* fall into arrow trap, etc. */
	}
	/* Warning alerts you to ice danger */
	if (Warning && is_ice(u.ux,u.uy)) {
		static const char * const icewarnings[] = {
			"The ice seems very soft and slushy.",
			"You feel the ice shift beneath you!",
			"The ice, is gonna BREAK!",     /* The Dead Zone */
		};
		long time_left = spot_time_left(u.ux, u.uy, MELT_ICE_AWAY);
		if (time_left && time_left < 15L)
			pline("%s",
				(time_left < 5L)  ? icewarnings[2] :
				(time_left < 10L) ? icewarnings[1] : icewarnings[0]);
	}

	if ((mtmp = m_at(u.ux, u.uy)) && !u.uswallow) {
		mtmp->mundetected = mtmp->msleeping = 0;
		switch (mtmp->data->mlet) {
			case S_PIERCER:
				pline("%s suddenly drops from the %s!",
				      Amonnam(mtmp), ceiling(u.ux, u.uy));
				if (mtmp->mtame) /* jumps to greet you, not attack */
					;
				else if (uarmh && is_metallic(uarmh))
					pline("Its blow glances off your %s.", helm_simple_name(uarmh));
				else if (u.uac + 3 <= rnd(20))
					pline("You are almost hit by %s!",
					      x_monnam(mtmp, ARTICLE_A, "falling", 0, true));
				else {
					int dmg;
					pline("You are hit by %s!",
					      x_monnam(mtmp, ARTICLE_A, "falling", 0, true));
					dmg = d(4, 6);
					if (Half_physical_damage) dmg = (dmg + 1) / 2;
					mdamageu(mtmp, dmg);
				}
				break;
			default: /* monster surprises you. */
				if (mtmp->mtame)
					pline("%s jumps near you from the %s.",
					      Amonnam(mtmp), ceiling(u.ux, u.uy));
				else if (mtmp->mpeaceful) {
					pline("You surprise %s!",
					      Blind && !sensemon(mtmp) ?
						      "something" :
						      a_monnam(mtmp));
					mtmp->mpeaceful = 0;
				} else
					pline("%s attacks you by surprise!",
					      Amonnam(mtmp));
				break;
		}
		mnexto(mtmp); /* have to move the monster */
	}
}

static boolean monstinroom(struct permonst *mdat, int roomno) {
	struct monst *mtmp;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		if (!DEADMONSTER(mtmp) && mtmp->data == mdat &&
		    index(in_rooms(mtmp->mx, mtmp->my, 0), roomno + ROOMOFFSET))
			return true;
	return false;
}

char *in_rooms(xchar x, xchar y, int typewanted) {
	static char buf[5];
	char rno, *ptr = &buf[4];
	int typefound, min_x, min_y, max_x, max_y_offset, step;
	struct rm *lev;

#define goodtype(rno) (!typewanted ||                                                 \
		       ((typefound = rooms[rno - ROOMOFFSET].rtype) == typewanted) || \
		       ((typewanted == SHOPBASE) && (typefound > SHOPBASE)))

	switch (rno = levl[x][y].roomno) {
		case NO_ROOM:
			return ptr;
		case SHARED:
			step = 2;
			break;
		case SHARED_PLUS:
			step = 1;
			break;
		default: /* i.e. a regular room # */
			if (goodtype(rno))
				*(--ptr) = rno;
			return ptr;
	}

	min_x = x - 1;
	max_x = x + 1;
	if (x < 1)
		min_x += step;
	else if (x >= COLNO)
		max_x -= step;

	min_y = y - 1;
	max_y_offset = 2;
	if (min_y < 0) {
		min_y += step;
		max_y_offset -= step;
	} else if ((min_y + max_y_offset) >= ROWNO)
		max_y_offset -= step;

	for (x = min_x; x <= max_x; x += step) {
		lev = &levl[x][min_y];
		y = 0;
		if (((rno = lev[y].roomno) >= ROOMOFFSET) &&
		    !index(ptr, rno) && goodtype(rno))
			*(--ptr) = rno;
		y += step;
		if (y > max_y_offset)
			continue;
		if (((rno = lev[y].roomno) >= ROOMOFFSET) &&
		    !index(ptr, rno) && goodtype(rno))
			*(--ptr) = rno;
		y += step;
		if (y > max_y_offset)
			continue;
		if (((rno = lev[y].roomno) >= ROOMOFFSET) &&
		    !index(ptr, rno) && goodtype(rno))
			*(--ptr) = rno;
	}
	return ptr;
}

/* is (x,y) in a town? */
boolean in_town(int x, int y) {
	s_level *slev = Is_special(&u.uz);
	struct mkroom *sroom;
	boolean has_subrooms = false;

	if (!slev || !slev->flags.town) return false;

	/*
	 * See if (x,y) is in a room with subrooms, if so, assume it's the
	 * town.  If there are no subrooms, the whole level is in town.
	 */
	for (sroom = &rooms[0]; sroom->hx > 0; sroom++) {
		if (sroom->nsubrooms > 0) {
			has_subrooms = true;
			if (inside_room(sroom, x, y)) return true;
		}
	}

	return !has_subrooms;
}

static void move_update(boolean newlev) {
	char *ptr1, *ptr2, *ptr3, *ptr4;

	strcpy(u.urooms0, u.urooms);
	strcpy(u.ushops0, u.ushops);
	if (newlev) {
		u.urooms[0] = '\0';
		u.uentered[0] = '\0';
		u.ushops[0] = '\0';
		u.ushops_entered[0] = '\0';
		strcpy(u.ushops_left, u.ushops0);
		return;
	}
	strcpy(u.urooms, in_rooms(u.ux, u.uy, 0));

	for (ptr1 = &u.urooms[0],
	    ptr2 = &u.uentered[0],
	    ptr3 = &u.ushops[0],
	    ptr4 = &u.ushops_entered[0];
	     *ptr1;
	     ptr1++) {
		if (!index(u.urooms0, *ptr1))
			*(ptr2++) = *ptr1;
		if (IS_SHOP(*ptr1 - ROOMOFFSET)) {
			*(ptr3++) = *ptr1;
			if (!index(u.ushops0, *ptr1))
				*(ptr4++) = *ptr1;
		}
	}
	*ptr2 = '\0';
	*ptr3 = '\0';
	*ptr4 = '\0';

	/* filter u.ushops0 -> u.ushops_left */
	for (ptr1 = &u.ushops0[0], ptr2 = &u.ushops_left[0]; *ptr1; ptr1++)
		if (!index(u.ushops, *ptr1))
			*(ptr2++) = *ptr1;
	*ptr2 = '\0';
}

void check_special_room(boolean newlev) {
	struct monst *mtmp;
	char *ptr;

	move_update(newlev);

	if (*u.ushops0)
		u_left_shop(u.ushops_left, newlev);

	if (!*u.uentered && !*u.ushops_entered) /* implied by newlev */
		return;				/* no entrance messages necessary */

	/* Did we just enter a shop? */
	if (*u.ushops_entered)
		u_entered_shop(u.ushops_entered);

	for (ptr = &u.uentered[0]; *ptr; ptr++) {
		int roomno = *ptr - ROOMOFFSET, rt = rooms[roomno].rtype;

		/* Did we just enter some other special room? */
		/* vault.c insists that a vault remain a VAULT,
		 * BEEHIVEs have special colouration,
		 * and temples should remain TEMPLEs,
		 * but everything else gives a message only the first time */
		switch (rt) {
			case ZOO:
				pline("Welcome to David's treasure zoo!");
				break;
			case SWAMP:
				pline("It %s rather %s down here.",
				      Blind ? "feels" : "looks",
				      Blind ? "humid" : "muddy");
				break;
			case COURT:
				pline("You enter an opulent throne room!");
				break;
			case REALZOO:
				pline("You enter a smelly zoo!");
				break;
			case GIANTCOURT:
				pline("You enter a giant throne room!");
				break;
			case DRAGONLAIR:
				pline("You enter a dragon lair...");
				break;
			case BADFOODSHOP:
				pline("You enter an abandoned store...");
				break;
			case LEPREHALL:
				pline("You enter a leprechaun hall!");
				break;
			case MORGUE:
				if (midnight()) {
					const char *run = locomotion(youmonst.data, "Run");
					pline("%s away!  %s away!", run, run);
				} else
					pline("You have an uncanny feeling...");
				break;
			case BEEHIVE:
				if (monstinroom(&mons[PM_QUEEN_BEE], roomno)) {
					pline("You enter a giant beehive!");
				}
				rt = 0;
				break;
			case LEMUREPIT:
				pline("You enter a pit of screaming lemures!");
				break;
			case MIGOHIVE:
				pline("You enter a strange hive!");
				break;
			case FUNGUSFARM:
				pline("You enter a room full of fungi!");
				break;
			case COCKNEST:
				pline("You enter a disgusting nest!");
				break;
			case ANTHOLE:
				pline("You enter an anthole!");
				break;
			case BARRACKS:
				if (monstinroom(&mons[PM_SOLDIER], roomno) ||
				    monstinroom(&mons[PM_SERGEANT], roomno) ||
				    monstinroom(&mons[PM_LIEUTENANT], roomno) ||
				    monstinroom(&mons[PM_CAPTAIN], roomno))
					pline("You enter a military barracks!");
				else
					pline("You enter an abandoned barracks.");
				break;
			case DELPHI:
				if (monstinroom(&mons[PM_ORACLE], roomno))
					verbalize("%s, %s, welcome to Delphi!",
						  Hello(NULL), plname);
				break;
			case TEMPLE:
				intemple(roomno + ROOMOFFSET);
			/* fall through */
			default:
				rt = 0;
		}

		if (rt != 0) {
			rooms[roomno].rtype = OROOM;
			if (!search_special(rt)) {
				/* No more room of that type */
				switch (rt) {
					case COURT:
					case GIANTCOURT:
						level.flags.has_court = 0;
						break;
					case SWAMP:
						level.flags.has_swamp = 0;
						break;
					case MORGUE:
						level.flags.has_morgue = 0;
						break;
					case ZOO:
					case REALZOO:
						level.flags.has_zoo = 0;
						break;
					case BARRACKS:
						level.flags.has_barracks = 0;
						break;
					case TEMPLE:
						level.flags.has_temple = 0;
						break;
					case BEEHIVE:
						level.flags.has_beehive = 0;
						break;
					case LEMUREPIT:
						level.flags.has_lemurepit = 0;
						break;
					case MIGOHIVE:
						level.flags.has_migohive = 0;
						break;
					case FUNGUSFARM:
						level.flags.has_fungusfarm = 0;
						break;
				}
			}
			if (rt == COURT || rt == SWAMP || rt == MORGUE || rt == ZOO)
				for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
					if (!DEADMONSTER(mtmp) && !Stealth && !rn2(3)) mtmp->msleeping = 0;
		}
	}

	return;
}

int dopickup(void) {
	int count;
	struct trap *traphere = t_at(u.ux, u.uy);
	/* awful kludge to work around parse()'s pre-decrement */
	count = (multi || (save_cm && *save_cm == ',')) ? multi + 1 : 0;
	multi = 0; /* always reset */
	/* uswallow case added by GAN 01/29/87 */
	if (u.uswallow) {
		if (!u.ustuck->minvent) {
			if (is_animal(u.ustuck->data)) {
				pline("You pick up %s tongue.",
				      s_suffix(mon_nam(u.ustuck)));
				pline("But it's kind of slimy, so you drop it.");
			} else
				pline("You don't %s anything in here to pick up.",
				      Blind ? "feel" : "see");
			return 1;
		} else {
			int tmpcount = -count;
			return loot_mon(u.ustuck, &tmpcount, NULL);
		}
	}
	if (is_pool(u.ux, u.uy)) {
		if (Wwalking || is_floater(youmonst.data) || is_clinger(youmonst.data) || (Flying && !Breathless)) {
			pline("You cannot dive into the water to pick things up.");
			return 0;
		} else if (!Underwater) {
			pline("You can't even see the bottom, let alone pick something up.");
			return 0;
		}
	}
	if (is_lava(u.ux, u.uy)) {
		if (Wwalking || is_floater(youmonst.data) || is_clinger(youmonst.data) || (Flying && !Breathless)) {
			pline("You can't reach the bottom to pick things up.");
			return 0;
		} else if (!likes_lava(youmonst.data)) {
			pline("You would burn to a crisp trying to pick things up.");
			return 0;
		}
	}
	if (!OBJ_AT(u.ux, u.uy)) {
		pline("There is nothing here to pick up.");
		return 0;
	}
	if (!can_reach_floor()) {
		if (u.usteed && P_SKILL(P_RIDING) < P_BASIC)
			pline("You aren't skilled enough to reach from %s.",
			      y_monnam(u.usteed));
		else
			pline("You cannot reach the %s.", surface(u.ux, u.uy));

		return 0;
	}

	if (traphere && uteetering_at_seen_pit()) {
		/* Allow pickup from holes and trap doors that you escaped from
		 * because that stuff is teetering on the edge just like you, but
		 * not pits, because there is an elevation discrepancy with stuff
		 * in pits.
		 */
		pline("You cannot reach the bottom of the pit.");
		return 0;
	}

	return pickup(-count);
}

/* stop running if we see something interesting */
/* turn around a corner if that is the only way we can proceed */
/* do not turn left or right twice */
void lookaround(void) {
	int x, y, i, x0 = 0, y0 = 0, m0 = 1, i0 = 9;
	int corrct = 0, noturn = 0;
	struct monst *mtmp;
	struct trap *trap;

	/* Grid bugs stop if trying to move diagonal, even if blind.  Maybe */
	/* they polymorphed while in the middle of a long move. */
	if (u.umonnum == PM_GRID_BUG && u.dx && u.dy) {
		nomul(0);
		return;
	}

	if (Blind || context.run == 0) return;
	for (x = u.ux - 1; x <= u.ux + 1; x++)
		for (y = u.uy - 1; y <= u.uy + 1; y++) {
			if (!isok(x, y)) continue;

			if (u.umonnum == PM_GRID_BUG && x != u.ux && y != u.uy) continue;

			if (x == u.ux && y == u.uy) continue;

			if ((mtmp = m_at(x, y)) &&
			    mtmp->m_ap_type != M_AP_FURNITURE &&
			    mtmp->m_ap_type != M_AP_OBJECT &&
			    (!mtmp->minvis || See_invisible) && !mtmp->mundetected) {
				if ((context.run != 1 && !mtmp->mtame) || (x == u.ux + u.dx && y == u.uy + u.dy))
					goto stop;
			}

			if (levl[x][y].typ == STONE) continue;
			if (x == u.ux - u.dx && y == u.uy - u.dy) continue;

			if (IS_ROCK(levl[x][y].typ) || (levl[x][y].typ == ROOM) ||
			    IS_AIR(levl[x][y].typ))
				continue;
			else if (closed_door(x, y) ||
				 (mtmp && mtmp->m_ap_type == M_AP_FURNITURE &&
				  (mtmp->mappearance == S_hcdoor ||
				   mtmp->mappearance == S_vcdoor))) {
				if (x != u.ux && y != u.uy) continue;
				if (context.run != 1) goto stop;
				goto bcorr;
			} else if (levl[x][y].typ == CORR) {
			bcorr:
				if (levl[u.ux][u.uy].typ != ROOM) {
					if (context.run == 1 || context.run == 3 || context.run == 8) {
						i = dist2(x, y, u.ux + u.dx, u.uy + u.dy);
						if (i > 2) continue;
						if (corrct == 1 && dist2(x, y, x0, y0) != 1)
							noturn = 1;
						if (i < i0) {
							i0 = i;
							x0 = x;
							y0 = y;
							m0 = mtmp ? 1 : 0;
						}
					}
					corrct++;
				}
				continue;
			} else if ((trap = t_at(x, y)) && trap->tseen) {
				if (context.run == 1) goto bcorr; /* if you must */
				if (x == u.ux + u.dx && y == u.uy + u.dy) goto stop;
				continue;
			} else if (is_pool(x, y) || is_lava(x, y)) {
				/* water and lava only stop you if directly in front, and stop
				 * you even if you are running
				 */
				/* KMH, balance patch -- new intrinsic */
				if (!Levitation && !Flying && !is_clinger(youmonst.data) &&
				    x == u.ux + u.dx && y == u.uy + u.dy)
					/* No Wwalking check; otherwise they'd be able
					 * to test boots by trying to SHIFT-direction
					 * into a pool and seeing if the game allowed it
					 */
					goto stop;
				continue;
			} else { /* e.g. objects or trap or stairs */
				if (context.run == 1) goto bcorr;
				if (context.run == 8) continue;
				if (mtmp) continue; /* d */
				if (((x == u.ux - u.dx) && (y != u.uy + u.dy)) ||
				    ((y == u.uy - u.dy) && (x != u.ux + u.dx)))
					continue;
			}
		stop:
			nomul(0);
			return;
		} /* end for loops */

	if (corrct > 1 && context.run == 2) goto stop;
	if ((context.run == 1 || context.run == 3 || context.run == 8) &&
	    !noturn && !m0 && i0 && (corrct == 1 || (corrct == 2 && i0 == 1))) {
		/* make sure that we do not turn too far */
		if (i0 == 2) {
			if (u.dx == y0 - u.uy && u.dy == u.ux - x0)
				i = 2; /* straight turn right */
			else
				i = -2; /* straight turn left */
		} else if (u.dx && u.dy) {
			if ((u.dx == u.dy && y0 == u.uy) || (u.dx != u.dy && y0 != u.uy))
				i = -1; /* half turn left */
			else
				i = 1; /* half turn right */
		} else {
			if ((x0 - u.ux == y0 - u.uy && !u.dy) || (x0 - u.ux != y0 - u.uy && u.dy))
				i = 1; /* half turn right */
			else
				i = -1; /* half turn left */
		}

		i += u.last_str_turn;
		if (i <= 2 && i >= -2) {
			u.last_str_turn = i;
			u.dx = x0 - u.ux;
			u.dy = y0 - u.uy;
		}
	}
}

/* something like lookaround, but we are not running */
/* react only to monsters that might hit us */
int monster_nearby(void) {
	int x, y;
	struct monst *mtmp;

	/* Also see the similar check in dochugw() in monmove.c */
	for (x = u.ux - 1; x <= u.ux + 1; x++)
		for (y = u.uy - 1; y <= u.uy + 1; y++) {
			if (!isok(x, y)) continue;
			if (x == u.ux && y == u.uy) continue;
			if ((mtmp = m_at(x, y)) &&
			    mtmp->m_ap_type != M_AP_FURNITURE &&
			    mtmp->m_ap_type != M_AP_OBJECT &&
			    (!mtmp->mpeaceful || Hallucination) &&
			    (!is_hider(mtmp->data) || !mtmp->mundetected) &&
			    !noattacks(mtmp->data) &&
			    mtmp->mcanmove && !mtmp->msleeping && /* aplvax!jcn */
			    !onscary(u.ux, u.uy, mtmp) &&
			    canspotmon(mtmp))
				return 1;
		}
	return 0;
}

static void maybe_wail(void) {
	static short powers[] = {TELEPORT, SEE_INVIS, POISON_RES, COLD_RES,
				 SHOCK_RES, FIRE_RES, SLEEP_RES, DISINT_RES,
				 TELEPORT_CONTROL, STEALTH, FAST, INVIS};

	if (moves <= wailmsg + 50) return;

	wailmsg = moves;
	if (Role_if(PM_WIZARD) || Race_if(PM_ELF) || Role_if(PM_VALKYRIE)) {
		const char *who;
		int i, powercnt;

		who = (Role_if(PM_WIZARD) || Role_if(PM_VALKYRIE)) ?
			      urole.name.m :
			      "Elf";
		if (u.uhp == 1) {
			pline("%s is about to die.", who);
		} else {
			for (i = 0, powercnt = 0; i < SIZE(powers); ++i)
				if (u.uprops[powers[i]].intrinsic & INTRINSIC) ++powercnt;

			pline(powercnt >= 4 ? "%s, all your powers will be lost..." : "%s, your life force is running out.", who);
		}
	} else {
		You_hear(u.uhp == 1 ? "the wailing of the Banshee..." : "the howling of the CwnAnnwn...");
	}
}

void nomul(int nval) {
	if (multi < nval) return; /* This is a bug fix by ab@unido */
	u.uinvulnerable = false;  /* Kludge to avoid ctrl-C bug -dlc */
	u.usleep = 0;
	multi = nval;
	context.travel = context.travel1 = context.mv = false;
	context.run = 0;
}

/* called when a non-movement, multi-turn action has completed */
void unmul(const char *msg_override) {
	multi = 0; /* caller will usually have done this already */
	if (msg_override)
		nomovemsg = msg_override;
	else if (!nomovemsg)
		nomovemsg = "You can move again.";
	if (*nomovemsg) pline("%s", nomovemsg);
	nomovemsg = 0;
	u.usleep = 0;
	if (afternmv) (*afternmv)();
	afternmv = 0;
}

/* Print the amount of damage inflicted */
/* KMH -- Centralized to one function */
void showdmg(int n) {
	int lev;

	if (flags.showdmg && n > 1) {
		switch (Role_switch) {
			case PM_BARBARIAN:
			case PM_MONK:
				lev = 10;
				break;
			case PM_CAVEMAN:
			case PM_VALKYRIE:
				lev = 12;
				break;
			case PM_SAMURAI:
			case PM_KNIGHT:
				lev = 14;
				break;
			default:
				lev = 17;
				break;
		}
		switch (Race_switch) {
			case PM_GNOME:
				if (lev > 14) lev = 14;
				break;
		}
		if (wizard) lev = 1;
		if (u.ulevel >= lev)
			pline("(%d pts.)", n);
	}
	return;
}

void losehp(int n, const char *knam, int k_format /* WAC k_format is an int */) {
	/* [max] Invulnerable no dmg */
	if (Invulnerable) {
		n = 0;
		pline("You are unharmed!");
		/* NOTE: DO NOT RETURN - losehp is also called to check for death
		 * via u.uhp < 1
		 */
	} else if (flags.showdmg && n > 0)
		pline("[%d pts.]", n); /* WAC see damage */

	if (Upolyd) {
		u.mh -= n;
		if (u.mhmax < u.mh) u.mhmax = u.mh;
		if (u.mh < 1)
			rehumanize();
		else if (n > 0 && u.mh * 10 < u.mhmax && Unchanging)
			maybe_wail();
		return;
	} else {
		u.uhp -= n;
		if (u.uhp > u.uhpmax)
			u.uhpmax = u.uhp; /* perhaps n was negative */
	}

	context.botl = 1; /* Update status bar */

	if (u.uhp < 1) {
		killer.format = k_format;
		nhscopyz(&killer.name, knam); /* the thing that killed you */
		pline("You die...");
		done(DIED);
	} else if (n > 0 && u.uhp * 10 < u.uhpmax) {
		maybe_wail();
	}
}

int weight_cap(void) {
	long carrcap;

	carrcap = 25 * (ACURRSTR + ACURR(A_CON)) + 50;
	if (Upolyd) {
		/* consistent with can_carry() in mon.c */
		if (youmonst.data->mlet == S_NYMPH)
			carrcap = MAX_CARR_CAP;
		else if (!youmonst.data->cwt)
			carrcap = (carrcap * (long)youmonst.data->msize) / MZ_HUMAN;
		else if (!strongmonst(youmonst.data) || (strongmonst(youmonst.data) && (youmonst.data->cwt > WT_HUMAN)))
			carrcap = (carrcap * (long)youmonst.data->cwt / WT_HUMAN);
	}

	/* pugh@cornell */
	if (Levitation || Is_airlevel(&u.uz) || (u.usteed && strongmonst(u.usteed->data))) {
		carrcap = MAX_CARR_CAP;
	} else {
		if (carrcap > MAX_CARR_CAP) carrcap = MAX_CARR_CAP;
		if (!Flying) {
			if (EWounded_legs & LEFT_SIDE) carrcap -= 100;
			if (EWounded_legs & RIGHT_SIDE) carrcap -= 100;
		}
		if (carrcap < 0) carrcap = 0;
	}
	return (int)carrcap;
}

static int wc; /* current weight_cap(); valid after call to inv_weight() */

/* returns how far beyond the normal capacity the player is currently. */
/* inv_weight() is negative if the player is below normal capacity. */
int inv_weight(void) {
	struct obj *otmp = invent;
	int wt = 0;

	while (otmp) {
		if (otmp->oclass == COIN_CLASS)
			wt += (int)(((long)otmp->quan + 50L) / 100L);
		else if (otmp->otyp != BOULDER || !throws_rocks(youmonst.data))
			wt += otmp->owt;
		otmp = otmp->nobj;
	}
	wc = weight_cap();
	return wt - wc;
}

/*
 * Returns 0 if below normal capacity, or the number of "capacity units"
 * over the normal capacity the player is loaded.  Max is 5.
 */
int calc_capacity(int xtra_wt) {
	int cap, wt = inv_weight() + xtra_wt;

	if (wt <= 0) return UNENCUMBERED;
	if (wc <= 1) return OVERLOADED;
	cap = (wt * 2 / wc) + 1;
	return min(cap, OVERLOADED);
}

int near_capacity(void) {
	return calc_capacity(0);
}

int max_capacity(void) {
	int wt = inv_weight();

	return wt - (2 * wc);
}

boolean check_capacity(const char *str) {
	if (near_capacity() >= EXT_ENCUMBER) {
		if (str)
			plines(str);
		else
			pline("You can't do that while carrying so much stuff.");
		return true;
	}
	return false;
}

int inv_cnt(void) {
	struct obj *otmp = invent;
	int ct = 0;

	while (otmp) {
		ct++;
		otmp = otmp->nobj;
	}
	return ct;
}

/* Counts the money in an object chain. */
/* Intended use is for your or some monsters inventory, */
/* now that u.gold/m.gold is gone.*/
/* Counting money in a container might be possible too. */
long money_cnt(struct obj *otmp) {
	while (otmp) {
		/* Must change when silver & copper is implemented: */
		if (otmp->oclass == COIN_CLASS) return otmp->quan;
		otmp = otmp->nobj;
	}
	return 0;
}

/*hack.c*/
