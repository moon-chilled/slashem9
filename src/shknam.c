/*	SCCS Id: @(#)shknam.c	3.4	2003/01/09	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* shknam.c -- initialize a shop */

#include "hack.h"

static void mkshobj_at(const struct shclass *, int, int);
static void nameshk(struct monst *, const char *const *);
static int shkinit(const struct shclass *, struct mkroom *);

static void stock_blkmar(const struct shclass *, struct mkroom *, int);
/* WAC init shk services */
static void init_shk_services(struct monst *);

#define VEGETARIAN_CLASS (MAXOCLASSES+1)

/*
 *  Name prefix codes:
 *     dash          _  female, personal name
 *     underscore    _  female, general name
 *     plus          +  male, personal name
 *     vertical bar  |  male, general name (implied for most of shktools)
 *     equals        =  gender not specified, personal name
 *
 *  Personal names do not receive the honorific prefix "Mr." or "Ms.".
 */

static const char *const shkliquors[] = {
	/* Ukraine */
	"Njezjin", "Tsjernigof", "Ossipewsk", "Gorlowka",
	/* Belarus */
	"Gomel",
	/* N. Russia */
	"Konosja", "Weliki Oestjoeg", "Syktywkar", "Sablja",
	"Narodnaja", "Kyzyl",
	/* Silezie */
	"Walbrzych", "Swidnica", "Klodzko", "Raciborz", "Gliwice",
	"Brzeg", "Krnov", "Hradec Kralove",
	/* Schweiz */
	"Leuk", "Brig", "Brienz", "Thun", "Sarnen", "Burglen", "Elm",
	"Flims", "Vals", "Schuls", "Zum Loch",
	NULL};

static const char *const shkbooks[] = {
	/* Eire */
	"Skibbereen", "Kanturk", "Rath Luirc", "Ennistymon", "Lahinch",
	"Kinnegad", "Lugnaquillia", "Enniscorthy", "Gweebarra",
	"Kittamagh", "Nenagh", "Sneem", "Ballingeary", "Kilgarvan",
	"Cahersiveen", "Glenbeigh", "Kilmihil", "Kiltamagh",
	"Droichead Atha", "Inniscrone", "Clonegal", "Lisnaskea",
	"Culdaff", "Dunfanaghy", "Inishbofin", "Kesh",
	NULL};

static const char *const shkarmors[] = {
	/* Turquie */
	"Demirci", "Kalecik", "Boyabai", "Yildizeli", "Gaziantep",
	"Siirt", "Akhalataki", "Tirebolu", "Aksaray", "Ermenak",
	"Iskenderun", "Kadirli", "Siverek", "Pervari", "Malasgirt",
	"Bayburt", "Ayancik", "Zonguldak", "Balya", "Tefenni",
	"Artvin", "Kars", "Makharadze", "Malazgirt", "Midyat",
	"Birecik", "Kirikkale", "Alaca", "Polatli", "Nallihan",
	NULL};

static const char *const shkwands[] = {
	/* Wales */
	"Yr Wyddgrug", "Trallwng", "Mallwyd", "Pontarfynach",
	"Rhaeader", "Llandrindod", "Llanfair-ym-muallt",
	"Y-Fenni", "Maesteg", "Rhydaman", "Beddgelert",
	"Curig", "Llanrwst", "Llanerchymedd", "Caergybi",
	/* Scotland */
	"Nairn", "Turriff", "Inverurie", "Braemar", "Lochnagar",
	"Kerloch", "Beinn a Ghlo", "Drumnadrochit", "Morven",
	"Uist", "Storr", "Sgurr na Ciche", "Cannich", "Gairloch",
	"Kyleakin", "Dunvegan",
	NULL};

static const char *const shkrings[] = {
	/* Hollandse familienamen */
	"Feyfer", "Flugi", "Gheel", "Havic", "Haynin", "Hoboken",
	"Imbyze", "Juyn", "Kinsky", "Massis", "Matray", "Moy",
	"Olycan", "Sadelin", "Svaving", "Tapper", "Terwen", "Wirix",
	"Ypey",
	/* Skandinaviske navne */
	"Rastegaisa", "Varjag Njarga", "Kautekeino", "Abisko",
	"Enontekis", "Rovaniemi", "Avasaksa", "Haparanda",
	"Lulea", "Gellivare", "Oeloe", "Kajaani", "Fauske",
	NULL};

