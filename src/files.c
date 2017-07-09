/*	SCCS Id: @(#)files.c	3.4	2003/11/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"
#include "dlb.h"

/* WAC for config file */
#include "filename.h"
/* needs to be after hack.h. Caused .slashemrc to never be read on UNIX */

#ifdef TTY_GRAPHICS
#include "wintty.h" /* more() */
#endif

#if defined(GL_GRAPHICS) || defined(SDL_GRAPHICS)
#include "winGL.h"  /* Sdlgl_parse_options */
#endif

#ifdef PROXY_GRAPHICS
#include "winproxy.h" /* proxy_config_open() */
#endif

#include <ctype.h>

#if !defined(MAC) && !defined(O_WRONLY)
#include <fcntl.h>
#endif

#include <errno.h>
#ifdef _MSC_VER	/* MSC 6.0 defines errno quite differently */
# if (_MSC_VER >= 600)
#  define SKIP_ERRNO
# endif
#else
# ifdef NHSTDC
#  define SKIP_ERRNO
# endif
#endif
#ifndef SKIP_ERRNO
extern int errno;
#endif

#ifdef UNIX
#include <signal.h>
#endif

#ifndef NO_SIGNAL
#include <signal.h>
#endif

/* WAC moved to below
#include <sys\stat.h>
*/

/* ALI: For compatibility */
#ifndef FILE_AREAS
#define compress(file)		compress_area(NULL, file)
#define uncompress(file)	uncompress_area(NULL, file)
#endif

#ifdef WIN32
#include <sys\stat.h>
#endif
#ifndef O_BINARY	/* used for micros, no-op for others */
# define O_BINARY 0
#endif

#ifdef PREFIXES_IN_USE
#define FQN_NUMBUF 4
static char fqn_filename_buffer[FQN_NUMBUF][FQN_MAX_FILENAME];
#endif

#ifndef WIN32
char bones[] = "bonesnn.xxx";
char lock[PL_NSIZ+14] = "1lock"; /* long enough for uid+name+.99 */
#else
char bones[] = "bonesnn.xxx";
char lock[PL_NSIZ+25];		/* long enough for username+-+name+.99 */
#endif

#ifdef UNIX
# define SAVESIZE	(PL_NSIZ + 13)	/* save/99999player.e */
#else
# if defined(WIN32)
#  define SAVESIZE	(PL_NSIZ + 40)	/* username-player.NetHack-saved-game */
# else
#  define SAVESIZE        FILENAMELEN        /* from macconf.h or pcconf.h */
# endif
#endif

char SAVEF[SAVESIZE];	/* holds relative path of save file from playground */
#ifdef MICRO
char SAVEP[SAVESIZE];	/* holds path of directory for save file */
#endif

#ifdef HOLD_LOCKFILE_OPEN
struct level_ftrack {
int init;
int fd;					/* file descriptor for level file     */
int oflag;				/* open flags                         */
boolean nethack_thinks_it_is_open;	/* Does NetHack think it's open?       */
} lftrack;
# if defined(WIN32)
#include <share.h>
# endif
#endif /*HOLD_LOCKFILE_OPEN*/

#ifdef WIZARD
#define WIZKIT_MAX 128
static char wizkit[WIZKIT_MAX];
static FILE *fopen_wizkit_file(void);
#endif

#ifdef WIN32
static int lockptr;
#define Close close
#define DeleteFile unlink
#endif

#ifdef MAC
# define unlink macunlink
#endif

#ifdef USER_SOUNDS
extern char *sounddir;
#endif

extern int n_dgns;		/* from dungeon.c */

static char *set_bonesfile_name(char *,d_level*);
static char *set_bonestemp_name(void);
#ifdef COMPRESS
static void redirect(const char *,const char *,const char *,
	FILE *,boolean);
static void docompress_file(const char *,const char *,boolean);
#endif
#ifndef FILE_AREAS
static char *make_lockname(const char *,char *);
#endif
static FILE *fopen_config_file(const char *);
static int get_uchars(FILE *,char *,char *,uchar *,boolean,int,const char *);
int parse_config_line(FILE *,char *,char *,char *);
#ifdef NOCWD_ASSUMPTIONS
static void adjust_prefix(char *, int);
#endif
#ifdef SELF_RECOVER
static boolean copy_bytes(int, int);
#endif
#ifdef HOLD_LOCKFILE_OPEN
static int open_levelfile_exclusively(const char *, int, int);
#endif

/*
 * fname_encode()
 *
 *   Args:
 *	legal		zero-terminated list of acceptable file name characters
 *	quotechar	lead-in character used to quote illegal characters as hex digits
 *	s		string to encode
 *	callerbuf	buffer to house result
 *	bufsz		size of callerbuf
 *
 *   Notes:
 *	The hex digits 0-9 and A-F are always part of the legal set due to
 *	their use in the encoding scheme, even if not explicitly included in 'legal'.
 *
 *   Sample:
 *	The following call:
 *	    (void)fname_encode("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz",
 *				'%', "This is a % test!", buf, 512);
 *	results in this encoding:
 *	    "This%20is%20a%20%25%20test%21"
 */
char *
fname_encode (const char *legal, char quotechar, char *s, char *callerbuf, int bufsz)
{
	char *sp, *op;
	int cnt = 0;
	static char hexdigits[] = "0123456789ABCDEF";

	sp = s;
	op = callerbuf;
	*op = '\0';
	
	while (*sp) {
		/* Do we have room for one more character or encoding? */
		if ((bufsz - cnt) <= 4) return callerbuf;

		if (*sp == quotechar) {
			(void)sprintf(op, "%c%02X", quotechar, *sp);
			 op += 3;
			 cnt += 3;
		} else if ((index(legal, *sp) != 0) || (index(hexdigits, *sp) != 0)) {
			*op++ = *sp;
			*op = '\0';
			cnt++;
		} else {
			(void)sprintf(op,"%c%02X", quotechar, *sp);
			op += 3;
			cnt += 3;
		}
		sp++;
	}
	return callerbuf;
}

/*
 * fname_decode()
 *
 *   Args:
 *	quotechar	lead-in character used to quote illegal characters as hex digits
 *	s		string to decode
 *	callerbuf	buffer to house result
 *	bufsz		size of callerbuf
 */
char *
fname_decode (char quotechar, char *s, char *callerbuf, int bufsz)
{
	char *sp, *op;
	int k,calc,cnt = 0;
	static char hexdigits[] = "0123456789ABCDEF";

	sp = s;
	op = callerbuf;
	*op = '\0';
	calc = 0;

	while (*sp) {
		/* Do we have room for one more character? */
		if ((bufsz - cnt) <= 2) return callerbuf;
		if (*sp == quotechar) {
			sp++;
			for (k=0; k < 16; ++k) if (*sp == hexdigits[k]) break;
			if (k >= 16) return callerbuf;	/* impossible, so bail */
			calc = k << 4; 
			sp++;
			for (k=0; k < 16; ++k) if (*sp == hexdigits[k]) break;
			if (k >= 16) return callerbuf;	/* impossible, so bail */
			calc += k; 
			sp++;
			*op++ = calc;
			*op = '\0';
		} else {
			*op++ = *sp++;
			*op = '\0';
		}
		cnt++;
	}
	return callerbuf;
}

#ifndef PREFIXES_IN_USE
/*ARGSUSED*/
#endif
const char *
fqname (const char *basename, int whichprefix, int buffnum)
{
#ifndef PREFIXES_IN_USE
	return basename;
#else
	if (!basename || whichprefix < 0 || whichprefix >= PREFIX_COUNT)
		return basename;
	if (!fqn_prefix[whichprefix])
		return basename;
	if (buffnum < 0 || buffnum >= FQN_NUMBUF) {
		impossible("Invalid fqn_filename_buffer specified: %d",
								buffnum);
		buffnum = 0;
	}
	if (strlen(fqn_prefix[whichprefix]) + strlen(basename) >=
						    FQN_MAX_FILENAME) {
		impossible("fqname too long: %s + %s", fqn_prefix[whichprefix],
						basename);
		return basename;	/* XXX */
	}
	strcpy(fqn_filename_buffer[buffnum], fqn_prefix[whichprefix]);
	return strcat(fqn_filename_buffer[buffnum], basename);
#endif
}

