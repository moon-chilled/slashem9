from ruamel.yaml import YAML

# copy-pasted from monst.c
file_header="""/* This file is automatically generated from monst.yaml by "yaml2mon.py".  Do not edit. */
#include "config.h"
#include "permonst.h"
#include "monsym.h"
#include "dungeon.h"    /* prerequisite for eshk,vault,epri */
#include "eshk.h"
#include "vault.h"
#include "epri.h"
#include "egyp.h"


/* #ifdef MUSE */
#define MARM(x,y) x /* x is usually 10 */
/* #else
   #define MARM(x,y) y
   #endif */

#define NO_ATTK {0,0,0,0}

#define WT_ELF     800
#define WT_DRAGON 4500

#ifdef C
#undef C
#endif
#ifdef TEXTCOLOR
#include "color.h"
#define C(color)        color
#define HI_DOMESTIC     CLR_WHITE       /* use for player + friendlies */
#define HI_LORD         CLR_MAGENTA
#else
#define C(color)
t_init(void);
/*
 *      Entry Format:           (from permonst.h)
 *
 *      name, symbol (S_* defines),
 *      difficulty level, move rate, armor class, magic resistance,
 *      alignment, creation/geno flags (G_* defines),
 *      6 * attack structs ATTK(type , damage-type, # dice, # sides),
 *      weight (WT_* defines), nutritional value, extension length,
 *      sounds made (MS_* defines), physical size (MZ_* defines),
 *      resistances, resistances conferred (both MR_* defines),
 *      3 * flag bitmaps (M1_*, M2_*, and M3_* defines respectively)
 *      symbol color (C(x) macro)
 */
#define MON(nam,sym,lvl,gen,atk,siz,mr1,mr2,flg1,flg2,flg3,col) \
           {nam,sym,lvl,gen,atk,siz,mr1,mr2,flg1,flg2,flg3,C(col)}
/* LVL() and SIZ() collect several fields to cut down on # of args for MON() */
#define LVL(lvl,mov,ac,mr,aln) lvl,mov,ac,mr,aln
#define SIZ(wt,nut,pxl,snd,siz) wt,nut,pxl,snd,siz
/* ATTK() and A() are to avoid braces and commas within args to MON() */
#define ATTK(at,ad,n,d) {at,ad,n,d}
#define A(a1,a2,a3,a4,a5,a6) {a1,a2,a3,a4,a5,a6}


/*
 *      Rule #1:        monsters of a given class are contiguous in the
 *                      mons[] array.
 *
 *      Rule #2:        monsters of a given class are presented in ascending
 *                      order of strength.
 *
 *      Rule #3:        monster frequency is included in the geno mask;
 *                      the frequency can be from 0 to 7.  0's will also
 *                      be skipped during generation.
 *
 *      Rule #4:        monster subclasses (e.g. giants) should be kept
 *                      together, unless it violates Rule 2.  NOGEN monsters
 *                      won't violate Rule 2.
 *
 * Guidelines for color assignment:
 *
 *      * Use the same color for all `growth stages' of a monster (ex.
 *        little dog/big dog, baby naga/full-grown naga.
 *
 *      * Use colors given in names wherever possible. If the class has `real'
 *        members with strong color associations, use those.
 *
 *      * Favor `cool' colors for cold-resistent monsters, `warm' ones for
 *        fire-resistent ones.
 *
 *      * Try to reserve purple (magenta) for powerful `ruler' monsters (queen
 *        bee, kobold lord, &c.).
 *
 *      * Subject to all these constraints, try to use color to make as many
 *        distinctions as the / command (that is, within a monster letter
 *        distinct names should map to distinct colors).
 *
 * The aim in assigning colors is to be consistent enough so a player can
 * become `intuitive' about them, deducing some or all of these rules
 * unconsciously. Use your common sense.
 */

/* [Tom] I increased frequencies of all the "old" monsters, so the new ones
 *  are all that much more rare and special */

/* [Tom] I made many monsters NOHELL -- mostly natural animals and stuff */"""

