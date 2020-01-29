/*	SCCS Id: @(#)shk.c	3.4	2003/12/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "eshk.h"

/*#define DEBUG*/

#define PAY_SOME  2
#define PAY_BUY	  1
#define PAY_CANT  0 /* too poor */
#define PAY_SKIP  (-1)
#define PAY_BROKE (-2)

static void makekops(coord *);
static void call_kops(struct monst *, boolean);
static void kops_gone(boolean);

#define IS_SHOP(x) (rooms[x].rtype >= SHOPBASE)
#define no_cheat   ((ACURR(A_CHA) - rnl(3)) > 7)

extern const struct shclass shtypes[]; /* defined in shknam.c */
extern struct obj *thrownobj;	       /* defined in dothrow.c */

static long int followmsg; /* last time of follow message */

static void setpaid(struct monst *);
static long addupbill(struct monst *);
static void pacify_shk(struct monst *);
static struct bill_x *onbill(struct obj *, struct monst *, boolean);
static struct monst *next_shkp(struct monst *, boolean);
static long shop_debt(struct eshk *);
static char *shk_owns(char *, struct obj *);
static char *mon_owns(char *, struct obj *);
static void clear_unpaid(struct obj *);
static long check_credit(long, struct monst *);
static void pay(long, struct monst *);
static long get_cost(struct obj *, struct monst *);
static long set_cost(struct obj *, struct monst *);
static const char *shk_embellish(struct obj *, long);
static long cost_per_charge(struct monst *, struct obj *, boolean);
static long cheapest_item(struct monst *);
static int dopayobj(struct monst *, struct bill_x *,
		    struct obj **, int, boolean);
static long stolen_container(struct obj *, struct monst *, long,
			     boolean, boolean);
static long getprice(struct obj *, boolean);
static void shk_names_obj(struct monst *, struct obj *, const char *, long, const char *);
static struct obj *bp_to_obj(struct bill_x *);
static boolean inherits(struct monst *, int, int);
static void set_repo_loc(struct eshk *);
static boolean angry_shk_exists(void);
static void rile_shk(struct monst *);
static void rouse_shk(struct monst *, boolean);
static void remove_damage(struct monst *shkp, bool croaked);
static void sub_one_frombill(struct obj *, struct monst *);
static int extend_bill(struct eshk *, int);
static void add_one_tobill(struct obj *, boolean);
static void dropped_container(struct obj *, struct monst *,
			      boolean);
static void add_to_billobjs(struct obj *);
static void bill_box_content(struct obj *, boolean, boolean,
			     struct monst *);
static boolean rob_shop(struct monst *);

#define NOBOUND (-1) /* No lower/upper limit to charge       */
static void shk_other_services(void);
static void shk_identify(char *, struct monst *);
static void shk_uncurse(char *, struct monst *);
static void shk_appraisal(char *, struct monst *);
static void shk_weapon_works(char *, struct monst *);
static void shk_armor_works(char *, struct monst *);
static void shk_charge(char *, struct monst *);
static boolean shk_obj_match(struct obj *, struct monst *);
/*static int shk_class_match(long class, struct monst *shkp);*/
static boolean shk_offer_price(char *, long, struct monst *);
static void shk_smooth_charge(int *, int, int);

/*
	invariants: obj->unpaid iff onbill(obj) [unless bp->useup]
		obj->quan <= bp->bquan
 */

/*
    Transfer money from inventory to monster when paying
    shopkeepers, priests, oracle, succubus, & other demons.
    Simple with only gold coins.
    This routine will handle money changing when multiple
    coin types is implemented, only appropriate
    monsters will pay change.  (Peaceful shopkeepers, priests
    & the oracle try to maintain goodwill while selling
    their wares or services.  Angry monsters and all demons
    will keep anything they get their hands on.
    Returns the amount actually paid, so we can know
    if the monster kept the change.
 */
long money2mon(struct monst *mon, long amount) {
	struct obj *ygold = findgold(invent);

	if (amount <= 0) {
		impossible("%s payment in money2mon!", amount ? "negative" : "zero");
		return 0L;
	}
	if (!ygold || ygold->quan < amount) {
		impossible("Paying without %s money?", ygold ? "enough" : "");
		return 0L;
	}

	if (ygold->quan > amount)
		ygold = splitobj(ygold, amount);
	else if (ygold->owornmask)
		remove_worn_item(ygold, false); /* quiver */
	freeinv(ygold);
	add_to_minv(mon, ygold);
	context.botl = 1;
	return amount;
}

/*
    Transfer money from monster to inventory.
    Used when the shopkeeper pay for items, and when
    the priest gives you money for an ale.
 */
void money2u(struct monst *mon, long amount) {
	struct obj *mongold = findgold(mon->minvent);

	if (amount <= 0) {
		impossible("%s payment in money2u!", amount ? "negative" : "zero");
		return;
	}
	if (!mongold || mongold->quan < amount) {
		impossible("%s paying without %s money?", a_monnam(mon),
			   mongold ? "enough" : "");
		return;
	}

	if (mongold->quan > amount) mongold = splitobj(mongold, amount);
	obj_extract_self(mongold);

	if (!merge_choice(invent, mongold) && inv_cnt() >= 52) {
		pline("You have no room for the money!");
		dropy(mongold);
	} else {
		addinv(mongold);
		context.botl = 1;
	}
}

static struct monst *next_shkp(struct monst *shkp, boolean withbill) {
	for (; shkp; shkp = shkp->nmon) {
		if (DEADMONSTER(shkp)) continue;
		if (shkp->isshk && (ESHK(shkp)->billct || !withbill)) break;
	}

	if (shkp) {
		if (NOTANGRY(shkp)) {
			if (ESHK(shkp)->surcharge) pacify_shk(shkp);
		} else {
			if (!ESHK(shkp)->surcharge) rile_shk(shkp);
		}
	}
	return shkp;
}

char *shkname(struct monst *mtmp) {
	return ESHK(mtmp)->shknam;
}

void shkgone(struct monst *mtmp) {
	struct eshk *eshk = ESHK(mtmp);
	struct mkroom *sroom = &rooms[eshk->shoproom - ROOMOFFSET];
	struct obj *otmp;
	char *p;
	int sx, sy;

	/* [BUG: some of this should be done on the shop level */
	/*       even when the shk dies on a different level.] */
	if (on_level(&eshk->shoplevel, &u.uz)) {
		remove_damage(mtmp, true);
		sroom->resident = NULL;
		if (!search_special(ANY_SHOP))
			level.flags.has_shop = 0;

		/* items on shop floor revert to ordinary objects */
		for (sx = sroom->lx; sx <= sroom->hx; sx++)
			for (sy = sroom->ly; sy <= sroom->hy; sy++)
				for (otmp = level.objects[sx][sy]; otmp; otmp = otmp->nexthere)
					otmp->no_charge = 0;

		/* Make sure bill is set only when the
		   dead shk is the resident shk. */
		if ((p = index(u.ushops, eshk->shoproom)) != 0) {
			setpaid(mtmp);
			free(eshk->bill_p);
			eshk->billsz = 0;
			eshk->bill_p = NULL;
			/* remove eshk->shoproom from u.ushops */
			do {
				*p = *(p + 1);
			} while (*++p);
		}
	}
}

void set_residency(struct monst *shkp, boolean zero_out) {
	if (on_level(&(ESHK(shkp)->shoplevel), &u.uz))
		rooms[ESHK(shkp)->shoproom - ROOMOFFSET].resident =
			(zero_out) ? NULL : shkp;
}

void replshk(struct monst *mtmp, struct monst *mtmp2) {
	rooms[ESHK(mtmp2)->shoproom - ROOMOFFSET].resident = mtmp2;
}

void save_shk_bill(int fd, struct monst *shkp) {
	struct eshk *eshkp = ESHK(shkp);

	if (eshkp->bill_p && eshkp->bill_p != (struct bill_x *)-1000 &&
	    eshkp->billct)
		bwrite(fd, (void *)eshkp->bill_p,
		       eshkp->billct * sizeof(struct bill_x));
}

void restore_shk_bill(int fd, struct monst *shkp) {
	struct eshk *eshkp = ESHK(shkp);

	if (eshkp->bill_p && eshkp->bill_p != (struct bill_x *)-1000) {
		eshkp->billsz = eshkp->billct;
		if (eshkp->billsz) {
			eshkp->bill_p = (struct bill_x *)
				alloc(eshkp->billsz * sizeof(struct bill_x));
			mread(fd, (void *)eshkp->bill_p,
			      eshkp->billct * sizeof(struct bill_x));
		} else
			eshkp->bill_p = NULL;
	}
}

/* do shopkeeper specific structure munging -dlc */
void restshk(struct monst *shkp, boolean ghostly) {
	if (u.uz.dlevel) {
		struct eshk *eshkp = ESHK(shkp);

		/* shoplevel can change as dungeons move around */
		/* savebones guarantees that non-homed shk's will be gone */
		if (ghostly) {
			assign_level(&eshkp->shoplevel, &u.uz);
			if (ANGRY(shkp) && strncmpi(eshkp->customer, plname, PL_NSIZ))
				pacify_shk(shkp);
		}
	}
}

/* Clear the unpaid bit on all of the objects in the list. */
static void clear_unpaid(struct obj *list) {
	while (list) {
		if (Has_contents(list)) clear_unpaid(list->cobj);
		list->unpaid = 0;
		list = list->nobj;
	}
}

/* either you paid or left the shop or the shopkeeper died */
static void setpaid(struct monst *shkp) {
	struct obj *obj;
	struct monst *mtmp;

	/* FIXME: object handling should be limited to
	   items which are on this particular shk's bill */

	clear_unpaid(invent);
	clear_unpaid(fobj);
	clear_unpaid(level.buriedobjlist);
	if (thrownobj) thrownobj->unpaid = 0;
	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon)
		clear_unpaid(mtmp->minvent);
	for (mtmp = migrating_mons; mtmp; mtmp = mtmp->nmon)
		clear_unpaid(mtmp->minvent);

	while ((obj = billobjs) != 0) {
		obj_extract_self(obj);
		dealloc_obj(obj);
	}
	if (shkp) {
		ESHK(shkp)->billct = 0;
		ESHK(shkp)->credit = 0L;
		ESHK(shkp)->debit = 0L;
		ESHK(shkp)->loan = 0L;
	}
}

static long addupbill(struct monst *shkp) {
	int ct = ESHK(shkp)->billct;
	struct bill_x *bp = ESHK(shkp)->bill_p;
	long total = 0L;

	while (ct--) {
		total += bp->price * bp->bquan;
		bp++;
	}
	return total;
}

static void call_kops(struct monst *shkp, boolean nearshop) {
	/* Keystone Kops srt@ucla */
	boolean nokops;
	char kopname[20];

	strcpy(kopname, "Keystone Kops");

	if (!shkp) return;

	if (!Deaf)
		pline("An alarm sounds!");

	nokops = ((mvitals[PM_KEYSTONE_KOP].mvflags & G_GONE) &&
		  (mvitals[PM_KOP_SERGEANT].mvflags & G_GONE) &&
		  (mvitals[PM_KOP_LIEUTENANT].mvflags & G_GONE) &&
		  (mvitals[PM_KOP_KAPTAIN].mvflags & G_GONE));

	if (Is_blackmarket(&u.uz)) {
		nokops = ((mvitals[PM_SOLDIER].mvflags & G_GONE) &&
			  (mvitals[PM_SERGEANT].mvflags & G_GONE) &&
			  (mvitals[PM_LIEUTENANT].mvflags & G_GONE) &&
			  (mvitals[PM_CAPTAIN].mvflags & G_GONE));

		strcpy(kopname, "guards");
	}

	if (!angry_guards(Deaf) && nokops) {
		if (flags.verbose && !Deaf)
			pline("But no one seems to respond to it.");
		return;
	}

	if (nokops) return;

	{
		coord mm;

		if (nearshop && (!Is_blackmarket(&u.uz))) {
			/* Create swarm around you, if you merely "stepped out" */
			if (flags.verbose)
				pline("The %s appear!", kopname);
			mm.x = u.ux;
			mm.y = u.uy;
			makekops(&mm);
			return;
		}
		if (flags.verbose)
			pline("The %s are after you!", kopname);
		/* Create swarm near down staircase (hinders return to level) */
		if (Is_blackmarket(&u.uz)) {
			struct trap *trap = ftrap;
			while (trap) {
				if (trap->ttyp == MAGIC_PORTAL) {
					mm.x = trap->tx;
					mm.y = trap->ty;
				}
				trap = trap->ntrap;
			}
		} else {
			mm.x = xdnstair;
			mm.y = ydnstair;
		}
		makekops(&mm);
		/* Create swarm near shopkeeper (hinders return to shop) */
		mm.x = shkp->mx;
		mm.y = shkp->my;
		makekops(&mm);
	}
}

void blkmar_guards(struct monst *shkp) {
	struct monst *mt;
	struct eshk *eshkp = ESHK(shkp);
	boolean mesg_given = false;   /* Only give message if assistants peaceful */
	static boolean rlock = false; /* Prevent recursive calls (via wakeup) */

	if (rlock) return;
	rlock = true;

	/* wake up assistants */
	for (mt = fmon; mt; mt = mt->nmon) {
		if (DEADMONSTER(mt)) continue;
		/* non-tame named monsters are presumably
		 * black marketeer's assistants */
		if (!mt->mtame && NAME(mt) && *NAME(mt) && mt->mpeaceful &&
		    mt != shkp && inside_shop(mt->mx, mt->my) == eshkp->shoproom) {
			if (!mesg_given) {
				pline("%s calls for %s assistants!",
				      noit_Monnam(shkp), mhis(shkp));
				mesg_given = true;
			}
			wakeup(mt);
		}
	}
	rlock = false;
}

/* x,y is strictly inside shop */
char inside_shop(xchar x, xchar y) {
	char rno;

	rno = levl[x][y].roomno;
	if ((rno < ROOMOFFSET) || levl[x][y].edge || !IS_SHOP(rno - ROOMOFFSET))
		return NO_ROOM;
	else
		return rno;
}

void u_left_shop(char *leavestring, boolean newlev) {
	struct monst *shkp;
	struct eshk *eshkp;

	/*
	 * IF player
	 * ((didn't leave outright) AND
	 *  ((he is now strictly-inside the shop) OR
	 *   (he wasn't strictly-inside last turn anyway)))
	 * THEN (there's nothing to do, so just return)
	 */
	if (!*leavestring &&
	    (!levl[u.ux][u.uy].edge || levl[u.ux0][u.uy0].edge))
		return;

	shkp = shop_keeper(*u.ushops0);
	if (!shkp || !inhishop(shkp))
		return; /* shk died, teleported, changed levels... */

	eshkp = ESHK(shkp);
	if (!eshkp->billct && !eshkp->debit) /* bill is settled */
		return;

	if (!*leavestring && shkp->mcanmove && !shkp->msleeping) {
		/*
		 * Player just stepped onto shop-boundary (known from above logic).
		 * Try to intimidate him into paying his bill
		 */
		verbalize(NOTANGRY(shkp) ?
				  "%s!  Please pay before leaving." :
				  "%s!  Don't you leave without paying!",
			  plname);
		return;
	}

	if (rob_shop(shkp)) {
		if (Is_blackmarket(&u.uz))
			blkmar_guards(shkp);

		call_kops(shkp, (!newlev && levl[u.ux0][u.uy0].edge));
	}
}

/* robbery from outside the shop via telekinesis or grappling hook */
void remote_burglary(xchar x, xchar y) {
	struct monst *shkp;
	struct eshk *eshkp;

	shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));
	if (!shkp || !inhishop(shkp))
		return; /* shk died, teleported, changed levels... */

	eshkp = ESHK(shkp);
	if (!eshkp->billct && !eshkp->debit) /* bill is settled */
		return;

	if (rob_shop(shkp)) {
		if (Is_blackmarket(&u.uz))
			blkmar_guards(shkp);

		/*[might want to set 2nd arg based on distance from shop doorway]*/
		call_kops(shkp, false);
	}
}

/* shop merchandise has been taken; pay for it with any credit available;
   return false if the debt is fully covered by credit, true otherwise */
static boolean rob_shop(struct monst *shkp) {
	struct eshk *eshkp;
	long total;

	eshkp = ESHK(shkp);
	rouse_shk(shkp, true);
	total = (addupbill(shkp) + eshkp->debit);
	if (eshkp->credit >= total) {
		pline("Your credit of %ld %s is used to cover your shopping bill.",
		      eshkp->credit, currency(eshkp->credit));
		total = 0L; /* credit gets cleared by setpaid() */
	} else {
		pline("You escaped the shop without paying!");
		total -= eshkp->credit;
	}
	setpaid(shkp);
	if (!total) return false;

	/* by this point, we know an actual robbery has taken place */
	eshkp->robbed += total;
	pline("You stole %ld %s worth of merchandise.",
	      total, currency(total));
	if (!Role_if(PM_ROGUE)) { /* stealing is unlawful */
		adjalign(-sgn(u.ualign.type));
		pline("You feel like an evil rogue.");
	}

	hot_pursuit(shkp);
	return true;
}

void u_entered_shop(char *enterstring) {
	int rt;
	struct monst *shkp;
	struct eshk *eshkp;
	static const char no_shk[] = "This shop appears to be deserted.";
	static char empty_shops[5];

	if (!*enterstring)
		return;

	if (!(shkp = shop_keeper(*enterstring))) {
		if (!index(empty_shops, *enterstring) &&
		    in_rooms(u.ux, u.uy, SHOPBASE) !=
			    in_rooms(u.ux0, u.uy0, SHOPBASE))
			pline(no_shk);
		strcpy(empty_shops, u.ushops);
		u.ushops[0] = '\0';
		return;
	}

	eshkp = ESHK(shkp);

	if (!inhishop(shkp)) {
		/* dump core when referenced */
		eshkp->bill_p = (struct bill_x *)-1000;
		if (!index(empty_shops, *enterstring))
			pline(no_shk);
		strcpy(empty_shops, u.ushops);
		u.ushops[0] = '\0';
		return;
	}

	eshkp->billsz = 0;
	eshkp->bill_p = NULL;

	if ((!eshkp->visitct || *eshkp->customer) &&
	    strncmpi(eshkp->customer, plname, PL_NSIZ)) {
		/* You seem to be new here */
		eshkp->visitct = 0;
		eshkp->following = 0;
		strncpy(eshkp->customer, plname, PL_NSIZ);
		pacify_shk(shkp);
	}

	if (shkp->msleeping || !shkp->mcanmove || eshkp->following)
		return; /* no dialog */

	if (Invis) {
		pline("%s senses your presence.", shkname(shkp));
		if (!Is_blackmarket(&u.uz)) {
			verbalize("Invisible customers are not welcome!");
			return;
		}
	}

	if (Is_blackmarket(&u.uz) &&
	    u.umonnum > 0 && mons[u.umonnum].mlet != S_HUMAN) {
		verbalize("Non-human customers are not welcome!");
		return;
	}

	rt = rooms[*enterstring - ROOMOFFSET].rtype;

	if (ANGRY(shkp)) {
		verbalize("So, %s, you dare return to %s %s?!",
			  plname,
			  s_suffix(shkname(shkp)),
			  shtypes[rt - SHOPBASE].name);
	} else if (eshkp->robbed) {
		pline("%s mutters imprecations against shoplifters.", shkname(shkp));
	} else {
		verbalize("%s, %s!  Welcome%s to %s %s!",
			  Hello(shkp), plname,
			  eshkp->visitct++ ? " again" : "",
			  s_suffix(shkname(shkp)),
			  shtypes[rt - SHOPBASE].name);
	}
	/* can't do anything about blocking if teleported in */
	if (!inside_shop(u.ux, u.uy)) {
		boolean should_block;
		int cnt;
		const char *tool;
		struct obj *pick = carrying(PICK_AXE),
			   *mattock = carrying(DWARVISH_MATTOCK);

		if (pick || mattock) {
			cnt = 1;
			if (pick && mattock) { /* carrying both types */
				tool = "digging tool";
				cnt = 2; /* `more than 1' is all that matters */
			} else if (pick) {
				tool = "pick-axe";
				/* hack: `pick' already points somewhere into inventory */
				while ((pick = pick->nobj) != 0)
					if (pick->otyp == PICK_AXE) ++cnt;
			} else { /* assert(mattock != 0) */
				tool = "mattock";
				while ((mattock = mattock->nobj) != 0)
					if (mattock->otyp == DWARVISH_MATTOCK) ++cnt;
				/* [ALI] Shopkeeper indicates mattock(s) */
				if (!Blind) makeknown(DWARVISH_MATTOCK);
			}
			verbalize(NOTANGRY(shkp) ?
					  "Will you please leave your %s%s outside?" :
					  "Leave the %s%s outside.",
				  tool, plur(cnt));
			should_block = true;
		} else if (u.usteed) {
			verbalize(NOTANGRY(shkp) ?
					  "Will you please leave %s outside?" :
					  "Leave %s outside.",
				  y_monnam(u.usteed));
			should_block = true;
		} else {
			should_block = (Fast && (sobj_at(PICK_AXE, u.ux, u.uy) ||
						 sobj_at(DWARVISH_MATTOCK, u.ux, u.uy)));
		}
		if (should_block) dochug(shkp); /* shk gets extra move */
	}
	return;
}

