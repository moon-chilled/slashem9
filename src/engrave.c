/*	SCCS Id: @(#)engrave.c	3.4	2001/11/04	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "lev.h"
#include <ctype.h>

static struct engr *head_engr;

/* random engravings */
static const char *const random_mesg[] = {
	"Elbereth",
	/* trap engravings */
	"Vlad was here", "ad aerarium",
	/* take-offs and other famous engravings */
	"Owlbreath", "Galadriel",
	"Kilroy was here",
	"Frodo lives",
	"A.S. ->", "<- A.S.",			    /* Journey to the Center of the Earth */
	"You won't get it up the steps",	    /* Adventure */
	"Lasciate ogni speranza o voi ch'entrate.", /* Inferno */
	"Well Come",				    /* Prisoner */
	"We apologize for the inconvenience.",	    /* So Long... */
	"See you next Wednesday",		    /* Thriller */
	"notary sojak",				    /* Smokey Stover */
	"For a good time call 8?7-5309",
	"Please don't feed the animals.", /* Various zoos around the world */
	"Madam, in Eden, I'm Adam.",	  /* A palindrome */
	"Two thumbs up!",		  /* Siskel & Ebert */
	"Hello, World!",		  /* The First C Program */
#ifdef MAIL
	"You've got mail!", /* AOL */
#endif
	"As if!", /* Clueless */
	/* [Tom] added these */
	"Y?v?l s??ks!", /* just kidding... */
	"T?m ?as h?r?",
	/* Tsanth added these */
	"Gazortenplatz",			   /* Tribute to David Fizz */
	"John 3:16",				   /* You see this everywhere; why not here? */
	"Exhale! Exhale! Exhale!",		   /* Prodigy */
	"All you need is love.",		   /* The Beatles */
	"Please don't feed the animals.",	   /* Various zoos around the world */
	"....TCELES B HSUP   A magic spell?",	   /* Final Fantasy I (US) */
	"Madam, in Eden, I'm Adam.",		   /* A palindrome */
	"Two thumbs up!",			   /* Siskel & Ebert */
	"Hello, World!",			   /* The First Program line */
	"Turn around.",				   /* Various people at various times in history */
	"You've got mail!",			   /* AOL */
	"UY WUZ HERE",				   /* :] */
	"Time flies when you're having fun.",	   /* Who said this first, anyway? */
	"As if!",				   /* Clueless */
	"How bizarre, how bizarre.",		   /* OMC */
	"Silly rabbit, Trix are for kids!",	   /* Trix */
	"I'll be back!",			   /* Terminator */
	"That is not dead which can eternal lie.", /* HPL */
	"The cake is a lie",			   /* Portal */
};

char *random_engraving(char *outbuf) {
	const char *rumor;

	/* a random engraving may come from the "rumors" file,
	   or from the list above */
	if (!rn2(4) || !(rumor = getrumor(0, outbuf, true)) || !*rumor)
		strcpy(outbuf, random_mesg[rn2(SIZE(random_mesg))]);

	wipeout_text(outbuf, strlen(outbuf) / 4, 0);
	return outbuf;
}

/* Partial rubouts for engraving characters. -3. */
static const struct {
	char wipefrom;
	const char *wipeto;
} rubouts[] = {
	{'A', "^"},	{'B', "Pb["},	{'C', "("},	{'D', "|)["},
	{'E', "|FL[_"},	{'F', "|-"},	{'G', "C("},	{'H', "|-"},
	{'I', "|"},	{'K', "|<"},	{'L', "|_"},	{'M', "|"},
	{'N', "|\\"},	{'O', "C("},	{'P', "F"},	{'Q', "C("},
	{'R', "PF"},	{'T', "|"},	{'U', "J"},	{'V', "/\\"},
	{'W', "V/\\"},	{'Z', "/"},	{'b', "|"},	{'d', "c|"},
	{'e', "c"},	{'g', "c"},	{'h', "n"},	{'j', "i"},
	{'k', "|"},	{'l', "|"},	{'m', "nr"},	{'n', "r"},
	{'o', "c"},	{'q', "c"},	{'w', "v"},	{'y', "v"},
	{':', "."},	{';', ",:"},	{',', "."},	{'=', "-"},
	{'+', "|-"},	{'*', "+"},	{'@', "0a"},	{'0', "C("},
	{'1', "|"},	{'6', "o"},	{'7', "/"},	{'8', "3o"}};

void wipeout_text(char *engr, int cnt, unsigned seed /* for semi-controlled randomization */) {
	char *s;
	int i, j, nxt, use_rubout, lth = (int)strlen(engr);

	if (lth && cnt > 0) {
		while (cnt--) {
			/* pick next character */
			if (!seed) {
				/* random */
				nxt = rn2(lth);
				use_rubout = rn2(4);
			} else {
				/* predictable; caller can reproduce the same sequence by
				   supplying the same arguments later, or a pseudo-random
				   sequence by varying any of them */
				nxt = seed % lth;
				seed *= 31, seed %= (BUFSZ - 1);
				use_rubout = seed & 3;
			}
			s = &engr[nxt];
			if (*s == ' ') continue;

			/* rub out unreadable & small punctuation marks */
			if (index("?.,'`-|_", *s)) {
				*s = ' ';
				continue;
			}

			if (!use_rubout)
				i = SIZE(rubouts);
			else
				for (i = 0; i < SIZE(rubouts); i++)
					if (*s == rubouts[i].wipefrom) {
						/*
						 * Pick one of the substitutes at random.
						 */
						if (!seed)
							j = rn2(strlen(rubouts[i].wipeto));
						else {
							seed *= 31, seed %= (BUFSZ - 1);
							j = seed % (strlen(rubouts[i].wipeto));
						}
						*s = rubouts[i].wipeto[j];
						break;
					}

			/* didn't pick rubout; use '?' for unreadable character */
			if (i == SIZE(rubouts)) *s = '?';
		}
	}

	/* trim trailing spaces */
	while (lth && engr[lth - 1] == ' ')
		engr[--lth] = 0;
}