static const char *const shkfoods[] = {
	/* Indonesia */
	"Djasinga", "Tjibarusa", "Tjiwidej", "Pengalengan",
	"Bandjar", "Parbalingga", "Bojolali", "Sarangan",
	"Ngebel", "Djombang", "Ardjawinangun", "Berbek",
	"Papar", "Baliga", "Tjisolok", "Siboga", "Banjoewangi",
	"Trenggalek", "Karangkobar", "Njalindoeng", "Pasawahan",
	"Pameunpeuk", "Patjitan", "Kediri", "Pemboeang", "Tringanoe",
	"Makin", "Tipor", "Semai", "Berhala", "Tegal", "Samoe",
	NULL};

/*
static const char *const shkweapons[] = {
	// Perigord
	"Voulgezac", "Rouffiac", "Lerignac", "Touverac", "Guizengeard",
	"Melac", "Neuvicq", "Vanzac", "Picq", "Urignac", "Corignac",
	"Fleac", "Lonzac", "Vergt", "Queyssac", "Liorac", "Echourgnac",
	"Cazelon", "Eypau", "Carignan", "Monbazillac", "Jonzac",
	"Pons", "Jumilhac", "Fenouilledes", "Laguiolet", "Saujon",
	"Eymoutiers", "Eygurande", "Eauze", "Labouheyre",
	NULL};
*/

static const char *const shktools[] = {
	/* Spmi */
	"Ymla", "Eed-morra", "Cubask", "Nieb", "Bnowr Falr", "Telloc Cyaj",
	"Sperc", "Noskcirdneh", "Yawolloh", "Hyeghu", "Niskal", "Trahnil",
	"Htargcm", "Enrobwem", "Kachzi Rellim", "Regien", "Donmyar",
	"Yelpur", "Nosnehpets", "Stewe", "Renrut", "_Zlaw", "Nosalnef",
	"Rewuorb", "Rellenk", "Yad", "Cire Htims", "Y-crad", "Nenilukah",
	"Corsh", "Aned",

	"Lechaim", "Lexa", "Niod", // originally #ifdef WIN32 -MC

	"Nhoj-lee", "Evad\'kh", "Ettaw-noj", "Tsew-mot", "Ydna-s",
	"Yao-hang", "Tonbar", "Nivek", "Kivenhoug", // originally #ifdef MAC -MC

	NULL};

static const char *const shklight[] = {
	/* Romania */
	"Zarnesti", "Slanic", "Nehoiasu", "Ludus", "Sighisoara", "Nisipitu",
	"Razboieni", "Bicaz", "Dorohoi", "Vaslui", "Fetesti", "Tirgu Neamt",
	"Babadag", "Zimnicea", "Zlatna", "Jiu", "Eforie", "Mamaia",
	/* Bulgaria */
	"Silistra", "Tulovo", "Panagyuritshte", "Smolyan", "Kirklareli",
	"Pernik", "Lom", "Haskovo", "Dobrinishte", "Varvara", "Oryahovo",
	"Troyan", "Lovech", "Sliven",
	NULL};

static const char *const shkgeneral[] = {
	/* Suriname */
	"Hebiwerie", "Possogroenoe", "Asidonhopo", "Manlobbi",
	"Adjama", "Pakka Pakka", "Kabalebo", "Wonotobo",
	"Akalapi", "Sipaliwini",
	/* Greenland */
	"Annootok", "Upernavik", "Angmagssalik",
	/* N. Canada */
	"Aklavik", "Inuvik", "Tuktoyaktuk",
	"Chicoutimi", "Ouiatchouane", "Chibougamau",
	"Matagami", "Kipawa", "Kinojevis",
	"Abitibi", "Maganasipi",
	/* Iceland */
	"Akureyri", "Kopasker", "Budereyri", "Akranes", "Bordeyri",
	"Holmavik",
	NULL};


static const char *const shkhealthfoods[] = {
	/* Tibet */
	"Ga'er", "Zhangmu", "Rikaze", "Jiangji",
	"Changdu", "Linzhi", "Shigatse", "Gyantse",
	"Ganden", "Tsurphu", "Lhasa", "Tsedong",
	"Drepung",
	/* Hippie names */
	"=Azura", "=Blaze", "=Breanna", "=Breezy",
	"=Dharma", "=Feather", "=Jasmine", "=Luna",
	"=Melody", "=Moonjava", "=Petal", "=Rhiannon",
	"=Starla", "=Tranquilla", "=Windsong", "=Zennia",
	"=Zoe", "=Zora",
	NULL
};

// based on GTA characters' names
static const char *const shkguns[] = {
	// 3
	"Claude", "_Catalina", "8-Ball", "Salvatore",
	// Vice City
	"Tommy", "Ken", "Lance", "Sonny", "Ricardo",
	// San Andreas
	"Carl", "Sean", "Melvin",
	// 4
	"Niko", "Dimitri", "Mikhail", "Vlad",
	// Chinatown Wars
	"Huang", "Wu",
	// V
	"Michael", "Franklin", "Trevor", "Steve",
	"Weston",
	0
};

