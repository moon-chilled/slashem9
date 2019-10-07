/*	SCCS Id: @(#)recover.c	3.4	1999/10/23	*/
/*	Copyright (c) Janet Walz, 1992.				  */
/* NetHack may be freely redistributed.  See license for details. */

/*
 *  Utility for reconstructing NetHack save file from a set of individual
 *  level files.  Requires that the `checkpoint' option be enabled at the
 *  time NetHack creates those level files.
 */
#include "config.h"
#if !defined(O_WRONLY) && !defined(LSC)
# include <fcntl.h>
#endif
#ifdef WIN32
# include <errno.h>
# include "win32api.h"
#endif

int restore_savefile(char *);
void set_levelfile_name(int);
int open_levelfile(int);
int create_savefile(void);
void copy_bytes(int,int);

#ifdef UNIX
# define SAVESIZE	(PL_NSIZ + 13)	/* save/99999player.e */
#else
# ifdef WIN32
#  define SAVESIZE	(PL_NSIZ + 40)  /* username-player.NetHack-saved-game */
# else /* WIN32 */
#  define SAVESIZE	FILENAMELEN
# endif /* WIN32 */
#endif

#if defined(EXEPATH)
char *exepath(char *);
#endif

char savename[SAVESIZE]; /* holds relative path of save file from playground */


int
main(argc, argv)
int argc;
char *argv[];
{
	int argno;
	const char *dir = NULL;


	if (!dir) dir = getenv("NETHACKDIR");
	if (!dir) dir = getenv("HACKDIR");
#if defined(EXEPATH)
	if (!dir) dir = exepath(argv[0]);
#endif
	if (argc == 1 || (argc == 2 && !strcmp(argv[1], "-"))) {
	    fprintf(stderr,
		"Usage: %s [ -d directory ] base1 [ base2 ... ]\n", argv[0]);
#ifdef WIN32
	    if (dir) {
		fprintf(stderr, "\t(Unless you override it with -d, recover will look \n");
		fprintf(stderr, "\t in the %s directory on your system)\n", dir);
	    }
#endif
	    exit(EXIT_FAILURE);
	}

	argno = 1;
	if (!strncmp(argv[argno], "-d", 2)) {
		dir = argv[argno]+2;
		if (*dir == '=' || *dir == ':') dir++;
		if (!*dir && argc > argno) {
			argno++;
			dir = argv[argno];
		}
		if (!*dir) {
		    fprintf(stderr,
			"%s: flag -d must be followed by a directory name.\n",
			argv[0]);
		    exit(EXIT_FAILURE);
		}
		argno++;
	}
#ifdef SECURE
	if (dir
# ifdef HACKDIR
		&& strcmp(dir, HACKDIR)
# endif
		) {
		setgid(getgid());
		setuid(getuid());
	}
#endif	/* SECURE */

#ifdef HACKDIR
	if (!dir) dir = HACKDIR;
#endif

	if (dir && chdir((char *) dir) < 0) {
		fprintf(stderr, "%s: cannot chdir to %s.\n", argv[0], dir);
		exit(EXIT_FAILURE);
	}

	while (argc > argno) {
		if (restore_savefile(argv[argno]) == 0)
		    fprintf(stderr, "recovered \"%s\" to %s\n",
			    argv[argno], savename);
		argno++;
	}
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}

static char lock[256];

void
set_levelfile_name(lev)
int lev;
{
	char *tf;

	tf = rindex(lock, '.');
	if (!tf) tf = lock + strlen(lock);
	sprintf(tf, ".%d", lev);
}

int
open_levelfile(lev)
int lev;
{
	int fd;

	set_levelfile_name(lev);
#if defined(MICRO) || defined(WIN32)
	fd = open(lock, O_RDONLY | O_BINARY);
#else
	fd = open(lock, O_RDONLY, 0);
#endif
	return fd;
}