boolean can_reach_floor(void) {
	return !u.uswallow &&
	       /* Restricted/unskilled riders can't reach the floor */
	       !(u.usteed && P_SKILL(P_RIDING) < P_BASIC) &&
	       (!Levitation || Is_airlevel(&u.uz) || Is_waterlevel(&u.uz)) &&
	       (!u.uundetected || !is_hider(youmonst.data) || u.umonnum == PM_TRAPPER);
}

const char *surface(int x, int y) {
	struct rm *lev = &levl[x][y];

	if ((x == u.ux) && (y == u.uy) && u.uswallow &&
	    is_animal(u.ustuck->data))
		return "maw";
	else if (IS_AIR(lev->typ) && Is_airlevel(&u.uz))
		return "air";
	else if (is_pool(x, y))
		return (Underwater && !Is_waterlevel(&u.uz)) ? "bottom" : "water";
	else if (is_ice(x, y))
		return "ice";
	else if (is_lava(x, y))
		return "lava";
	else if (lev->typ == DRAWBRIDGE_DOWN)
		return "bridge";
	else if (IS_ALTAR(levl[x][y].typ))
		return "altar";
	else if (IS_GRAVE(levl[x][y].typ))
		return "headstone";
	else if (IS_FOUNTAIN(levl[x][y].typ))
		return "fountain";
	else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz)) ||
		 IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
		return "floor";
	else
		return "ground";
}

const char *ceiling(int x, int y) {
	struct rm *lev = &levl[x][y];
	const char *what;

	/* other room types will no longer exist when we're interested --
	 * see check_special_room()
	 */
	if (*in_rooms(x, y, VAULT))
		what = "vault's ceiling";
	else if (*in_rooms(x, y, TEMPLE))
		what = "temple's ceiling";
	else if (*in_rooms(x, y, SHOPBASE))
		what = "shop's ceiling";
	else if (IS_AIR(lev->typ))
		what = "sky";
	else if (Underwater)
		what = "water's surface";
	else if ((IS_ROOM(lev->typ) && !Is_earthlevel(&u.uz)) ||
		 IS_WALL(lev->typ) || IS_DOOR(lev->typ) || lev->typ == SDOOR)
		what = "ceiling";
	else
		what = "rock above";

	return what;
}

struct engr *engr_at(xchar x, xchar y) {
	struct engr *ep = head_engr;

	while (ep) {
		if (x == ep->engr_x && y == ep->engr_y)
			return ep;
		ep = ep->nxt_engr;
	}
	return NULL;
}

/* Decide whether a particular string is engraved at a specified
 * location; a case-insensitive substring match used.
 * Ignore headstones, in case the player names herself "Elbereth".
 */
int sengr_at(const char *s, xchar x, xchar y) {
	struct engr *ep = engr_at(x, y);

	return (ep && ep->engr_type != HEADSTONE &&
		ep->engr_time <= moves && strstri(ep->engr_txt, s) != 0);
}

void u_wipe_engr(int cnt) {
	if (can_reach_floor())
		wipe_engr_at(u.ux, u.uy, cnt);
}

void wipe_engr_at(xchar x, xchar y, xchar cnt) {
	struct engr *ep = engr_at(x, y);

	/* Headstones are indelible */
	if (ep && ep->engr_type != HEADSTONE) {
		if (ep->engr_type != BURN || is_ice(x, y)) {
			if (ep->engr_type != DUST && ep->engr_type != ENGR_BLOOD) {
				cnt = rn2(1 + 50 / (cnt + 1)) ? 0 : 1;
			}
			wipeout_text(ep->engr_txt, (int)cnt, 0);
			while (ep->engr_txt[0] == ' ')
				ep->engr_txt++;
			if (!ep->engr_txt[0]) del_engr(ep);
		}
	}
}