static const char *const shkblack[] = {
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	"One-eyed Sam", "One-eyed Sam", "One-eyed Sam",
	NULL};

/* STEPHEN WHITE'S NEW CODE */
static const char *shkpet[] = {
	"Silistra", "Tulovo", "Panagyuritshte", "Smolyan", "Kirklareli",
	"Pernik", "Lom", "Haskovo", "Dobrinishte", "Varvara", "Oryahovo",
	"Troyan", "Lovech", "Sliven",
	NULL};

/*
 * To add new shop types, all that is necessary is to edit the shtypes[] array.
 * See mkroom.h for the structure definition.  Typically, you'll have to lower
 * some or all of the probability fields in old entries to free up some
 * percentage for the new type.
 *
 * The placement type field is not yet used but will be in the near future.
 *
 * The iprobs array in each entry defines the probabilities for various kinds
 * of objects to be present in the given shop type.  You can associate with
 * each percentage either a generic object type (represented by one of the
 * *_CLASS macros) or a specific object (represented by an onames.h define).
 * In the latter case, prepend it with a unary minus so the code can know
 * (by testing the sign) whether to use mkobj() or mksobj().
 */
/* KMH -- Don't forget to update mkroom.h and lev_main.c when adding
 * a new shop type.
 */
const struct shclass shtypes[] = {
	{"general store", RANDOM_CLASS, 22, D_SHOP, {{100, RANDOM_CLASS}, {0, 0}, {0, 0}}, shkgeneral},
	{"used armor dealership", ARMOR_CLASS, 20, D_SHOP, {{90, ARMOR_CLASS}, {10, WEAPON_CLASS}, {0, 0}}, shkarmors},
	{"second-hand bookstore", SCROLL_CLASS, 4, D_SHOP, {{90, SCROLL_CLASS}, {10, SPBOOK_CLASS}, {0, 0}}, shkbooks},
	{"liquor emporium", POTION_CLASS, 4, D_SHOP, {{100, POTION_CLASS}, {0, 0}, {0, 0}}, shkliquors},
	/* KMH -- no longer "antique" */
	{"weapons outlet", WEAPON_CLASS, 20, D_SHOP,
	 {{80, WEAPON_CLASS}, {6, -BULLET}, {3, -BULLET}, {1, -SILVER_BULLET}, {10, ARMOR_CLASS}, {0, 0}},
	 shkguns},

	// TODO, add back antique weapons outlet, but without firearms/etc. --ELR
	/*
	{"antique weapons outlet", WEAPON_CLASS, 20, D_SHOP,
	 {{90, WEAPON_CLASS}, {10, ARMOR_CLASS}, {0, 0} },
	 shkweapons},
	*/

	{"delicatessen", FOOD_CLASS, 4, D_SHOP,
	 {{83, FOOD_CLASS}, {5, -POT_FRUIT_JUICE}, {4, -POT_BOOZE}, {5, -POT_WATER}, {3, -ICE_BOX}},
	 shkfoods},

	{"jewelers", RING_CLASS, 4, D_SHOP,
	 {{85, RING_CLASS}, {10, GEM_CLASS}, {5, AMULET_CLASS}, {0, 0}},
	 shkrings},

	{"quality apparel and accessories", WAND_CLASS, 4, D_SHOP,
	 {{90, WAND_CLASS}, {5, -LEATHER_GLOVES}, {5, -ELVEN_CLOAK}, {0, 0}},
	 shkwands},

	{"hardware store", TOOL_CLASS, 4, D_SHOP,
	 {{100, TOOL_CLASS}, {0, 0}},
	 shktools},

	/* STEPHEN WHITE'S NEW CODE */
	{"pet store", FOOD_CLASS, 4, D_SHOP,
	 {{67, -FIGURINE}, {5, -LEASH}, {10, -TRIPE_RATION}, {5, -SADDLE}, {10, -TIN_WHISTLE}, {3, -MAGIC_WHISTLE}},
	 shkpet},

	/* Robin Johnson -- 4% taken from pet store */
	{"frozen food store", FOOD_CLASS, 4, D_SHOP,
	 /* shopkeeper will pay for corpses, but they aren't generated */
	 /* on the shop floor */
	 {{90, -ICE_BOX}, {10, -TIN}, {0, -CORPSE}, {0, 0}},
	 shkfoods},

	{"rare books", SPBOOK_CLASS, 4, D_SHOP,
		{{90, SPBOOK_CLASS}, {10, SCROLL_CLASS}, {0, 0}},
		shkbooks},

	{"health food store", FOOD_CLASS, 2, D_SHOP,
		{{70, VEGETARIAN_CLASS}, {20, -POT_FRUIT_JUICE}, {4, -POT_HEALING},
		 {3, -POT_FULL_HEALING}, {2, -SCR_FOOD_DETECTION}, {1, -LUMP_OF_ROYAL_JELLY}},
		shkhealthfoods},


	/* Shops below this point are "unique".  That is they must all have a
	 * probability of zero.  They are only created via the special level
	 * loader.
	 */
	{"lighting store", TOOL_CLASS, 0, D_SHOP,
	 {{23, -WAX_CANDLE}, {33, -TALLOW_CANDLE}, {5, -TORCH}, {10, -BRASS_LANTERN}, {15, -OIL_LAMP}, {3, -MAGIC_LAMP}, {6, -MAGIC_CANDLE}, {5, -POT_OIL}},
	 shklight},

	{"black market", RANDOM_CLASS, 0, D_SHOP, {{100, RANDOM_CLASS}, {0, 0}, {0, 0}}, shkblack},
	{NULL, 0, 0, 0, {{0, 0}, {0, 0}, {0, 0}}, 0}};