int
create_savefile()
{
	int fd;

#if defined(MICRO) || defined(WIN32)
	fd = open(savename, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
	fd = creat(savename, FCMASK);
#endif
	return fd;
}

void
copy_bytes(ifd, ofd)
int ifd, ofd;
{
	char buf[BUFSIZ];
	int nfrom, nto;

	do {
		nfrom = read(ifd, buf, BUFSIZ);
		nto = write(ofd, buf, nfrom);
		if (nto != nfrom) {
			fprintf(stderr, "file copy failed!\n");
			exit(EXIT_FAILURE);
		}
	} while (nfrom == BUFSIZ);
}

int
restore_savefile(basename)
char *basename;
{
	int gfd, lfd, sfd;
	int lev, savelev, hpid;
	xchar levc;
	struct version_info version_data;

	/* level 0 file contains:
	 *	pid of creating process (ignored here)
	 *	level number for current level of save file
	 *	name of save file nethack would have created
	 *	and game state
	 */
	strcpy(lock, basename);
	gfd = open_levelfile(0);
	if (gfd < 0) {
#ifdef WIN32
 	    if(errno == EACCES) {
	  	fprintf(stderr,
			"\nThere are files from a game in progress under your name.");
		fprintf(stderr,"\nThe files are locked or inaccessible.");
		fprintf(stderr,"\nPerhaps the other game is still running?\n");
	    } else
	  	fprintf(stderr,
			"\nTrouble accessing level 0 (errno = %d).\n", errno);
#endif
	    fprintf(stderr, "Cannot open level 0 for %s.\n", basename);
	    return -1;
	}
	if (read(gfd, (void *) &hpid, sizeof hpid) != sizeof hpid) {
	    fprintf(stderr, "%s\n%s%s%s\n",
	     "Checkpoint data incompletely written or subsequently clobbered;",
		    "recovery for \"", basename, "\" impossible.");
	    close(gfd);
	    return -1;
	}
	if (read(gfd, (void *) &savelev, sizeof(savelev))
							!= sizeof(savelev)) {
	    fprintf(stderr,
	    "Checkpointing was not in effect for %s -- recovery impossible.\n",
		    basename);
	    close(gfd);
	    return -1;
	}
	if ((read(gfd, (void *) savename, sizeof savename)
		!= sizeof savename) ||
	    (read(gfd, (void *) &version_data, sizeof version_data)
		!= sizeof version_data)) {
	    fprintf(stderr, "Error reading %s -- can't recover.\n", lock);
	    close(gfd);
	    return -1;
	}

	/* save file should contain:
	 *	version info
	 *	current level (including pets)
	 *	(non-level-based) game state
	 *	other levels
	 */
	sfd = create_savefile();
	if (sfd < 0) {
	    fprintf(stderr, "Cannot create savefile %s.\n", savename);
	    close(gfd);
	    return -1;
	}

	lfd = open_levelfile(savelev);
	if (lfd < 0) {
	    fprintf(stderr, "Cannot open level of save for %s.\n", basename);
	    close(gfd);
	    close(sfd);
	    return -1;
	}

	if (write(sfd, (void *) &version_data, sizeof version_data)
		!= sizeof version_data) {
	    fprintf(stderr, "Error writing %s; recovery failed.\n", savename);
	    close(gfd);
	    close(sfd);
	    return -1;
	}

	copy_bytes(lfd, sfd);
	close(lfd);
	unlink(lock);

	copy_bytes(gfd, sfd);
	close(gfd);
	set_levelfile_name(0);
	unlink(lock);

	for (lev = 1; lev < 256; lev++) {
		/* level numbers are kept in xchars in save.c, so the
		 * maximum level number (for the endlevel) must be < 256
		 */
		if (lev != savelev) {
			lfd = open_levelfile(lev);
			if (lfd >= 0) {
				/* any or all of these may not exist */
				levc = (xchar) lev;
				write(sfd, (void *) &levc, sizeof(levc));
				copy_bytes(lfd, sfd);
				close(lfd);
				unlink(lock);
			}
		}
	}

	close(sfd);

	return 0;
}

#ifdef EXEPATH
# ifdef __DJGPP__
#define PATH_SEPARATOR '/'
# else
#define PATH_SEPARATOR '\\'
# endif

#define EXEPATHBUFSZ 256
char exepathbuf[EXEPATHBUFSZ];

char *exepath(char *str) {
	char *tmp, *tmp2;
	int bsize;

	if (!str) return NULL;
	bsize = EXEPATHBUFSZ;
	tmp = exepathbuf;
#ifndef WIN32
	strcpy(tmp, str);
#else
	*(tmp + GetModuleFileName(NULL, tmp, bsize)) = '\0';
#endif
	tmp2 = strrchr(tmp, PATH_SEPARATOR);
	if (tmp2) *tmp2 = '\0';
	return tmp;
}
#endif /* EXEPATH */
/*recover.c*/