/* reasonbuf must be at least BUFSZ, supplied by caller */
/*ARGSUSED*/
int 
validate_prefix_locations (char *reasonbuf)
{
#if defined(NOCWD_ASSUMPTIONS)
	FILE *fp;
	const char *filename;
	int prefcnt, failcount = 0;
	char panicbuf1[BUFSZ], panicbuf2[BUFSZ], *details;

	if (reasonbuf) reasonbuf[0] = '\0';
	for (prefcnt = 1; prefcnt < PREFIX_COUNT; prefcnt++) {
		/* don't test writing to configdir or datadir; they're readonly */
		if (prefcnt == CONFIGPREFIX || prefcnt == DATAPREFIX) continue;
		filename = fqname("validate", prefcnt, 3);
		if ((fp = fopen(filename, "w"))) {
			fclose(fp);
			(void) unlink(filename);
		} else {
			if (reasonbuf) {
				if (failcount) strcat(reasonbuf,", ");
				strcat(reasonbuf, fqn_prefix_names[prefcnt]);
			}
			/* the paniclog entry gets the value of errno as well */
			sprintf(panicbuf1,"Invalid %s", fqn_prefix_names[prefcnt]);
#if defined (NHSTDC) && !defined(NOTSTDC)
			if (!(details = strerror(errno)))
#endif
			details = "";
			sprintf(panicbuf2,"\"%s\", (%d) %s",
				fqn_prefix[prefcnt], errno, details);
			paniclog(panicbuf1, panicbuf2);
			failcount++;
		}	
	}
	if (failcount)
		return 0;
	else
#endif
	return 1;
}

/*
 * When file areas are in use, fopen_datafile_area is used instead.
 */

#ifndef FILE_AREA
/* fopen a file, with OS-dependent bells and whistles */
/* NOTE: a simpler version of this routine also exists in util/dlb_main.c */
FILE *
fopen_datafile(filename, mode, prefix)
const char *filename, *mode;
int prefix;
{
	FILE *fp;

	filename = fqname(filename, prefix, prefix == TROUBLEPREFIX ? 3 : 0);
	fp = fopen(filename, mode);
	return fp;
}
#endif	/* FILE_AREA */

/* ----------  BEGIN LEVEL FILE HANDLING ----------- */

/* Construct a file name for a level-type file, which is of the form
 * something.level (with any old level stripped off).
 * This assumes there is space on the end of 'file' to append
 * a two digit number.  This is true for 'level'
 * but be careful if you use it for other things -dgk
 */
void 
set_levelfile_name (char *file, int lev)
{
	char *tf;

	tf = rindex(file, '.');
	if (!tf) tf = eos(file);
	sprintf(tf, ".%d", lev);
	return;
}

int 
create_levelfile (int lev, char errbuf[])
{
	int fd;
#ifndef FILE_AREAS
	const char *fq_lock;
#endif

	if (errbuf) *errbuf = '\0';
	set_levelfile_name(lock, lev);
#ifndef FILE_AREAS
	fq_lock = fqname(lock, LEVELPREFIX, 0);
#endif

#if defined(MICRO) || defined(WIN32)
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
# ifdef FILE_AREAS
	fd = open_area(FILE_AREA_LEVL, lock,
	  O_WRONLY |O_CREAT | O_TRUNC | O_BINARY, FCMASK);
# else
#  ifdef HOLD_LOCKFILE_OPEN
	if (lev == 0)
	    fd = open_levelfile_exclusively(fq_lock, lev,
			    O_WRONLY |O_CREAT | O_TRUNC | O_BINARY);
	else
#  endif
	fd = open(fq_lock, O_WRONLY |O_CREAT | O_TRUNC | O_BINARY, FCMASK);
# endif
#else	/* MICRO */
# ifdef FILE_AREAS
	fd = creat_area(FILE_AREA_LEVL, lock, FCMASK);
# else
# ifdef MAC
	fd = maccreat(fq_lock, LEVL_TYPE);
# else
	fd = creat(fq_lock, FCMASK);
# endif
# endif	/* FILE_AREAS */
#endif /* MICRO || WIN32 */

	if (fd >= 0)
	    level_info[lev].flags |= LFILE_EXISTS;
	else if (errbuf)        /* failure explanation */
	    sprintf(errbuf,
		    "Cannot create file \"%s\" for level %d (errno %d).",
		    lock, lev, errno);

	return fd;
}


int 
open_levelfile (int lev, char errbuf[])
{
	int fd;
#ifndef FILE_AREAS
	const char *fq_lock;
#endif

	if (errbuf) *errbuf = '\0';
	set_levelfile_name(lock, lev);
#ifndef FILE_AREAS
	fq_lock = fqname(lock, LEVELPREFIX, 0);
#endif
#ifdef FILE_AREAS
	fd = open_area(FILE_AREA_LEVL, lock, O_RDONLY | O_BINARY, 0);
#else
# ifdef MAC
	fd = macopen(fq_lock, O_RDONLY | O_BINARY, LEVL_TYPE);
# else
#  ifdef HOLD_LOCKFILE_OPEN
	if (lev == 0)
		fd = open_levelfile_exclusively(fq_lock, lev, O_RDONLY | O_BINARY );
	else
#  endif
	fd = open(fq_lock, O_RDONLY | O_BINARY, 0);
# endif
#endif	/* FILE_AREAS */

	/* for failure, return an explanation that our caller can use;
	   settle for `lock' instead of `fq_lock' because the latter
	   might end up being too big for nethack's BUFSZ */
	if (fd < 0 && errbuf)
	    sprintf(errbuf,
		    "Cannot open file \"%s\" for level %d (errno %d).",
		    lock, lev, errno);

	return fd;
}


void 
delete_levelfile (int lev)
{
	/*
	 * Level 0 might be created by port specific code that doesn't
	 * call create_levfile(), so always assume that it exists.
	 */
	if (lev == 0 || (level_info[lev].flags & LFILE_EXISTS)) {
		set_levelfile_name(lock, lev);
#ifdef FILE_AREAS
		(void) remove_area(FILE_AREA_LEVL, lock, 0);
#else
# ifdef HOLD_LOCKFILE_OPEN
		if (lev == 0) really_close();
# endif
		(void) unlink(fqname(lock, LEVELPREFIX, 0));
#endif
		level_info[lev].flags &= ~LFILE_EXISTS;
	}
}


void 
clearlocks (void)
{
/* [Tom] Watcom.....
#if !defined(PC_LOCKING)
	eraseall(levels, alllevels);
	if (ramdisk)
		eraseall(permbones, alllevels);
#else
	register int x;

# ifdef UNIX
	(void) signal(SIGHUP, SIG_IGN);
# endif */
	/* can't access maxledgerno() before dungeons are created -dlc */
	int x;
	for (x = (n_dgns ? maxledgerno() : 0); x >= 0; x--)
		delete_levelfile(x);	/* not all levels need be present */
/* #endif*/
}

#ifdef HOLD_LOCKFILE_OPEN
static int
open_levelfile_exclusively(name, lev, oflag)
const char *name;
int lev, oflag;
{
	int reslt, fd;
	if (!lftrack.init) {
		lftrack.init = 1;
		lftrack.fd = -1;
	}
	if (lftrack.fd >= 0) {
		/* check for compatible access */
		if (lftrack.oflag == oflag) {
			fd = lftrack.fd;
			reslt = lseek(fd, 0L, SEEK_SET);
			if (reslt == -1L)
			    panic("open_levelfile_exclusively: lseek failed %d", errno);
			lftrack.nethack_thinks_it_is_open = TRUE;
		} else {
			really_close();
			fd = sopen(name, oflag,SH_DENYRW, FCMASK);
			lftrack.fd = fd;
			lftrack.oflag = oflag;
			lftrack.nethack_thinks_it_is_open = TRUE;
		}
	} else {
			fd = sopen(name, oflag,SH_DENYRW, FCMASK);
			lftrack.fd = fd;
			lftrack.oflag = oflag;
			if (fd >= 0)
			    lftrack.nethack_thinks_it_is_open = TRUE;
	}
	return fd;
}

void 
really_close (void)
{
	int fd = lftrack.fd;
	lftrack.nethack_thinks_it_is_open = FALSE;
	lftrack.fd = -1;
	lftrack.oflag = 0;
	(void)_close(fd);
	return;
}

int 
close (int fd)
{
 	if (lftrack.fd == fd) {
		really_close();	/* close it, but reopen it to hold it */
		fd = open_levelfile(0, NULL);
		lftrack.nethack_thinks_it_is_open = FALSE;
		return 0;
	}
	return _close(fd);
}
#endif
	
/* ----------  END LEVEL FILE HANDLING ----------- */


/* ----------  BEGIN BONES FILE HANDLING ----------- */

/* set up "file" to be file name for retrieving bones, and return a
 * bonesid to be read/written in the bones file.
 */