#if 0
/* validate shop probabilities; otherwise incorrect local changes could
   end up provoking infinite loops or wild subscripts fetching garbage */
void init_shop_selection() {
	int i, j, item_prob, shop_prob;

	for (shop_prob = 0, i = 0; i < SIZE(shtypes); i++) {
		shop_prob += shtypes[i].prob;
		for (item_prob = 0, j = 0; j < SIZE(shtypes[0].iprobs); j++)
			item_prob += shtypes[i].iprobs[j].iprob;
		if (item_prob != 100)
			panic("item probabilities total to %d for %s shops!",
			      item_prob, shtypes[i].name);
	}
	if (shop_prob != 100)
		panic("shop probabilities total to %d!", shop_prob);
}
#endif /*0*/

static int shkveg(void) {
	int i, j, maxprob, prob;
	uchar oclass = FOOD_CLASS;
	int ok[NUM_OBJECTS];

	j = maxprob = 0;
	for (i = bases[oclass]; i < NUM_OBJECTS; ++i) {
		if (objects[i].oc_material == VEGGY) {
			ok[j++] = i;
			maxprob += objects[i].oc_prob;
		}
	}
	prob = rnd(maxprob);

	j = 0;
	i = ok[0];
	while ((prob -= objects[i].oc_prob) > 0) {
		j++;
		i = ok[j];
	}

	if (objects[i].oc_class != oclass || !OBJ_NAME(objects[i]))
		panic("shkveg probtype error, oclass=%d i=%d", oclass, i);
	return i;
}


/* make an object of the appropriate type for a shop square */
static void mkshobj_at(const struct shclass *shp, int sx, int sy) {
	struct monst *mtmp;
	int atype;
	struct permonst *ptr;

	// don't generate mimics in health food stores for monk players;
	// they're supposed to be a refuse for monks
	if (rn2(100) < depth(&u.uz) && !(shp->shknms == shkhealthfoods && Role_if(PM_MONK)) &&
	    !MON_AT(sx, sy) && (ptr = mkclass(S_MIMIC, 0)) &&
	    (mtmp = makemon(ptr, sx, sy, NO_MM_FLAGS)) != 0) {
		/* note: makemon will set the mimic symbol to a shop item */
		if (rn2(10) >= depth(&u.uz)) {
			mtmp->m_ap_type = M_AP_OBJECT;
			mtmp->mappearance = STRANGE_OBJECT;
		}
	} else {
		atype = get_shop_item(shp - shtypes);
		if (atype == VEGETARIAN_CLASS)
			mksobj_at(shkveg(), sx, sy, true, true);
		else if (atype < 0)
			mksobj_at(-atype, sx, sy, true, true);
		else
			mkobj_at(atype, sx, sy, true);
	}
}