/*
   Decide whether two unpaid items are mergable; caller is responsible for
   making sure they're unpaid and the same type of object; we check the price
   quoted by the shopkeeper and also that they both belong to the same shk.
 */
boolean same_price(struct obj *obj1, struct obj *obj2) {
	struct monst *shkp1, *shkp2;
	struct bill_x *bp1 = 0, *bp2 = 0;
	boolean are_mergable = false;

	/* look up the first object by finding shk whose bill it's on */
	for (shkp1 = next_shkp(fmon, true); shkp1;
	     shkp1 = next_shkp(shkp1->nmon, true))
		if ((bp1 = onbill(obj1, shkp1, true)) != 0) break;
	/* second object is probably owned by same shk; if not, look harder */
	if (shkp1 && (bp2 = onbill(obj2, shkp1, true)) != 0) {
		shkp2 = shkp1;
	} else {
		for (shkp2 = next_shkp(fmon, true); shkp2;
		     shkp2 = next_shkp(shkp2->nmon, true))
			if ((bp2 = onbill(obj2, shkp2, true)) != 0) break;
	}

	if (!bp1 || !bp2)
		impossible("same_price: object wasn't on any bill!");
	else
		are_mergable = (shkp1 == shkp2 && bp1->price == bp2->price);
	return are_mergable;
}

/*
 * Figure out how much is owed to a given shopkeeper.
 * At present, we ignore any amount robbed from the shop, to avoid
 * turning the `$' command into a way to discover that the current
 * level is bones data which has a shk on the warpath.
 */
static long shop_debt(struct eshk *eshkp) {
	struct bill_x *bp;
	int ct;
	long debt = eshkp->debit;

	for (bp = eshkp->bill_p, ct = eshkp->billct; ct > 0; bp++, ct--)
		debt += bp->price * bp->bquan;
	return debt;
}

/* called in response to the `$' command */
void shopper_financial_report(void) {
	struct monst *shkp, *this_shkp = shop_keeper(inside_shop(u.ux, u.uy));
	struct eshk *eshkp;
	long amt;
	int pass;

	if (this_shkp &&
	    !(ESHK(this_shkp)->credit || shop_debt(ESHK(this_shkp)))) {
		pline("You have no credit or debt in here.");
		this_shkp = 0; /* skip first pass */
	}

	/* pass 0: report for the shop we're currently in, if any;
	   pass 1: report for all other shops on this level. */
	for (pass = this_shkp ? 0 : 1; pass <= 1; pass++)
		for (shkp = next_shkp(fmon, false);
		     shkp;
		     shkp = next_shkp(shkp->nmon, false)) {
			if ((shkp != this_shkp) ^ pass) continue;
			eshkp = ESHK(shkp);
			if ((amt = eshkp->credit) != 0)
				pline("You have %ld %s credit at %s %s.",
				      amt, currency(amt), s_suffix(shkname(shkp)),
				      shtypes[eshkp->shoptype - SHOPBASE].name);
			else if (shkp == this_shkp)
				pline("You have no credit in here.");
			if ((amt = shop_debt(eshkp)) != 0)
				pline("You owe %s %ld %s.",
				      shkname(shkp), amt, currency(amt));
			else if (shkp == this_shkp)
				pline("You don't owe any money here.");
		}
}

int inhishop(struct monst *mtmp) {
	return index(in_rooms(mtmp->mx, mtmp->my, SHOPBASE),
		     ESHK(mtmp)->shoproom) &&
	       on_level(&(ESHK(mtmp)->shoplevel), &u.uz);
}

struct monst *shop_keeper(char rmno) {
	struct monst *shkp = rmno >= ROOMOFFSET ?
				     rooms[rmno - ROOMOFFSET].resident :
				     0;

	if (shkp) {
		if (NOTANGRY(shkp)) {
			if (ESHK(shkp)->surcharge) pacify_shk(shkp);
		} else {
			if (!ESHK(shkp)->surcharge) rile_shk(shkp);
		}
	}
	return shkp;
}

boolean tended_shop(struct mkroom *sroom) {
	struct monst *mtmp = sroom->resident;

	if (!mtmp)
		return false;
	else
		return inhishop(mtmp);
}

static struct bill_x *onbill(struct obj *obj, struct monst *shkp, boolean silent) {
	if (shkp) {
		struct bill_x *bp = ESHK(shkp)->bill_p;
		int ct = ESHK(shkp)->billct;

		while (--ct >= 0)
			if (bp->bo_id == obj->o_id) {
				if (!obj->unpaid) pline("onbill: paid obj on bill?");
				return bp;
			} else
				bp++;
	}
	if (obj->unpaid & !silent) pline("onbill: unpaid obj not on bill?");
	return NULL;
}

/* Delete the contents of the given object. */
void delete_contents(struct obj *obj) {
	struct obj *curr;

	while ((curr = obj->cobj) != 0) {
		if (Has_contents(curr)) delete_contents(curr);
		obj_extract_self(curr);
		if (evades_destruction(curr)) {
			switch (obj->where) {
				case OBJ_FREE:
				case OBJ_ONBILL:
					impossible("indestructible object %s",
						   obj->where == OBJ_FREE ? "free" : "on bill");
					obfree(curr, NULL);
					break;
				case OBJ_FLOOR:
					place_object(curr, obj->ox, obj->oy);
					/* No indestructible objects currently stack */
					break;
				case OBJ_CONTAINED:
					add_to_container(obj->ocontainer, curr);
					break;
				case OBJ_INVENT:
					if (!flooreffects(curr, u.ux, u.uy, "fall"))
						place_object(curr, u.ux, u.uy);
					break;
				case OBJ_MINVENT:
					if (!flooreffects(curr,
							  obj->ocarry->mx, obj->ocarry->my, "fall"))
						place_object(curr, obj->ocarry->mx, obj->ocarry->my);
					break;
				case OBJ_MIGRATING:
					add_to_migration(curr);
					/* Copy migration destination */
					curr->ox = obj->ox;
					curr->oy = obj->oy;
					curr->owornmask = obj->owornmask;
					break;
				case OBJ_BURIED:
					add_to_buried(curr);
					curr->ox = obj->ox;
					curr->oy = obj->oy;
					break;
				default:
					panic("delete_contents");
					break;
			}
		} else
			obfree(curr, NULL);
	}
}

/* called with two args on merge */
void obfree(struct obj *obj, struct obj *merge) {
	struct bill_x *bp;
	struct bill_x *bpm;
	struct monst *shkp;

	if (obj == usaddle) dismount_steed(DISMOUNT_GENERIC);
	if (obj->otyp == LEASH && obj->leashmon) o_unleash(obj);
	if (obj->oclass == SPBOOK_CLASS) book_disappears(obj);
	if (obj->oclass == FOOD_CLASS) food_disappears(obj);
	/* [ALI] Enforce new rules: Containers must have their contents
	 * deleted while still in situ so that we can place any
	 * indestructible objects they may contain.
	 */
	if (Has_contents(obj)) {
		FILE *fp;
		int known;
		xchar x, y;
		struct obj *otmp;
		pline("BUG: obfree() called on non-empty container.  See buglog for details.");
		fp = fopen("buglog", "a");
		if (fp) {
			fprintf(fp,
				"%08ld: BUG: obfree() called on non-empty container.\n",
				yyyymmdd((time_t)0L));
			known = objects[obj->otyp].oc_name_known;
			objects[obj->otyp].oc_name_known = 1;
			obj->known = obj->bknown = obj->dknown = obj->rknown = 1;
			fprintf(fp, "Container: %s\n", doname(obj));
			objects[obj->otyp].oc_name_known = known;
			fprintf(fp, "ID: %d\n", obj->o_id);
			fprintf(fp, "Contents of %s:\n", the(xname(obj)));
			for (otmp = obj->cobj; otmp; otmp = obj->nobj) {
				known = objects[otmp->otyp].oc_name_known;
				objects[otmp->otyp].oc_name_known = 1;
				otmp->known = otmp->bknown =
					otmp->dknown = otmp->rknown = 1;
				fprintf(fp, "\t%s\n", doname(otmp));
				objects[otmp->otyp].oc_name_known = known;
			}
			switch (obj->where) {
				case OBJ_FREE:
					fprintf(fp, "Container is on free list.\n");
					break;
				case OBJ_FLOOR:
					fprintf(fp,
						"Container is on the floor at (%d, %d)\n",
						obj->ox, obj->oy);
					break;
				case OBJ_CONTAINED:
					otmp = obj->ocontainer;
					known = objects[otmp->otyp].oc_name_known;
					objects[otmp->otyp].oc_name_known = 1;
					otmp->known = otmp->bknown =
						otmp->dknown = otmp->rknown = 1;
					get_obj_location(otmp, &x, &y,
							 BURIED_TOO | CONTAINED_TOO);
					fprintf(fp,
						"Container is contained in %s at (%d, %d)\n",
						doname(otmp), x, y);
					objects[otmp->otyp].oc_name_known = known;
					break;
				case OBJ_INVENT:
					fprintf(fp,
						"Container is in hero's inventory\n");
					break;
				case OBJ_MINVENT:
					get_obj_location(otmp, &x, &y, 0);
					fprintf(fp,
						"Container is in %s's inventory at (%d, %d)\n",
						s_suffix(noit_mon_nam(obj->ocarry)), x, y);
					break;
				case OBJ_MIGRATING:
					fprintf(fp,
						"Container is migrating to level %d of %s\n",
						otmp->oy, dungeons[otmp->ox].dname);
					break;
				case OBJ_BURIED:
					fprintf(fp, "Container is buried at (%d, %d)\n",
						obj->ox, obj->oy);
					break;
				case OBJ_ONBILL:
					fprintf(fp, "Container is on shopping bill.\n");
					break;
				default:
					fprintf(fp,
						"Container is nowhere (%d).\n", obj->where);
					break;
			}
			fprintf(fp, "\n");
			fclose(fp);
		}
	}
	if (Has_contents(obj)) delete_contents(obj);

	shkp = 0;
	if (obj->unpaid) {
		/* look for a shopkeeper who owns this object */
		for (shkp = next_shkp(fmon, true); shkp;
		     shkp = next_shkp(shkp->nmon, true))
			if (onbill(obj, shkp, true)) break;
	}
	/* sanity check, more or less */
	if (!shkp) shkp = shop_keeper(*u.ushops);
	/*
	 * Note:  `shkp = shop_keeper(*u.ushops)' used to be
	 *	  unconditional.  But obfree() is used all over
	 *	  the place, so making its behavior be dependent
	 *	  upon player location doesn't make much sense.
	 */

	if ((bp = onbill(obj, shkp, false)) != 0) {
		if (!merge) {
			bp->useup = 1;
			obj->unpaid = 0; /* only for doinvbill */
			add_to_billobjs(obj);
			return;
		}
		bpm = onbill(merge, shkp, false);
		if (!bpm) {
			/* this used to be a rename */
			impossible("obfree: not on bill??");
			return;
		} else {
			/* this was a merger */
			bpm->bquan += bp->bquan;
			ESHK(shkp)->billct--;
			*bp = ESHK(shkp)->bill_p[ESHK(shkp)->billct];
		}
	}

	if (obj == uwep)
		uwepgone();
	else if (obj == uswapwep)
		uswapwepgone();
	else if (obj == uquiver)
		uqwepgone();
	dealloc_obj(obj);
}

void shk_free(struct monst *shkp) {
	if (ESHK(shkp)->bill_p && ESHK(shkp)->bill_p != (struct bill_x *)-1000)
		free(ESHK(shkp)->bill_p);
	free(shkp);
}

static long check_credit(long tmp, struct monst *shkp) {
	long credit = ESHK(shkp)->credit;

	if (credit == 0L) return tmp;
	if (credit >= tmp) {
		pline("The price is deducted from your credit.");
		ESHK(shkp)->credit -= tmp;
		tmp = 0L;
	} else {
		pline("The price is partially covered by your credit.");
		ESHK(shkp)->credit = 0L;
		tmp -= credit;
	}
	return tmp;
}

static void pay(long tmp, struct monst *shkp) {
	long robbed = ESHK(shkp)->robbed;
	long balance = ((tmp <= 0L) ? tmp : check_credit(tmp, shkp));

	if (balance > 0)
		money2mon(shkp, balance);
	else if (balance < 0)
		money2u(shkp, -balance);

	context.botl = 1;
	if (robbed) {
		robbed -= tmp;
		if (robbed < 0) robbed = 0L;
		ESHK(shkp)->robbed = robbed;
	}
}

/* return shkp to home position */
void home_shk(struct monst *shkp, boolean killkops) {
	xchar x = ESHK(shkp)->shk.x, y = ESHK(shkp)->shk.y;

	mnearto(shkp, x, y, true);
	level.flags.has_shop = 1;
	if (killkops) {
		kops_gone(true);

		pacify_guards();
	}
	after_shk_move(shkp);
}

static boolean angry_shk_exists() {
	struct monst *shkp;

	for (shkp = next_shkp(fmon, false);
	     shkp;
	     shkp = next_shkp(shkp->nmon, false))
		if (ANGRY(shkp)) return true;
	return false;
}

/* remove previously applied surcharge from all billed items */
static void pacify_shk(struct monst *shkp) {
	NOTANGRY(shkp) = true; /* make peaceful */
	if (ESHK(shkp)->surcharge) {
		struct bill_x *bp = ESHK(shkp)->bill_p;
		int ct = ESHK(shkp)->billct;

		ESHK(shkp)->surcharge = false;
		while (ct-- > 0) {
			long reduction = (bp->price + 3L) / 4L;
			bp->price -= reduction; /* undo 33% increase */
			bp++;
		}
	}
}

/* add aggravation surcharge to all billed items */
static void rile_shk(struct monst *shkp) {
	NOTANGRY(shkp) = false; /* make angry */
	if (!ESHK(shkp)->surcharge) {
		struct bill_x *bp = ESHK(shkp)->bill_p;
		int ct = ESHK(shkp)->billct;

		ESHK(shkp)->surcharge = true;
		while (ct-- > 0) {
			long surcharge = (bp->price + 2L) / 3L;
			bp->price += surcharge;
			bp++;
		}
	}
}

/* wakeup and/or unparalyze shopkeeper */
static void rouse_shk(struct monst *shkp, boolean verbosely) {
	if (!shkp->mcanmove || shkp->msleeping) {
		/* greed induced recovery... */
		if (verbosely && canspotmon(shkp))
			pline("%s %s.", Monnam(shkp),
			      shkp->msleeping ? "wakes up" : "can move again");
		shkp->msleeping = 0;
		shkp->mfrozen = 0;
		shkp->mcanmove = 1;
	}
}

void make_happy_shk(struct monst *shkp, boolean silentkops) {
	boolean wasmad = ANGRY(shkp);
	struct eshk *eshkp = ESHK(shkp);
	boolean guilty = wasmad ||
			 eshkp->surcharge || eshkp->following || eshkp->robbed;

	pacify_shk(shkp);
	eshkp->following = 0;
	eshkp->robbed = 0L;
	if (guilty && !Role_if(PM_ROGUE)) {
		adjalign(sgn(u.ualign.type));
		pline("You feel your guilt vanish.");
	}
	if (!inhishop(shkp)) {
		char shk_nam[BUFSZ];
		boolean vanished = canseemon(shkp);

		strcpy(shk_nam, mon_nam(shkp));
		if (on_level(&eshkp->shoplevel, &u.uz)) {
			home_shk(shkp, false);
			/* didn't disappear if shk can still be seen */
			if (canseemon(shkp)) vanished = false;
		} else {
			/* if sensed, does disappear regardless whether seen */
			if (sensemon(shkp)) vanished = true;
			/* can't act as porter for the Amulet, even if shk
			   happens to be going farther down rather than up */
			mdrop_special_objs(shkp);
			/* arrive near shop's door */
			migrate_to_level(shkp, ledger_no(&eshkp->shoplevel),
					 MIGR_APPROX_XY, &eshkp->shd);
		}
		if (vanished)
			pline("Satisfied, %s suddenly disappears!", shk_nam);
	} else if (wasmad)
		pline("%s calms down.", Monnam(shkp));

	if (!angry_shk_exists()) {
		kops_gone(silentkops);

		pacify_guards();
	}
}

void hot_pursuit(struct monst *shkp) {
	if (!shkp->isshk) return;

	rile_shk(shkp);
	strncpy(ESHK(shkp)->customer, plname, PL_NSIZ);
	ESHK(shkp)->following = 1;
}

/* used when the shkp is teleported or falls (ox == 0) out of his shop,
 * or when the player is not on a costly_spot and he
 * damages something inside the shop.  these conditions
 * must be checked by the calling function.
 */
void make_angry_shk(struct monst *shkp, xchar ox, xchar oy) {
	xchar sx, sy;
	struct eshk *eshkp = ESHK(shkp);

	/* all pending shop transactions are now "past due" */
	if (eshkp->billct || eshkp->debit || eshkp->loan || eshkp->credit) {
		eshkp->robbed += (addupbill(shkp) + eshkp->debit + eshkp->loan);
		eshkp->robbed -= eshkp->credit;
		if (eshkp->robbed < 0L) eshkp->robbed = 0L;
		/* billct, debit, loan, and credit will be cleared by setpaid */
		setpaid(shkp);
	}

	/* If you just used a wand of teleportation to send the shk away, you
	   might not be able to see her any more.  Monnam would yield "it",
	   which makes this message look pretty silly, so temporarily restore
	   her original location during the call to Monnam. */
	sx = shkp->mx, sy = shkp->my;
	if (isok(ox, oy) && cansee(ox, oy) && !cansee(sx, sy))
		shkp->mx = ox, shkp->my = oy;
	pline("%s %s!", Monnam(shkp),
	      !ANGRY(shkp) ? "gets angry" : "is furious");
	shkp->mx = sx, shkp->my = sy;
	hot_pursuit(shkp);
}

static const char no_money[] = "Moreover, you%s have no money.";
static const char not_enough_money[] = "Besides, you don't have enough to interest %s.";

// delivers the cheapest item on the list
static long cheapest_item(struct monst *shkp) {
	int ct = ESHK(shkp)->billct;
	struct bill_x *bp = ESHK(shkp)->bill_p;
	long gmin = (bp->price * bp->bquan);

	while (ct--) {
		if (bp->price * bp->bquan < gmin)
			gmin = bp->price * bp->bquan;
		bp++;
	}
	return gmin;
}