boolean sense_engr_at(int x, int y, boolean read_it /* Read any sensed engraving */) {
	struct engr *ep = engr_at(x, y);
	int sensed = 0;
	char buf[BUFSZ];

	/* Sensing an engraving does not require sight,
	 * nor does it necessarily imply comprehension (literacy).
	 */
	if (ep && ep->engr_txt[0]) {
		switch (ep->engr_type) {
			case DUST:
				if (!Blind) {
					sensed = 1;
					pline("Something is written %s in the %s.",
					      Levitation ? "below you" : "here",
					      is_ice(x, y) ? "frost" : "dust");
				}
				break;
			case ENGRAVE:
			case HEADSTONE:
				if (!Blind || can_reach_floor()) {
					sensed = 1;
					pline("Something is engraved %s on the %s.",
					      Levitation ? "below you" : "here", surface(x, y));
				}
				break;
			case BURN:
				if (!Blind || can_reach_floor()) {
					sensed = 1;
					pline("Some text has been %s into the %s %s.",
					      is_ice(x, y) ? "melted" : "burned",
					      surface(x, y), Levitation ? "below you" : "here");
				}
				break;
			case MARK:
				if (!Blind) {
					sensed = 1;
					pline("There's some graffiti on the %s %s.",
					      surface(x, y), Levitation ? "below you" : "here");
				}
				break;
			case ENGR_BLOOD:
				/* "It's a message!  Scrawled in blood!"
				 * "What's it say?"
				 * "It says... `See you next Wednesday.'" -- Thriller
				 */
				if (!Blind) {
					sensed = 1;
					pline("You see a message scrawled in blood %s.",
					      Levitation ? "below you" : "here");
				}
				break;
			default:
				impossible("Something is written in a very strange way.");
				sensed = 1;
		}
		if (sensed && !read_it && iflags.cmdassist) {
			pline("Use \"r.\" to read it.");
		} else if (sensed && read_it) {
			char *et;
			unsigned len, maxelen = BUFSZ - sizeof("You feel the words: \"\". ");
			len = strlen(ep->engr_txt);
			if (len > maxelen) {
				strncpy(buf, ep->engr_txt, (int)maxelen);
				buf[maxelen] = '\0';
				et = buf;
			} else
				et = ep->engr_txt;

			/* If you can engrave an 'x', you can "read" it --ALI */
			if (len != 1 || (!index(et, 'x') && !index(et, 'X')))
				u.uconduct.literate++;

			pline("You %s: \"%s\".",
			      Blind ? "feel the words" : "read", et);
			if (context.run > 1) nomul(0);
			return true;
		}
	}
	return false;
}

void make_engr_at(int x, int y, const char *s, long e_time, xchar e_type) {
	struct engr *ep;

	if ((ep = engr_at(x, y)) != 0)
		del_engr(ep);
	ep = newengr(strlen(s) + 1);
	ep->nxt_engr = head_engr;
	head_engr = ep;
	ep->engr_x = x;
	ep->engr_y = y;
	ep->engr_txt = (char *)(ep + 1);
	strcpy(ep->engr_txt, s);
	/* engraving Elbereth shows wisdom */
	if (!in_mklev && !strcmp(s, "Elbereth")) exercise(A_WIS, true);
	ep->engr_time = e_time;
	ep->engr_type = e_type > 0 ? e_type : rnd(N_ENGRAVE - 1);
	ep->engr_lth = strlen(s) + 1;
}

/* delete any engraving at location <x,y> */
void del_engr_at(int x, int y) {
	struct engr *ep = engr_at(x, y);

	if (ep)
		del_engr(ep);
}

/*
 *	freehand - returns true if player has a free hand
 */
boolean freehand(void) {
	return (!uwep || !welded(uwep) ||
		(!bimanual(uwep) && (!uarms || !uarms->cursed)));
	/*	if ((uwep && bimanual(uwep)) ||
		    (uwep && uarms))
			return 0;
		else
			return 1;*/
}

static const char styluses[] = {
	ALL_CLASSES, ALLOW_NONE, TOOL_CLASS, WEAPON_CLASS, WAND_CLASS,
	GEM_CLASS, RING_CLASS, 0};

/* Mohs' Hardness Scale:
 *  1 - Talc		 6 - Orthoclase
 *  2 - Gypsum		 7 - Quartz
 *  3 - Calcite		 8 - Topaz
 *  4 - Fluorite	 9 - Corundum
 *  5 - Apatite		10 - Diamond
 *
 * Since granite is a igneous rock hardness ~ 7, anything >= 8 should
 * probably be able to scratch the rock.
 * Devaluation of less hard gems is not easily possible because obj struct
 * does not contain individual oc_cost currently. 7/91
 *
 * steel     -	5-8.5	(usu. weapon)
 * diamond    - 10			* jade	     -	5-6	 (nephrite)
 * ruby       -  9	(corundum)	* turquoise  -	5-6
 * sapphire   -  9	(corundum)	* opal	     -	5-6
 * topaz      -  8			* glass      - ~5.5
 * emerald    -  7.5-8	(beryl)		* dilithium  -	4-5??
 * aquamarine -  7.5-8	(beryl)		* iron	     -	4-5
 * garnet     -  7.25	(var. 6.5-8)	* fluorite   -	4
 * agate      -  7	(quartz)	* brass      -	3-4
 * amethyst   -  7	(quartz)	* gold	     -	2.5-3
 * jasper     -  7	(quartz)	* silver     -	2.5-3
 * onyx       -  7	(quartz)	* copper     -	2.5-3
 * moonstone  -  6	(orthoclase)	* amber      -	2-2.5
 */

