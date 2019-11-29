#include "config.h"
#include "permonst.h"
#include "mondata.h"
#include "pm.h"

// returns true if monster can attack at range
static bool ranged_attk(struct permonst *mon) {
	int i, j;
	int atk_mask = (1<<AT_BREA) | (1<<AT_SPIT) | (1<<AT_GAZE);

	for (i = 0; i < NATTK; i++) {
		if ((j=mon->mattk[i].aatyp) >= AT_WEAP || (atk_mask & (1<<j)))
			return true;
	}

	return false;
}

/* This routine is designed to return an integer value which represents
 * an approximation of monster strength.  It uses a similar method of
 * determination as "experience()" to arrive at the strength.
 */
int mstrength(struct permonst *mon) {
	int i, tmp2, n, tmp = mon->mlevel;

	if(tmp > 49)            /* special fixed hp monster */
		tmp = 2*(tmp - 6) / 4;

	// For creation in groups
	n = (!!(mon->geno & G_SGROUP));
	n += (!!(mon->geno & G_LGROUP)) << 1;
	n += (!!(mon->geno & G_VLGROUP)) << 2;

	// For ranged attacks
	if (ranged_attk(mon)) n++;

	// For higher ac values
	n += (mon->ac < 4);
	n += (mon->ac < 0);

	// For very fast monsters
	n += (mon->mmove >= 18);

	// For each attack and "special" attack
	for(i = 0; i < NATTK; i++) {

		tmp2 = mon->mattk[i].aatyp;
		n += (tmp2 > 0);
		n += (tmp2 == AT_MAGC);
		n += (tmp2 == AT_WEAP && (mon->mflags2 & M2_STRONG));
	}

	// For each "special" damage type
	for(i = 0; i < NATTK; i++) {
		tmp2 = mon->mattk[i].adtyp;
		if ((tmp2 == AD_DRLI) || (tmp2 == AD_STON) || (tmp2 == AD_DRST)
		                || (tmp2 == AD_DRDX) || (tmp2 == AD_DRCO) || (tmp2 == AD_WERE))
			n += 2;
		else if (mon != &mons[PM_GRID_BUG]) n += (tmp2 != AD_PHYS);
		n += ((int) (mon->mattk[i].damd * mon->mattk[i].damn) > 23);
	}
	// tom's nasties
	if (extra_nasty(mon)) n += 5;

	/* Leprechauns are special cases.  They have many hit dice so they
	   can hit and are hard to kill, but they don't really do much damage. */
	if (mon == &mons[PM_LEPRECHAUN]) n -= 2;

	// Finally, adjust the monster level  0 <= n <= 24 (approx.)
	if(n == 0) tmp--;
	else if(n >= 6) tmp += ( n / 2 );
	else tmp += ( n / 3 + 1);

	return (tmp >= 0) ? tmp : 0;
}