int dopay(void) {
	struct eshk *eshkp;
	struct monst *shkp;
	struct monst *nxtm, *resident;
	long ltmp;
	long umoney;

	int pass, tmp, sk = 0, seensk = 0;
	boolean paid = false, stashed_gold = (hidden_gold() > 0L);

	multi = 0;

	/* find how many shk's there are, how many are in */
	/* sight, and are you in a shop room with one.    */
	nxtm = resident = 0;
	for (shkp = next_shkp(fmon, false);
	     shkp;
	     shkp = next_shkp(shkp->nmon, false)) {
		sk++;
		if (ANGRY(shkp) && distu(shkp->mx, shkp->my) <= 2) nxtm = shkp;
		if (canspotmon(shkp)) seensk++;
		if (inhishop(shkp) && (*u.ushops == ESHK(shkp)->shoproom))
			resident = shkp;
	}

	if (nxtm) {	     /* Player should always appease an */
		shkp = nxtm; /* irate shk standing next to them. */
		goto proceed;
	}

	/* KMH -- Permit paying adjacent gypsies */
	for (nxtm = fmon; nxtm; nxtm = nxtm->nmon) {
		if (!nxtm->isgyp || !nxtm->mpeaceful ||
		    distu(nxtm->mx, nxtm->my) > 2 || !canspotmon(nxtm))
			continue;
		shkp = nxtm;
		sk++;
		seensk++;
	}

	if ((!sk && (!Blind || Blind_telepat)) || (!Blind && !seensk)) {
		pline("There appears to be no shopkeeper here to receive your payment.");
		return 0;
	}

	if (!seensk) {
		pline("You can't see...");
		return 0;
	}

	/* the usual case.  allow paying at a distance when */
	/* inside a tended shop.  should we change that?    */
	if (sk == 1 && resident) {
		shkp = resident;
		goto proceed;
	}

	if (seensk == 1) {
		/* KMH -- Permit paying gypsies */
		if (shkp && shkp->isgyp) {
			gypsy_chat(shkp);
			return 1;
		}

		for (shkp = next_shkp(fmon, false);
		     shkp;
		     shkp = next_shkp(shkp->nmon, false))
			if (canspotmon(shkp)) break;
		if (shkp != resident && distu(shkp->mx, shkp->my) > 2) {
			pline("%s is not near enough to receive your payment.",
			      Monnam(shkp));
			return 0;
		}
	} else {
		struct monst *mtmp;
		coord cc;
		int cx, cy;

		pline("Pay whom?");
		cc.x = u.ux;
		cc.y = u.uy;
		if (getpos(&cc, true, "the creature you want to pay") < 0)
			return 0; /* player pressed ESC */
		cx = cc.x;
		cy = cc.y;
		if (cx < 0) {
			pline("Try again...");
			return 0;
		}
		if (u.ux == cx && u.uy == cy) {
			pline("You are generous to yourself.");
			return 0;
		}
		mtmp = m_at(cx, cy);
		if (!mtmp) {
			pline("There is no one there to receive your payment.");
			return 0;
		}
		/* KMH -- Permit paying gypsies */
		if (mtmp->isgyp && mtmp->mpeaceful) {
			if (distu(mtmp->mx, mtmp->my) <= 2) {
				gypsy_chat(mtmp);
				return 1;
			}
		} else if (!mtmp->isshk) {
			pline("%s is not interested in your payment.",
			      Monnam(mtmp));
			return 0;
		}
		if (mtmp != resident && distu(mtmp->mx, mtmp->my) > 2) {
			pline("%s is too far to receive your payment.",
			      Monnam(mtmp));
			return 0;
		}
		shkp = mtmp;
	}

	if (!shkp) {
#ifdef DEBUG
		pline("dopay: null shkp.");
#endif
		return 0;
	}
proceed:
	eshkp = ESHK(shkp);
	ltmp = eshkp->robbed;

	/* wake sleeping shk when someone who owes money offers payment */
	if (ltmp || eshkp->billct || eshkp->debit)
		rouse_shk(shkp, true);

	if (!shkp->mcanmove || shkp->msleeping) { /* still asleep/paralyzed */
		pline("%s %s.", Monnam(shkp),
		      rn2(2) ? "seems to be napping" : "doesn't respond");
		return 0;
	}

	if (shkp != resident && NOTANGRY(shkp)) {
		umoney = money_cnt(invent);
		if (!ltmp)
			pline("You do not owe %s anything.", mon_nam(shkp));
		else if (!umoney) {
			pline("You %shave no money.", stashed_gold ? "seem to " : "");
			if (stashed_gold)

				pline("But you have some gold stashed away.");
		} else {
			if (umoney > ltmp) {
				pline("You give %s the %ld gold piece%s %s asked for.",
				      mon_nam(shkp), ltmp, plur(ltmp), mhe(shkp));
				pay(ltmp, shkp);
			} else {
				pline("You give %s all your%s gold.", mon_nam(shkp),
				      stashed_gold ? " openly kept" : "");
				pay(umoney, shkp);
				if (stashed_gold) pline("But you have hidden gold!");
			}
			if ((umoney < ltmp / 2L) || (umoney < ltmp && stashed_gold))
				pline("Unfortunately, %s doesn't look satisfied.", mhe(shkp));
			else
				make_happy_shk(shkp, false);
		}
		return 1;
	}

	/* ltmp is still eshkp->robbed here */
	if (!eshkp->billct && !eshkp->debit) {
		umoney = money_cnt(invent);
		if (!ltmp && NOTANGRY(shkp)) {
			pline("You do not owe %s anything.", mon_nam(shkp));
			if (!umoney)
				pline(no_money, stashed_gold ? " seem to" : "");

			/*		    else */
			shk_other_services();

		} else if (ltmp) {
			pline("%s is after blood, not money!", Monnam(shkp));
			if (umoney < ltmp / 2L || (umoney < ltmp && stashed_gold)) {
				if (!umoney)
					pline(no_money, stashed_gold ? " seem to" : "");
				else
					pline(not_enough_money, mhim(shkp));
				return 1;
			}
			pline("But since %s shop has been robbed recently,",
			      mhis(shkp));
			pline("you %scompensate %s for %s losses.",
			      (umoney < ltmp) ?
				      "partially " :
				      "",
			      mon_nam(shkp), mhis(shkp));
			pay(umoney < ltmp ? umoney : ltmp, shkp);
			make_happy_shk(shkp, false);
		} else {
			/* shopkeeper is angry, but has not been robbed --
			 * door broken, attacked, etc. */
			pline("%s is after your hide, not your money!",
			      Monnam(shkp));
			if (umoney < 1000L) {
				if (!umoney)
					pline(no_money, stashed_gold ? " seem to" : "");
				else
					pline(not_enough_money, mhim(shkp));
				return 1;
			}
			pline("You try to appease %s by giving %s 1000 gold pieces.",
			      x_monnam(shkp, ARTICLE_THE, "angry", 0, false),
			      mhim(shkp));
			pay(1000L, shkp);
			if (strncmp(eshkp->customer, plname, PL_NSIZ) || rn2(3))
				make_happy_shk(shkp, false);
			else
				pline("But %s is as angry as ever.", mon_nam(shkp));
		}
		return 1;
	}
	if (shkp != resident) {
		impossible("dopay: not to shopkeeper?");
		if (resident) setpaid(resident);
		return 0;
	}
	/* pay debt, if any, first */
	if (eshkp->debit) {
		long dtmp = eshkp->debit;
		long loan = eshkp->loan;
		char sbuf[BUFSZ];
		umoney = money_cnt(invent);
		sprintf(sbuf, "You owe %s %ld %s ",
			shkname(shkp), dtmp, currency(dtmp));
		if (loan) {
			if (loan == dtmp)
				strcat(sbuf, "you picked up in the store.");
			else
				strcat(sbuf,
				       "for gold picked up and the use of merchandise.");
		} else
			strcat(sbuf, "for the use of merchandise.");
		plines(sbuf);
		if (umoney + eshkp->credit < dtmp) {
			pline("But you don't%s have enough gold%s.",

			      stashed_gold ? " seem to" : "",
			      eshkp->credit ? " or credit" : "");
			return 1;
		} else {
			if (eshkp->credit >= dtmp) {
				eshkp->credit -= dtmp;
				eshkp->debit = 0L;
				eshkp->loan = 0L;
				pline("Your debt is covered by your credit.");
			} else if (!eshkp->credit) {
				money2mon(shkp, dtmp);
				eshkp->debit = 0L;
				eshkp->loan = 0L;
				pline("You pay that debt.");
				context.botl = 1;
			} else {
				dtmp -= eshkp->credit;
				eshkp->credit = 0L;
				money2mon(shkp, dtmp);
				eshkp->debit = 0L;
				eshkp->loan = 0L;
				pline("That debt is partially offset by your credit.");
				pline("You pay the remainder.");
				context.botl = 1;
			}
			paid = true;
		}
	}
	/* now check items on bill */
	if (eshkp->billct) {
		boolean itemize;
		umoney = money_cnt(invent);
		if (!umoney && !eshkp->credit) {
			pline("You %shave no money or credit%s.",
			      stashed_gold ? "seem to " : "",
			      paid ? " left" : "");
			return 0;
		}
		if ((umoney + eshkp->credit) < cheapest_item(shkp)) {
			pline("You don't have enough money to buy%s the item%s you picked.",
			      eshkp->billct > 1 ? " any of" : "", plur(eshkp->billct));
			if (stashed_gold)
				pline("Maybe you have some gold stashed away?");
			return 0;
		}

		/* this isn't quite right; it itemizes without asking if the
		 * single item on the bill is partly used up and partly unpaid */
		itemize = (eshkp->billct > 1 ? yn("Itemized billing?") == 'y' : 1);

		for (pass = 0; pass <= 1; pass++) {
			tmp = 0;
			while (tmp < eshkp->billct) {
				struct obj *otmp;
				struct bill_x *bp = &(eshkp->bill_p[tmp]);

				/* find the object on one of the lists */
				if ((otmp = bp_to_obj(bp)) != 0) {
					/* if completely used up, object quantity is stale;
					   restoring it to its original value here avoids
					   making the partly-used-up code more complicated */
					if (bp->useup) otmp->quan = bp->bquan;
				} else {
					impossible("Shopkeeper administration out of order.");
					setpaid(shkp); /* be nice to the player */
					return 1;
				}
				if (pass == bp->useup && otmp->quan == bp->bquan) {
					/* pay for used-up items on first pass and others
					 * on second, so player will be stuck in the store
					 * less often; things which are partly used up
					 * are processed on both passes */
					tmp++;
				} else {
					switch (dopayobj(shkp, bp, &otmp, pass, itemize)) {
						case PAY_CANT:
							return 1; /*break*/
						case PAY_BROKE:
							paid = true;
							goto thanks; /*break*/
						case PAY_SKIP:
							tmp++;
							continue; /*break*/
						case PAY_SOME:
							paid = true;
							if (itemize) bot();
							continue; /*break*/
						case PAY_BUY:
							paid = true;
							break;
					}
					if (itemize) bot();
					*bp = eshkp->bill_p[--eshkp->billct];
				}
			}
		}
	thanks:
		if (!itemize)
			update_inventory(); /* Done in dopayobj() if itemize. */
	}
	if (!ANGRY(shkp) && paid)
		verbalize("Thank you for shopping in %s %s!",
			  s_suffix(shkname(shkp)),
			  shtypes[eshkp->shoptype - SHOPBASE].name);
	return 1;
}

/*
** FUNCTION shk_other_services
**
** Called when you don't owe any money.  Called after all checks have been
** made (in shop, not angry shopkeeper, etc.)
*/
static void shk_other_services(void) {
	char *slang;	    /* What shk calls you		*/
	struct monst *shkp; /* The shopkeeper		*/
	/*WAC - Windowstuff*/
	winid tmpwin;
	anything any;
	menu_item *selected;
	int n;

	/* Do you want to use other services */
	if (yn("Do you wish to try our other services?") != 'y') return;

	/* Init your name */
	if (!is_human(youmonst.data))
		slang = "ugly";
	else
		slang = (flags.female) ? "lady" : "buddy";

	/* Init the shopkeeper */
	shkp = shop_keeper(/* roomno= */ *u.ushops);
	if (!ESHK(shkp)->services) return;

	/*
	** Figure out what services he offers
	**
	** i = identify
	** a = appraise weapon's worth
	** u = uncurse
	** w = weapon-works (including poison)
	** p = poison weapon
	** r = armor-works
	** c = charge wands
	*/
	/*WAC - did this using the windowing system...*/
	any.a_void = 0; /* zero out all bits */
	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);

	/* All shops can identify (some better than others) */
	any.a_int = 1;
	if (ESHK(shkp)->services & (SHK_ID_BASIC | SHK_ID_PREMIUM))
		add_menu(tmpwin, NO_GLYPH, &any, 'i', 0, ATR_NONE,
			 "Identify", MENU_UNSELECTED);

	/* All shops can uncurse */
	any.a_int = 2;
	if (ESHK(shkp)->services & (SHK_UNCURSE))
		add_menu(tmpwin, NO_GLYPH, &any, 'u', 0, ATR_NONE,
			 "Uncurse", MENU_UNSELECTED);

	/* Weapon appraisals.  Weapon & general stores can do this. */
	if ((ESHK(shkp)->services & (SHK_UNCURSE)) &&
	    (shk_class_match(WEAPON_CLASS, shkp))) {
		any.a_int = 3;
		add_menu(tmpwin, NO_GLYPH, &any, 'a', 0, ATR_NONE,
			 "Appraise", MENU_UNSELECTED);
	}

	/* Weapon-works!  Only a weapon store. */
	if ((ESHK(shkp)->services & (SHK_SPECIAL_A | SHK_SPECIAL_B | SHK_SPECIAL_C)) && (shk_class_match(WEAPON_CLASS, shkp) == SHK_MATCH)) {
		any.a_int = 4;
		if (ESHK(shkp)->services & (SHK_SPECIAL_A | SHK_SPECIAL_B))
			add_menu(tmpwin, NO_GLYPH, &any, 'w', 0, ATR_NONE,
				 "Weapon-works", MENU_UNSELECTED);
		else
			add_menu(tmpwin, NO_GLYPH, &any, 'p', 0, ATR_NONE,
				 "Poison", MENU_UNSELECTED);
	}

	/* Armor-works */
	if ((ESHK(shkp)->services & (SHK_SPECIAL_A | SHK_SPECIAL_B)) && (shk_class_match(ARMOR_CLASS, shkp) == SHK_MATCH)) {
		any.a_int = 5;
		add_menu(tmpwin, NO_GLYPH, &any, 'r', 0, ATR_NONE,
			 "Armor-works", MENU_UNSELECTED);
	}

	/* Charging: / ( = */
	if ((ESHK(shkp)->services & (SHK_SPECIAL_A | SHK_SPECIAL_B)) &&
	    ((shk_class_match(WAND_CLASS, shkp) == SHK_MATCH) ||
	     (shk_class_match(TOOL_CLASS, shkp) == SHK_MATCH) ||
	     (shk_class_match(SPBOOK_CLASS, shkp) == SHK_MATCH) ||
	     (shk_class_match(RING_CLASS, shkp) == SHK_MATCH))) {
		any.a_int = 6;
		add_menu(tmpwin, NO_GLYPH, &any, 'c', 0, ATR_NONE,
			 "Charge", MENU_UNSELECTED);
	}

	end_menu(tmpwin, "Services Available:");
	n = select_menu(tmpwin, PICK_ONE, &selected);
	destroy_nhwindow(tmpwin);

	if (n > 0)
		switch (selected[0].item.a_int) {
			case 1:
				shk_identify(slang, shkp);
				break;

			case 2:
				shk_uncurse(slang, shkp);
				break;

			case 3:
				shk_appraisal(slang, shkp);
				break;

			case 4:
				shk_weapon_works(slang, shkp);
				break;

			case 5:
				shk_armor_works(slang, shkp);
				break;

			case 6:
				shk_charge(slang, shkp);
				break;
			default:
				pline("Unknown Service");
				break;
		}
}

/* return 2 if used-up portion paid */
/*	  1 if paid successfully    */
/*	  0 if not enough money     */
/*	 -1 if skip this object     */
/*	 -2 if no money/credit left */

/* which: 0 => used-up item, 1 => other (unpaid or lost) */
static int dopayobj(struct monst *shkp, struct bill_x *bp, struct obj **obj_p, int which, boolean itemize) {
	struct obj *obj = *obj_p;
	long ltmp, quan, save_quan;
	long umoney = money_cnt(invent);
	int buy;
	boolean stashed_gold = (hidden_gold() > 0L),
		consumed = (which == 0);

	if (!obj->unpaid && !bp->useup) {
		impossible("Paid object on bill??");
		return PAY_BUY;
	}
	if (itemize && umoney + ESHK(shkp)->credit == 0L) {
		pline("You %shave no money or credit left.",
		      stashed_gold ? "seem to " : "");
		return PAY_BROKE;
	}
	/* we may need to temporarily adjust the object, if part of the
	   original quantity has been used up but part remains unpaid  */
	save_quan = obj->quan;
	if (consumed) {
		/* either completely used up (simple), or split needed */
		quan = bp->bquan;
		if (quan > obj->quan) /* difference is amount used up */
			quan -= obj->quan;
	} else {
		/* dealing with ordinary unpaid item */
		quan = obj->quan;
	}
	obj->quan = quan; /* to be used by doname() */
	obj->unpaid = 0;  /* ditto */
	ltmp = bp->price * quan;
	buy = PAY_BUY; /* flag; if changed then return early */

	if (itemize) {
		char qbuf[BUFSZ];
		sprintf(qbuf, "%s for %ld %s.  Pay?", quan == 1L ? Doname2(obj) : doname(obj), ltmp, currency(ltmp));
		if (yn(qbuf) == 'n') {
			buy = PAY_SKIP;			    /* don't want to buy */
		} else if (quan < bp->bquan && !consumed) { /* partly used goods */
			obj->quan = bp->bquan - save_quan;  /* used up amount */
			verbalize("%s for the other %s before buying %s.",
				  ANGRY(shkp) ? "Pay" : "Please pay", xname(obj),
				  save_quan > 1L ? "these" : "this one");
			buy = PAY_SKIP; /* shk won't sell */
		}
	}
	if (buy == PAY_BUY && umoney + ESHK(shkp)->credit < ltmp) {
		pline("You don't%s have money%s enough to pay for %s.",
		      stashed_gold ? " seem to" : "",
		      (ESHK(shkp)->credit > 0L) ? " or credit" : "",
		      doname(obj));
		buy = itemize ? PAY_SKIP : PAY_CANT;
	}

	if (buy != PAY_BUY) {
		/* restore unpaid object to original state */
		obj->quan = save_quan;
		obj->unpaid = 1;
		return buy;
	}

	pay(ltmp, shkp);
	shk_names_obj(shkp, obj, consumed ? "paid for %s at a cost of %ld gold piece%s.%s" : "bought %s for %ld gold piece%s.%s", ltmp, "");
	obj->quan = save_quan; /* restore original count */
	/* quan => amount just bought, save_quan => remaining unpaid count */
	if (consumed) {
		if (quan != bp->bquan) {
			/* eliminate used-up portion; remainder is still unpaid */
			bp->bquan = obj->quan;
			obj->unpaid = 1;
			bp->useup = 0;
			buy = PAY_SOME;
		} else { /* completely used-up, so get rid of it */
			obj_extract_self(obj);
			/* assert( obj == *obj_p ); */
			dealloc_obj(obj);
			*obj_p = 0; /* destroy pointer to freed object */
		}
	} else if (itemize)
		update_inventory(); /* Done just once in dopay() if !itemize. */
	return buy;
}

static coord repo_location; /* repossession context */

/* routine called after dying (or quitting) */
/* croaked: -1: escaped dungeon; 0: quit; 1: died */
boolean paybill(int croaked) {
	struct monst *mtmp, *mtmp2, *resident = NULL;
	boolean taken = false;
	int numsk = 0;

	/* if we escaped from the dungeon, shopkeepers can't reach us;
	   shops don't occur on level 1, but this could happen if hero
	   level teleports out of the dungeon and manages not to die */
	if (croaked < 0) return false;

	/* this is where inventory will end up if any shk takes it */
	repo_location.x = repo_location.y = 0;

	/* give shopkeeper first crack */
	if ((mtmp = shop_keeper(*u.ushops)) && inhishop(mtmp)) {
		numsk++;
		resident = mtmp;
		taken = inherits(resident, numsk, croaked);
	}
	for (mtmp = next_shkp(fmon, false);
	     mtmp;
	     mtmp = next_shkp(mtmp2, false)) {
		mtmp2 = mtmp->nmon;
		if (mtmp != resident) {
			/* for bones: we don't want a shopless shk around */
			if (!on_level(&(ESHK(mtmp)->shoplevel), &u.uz))
				mongone(mtmp);
			else {
				numsk++;
				taken |= inherits(mtmp, numsk, croaked);
			}
		}
	}
	if (numsk == 0) return false;
	return taken;
}

static boolean inherits(struct monst *shkp, int numsk, int croaked) {
	long loss = 0L;
	long umoney;
	struct eshk *eshkp = ESHK(shkp);
	boolean take = false, taken = false;
	int roomno = *u.ushops;
	char takes[BUFSZ];

	/* the simplifying principle is that first-come */
	/* already took everything you had.		*/
	if (numsk > 1) {
		if (cansee(shkp->mx, shkp->my) && croaked)
			pline("%s %slooks at your corpse%s and %s.",
			      Monnam(shkp),
			      (!shkp->mcanmove || shkp->msleeping) ? "wakes up, " : "",
			      !rn2(2) ? (shkp->female ? ", shakes her head," :
							", shakes his head,") :
					"",
			      !inhishop(shkp) ? "disappears" : "sighs");
		rouse_shk(shkp, false); /* wake shk for bones */
		taken = (roomno == eshkp->shoproom);
		goto skip;
	}

	/* get one case out of the way: you die in the shop, the */
	/* shopkeeper is peaceful, nothing stolen, nothing owed. */
	if (roomno == eshkp->shoproom && inhishop(shkp) &&
	    !eshkp->billct && !eshkp->robbed && !eshkp->debit &&
	    NOTANGRY(shkp) && !eshkp->following) {
		if (invent)
			pline("%s gratefully inherits all your possessions.",
			      shkname(shkp));
		set_repo_loc(eshkp);
		goto clear;
	}

	if (eshkp->billct || eshkp->debit || eshkp->robbed) {
		if (roomno == eshkp->shoproom && inhishop(shkp))
			loss = addupbill(shkp) + eshkp->debit;
		if (loss < eshkp->robbed) loss = eshkp->robbed;
		take = true;
	}

	if (eshkp->following || ANGRY(shkp) || take) {
		if (!invent) goto skip;
		umoney = money_cnt(invent);
		takes[0] = '\0';
		if (!shkp->mcanmove || shkp->msleeping)
			strcat(takes, "wakes up and ");
		if (distu(shkp->mx, shkp->my) > 2)
			strcat(takes, "comes and ");
		strcat(takes, "takes");

		if (loss > umoney || !loss || roomno == eshkp->shoproom) {
			eshkp->robbed -= umoney;
			if (eshkp->robbed < 0L) eshkp->robbed = 0L;
			if (umoney > 0) money2mon(shkp, umoney);
			context.botl = 1;
			pline("%s %s all your possessions.",
			      shkname(shkp), takes);
			taken = true;
			/* where to put player's invent (after disclosure) */
			set_repo_loc(eshkp);
		} else {
			money2mon(shkp, loss);
			context.botl = 1;
			pline("%s %s the %ld %s %sowed %s.",
			      Monnam(shkp), takes,
			      loss, currency(loss),
			      strncmp(eshkp->customer, plname, PL_NSIZ) ?
				      "" :
				      "you ",
			      shkp->female ? "her" : "him");
			/* shopkeeper has now been paid in full */
			pacify_shk(shkp);
			eshkp->following = 0;
			eshkp->robbed = 0L;
		}
	skip:
		/* in case we create bones */
		rouse_shk(shkp, false); /* wake up */
		if (!inhishop(shkp))
			home_shk(shkp, false);
	}
clear:
	setpaid(shkp);
	return taken;
}