/* return 1 if action took 1 (or more) moves, 0 if error or aborted */
int doengrave(void) {
	bool dengr = false;	// true if we wipe out the current engraving
	bool doblind = false;	// true if engraving blinds the player
	bool doknown = false;	// true if we identify the stylus
	bool postknown = false;	// true if we identify the stylus, but not until after getting the engrave message
	bool eow = false;	// true if we are overwriting oep
	bool jello = false;	// true if we are engraving in slime
	bool ptext = true;	// true if we must prompt for engrave text
	bool teleengr = false;	// true if we move the old engraving
	bool zapwand = false;	// true if we remove a wand charge
	xchar type = DUST;	// Type of engraving made

	//TODO: dynamic buffers
	char buf[BUFSZ] = {0};	// Buffer for final/poly engraving text
	char ebuf[BUFSZ-1];	// Buffer for initial engraving text
	char fbuf[BUFSZ];	// Buffer for 'your fingers'
	char qbuf[QBUFSZ];	// Buffer for query text
	char post_engr_text[BUFSZ];// Text displayed after engraving prompt
	const char *everb;	// Present tense of engraving type
	const char *eloc;	// Where the engraving is (ie dust/floor/...)
	char *sp;		// Place holder for space count of engr text
	int len;		// # of nonspace chars of new engraving text
	int maxelen;		// Max allowable length of engraving text
	struct engr *oep = engr_at(u.ux, u.uy);
	/* The current engraving */
	struct obj *otmp; /* Object selected with which to engrave */
	char *writer;

	multi = 0;	  /* moves consumed */
	nomovemsg = NULL; /* occupation end message */

	buf[0] = 0;
	ebuf[0] = 0;
	post_engr_text[0] = 0;
	maxelen = BUFSZ - 1;
	if (is_demon(youmonst.data) || youmonst.data->mlet == S_VAMPIRE)
		type = ENGR_BLOOD;

	/* Can the adventurer engrave at all? */

	if (u.uswallow) {
		if (is_animal(u.ustuck->data)) {
			pline("What would you write?  \"Jonah was here\"?");
			return false;
		} else if (is_whirly(u.ustuck->data)) {
			pline("You can't reach the %s.", surface(u.ux, u.uy));
			return false;
		} else
			jello = true;
	} else if (is_lava(u.ux, u.uy)) {
		pline("You can't write on the lava!");
		return false;
	} else if (is_pool(u.ux, u.uy) || IS_FOUNTAIN(levl[u.ux][u.uy].typ)) {
		pline("You can't write on the water!");
		return false;
	}
	if (Is_airlevel(&u.uz) || Is_waterlevel(&u.uz) /* in bubble */) {
		pline("You can't write in thin air!");
		return false;
	}
	if (cantwield(youmonst.data)) {
		pline("You can't even hold anything!");
		return false;
	}
	if (check_capacity(NULL)) return false;

	/* One may write with finger, or weapon, or wand, or..., or...
	 * Edited by GAN 10/20/86 so as not to change weapon wielded.
	 */

	otmp = getobj(styluses, "write with");
	if (!otmp) return false; /* otmp == zeroobj if fingers */

	if (otmp == &zeroobj) {
		strcat(strcpy(fbuf, "your "), makeplural(body_part(FINGER)));
		writer = fbuf;
	} else {
		writer = yname(otmp);
	}

	/* There's no reason you should be able to write with a wand
	 * while both your hands are tied up.
	 */
	if (!freehand() && otmp != uwep && !otmp->owornmask) {
		pline("You have no free %s to write with!", body_part(HAND));
		return false;
	}

	if (jello) {
		pline("You tickle %s with %s.", mon_nam(u.ustuck), writer);
		pline("Your message dissolves...");
		return false;
	}
	if (otmp->oclass != WAND_CLASS && !can_reach_floor()) {
		pline("You can't reach the %s!", surface(u.ux, u.uy));
		return false;
	}
	if (IS_ALTAR(levl[u.ux][u.uy].typ)) {
		pline("You make a motion towards the altar with %s.", writer);
		altar_wrath(u.ux, u.uy);
		return false;
	}
	if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
		if (otmp == &zeroobj) { /* using only finger */
			pline("You would only make a small smudge on the %s.",
			      surface(u.ux, u.uy));
			return false;
		} else if (!levl[u.ux][u.uy].disturbed) {
			pline("You disturb the undead!");
			levl[u.ux][u.uy].disturbed = 1;
			makemon(&mons[PM_GHOUL], u.ux, u.uy, NO_MM_FLAGS);
			exercise(A_WIS, false);
			return true;
		}
	}

	/* SPFX for items */

	switch (otmp->oclass) {
		default:
		case AMULET_CLASS:
		case CHAIN_CLASS:
		case POTION_CLASS:
		case COIN_CLASS:
			break;

		case RING_CLASS:
		/* "diamond" rings and others should work */
		case GEM_CLASS:
			/* diamonds & other hard gems should work */
			if (objects[otmp->otyp].oc_tough) {
				type = ENGRAVE;
				break;
			}
			break;

		case ARMOR_CLASS:
			if (is_boots(otmp)) {
				type = DUST;
				break;
			}
		fallthru;
		/* Objects too large to engrave with */
		case BALL_CLASS:
		case ROCK_CLASS:
			pline("You can't engrave with such a large object!");
			ptext = false;
			break;

		/* Objects too silly to engrave with */
		case FOOD_CLASS:
		case SCROLL_CLASS:
		case SPBOOK_CLASS:
			pline("%s would get %s.", Yname2(otmp),
			      is_ice(u.ux, u.uy) ? "all frosty" : "too dirty");
			ptext = false;
			break;

		case RANDOM_CLASS: /* This should mean fingers */
			break;

		/* The charge is removed from the wand before prompting for
	 * the engraving text, because all kinds of setup decisions
	 * and pre-engraving messages are based upon knowing what type
	 * of engraving the wand is going to do.  Also, the player
	 * will have potentially seen "You wrest .." message, and
	 * therefore will know they are using a charge.
	 */
		case WAND_CLASS:
			if (zappable(otmp)) {
				check_unpaid(otmp);
				zapwand = true;
				if (Levitation) ptext = false;

				switch (otmp->otyp) {
					/* DUST wands */
					default:
						break;

					/* NODIR wands */
					case WAN_LIGHT:
					case WAN_SECRET_DOOR_DETECTION:
					case WAN_CREATE_MONSTER:
					case WAN_CREATE_HORDE:
					case WAN_ENLIGHTENMENT:
					case WAN_WISHING:
						zapnodir(otmp);
						break;

					/* IMMEDIATE wands */
					/* If wand is "IMMEDIATE", remember to affect the
					 * previous engraving even if turning to dust.
					 */
					case WAN_STRIKING:
						strcpy(post_engr_text, "The wand unsuccessfully fights your attempt to write!");
						postknown = true;
						break;
					case WAN_SLOW_MONSTER:
						if (!Blind) {
							sprintf(post_engr_text, "The bugs on the %s slow down!", surface(u.ux, u.uy));
							postknown = true;
						}
						break;
					case WAN_SPEED_MONSTER:
						if (!Blind) {
							sprintf(post_engr_text, "The bugs on the %s speed up!", surface(u.ux, u.uy));
							postknown = true;
						}
						break;
					case WAN_HEALING:
					case WAN_EXTRA_HEALING:
						if (!Blind) {
							sprintf(post_engr_text, "The bugs on the %s look healthier!", surface(u.ux, u.uy));
						}
						break;
					case WAN_FEAR:
						if (!Blind) {
							sprintf(post_engr_text, "The bugs on the %s run away!", surface(u.ux, u.uy));
							postknown = true;
						}
						break;
					case WAN_POLYMORPH:
						if (oep) {
							if (!Blind) {
								type = 0; /* random */
								random_engraving(buf);
								postknown = true;
							}
							dengr = true;
						}
						break;
					case WAN_DRAINING: /* KMH */
						if (oep) {
							/*
							 * [ALI] Wand of draining give messages like
							 * either polymorph or cancellation/make
							 * invisible depending on whether the
							 * old engraving is completely wiped or not.
							 * Note: Blindness has slightly different
							 * effect than with wand of polymorph.
							 */
							u_wipe_engr(5);
							oep = engr_at(u.ux, u.uy);
							if (!Blind) {
								if (!oep) {
									pline("The engraving on the %s vanishes!", surface(u.ux, u.uy));
								} else {
									strcpy(buf, oep->engr_txt);
									dengr = true;
								}
							}
						}
						break;
					case WAN_NOTHING:
					case WAN_UNDEAD_TURNING:
					case WAN_OPENING:
					case WAN_LOCKING:
					case WAN_PROBING:
						break;

					/* RAY wands */
					case WAN_MAGIC_MISSILE:
						ptext = true;
						if (!Blind) {
							sprintf(post_engr_text, "The %s is riddled by bullet holes!", surface(u.ux, u.uy));
							postknown = true;
						}
						break;

					/* can't tell sleep from death - Eric Backus */
					case WAN_SLEEP:
					case WAN_DEATH:
						if (!Blind) {
							sprintf(post_engr_text,
								"The bugs on the %s stop moving!",
								surface(u.ux, u.uy));
						}
						break;

					case WAN_COLD:
						if (!Blind) {
							strcpy(post_engr_text, "A few ice cubes drop from the wand.");
							postknown = true;
						}
						if (!oep || (oep->engr_type != BURN)) break;
						else fallthru;
					case WAN_CANCELLATION:
					case WAN_MAKE_INVISIBLE:
						if (oep && oep->engr_type != HEADSTONE) {
							if (!Blind)
								pline("The engraving on the %s vanishes!", surface(u.ux, u.uy));
							dengr = true;
						}
						break;
					case WAN_TELEPORTATION:
						if (oep && oep->engr_type != HEADSTONE) {
							if (!Blind)
								pline("The engraving on the %s vanishes!", surface(u.ux, u.uy));
							teleengr = true;
						}
						break;

					/* type = ENGRAVE wands */
					case WAN_DIGGING:
						ptext = true;
						type = ENGRAVE;
						if (!objects[otmp->otyp].oc_name_known) {
							if (flags.verbose)
								pline("This %s is a wand of digging!",
								      xname(otmp));
							doknown = true;
						}
						if (!Blind)
							strcpy(post_engr_text,
							       IS_GRAVE(levl[u.ux][u.uy].typ) ? "Chips fly out from the headstone." :
							       is_ice(u.ux, u.uy) ? "Ice chips fly up from the ice surface!" :
							       (level.locations[u.ux][u.uy].typ == DRAWBRIDGE_DOWN) ? "Splinters fly up from the bridge." :
							       "Gravel flies up from the floor.");
						else
							strcpy(post_engr_text, "You hear drilling!");
						break;

					/* type = BURN wands */
					case WAN_FIRE:
						ptext = true;
						type = BURN;
						if (!objects[otmp->otyp].oc_name_known) {
							if (flags.verbose)
								pline("This %s is a wand of fire!", xname(otmp));
							doknown = true;
						}
						strcpy(post_engr_text,
						       Blind ? "You feel the wand heat up." :
							       "Flames fly from the wand.");
						break;
					case WAN_FIREBALL:
						ptext = true;
						type = BURN;
						if (!objects[otmp->otyp].oc_name_known) {
							if (flags.verbose)
								pline("This %s is a wand of fireballs!", xname(otmp));
							doknown = true;
						}
						strcpy(post_engr_text,
						       Blind ? "You feel the wand heat up." :
							       "Flames fly from the wand.");
						break;
					case WAN_LIGHTNING:
						ptext = true;
						type = BURN;
						if (!objects[otmp->otyp].oc_name_known) {
							if (flags.verbose)
								pline("This %s is a wand of lightning!",
								      xname(otmp));
							doknown = true;
						}
						if (!Blind) {
							strcpy(post_engr_text,
							       "Lightning arcs from the wand.");
							doblind = true;
						} else
							strcpy(post_engr_text, "You hear crackling!");
						break;

						/* type = MARK wands */
						/* type = ENGR_BLOOD wands */
				}
			} else /* end if zappable */
				if (!can_reach_floor()) {
				pline("You can't reach the %s!", surface(u.ux, u.uy));
				return false;
			}
			break;

		case WEAPON_CLASS:
			if (is_blade(otmp)) {
				if ((int)otmp->spe > -3)
					type = ENGRAVE;
				else
					pline("%s too dull for engraving.", Yobjnam2(otmp, "are"));
			}
			break;

		case TOOL_CLASS:
			if (otmp == ublindf) {
				pline("That is a bit difficult to engrave with, don't you think?");
				return false;
			}

			if (is_lightsaber(otmp)) {
				if (otmp->lamplit)
					type = BURN;
				else
					pline("Your %s is deactivated!", aobjnam(otmp, "are"));
			} else {
				switch (otmp->otyp) {
					case MAGIC_MARKER:
						if (otmp->spe <= 0)
							pline("Your marker has dried out.");
						else
							type = MARK;
						break;
					case TOWEL:
						/* Can't really engrave with a towel */
						ptext = false;
						if (oep)
							if ((oep->engr_type == DUST) ||
							    (oep->engr_type == ENGR_BLOOD) ||
							    (oep->engr_type == MARK)) {
								if (!Blind)
									pline("You wipe out the message here.");
								else
									pline("%s %s.", Yobjnam2(otmp, "get"),
									      is_ice(u.ux, u.uy) ?  "frosty" : "dusty");
								dengr = true;
							} else {
								pline("%s can't wipe out this engraving.",
								      Yname2(otmp));
							}
						else
							pline("%s %s.", Yobjnam2(otmp, "get"),
							      is_ice(u.ux, u.uy) ? "frosty" : "dusty");
						break;
					default:
						break;
				}
			}
			break;

		case VENOM_CLASS:
			if (wizard) {
				pline("Writing a poison pen letter??");
				break;
			}
		fallthru;
		case ILLOBJ_CLASS:
			impossible("You're engraving with an illegal object!");
			break;
	}

	if (IS_GRAVE(levl[u.ux][u.uy].typ)) {
		if (type == ENGRAVE || type == 0)
			type = HEADSTONE;
		else {
			/* ensures the "cannot wipe out" case */
			type = DUST;
			dengr = false;
			teleengr = false;
			buf[0] = 0;
		}
	}

	/* End of implement setup */

	/* Identify stylus */
	if (doknown) {
		makeknown(otmp->otyp);
		more_experienced(0, 10);
	}

	if (teleengr) {
		rloc_engr(oep);
		oep = NULL;
	}

	if (dengr) {
		del_engr(oep);
		oep = NULL;
	}

	/* Something has changed the engraving here */
	if (*buf) {
		make_engr_at(u.ux, u.uy, buf, moves, type);
		pline("The engraving looks different now.");
		ptext = false;
	}

	if (zapwand && (otmp->spe < 0)) {
		pline("%s %sturns to dust.",
		      The(xname(otmp)), Blind ? "" : "glows violently, then ");
		if (!IS_GRAVE(levl[u.ux][u.uy].typ))
			pline("You are not going to get anywhere trying to write in the %s with your dust.",
			      is_ice(u.ux, u.uy) ? "frost" : "dust");
		useup(otmp);
		otmp = NULL; /* wand is now gone */
		ptext = false;
	}

	if (!ptext) { /* Early exit for some implements. */
		if (otmp && otmp->oclass == WAND_CLASS && !can_reach_floor())
			pline("You can't reach the %s!", surface(u.ux, u.uy));
		return true;
	}

	/* Special effects should have deleted the current engraving (if
	 * possible) by now.
	 */

	if (oep) {
		char c = 'n';

		/* Give player the choice to add to engraving. */

		if (type == HEADSTONE) {
			/* no choice, only append */
			c = 'y';
		} else if ((type == oep->engr_type) && (!Blind ||
							(oep->engr_type == BURN) || (oep->engr_type == ENGRAVE))) {
			c = yn_function("Do you want to add to the current engraving?",
					ynqchars, 'y');
			if (c == 'q') {
				pline("%s", "Never mind.");
				return false;
			}
		}

		if (c == 'n' || Blind) {
			if ((oep->engr_type == DUST) || (oep->engr_type == ENGR_BLOOD) ||
			    (oep->engr_type == MARK)) {
				if (!Blind) {
					pline("You wipe out the message that was %s here.",
					      ((oep->engr_type == DUST) ? "written in the dust" :
									  ((oep->engr_type == ENGR_BLOOD) ? "scrawled in blood" :
													    "written")));
					del_engr(oep);
					oep = NULL;
				} else
					/* Don't delete engr until after we *know* we're engraving */
					eow = true;
			} else if ((type == DUST) || (type == MARK) || (type == ENGR_BLOOD)) {
				pline("You cannot wipe out the message that is %s the %s here.",
				      oep->engr_type == BURN ?
					      (is_ice(u.ux, u.uy) ? "melted into" : "burned into") :
					      "engraved in",
				      surface(u.ux, u.uy));
				return true;
			} else if ((type != oep->engr_type) || (c == 'n')) {
				if (!Blind || can_reach_floor())
					pline("You will overwrite the current message.");
				eow = true;
			}
		}
	}

	eloc = surface(u.ux, u.uy);
	switch (type) {
		default:
			everb = (oep && !eow ? "add to the weird writing on" :
					       "write strangely on");
			break;
		case DUST:
			everb = (oep && !eow ? "add to the writing in" :
					       "write in");
			eloc = (is_ice(u.ux, u.uy) ? "frost" : "dust");
			break;
		case HEADSTONE:
			everb = (oep && !eow ? "add to the epitaph on" :
					       "engrave on");
			break;
		case ENGRAVE:
			everb = (oep && !eow ? "add to the engraving in" :
					       "engrave in");
			break;
		case BURN:
			everb = (oep && !eow ?
					 (is_ice(u.ux, u.uy) ? "add to the text melted into" :
							       "add to the text burned into") :
					 (is_ice(u.ux, u.uy) ? "melt into" : "burn into"));
			break;
		case MARK:
			everb = (oep && !eow ? "add to the graffiti on" :
					       "scribble on");
			break;
		case ENGR_BLOOD:
			everb = (oep && !eow ? "add to the scrawl on" :
					       "scrawl on");
			break;
	}

	/* Tell adventurer what is going on */
	if (otmp != &zeroobj)
		pline("You %s the %s with %s.", everb, eloc, doname(otmp));
	else
		pline("You %s the %s with your %s.", everb, eloc,
		      makeplural(body_part(FINGER)));

	/* Prompt for engraving! */
	sprintf(qbuf, "What do you want to %s the %s here?", everb, eloc);
	getlin(qbuf, ebuf);

	/* Count the actual # of chars engraved not including spaces */
	len = strlen(ebuf);
	for (sp = ebuf; *sp; sp++)
		if (isspace(*sp)) len -= 1;

	if (len == 0 || index(ebuf, '\033')) {
		if (zapwand) {
			if (!Blind)
				pline("%s, then %s.",
				      Tobjnam(otmp, "glow"), otense(otmp, "fade"));
			return true;
		} else {
			pline("%s", "Never mind.");
			return false;
		}
	}

	/* A single `x' is the traditional signature of an illiterate person */
	if (len != 1 || (!index(ebuf, 'x') && !index(ebuf, 'X')))
		u.uconduct.literate++;

	/* Mix up engraving if surface or state of mind is unsound.
	   Note: this won't add or remove any spaces. */
	for (sp = ebuf; *sp; sp++) {
		if (isspace(*sp)) continue;
		if (((type == DUST || type == ENGR_BLOOD) && !rn2(25)) ||
		    (Blind && !rn2(11)) || (Confusion && !rn2(7)) ||
		    (Stunned && !rn2(4)) || (Hallucination && !rn2(2)))
			*sp = ' ' + rnd(96 - 2); /* ASCII '!' thru '~'
						   (excludes ' ' and DEL) */
	}

	/* Previous engraving is overwritten */
	if (eow) {
		del_engr(oep);
		oep = NULL;
	}

	/* Figure out how long it took to engrave, and if player has
	 * engraved too much.
	 */
	switch (type) {
		default:
			multi = -(len / 10);
			if (multi) nomovemsg = "You finish your weird engraving.";
			break;
		case DUST:
			multi = -(len / 10);
			if (multi) nomovemsg = "You finish writing in the dust.";
			break;
		case HEADSTONE:
		case ENGRAVE:
			multi = -(len / 10);
			if ((otmp->oclass == WEAPON_CLASS) &&
			    ((otmp->otyp != ATHAME) || otmp->cursed)) {
				multi = -len;
				maxelen = ((otmp->spe + 3) * 2) + 1;
				/* -2 = 3, -1 = 5, 0 = 7, +1 = 9, +2 = 11
			 * Note: this does not allow a +0 anything (except
			 *	 an athame) to engrave "Elbereth" all at once.
			 *	 However, you could now engrave "Elb", then
			 *	 "ere", then "th".
			 */
				pline("%s dull.", Yobjnam2(otmp, "get"));
				if (otmp->unpaid) {
					struct monst *shkp = shop_keeper(*u.ushops);
					if (shkp) {
						pline("You damage it, you pay for it!");
						bill_dummy_object(otmp);
					}
				}
				if (len > maxelen) {
					multi = -maxelen;
					otmp->spe = -3;
				} else if (len > 1)
					otmp->spe -= len >> 1;
				else
					otmp->spe -= 1; /* Prevent infinite engraving */
			} else if ((otmp->oclass == RING_CLASS) ||
				   (otmp->oclass == GEM_CLASS))
				multi = -len;
			if (multi) nomovemsg = "You finish engraving.";
			break;
		case BURN:
			multi = -(len / 10);
			if (multi)
				nomovemsg = is_ice(u.ux, u.uy) ?
						    "You finish melting your message into the ice." :
						    "You finish burning your message into the floor.";
			break;
		case MARK:
			multi = -(len / 10);
			if ((otmp->oclass == TOOL_CLASS) &&
			    (otmp->otyp == MAGIC_MARKER)) {
				maxelen = (otmp->spe) * 2; /* one charge / 2 letters */
				if (len > maxelen) {
					pline("Your marker dries out.");
					otmp->spe = 0;
					multi = -(maxelen / 10);
				} else if (len > 1)
					otmp->spe -= len >> 1;
				else
					otmp->spe -= 1; /* Prevent infinite grafitti */
			}
			if (multi) nomovemsg = "You finish defacing the dungeon.";
			break;
		case ENGR_BLOOD:
			multi = -(len / 10);
			if (multi) nomovemsg = "You finish scrawling.";
			break;
	}

	/* Chop engraving down to size if necessary */
	if (len > maxelen) {
		for (sp = ebuf; (maxelen && *sp); sp++)
			if (!isspace((int)*sp)) maxelen--;
		if (!maxelen && *sp) {
			*sp = 0;
			if (multi) nomovemsg = "You cannot write any more.";
			pline("You only are able to write \"%s\"", ebuf);
		}
	}

	/* Add to existing engraving */
	if (oep) strcpy(buf, oep->engr_txt);

	strncat(buf, ebuf, (sizeof(buf) - strlen(buf) - 1));

	make_engr_at(u.ux, u.uy, buf, (moves - multi), type);

	if (post_engr_text[0])
		pline("%s", post_engr_text);

	if (doblind) flashburn(rnd(50));

	if (postknown) {
		makeknown(otmp->otyp);
	}

	return true;
}