/* extract a shopkeeper name for the given shop type */
static void nameshk(struct monst *shk, const char *const *nlp) {
	int i, trycnt, names_avail;
	const char *shname = 0;
	struct monst *mtmp;
	int name_wanted;

	if (nlp == shklight && Is_minetn_level(&u.uz)) {
		/* special-case minetown lighting shk */
		shname = "+Izchak";
		shk->female = false;
	} else if (nlp == shkblack) {
		/* special-case black marketeer */
		shname = "One-eyed Sam";
		shk->female = shk->data->mflags2 & M2_MALE ? false : true;
	} else {
		/* We want variation from game to game, without needing the save
		   and restore support which would be necessary for randomization;
		   try not to make too many assumptions about time_t's internals;
		   use ledger_no rather than depth to keep mine town distinct. */
		int nseed = (int)((long)u.ubirthday / 257L);

		name_wanted = ledger_no(&u.uz) + (nseed % 13) - (nseed % 5);
		if (name_wanted < 0) name_wanted += (13 + 5);
		shk->female = name_wanted & 1;

		for (names_avail = 0; nlp[names_avail]; names_avail++)
			continue;

		for (trycnt = 0; trycnt < 50; trycnt++) {
			if (nlp == shktools) {
				shname = shktools[rn2(names_avail)];
				shk->female = false; // reversed below for '_' prefix
			} else if (name_wanted < names_avail) {
				shname = nlp[name_wanted];
			} else if ((i = rn2(names_avail)) != 0) {
				shname = nlp[i - 1];
			} else if (nlp != shkgeneral) {
				nlp = shkgeneral; /* try general names */
				for (names_avail = 0; nlp[names_avail]; names_avail++)
					continue;
				continue; /* next `trycnt' iteration */
			} else {
				shname = shk->female ? "-Lucrezia" : "+Dirk";
			}

			if (*shname == '_' || *shname == '-') shk->female = 1;
			else if (*shname == '|' || *shname == '+') shk->female = 0;

			/* is name already in use on this level? */
			for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
				if (DEADMONSTER(mtmp) || (mtmp == shk) || !mtmp->isshk) continue;
				if (strcmp(ESHK(mtmp)->shknam, shname)) continue;
				break;
			}
			if (!mtmp) break; /* new name */
		}
	}
	strncpy(ESHK(shk)->shknam, shname, PL_NSIZ);
	ESHK(shk)->shknam[PL_NSIZ - 1] = 0;
}

void neweshk(struct monst *mtmp) {
	if (!mtmp->mextra) mtmp->mextra = newmextra();
	if (!ESHK(mtmp)) {
		ESHK(mtmp) = new(struct eshk);
	}
}

void free_eshk(struct monst *mtmp) {
	if (mtmp->mextra && ESHK(mtmp)) {
		free(ESHK(mtmp));
		ESHK(mtmp) = NULL;
	}

	mtmp->isshk = 0;
}