static void set_repo_loc(struct eshk *eshkp) {
	xchar ox, oy;

	/* if you're not in this shk's shop room, or if you're in its doorway
	    or entry spot, then your gear gets dumped all the way inside */
	if (*u.ushops != eshkp->shoproom ||
	    IS_DOOR(levl[u.ux][u.uy].typ) ||
	    (u.ux == eshkp->shk.x && u.uy == eshkp->shk.y)) {
		/* shk.x,shk.y is the position immediately in
		 * front of the door -- move in one more space
		 */
		ox = eshkp->shk.x;
		oy = eshkp->shk.y;
		ox += sgn(ox - eshkp->shd.x);
		oy += sgn(oy - eshkp->shd.y);
	} else { /* already inside this shk's shop */
		ox = u.ux;
		oy = u.uy;
	}
	/* finish_paybill will deposit invent here */
	repo_location.x = ox;
	repo_location.y = oy;
}

/* called at game exit, after inventory disclosure but before making bones */
void finish_paybill() {
	struct obj *otmp;
	int ox = repo_location.x,
	    oy = repo_location.y;

#if 0 /* don't bother */
	if (ox == 0 && oy == 0) impossible("finish_paybill: no location");
#endif
	/* normally done by savebones(), but that's too late in this case */
	unleash_all();
	/* transfer all of the character's inventory to the shop floor */
	while ((otmp = invent) != 0) {
		otmp->owornmask = 0L;	 /* perhaps we should call setnotworn? */
		otmp->lamplit = 0;	 /* avoid "goes out" msg from freeinv */
		if (rn2(5)) curse(otmp); /* normal bones treatment for invent */
		obj_extract_self(otmp);
		place_object(otmp, ox, oy);
	}
}

/* find obj on one of the lists */
static struct obj *bp_to_obj(struct bill_x *bp) {
	struct obj *obj;
	uint id = bp->bo_id;

	if (bp->useup)
		obj = o_on(id, billobjs);
	else
		obj = find_oid(id);
	return obj;
}

/*
 * Look for o_id on all lists but billobj.  Return obj or NULL if not found.
 * Its OK for restore_timers() to call this function, there should not
 * be any timeouts on the billobjs chain.
 */
struct obj *find_oid(unsigned id) {
	struct obj *obj;
	struct monst *mon, *mmtmp[3];
	int i;

	/* first check various obj lists directly */
	if ((obj = o_on(id, invent)) != 0) return obj;
	if ((obj = o_on(id, fobj)) != 0) return obj;
	if ((obj = o_on(id, level.buriedobjlist)) != 0) return obj;
	if ((obj = o_on(id, migrating_objs)) != 0) return obj;

	/* not found yet; check inventory for members of various monst lists */
	mmtmp[0] = fmon;
	mmtmp[1] = migrating_mons;
	mmtmp[2] = mydogs; /* for use during level changes */
	for (i = 0; i < 3; i++)
		for (mon = mmtmp[i]; mon; mon = mon->nmon)
			if ((obj = o_on(id, mon->minvent)) != 0) return obj;

	/* not found at all */
	return NULL;
}

/* calculate the value that the shk will charge for [one of] an object */
static long get_cost(struct obj *obj, struct monst *shkp) {
	long tmp = getprice(obj, false);

	if (!tmp) tmp = 5L;
	/* shopkeeper may notice if the player isn't very knowledgeable -
	   especially when gem prices are concerned */
	if (!obj->dknown || !objects[obj->otyp].oc_name_known) {
		if (obj->oclass == GEM_CLASS &&
		    objects[obj->otyp].oc_material == GLASS) {
			int i;
			/* get a value that's 'random' from game to game, but the
			   same within the same game */
			boolean pseudorand =
				(((int)u.ubirthday % obj->otyp) >= obj->otyp / 2);

			/* all gems are priced high - real or not */
			switch (obj->otyp - LAST_GEM) {
				case 1: /* white */
					i = pseudorand ? DIAMOND : OPAL;
					break;
				case 2: /* blue */
					i = pseudorand ? SAPPHIRE : AQUAMARINE;
					break;
				case 3: /* red */
					i = pseudorand ? RUBY : JASPER;
					break;
				case 4: /* yellowish brown */
					i = pseudorand ? AMBER : TOPAZ;
					break;
				case 5: /* orange */
					i = pseudorand ? JACINTH : AGATE;
					break;
				case 6: /* yellow */
					i = pseudorand ? CITRINE : CHRYSOBERYL;
					break;
				case 7: /* black */
					i = pseudorand ? BLACK_OPAL : JET;
					break;
				case 8: /* green */
					i = pseudorand ? EMERALD : JADE;
					break;
				case 9: /* violet */
					i = pseudorand ? AMETHYST : FLUORITE;
					break;
				default:
					impossible("bad glass gem %d?", obj->otyp);
					i = STRANGE_OBJECT;
					break;
			}
			tmp = (long)objects[i].oc_cost;
		} else if (!(obj->o_id % 4)) /* arbitrarily impose surcharge */
			tmp += tmp / 3L;
	}
	if ((Role_if(PM_TOURIST) && u.ulevel < (MAXULEV / 2)) || ((uarmu && !uarmu->oinvis) && (!uarm || uarm->oinvis) && (!uarmc || uarmc->oinvis)) /* touristy shirt visible */
	    || (uarmh && !uarmh->oinvis && uarmh->otyp == DUNCE_CAP))
		tmp += tmp / 3L;

	if (ACURR(A_CHA) > 18)
		tmp /= 2L;
	else if (ACURR(A_CHA) > 17)
		tmp -= tmp / 3L;
	else if (ACURR(A_CHA) > 15)
		tmp -= tmp / 4L;
	else if (ACURR(A_CHA) < 6)
		tmp *= 2L;
	else if (ACURR(A_CHA) < 8)
		tmp += tmp / 2L;
	else if (ACURR(A_CHA) < 11)
		tmp += tmp / 3L;
	if (tmp <= 0L)
		tmp = 1L;
	else if (obj->oartifact)
		tmp *= 4L;

	/* character classes who are discriminated against... */
	/* barbarians are gullible... */
	if (Role_if(PM_BARBARIAN)) tmp *= 3L;
	/* rogues are untrustworthy... */
	if (Role_if(PM_ROGUE)) tmp *= 2L;
	/* samurais are from out of town... */
	if (Role_if(PM_SAMURAI)) tmp *= 2L;

	/* anger surcharge should match rile_shk's */
	if (shkp && ESHK(shkp)->surcharge) tmp += (tmp + 2L) / 3L;

	/* KMH, balance patch -- healthstone replaces rotting/health */
	if (Is_blackmarket(&u.uz)) {
		if (obj->oclass == RING_CLASS || obj->oclass == AMULET_CLASS ||
		    obj->oclass == POTION_CLASS || obj->oclass == SCROLL_CLASS ||
		    obj->oclass == SPBOOK_CLASS || obj->oclass == WAND_CLASS ||
		    obj->otyp == LUCKSTONE || obj->otyp == LOADSTONE ||
		    obj->otyp == HEALTHSTONE || objects[obj->otyp].oc_magic) {
			tmp *= 50;
		} else {
			tmp *= 25;
		}
	}

	return tmp;
}

/* returns the price of a container's content.  the price
 * of the "top" container is added in the calling functions.
 * a different price quoted for selling as vs. buying.
 */
long contained_cost(struct obj *obj, struct monst *shkp, long price, boolean usell, boolean unpaid_only) {
	struct obj *otmp;

	/* the price of contained objects */
	for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
		if (otmp->oclass == COIN_CLASS) continue;
		/* the "top" container is evaluated by caller */
		if (usell) {
			if (saleable(shkp, otmp) &&
			    !otmp->unpaid && otmp->oclass != BALL_CLASS &&
			    !is_hazy(otmp) && !(otmp->oclass == FOOD_CLASS && otmp->oeaten) &&
			    !(Is_candle(otmp) && otmp->age < 20L * objects[otmp->otyp].oc_cost))
				price += set_cost(otmp, shkp);
		} else if (!otmp->no_charge &&
			   (!unpaid_only || (unpaid_only && otmp->unpaid))) {
			price += get_cost(otmp, shkp) * otmp->quan;
		}

		if (Has_contents(otmp))
			price += contained_cost(otmp, shkp, price, usell, unpaid_only);
	}

	return price;
}

long contained_gold(struct obj *obj) {
	struct obj *otmp;
	long value = 0L;

	/* accumulate contained gold */
	for (otmp = obj->cobj; otmp; otmp = otmp->nobj)
		if (otmp->oclass == COIN_CLASS)
			value += otmp->quan;
		else if (Has_contents(otmp))
			value += contained_gold(otmp);

	return value;
}

static void dropped_container(struct obj *obj, struct monst *shkp, boolean sale) {
	struct obj *otmp;

	/* the "top" container is treated in the calling fn */
	for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
		if (otmp->oclass == COIN_CLASS) continue;

		if (!otmp->unpaid && !(sale && saleable(shkp, otmp)))
			otmp->no_charge = 1;

		if (Has_contents(otmp))
			dropped_container(otmp, shkp, sale);
	}
}

void picked_container(struct obj *obj) {
	struct obj *otmp;

	/* the "top" container is treated in the calling fn */
	for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
		if (otmp->oclass == COIN_CLASS) continue;

		if (otmp->no_charge)
			otmp->no_charge = 0;

		if (Has_contents(otmp))
			picked_container(otmp);
	}
}

/* calculate how much the shk will pay when buying [all of] an object */
static long set_cost(struct obj *obj, struct monst *shkp) {
	long tmp = getprice(obj, true) * obj->quan;

	if ((Role_if(PM_TOURIST) && u.ulevel < (MAXULEV / 2)) || ((uarmu && !uarmu->oinvis) && (!uarm || uarm->oinvis) && (!uarmc || uarmc->oinvis)) /* touristy shirt visible */
	    || (uarmh && !uarmh->oinvis && uarmh->otyp == DUNCE_CAP))
		tmp /= 3L;
	else
		tmp /= 2L;

	/* shopkeeper may notice if the player isn't very knowledgeable -
	   especially when gem prices are concerned */
	if (!obj->dknown || !objects[obj->otyp].oc_name_known) {
		if (obj->oclass == GEM_CLASS) {
			/* different shop keepers give different prices */
			if (objects[obj->otyp].oc_material == GEMSTONE ||
			    objects[obj->otyp].oc_material == GLASS) {
				tmp = (obj->otyp % (6 - shkp->m_id % 3));
				tmp = (tmp + 3) * obj->quan;
			}
		} else if (tmp > 1L && !rn2(4))
			tmp -= tmp / 4L;
	}
	return tmp;
}

/* called from doinv(invent.c) for inventory of unpaid objects */
long unpaid_cost(struct obj *unpaid_obj) {
	struct bill_x *bp = NULL;
	struct monst *shkp;

	for (shkp = next_shkp(fmon, true); shkp;
	     shkp = next_shkp(shkp->nmon, true))
		if ((bp = onbill(unpaid_obj, shkp, true)) != 0) break;

	/* onbill() gave no message if unexpected problem occurred */
	if (!bp) impossible("unpaid_cost: object wasn't on any bill!");

	return bp ? unpaid_obj->quan * bp->price : 0L;
}

static int extend_bill(struct eshk *eshkp, int to_add) {
	struct bill_x *new;
	int bct = eshkp->billct + to_add;

	if (bct > eshkp->billsz) {
		if (eshkp->bill_p)
			new = (struct bill_x *)realloc((void *)eshkp->bill_p,
						       bct * sizeof(struct bill_x));
		else
			new = alloc(bct * sizeof(struct bill_x));
		if (!new)
			return 0;
		eshkp->billsz = bct;
		eshkp->bill_p = new;
	}
	return 1;
}

static void add_one_tobill(struct obj *obj, boolean dummy) {
	struct monst *shkp;
	struct bill_x *bp;
	int bct;
	char roomno = *u.ushops;

	if (!roomno) return;
	if (!(shkp = shop_keeper(roomno))) return;
	if (!inhishop(shkp)) return;

	if (onbill(obj, shkp, false) || /* perhaps thrown away earlier */
	    (obj->oclass == FOOD_CLASS && obj->oeaten))
		return;

	if (!extend_bill(ESHK(shkp), 1)) {
		pline("You got that for free!");
		return;
	}

	/* To recognize objects the shopkeeper is not interested in. -dgk
	 */
	if (obj->no_charge) {
		obj->no_charge = 0;
		return;
	}

	bct = ESHK(shkp)->billct;
	bp = &(ESHK(shkp)->bill_p[bct]);
	bp->bo_id = obj->o_id;
	bp->bquan = obj->quan;
	if (dummy) {		      /* a dummy object must be inserted into  */
		bp->useup = 1;	      /* the billobjs chain here.  crucial for */
		add_to_billobjs(obj); /* eating floorfood in shop.  see eat.c  */
	} else
		bp->useup = 0;
	bp->price = get_cost(obj, shkp);
	ESHK(shkp)->billct++;
	obj->unpaid = 1;
}

static void add_to_billobjs(struct obj *obj) {
	if (obj->where != OBJ_FREE)
		panic("add_to_billobjs: obj not free");
	if (obj->timed)
		obj_stop_timers(obj);

	obj->nobj = billobjs;
	billobjs = obj;
	obj->where = OBJ_ONBILL;
}

/* recursive billing of objects within containers. */
static void bill_box_content(struct obj *obj, boolean ininv, boolean dummy, struct monst *shkp) {
	struct obj *otmp;

	for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
		if (otmp->oclass == COIN_CLASS) continue;

		/* the "top" box is added in addtobill() */
		if (!otmp->no_charge)
			add_one_tobill(otmp, dummy);
		if (Has_contents(otmp))
			bill_box_content(otmp, ininv, dummy, shkp);
	}
}

/* shopkeeper tells you what you bought or sold, sometimes partly IDing it */
/* fmt = "%s %ld %s %s", doname(obj), amt, plur(amt), arg */
static void shk_names_obj(struct monst *shkp, struct obj *obj, const char *fmt, long amt, const char *arg) {
	char *obj_name, fmtbuf[BUFSZ];
	boolean was_unknown = !obj->dknown;

	obj->dknown = true;
	/* Use real name for ordinary weapons/armor, and spell-less
	 * scrolls/books (that is, blank and mail), but only if the
	 * object is within the shk's area of interest/expertise.
	 */
	if (!objects[obj->otyp].oc_magic && saleable(shkp, obj) &&
	    (obj->oclass == WEAPON_CLASS || obj->oclass == ARMOR_CLASS ||
	     obj->oclass == SCROLL_CLASS || obj->oclass == SPBOOK_CLASS ||
	     obj->otyp == MIRROR)) {
		was_unknown |= !objects[obj->otyp].oc_name_known;
		makeknown(obj->otyp);
	}
	obj_name = doname(obj);
	/* Use an alternate message when extra information is being provided */
	if (was_unknown) {
		sprintf(fmtbuf, "%%s; you %s", fmt);
		obj_name[0] = highc(obj_name[0]);
		pline(fmtbuf, obj_name, (obj->quan > 1) ? "them" : "it",
		      amt, plur(amt), arg);
	} else {
		sprintf(fmtbuf, "You %s", fmt);
		pline(fmtbuf, obj_name, amt, plur(amt), arg);
	}
}

void addtobill(struct obj *obj, boolean ininv, boolean dummy, boolean silent) {
	struct monst *shkp;
	char roomno = *u.ushops;
	long ltmp = 0L, cltmp = 0L, gltmp = 0L;
	boolean container = Has_contents(obj);

	if (!*u.ushops) return;

	if (!(shkp = shop_keeper(roomno))) return;

	if (!inhishop(shkp)) return;

	if (/* perhaps we threw it away earlier */
	    onbill(obj, shkp, false) ||
	    (obj->oclass == FOOD_CLASS && obj->oeaten)) return;

	if (!extend_bill(ESHK(shkp), 1)) {
		pline("You got that for free!");
		return;
	}

	if (obj->oclass == COIN_CLASS) {
		costly_gold(obj->ox, obj->oy, obj->quan);
		return;
	}

	if (!obj->no_charge)
		ltmp = get_cost(obj, shkp);

	if (obj->no_charge && !container) {
		obj->no_charge = 0;
		return;
	}

	if (container) {
		if (obj->cobj == NULL) {
			if (obj->no_charge) {
				obj->no_charge = 0;
				return;
			} else {
				add_one_tobill(obj, dummy);
				goto speak;
			}
		} else {
			cltmp += contained_cost(obj, shkp, cltmp, false, false);
			gltmp += contained_gold(obj);
		}

		if (ltmp) add_one_tobill(obj, dummy);
		if (cltmp) bill_box_content(obj, ininv, dummy, shkp);
		picked_container(obj); /* reset contained obj->no_charge */

		ltmp += cltmp;

		if (gltmp) {
			costly_gold(obj->ox, obj->oy, gltmp);
			if (!ltmp) return;
		}

		if (obj->no_charge) {
			obj->no_charge = 0;
			if (!ltmp) return;
		}

	} else /* i.e., !container */
		add_one_tobill(obj, dummy);
speak:
	if (shkp->mcanmove && !shkp->msleeping && !silent) {
		char buf[BUFSZ];

		if (!ltmp) {
			pline("%s has no interest in %s.", Monnam(shkp),
			      the(xname(obj)));
			return;
		}
		strcpy(buf, "\"For you, ");
		if (ANGRY(shkp))
			strcat(buf, "scum ");
		else {
			static const char *honored[5] = {
				"good", "honored", "most gracious", "esteemed",
				"most renowned and sacred"};
			strcat(buf, honored[rn2(4) + u.uevent.udemigod]);
			if (!is_human(youmonst.data))
				strcat(buf, " creature");
			else
				strcat(buf, (flags.female) ? " lady" : " sir");
		}
		if (ininv) {
			long quan = obj->quan;
			obj->quan = 1L; /* fool xname() into giving singular */
			pline("%s; only %ld %s %s.\"", buf, ltmp,
			      (quan > 1L) ? "per" : "for this", xname(obj));
			obj->quan = quan;
		} else
			pline("%s will cost you %ld %s%s.",
			      The(xname(obj)), ltmp, currency(ltmp),
			      (obj->quan > 1L) ? " each" : "");
	} else if (!silent) {
		if (ltmp)
			pline("The list price of %s is %ld %s%s.",
			      the(xname(obj)), ltmp, currency(ltmp),
			      (obj->quan > 1L) ? " each" : "");
		else
			pline("%s does not notice.", Monnam(shkp));
	}
}

void splitbill(struct obj *obj, struct obj *otmp) {
	/* otmp has been split off from obj */
	struct bill_x *bp;
	long tmp;
	struct monst *shkp = shop_keeper(*u.ushops);

	if (!shkp || !inhishop(shkp)) {
		impossible("splitbill: no resident shopkeeper??");
		return;
	}
	bp = onbill(obj, shkp, false);
	if (!bp) {
		impossible("splitbill: not on bill?");
		return;
	}
	if (bp->bquan < otmp->quan) {
		impossible("Negative quantity on bill??");
	}
	if (bp->bquan == otmp->quan) {
		impossible("Zero quantity on bill??");
	}
	bp->bquan -= otmp->quan;

	if (!extend_bill(ESHK(shkp), 1))
		otmp->unpaid = 0;
	else {
		tmp = bp->price;
		bp = &(ESHK(shkp)->bill_p[ESHK(shkp)->billct]);
		bp->bo_id = otmp->o_id;
		bp->bquan = otmp->quan;
		bp->useup = 0;
		bp->price = tmp;
		ESHK(shkp)->billct++;
	}
}

static void sub_one_frombill(struct obj *obj, struct monst *shkp) {
	struct bill_x *bp;

	if ((bp = onbill(obj, shkp, false)) != 0) {
		struct obj *otmp;

		obj->unpaid = 0;
		if (bp->bquan > obj->quan) {
			otmp = newobj(0);
			*otmp = *obj;
			bp->bo_id = otmp->o_id = context.ident++;
			otmp->where = OBJ_FREE;
			otmp->quan = (bp->bquan -= obj->quan);
			otmp->owt = 0; /* superfluous */
			otmp->onamelth = 0;
			otmp->oxlth = 0;
			otmp->oattached = OATTACHED_NOTHING;
			bp->useup = 1;
			add_to_billobjs(otmp);
			return;
		}
		ESHK(shkp)->billct--;
		*bp = ESHK(shkp)->bill_p[ESHK(shkp)->billct];
		return;
	} else if (obj->unpaid) {
		impossible("sub_one_frombill: unpaid object not on bill");
		obj->unpaid = 0;
	}
}

/* recursive check of unpaid objects within nested containers. */
void subfrombill(struct obj *obj, struct monst *shkp) {
	struct obj *otmp;

	sub_one_frombill(obj, shkp);

	if (Has_contents(obj))
		for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
			if (otmp->oclass == COIN_CLASS) continue;

			if (Has_contents(otmp))
				subfrombill(otmp, shkp);
			else
				sub_one_frombill(otmp, shkp);
		}
}