static char *
set_bonesfile_name(file, lev)
char *file;
d_level *lev;
{
	s_level *sptr;
	char *dptr;

	sprintf(file, "bon%c%s", dungeons[lev->dnum].boneid,
			In_quest(lev) ? urole.filecode : "0");
	dptr = eos(file);
	if ((sptr = Is_special(lev)) != 0)
	    sprintf(dptr, ".%c", sptr->boneid);
	else
	    sprintf(dptr, ".%d", lev->dlevel);
	return(dptr-2);
}

/* set up temporary file name for writing bones, to avoid another game's
 * trying to read from an uncompleted bones file.  we want an uncontentious
 * name, so use one in the namespace reserved for this game's level files.
 * (we are not reading or writing level files while writing bones files, so
 * the same array may be used instead of copying.)
 */
static char *
set_bonestemp_name()
{
	char *tf;

	tf = rindex(lock, '.');
	if (!tf) tf = eos(lock);
	sprintf(tf, ".bn");
	return lock;
}

int
create_bonesfile(lev, bonesid, errbuf)
d_level *lev;
char **bonesid;
char errbuf[];
{
	const char *file;
	int fd;

	if (errbuf) *errbuf = '\0';
	*bonesid = set_bonesfile_name(bones, lev);
	file = set_bonestemp_name();
#ifndef FILE_AREAS
	file = fqname(file, BONESPREFIX, 0);
#endif

#if defined(MICRO) || defined(WIN32)
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
# ifdef FILE_AREAS
	fd = open_area(FILE_AREA_BONES, file,
	  O_WRONLY |O_CREAT | O_TRUNC | O_BINARY, FCMASK);
# else
	fd = open(file, O_WRONLY |O_CREAT | O_TRUNC | O_BINARY, FCMASK);
# endif
#else
# ifdef FILE_AREAS
	fd = creat_area(FILE_AREA_BONES, file, FCMASK);
# else
# ifdef MAC
	fd = maccreat(file, BONE_TYPE);
# else
	fd = creat(file, FCMASK);
# endif
# endif	/* FILE_AREAS */
#endif
	if (fd < 0 && errbuf) /* failure explanation */
	    sprintf(errbuf,
		    "Cannot create bones \"%s\", id %s (errno %d).",
		    lock, *bonesid, errno);

	return fd;
}

/* move completed bones file to proper name */
void
commit_bonesfile(lev)
d_level *lev;
{
	const char *fq_bones, *tempname;
	int ret;

	(void) set_bonesfile_name(bones, lev);
#ifndef FILE_AREAS
	fq_bones = fqname(bones, BONESPREFIX, 0);
#endif
	tempname = set_bonestemp_name();
#ifndef FILE_AREAS
	tempname = fqname(tempname, BONESPREFIX, 1);
#endif

#ifdef FILE_AREAS
	ret = rename_area(FILE_AREA_BONES, tempname, bones);
#else
# if (defined(SYSV) && !defined(SVR4)) || defined(GENIX)
	/* old SYSVs don't have rename.  Some SVR3's may, but since they
	 * also have link/unlink, it doesn't matter. :-)
	 */
	(void) unlink(fq_bones);
	ret = link(tempname, fq_bones);
	ret += unlink(tempname);
# else
	ret = rename(tempname, fq_bones);
# endif
#endif	/* FILE_AREAS */
#ifdef WIZARD
	if (wizard && ret != 0)
#ifdef FILE_AREAS
		pline("couldn't rename %s to %s.", tempname, bones);
#else
		pline("couldn't rename %s to %s.", tempname, fq_bones);
#endif
#endif
}


int
open_bonesfile(lev, bonesid)
d_level *lev;
char **bonesid;
{
	const char *fq_bones;
	int fd;

	*bonesid = set_bonesfile_name(bones, lev);
#ifdef FILE_AREAS
	uncompress_area(FILE_AREA_BONES, bones);  /* no effect if nonexistent */
	fd = open_area(FILE_AREA_BONES, bones, O_RDONLY | O_BINARY, 0);
#else
	fq_bones = fqname(bones, BONESPREFIX, 0);
	uncompress(fq_bones);	/* no effect if nonexistent */
# ifdef MAC
	fd = macopen(fq_bones, O_RDONLY | O_BINARY, BONE_TYPE);
# else
	fd = open(fq_bones, O_RDONLY | O_BINARY, 0);
# endif
#endif	/* FILE_AREAS */
	return fd;
}


int
delete_bonesfile(lev)
d_level *lev;
{
	(void) set_bonesfile_name(bones, lev);
#ifdef FILE_AREAS
	return !(remove_area(FILE_AREA_BONES, bones) < 0);
#else
	return !(unlink(fqname(bones, BONESPREFIX, 0)) < 0);
#endif
}


/* assume we're compressing the recently read or created bonesfile, so the
 * file name is already set properly */
void 
compress_bonesfile (void)
{
#ifdef FILE_AREAS
	compress_area(FILE_AREA_BONES, bones);
#else
	compress(fqname(bones, BONESPREFIX, 0));
#endif
}

/* ----------  END BONES FILE HANDLING ----------- */


/* ----------  BEGIN SAVE FILE HANDLING ----------- */

/* set savefile name in OS-dependent manner from pre-existing plname,
 * avoiding troublesome characters */
void 
set_savefile_name (void)
{
#if defined(WIN32)
	char fnamebuf[BUFSZ], encodedfnamebuf[BUFSZ];
#endif
# if defined(MICRO)
	strcpy(SAVEF, SAVEP);
	{
		int i = strlen(SAVEP);
		(void)strncat(SAVEF, plname, 8);
		regularize(SAVEF+i);
	}
	strcat(SAVEF, ".sav");
# else
#  ifndef FILE_AREAS
#  if defined(WIN32)
	/* Obtain the name of the logged on user and incorporate
	 * it into the name. */
	sprintf(fnamebuf, "%s-%s", get_username(0), plname);
	(void)fname_encode("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-.",
				'%', fnamebuf, encodedfnamebuf, BUFSZ);
	sprintf(SAVEF, "%s.NetHack-saved-game", encodedfnamebuf);
#  else
	sprintf(SAVEF, "save/%d%s", (int)getuid(), plname);
	regularize(SAVEF+5);	/* avoid . or / in name */
#  endif /* WIN32 */
#  else
	sprintf(SAVEF, "%d%s", (int)getuid(), plname);
	regularize(SAVEF);      /* avoid . or / in name */
#  endif
# endif	/* MICRO */
}

#ifdef INSURANCE
void 
save_savefile_name (int fd)
{
	(void) write(fd, (void *) SAVEF, sizeof(SAVEF));
}
#endif


#if defined(WIZARD) && !defined(MICRO)
/* change pre-existing savefile name to indicate an error savefile */
void 
set_error_savefile (void)
{
      {
	char *semi_colon = rindex(SAVEF, ';');
	if (semi_colon) *semi_colon = '\0';
      }
	strcat(SAVEF, ".e;1");
# ifdef MAC
	strcat(SAVEF, "-e");
# else
	strcat(SAVEF, ".e");
# endif
}
#endif