montosym={
    "ant": "S_ANT",
    "blob": "S_BLOB",
    "cockatrice": "S_COCKATRICE",
    "dog": "S_DOG",
    "eye": "S_EYE",
    "feline": "S_FELINE",
    "gremlin": "S_GREMLIN",
    "humanoid": "S_HUMANOID",
    "imp": "S_IMP",
    "jelly": "S_JELLY",
    "kobold": "S_KOBOLD",
    "leprechaun": "S_LEPRECHAUN",
    "mimic": "S_MIMIC",
    "nymph": "S_NYMPH",
    "orc": "S_ORC",
    "piercer": "S_PIERCER",
    "quadruped": "S_QUADRUPED",
    "rodent": "S_RODENT",
    "spider": "S_SPIDER",
    "trapper": "S_TRAPPER",
    "unicorn": "S_UNICORN",
    "vortex": "S_VORTEX",
    "worm": "S_WORM",
    "xan": "S_XAN",
    "light": "S_LIGHT",
    "zruty": "S_ZRUTY",
    "zouthern": "S_ZOUTHERN",
    "angel": "S_ANGEL",
    "bat": "S_BAT",
    "centaur": "S_CENTAUR",
    "dragon": "S_DRAGON",
    "elemental": "S_ELEMENTAL",
    "fungus": "S_FUNGUS",
    "gnome": "S_GNOME",
    "giant": "S_GIANT",
    "jabberwock": "S_JABBERWOCK",
    "kop": "S_KOP",
    "lich": "S_LICH",
    "mummy": "S_MUMMY",
    "naga": "S_NAGA",
    "ogre": "S_OGRE",
    "pudding": "S_PUDDING",
    "quantmech": "S_QUANTMECH",
    "rustmonst": "S_RUSTMONST",
    "snake": "S_SNAKE",
    "troll": "S_TROLL",
    "umber": "S_UMBER",
    "vampire": "S_VAMPIRE",
    "wraith": "S_WRAITH",
    "xorn": "S_XORN",
    "yeti": "S_YETI",
    "zombie": "S_ZOMBIE",
    "human": "S_HUMAN",
    "ghost": "S_GHOST",
    "golem": "S_GOLEM",
    "demon": "S_DEMON",
    "eel": "S_EEL",
    "lizard": "S_LIZARD",
    "bad food": "S_BAD_FOOD",
    "bad coins": "S_BAD_COINS"}

genflags = {
    "vlgroup": "G_VLGROUP",
    "unique": "G_UNIQ",
    "nohell": "G_NOHELL",
    "onlyhell": "G_HELL",
    "nogen": "G_NOGEN",
    "sgroup":

# alignment: chaotic -> -1, neutral -> 0, lawful -> 1
# none: -128, coalined: 1, opaligned: -1

# generation flags:
#


#class Permonst:
#    def __init__(name="", sym='', mlevel mmove ac mr aligntyp):
#        const char      *mname;
#        char            mlet;
#        schar           mlevel,
#                        mmove,
#                        ac,
#                        mr;
#        aligntyp        maligntyp;
#        unsigned short  geno;
#        struct  attack  mattk[NATTK]
#        unsigned short  cwt,
#                        cnutrit;
#        short           pxlth;
#        uchar           msound;
#        uchar           msize;
#        unsigned long   mresists;
#        uchar           mconveys;
#        unsigned long   mflags1,
#                        mflags2;
#        unsigned short  mflags3;
#        uchar           mcolor;
#};


def loadfile(filename="monst.yaml"):
    yaml = YAML()

    with open(filename, "r") as outfile:
        outyaml = yaml.load(outfile.read())

    return dict(outyaml)


def yaml2monst(yaml):
    out="MON("

    # there's only one key
    # monster name
    out += "\"" + list(yaml.keys())[0] + "\","

    # We just want the data, which is indexed by name.  Harder to parse, but more intuitive to write
    yaml = yaml[list(yaml.keys())[0]]

    # symbol
    out += montosym[yaml["sym"]] + ","

    align2num = {"neutral": 0, "chaotic": -1, "lawful": 1}

    # level, speed, ac, mr,
    out += "LVL({},{},{},{},{}),".format(str(yaml["level"]), str(yaml["speed"]), str(yaml["ac"]), str(yaml["mr"]), str(align2num[yaml["align"]])