static long stolen_container(struct obj *obj, struct monst *shkp, long price, boolean ininv, boolean destruction) {
	struct obj *otmp;

	if (!(destruction && evades_destruction(obj))) {
		if (ininv && obj->unpaid)
			price += get_cost(obj, shkp);
		else {
			if (!obj->no_charge)
				price += get_cost(obj, shkp);
			obj->no_charge = 0;
		}
	}

	/* the price of contained objects, if any */
	for (otmp = obj->cobj; otmp; otmp = otmp->nobj) {
		if (otmp->oclass == COIN_CLASS) continue;

		if (!Has_contents(otmp)) {
			if (!(destruction && evades_destruction(otmp))) {
				if (ininv) {
					if (otmp->unpaid)
						price += otmp->quan * get_cost(otmp, shkp);
				} else {
					if (!otmp->no_charge) {
						if (otmp->oclass != FOOD_CLASS || !otmp->oeaten)
							price += otmp->quan * get_cost(otmp, shkp);
					}
					otmp->no_charge = 0;
				}
			}
		} else
			price += stolen_container(otmp, shkp, price, ininv,
						  destruction);
	}

	return price;
}

long stolen_value(struct obj *obj, xchar x, xchar y, boolean peaceful, boolean silent, boolean destruction) {
	long value = 0L, gvalue = 0L;
	struct monst *shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));

	if (!shkp || !inhishop(shkp))
		return 0L;

	if (obj->oclass == COIN_CLASS) {
		gvalue += obj->quan;
	} else if (Has_contents(obj)) {
		boolean ininv = !!count_unpaid(obj->cobj);

		value += stolen_container(obj, shkp, value, ininv, destruction);
		if (!ininv) gvalue += contained_gold(obj);
	} else if (!obj->no_charge && !(destruction && evades_destruction(obj)) &&
		   // treat items inside containers as 'saleable'
		   (saleable(shkp, obj) || obj->where == OBJ_CONTAINED) &&
		   (obj->oclass != FOOD_CLASS || !obj->oeaten))  {

		value += obj->quan * get_cost(obj, shkp);
	}

	if (gvalue + value == 0L) return 0L;

	value += gvalue;

	if (peaceful) {
		boolean credit_use = !!ESHK(shkp)->credit;
		value = check_credit(value, shkp);
		/* 'peaceful' affects general treatment, but doesn't affect
		 * the fact that other code expects that all charges after the
		 * shopkeeper is angry are included in robbed, not debit */
		if (ANGRY(shkp))
			ESHK(shkp)->robbed += value;
		else
			ESHK(shkp)->debit += value;

		if (!silent) {
			const char *still = "";

			if (credit_use) {
				if (ESHK(shkp)->credit) {
					pline("You have %ld %s credit remaining.",
					      ESHK(shkp)->credit, currency(ESHK(shkp)->credit));
					return value;
				} else if (!value) {
					pline("You have no credit remaining.");
					return 0;
				}
				still = "still ";
			}
			if (obj->oclass == COIN_CLASS)
				pline("You %sowe %s %ld %s!", still,
				      mon_nam(shkp), value, currency(value));
			else
				pline("You %sowe %s %ld %s for %s!", still,
				      mon_nam(shkp), value, currency(value),
				      obj->quan > 1L ? "them" : "it");
		}
	} else {
		ESHK(shkp)->robbed += value;

		if (!silent) {
			if (cansee(shkp->mx, shkp->my)) {
				Norep("%s booms: \"%s, you are a thief!\"",
				      Monnam(shkp), plname);
			} else
				Norep("You hear a scream, \"Thief!\"");
		}
		hot_pursuit(shkp);
		angry_guards(false);
	}
	return value;
}

/* auto-response flag for/from "sell foo?" 'a' => 'y', 'q' => 'n' */
static char sell_response = 'a';
static int sell_how = SELL_NORMAL;
/* can't just use sell_response='y' for auto_credit because the 'a' response
   shouldn't carry over from ordinary selling to credit selling */
static boolean auto_credit = false;

void sellobj_state(int deliberate) {
	/* If we're deliberately dropping something, there's no automatic
	   response to the shopkeeper's "want to sell" query; however, if we
	   accidentally drop anything, the shk will buy it/them without asking.
	   This retains the old pre-query risk that slippery fingers while in
	   shops entailed:  you drop it, you've lost it.
	 */
	sell_response = (deliberate != SELL_NORMAL) ? '\0' : 'a';
	sell_how = deliberate;
	auto_credit = false;
}

void sellobj(struct obj *obj, xchar x, xchar y) {
	struct monst *shkp;
	struct eshk *eshkp;
	long ltmp = 0L, cltmp = 0L, gltmp = 0L, offer;
	boolean saleitem, cgold = false, container = Has_contents(obj);
	boolean isgold = (obj->oclass == COIN_CLASS);
	boolean only_partially_your_contents = false;

	if (!(shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) ||
	    !inhishop(shkp)) return;
	if (!costly_spot(x, y)) return;
	if (!*u.ushops) return;

	if (obj->unpaid && !container && !isgold) {
		sub_one_frombill(obj, shkp);
		return;
	}
	if (container) {
		/* find the price of content before subfrombill */
		cltmp += contained_cost(obj, shkp, cltmp, true, false);
		/* find the value of contained gold */
		gltmp += contained_gold(obj);
		cgold = (gltmp > 0L);
	}

	saleitem = saleable(shkp, obj);
	if (!isgold && !obj->unpaid && saleitem)
		ltmp = set_cost(obj, shkp);

	offer = ltmp + cltmp;

	/* get one case out of the way: nothing to sell, and no gold */
	if (!isgold &&
	    ((offer + gltmp) == 0L || sell_how == SELL_DONTSELL)) {
		boolean unpaid = (obj->unpaid ||
				  (container && count_unpaid(obj->cobj)));

		if (container) {
			dropped_container(obj, shkp, false);
			if (!obj->unpaid && !saleitem)
				obj->no_charge = 1;
			if (obj->unpaid || count_unpaid(obj->cobj))
				subfrombill(obj, shkp);
		} else
			obj->no_charge = 1;

		if (!unpaid && (sell_how != SELL_DONTSELL))
			pline("%s seems uninterested.", Monnam(shkp));
		return;
	}

	/* you dropped something of your own - probably want to sell it */
	rouse_shk(shkp, true); /* wake up sleeping or paralyzed shk */
	eshkp = ESHK(shkp);

	if (ANGRY(shkp)) { /* they become shop-objects, no pay */
		pline("Thank you, scum!");
		subfrombill(obj, shkp);
		return;
	}

	if (eshkp->robbed) { /* shkp is not angry? */
		if (isgold)
			offer = obj->quan;
		else if (cgold)
			offer += cgold;
		if ((eshkp->robbed -= offer < 0L))
			eshkp->robbed = 0L;
		if (offer) verbalize("Thank you for your contribution to restock this recently plundered shop.");
		subfrombill(obj, shkp);
		return;
	}

	if (isgold || cgold) {
		if (!cgold) gltmp = obj->quan;

		if (eshkp->debit >= gltmp) {
			if (eshkp->loan) { /* you carry shop's gold */
				if (eshkp->loan >= gltmp)
					eshkp->loan -= gltmp;
				else
					eshkp->loan = 0L;
			}
			eshkp->debit -= gltmp;
			pline("Your debt is %spaid off.",
			      eshkp->debit ? "partially " : "");
		} else {
			long delta = gltmp - eshkp->debit;

			eshkp->credit += delta;
			if (eshkp->debit) {
				eshkp->debit = 0L;
				eshkp->loan = 0L;
				pline("Your debt is paid off.");
			}
			pline("%ld %s %s added to your credit.",
			      delta, currency(delta), delta > 1L ? "are" : "is");
		}
		if (offer)
			goto move_on;
		else {
			if (!isgold) {
				if (container)
					dropped_container(obj, shkp, false);
				if (!obj->unpaid && !saleitem) obj->no_charge = 1;
				subfrombill(obj, shkp);
			}
			return;
		}
	}
move_on:
	if ((!saleitem && !(container && cltmp > 0L)) || obj->oclass == BALL_CLASS || obj->oclass == CHAIN_CLASS || offer == 0L || is_hazy(obj) || (obj->oclass == FOOD_CLASS && obj->oeaten) || (Is_candle(obj) && obj->age < 20L * (long)objects[obj->otyp].oc_cost)) {
		pline("%s seems uninterested%s.", Monnam(shkp),
		      cgold ? " in the rest" : "");
		if (container)
			dropped_container(obj, shkp, false);
		obj->no_charge = 1;
		return;
	}

	if (!money_cnt(shkp->minvent)) {
		char c, qbuf[BUFSZ];
		long tmpcr = ((offer * 9L) / 10L) + (offer <= 1L);

		if (sell_how == SELL_NORMAL || auto_credit) {
			c = sell_response = 'y';
		} else if (sell_response != 'n') {
			pline("%s cannot pay you at present.", Monnam(shkp));
			sprintf(qbuf,
				"Will you accept %ld %s in credit for %s?",
				tmpcr, currency(tmpcr), doname(obj));
			/* won't accept 'a' response here */
			/* KLY - 3/2000 yes, we will, it's a damn nuisance
			   to have to constantly hit 'y' to sell for credit */
			c = ynaq(qbuf);
			if (c == 'a') {
				c = 'y';
				auto_credit = true;
			}
		} else /* previously specified "quit" */
			c = 'n';

		if (c == 'y') {
			shk_names_obj(shkp, obj, (sell_how != SELL_NORMAL) ? "traded %s for %ld zorkmid%s in %scredit." : "relinquish %s and acquire %ld zorkmid%s in %scredit.",
				      tmpcr,
				      (eshkp->credit > 0L) ? "additional " : "");
			eshkp->credit += tmpcr;
			subfrombill(obj, shkp);
		} else {
			if (c == 'q') sell_response = 'n';
			if (container)
				dropped_container(obj, shkp, false);
			if (!obj->unpaid) obj->no_charge = 1;
			subfrombill(obj, shkp);
		}
	} else {
		char qbuf[BUFSZ];
		long shkmoney = money_cnt(shkp->minvent);
		boolean short_funds = (offer > shkmoney);
		if (short_funds) offer = shkmoney;
		if (!sell_response) {
			only_partially_your_contents =
				(contained_cost(obj, shkp, 0L, false, false) !=
				 contained_cost(obj, shkp, 0L, false, true));
			sprintf(qbuf,
				"%s offers%s %ld gold piece%s for%s %s %s.  Sell %s?",
				Monnam(shkp), short_funds ? " only" : "",
				offer, plur(offer),
				(!ltmp && cltmp && only_partially_your_contents) ?
					" your items in" :
					(!ltmp && cltmp) ? " the contents of" : "",
				obj->unpaid ? "the" : "your", cxname(obj),
				(obj->quan == 1L &&
				 !(!ltmp && cltmp && only_partially_your_contents)) ?
					"it" :
					"them");
		} else
			qbuf[0] = '\0'; /* just to pacify lint */

		switch (sell_response ? sell_response : ynaq(qbuf)) {
			case 'q':
				sell_response = 'n';
			//fallthru
			case 'n':
				if (container)
					dropped_container(obj, shkp, false);
				if (!obj->unpaid) obj->no_charge = 1;
				subfrombill(obj, shkp);
				break;
			case 'a':
				sell_response = 'y';
			//fallthru
			case 'y':
				if (container)
					dropped_container(obj, shkp, true);
				if (!obj->unpaid && !saleitem) obj->no_charge = 1;
				subfrombill(obj, shkp);
				pay(-offer, shkp);
				shk_names_obj(shkp, obj, (sell_how != SELL_NORMAL) ? (!ltmp && cltmp && only_partially_your_contents) ? "sold some items inside %s for %ld gold pieces%s.%s" : "sold %s for %ld gold piece%s.%s" : "relinquish %s and receive %ld gold piece%s in compensation.%s",
					      offer, "");
				break;
			default:
				impossible("invalid sell response");
		}
	}
}

/* 0: deliver count 1: paged */
int doinvbill(int mode) {
	struct monst *shkp;
	struct eshk *eshkp;
	struct bill_x *bp, *end_bp;
	struct obj *obj;
	long totused;
	char *buf_p;
	winid datawin;

	shkp = shop_keeper(*u.ushops);
	if (!shkp || !inhishop(shkp)) {
		if (mode != 0) impossible("doinvbill: no shopkeeper?");
		return 0;
	}
	eshkp = ESHK(shkp);

	if (mode == 0) {
		/* count expended items, so that the `I' command can decide
		   whether to include 'x' in its prompt string */
		int cnt = !eshkp->debit ? 0 : 1;

		for (bp = eshkp->bill_p, end_bp = &eshkp->bill_p[eshkp->billct];
		     bp < end_bp; bp++)
			if (bp->useup ||
			    ((obj = bp_to_obj(bp)) != 0 && obj->quan < bp->bquan))
				cnt++;
		return cnt;
	}

	datawin = create_nhwindow(NHW_MENU);
	putstr(datawin, 0, "Unpaid articles already used up:");
	putstr(datawin, 0, "");

	totused = 0L;
	for (bp = eshkp->bill_p, end_bp = &eshkp->bill_p[eshkp->billct];
	     bp < end_bp; bp++) {
		obj = bp_to_obj(bp);
		if (!obj) {
			impossible("Bad shopkeeper administration.");
			goto quit;
		}
		if (bp->useup || bp->bquan > obj->quan) {
			long oquan, uquan, thisused;
			unsigned save_unpaid;

			save_unpaid = obj->unpaid;
			oquan = obj->quan;
			uquan = (bp->useup ? bp->bquan : bp->bquan - oquan);
			thisused = bp->price * uquan;
			totused += thisused;
			obj->unpaid = 0; /* ditto */
			/* Why 'x'?  To match `I x', more or less. */
			buf_p = xprname(obj, NULL, 'x', false, thisused, uquan);
			obj->unpaid = save_unpaid;
			putstr(datawin, 0, buf_p);
		}
	}
	if (eshkp->debit) {
		/* additional shop debt which has no itemization available */
		if (totused) putstr(datawin, 0, "");
		totused += eshkp->debit;
		buf_p = xprname(NULL,
				"usage charges and/or other fees",
				GOLD_SYM, false, eshkp->debit, 0L);
		putstr(datawin, 0, buf_p);
	}
	buf_p = xprname(NULL, "Total:", '*', false, totused, 0L);
	putstr(datawin, 0, "");
	putstr(datawin, 0, buf_p);
	display_nhwindow(datawin, false);
quit:
	destroy_nhwindow(datawin);
	return 0;
}

#define HUNGRY 2

static long getprice(struct obj *obj, boolean shk_buying) {
	long tmp = (long)objects[obj->otyp].oc_cost;

	if (obj->oartifact) {
		tmp = arti_cost(obj);
		if (shk_buying) tmp /= 4;
	}
	switch (obj->oclass) {
		case FOOD_CLASS:
			/* simpler hunger check, (2-4)*cost */
			if (u.uhs >= HUNGRY && !shk_buying) tmp *= (long)u.uhs;
			if (obj->oeaten) tmp = 0L;
			break;
		case WAND_CLASS:
			if (obj->spe == -1) tmp = 0L;
			break;
		case POTION_CLASS:
			if (obj->otyp == POT_WATER && !obj->blessed && !obj->cursed)
				tmp = 0L;
			break;
		case ARMOR_CLASS:
		case WEAPON_CLASS:
			if (obj->spe > 0) tmp += 10L * (long)obj->spe;
			/* Don't buy activated explosives! */
			if (is_grenade(obj) && obj->oarmed) tmp = 0L;
			break;
		case TOOL_CLASS:
			if (Is_candle(obj) &&
			    obj->age < 20L * (long)objects[obj->otyp].oc_cost)
				tmp /= 2L;
			else if (obj->otyp == TORCH) {
				if (obj->age == 0) {
					tmp = 0L;
				} else if (obj->age < 25) {
					tmp /= 4L;
				} else if (obj->age < 50) {
					tmp /= 2L;
				}
			}
			break;
	}
	return tmp;
}

/* shk catches thrown pick-axe */
struct monst *shkcatch(struct obj *obj, xchar x, xchar y) {
	struct monst *shkp;

	if (!(shkp = shop_keeper(inside_shop(x, y))) ||
	    !inhishop(shkp)) return 0;

	if (shkp->mcanmove && !shkp->msleeping &&
	    (*u.ushops != ESHK(shkp)->shoproom || !inside_shop(u.ux, u.uy)) &&
	    dist2(shkp->mx, shkp->my, x, y) < 3 &&
	    /* if it is the shk's pos, you hit and anger him */
	    (shkp->mx != x || shkp->my != y)) {
		if (mnearto(shkp, x, y, true))
			verbalize("Out of my way, scum!");
		if (cansee(x, y)) {
			pline("%s nimbly%s catches %s.",
			      Monnam(shkp),
			      (x == shkp->mx && y == shkp->my) ? "" : " reaches over and",
			      the(xname(obj)));
			if (!canspotmon(shkp))
				map_invisible(x, y);
			delay_output();
			mark_synch();
		}
		subfrombill(obj, shkp);
		mpickobj(shkp, obj);
		return shkp;
	}
	return NULL;
}

void add_damage(xchar x, xchar y, long cost) {
	struct damage *tmp_dam;
	char *shops;

	if (IS_DOOR(levl[x][y].typ)) {
		struct monst *mtmp;

		/* Don't schedule for repair unless it's a real shop entrance */
		for (shops = in_rooms(x, y, SHOPBASE); *shops; shops++)
			if ((mtmp = shop_keeper(*shops)) != 0 &&
			    x == ESHK(mtmp)->shd.x && y == ESHK(mtmp)->shd.y)
				break;
		if (!*shops) return;
	}
	for (tmp_dam = level.damagelist; tmp_dam; tmp_dam = tmp_dam->next)
		if (tmp_dam->place.x == x && tmp_dam->place.y == y) {
			tmp_dam->cost += cost;
			return;
		}
	tmp_dam = alloc((unsigned)sizeof(struct damage));
	tmp_dam->when = monstermoves;
	tmp_dam->place.x = x;
	tmp_dam->place.y = y;
	tmp_dam->cost = cost;
	tmp_dam->typ = levl[x][y].typ;
	tmp_dam->next = level.damagelist;
	level.damagelist = tmp_dam;
	/* If player saw damage, display as a wall forever */
	if (cansee(x, y))
		levl[x][y].seenv = SVALL;
}

/*
 * Do something about damage. Either (!croaked) try to repair it, or
 * (croaked) just discard damage structs for non-shared locations, since
 * they'll never get repaired. Assume that shared locations will get
 * repaired eventually by the other shopkeeper(s). This might be an erroneous
 * assumption (they might all be dead too), but we have no reasonable way of
 * telling that.
 */
static void remove_damage(struct monst *shkp, bool croaked) {
	struct damage *tmp_dam, *tmp2_dam;
	bool did_repair = false, saw_door = false;
	bool saw_floor = false, stop_picking = false;
	bool doorway_trap = false;
	int saw_walls = 0, saw_untrap = 0;
	char trapmsg[BUFSZ];

	tmp_dam = level.damagelist;
	tmp2_dam = 0;
	while (tmp_dam) {
		xchar x = tmp_dam->place.x, y = tmp_dam->place.y;
		char shops[5];
		int disposition;
		uint old_doormask = 0;

		disposition = 0;
		strcpy(shops, in_rooms(x, y, SHOPBASE));
		if (index(shops, ESHK(shkp)->shoproom)) {
			if (IS_DOOR(levl[x][y].typ))
				old_doormask = levl[x][y].doormask;

			if (croaked)
				disposition = (shops[1]) ? 0 : 1;
			else if (stop_picking)
				disposition = repair_damage(shkp, tmp_dam, false);
			else {
				/* Defer the stop_occupation() until after repair msgs */
				if (closed_door(x, y))
					stop_picking = picking_at(x, y);
				disposition = repair_damage(shkp, tmp_dam, false);
				if (!disposition)
					stop_picking = false;
			}
		}

		if (!disposition) {
			tmp2_dam = tmp_dam;
			tmp_dam = tmp_dam->next;
			continue;
		}

		if (disposition > 1) {
			did_repair = true;
			if (cansee(x, y)) {
				if (IS_WALL(levl[x][y].typ)) {
					saw_walls++;
				} else if (IS_DOOR(levl[x][y].typ) && !(old_doormask & (D_ISOPEN|D_CLOSED))) { // an existing door here implies trap removal
					saw_door = true;
				} else if (disposition == 3) { // untrapped
					saw_untrap++;
					if (IS_DOOR(levl[x][y].typ)) doorway_trap = true;
				} else {
					saw_floor = true;
				}
			}
		}

		tmp_dam = tmp_dam->next;
		if (!tmp2_dam) {
			free(level.damagelist);
			level.damagelist = tmp_dam;
		} else {
			free(tmp2_dam->next);
			tmp2_dam->next = tmp_dam;
		}
	}
	if (!did_repair)
		return;
	if (saw_untrap) {
		sprintf(trapmsg, "%s trap%s",
				(saw_untrap > 3) ? "several" :
				(saw_untrap > 1) ? "some" : "a",
				plur(saw_untrap));
		sprintf(eos(trapmsg), " %s", vtense(trapmsg, "are"));
		sprintf(eos(trapmsg), " removed from the %s",
				(doorway_trap && saw_untrap == 1) ? "doorway" : "floor");
	} else {
		trapmsg[0] = '\0';
	}

	if (saw_walls) {
		char wallbuf[BUFSZ];

		sprintf(wallbuf, "section%s", plur(saw_walls));

		pline("Suddenly, %s %s of wall %s up!",
		      (saw_walls == 1) ? "a" : (saw_walls <= 3) ? "some" : "several",
		      wallbuf, vtense(wallbuf, "close"));
		if (saw_door)
			pline("The shop door reappears!");
		if (saw_floor)
			pline("The floor is repaired!");
		if (saw_untrap)
			pline("%s!", upstart(trapmsg));
	} else {
		if (saw_door || saw_floor || saw_untrap)
			pline("Suddenly, %s%s%s%s%s!",
				saw_door ? "the shop door reappears" : "",
				(saw_door && saw_floor) ? " and " : "",
				saw_floor ? "the floor damage is gone" : "",
				((saw_door || saw_floor) && *trapmsg) ? " and " : "",
				trapmsg);
		else if (inside_shop(u.ux, u.uy) == ESHK(shkp)->shoproom)
			pline("You feel more claustrophobic than before.");
		else if (!Deaf && !rn2(10))
			Norep("The dungeon acoustics noticeably change.");
	}
	if (stop_picking)
		stop_occupation();
}