/* create save file, overwriting one if it already exists */
int 
create_savefile (void)
{
#ifndef FILE_AREAS
	const char *fq_save;
#endif
	int fd;

#ifdef FILE_AREAS
# ifdef MICRO
	fd = open_area(FILE_AREA_SAVE, SAVEF,
	  O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
# else
	fd = creat_area(FILE_AREA_SAVE, SAVEF, FCMASK);
# endif
#else	/* FILE_AREAS */
	fq_save = fqname(SAVEF, SAVEPREFIX, 0);
# if defined(MICRO) || defined(WIN32)
	fd = open(fq_save, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
# else
#  ifdef MAC
	fd = maccreat(fq_save, SAVE_TYPE);
#  else
	fd = creat(fq_save, FCMASK);
#  endif
# endif /* MICRO */
#endif  /* FILE_AREAS */

	return fd;
}


/* open savefile for reading */
int 
open_savefile (void)
{
	int fd;

#ifdef FILE_AREAS
	fd = open_area(FILE_AREA_SAVE, SAVEF, O_RDONLY | O_BINARY, 0);
#else
	const char *fq_save;
	fq_save = fqname(SAVEF, SAVEPREFIX, 0);
# ifdef MAC
	fd = macopen(fq_save, O_RDONLY | O_BINARY, SAVE_TYPE);
# else
	fd = open(fq_save, O_RDONLY | O_BINARY, 0);
# endif
#endif	/* FILE_AREAS */
	return fd;
}


/* delete savefile */
int 
delete_savefile (void)
{
/*WAC OK...this is probably a contreversial addition.  It's an option tho*/
#ifdef KEEP_SAVE
/* Wizard mode already has prompt*/
	if (flags.keep_savefile && !wizard) {
		return 1; /*Should this return 0?*/
	}
#endif

#ifdef FILE_AREAS
	(void) remove_area(FILE_AREA_SAVE, SAVEF);
#else
	(void) unlink(fqname(SAVEF, SAVEPREFIX, 0));
#endif
	return 0;	/* for restore_saved_game() (ex-xxxmain.c) test */
}


/* try to open up a save file and prepare to restore it */
int 
restore_saved_game (void)
{
#ifndef FILE_AREAS
	const char *fq_save;
#endif
	int fd;

	set_savefile_name();
#ifndef FILE_AREAS
	fq_save = fqname(SAVEF, SAVEPREFIX, 0);

	uncompress(fq_save);
#else
	uncompress_area(FILE_AREA_SAVE, SAVEF);
#endif
	if ((fd = open_savefile()) < 0) return fd;

#ifndef FILE_AREAS
	if (!uptodate(fd, fq_save)) {
#else
	if (!uptodate(fd, SAVEF)) {
#endif
	    (void) close(fd),  fd = -1;
	    (void) delete_savefile();
	}
	return fd;
}

void
free_saved_games(saved)
char** saved;
{
    if ( saved ) {
	int i=0;
	while (saved[i]) free((void *)saved[i++]);
	free((void *)saved);
    }
}


/* ----------  END SAVE FILE HANDLING ----------- */


/* ----------  BEGIN FILE COMPRESSION HANDLING ----------- */

#ifdef COMPRESS

static void
redirect(filearea, filename, mode, stream, uncomp)
const char *filearea, *filename, *mode;
FILE *stream;
boolean uncomp;
{
#ifndef FILE_AREAS
	if (freopen(filename, mode, stream) == NULL) {
#else
	if (freopen_area(filearea, filename, mode, stream) == NULL) {
#endif
		(void) fprintf(stderr, "redirect of %s for %scompress failed\n",
			filename, uncomp ? "un" : "");
		terminate(EXIT_FAILURE);
	}
}

/*
 * using system() is simpler, but opens up security holes and causes
 * problems on at least Interactive UNIX 3.0.1 (SVR3.2), where any
 * setuid is renounced by /bin/sh, so the files cannot be accessed.
 *
 * cf. child() in unixunix.c.
 */
static void
docompress_file(filearea, filename, uncomp)
const char *filearea, *filename;
boolean uncomp;
{
	char cfn[80];
	FILE *cf;
	const char *args[10];
# ifdef COMPRESS_OPTIONS
	char opts[80];
# endif
	int i = 0;
	int f;
# ifdef TTY_GRAPHICS
	boolean istty = !strncmpi(windowprocs.name, "tty", 3);
# endif

	strcpy(cfn, filename);
# ifdef COMPRESS_EXTENSION
	strcat(cfn, COMPRESS_EXTENSION);
# endif
	/* when compressing, we know the file exists */
	if (uncomp) {
	    if ((cf = fopen_datafile_area(filearea, cfn, RDBMODE, FALSE)) ==
	      NULL)
		    return;
	    (void) fclose(cf);
	}

	args[0] = COMPRESS;
	if (uncomp) args[++i] = "-d";	/* uncompress */
# ifdef COMPRESS_OPTIONS
	{
	    /* we can't guarantee there's only one additional option, sigh */
	    char *opt;
	    boolean inword = FALSE;

	    strcpy(opts, COMPRESS_OPTIONS);
	    opt = opts;
	    while (*opt) {
		if ((*opt == ' ') || (*opt == '\t')) {
		    if (inword) {
			*opt = '\0';
			inword = FALSE;
		    }
		} else if (!inword) {
		    args[++i] = opt;
		    inword = TRUE;
		}
		opt++;
	    }
	}
# endif
	args[++i] = NULL;

# ifdef TTY_GRAPHICS
	/* If we don't do this and we are right after a y/n question *and*
	 * there is an error message from the compression, the 'y' or 'n' can
	 * end up being displayed after the error message.
	 */
	if (istty)
	    mark_synch();
# endif
	f = fork();
	if (f == 0) {	/* child */
# ifdef TTY_GRAPHICS
		/* any error messages from the compression must come out after
		 * the first line, because the more() to let the user read
		 * them will have to clear the first line.  This should be
		 * invisible if there are no error messages.
		 */
		if (istty)
		    raw_print("");
# endif
		/* run compressor without privileges, in case other programs
		 * have surprises along the line of gzip once taking filenames
		 * in GZIP.
		 */
		/* assume all compressors will compress stdin to stdout
		 * without explicit filenames.  this is true of at least
		 * compress and gzip, those mentioned in config.h.
		 */
		if (uncomp) {
			redirect(filearea, cfn, RDBMODE, stdin, uncomp);
			redirect(filearea, filename, WRBMODE, stdout, uncomp);
		} else {
			redirect(filearea, filename, RDBMODE, stdin, uncomp);
			redirect(filearea, cfn, WRBMODE, stdout, uncomp);
		}
		(void) setgid(getgid());
		(void) setuid(getuid());
		(void) execv(args[0], (char *const *) args);
		perror(NULL);
		(void) fprintf(stderr, "Exec to %scompress %s failed.\n",
			uncomp ? "un" : "", filename);
		terminate(EXIT_FAILURE);
	} else if (f == -1) {
		perror(NULL);
		pline("Fork to %scompress %s failed.",
			uncomp ? "un" : "", filename);
		return;
	}
# ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
	(void) signal(SIGQUIT, SIG_IGN);
# endif
	(void) wait((int *)&i);
# ifndef NO_SIGNAL
	(void) signal(SIGINT, (SIG_RET_TYPE) done1);
# endif
# ifdef WIZARD
	if (wizard) (void) signal(SIGQUIT, SIG_DFL);
# endif
	if (i == 0) {
	    /* (un)compress succeeded: remove file left behind */
	    if (uncomp)
#ifndef FILE_AREAS
		(void) unlink(cfn);
#else
		(void) remove_area(filearea, cfn);
#endif
	    else
#ifndef FILE_AREAS
		(void) unlink(filename);
#else
		(void) remove_area(filearea, filename);
#endif
	} else {
	    /* (un)compress failed; remove the new, bad file */
	    if (uncomp) {
		raw_printf("Unable to uncompress %s", filename);
		(void) unlink(filename);
	    } else {
		/* no message needed for compress case; life will go on */
		(void) unlink(cfn);
	    }
#ifdef TTY_GRAPHICS
	    /* Give them a chance to read any error messages from the
	     * compression--these would go to stdout or stderr and would get
	     * overwritten only in tty mode.  It's still ugly, since the
	     * messages are being written on top of the screen, but at least
	     * the user can read them.
	     */
	    if (istty)
	    {
		clear_nhwindow(WIN_MESSAGE);
		more();
		/* No way to know if this is feasible */
		/* doredraw(); */
	    }
#endif
	}
}
#endif	/* COMPRESS */

/* compress file */
void
compress_area(filearea, filename)
const char *filearea, *filename;
{
#ifndef COMPRESS
#if defined(MAC_MPW) || defined(__MWERKS__)
# pragma unused(filename)
#endif
#else
	docompress_file(filearea, filename, FALSE);
#endif
}


/* uncompress file if it exists */
void
uncompress_area(filearea, filename)
const char *filearea, *filename;
{
#ifndef COMPRESS
#if defined(MAC_MPW) || defined(__MWERKS__)
# pragma unused(filename)
#endif
#else
	docompress_file(filearea, filename, TRUE);
#endif
}

/* ----------  END FILE COMPRESSION HANDLING ----------- */


/*
 * When file areas are in use, (un)lock_file_area are used instead.
 */

#ifndef FILE_AREAS

/* ----------  BEGIN FILE LOCKING HANDLING ----------- */

static int nesting = 0;

#ifdef NO_FILE_LINKS	/* implies UNIX */
static int lockfd;	/* for lock_file() to pass to unlock_file() */
#endif

#define HUP	if (!program_state.done_hup)

static char *
make_lockname(filename, lockname)
const char *filename;
char *lockname;
{
#if defined(MAC_MPW) || defined(__MWERKS__)
# pragma unused(filename,lockname)
	return NULL;
#else
# if defined(UNIX) || defined(WIN32)
#  ifdef NO_FILE_LINKS
	strcpy(lockname, LOCKDIR);
	strcat(lockname, "/");
	strcat(lockname, filename);
#  else
	strcpy(lockname, filename);
#  endif
	strcat(lockname, "_lock");
	return lockname;
# else
	lockname[0] = '\0';
	return NULL;
# endif  /* UNIX || WIN32 */
#endif
}


/* lock a file */
boolean
lock_file(filename, whichprefix, retryct)
const char *filename;
int whichprefix;
int retryct;
{
#if defined(MAC_MPW) || defined(__MWERKS__)
# pragma unused(filename, retryct)
#endif
	char locknambuf[BUFSZ];
	const char *lockname;

	nesting++;
	if (nesting > 1) {
	    impossible("TRIED TO NEST LOCKS");
	    return TRUE;
	}

	lockname = make_lockname(filename, locknambuf);
	filename = fqname(filename, whichprefix, 0);
#ifndef NO_FILE_LINKS	/* LOCKDIR should be subsumed by LOCKPREFIX */
	lockname = fqname(lockname, LOCKPREFIX, 2);
#endif

#ifdef UNIX
# ifdef NO_FILE_LINKS
	while ((lockfd = open(lockname, O_RDWR|O_CREAT|O_EXCL, 0666)) == -1) {
# else
	while (link(filename, lockname) == -1) {
# endif
	    register int errnosv = errno;

	    switch (errnosv) {	/* George Barbanis */
	    case EEXIST:
		if (retryct--) {
		    HUP raw_printf(
			    "Waiting for access to %s.  (%d retries left).",
			    filename, retryct);
# if defined(SYSV) || defined(ULTRIX)
		    (void)
# endif
			sleep(1);
		} else {
		    HUP (void) raw_print("I give up.  Sorry.");
		    HUP raw_printf("Perhaps there is an old %s around?",
					lockname);
		    nesting--;
		    return FALSE;
		}

		break;
	    case ENOENT:
		HUP raw_printf("Can't find file %s to lock!", filename);
		nesting--;
		return FALSE;
	    case EACCES:
		HUP raw_printf("No write permission to lock %s!", filename);
		nesting--;
		return FALSE;
	    default:
		HUP perror(lockname);
		HUP raw_printf(
			     "Cannot lock %s for unknown reason (%d).",
			       filename, errnosv);
		nesting--;
		return FALSE;
	    }

	}
#endif  // UNIX

#ifdef WIN32
#define OPENFAILURE(fd) (fd < 0)

    lockptr = -1;
    while (--retryct && OPENFAILURE(lockptr)) {
	lockptr = sopen(lockname, O_RDWR|O_CREAT, SH_DENYRW, S_IWRITE);

	if (OPENFAILURE(lockptr)) {
	    raw_printf("Waiting for access to %s.  (%d retries left).",
			filename, retryct);
	    Delay(50);
	}
    }
    if (!retryct) {
	raw_printf("I give up.  Sorry.");
	nesting--;
	return FALSE;
    }
#endif // WIN32
	return TRUE;
}


/* unlock file, which must be currently locked by lock_file */
void
unlock_file(filename)
const char *filename;
{
	char locknambuf[BUFSZ];
	const char *lockname;

	if (nesting == 1) {
		lockname = make_lockname(filename, locknambuf);
#ifndef NO_FILE_LINKS	/* LOCKDIR should be subsumed by LOCKPREFIX */
		lockname = fqname(lockname, LOCKPREFIX, 2);
#endif

#ifdef UNIX
		if (unlink(lockname) < 0)
			HUP raw_printf("Can't unlink %s.", lockname);
# ifdef NO_FILE_LINKS
		(void) close(lockfd);
# endif

#endif  /* UNIX */

#ifdef WIN32
		if (lockptr) Close(lockptr);
		DeleteFile(lockname);
		lockptr = 0;
#endif /* WIN32 */
	}

	nesting--;
}

/* ----------  END FILE LOCKING HANDLING ----------- */

#endif	/* FILE_AREAS */

/* ----------  BEGIN CONFIG FILE HANDLING ----------- */

const char *configfile =
                        NH_CONFIG_FILE;
/* WAC This stuff is in filename.h now
#ifdef UNIX
			".nethackrc";
#else
# ifdef MAC
			"NetHack Defaults";
#  ifdef WIN32
			"defaults.nh";
#  else
			"NetHack.cnf";
#  endif
# endif
#endif
*/


#define fopenp fopen

static FILE *
fopen_config_file(filename)
const char *filename;
{
	FILE *fp;
#ifdef UNIX
	char	tmp_config[BUFSZ];
	char *envp;
#endif

	/* "filename" is an environment variable, so it should hang around */
	/* if set, it is expected to be a full path name (if relevant) */
	if (filename) {
#ifdef UNIX
		if (access(filename, 4) == -1) {
			/* 4 is R_OK on newer systems */
			/* nasty sneaky attempt to read file through
			 * NetHack's setuid permissions -- this is the only
			 * place a file name may be wholly under the player's
			 * control
			 */
			raw_printf("Access to %s denied (%d).",
					filename, errno);
			wait_synch();
			/* fall through to standard names */
		} else
#endif
		if ((fp = fopenp(filename, "r")) != NULL) {
		    configfile = filename;
		    return(fp);
#ifdef UNIX
		} else {
		    /* access() above probably caught most problems for UNIX */
		    raw_printf("Couldn't open requested config file %s (%d).",
					filename, errno);
		    wait_synch();
		    /* fall through to standard names */
#endif
		}
	}

#if defined(MICRO) || defined(MAC) || defined(WIN32)
	if ((fp = fopenp(fqname(configfile, CONFIGPREFIX, 0), "r"))
								!= NULL)
		return(fp);
#else
	/* constructed full path names don't need fqname() */
	envp = nh_getenv("HOME");
	if (!envp)
		strcpy(tmp_config, configfile);
	else
		sprintf(tmp_config, "%s/%s", envp, configfile);
	if ((fp = fopenp(tmp_config, "r")) != NULL)
		return(fp);
# if defined(__APPLE__)
	/* try an alternative */
	if (envp) {
		sprintf(tmp_config, "%s/%s", envp, "Library/Preferences/NetHack Defaults");
		if ((fp = fopenp(tmp_config, "r")) != NULL)
			return(fp);
		sprintf(tmp_config, "%s/%s", envp, "Library/Preferences/NetHack Defaults.txt");
		if ((fp = fopenp(tmp_config, "r")) != NULL)
			return(fp);
	}
# endif
	if (errno != ENOENT) {
	    char *details;

	    /* e.g., problems when setuid NetHack can't search home
	     * directory restricted to user */

#if defined (NHSTDC) && !defined(NOTSTDC)
	    if ((details = strerror(errno)) == 0)
#endif
		details = "";
	    raw_printf("Couldn't open default config file %s %s(%d).",
			tmp_config, details, errno);
	    wait_synch();
	}
	else if (!strncmp(windowprocs.name, "proxy/", 6)) {
	    fp = fopenp("/etc/slashem/proxy.slashemrc", "r");
	    if (fp != NULL)
		return(fp);
	    else if (errno != ENOENT) {
		raw_printf("Couldn't open /etc/slashem/proxy.slashemrc (%d).",
				errno);
		wait_synch();
	    }
	}
#endif
	return NULL;

}


/*
 * Retrieve a list of integers from a file into a uchar array.
 *
 * NOTE: zeros are inserted unless modlist is TRUE, in which case the list
 *  location is unchanged.  Callers must handle zeros if modlist is FALSE.
 */
static int
get_uchars(fp, buf, bufp, list, modlist, size, name)
    FILE *fp;		/* input file pointer */
    char *buf;		/* read buffer, must be of size BUFSZ */
    char *bufp;		/* current pointer */
    uchar *list;	/* return list */
    boolean modlist;	/* TRUE: list is being modified in place */
    int  size;		/* return list size */
    const char *name;		/* name of option for error message */
{
    unsigned int num = 0;
    int count = 0;
    boolean havenum = FALSE;

    while (1) {
	switch(*bufp) {
	    case ' ':  case '\0':
	    case '\t': case '\n':
		if (havenum) {
		    /* if modifying in place, don't insert zeros */
		    if (num || !modlist) list[count] = num;
		    count++;
		    num = 0;
		    havenum = FALSE;
		}
		if (count == size || !*bufp) return count;
		bufp++;
		break;

	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	    case '8': case '9':
		havenum = TRUE;
		num = num*10 + (*bufp-'0');
		bufp++;
		break;

	    case '\\':
		if (fp == NULL)
		    goto gi_error;
		do  {
		    if (!fgets(buf, BUFSZ, fp)) goto gi_error;
		} while (buf[0] == '#');
		bufp = buf;
		break;

	    default:
gi_error:
		raw_printf("Syntax error in %s", name);
		wait_synch();
		return count;
	}
    }
    /*NOTREACHED*/
}

#ifdef NOCWD_ASSUMPTIONS
static void
adjust_prefix(bufp, prefixid)
char *bufp;
int prefixid;
{
	char *ptr;

	if (!bufp) return;
	/* Backward compatibility, ignore trailing ;n */ 
	if ((ptr = index(bufp, ';')) != 0) *ptr = '\0';
	if (strlen(bufp) > 0) {
		fqn_prefix[prefixid] = (char *)alloc(strlen(bufp)+2);
		strcpy(fqn_prefix[prefixid], bufp);
		append_slash(fqn_prefix[prefixid]);
	}
}
#endif

#define match_varname(INP,NAM,LEN) match_optname(INP, NAM, LEN, TRUE)

/*ARGSUSED*/
int
parse_config_line(fp, buf, tmp_ramdisk, tmp_levels)
FILE		*fp;
char		*buf;
char		*tmp_ramdisk;
char		*tmp_levels;
{
#if defined(MAC_MPW) || defined(__MWERKS__)
# pragma unused(tmp_ramdisk,tmp_levels)
#endif
	char		*bufp, *altp;
	uchar   translate[MAXPCHARS];
	int   len;

	if (*buf == '#')
		return 1;

	/* remove trailing whitespace */
	bufp = eos(buf);
	while (--bufp > buf && isspace((int)*bufp))
		continue;

	if (bufp <= buf)
		return 1;		/* skip all-blank lines */
	else
		*(bufp + 1) = '\0';	/* terminate line */

	/* find the '=' or ':' */
	bufp = index(buf, '=');
	altp = index(buf, ':');
	if (!bufp || (altp && altp < bufp)) bufp = altp;
	if (!bufp) return 0;

	/* skip  whitespace between '=' and value */
	do { ++bufp; } while (isspace((int)*bufp));

	/* Go through possible variables */
	/* some of these (at least LEVELS and SAVE) should now set the
	 * appropriate fqn_prefix[] rather than specialized variables
	 */
	if (match_varname(buf, "OPTIONS", 4)) {
		parseoptions(bufp, TRUE, TRUE);
		if (plname[0])		/* If a name was given */
			plnamesuffix();	/* set the character class */
	} else if (match_varname(buf, "TILESETS", 7)) {
		parsetileset(bufp);
	} else if (match_varname(buf, "AUTOPICKUP_EXCEPTION", 5)) {
		add_autopickup_exception(bufp);
#ifdef NOCWD_ASSUMPTIONS
	} else if (match_varname(buf, "HACKDIR", 4)) {
		adjust_prefix(bufp, HACKPREFIX);
	} else if (match_varname(buf, "LEVELDIR", 4) ||
		   match_varname(buf, "LEVELS", 4)) {
		adjust_prefix(bufp, LEVELPREFIX);
	} else if (match_varname(buf, "SAVEDIR", 4)) {
		adjust_prefix(bufp, SAVEPREFIX);
	} else if (match_varname(buf, "BONESDIR", 5)) {
		adjust_prefix(bufp, BONESPREFIX);
	} else if (match_varname(buf, "DATADIR", 4)) {
		adjust_prefix(bufp, DATAPREFIX);
	} else if (match_varname(buf, "SCOREDIR", 4)) {
		adjust_prefix(bufp, SCOREPREFIX);
	} else if (match_varname(buf, "LOCKDIR", 4)) {
		adjust_prefix(bufp, LOCKPREFIX);
	} else if (match_varname(buf, "CONFIGDIR", 4)) {
		adjust_prefix(bufp, CONFIGPREFIX);
	} else if (match_varname(buf, "TROUBLEDIR", 4)) {
		adjust_prefix(bufp, TROUBLEPREFIX);
#else /*NOCWD_ASSUMPTIONS*/
# ifdef MICRO
	} else if (match_varname(buf, "HACKDIR", 4)) {
		(void) strncpy(hackdir, bufp, PATHLEN-1);
	} else if (match_varname(buf, "LEVELS", 4)) {
		(void) strncpy(tmp_levels, bufp, PATHLEN-1);

	} else if (match_varname(buf, "SAVE", 4)) {
		char *ptr;
		if ((ptr = index(bufp, ';')) != 0) {
			*ptr = '\0';
		}

		(void) strncpy(SAVEP, bufp, SAVESIZE-1);
		append_slash(SAVEP);
# endif /* MICRO */
#endif /*NOCWD_ASSUMPTIONS*/

	} else if (match_varname(buf, "NAME", 4)) {
	    (void) strncpy(plname, bufp, PL_NSIZ-1);
	    plnamesuffix();
	} else if (match_varname(buf, "MSGTYPE", 7)) {
	    char pattern[256];
	    char msgtype[11];
	    if (sscanf(bufp, "%10s \"%255[^\"]\"", msgtype, pattern) == 2) {
		int typ = MSGTYP_NORMAL;
		if (!strcasecmp("norep", msgtype)) typ = MSGTYP_NOREP;
		else if (!strcasecmp("hide", msgtype)) typ = MSGTYP_NOSHOW;
		else if (!strcasecmp("noshow", msgtype)) typ = MSGTYP_NOSHOW;
		else if (!strcasecmp("more", msgtype)) typ = MSGTYP_STOP;
		else if (!strcasecmp("stop", msgtype)) typ = MSGTYP_STOP;
		if (typ != MSGTYP_NORMAL) {
		    msgpline_add(typ, pattern);
		}
	    }
	} else if (match_varname(buf, "ROLE", 4) ||
		   match_varname(buf, "CHARACTER", 4)) {
	    if ((len = str2role(bufp)) >= 0)
	    	flags.initrole = len;
	} else if (match_varname(buf, "DOGNAME", 3)) {
	    (void) strncpy(dogname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "CATNAME", 3)) {
	    (void) strncpy(catname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "WOLFNAME", 3)) {
            (void) strncpy(wolfname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "GHOULNAME", 3)) {
            (void) strncpy(ghoulname, bufp, PL_PSIZ-1);
#if 0
	} else if (match_varname(buf, "BATNAME", 3)) {
            (void) strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "SNAKENAME", 3)) {
            (void) strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "RATNAME", 3)) {
            (void) strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "BADGERNAME", 3)) {
            (void) strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "REDDRAGONNAME", 3)) {
            (void) strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "WHITEDRAGONNAME", 3)) {
            (void) strncpy(batname, bufp, PL_PSIZ-1);
#endif

	} else if (match_varname(buf, "BOULDER", 3)) {
	    (void) get_uchars(fp, buf, bufp, &iflags.bouldersym, TRUE,
			      1, "BOULDER");
	} else if (match_varname(buf, "MENUCOLOR", 9)) {
#ifdef MENU_COLOR
	   add_menu_coloring(bufp);
#endif
	} else if (match_varname(buf, "GRAPHICS", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXPCHARS, "GRAPHICS");
	    assign_graphics(translate, len, MAXPCHARS, 0);
	} else if (match_varname(buf, "DUNGEON", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXDCHARS, "DUNGEON");
	    assign_graphics(translate, len, MAXDCHARS, 0);
	} else if (match_varname(buf, "TRAPS", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXTCHARS, "TRAPS");
	    assign_graphics(translate, len, MAXTCHARS, MAXDCHARS);
	} else if (match_varname(buf, "EFFECTS", 4)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXECHARS, "EFFECTS");
	    assign_graphics(translate, len, MAXECHARS, MAXDCHARS+MAXTCHARS);
#ifdef USER_DUNGEONCOLOR
	} else if (match_varname(buf, "DUNGEONCOLOR", 10)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXDCHARS, "DUNGEONCOLOR");
	    assign_colors(translate, len, MAXDCHARS, 0);
	} else if (match_varname(buf, "TRAPCOLORS", 7)) {
	    len = get_uchars(fp, buf, bufp, translate, FALSE,
			     MAXTCHARS, "TRAPCOLORS");
	    assign_colors(translate, len, MAXTCHARS, MAXDCHARS);
#endif

	} else if (match_varname(buf, "OBJECTS", 3)) {
	    /* oc_syms[0] is the RANDOM object, unused */
	    (void) get_uchars(fp, buf, bufp, &(oc_syms[1]), TRUE,
					MAXOCLASSES-1, "OBJECTS");
	} else if (match_varname(buf, "MONSTERS", 3)) {
	    /* monsyms[0] is unused */
	    (void) get_uchars(fp, buf, bufp, &(monsyms[1]), TRUE,
					MAXMCLASSES-1, "MONSTERS");
	} else if (match_varname(buf, "WARNINGS", 5)) {
	    (void) get_uchars(fp, buf, bufp, translate, TRUE,
					WARNCOUNT, "WARNINGS");
	    assign_warnings(translate);
#ifdef WIZARD
	} else if (match_varname(buf, "WIZKIT", 6)) {
	    (void) strncpy(wizkit, bufp, WIZKIT_MAX-1);
#endif
#ifdef USER_SOUNDS
	} else if (match_varname(buf, "SOUNDDIR", 8)) {
		sounddir = (char *)strdup(bufp);
	} else if (match_varname(buf, "SOUND", 5)) {
		add_sound_mapping(bufp);
#endif
#if defined(GL_GRAPHICS) || defined(SDL_GRAPHICS)
	} else if (match_varname(buf, "GL_OPTIONS", 10)) {
		Sdlgl_parse_options(bufp, TRUE, TRUE);
#endif
	} else
		return 0;
	return 1;
}