// create a new shopkeeper in the given room
static int shkinit(const struct shclass *shp, struct mkroom *sroom) {
	int sh, sx, sy;
	struct monst *shk;
	long shkmoney; /* Temporary placeholder for Shopkeeper's initial capital */

	/* place the shopkeeper in the given room */
	sh = sroom->fdoor;
	sx = doors[sh].x;
	sy = doors[sh].y;

	/* check that the shopkeeper placement is sane */
	if (sroom->irregular) {
		int rmno = (sroom - rooms) + ROOMOFFSET;
		if (isok(sx - 1, sy) && !levl[sx - 1][sy].edge &&
		    (int)levl[sx - 1][sy].roomno == rmno)
			sx--;
		else if (isok(sx + 1, sy) && !levl[sx + 1][sy].edge &&
			 (int)levl[sx + 1][sy].roomno == rmno)
			sx++;
		else if (isok(sx, sy - 1) && !levl[sx][sy - 1].edge &&
			 (int)levl[sx][sy - 1].roomno == rmno)
			sy--;
		else if (isok(sx, sy + 1) && !levl[sx][sy + 1].edge &&
			 (int)levl[sx][sy + 1].roomno == rmno)
			sx++;
		else
			goto shk_failed;
	} else if (sx == sroom->lx - 1)
		sx++;
	else if (sx == sroom->hx + 1)
		sx--;
	else if (sy == sroom->ly - 1)
		sy++;
	else if (sy == sroom->hy + 1)
		sy--;
	else {
shk_failed:
#ifdef DEBUG
		/* Said to happen sometimes, but I have never seen it. */
		/* Supposedly fixed by fdoor change in mklev.c */
		if (wizard) {
			int j = sroom->doorct;

			pline("Where is shopdoor?");
			pline("Room at (%d,%d),(%d,%d).",
			      sroom->lx, sroom->ly, sroom->hx, sroom->hy);
			pline("doormax=%d doorct=%d fdoor=%d",
			      doorindex, sroom->doorct, sh);
			while (j--) {
				pline("door [%d,%d]", doors[sh].x, doors[sh].y);
				sh++;
			}
			display_nhwindow(WIN_MESSAGE, false);
		}
#endif
		return -1;
	}

	if (MON_AT(sx, sy)) rloc(m_at(sx, sy), false); /* insurance */

	/* now initialize the shopkeeper monster structure */

	shk = NULL;
	if (Is_blackmarket(&u.uz)) {
		shk = makemon(&mons[PM_BLACK_MARKETEER], sx, sy, MM_ESHK);
	}
	if (!shk) {
		if (!(shk = makemon(&mons[PM_SHOPKEEPER], sx, sy, MM_ESHK)))
			return -1;
	}

	shk->isshk = shk->mpeaceful = 1;
	set_malign(shk);
	shk->msleeping = 0;
	shk->mtrapseen = ~0; /* we know all the traps already */
	ESHK(shk)->shoproom = (sroom - rooms) + ROOMOFFSET;
	sroom->resident = shk;
	ESHK(shk)->shoptype = sroom->rtype;
	assign_level(&(ESHK(shk)->shoplevel), &u.uz);
	ESHK(shk)->shd.x = doors[sh].x;
	ESHK(shk)->shd.y = doors[sh].y;
	ESHK(shk)->shk.x = sx;
	ESHK(shk)->shk.y = sy;
	ESHK(shk)->robbed = 0L;
	ESHK(shk)->credit = 0L;
	ESHK(shk)->debit = 0L;
	ESHK(shk)->loan = 0L;
	ESHK(shk)->visitct = 0;
	ESHK(shk)->following = 0;
	ESHK(shk)->billct = 0;
	/* WAC init services */
	init_shk_services(shk);

	shkmoney = 1000L + 30L * (long)rnd(100); /* initial capital */
	/* [CWC] Lets not create the money yet until we see if the
		 shk is a black marketeer, else we'll have to create
		 another money object */

	if (shp->shknms == shkrings)
		mongets(shk, TOUCHSTONE);
	nameshk(shk, shp->shknms);

	if (Is_blackmarket(&u.uz))
		shkmoney = 7 * shkmoney + rn2(3 * shkmoney);

	mkmonmoney(shk, shkmoney);

	if (Is_blackmarket(&u.uz)) {
		struct obj *otmp;
		/* make sure black marketeer can wield Thiefbane */
		shk->data->maligntyp = -1;
		/* black marketeer's equipment */
		otmp = mksobj(LONG_SWORD, false, false);
		otmp = oname(otmp, artiname(ART_THIEFBANE));
		mpickobj(shk, otmp);
		if (otmp->spe < 5) otmp->spe += rnd(5);
		otmp = mksobj(SHIELD_OF_REFLECTION, false, false);
		mpickobj(shk, otmp);
		if (otmp->spe < 5) otmp->spe += rnd(5);
		otmp = mksobj(GRAY_DRAGON_SCALE_MAIL, false, false);
		mpickobj(shk, otmp);
		if (otmp->spe < 5) otmp->spe += rnd(5);
		otmp = mksobj(SPEED_BOOTS, false, false);
		mpickobj(shk, otmp);
		if (otmp->spe < 5) otmp->spe += rnd(5);
		otmp = mksobj(AMULET_OF_LIFE_SAVING, false, false);
		mpickobj(shk, otmp);
		/* wear armor and amulet */
		m_dowear(shk, true);
		otmp = mksobj(SKELETON_KEY, false, false);
		mpickobj(shk, otmp);
	}

	return sh;
}

/* stock a newly-created room with objects */
void stock_room(int shp_indx, struct mkroom *sroom) {
	/*
	 * Someday soon we'll dispatch on the shdist field of shclass to do
	 * different placements in this routine. Currently it only supports
	 * shop-style placement (all squares except a row nearest the first
	 * door get objects).
	 */
	int sx, sy, sh;
	char buf[BUFSZ];
	int rmno = (sroom - rooms) + ROOMOFFSET;
	if (shp_indx+SHOPBASE == FOODSHOP && Is_minetn_level(&u.uz) && Role_if(PM_MONK)) {
		// special-case override for minetown food store for monks
		shp_indx = HEALTHSHOP - SHOPBASE;
	}

	const struct shclass *shp = &shtypes[shp_indx];

	/* first, try to place a shopkeeper in the room */
	if ((sh = shkinit(shp, sroom)) < 0)
		return;

	/* make sure no doorways without doors, and no */
	/* trapped doors, in shops.			   */
	sx = doors[sroom->fdoor].x;
	sy = doors[sroom->fdoor].y;

	if (levl[sx][sy].doormask == D_NODOOR) {
		levl[sx][sy].doormask = D_ISOPEN;
		newsym(sx, sy);
	}
	if (levl[sx][sy].typ == SDOOR) {
		cvt_sdoor_to_door(&levl[sx][sy]); /* .typ = DOOR */
		newsym(sx, sy);
	}
	if (levl[sx][sy].doormask & D_TRAPPED)
		levl[sx][sy].doormask = D_LOCKED;

	if (levl[sx][sy].doormask == D_LOCKED) {
		int m = sx, n = sy;

		if (inside_shop(sx + 1, sy))
			m--;
		else if (inside_shop(sx - 1, sy))
			m++;
		if (inside_shop(sx, sy + 1))
			n--;
		else if (inside_shop(sx, sy - 1))
			n++;
		sprintf(buf, "Closed for inventory");
		make_engr_at(m, n, buf, 0L, DUST);
	}

	if (Is_blackmarket(&u.uz)) {
		stock_blkmar(shp, sroom, sh);
		level.flags.has_shop = true;
		return;
	}

	for (sx = sroom->lx; sx <= sroom->hx; sx++)
		for (sy = sroom->ly; sy <= sroom->hy; sy++) {
			if (sroom->irregular) {
				if (levl[sx][sy].edge || (int)levl[sx][sy].roomno != rmno ||
				    distmin(sx, sy, doors[sh].x, doors[sh].y) <= 1)
					continue;
			} else if ((sx == sroom->lx && doors[sh].x == sx - 1) ||
				   (sx == sroom->hx && doors[sh].x == sx + 1) ||
				   (sy == sroom->ly && doors[sh].y == sy - 1) ||
				   (sy == sroom->hy && doors[sh].y == sy + 1))
				continue;
			mkshobj_at(shp, sx, sy);
		}

	/*
	 * Special monster placements (if any) should go here: that way,
	 * monsters will sit on top of objects and not the other way around.
	 */

	level.flags.has_shop = true;
}