/*
 * 0: repair postponed, 1: silent repair (no messages), 2: normal repair
 * 3: untrap
 */

// catchup => restoring a level
int repair_damage(struct monst *shkp, struct damage *tmp_dam, boolean catchup) {
	xchar x, y, i;
	xchar litter[9];
	struct monst *mtmp;
	struct obj *otmp;
	struct trap *ttmp;

	if ((monstermoves - tmp_dam->when) < REPAIR_DELAY)
		return 0;
	if (shkp->msleeping || !shkp->mcanmove || ESHK(shkp)->following)
		return 0;
	x = tmp_dam->place.x;
	y = tmp_dam->place.y;
	if (!IS_ROOM(tmp_dam->typ)) {
		if (x == u.ux && y == u.uy)
			if (!Passes_walls)
				return 0;
		if (x == shkp->mx && y == shkp->my)
			return 0;
		if ((mtmp = m_at(x, y)) && (!passes_walls(mtmp->data)))
			return 0;
	}
	if ((ttmp = t_at(x, y)) != 0) {
		if (x == u.ux && y == u.uy)
			if (!Passes_walls)
				return 0;
		if (ttmp->ttyp == LANDMINE || ttmp->ttyp == BEAR_TRAP) {
			/* convert to an object */
			otmp = mksobj((ttmp->ttyp == LANDMINE) ? LAND_MINE :
								 BEARTRAP,
				      true, false);
			otmp->quan = 1;
			otmp->owt = weight(otmp);
			mpickobj(shkp, otmp);
		}
		deltrap(ttmp);
		if (IS_DOOR(tmp_dam->typ) && !(levl[x][y].doormask & D_ISOPEN)) {
			levl[x][y].doormask = D_CLOSED;
			block_point(x, y);
		} else if (IS_WALL(tmp_dam->typ)) {
			levl[x][y].typ = tmp_dam->typ;
			block_point(x, y);
		}
		newsym(x, y);
		return 3;
	}
	if (IS_ROOM(tmp_dam->typ)) {
		/* No messages, because player already filled trap door */
		return 1;
	}
	if ((tmp_dam->typ == levl[x][y].typ) &&
	    (!IS_DOOR(tmp_dam->typ) || (levl[x][y].doormask > D_BROKEN)))
		/* No messages if player already replaced shop door */
		return 1;
	levl[x][y].typ = tmp_dam->typ;
	memset((void *)litter, 0, sizeof(litter));
	if ((otmp = level.objects[x][y]) != 0) {
		/* Scatter objects haphazardly into the shop */
#define NEED_UPDATE 1
#define OPEN	    2
#define INSHOP	    4
#define horiz(i)    ((i % 3) - 1)
#define vert(i)	    ((i / 3) - 1)
		for (i = 0; i < 9; i++) {
			if ((i == 4) || (!ZAP_POS(levl[x + horiz(i)][y + vert(i)].typ)))
				continue;
			litter[i] = OPEN;
			if (inside_shop(x + horiz(i),
					y + vert(i)) == ESHK(shkp)->shoproom)
				litter[i] |= INSHOP;
		}
		if (Punished && !u.uswallow &&
		    ((uchain->ox == x && uchain->oy == y) ||
		     (uball->ox == x && uball->oy == y))) {
			/*
			 * Either the ball or chain is in the repair location.
			 *
			 * Take the easy way out and put ball&chain under hero.
			 */
			verbalize("Get your junk out of my wall!");
			unplacebc(); /* pick 'em up */
			placebc();   /* put 'em down */
		}
		while ((otmp = level.objects[x][y]) != 0)
			/* Don't mess w/ boulders -- just merge into wall */
			if ((otmp->otyp == BOULDER) || (otmp->otyp == ROCK)) {
				obj_extract_self(otmp);
				obfree(otmp, NULL);
			} else {
				while (!(litter[i = rn2(9)] & INSHOP))
					;
				remove_object(otmp);
				place_object(otmp, x + horiz(i), y + vert(i));
				litter[i] |= NEED_UPDATE;
			}
	}
	if (catchup) return 1; /* repair occurred while off level */

	block_point(x, y);
	if (IS_DOOR(tmp_dam->typ)) {
		levl[x][y].doormask = D_CLOSED; /* arbitrary */
		newsym(x, y);
	} else {
		/* don't set doormask  - it is (hopefully) the same as it was */
		/* if not, perhaps save it with the damage array...  */

		if (IS_WALL(tmp_dam->typ) && cansee(x, y)) {
			/* Player sees actual repair process, so they KNOW it's a wall */
			levl[x][y].seenv = SVALL;
			newsym(x, y);
		}
		/* Mark this wall as "repaired".  There currently is no code */
		/* to do anything about repaired walls, so don't do it.	 */
	}
	for (i = 0; i < 9; i++)
		if (litter[i] & NEED_UPDATE)
			newsym(x + horiz(i), y + vert(i));
	return 2;
#undef NEED_UPDATE
#undef OPEN
#undef INSHOP
#undef vert
#undef horiz
}
/*
 * shk_move: return 1: moved  0: didn't  -1: let m_move do it  -2: died
 */
int shk_move(struct monst *shkp) {
	xchar gx, gy, omx, omy;
	int udist;
	schar appr;
	struct eshk *eshkp = ESHK(shkp);
	int z;
	boolean uondoor = false, satdoor, avoid = false, badinv;

	omx = shkp->mx;
	omy = shkp->my;

	if (inhishop(shkp))
		remove_damage(shkp, false);

	if ((udist = distu(omx, omy)) < 3 &&
	    (shkp->data != &mons[PM_GRID_BUG] || (omx == u.ux || omy == u.uy))) {
		if (ANGRY(shkp) ||
		    (Conflict && !resist(shkp, RING_CLASS, 0, 0))) {
			if (Displaced)
				pline("Your displaced image doesn't fool %s!",
				      mon_nam(shkp));
			mattacku(shkp);
			return 0;
		}
		if (eshkp->following) {
			if (strncmp(eshkp->customer, plname, PL_NSIZ)) {
				verbalize("%s, %s!  I was looking for %s.",
					  Hello(shkp), plname, eshkp->customer);
				eshkp->following = 0;
				return 0;
			}
			if (moves > followmsg + 4) {
				verbalize("%s, %s!  Didn't you forget to pay?",
					  Hello(shkp), plname);
				followmsg = moves;
				if (!rn2(9)) {
					pline("%s doesn't like customers who don't pay.",
					      Monnam(shkp));
					rile_shk(shkp);
				}
			}
			if (udist < 2)
				return 0;
		}
	}

	appr = 1;
	gx = eshkp->shk.x;
	gy = eshkp->shk.y;
	satdoor = (gx == omx && gy == omy);
	if (eshkp->following || ((z = holetime()) >= 0 && z * z <= udist)) {
		/* [This distance check used to apply regardless of
		    whether the shk was following, but that resulted in
		    m_move() sometimes taking the shk out of the shop if
		    the player had fenced him in with boulders or traps.
		    Such voluntary abandonment left unpaid objects in
		    invent, triggering billing impossibilities on the
		    next level once the character fell through the hole.] */
		if (udist > 4 && eshkp->following && !eshkp->billct)
			return -1; /* leave it to m_move */
		gx = u.ux;
		gy = u.uy;
	} else if (ANGRY(shkp)) {
		/* Move towards the hero if the shopkeeper can see him. */
		if (shkp->mcansee && m_canseeu(shkp)) {
			gx = u.ux;
			gy = u.uy;
		}
		avoid = false;
	} else {
#define GDIST(x, y) (dist2(x, y, gx, gy))
		if ((Is_blackmarket(&u.uz) && u.umonnum > 0 &&
		     mons[u.umonnum].mlet != S_HUMAN) ||
		    /* WAC Let you out if you're stuck inside */
		    (!Is_blackmarket(&u.uz) && (Invis || u.usteed) && !inside_shop(u.ux, u.uy))) {
			avoid = false;
		} else {
			uondoor = (u.ux == eshkp->shd.x && u.uy == eshkp->shd.y);
			if (uondoor) {
				badinv = (carrying(PICK_AXE) || carrying(DWARVISH_MATTOCK) ||
					  (Fast && (sobj_at(PICK_AXE, u.ux, u.uy) ||
						    sobj_at(DWARVISH_MATTOCK, u.ux, u.uy))));
				if (satdoor && badinv)
					return 0;
				avoid = !badinv;
			} else {
				avoid = (*u.ushops && distu(gx, gy) > 8);
				badinv = false;
			}

			if (((!eshkp->robbed && !eshkp->billct && !eshkp->debit) || avoid) && GDIST(omx, omy) < 3) {
				if (!badinv && !onlineu(omx, omy))
					return 0;
				if (satdoor)
					appr = gx = gy = 0;
			}
		}
	}

	z = move_special(shkp, inhishop(shkp), appr, uondoor, avoid, omx, omy, gx, gy);
	if (z > 0) after_shk_move(shkp);

	return z;
}

/* called after shopkeeper moves, in case the move causes re-entry into shop */
void after_shk_move(struct monst *shkp) {
	struct eshk *eshkp = ESHK(shkp);

	if (eshkp->bill_p == (struct bill_x *)-1000 && inhishop(shkp)) {
		/* reset bill_p, need to re-calc player's occupancy too */
		eshkp->billsz = 0;
		eshkp->bill_p = NULL;
		check_special_room(false);
	}
}

/* for use in levl_follower (mondata.c) */
boolean is_fshk(struct monst *mtmp) {
	return mtmp->isshk && ESHK(mtmp)->following;
}

/* You are digging in the shop. */
void shopdig(int fall) {
	struct monst *shkp = shop_keeper(*u.ushops);
	int lang;
	const char *grabs = "grabs";

	if (!shkp) return;

	/* 0 == can't speak, 1 == makes animal noises, 2 == speaks */
	lang = 0;
	if (shkp->msleeping || !shkp->mcanmove || is_silent(shkp->data))
		; /* lang stays 0 */
	else if (shkp->data->msound <= MS_ANIMAL)
		lang = 1;
	else if (shkp->data->msound >= MS_HUMANOID)
		lang = 2;

	if (!inhishop(shkp)) {
		if (Role_if(PM_KNIGHT)) {
			pline("You feel like a common thief.");
			adjalign(-sgn(u.ualign.type));
		}
		/* WAC He may not be here now,  but... */
		make_angry_shk(shkp, 0, 0); /* No spot in particular*/
		return;
	}

	if (!fall) {
		if (lang == 2) {
			if (u.utraptype == TT_PIT)
				verbalize(
					"Be careful, %s, or you might fall through the floor.",
					flags.female ? "madam" : "sir");
			else
				verbalize("%s, do not damage the floor here!",
					  flags.female ? "Madam" : "Sir");
		}
		if (Role_if(PM_KNIGHT)) {
			pline("You feel like a common thief.");
			adjalign(-sgn(u.ualign.type));
		}
	} else if (!um_dist(shkp->mx, shkp->my, 5) &&
		   !shkp->msleeping && shkp->mcanmove &&
		   (ESHK(shkp)->billct || ESHK(shkp)->debit)) {
		struct obj *obj, *obj2;
		if (nolimbs(shkp->data)) {
			grabs = "knocks off";
#if 0
			/* This is what should happen, but for balance
			 * reasons, it isn't currently.
			 */
			if (lang == 2)
				pline("%s curses %s inability to grab your backpack!",
				      shkname(shkp), mhim(shkp));
			rile_shk(shkp);
			return;
#endif
		}
		if (distu(shkp->mx, shkp->my) > 2) {
			mnexto(shkp);
			/* for some reason the shopkeeper can't come next to you */
			if (distu(shkp->mx, shkp->my) > 2) {
				if (lang == 2)
					pline("%s curses you in anger and frustration!",
					      shkname(shkp));
				rile_shk(shkp);
				return;
			} else
				pline("%s %s, and %s your backpack!",
				      shkname(shkp),
				      makeplural(locomotion(shkp->data, "leap")), grabs);
		} else
			pline("%s %s your backpack!", shkname(shkp), grabs);

		for (obj = invent; obj; obj = obj2) {
			obj2 = obj->nobj;
			if ((obj->owornmask & ~(W_SWAPWEP | W_QUIVER)) != 0 ||
			    (obj == uswapwep && u.twoweap) ||
			    (obj->otyp == LEASH && obj->leashmon)) continue;
			if (obj == current_wand) continue;
			setnotworn(obj);
			freeinv(obj);
			subfrombill(obj, shkp);
			add_to_minv(shkp, obj); /* may free obj */
		}
	} else
		/* WAC He may not be here now,  but... */
		rile_shk(shkp);
}

/* modified by M. Campostrini (campo@sunthpi3.difi.unipi.it) */
/* to allow for multiple choices of kops */
static void makekops(coord *mm) {
	int kop_cnt[5];
	int kop_pm[5];
	int ik, cnt;
	coord *mc;

	kop_pm[0] = PM_KEYSTONE_KOP;
	kop_pm[1] = PM_KOP_SERGEANT;
	kop_pm[2] = PM_KOP_LIEUTENANT;
	kop_pm[3] = PM_KOP_KAPTAIN;
	kop_pm[4] = 0;

	cnt = abs(depth(&u.uz)) + rnd(5);

	if (Is_blackmarket(&u.uz)) {
		kop_pm[0] = PM_SOLDIER;
		kop_pm[1] = PM_SERGEANT;
		kop_pm[2] = PM_LIEUTENANT;
		kop_pm[3] = PM_CAPTAIN;
		kop_pm[4] = 0;

		cnt = 7 + rnd(10);
	}

	kop_cnt[0] = cnt;
	kop_cnt[1] = (cnt / 3) + 1; /* at least one sarge */
	kop_cnt[2] = (cnt / 6);	    /* maybe a lieutenant */
	kop_cnt[3] = (cnt / 9);	    /* and maybe a kaptain */

	mc = alloc(cnt * sizeof(coord));
	for (ik = 0; kop_pm[ik]; ik++) {
		if (!(mvitals[kop_pm[ik]].mvflags & G_GONE)) {
			cnt = epathto(mc, kop_cnt[ik], mm->x, mm->y, &mons[kop_pm[ik]]);
			while (--cnt >= 0)
				makemon(&mons[kop_pm[ik]], mc[cnt].x, mc[cnt].y, NO_MM_FLAGS);
		}
	}
	free(mc);
}

void pay_for_damage(const char *dmgstr, boolean cant_mollify) {
	struct monst *shkp = NULL;
	char shops_affected[5];
	boolean uinshp = (*u.ushops != '\0');
	char qbuf[80];
	xchar x, y;
	boolean dugwall = !strcmp(dmgstr, "dig into") || /* wand */
			  !strcmp(dmgstr, "damage");	 /* pick-axe */
	struct damage *tmp_dam, *appear_here = 0;
	/* any number >= (80*80)+(25*25) would do, actually */
	long cost_of_damage = 0L;
	uint nearest_shk = 8000, nearest_damage = 8000;
	int picks = 0;

	for (tmp_dam = level.damagelist;
	     (tmp_dam && (tmp_dam->when == monstermoves));
	     tmp_dam = tmp_dam->next) {
		char *shp;

		if (!tmp_dam->cost)
			continue;
		cost_of_damage += tmp_dam->cost;
		strcpy(shops_affected,
		       in_rooms(tmp_dam->place.x, tmp_dam->place.y, SHOPBASE));
		for (shp = shops_affected; *shp; shp++) {
			struct monst *tmp_shk;
			uint shk_distance;

			if (!(tmp_shk = shop_keeper(*shp)))
				continue;
			if (tmp_shk == shkp) {
				uint damage_distance =
					distu(tmp_dam->place.x, tmp_dam->place.y);

				if (damage_distance < nearest_damage) {
					nearest_damage = damage_distance;
					appear_here = tmp_dam;
				}
				continue;
			}
			if (!inhishop(tmp_shk))
				continue;
			shk_distance = distu(tmp_shk->mx, tmp_shk->my);
			if (shk_distance > nearest_shk)
				continue;
			if ((shk_distance == nearest_shk) && picks) {
				if (rn2(++picks))
					continue;
			} else
				picks = 1;
			shkp = tmp_shk;
			nearest_shk = shk_distance;
			appear_here = tmp_dam;
			nearest_damage = distu(tmp_dam->place.x, tmp_dam->place.y);
		}
	}

	if (!cost_of_damage || !shkp)
		return;

	x = appear_here->place.x;
	y = appear_here->place.y;

	/* not the best introduction to the shk... */
	strncpy(ESHK(shkp)->customer, plname, PL_NSIZ);

	/* if the shk is already on the war path, be sure it's all out */
	if (ANGRY(shkp) || ESHK(shkp)->following) {
		hot_pursuit(shkp);
		return;
	}

	/* if the shk is not in their shop.. */
	if (!*in_rooms(shkp->mx, shkp->my, SHOPBASE)) {
		if (!cansee(shkp->mx, shkp->my))
			return;
		goto getcad;
	}

	if (uinshp) {
		if (um_dist(shkp->mx, shkp->my, 1) &&
		    !um_dist(shkp->mx, shkp->my, 3)) {
			pline("%s leaps towards you!", shkname(shkp));
			mnexto(shkp);
		}
		if (um_dist(shkp->mx, shkp->my, 1)) goto getcad;
	} else {
		/*
		 * Make shkp show up at the door.  Effect:  If there is a monster
		 * in the doorway, have the hero hear the shopkeeper yell a bit,
		 * pause, then have the shopkeeper appear at the door, having
		 * yanked the hapless critter out of the way.
		 */
		if (MON_AT(x, y)) {
			if (!Deaf) {
				You_hear("an angry voice:");
				verbalize("Out of my way, scum!");
				wait_synch();
#if defined(UNIX) && !defined(SYSV)
				sleep(1);
#endif
			}
		}
		mnearto(shkp, x, y, true);
	}

	if ((um_dist(x, y, 1) && !uinshp) || cant_mollify ||
	    (money_cnt(invent) + ESHK(shkp)->credit) < cost_of_damage || !rn2(50)) {
		if (um_dist(x, y, 1) && !uinshp) {
			pline("%s shouts:", shkname(shkp));
			verbalize("Who dared %s my %s?", dmgstr,
				  dugwall ? "shop" : "door");
		} else {
		getcad:
			verbalize("How dare you %s my %s?", dmgstr,
				  dugwall ? "shop" : "door");
		}
		hot_pursuit(shkp);
		return;
	}

	if (Invis) pline("Your invisibility does not fool %s!", shkname(shkp));
	sprintf(qbuf, "\"Cad!  You did %ld %s worth of damage!\"  Pay? ",
		cost_of_damage, currency(cost_of_damage));
	if (yn(qbuf) != 'n') {
		cost_of_damage = check_credit(cost_of_damage, shkp);
		money2mon(shkp, cost_of_damage);
		context.botl = 1;
		pline("Mollified, %s accepts your restitution.",
		      shkname(shkp));
		/* move shk back to his home loc */
		home_shk(shkp, false);
		pacify_shk(shkp);
	} else {
		verbalize("Oh, yes!  You'll pay!");
		hot_pursuit(shkp);
		adjalign(-sgn(u.ualign.type));
	}
}
/* called in dokick.c when we kick an object that might be in a store */
boolean costly_spot(xchar x, xchar y) {
	struct monst *shkp;

	if (!level.flags.has_shop) return false;
	shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));
	if (!shkp || !inhishop(shkp)) return false;

	return inside_shop(x, y) &&
	       !(x == ESHK(shkp)->shk.x &&
		 y == ESHK(shkp)->shk.y);
}

/* called by dotalk(sounds.c) when #chatting; returns obj if location
   contains shop goods and shopkeeper is willing & able to speak */
struct obj *shop_object(xchar x, xchar y) {
	struct obj *otmp;
	struct monst *shkp;

	if (!(shkp = shop_keeper(*in_rooms(x, y, SHOPBASE))) || !inhishop(shkp))
		return NULL;

	for (otmp = level.objects[x][y]; otmp; otmp = otmp->nexthere)
		if (otmp->oclass != COIN_CLASS)
			break;
	/* note: otmp might have ->no_charge set, but that's ok */
	return (otmp && costly_spot(x, y) && NOTANGRY(shkp) && shkp->mcanmove && !shkp->msleeping) ? otmp : NULL;
}

