/*	SCCS Id: @(#)version.c	3.4	2003/12/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
/*
 * All the references to the contents of patchlevel.h have been moved
 * into makedefs....
 */
#include "patchlevel.h"

char *version_string_tmp(void) {
	static char buf[128];
	sprintf(buf, "%d.%d.%dE%dF%d", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, EDITLEVEL, FIXLEVEL);
	return buf;
}

char *full_version_string_tmp(void) {
	static char buf[BUFSZ];

	sprintf(buf, "%s %s%s Version %s.", PORT_ID, DEF_GAME_NAME,
#ifdef ALPHA
			" Alpha",
#elif defined(BETA)
			" Beta",
#else
			"",
#endif
			version_string_tmp());

	return buf;
}

int doversion(void) {
	plines(full_version_string_tmp());
	return 0;
}

boolean check_version(struct version_info *version_data, const char *filename, boolean complain) {
	if (
#ifdef VERSION_COMPATIBILITY
	    version_data->incarnation < VERSION_COMPATIBILITY ||
	    version_data->incarnation > VERSION_NUMBER
#else
	    version_data->incarnation != VERSION_NUMBER
#endif
	  ) {
	    if (complain)
		pline("Version mismatch for file \"%s\".", filename);
	    return false;
	} else if (version_data->struct_sizes != VERSION_SANITY) {
	    if (complain)
		pline("Configuration incompatibility for file \"%s\".",
		      filename);
	    return false;
	}
	return true;
}

/* this used to be based on file date and somewhat OS-dependant,
   but now examines the initial part of the file's contents */
boolean uptodate(int fd, const char *name) {
    int rlen;
    struct version_info vers_info;
    boolean verbose = name ? true : false;

    rlen = read(fd, (void *) &vers_info, sizeof vers_info);
    minit();		/* ZEROCOMP */
    if (rlen == 0) {
	if (verbose) {
	    pline("File \"%s\" is empty?", name);
	    wait_synch();
	}
	return false;
    }
    if (!check_version(&vers_info, name, verbose)) {
	if (verbose) wait_synch();
	return false;
    }
    return true;
}

void store_version(int fd) {
	const static struct version_info version_data = {
			VERSION_NUMBER, VERSION_SANITY
	};

	bufoff(fd);
	/* bwrite() before bufon() uses plain write() */
	bwrite(fd,(void *)&version_data,(unsigned)(sizeof version_data));
	bufon(fd);
	return;
}

unsigned long get_feature_notice_ver (char *str) {
	char buf[BUFSZ];
	int ver_maj, ver_min, patch;
	char *istr[3];
	int j = 0;

	if (!str) return 0L;
	str = strcpy(buf, str);
	istr[j] = str;
	while (*str) {
		if (*str == '.') {
			*str++ = '\0';
			j++;
			istr[j] = str;
			if (j == 2) break;
		} else if (index("0123456789", *str) != 0) {
			str++;
		} else
			return 0L;
	}
	if (j != 2) return 0L;
	ver_maj = atoi(istr[0]);
	ver_min = atoi(istr[1]);
	patch = atoi(istr[2]);
	return FEATURE_NOTICE_VER(ver_maj,ver_min,patch);
	/* macro from hack.h */
}

unsigned long get_current_feature_ver(void) {
	return FEATURE_NOTICE_VER(VERSION_MAJOR,VERSION_MINOR,PATCHLEVEL);
}

/*version.c*/