#ifdef USER_SOUNDS
boolean
can_read_file(filename)
const char *filename;
{
	return (access(filename, 4) == 0);
}
#endif /* USER_SOUNDS */

void
read_config_file(filename)
const char *filename;
{
#define tmp_levels	NULL
#define tmp_ramdisk	NULL

#if defined(MICRO) || defined(WIN32)
#undef tmp_levels
	char	tmp_levels[PATHLEN];
#endif
	char	buf[4*BUFSZ];
	FILE	*fp;
	int     i;
#ifdef PROXY_GRAPHICS
	int	found = FALSE;

	if (!(fp = fopen_config_file(filename)))
	    goto clnt_process;
	else
	    found = TRUE;
#else
	if (!(fp = fopen_config_file(filename))) goto post_process;
#endif

#if defined(MICRO) || defined(WIN32)
	tmp_levels[0] = 0;
#endif
	/* begin detection of duplicate configfile options */
	set_duplicate_opt_detection(1);

	while (fgets(buf, 4*BUFSZ, fp)) {
		if (!parse_config_line(fp, buf, tmp_ramdisk, tmp_levels)) {
			raw_printf("Bad option line:  \"%.50s\"", buf);
			wait_synch();
		}
	}
	(void) fclose(fp);
	
	/* turn off detection of duplicate configfile options */
	set_duplicate_opt_detection(0);

#ifdef PROXY_GRAPHICS
clnt_process:
	/*
	 * When acting as a proxy server, allow the client to provide
	 * its own config file which overrides values in our config file.
	 * Note: We don't want to warn of values being present in both
	 * files, but we do want to warn of duplicates within each file.
	 */
	if (!strncmp(windowprocs.name, "proxy/", 6) &&
		(fp = proxy_config_file_open())) {
	    found = TRUE;
	    set_duplicate_opt_detection(1);
	    while (fgets(buf, 4*BUFSZ, fp)) {
		if (match_varname(buf, "TILESETS", 7) ||
		   match_varname(buf, "HACKDIR", 4) ||
		   match_varname(buf, "LEVELDIR", 4) ||
		   match_varname(buf, "LEVELS", 4) ||
		   match_varname(buf, "SAVEDIR", 4) ||
		   match_varname(buf, "BONESDIR", 5) ||
		   match_varname(buf, "DATADIR", 4) ||
		   match_varname(buf, "SCOREDIR", 4) ||
		   match_varname(buf, "LOCKDIR", 4) ||
		   match_varname(buf, "CONFIGDIR", 4) ||
		   match_varname(buf, "TROUBLEDIR", 4) ||
		   match_varname(buf, "SOUNDDIR", 8) ||
		   match_varname(buf, "SOUND", 5)) {
		    /* Quietly ignore many commands. There's no sense in
		     * the client configuring these and some introduce
		     * potential security breachs.
		     */
		    continue;
		}
		if (!parse_config_line(fp, buf, tmp_ramdisk, tmp_levels)) {
		    pline("Bad option line:  \"%.50s\"", buf);
		    wait_synch();
		}
	    }
	    proxy_config_file_close(fp);
	    set_duplicate_opt_detection(0);
	}

	if (!found) goto post_process;
#endif

post_process:
	if (!no_tilesets) {
		for(i = 0; strlen(def_tilesets[i].name); i++) {
			strcpy(tilesets[i].name, def_tilesets[i].name);
			strcpy(tilesets[i].file, def_tilesets[i].file);
			tilesets[i].flags = def_tilesets[i].flags;
		}
		no_tilesets = i;
	}
	if (tileset[0] != '\0') {
		unsigned int len = strlen(tileset);
		for(i = 0; i < no_tilesets; i++)
			if (len == strlen(tilesets[i].name) &&
			    !strncmpi(tilesets[i].name, tileset, len))
				break;
		if (i == no_tilesets) {
			pline("Tileset %s not defined.", tileset);
			tileset[0] = '\0';
		}
		else
			strcpy(tileset, tilesets[i].name);
	}
	return;
}