/* give price quotes for all objects linked to this one (ie, on this spot) */
void price_quote(struct obj *first_obj) {
	struct obj *otmp;
	char buf[BUFSZ], price[40];
	long cost;
	int cnt = 0;
	winid tmpwin;
	struct monst *shkp = shop_keeper(inside_shop(u.ux, u.uy));

	tmpwin = create_nhwindow(NHW_MENU);
	putstr(tmpwin, 0, "Fine goods for sale:");
	putstr(tmpwin, 0, "");
	for (otmp = first_obj; otmp; otmp = otmp->nexthere) {
		if (otmp->oclass == COIN_CLASS) continue;
		cost = (otmp->no_charge || otmp == uball || otmp == uchain) ? 0L :
									      get_cost(otmp, NULL);
		if (Has_contents(otmp))
			cost += contained_cost(otmp, shkp, 0L, false, false);
		if (!cost) {
			strcpy(price, "no charge");
		} else {
			sprintf(price, "%ld %s%s", cost, currency(cost),
				otmp->quan > 1L ? " each" : "");
		}
		sprintf(buf, "%s, %s", doname(otmp), price);
		putstr(tmpwin, 0, buf), cnt++;
	}
	if (cnt > 1) {
		display_nhwindow(tmpwin, true);
	} else if (cnt == 1) {
		if (first_obj->no_charge || first_obj == uball || first_obj == uchain) {
			pline("%s!", buf); /* buf still contains the string */
		} else {
			/* print cost in slightly different format, so can't reuse buf */
			cost = get_cost(first_obj, NULL);
			if (Has_contents(first_obj))
				cost += contained_cost(first_obj, shkp, 0L, false, false);
			pline("%s, price %ld %s%s%s", doname(first_obj),
			      cost, currency(cost), first_obj->quan > 1L ? " each" : "",
			      shk_embellish(first_obj, cost));
		}
	}
	destroy_nhwindow(tmpwin);
}

static const char *shk_embellish(struct obj *itm, long cost) {
	if (!rn2(3)) {
		int o, choice = rn2(5);
		if (choice == 0) choice = (cost < 100L ? 1 : cost < 500L ? 2 : 3);
		switch (choice) {
			case 4:
				if (cost < 10L)
					break;
				else
					o = itm->oclass;
				if (o == FOOD_CLASS) return ", gourmets' delight!";
				if (objects[itm->otyp].oc_name_known ? objects[itm->otyp].oc_magic : (o == AMULET_CLASS || o == RING_CLASS || o == WAND_CLASS || o == POTION_CLASS || o == SCROLL_CLASS || o == SPBOOK_CLASS))
					return ", painstakingly developed!";
				return ", superb craftsmanship!";
			case 3:
				return ", finest quality.";
			case 2:
				return ", an excellent choice.";
			case 1:
				return ", a real bargain.";
			default:
				break;
		}
	} else if (itm->oartifact) {
		return ", one of a kind!";
	}
	return ".";
}

/* First 4 supplied by Ronen and Tamar, remainder by development team */
const char *Izchak_speaks[] = {
	"%s says: 'These shopping malls give me a headache.'",
	"%s says: 'Slow down.  Think clearly.'",
	"%s says: 'You need to take things one at a time.'",
	"%s says: 'I don't like poofy coffee... give me Columbian Supremo.'",
	"%s says that getting the devteam's agreement on anything is difficult.",
	"%s says that he has noticed those who serve their deity will prosper.",
	"%s says: 'Don't try to steal from me - I have friends in high places!'",
	"%s says: 'You may well need something from this shop in the future.'",
	"%s comments about the Valley of the Dead as being a gateway."};

void shk_chat(struct monst *shkp) {
	struct eshk *eshk;
	long shkmoney;
	if (!shkp->isshk) {
		/* The monster type is shopkeeper, but this monster is
		   not actually a shk, which could happen if someone
		   wishes for a shopkeeper statue and then animates it.
		   (Note: shkname() would be "" in a case like this.) */
		pline("%s asks whether you've seen any untended shops recently.",
		      Monnam(shkp));
		/* [Perhaps we ought to check whether this conversation
		   is taking place inside an untended shop, but a shopless
		   shk can probably be expected to be rather disoriented.] */
		return;
	}

	eshk = ESHK(shkp);
	if (ANGRY(shkp))
		pline("%s mentions how much %s dislikes %s customers.",
		      shkname(shkp), mhe(shkp),
		      eshk->robbed ? "non-paying" : "rude");
	else if (eshk->following) {
		if (strncmp(eshk->customer, plname, PL_NSIZ)) {
			verbalize("%s %s!  I was looking for %s.",
				  Hello(shkp), plname, eshk->customer);
			eshk->following = 0;
		} else {
			verbalize("%s %s!  Didn't you forget to pay?",
				  Hello(shkp), plname);
		}
	} else if (eshk->billct) {
		long total = addupbill(shkp) + eshk->debit;
		pline("%s says that your bill comes to %ld %s.",
		      shkname(shkp), total, currency(total));
	} else if (eshk->debit)
		pline("%s reminds you that you owe %s %ld %s.",
		      shkname(shkp), mhim(shkp),
		      eshk->debit, currency(eshk->debit));
	else if (eshk->credit)
		pline("%s encourages you to use your %ld %s of credit.",
		      shkname(shkp), eshk->credit, currency(eshk->credit));
	else if (eshk->robbed)
		pline("%s complains about a recent robbery.", shkname(shkp));
	else if ((shkmoney = money_cnt(shkp->minvent)) < 50)
		pline("%s complains that business is bad.", shkname(shkp));
	else if (shkmoney > 4000)
		pline("%s says that business is good.", shkname(shkp));
	else if (strcmp(shkname(shkp), "Izchak") == 0)
		pline(Izchak_speaks[rn2(SIZE(Izchak_speaks))], shkname(shkp));
	else
		pline("%s talks about the problem of shoplifters.", shkname(shkp));
}

static void kops_gone(boolean silent) {
	int cnt = 0;
	struct monst *mtmp, *mtmp2;

	for (mtmp = fmon; mtmp; mtmp = mtmp2) {
		mtmp2 = mtmp->nmon;
		if (mtmp->data->mlet == S_KOP) {
			if (canspotmon(mtmp)) cnt++;
			mongone(mtmp);
		}
	}
	if (cnt && !silent)
		pline("The Kop%s (disappointed) vanish%s into thin air.",
		      plur(cnt), cnt == 1 ? "es" : "");
}

/* altusage: some items have an "alternate" use with different cost */
static long cost_per_charge(struct monst *shkp, struct obj *otmp, boolean altusage) {
	long tmp = 0L;

	if (!shkp || !inhishop(shkp)) return 0L; /* insurance */
	tmp = get_cost(otmp, shkp);

	/* The idea is to make the exhaustive use of */
	/* an unpaid item more expensive than buying */
	/* it outright.				     */
	/* KMH, balance patch -- removed abusive orbs */
	if (otmp->otyp == MAGIC_LAMP /*||
	   otmp->otyp == ORB_OF_DESTRUCTION*/
	) {			     /* 1 */
		/* normal use (ie, as light source) of a magic lamp never
		   degrades its value, but not charging anything would make
		   identifcation too easy; charge an amount comparable to
		   what is charged for an ordinary lamp (don't bother with
		   angry shk surchage) */
		if (!altusage)
			tmp = (long)objects[OIL_LAMP].oc_cost;
		else
			tmp += tmp / 3L;	 /* djinni is being released */
	} else if (otmp->otyp == MAGIC_MARKER) { /* 70 - 100 */
		/* no way to determine in advance   */
		/* how many charges will be wasted. */
		/* so, arbitrarily, one half of the */
		/* price per use.		    */
		tmp /= 2L;
	} else if (otmp->otyp == BAG_OF_TRICKS || /* 1 - 20 */
		   otmp->otyp == MEDICAL_KIT ||
		   otmp->otyp == HORN_OF_PLENTY) {
		tmp /= 5L;
	} else if (otmp->otyp == CRYSTAL_BALL || /* 1 - 5 */
		   /*otmp->otyp == ORB_OF_ENCHANTMENT ||
	                otmp->otyp == ORB_OF_CHARGING ||*/
		   otmp->otyp == OIL_LAMP || /* 1 - 10 */
		   otmp->otyp == BRASS_LANTERN ||
		   (otmp->otyp >= MAGIC_FLUTE &&
		    otmp->otyp <= DRUM_OF_EARTHQUAKE) || /* 5 - 9 */
		   otmp->oclass == WAND_CLASS) {	 /* 3 - 11 */
		if (otmp->spe > 1) tmp /= 4L;
	} else if (otmp->otyp == TORCH) {
		tmp /= 2L;
	} else if (otmp->oclass == SPBOOK_CLASS) {
		/* Normal use is studying. Alternate use is using up a charge */
		if (altusage)
			tmp /= 10L; /* 2 - 4 */
		else
			tmp -= tmp / 5L;
	} else if (otmp->otyp == CAN_OF_GREASE || otmp->otyp == TINNING_KIT || otmp->otyp == EXPENSIVE_CAMERA) {
		tmp /= 10L;
	} else if (otmp->otyp == POT_OIL) {
		tmp /= 5L;
	}
	return tmp;
}

/* Charge the player for partial use of an unpaid object.
 *
 * Note that bill_dummy_object() should be used instead
 * when an object is completely used.
 */
void check_unpaid_usage(struct obj *otmp, boolean altusage) {
	struct monst *shkp;
	const char *fmt, *arg1, *arg2;
	long tmp;

	/* MRKR: Torches are a special case. As weapons they can have */
	/*       a 'charge' == plus value, which is independent of their */
	/*       use as a light source. */

	/* WAC - now checks for items that aren't carried */
	if ((!otmp->unpaid || !*u.ushops ||
	     (otmp->spe <= 0 && objects[otmp->otyp].oc_charged &&
	      otmp->otyp != TORCH)) &&
	    (carried(otmp) || !costly_spot(otmp->ox, otmp->oy) ||
	     otmp->no_charge))
		return;
	if (!(shkp = shop_keeper(*u.ushops)) || !inhishop(shkp))
		return;
	if ((tmp = cost_per_charge(shkp, otmp, altusage)) == 0L)
		return;

	arg1 = arg2 = "";
	if (otmp->oclass == SPBOOK_CLASS && !altusage) {
		fmt = "%sYou owe%s %ld %s.";
		arg1 = rn2(2) ? "This is no free library, cad!  " : "";
		arg2 = ESHK(shkp)->debit > 0L ? " an additional" : "";
	} else if (otmp->otyp == POT_OIL) {
		fmt = "%s%sThat will cost you %ld %s (Yendorian Fuel Tax).";
	} else {
		fmt = "%s%sUsage fee, %ld %s.";
		if (!rn2(3)) arg1 = "Hey!  ";
		if (!rn2(3)) arg2 = "Ahem.  ";
	}

	if (shkp->mcanmove || !shkp->msleeping)
		verbalize(fmt, arg1, arg2, tmp, currency(tmp));
	ESHK(shkp)->debit += tmp;
	exercise(A_WIS, true); /* you just got info */
}

/* for using charges of unpaid objects "used in the normal manner" */
void check_unpaid(struct obj *otmp) {
	check_unpaid_usage(otmp, false); /* normal item use */
}

void costly_gold(xchar x, xchar y, long amount) {
	long delta;
	struct monst *shkp;
	struct eshk *eshkp;

	if (!costly_spot(x, y)) return;
	/* shkp now guaranteed to exist by costly_spot() */
	shkp = shop_keeper(*in_rooms(x, y, SHOPBASE));

	eshkp = ESHK(shkp);
	if (eshkp->credit >= amount) {
		if (eshkp->credit > amount)
			pline("Your credit is reduced by %ld %s.",
			      amount, currency(amount));
		else
			pline("Your credit is erased.");
		eshkp->credit -= amount;
	} else {
		delta = amount - eshkp->credit;
		if (eshkp->credit)
			pline("Your credit is erased.");
		if (eshkp->debit)
			pline("Your debt increases by %ld %s.",
			      delta, currency(delta));
		else
			pline("You owe %s %ld %s.",
			      shkname(shkp), delta, currency(delta));
		eshkp->debit += delta;
		eshkp->loan += delta;
		eshkp->credit = 0L;
	}
}

/* used in domove to block diagonal shop-exit */
/* x,y should always be a door */
boolean block_door(xchar x, xchar y) {
	int roomno = *in_rooms(x, y, SHOPBASE);
	struct monst *shkp;

	if (roomno < 0 || !IS_SHOP(roomno)) return false;
	if (!IS_DOOR(levl[x][y].typ)) return false;
	if (roomno != *u.ushops) return false;

	if (!(shkp = shop_keeper((char)roomno)) || !inhishop(shkp))
		return false;

	if (shkp->mx == ESHK(shkp)->shk.x && shkp->my == ESHK(shkp)->shk.y
	    /* Actually, the shk should be made to block _any_
	                 * door, including a door the player digs, if the
	                 * shk is within a 'jumping' distance.
	                 */
	    && ESHK(shkp)->shd.x == x && ESHK(shkp)->shd.y == y && shkp->mcanmove && !shkp->msleeping && (ESHK(shkp)->debit || ESHK(shkp)->billct || ESHK(shkp)->robbed)) {
		pline("%s%s blocks your way!", shkname(shkp),
		      Invis ? " senses your motion and" : "");
		return true;
	}
	return false;
}

/* used in domove to block diagonal shop-entry */
/* u.ux, u.uy should always be a door */
boolean block_entry(xchar x, xchar y) {
	xchar sx, sy;
	int roomno;
	struct monst *shkp;

	if (!(IS_DOOR(levl[u.ux][u.uy].typ) &&
	      levl[u.ux][u.uy].doormask == D_BROKEN)) return false;

	roomno = *in_rooms(x, y, SHOPBASE);
	if (roomno < 0 || !IS_SHOP(roomno)) return false;
	if (!(shkp = shop_keeper((char)roomno)) || !inhishop(shkp))
		return false;

	if (ESHK(shkp)->shd.x != u.ux || ESHK(shkp)->shd.y != u.uy)
		return false;

	sx = ESHK(shkp)->shk.x;
	sy = ESHK(shkp)->shk.y;

	/* KMH, balacne patch -- allow other picks */
	if (shkp->mx == sx && shkp->my == sy && shkp->mcanmove && !shkp->msleeping && (x == sx - 1 || x == sx + 1 || y == sy - 1 || y == sy + 1) && (Invis || carrying(PICK_AXE) || carrying(DWARVISH_MATTOCK) || u.usteed)) {
		pline("%s%s blocks your way!", shkname(shkp),
		      Invis ? " senses your motion and" : "");
		return true;
	}
	return false;
}

// "your " or "Foobar's " (note the trailing space)
char *shk_your(char *buf, struct obj *obj) {
	if (!shk_owns(buf, obj) && !mon_owns(buf, obj))
		strcpy(buf, carried(obj) ? "your" : "the");

	return strcat(buf, " ");
}

char *Shk_Your(char *buf, struct obj *obj) {
	shk_your(buf, obj);
	*buf = highc(*buf);
	return buf;
}

static char *shk_owns(char *buf, struct obj *obj) {
	struct monst *shkp;
	xchar x, y;

	if (get_obj_location(obj, &x, &y, 0) &&
	    (obj->unpaid ||
	     (obj->where == OBJ_FLOOR && !obj->no_charge && costly_spot(x, y)))) {
		shkp = shop_keeper(inside_shop(x, y));
		return strcpy(buf, shkp ? s_suffix(shkname(shkp)) : "the");
	}
	return NULL;
}

static char *mon_owns(char *buf, struct obj *obj) {
	if (obj->where == OBJ_MINVENT)
		return strcpy(buf, s_suffix(y_monnam(obj->ocarry)));
	return NULL;
}

static const char identify_types[] = {ALL_CLASSES, 0};
static const char weapon_types[] = {WEAPON_CLASS, TOOL_CLASS, 0};
static const char armor_types[] = {ARMOR_CLASS, 0};

/*
** FUNCTION shk_identify
**
** Pay the shopkeeper to identify an item.
*/
static const char ident_chars[] = "bp";

static void shk_identify(char *slang, struct monst *shkp) {
	struct obj *obj;  /* The object to identify       */
	int charge, mult; /* Cost to identify             */
	/*
		char sbuf[BUFSZ];
	 */
	boolean guesswork;	/* Will shkp be guessing?       */
	boolean ripoff = false; /* Shkp ripping you off?        */
	char ident_type;

	/* Pick object */
	if (!(obj = getobj(identify_types, "have identified"))) return;

	/* Will shk be guessing? */
	if ((guesswork = !shk_obj_match(obj, shkp))) {
		verbalize("I don't handle that sort of item, but I could try...");
	}

	/* Here we go */
	/* KMH -- fixed */
	if (ESHK(shkp)->services & SHK_ID_BASIC &&
	    ESHK(shkp)->services & SHK_ID_PREMIUM) {
		ident_type = yn_function("[B]asic service or [P]remier",
					 ident_chars, '\0');
		if (ident_type == '\0') return;
	} else if (ESHK(shkp)->services & SHK_ID_BASIC) {
		verbalize("I only offer basic identification.");
		ident_type = 'b';
	} else if (ESHK(shkp)->services & SHK_ID_PREMIUM) {
		verbalize("I only make complete identifications.");
		ident_type = 'p';
	}

	/*
	** Shopkeeper is ripping you off if:
	** Basic service and object already known.
	** Premier service, object known, + know blessed/cursed and
	**      rustproof, etc.
	*/
	if (obj->dknown && objects[obj->otyp].oc_name_known) {
		if (ident_type == 'b') ripoff = true;
		if (ident_type == 'p' && obj->bknown && obj->rknown && obj->known) ripoff = true;
	}

	/* Compute the charge */

	if (ripoff) {
		if (no_cheat) {
			verbalize("That item's already identified!");
			return;
		}
		/* Object already identified: Try and cheat the customer. */
		pline("%s chuckles greedily...", mon_nam(shkp));
		mult = 1;

		/* basic */
	} else if (ident_type == 'b')
		mult = 1;

	/* premier */
	else
		mult = 2;

	switch (obj->oclass) {
		case AMULET_CLASS:
			charge = 375 * mult;
			break;
		case WEAPON_CLASS:
			charge = 75 * mult;
			break;
		case ARMOR_CLASS:
			charge = 100 * mult;
			break;
		case FOOD_CLASS:
			charge = 25 * mult;
			break;
		case SCROLL_CLASS:
			charge = 150 * mult;
			break;
		case SPBOOK_CLASS:
			charge = 250 * mult;
			break;
		case POTION_CLASS:
			charge = 150 * mult;
			break;
		case RING_CLASS:
			charge = 300 * mult;
			break;
		case WAND_CLASS:
			charge = 200 * mult;
			break;
		case TOOL_CLASS:
			charge = 50 * mult;
			break;
		case GEM_CLASS:
			charge = 500 * mult;
			break;
		default:
			charge = 75 * mult;
			break;
	}

	/* Artifacts cost more to deal with */
	/* KMH -- Avoid floating-point */
	if (obj->oartifact) charge = charge * 3 / 2;

	/* Smooth out the charge a bit (lower bound only) */
	shk_smooth_charge(&charge, 25, 750);

	/* Go ahead? */
	if (shk_offer_price(slang, charge, shkp) == false) return;

	/* Shopkeeper deviousness */
	if (ident_type == 'b') {
		if (Hallucination) {
			pline("You hear %s tell you it's a pot of flowers.",
			      mon_nam(shkp));
			return;
		} else if (Confusion) {
			pline("%s tells you but you forget.", mon_nam(shkp));
			return;
		}
	}

	/* Is shopkeeper guessing? */
	if (guesswork) {
		/*
		** Identify successful if rn2() < #.
		*/
		if (!rn2(ident_type == 'b' ? 4 : 2)) {
			verbalize("Success!");
			/* Rest of msg will come from identify(); */
		} else {
			verbalize("Sorry.  I guess it's not your lucky day.");
			return;
		}
	}

	/* Premier service */
	if (ident_type == 'p') {
		identify(obj);
	} else {
		/* Basic */
		makeknown(obj->otyp);
		obj->dknown = 1;
		prinv(NULL, obj, 0L); /* Print result */
	}
}

/*
** FUNCTION shk_uncurse
**
** Uncurse an item for the customer
*/
static void shk_uncurse(char *slang, struct monst *shkp) {
	struct obj *obj; /* The object picked            */
	int charge;	 /* How much to uncurse          */

	/* Pick object */
	if (!(obj = getobj(identify_types, "uncurse"))) return;

	/* Charge is same as cost */
	charge = get_cost(obj, shop_keeper(/* roomno= */ *u.ushops));

	/* Artifacts cost more to deal with */
	/* KMH -- Avoid floating-point */
	if (obj->oartifact) charge = charge * 3 / 2;

	/* Smooth out the charge a bit */
	shk_smooth_charge(&charge, 50, 250);

	/* Go ahead? */
	if (shk_offer_price(slang, charge, shkp) == false) return;

	/* Shopkeeper responses */
	/* KMH -- fixed bknown, curse(), bless(), uncurse() */
	if (!obj->bknown && !Role_if(PM_PRIEST) && !Role_if(PM_NECROMANCER) &&
	    !no_cheat) {
		/* Not identified! */
		pline("%s snickers and says \"See, nice and uncursed!\"",
		      mon_nam(shkp));
		obj->bknown = false;
	} else if (Confusion) {
		/* Curse the item! */
		pline("You accidentally ask for the item to be cursed");
		curse(obj);
	} else if (Hallucination) {
		/*
		** Let's have some fun:  If you're hallucinating,
		** then there's a chance for the object to be blessed!
		*/
		if (!rn2(4)) {
			pline("Distracted by your blood-shot %s, the shopkeeper",
			      makeplural(body_part(EYE)));
			pline("accidentally blesses the item!");
			bless(obj);
		} else {
			pline("You can't see straight and point to the wrong item");
		}
	} else {
		verbalize("All done - safe to handle, now!");
		uncurse(obj);
	}
}