void save_engravings(int fd, int mode) {
	struct engr *ep = head_engr;
	struct engr *ep2;
	unsigned no_more_engr = 0;

	while (ep) {
		ep2 = ep->nxt_engr;
		if (ep->engr_lth && ep->engr_txt[0] && perform_bwrite(mode)) {
			bwrite(fd, (void *)&(ep->engr_lth), sizeof(ep->engr_lth));
			bwrite(fd, (void *)ep, sizeof(struct engr) + ep->engr_lth);
		}
		if (release_data(mode))
			dealloc_engr(ep);
		ep = ep2;
	}
	if (perform_bwrite(mode))
		bwrite(fd, (void *)&no_more_engr, sizeof no_more_engr);
	if (release_data(mode))
		head_engr = 0;
}

void rest_engravings(int fd) {
	struct engr *ep;
	unsigned lth;

	head_engr = 0;
	while (1) {
		mread(fd, (void *)&lth, sizeof(unsigned));
		if (lth == 0) return;
		ep = newengr(lth);
		mread(fd, (void *)ep, sizeof(struct engr) + lth);
		ep->nxt_engr = head_engr;
		head_engr = ep;
		ep->engr_txt = (char *)(ep + 1); /* Andreas Bormann */
		/* mark as finished for bones levels -- no problem for
		 * normal levels as the player must have finished engraving
		 * to be able to move again */
		ep->engr_time = moves;
	}
}