#ifdef WIZARD
static FILE *
fopen_wizkit_file()
{
	FILE *fp;
#ifdef UNIX
	char	tmp_wizkit[BUFSZ];
#endif
	char *envp;

	envp = nh_getenv("WIZKIT");
	if (envp && *envp) (void) strncpy(wizkit, envp, WIZKIT_MAX - 1);
	if (!wizkit[0]) return NULL;

#ifdef UNIX
	if (access(wizkit, 4) == -1) {
		/* 4 is R_OK on newer systems */
		/* nasty sneaky attempt to read file through
		 * NetHack's setuid permissions -- this is a
		 * place a file name may be wholly under the player's
		 * control
		 */
		raw_printf("Access to %s denied (%d).",
				wizkit, errno);
		wait_synch();
		/* fall through to standard names */
	} else
#endif
	if ((fp = fopenp(wizkit, "r")) != NULL) {
	    return(fp);
#ifdef UNIX
	} else {
	    /* access() above probably caught most problems for UNIX */
	    raw_printf("Couldn't open requested config file %s (%d).",
				wizkit, errno);
	    wait_synch();
#endif
	}

#if defined(MICRO) || defined(MAC) || defined(WIN32)
	if ((fp = fopenp(fqname(wizkit, CONFIGPREFIX, 0), "r"))
								!= NULL)
		return(fp);
#else
	envp = nh_getenv("HOME");
	if (envp)
		sprintf(tmp_wizkit, "%s/%s", envp, wizkit);
	else 	strcpy(tmp_wizkit, wizkit);
	if ((fp = fopenp(tmp_wizkit, "r")) != NULL)
		return(fp);
	else if (errno != ENOENT) {
		/* e.g., problems when setuid NetHack can't search home
		 * directory restricted to user */
		raw_printf("Couldn't open default wizkit file %s (%d).",
					tmp_wizkit, errno);
		wait_synch();
	}
#endif
	return NULL;
}

