/*	SCCS Id: @(#)version.c	3.4	2003/12/06	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "patchlevel.h"

char *version_string_tmp(void) {
	static char buf[128];
	sprintf(buf, "0.%dE%d", VERSION_NUM, VERSION_EDITLEVEL);
#ifdef ALPHA
	strcat(buf, "-Alpha");
#elif defined(BETA)
	strcat(buf, "-Beta");
#endif
	return buf;
}

char *full_version_string_tmp(void) {
	static char buf[BUFSZ];

	sprintf(buf, "%s %s Version %s", PORT_ID, DEF_GAME_NAME, version_string_tmp());
#ifdef SLASHEM_GIT_COMMIT_REV
	strcat(buf, " (");
	strcat(buf, SLASHEM_GIT_COMMIT_REV);
	strcat(buf, ")");
#endif
	strcat(buf, ".");

	return buf;
}

int doversion(void) {
	plines(full_version_string_tmp());
	return 0;
}

bool check_version(struct version_info *version_data, const char *filename, boolean complain) {
	if (version_data->incarnation != VERSION_NUM) {
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

	rlen = read(fd, &vers_info, sizeof vers_info);
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
	static struct version_info version_data = {VERSION_NUM, VERSION_SANITY};

	bufoff(fd);
	/* bwrite() before bufon() uses plain write() */
	bwrite(fd, &version_data, sizeof(version_data));
	bufon(fd);
	return;
}

/*version.c*/