void del_engr(struct engr *ep) {
	if (ep == head_engr) {
		head_engr = ep->nxt_engr;
	} else {
		struct engr *ept;

		for (ept = head_engr; ept; ept = ept->nxt_engr)
			if (ept->nxt_engr == ep) {
				ept->nxt_engr = ep->nxt_engr;
				break;
			}
		if (!ept) {
			impossible("Error in del_engr?");
			return;
		}
	}
	dealloc_engr(ep);
}

/* randomly relocate an engraving */
void rloc_engr(struct engr *ep) {
	int tx, ty, tryct = 200;

	do {
		if (--tryct < 0) return;
		tx = rn1(COLNO - 3, 2);
		ty = rn2(ROWNO);
	} while (engr_at(tx, ty) ||
		 !goodpos(tx, ty, NULL, 0));

	ep->engr_x = tx;
	ep->engr_y = ty;
}

/* Epitaphs for random headstones */
static const char *epitaphs[] = {
	"Rest in peace",
	"R.I.P.",
	"Rest In Pieces",
	"Note -- there are NO valuable items in this grave",
	"1994-1995. The Longest-Lived Hacker Ever",
	"The Grave of the Unknown Hacker",
	"We weren't sure who this was, but we buried him here anyway",
	"Sparky -- he was a very good dog",
	"Beware of Electric Third Rail",
	"Made in Taiwan",
	"Og friend. Og good dude. Og died. Og now food",
	"Beetlejuice Beetlejuice Beetlejuice",
	"Look out below!",
	"Please don't dig me up. I'm perfectly happy down here. -- Resident",
	"Postman, please note forwarding address: Gehennom, Asmodeus's Fortress, fifth lemure on the left",
	"Mary had a little lamb/Its fleece was white as snow/When Mary was in trouble/The lamb was first to go",
	"Be careful, or this could happen to you!",
	"Soon you'll join this fellow in hell! -- the Wizard of Yendor",
	"Caution! This grave contains toxic waste",
	"Sum quod eris",
	"Here lies an Atheist, all dressed up and no place to go",
	"Here lies Ezekiel, age 102.  The good die young.",
	"Here lies my wife: Here let her lie! Now she's at rest and so am I.",
	"Here lies Johnny Yeast. Pardon me for not rising.",
	"He always lied while on the earth and now he's lying in it",
	"I made an ash of myself",
	"Soon ripe. Soon rotten. Soon gone. But not forgotten.",
	"Here lies the body of Jonathan Blake. Stepped on the gas instead of the brake.",
	"Go away!",
	/* From SLASH'EM */
	"This old man, he played one, he played knick-knack on my thumb."};

/* Create a headstone at the given location.
 * The caller is responsible for newsym(x, y).
 */
void make_grave(int x, int y, const char *str) {
	/* Can we put a grave here? */
	if ((levl[x][y].typ != ROOM && levl[x][y].typ != GRAVE) || t_at(x, y)) return;

	/* Make the grave */
	levl[x][y].typ = GRAVE;

	/* Engrave the headstone */
	if (!str) str = epitaphs[rn2(SIZE(epitaphs))];
	del_engr_at(x, y);
	make_engr_at(x, y, str, 0L, HEADSTONE);
	return;
}

/*engrave.c*/