/* stock a newly-created black market with objects */
static void stock_blkmar(const struct shclass *shp, struct mkroom *sroom, int sh) {
	/*
	 * Someday soon we'll dispatch on the shdist field of shclass to do
	 * different placements in this routine. Currently it only supports
	 * shop-style placement (all squares except a row nearest the first
	 * door get objects).
	 */
	/* [max] removed register int cl,  char buf[bufsz] */
	int i, sx, sy, first = 0, next = 0, total, partial, typ;
	struct obj *otmp;
	int *clp, *lastclp;
	int goodcl[12];

	goodcl[0] = WEAPON_CLASS;
	goodcl[1] = ARMOR_CLASS;
	goodcl[2] = RING_CLASS;
	goodcl[3] = AMULET_CLASS;
	goodcl[4] = TOOL_CLASS;
	goodcl[5] = FOOD_CLASS;
	goodcl[6] = POTION_CLASS;
	goodcl[7] = SCROLL_CLASS;
	goodcl[8] = SPBOOK_CLASS;
	goodcl[9] = WAND_CLASS;
	goodcl[10] = GEM_CLASS;
	goodcl[11] = 0;

	/*
	int blkmar_gen[NUM_OBJECTS+2];
	for (i=0; i < NUM_OBJECTS; i++) {
		blkmar_gen[i] = 0;
	}
	*/

	total = 0;
	for (clp = goodcl; *clp != 0; clp++) {
		lastclp = clp;
		first = bases[*clp];
		/* this assumes that luckstone & loadstone comes just after the gems */
		next = (*clp == GEM_CLASS) ? (LOADSTONE + 1) : bases[(*clp) + 1];
		total += next - first;
	}
	if (total == 0) return;

	if (sroom->hx - sroom->lx < 2) return;
	clp = goodcl - 1;
	partial = 0;
	for (sx = sroom->lx + 1; sx <= sroom->hx; sx++) {
		if (sx == sroom->lx + 1 ||
		    ((sx - sroom->lx - 2) * total) / (sroom->hx - sroom->lx - 1) > partial) {
			clp++;
			if (clp > lastclp) clp = lastclp;
			first = bases[*clp];
			next = (*clp == GEM_CLASS) ? (LOADSTONE + 1) : bases[(*clp) + 1];
			partial += next - first;
		}

		for (sy = sroom->ly; sy <= sroom->hy; sy++) {
			if ((sx == sroom->lx && doors[sh].x == sx - 1) ||
			    (sx == sroom->hx && doors[sh].x == sx + 1) ||
			    (sy == sroom->ly && doors[sh].y == sy - 1) ||
			    (sy == sroom->hy && doors[sh].y == sy + 1) || (rn2(3)))
				continue;

			for (i = 0; i < 50; i++) {
				typ = rn2(next - first) + first;

				/* forbidden objects  */
				if (typ == AMULET_OF_YENDOR || typ == CANDELABRUM_OF_INVOCATION ||
				    typ == BELL_OF_OPENING || typ == SPE_BOOK_OF_THE_DEAD ||
				    objects[typ].oc_nowish || typ == 0)
					continue;

				otmp = mkobj_at(RANDOM_CLASS, sx, sy, true);
				/* generate multiple copies with decreasing probabilities */
				/*        if (rn2(blkmar_gen[typ]+1) && i<49)  continue; */

				/*        otmp = mksobj_at(typ, sx, sy, true, true);
					  blkmar_gen[typ]++;*/

				/* prevent wishing abuse */
				if (typ == WAN_WISHING) {
					otmp->spe = 0;
					otmp->recharged = 1;
				}
				if (typ == MAGIC_LAMP) {
					otmp->spe = 0;
				}

				break;
			}
		}
	}

	/*
	 * Special monster placements (if any) should go here: that way,
	 * monsters will sit on top of objects and not the other way around.
	 */
}