/*
** FUNCTION shk_appraisal
**
** Appraise a weapon or armor
*/
static const char basic_damage[] =
	"Basic damage against small foes %s, against large foes %s.";

static void shk_appraisal(char *slang, struct monst *shkp) {
	struct obj *obj;     /* The object picked            */
	int charge;	     /* How much for appraisal       */
	boolean guesswork;   /* Shopkeeper unsure?           */
	char ascii_wsdam[5]; /* Ascii form of damage         */
	char ascii_wldam[5];

	/* Pick object */
	if (!(obj = getobj(weapon_types, "appraise"))) return;

	charge = get_cost(obj, shop_keeper(/* roomno= */ *u.ushops)) / 3;

	/* Smooth out the charge a bit */
	shk_smooth_charge(&charge, 5, 50);

	/* If not identified, complain. */
	/* KMH -- Why should it matter? */
	/*	if ( ! (obj->known && objects[obj->otyp].oc_name_known) )
		{
			verbalize("This weapon needs to be identified first!");
			return;
		} else */
	if (shk_class_match(WEAPON_CLASS, shkp) == SHK_MATCH) {
		verbalize("Ok, %s, let's see what we have here.", slang);
		guesswork = false;
	} else {
		verbalize("Mind you, I'm not an expert in this field.");
		guesswork = true;
	}

	/* Go ahead? */
	if (shk_offer_price(slang, charge, shkp) == false) return;

	/* Shopkeeper deviousness */
	if (Confusion) {
		pline("The numbers get all mixed up in your head.");
		return;
	} else if (Hallucination) {
		pline("You hear %s say it'll \"knock 'em dead\"",
		      mon_nam(shkp));
		return;
	}

	/* Convert damage to ascii */
	sprintf(ascii_wsdam, "%d", objects[obj->otyp].oc_wsdam);
	sprintf(ascii_wldam, "%d", objects[obj->otyp].oc_wldam);

	/* Will shopkeeper be unsure? */
	if (guesswork) {
		switch (rn2(10)) {
			case 1:
				/* Shkp's an idiot */
				verbalize("Sorry, %s, but I'm not certain.", slang);
				break;

			case 2:
				/* Not sure about large foes */
				verbalize(basic_damage, ascii_wsdam, "?");
				break;

			case 3:
				/* Not sure about small foes */
				verbalize(basic_damage, "?", ascii_wldam);
				break;

			default:
				verbalize(basic_damage, ascii_wsdam, ascii_wldam);
				break;
		}
	} else {
		verbalize(basic_damage, ascii_wsdam, ascii_wldam);
	}
}

/*
** FUNCTION shk_weapon_works
**
** Perform ops on weapon for customer
*/
static const char we_offer[] = "We offer the finest service available!";
static void shk_weapon_works(char *slang, struct monst *shkp) {
	struct obj *obj;
	int charge;
	winid tmpwin;
	anything any;
	menu_item *selected;
	int service;
	int n;

	/* Pick weapon */
	if (ESHK(shkp)->services & (SHK_SPECIAL_A | SHK_SPECIAL_B))
		obj = getobj(weapon_types, "improve");
	else
		obj = getobj(weapon_types, "poison");
	if (!obj) return;

	/* Check if you asked for a non weapon tool to be improved */
	if (obj->oclass == TOOL_CLASS && !is_weptool(obj))
		pline("%s grins greedily...", mon_nam(shkp));

	if (ESHK(shkp)->services & (SHK_SPECIAL_A | SHK_SPECIAL_B)) {
		any.a_void = 0; /* zero out all bits */
		tmpwin = create_nhwindow(NHW_MENU);
		start_menu(tmpwin);

		if (ESHK(shkp)->services & SHK_SPECIAL_A) {
			any.a_int = 1;
			add_menu(tmpwin, NO_GLYPH, &any, 'w', 0, ATR_NONE,
				 "Ward against damage", MENU_UNSELECTED);
		}
		if (ESHK(shkp)->services & SHK_SPECIAL_B) {
			any.a_int = 2;
			add_menu(tmpwin, NO_GLYPH, &any, 'e', 0, ATR_NONE,
				 "Enchant", MENU_UNSELECTED);
		}

		/* Can object be poisoned? */
		if (is_poisonable(obj) && (ESHK(shkp)->services & SHK_SPECIAL_C)) {
			any.a_int = 3;
			add_menu(tmpwin, NO_GLYPH, &any, 'p', 0, ATR_NONE,
				 "Poison", MENU_UNSELECTED);
		}

		end_menu(tmpwin, "Weapon-works:");
		n = select_menu(tmpwin, PICK_ONE, &selected);
		destroy_nhwindow(tmpwin);
		if (n > 0)
			service = selected[0].item.a_int;
		else
			service = 0;
	} else
		service = 3;

	/* Here we go */
	if (service > 0)
		verbalize(we_offer);
	else
		pline("Never mind.");

	switch (service) {
		case 0:
			break;

		case 1:
			verbalize("This'll leave your %s untouchable!", xname(obj));

			/* Costs more the more eroded it is (oeroded 0-3 * 2) */
			charge = 500 * (obj->oeroded + obj->oeroded2 + 1);
			if (obj->oeroded + obj->oeroded2 > 2)
				verbalize("This thing's in pretty sad condition, %s", slang);

			/* Another warning if object is naturally rustproof */
			if (obj->oerodeproof || !is_damageable(obj))
				pline("%s gives you a suspciously happy smile...",
				      mon_nam(shkp));

			/* Artifacts cost more to deal with */
			if (obj->oartifact) charge = charge * 3 / 2;

			/* Smooth out the charge a bit */
			shk_smooth_charge(&charge, 200, 1500);

			if (shk_offer_price(slang, charge, shkp) == false) return;

			/* Have some fun, but for this $$$ it better work. */
			if (Confusion)
				pline("You fall over in appreciation");
			else if (Hallucination)
				pline("Your  - tin roof, un-rusted!");

			obj->oeroded = obj->oeroded2 = 0;
			obj->rknown = true;
			obj->oerodeproof = true;
			break;

		case 2:
			verbalize("Guaranteed not to harm your weapon, or your money back!");
			/*
		** The higher the enchantment, the more costly!
		** Gets to the point where you need to rob fort ludios
		** in order to get it to +5!!
		*/
			charge = (obj->spe + 1) * (obj->spe + 1) * 625;

			if (obj->spe < 0) charge = 100;

			/* Artifacts cost more to deal with */
			if (obj->oartifact) charge *= 2;

			/* Smooth out the charge a bit (lower bound only) */
			shk_smooth_charge(&charge, 50, NOBOUND);

			if (shk_offer_price(slang, charge, shkp) == false) return;
			if (obj->spe + 1 > 5) {
				verbalize("I can't enchant this any higher!");
				charge = 0;
				break;
			}
			/* Have some fun! */
			if (Confusion)
				pline("Your %s unexpectedly!", aobjnam(obj, "vibrate"));
			else if (Hallucination)
				pline("Your %s to evaporate into thin air!", aobjnam(obj, "seem"));
			/* ...No actual vibrating and no evaporating */

			if (obj->otyp == WORM_TOOTH) {
				obj->otyp = CRYSKNIFE;
				pline("Your weapon seems sharper now.");
				obj->cursed = 0;
				break;
			}

			obj->spe++;
			break;

		case 3:
			verbalize("Just imagine what poisoned %s can do!", xname(obj));

			charge = 10 * obj->quan;

			if (shk_offer_price(slang, charge, shkp) == false) return;

			obj->opoisoned = true;
			break;

		default:
			impossible("Unknown Weapon Enhancement");
			break;
	}
}

/*
** FUNCTION shk_armor_works
**
** Perform ops on armor for customer
*/
static void shk_armor_works(char *slang, struct monst *shkp) {
	struct obj *obj;
	int charge;
	/*WAC - Windowstuff*/
	winid tmpwin;
	anything any;
	menu_item *selected;
	int n;

	/* Pick armor */
	if (!(obj = getobj(armor_types, "improve"))) return;

	/* Here we go */
	/*WAC - did this using the windowing system...*/
	any.a_void = 0; /* zero out all bits */
	tmpwin = create_nhwindow(NHW_MENU);
	start_menu(tmpwin);
	any.a_int = 1;
	if (ESHK(shkp)->services & (SHK_SPECIAL_A))
		add_menu(tmpwin, NO_GLYPH, &any, 'r', 0, ATR_NONE, "Rust/Fireproof", MENU_UNSELECTED);
	any.a_int = 2;
	if (ESHK(shkp)->services & (SHK_SPECIAL_B))
		add_menu(tmpwin, NO_GLYPH, &any, 'e', 0, ATR_NONE, "Enchant", MENU_UNSELECTED);
	end_menu(tmpwin, "Armor-works:");
	n = select_menu(tmpwin, PICK_ONE, &selected);
	destroy_nhwindow(tmpwin);

	verbalize(we_offer);

	if (n > 0)
		switch (selected[0].item.a_int) {
			case 1:
				if (!flags.female && is_human(youmonst.data))
					verbalize("They'll call you the man of stainless steel!");

				/* Costs more the more rusty it is (oeroded 0-3) */
				charge = 300 * (obj->oeroded + 1);
				if (obj->oeroded > 2) verbalize("Yikes!  This thing's a mess!");

				/* Artifacts cost more to deal with */
				/* KMH -- Avoid floating-point */
				if (obj->oartifact) charge = charge * 3 / 2;

				/* Smooth out the charge a bit */
				shk_smooth_charge(&charge, 100, 1000);

				if (shk_offer_price(slang, charge, shkp) == false) return;

				/* Have some fun, but for this $$$ it better work. */
				if (Confusion)
					pline("You forget how to put your %s back on!", xname(obj));
				else if (Hallucination)
					pline("You mistake your %s for a pot and...", xname(obj));

				obj->oeroded = 0;
				obj->rknown = true;
				obj->oerodeproof = true;
				break;

			case 2:
				verbalize("Nobody will ever hit on you again.");

				/* Higher enchantment levels cost more. */
				charge = (obj->spe + 1) * (obj->spe + 1) * 500;

				if (obj->spe < 0) charge = 100;

				/* Artifacts cost more to deal with */
				if (obj->oartifact) charge *= 2;

				/* Smooth out the charge a bit */
				shk_smooth_charge(&charge, 50, NOBOUND);

				if (shk_offer_price(slang, charge, shkp) == false) return;
				if (obj->spe + 1 > 3) {
					verbalize("I can't enchant this any higher!");
					charge = 0;
					break;
				}
				/* Have some fun! */
				if (Hallucination) pline("Your %s looks dented.", xname(obj));

				if (obj->otyp >= GRAY_DRAGON_SCALES &&
				    obj->otyp <= YELLOW_DRAGON_SCALES) {
					/* dragon scales get turned into dragon scale mail */
					pline("Your %s merges and hardens!", xname(obj));
					setworn(NULL, W_ARM);
					/* assumes same order */
					obj->otyp = GRAY_DRAGON_SCALE_MAIL +
						    obj->otyp - GRAY_DRAGON_SCALES;
					obj->cursed = 0;
					obj->known = 1;
					setworn(obj, W_ARM);
					break;
				}

				obj->spe++;
				adj_abon(obj, 1);
				break;

			default:
				pline("Unknown Armor Enhancement");
				break;
		}
}

/*
** FUNCTION shk_charge
**
** Charge something (for a price!)
*/
static const char wand_types[] = {WAND_CLASS, 0};
static const char tool_types[] = {TOOL_CLASS, 0};
static const char ring_types[] = {RING_CLASS, 0};
static const char spbook_types[] = {SPBOOK_CLASS, 0};

static void shk_charge(char *slang, struct monst *shkp) {
	struct obj *obj = NULL; /* The object picked            */
	struct obj *tobj;	/* Temp obj                     */
	char type;		/* Basic/premier service        */
	int charge;		/* How much to charge customer  */
	char invlet;		/* Inventory letter             */

	/* What type of shop are we? */
	if (shk_class_match(WAND_CLASS, shkp) == SHK_MATCH)
		obj = getobj(wand_types, "charge");
	else if (shk_class_match(TOOL_CLASS, shkp) == SHK_MATCH)
		obj = getobj(tool_types, "charge");
	else if (shk_class_match(RING_CLASS, shkp) == SHK_MATCH)
		obj = getobj(ring_types, "charge");
	else if (shk_class_match(SPBOOK_CLASS, shkp) == SHK_MATCH)
		obj = getobj(spbook_types, "charge");
	if (!obj) return;

	/*
	** Wand shops can offer special service!
	** Extra charges (for a lot of extra money!)
	*/
	if (obj->oclass == WAND_CLASS) {
		/* What type of service? */
		if ((ESHK(shkp)->services & (SHK_SPECIAL_A | SHK_SPECIAL_B)) == (SHK_SPECIAL_A | SHK_SPECIAL_B)) {
			type = yn_function("[B]asic service or [P]remier", ident_chars, '\0');
			if (type == '\0') return;
		} else if (ESHK(shkp)->services & SHK_SPECIAL_A) {
			pline("I only perform basic charging.");
			type = 'b';
		} else if (ESHK(shkp)->services & SHK_SPECIAL_B) {
			pline("I only perform complete charging.");
			type = 'p';
		} else {
			impossible("Shopkeeper cannot perform any services??");
			type = 'b';
		}
	} else {
		type = 'b';
	}

	/* Compute charge */
	if (type == 'b')
		charge = 300;
	else
		charge = 1000;

	/* Wands of wishing should be hard to get recharged */
	if (/*obj->oclass == WAND_CLASS &&*/ obj->otyp == WAN_WISHING)
		charge *= 3;
	else /* Smooth out the charge a bit */
		shk_smooth_charge(&charge, 100, 1000);

	/* Go for it? */
	if (shk_offer_price(slang, charge, shkp) == false) return;

	/* Shopkeeper deviousness */
	if ((Confusion || Hallucination) && !no_cheat) {
		pline("%s says it's charged and pushes you toward the door",
		      Monnam(shkp));
		return;
	}

	/* Do it */
	invlet = obj->invlet;
	recharge(obj, (type == 'b') ? 0 : 1);

	/*
	** Did the object blow up?  We need to check this in a way
	** that has nothing to do with dereferencing the obj pointer.
	** We saved the inventory letter of this item; now cycle
	** through all objects and see if there is an object
	** with that letter.
	*/
	for (obj = 0, tobj = invent; tobj; tobj = tobj->nobj)
		if (tobj->invlet == invlet) {
			obj = tobj;
			break;
		}
	if (!obj) {
		verbalize("Oops!  Sorry about that...");
		return;
	}

	/* Wands get special treatment */
	if (obj->oclass == WAND_CLASS) {
		/* Wand of wishing? */
		if (obj->otyp == WAN_WISHING) {
			/* Premier gives you ONE more charge */
			/* KMH -- Okay, but that's pretty generous */
			if (type == 'p') obj->spe++;

			/* Fun */
			verbalize("Since you'll have everything you always wanted,");
			verbalize("...How about loaning me some money?");
			money2mon(shkp, money_cnt(invent));
			makeknown(obj->otyp);
			bot();
		} else {
			/*
			** Basic: recharge() will have given 1 charge.
			** Premier: recharge() will have given 5-10, say.
			** Add a few more still.
			*/
			if (obj->spe < 16)
				obj->spe += rn1(5, 5);
			else if (obj->spe < 20)
				obj->spe += 1;
		}
	}
}

/*
** FUNCTION shk_obj_match
**
** Does object "obj" match the type of shop?
*/
static boolean shk_obj_match(struct obj *obj, struct monst *shkp) {
	/* object matches type of shop? */
	return saleable(shkp, obj);
}

/*
** FUNCTION shk_offer_price
**
** Tell customer how much it'll cost, ask if he wants to pay,
** and deduct from $$ if agreable.
*/
static boolean shk_offer_price(char *slang, long charge, struct monst *shkp) {
	char sbuf[BUFSZ];
	long credit = ESHK(shkp)->credit;

	/* Ask y/n if player wants to pay */
	sprintf(sbuf, "It'll cost you %ld zorkmid%s.  Interested?",
		charge, plur(charge));

	if (yn(sbuf) != 'y') {
		verbalize("It's your call, %s.", slang);
		return false;
	}

	/* Player _wants_ to pay, but can he? */
	/* WAC -- Check the credit:  but don't use check_credit
	 * since we don't want to charge him for part of it if he can't pay for all
	 * of it
	 */
	if (charge > (money_cnt(invent) + credit)) {
		verbalize("Cash on the spot, %s, and you ain't got the dough!",
			  slang);
		return false;
	}

	/* Charge the customer */
	charge = check_credit(charge, shkp); /* Deduct the credit first */

	money2mon(shkp, charge);
	bot();

	return true;
}

/*
** FUNCTION shk_smooth_charge
**
** Smooth out the lower/upper bounds on the price to get something
** done.  Make sure that it (1) varies depending on charisma and
** (2) is constant.
*/
static void shk_smooth_charge(int *pcharge, int lower, int upper) {
	int charisma;
	int bonus;

	/* KMH -- Avoid using floating-point arithmetic */
	if (ACURR(A_CHA) > 21)
		*pcharge *= 11;
	else if (ACURR(A_CHA) > 18)
		*pcharge *= 12;
	else if (ACURR(A_CHA) > 15)
		*pcharge *= 13;
	else if (ACURR(A_CHA) > 12)
		*pcharge *= 14;
	else if (ACURR(A_CHA) > 10)
		*pcharge *= 15;
	else if (ACURR(A_CHA) > 8)
		*pcharge *= 16;
	else if (ACURR(A_CHA) > 7)
		*pcharge *= 17;
	else if (ACURR(A_CHA) > 6)
		*pcharge *= 18;
	else if (ACURR(A_CHA) > 5)
		*pcharge *= 19;
	else if (ACURR(A_CHA) > 4)
		*pcharge *= 20;
	else
		*pcharge *= 21;
	*pcharge /= 10;

	if (Is_blackmarket(&u.uz)) *pcharge *= 3;

	/* Skip upper stuff? */
	if (upper == NOBOUND) goto check_lower;

	/* This should give us something like a charisma of 5 to 25. */
	charisma = ABASE(A_CHA) + ABON(A_CHA) + ATEMP(A_CHA);

	/* Now: 0 to 10 = 0.  11 and up = 1 to whatever. */
	if (charisma <= 10)
		charisma = 0;
	else
		charisma -= 10;

	/* Charismatic players get smaller upper bounds */
	bonus = ((upper / 50) * charisma);

	/* Adjust upper.  Upper > lower! */
	upper -= bonus;
	upper = (upper >= lower) ? upper : lower;

	/* Ok, do the min/max stuff */
	if (*pcharge > upper) *pcharge = upper;
check_lower:
	if (*pcharge < lower) *pcharge = lower;
}

#ifdef DEBUG
// in this case, display your bill(s)
int wiz_debug_cmd(void) {
	int win, special = 0;
	struct obj *obj;
	struct monst *shkp, *ushkp;
	struct bill_x *bp;
	int ct;
	char buf[BUFSIZ];
	char buf2[BUFSIZ];

	win = create_nhwindow(NHW_MENU);
	ushkp = shop_keeper(*u.ushops);
	shkp = next_shkp(fmon, true);
	if (!shkp) {
		shkp = ushkp;
		special++;
	}
	if (!shkp)
		putstr(win, 0, "No shopkeepers with bills");
	else
		for (; shkp;) {
			bp = ESHK(shkp)->bill_p;
			ct = ESHK(shkp)->billct;
			if (ct) {
				sprintf(buf, "Your bill with %s", noit_mon_nam(shkp));
				if (shkp == ushkp) {
					strcat(buf, " (here)");
					ushkp = NULL;
				}
				strcat(buf, ":");
				putstr(win, 0, buf);
				putstr(win, 0, "Price   Quan    Used?   Object");
				while (--ct >= 0) {
					obj = bp_to_obj(bp);
					if (obj) {
						if (!obj->unpaid)
							*buf2 = '*'; /* Bad entry */
						strcpy(obj->unpaid ? buf2 : buf2 + 1, xname(obj));
					} else
						sprintf(buf2, "Unknown, with ID %d", bp->bo_id);
					sprintf(buf, "%-7d %-7d %-7s %s", bp->price, bp->bquan,
						bp->useup ? "Yes" : "No", buf2);
					putstr(win, 0, buf);
					bp++;
				}
			} else {
				sprintf(buf, "You do not owe %s anything.", noit_mon_nam(shkp));
				putstr(win, 0, buf);
			}
			if (special)
				break;
			shkp = next_shkp(shkp->nmon, true);
			if (!shkp) {
				shkp = ushkp;
				special++;
			}
			if (shkp)
				putstr(win, 0, "");
		}
	display_nhwindow(win, false);
	destroy_nhwindow(win);
	return 0;
}
#endif /* DEBUG */

/*shk.c*/