void
read_wizkit()
{
	FILE *fp;
	char *ep, buf[BUFSZ];
	struct obj *otmp;
	boolean bad_items = FALSE, skip = FALSE;

	if (!wizard || !(fp = fopen_wizkit_file())) return;

	while (fgets(buf, (int)(sizeof buf), fp)) {
	    ep = index(buf, '\n');
	    if (skip) {	/* in case previous line was too long */
		if (ep) skip = FALSE; /* found newline; next line is normal */
	    } else {
		if (!ep) skip = TRUE; /* newline missing; discard next fgets */
		else *ep = '\0';		/* remove newline */

		if (buf[0]) {
			otmp = readobjnam(buf, NULL, FALSE);
			if (otmp) {
			    if (otmp != &zeroobj)
				otmp = addinv(otmp);
			} else {
			    /* .60 limits output line width to 79 chars */
			    raw_printf("Bad wizkit item: \"%.60s\"", buf);
			    bad_items = TRUE;
			}
		}
	    }
	}
	if (bad_items)
	    wait_synch();
	(void) fclose(fp);
	return;
}

#endif /*WIZARD*/

/* ----------  END CONFIG FILE HANDLING ----------- */

/* ----------  BEGIN SCOREBOARD CREATION ----------- */

/* verify that we can write to the scoreboard file; if not, try to create one */
void
check_recordfile(dir)
const char *dir;
{
#if defined(MAC_MPW) || defined(__MWERKS__)
# pragma unused(dir)
#endif
	int fd;
#ifndef FILE_AREAS
	const char *fq_record;
#endif

#ifdef UNIX
# ifdef FILE_AREAS
	fd = open_area(NH_RECORD_AREA, NH_RECORD, O_RDWR, 0);
# else
	fq_record = fqname(NH_RECORD, SCOREPREFIX, 0);
	fd = open(fq_record, O_RDWR, 0);
# endif

	if (fd >= 0) {
	    (void) close(fd);   /* NH_RECORD is accessible */
#  ifdef FILE_AREAS
	} else if ((fd = open_area(NH_RECORD_AREA, NH_RECORD, O_CREAT|O_RDWR, FCMASK)) >= 0) {
#  else
	} else if ((fd = open(fq_record, O_CREAT|O_RDWR, FCMASK)) >= 0) {
#  endif
	    (void) close(fd);   /* NH_RECORD newly created */
	} else {
# ifdef FILE_AREAS
	    raw_printf("Warning: cannot write scoreboard file %s", NH_RECORD);
# else
	    raw_printf("Warning: cannot write scoreboard file %s", fq_record);
# endif
	    wait_synch();
	}
#endif  /* !UNIX && !VMS */
#if defined(MICRO) || defined(WIN32)
	char tmp[PATHLEN];
	strcpy(tmp, NH_RECORD);
# ifndef FILE_AREAS
	fq_record = fqname(NH_RECORD, SCOREPREFIX, 0);
# endif

# ifdef FILE_AREAS
	if ((fd = open_area(NH_RECORD_AREA, tmp, O_RDWR)) < 0) {
# else
	if ((fd = open(fq_record, O_RDWR)) < 0) {
# endif
	    /* try to create empty record */
# if defined(FILE_AREAS)
	    if ((fd = open_area(NH_RECORD_AREA, tmp, O_CREAT|O_RDWR,
	      S_IREAD|S_IWRITE)) < 0) {
# else
	    if ((fd = open(fq_record, O_CREAT|O_RDWR, S_IREAD|S_IWRITE)) < 0) {
# endif
	raw_printf("Warning: cannot write record %s", tmp);
		wait_synch();
		} else          /* create succeeded */
		(void) close(fd);
	} else		/* open succeeded */
	    (void) close(fd);
#else /* MICRO || WIN32*/

# ifdef MAC
	/* Create the record file, if necessary */
	fq_record = fqname(NH_RECORD, SCOREPREFIX, 0);
	fd = macopen (fq_record, O_RDWR | O_CREAT, LOGF_TYPE);
	if (fd != -1) macclose (fd);

	/* Create the logfile, if necessary */
	fq_record = fqname(LOGFILE, SCOREPREFIX, 0);
	fd = macopen (fq_record, O_RDWR | O_CREAT, LOGF_TYPE);
	if (fd != -1) macclose (fd);
# endif /* MAC */

#endif /* MICRO || WIN32*/
}

/* ----------  END SCOREBOARD CREATION ----------- */

/* ----------  BEGIN PANIC/IMPOSSIBLE LOG ----------- */

/*ARGSUSED*/
void
paniclog(type, reason)
const char *type;	/* panic, impossible, trickery */
const char *reason;	/* explanation */
{
#ifdef PANICLOG
	FILE *lfile;
	char buf[BUFSZ];

	if (!program_state.in_paniclog) {
		program_state.in_paniclog = 1;
		lfile = fopen_datafile(PANICLOG, "a", TROUBLEPREFIX);
		if (lfile) {
		    (void) fprintf(lfile, "%s %08ld: %s %s\n",
				   version_string(buf), yyyymmdd((time_t)0L),
				   type, reason);
		    (void) fclose(lfile);
		}
		program_state.in_paniclog = 0;
	}
#endif /* PANICLOG */
	return;
}

/* ----------  END PANIC/IMPOSSIBLE LOG ----------- */

#ifdef SELF_RECOVER

/* ----------  BEGIN INTERNAL RECOVER ----------- */
boolean
recover_savefile()
{
	int gfd, lfd, sfd;
	int lev, savelev, hpid;
	xchar levc;
	struct version_info version_data;
	int processed[256];
	char savename[SAVESIZE], errbuf[BUFSZ];

	for (lev = 0; lev < 256; lev++)
		processed[lev] = 0;

	/* level 0 file contains:
	 *	pid of creating process (ignored here)
	 *	level number for current level of save file
	 *	name of save file nethack would have created
	 *	and game state
	 */
	gfd = open_levelfile(0, errbuf);
	if (gfd < 0) {
	    raw_printf("%s\n", errbuf);
	    return FALSE;
	}
	if (read(gfd, (void *) &hpid, sizeof hpid) != sizeof hpid) {
	    raw_printf(
"\nCheckpoint data incompletely written or subsequently clobbered. Recovery impossible.");
	    (void)close(gfd);
	    return FALSE;
	}
	if (read(gfd, (void *) &savelev, sizeof(savelev))
							!= sizeof(savelev)) {
	    raw_printf("\nCheckpointing was not in effect for %s -- recovery impossible.\n",
			lock);
	    (void)close(gfd);
	    return FALSE;
	}
	if ((read(gfd, (void *) savename, sizeof savename)
		!= sizeof savename) ||
	    (read(gfd, (void *) &version_data, sizeof version_data)
		!= sizeof version_data)) {
	    raw_printf("\nError reading %s -- can't recover.\n", lock);
	    (void)close(gfd);
	    return FALSE;
	}

	/* save file should contain:
	 *	version info
	 *	current level (including pets)
	 *	(non-level-based) game state
	 *	other levels
	 */
	set_savefile_name();
	sfd = create_savefile();
	if (sfd < 0) {
	    raw_printf("\nCannot recover savefile %s.\n", SAVEF);
	    (void)close(gfd);
	    return FALSE;
	}

	lfd = open_levelfile(savelev, errbuf);
	if (lfd < 0) {
	    raw_printf("\n%s\n", errbuf);
	    (void)close(gfd);
	    (void)close(sfd);
	    delete_savefile();
	    return FALSE;
	}

	if (write(sfd, (void *) &version_data, sizeof version_data)
		!= sizeof version_data) {
	    raw_printf("\nError writing %s; recovery failed.", SAVEF);
	    (void)close(gfd);
	    (void)close(sfd);
	    delete_savefile();
	    return FALSE;
	}

	if (!copy_bytes(lfd, sfd)) {
		(void) close(lfd);
		(void) close(sfd);
		delete_savefile();
		return FALSE;
	}
	(void)close(lfd);
	processed[savelev] = 1;

	if (!copy_bytes(gfd, sfd)) {
		(void) close(lfd);
		(void) close(sfd);
		delete_savefile();
		return FALSE;
	}
	(void)close(gfd);
	processed[0] = 1;

	for (lev = 1; lev < 256; lev++) {
		/* level numbers are kept in xchars in save.c, so the
		 * maximum level number (for the endlevel) must be < 256
		 */
		if (lev != savelev) {
			lfd = open_levelfile(lev, NULL);
			if (lfd >= 0) {
				/* any or all of these may not exist */
				levc = (xchar) lev;
				write(sfd, (void *) &levc, sizeof(levc));
				if (!copy_bytes(lfd, sfd)) {
					(void) close(lfd);
					(void) close(sfd);
					delete_savefile();
					return FALSE;
				}
				(void)close(lfd);
				processed[lev] = 1;
			}
		}
	}
	(void)close(sfd);

#ifdef HOLD_LOCKFILE_OPEN
	really_close();
#endif
	/*
	 * We have a successful savefile!
	 * Only now do we erase the level files.
	 */
	for (lev = 0; lev < 256; lev++) {
		if (processed[lev]) {
			const char *fq_lock;
			set_levelfile_name(lock, lev);
			fq_lock = fqname(lock, LEVELPREFIX, 3);
			(void) unlink(fq_lock);
		}
	}
	return TRUE;
}

boolean
copy_bytes(ifd, ofd)
int ifd, ofd;
{
	char buf[BUFSIZ];
	int nfrom, nto;

	do {
		nfrom = read(ifd, buf, BUFSIZ);
		nto = write(ofd, buf, nfrom);
		if (nto != nfrom) return FALSE;
	} while (nfrom == BUFSIZ);
	return TRUE;
}

/* ----------  END INTERNAL RECOVER ----------- */
#endif /*SELF_RECOVER*/

/*files.c*/