static void init_shk_services(struct monst *shk) {
	ESHK(shk)->services = 0L;

	/* KMH, balance patch 2 -- Increase probability of shopkeeper services.
	 * Requested by Dave <mitch45678@aol.com>
	 */
	if (Is_blackmarket(&u.uz)) {
		ESHK(shk)->services =
			SHK_ID_BASIC | SHK_ID_PREMIUM | SHK_UNCURSE | SHK_APPRAISE |
			SHK_SPECIAL_A | SHK_SPECIAL_B | SHK_SPECIAL_C;
		return;
	}

	/* Guarantee some form of identification
	 * 1/3 		both Basic and Premium ID
	 * 2/15 	Premium ID only
	 * 8/15 	Basic ID only
	 */
	if (!rn2(2))
		ESHK(shk)->services |= (SHK_ID_BASIC | SHK_ID_PREMIUM);
	else if (!rn2(4))
		ESHK(shk)->services |= SHK_ID_PREMIUM;
	else
		ESHK(shk)->services |= SHK_ID_BASIC;

	if (!rn2(3)) ESHK(shk)->services |= SHK_UNCURSE;

	if (!rn2(3) && shk_class_match(WEAPON_CLASS, shk))
		ESHK(shk)->services |= SHK_APPRAISE;

	if ((shk_class_match(WEAPON_CLASS, shk) == SHK_MATCH) ||
	    (shk_class_match(ARMOR_CLASS, shk) == SHK_MATCH) ||
	    (shk_class_match(WAND_CLASS, shk) == SHK_MATCH) ||
	    (shk_class_match(TOOL_CLASS, shk) == SHK_MATCH) ||
	    (shk_class_match(SPBOOK_CLASS, shk) == SHK_MATCH) ||
	    (shk_class_match(RING_CLASS, shk) == SHK_MATCH)) {
		if (!rn2(4 /*5*/)) ESHK(shk)->services |= SHK_SPECIAL_A;
		if (!rn2(4 /*5*/)) ESHK(shk)->services |= SHK_SPECIAL_B;
	}
	if (!rn2(4 /*5*/) && (shk_class_match(WEAPON_CLASS, shk) == SHK_MATCH))
		ESHK(shk)->services |= SHK_SPECIAL_C;

	return;
}

// does shkp's shop stock this item type?
bool saleable(struct monst *shkp, struct obj *obj) {
	int i, shp_indx = ESHK(shkp)->shoptype - SHOPBASE;
	const struct shclass *shp = &shtypes[shp_indx];

	if (shp->symb == RANDOM_CLASS)
		return true;

	for (i = 0; i < SIZE(shtypes[0].iprobs) && shp->iprobs[i].iprob; i++) {
		// pseudo-class needs special handling
		if (shp->iprobs[i].itype == VEGETARIAN_CLASS) {
			if ((obj->otyp == TIN || obj->otyp == CORPSE)
			    && ((obj->corpsenm >= LOW_PM && vegetarian(&mons[obj->corpsenm]))
				|| (obj->otyp == TIN && obj->spe == 1)))	// spinach
				return true;

			if (obj->oclass == FOOD_CLASS
			    && (objects[obj->otyp].oc_material == VEGGY || obj->otyp == EGG))
				return true;
		} else if ((shp->iprobs[i].itype < 0) ?
			   shp->iprobs[i].itype == -obj->otyp :
			   shp->iprobs[i].itype == obj->oclass)
			return true;
	}

	// not found
	return false;
}

/* positive value: class; negative value: specific object type */
int get_shop_item(int type) {
	const struct shclass *shp = shtypes + type;
	int i, j;

	/* select an appropriate object type at random */
	for (j = rnd(100), i = 0; (j -= shp->iprobs[i].iprob) > 0; i++)
		continue;

	return shp->iprobs[i].itype;
}

const char *shkname(struct monst *mtmp) {
	const char *shknm = ESHK(mtmp)->shknam;

	// strip prefix if present
	if (!letter(*shknm)) ++shknm;
	return shknm;
}

bool shkname_is_pname(struct monst *mtmp) {
	const char *shknm = ESHK(mtmp)->shknam;

	return *shknm == '-' || *shknm == '+' || *shknm == '=';
}

/*shknam.c*/
