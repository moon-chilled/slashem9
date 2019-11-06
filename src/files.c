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

#ifdef PROXY_GRAPHICS
#include "winproxy.h" /* proxy_config_open() */
#endif

#include <ctype.h>

#if (!defined(MAC) && !defined(O_WRONLY)) || defined(USE_FCNTL)
#include <fcntl.h>
#endif

#include <errno.h>
// MSC 6.0 defines errno quite differently
#if defined(_MSC_VER) && (_MSC_VER >= 600)
# define SKIP_ERRNO
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

#ifdef WIN32
#include <sys/stat.h>
#endif
#ifndef O_BINARY	/* used for micros, no-op for others */
# define O_BINARY 0
#endif

#include <strings.h>

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
#  define SAVESIZE        FILENAMELEN
# endif
#endif

char SAVEF[SAVESIZE];	/* holds relative path of save file from playground */

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

#define WIZKIT_MAX 128
static char wizkit[WIZKIT_MAX];
static FILE *fopen_wizkit_file(void);

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
#if !defined(USE_FCNTL)
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
char *fname_encode(const char *legal, char quotechar, char *s, char *callerbuf, int bufsz) {
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
			sprintf(op, "%c%02X", quotechar, *sp);
			op += 3;
			cnt += 3;
		} else if ((index(legal, *sp) != 0) || (index(hexdigits, *sp) != 0)) {
			*op++ = *sp;
			*op = '\0';
			cnt++;
		} else {
			sprintf(op,"%c%02X", quotechar, *sp);
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
char *fname_decode(char quotechar, char *s, char *callerbuf, int bufsz) {
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

/* ----------  BEGIN LEVEL FILE HANDLING ----------- */

/* Construct a file name for a level-type file, which is of the form
 * something.level (with any old level stripped off).
 * This assumes there is space on the end of 'file' to append
 * a two digit number.  This is true for 'level'
 * but be careful if you use it for other things -dgk
 */
void set_levelfile_name(char *file, int lev) {
	char *tf;

	tf = rindex(file, '.');
	if (!tf) tf = eos(file);
	sprintf(tf, ".%d", lev);
	return;
}

int create_levelfile(int lev, char errbuf[]) {
	int fd;

	if (errbuf) *errbuf = '\0';
	set_levelfile_name(lock, lev);

#ifdef WIN32
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
#  ifdef HOLD_LOCKFILE_OPEN
	if (lev == 0)
		fd = open_levelfile_exclusively(lock, lev, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY);
	else
#  endif
		fd = open(lock, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, FCMASK);
#else	// WIN32
	fd = creat(lock, FCMASK);
#endif /* WIN32 */

	if (fd >= 0)
		level_info[lev].flags |= LFILE_EXISTS;
	else if (errbuf)        /* failure explanation */
		sprintf(errbuf,
		        "Cannot create file \"%s\" for level %d (errno %d).",
		        lock, lev, errno);

	return fd;
}


int open_levelfile(int lev, char errbuf[]) {
	int fd;

	if (errbuf) *errbuf = '\0';
	set_levelfile_name(lock, lev);
# ifdef HOLD_LOCKFILE_OPEN
	if (lev == 0)
		fd = open_levelfile_exclusively(lock, lev, O_RDONLY | O_BINARY );
	else
# endif
		fd = open(lock, O_RDONLY | O_BINARY, 0);

	// for failure, return an explanation that our caller can use
	if (fd < 0 && errbuf)
		sprintf(errbuf,
		        "Cannot open file \"%s\" for level %d (errno %d).",
		        lock, lev, errno);

	return fd;
}


void delete_levelfile(int lev) {
	/*
	 * Level 0 might be created by port specific code that doesn't
	 * call create_levfile(), so always assume that it exists.
	 */
	if (lev == 0 || (level_info[lev].flags & LFILE_EXISTS)) {
		set_levelfile_name(lock, lev);
# ifdef HOLD_LOCKFILE_OPEN
		if (lev == 0) really_close();
# endif
		unlink(lock);
		level_info[lev].flags &= ~LFILE_EXISTS;
	}
}


void clearlocks(void) {
	/* [Tom] Watcom.....
	#if !defined(PC_LOCKING)
		eraseall(levels, alllevels);
		if (ramdisk)
			eraseall(permbones, alllevels);
	#else
		int x;

	# ifdef UNIX
		signal(SIGHUP, SIG_IGN);
	# endif */
	/* can't access maxledgerno() before dungeons are created -dlc */
	int x;
	for (x = (n_dgns ? maxledgerno() : 0); x >= 0; x--)
		delete_levelfile(x);	/* not all levels need be present */
	/* #endif*/
}

#ifdef HOLD_LOCKFILE_OPEN
static int open_levelfile_exclusively(const char *name, int lev, int oflag) {
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
			lftrack.nethack_thinks_it_is_open = true;
		} else {
			really_close();
			fd = sopen(name, oflag,SH_DENYRW, FCMASK);
			lftrack.fd = fd;
			lftrack.oflag = oflag;
			lftrack.nethack_thinks_it_is_open = true;
		}
	} else {
		fd = sopen(name, oflag,SH_DENYRW, FCMASK);
		lftrack.fd = fd;
		lftrack.oflag = oflag;
		if (fd >= 0)
			lftrack.nethack_thinks_it_is_open = true;
	}
	return fd;
}

void really_close(void) {
	int fd = lftrack.fd;
	lftrack.nethack_thinks_it_is_open = false;
	lftrack.fd = -1;
	lftrack.oflag = 0;
	_close(fd);
	return;
}

int close(int fd) {
	if (lftrack.fd == fd) {
		really_close();	/* close it, but reopen it to hold it */
		fd = open_levelfile(0, NULL);
		lftrack.nethack_thinks_it_is_open = false;
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
static char *set_bonesfile_name(char *file, d_level *lev) {
	s_level *sptr;
	char *dptr;

	sprintf(file, "bon%c%s", dungeons[lev->dnum].boneid,
	        In_quest(lev) ? urole.filecode : "0");
	dptr = eos(file);
	if ((sptr = Is_special(lev)) != 0)
		sprintf(dptr, ".%c", sptr->boneid);
	else
		sprintf(dptr, ".%d", lev->dlevel);
	return dptr-2;
}

/* set up temporary file name for writing bones, to avoid another game's
 * trying to read from an uncompleted bones file.  we want an uncontentious
 * name, so use one in the namespace reserved for this game's level files.
 * (we are not reading or writing level files while writing bones files, so
 * the same array may be used instead of copying.)
 */
static char *set_bonestemp_name(void) {
	char *tf;

	tf = rindex(lock, '.');
	if (!tf) tf = eos(lock);
	sprintf(tf, ".bn");
	return lock;
}

int create_bonesfile(d_level *lev, char **bonesid, char errbuf[]) {
	const char *file;
	int fd;

	if (errbuf) *errbuf = '\0';
	*bonesid = set_bonesfile_name(bones, lev);
	file = set_bonestemp_name();

#ifdef WIN32
	/* Use O_TRUNC to force the file to be shortened if it already
	 * exists and is currently longer.
	 */
	fd = open(file, O_WRONLY |O_CREAT | O_TRUNC | O_BINARY, FCMASK);
#else
	fd = creat(file, FCMASK);
#endif
	if (fd < 0 && errbuf) /* failure explanation */
		sprintf(errbuf,
		        "Cannot create bones \"%s\", id %s (errno %d).",
		        lock, *bonesid, errno);

	return fd;
}

/* move completed bones file to proper name */
void commit_bonesfile(d_level *lev) {
	const char *tempname;
	int ret;

	set_bonesfile_name(bones, lev);
	tempname = set_bonestemp_name();

# if (defined(SYSV) && !defined(SVR4)) || defined(GENIX)
	/* old SYSVs don't have rename.  Some SVR3's may, but since they
	 * also have link/unlink, it doesn't matter. :-)
	 */
	unlink(bones);
	ret = link(tempname, bones);
	ret += unlink(tempname);
# else
	ret = rename(tempname, bones);
# endif
	if (wizard && ret != 0)
		pline("couldn't rename %s to %s.", tempname, bones);
}


int open_bonesfile(d_level *lev, char **bonesid) {
	int fd;

	*bonesid = set_bonesfile_name(bones, lev);
	fd = open(bones, O_RDONLY | O_BINARY, 0);
	return fd;
}


boolean delete_bonesfile(d_level *lev) {
	set_bonesfile_name(bones, lev);
	return !(unlink(bones) < 0);
}
/* ----------  END BONES FILE HANDLING ----------- */


/* ----------  BEGIN SAVE FILE HANDLING ----------- */

/* set savefile name in OS-dependent manner from pre-existing plname,
 * avoiding troublesome characters */
void set_savefile_name(void) {
#if defined(WIN32)
	char fnamebuf[BUFSZ], encodedfnamebuf[BUFSZ];
#endif

#if defined(WIN32)
	/* Obtain the name of the logged on user and incorporate
	 * it into the name. */
	sprintf(fnamebuf, "%s-%s", get_username(0), plname);
	fname_encode("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_-.",
	             '%', fnamebuf, encodedfnamebuf, BUFSZ);
	sprintf(SAVEF, "%s.NetHack-saved-game", encodedfnamebuf);
#else
	sprintf(SAVEF, "save/%d%s", (int)getuid(), plname);
	regularize(SAVEF+5);	/* avoid . or / in name */
#endif /* WIN32 */
}

#ifdef INSURANCE
void save_savefile_name(int fd) {
	write(fd, (void *) SAVEF, sizeof(SAVEF));
}
#endif


/* change pre-existing savefile name to indicate an error savefile */
void set_error_savefile(void) {
	char *semi_colon = rindex(SAVEF, ';');
	if (semi_colon) *semi_colon = '\0';
	strcat(SAVEF, ".e;1");
# ifdef MAC
	strcat(SAVEF, "-e");
# else
	strcat(SAVEF, ".e");
# endif
}


/* create save file, overwriting one if it already exists */
int create_savefile(void) {
	int fd;

#ifdef WIN32
	fd = open(SAVEF, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, FCMASK);
#else
	fd = creat(SAVEF, FCMASK);
#endif // WIN32

	return fd;
}


/* open savefile for reading */
int open_savefile(void) {
	return open(SAVEF, O_RDONLY | O_BINARY, 0);
}


/* delete savefile */
int delete_savefile(void) {
	/*WAC OK...this is probably a contreversial addition.  It's an option tho*/
#ifdef KEEP_SAVE
	/* Wizard mode already has prompt*/
	if (flags.keep_savefile && !wizard) {
		return 1; /*Should this return 0?*/
	}
#endif

	unlink(SAVEF);
	return 0;	/* for restore_saved_game() (ex-xxxmain.c) test */
}


/* try to open up a save file and prepare to restore it */
int restore_saved_game(void) {
	int fd;

	set_savefile_name();
	if ((fd = open_savefile()) < 0) return fd;

	if (!uptodate(fd, SAVEF)) {
		close(fd);
		fd = -1;
		delete_savefile();
	}
	return fd;
}

void free_saved_games(char** saved) {
	if (saved) {
		int i = 0;
		while (saved[i]) free(saved[i++]);
		free(saved);
	}
}


/* ----------  END SAVE FILE HANDLING ----------- */



/* ----------  BEGIN FILE LOCKING HANDLING ----------- */

static int nesting = 0;

#if defined(NO_FILE_LINKS) || defined(USE_FCNTL) // implies UNIX
static int lockfd;	// for lock_file() to pass to unlock_file()
#endif

#ifdef USE_FCNTL
struct flock sflock; // for unlocking, sae as above
#endif

#define HUP	if (!program_state.done_hup)


#if !defined(USE_FCNTL)
static char *make_lockname(const char *filename, char *lockname) {
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
}
#endif // !USE_FCNTL


/* lock a file */
boolean lock_file(const char *filename, int whichprefix, int retryct) {
#ifndef USE_FCNTL
	char locknambuf[BUFSZ];
	const char *lockname;
#endif

	nesting++;
	if (nesting > 1) {
		impossible("TRIED TO NEST LOCKS");
		return true;
	}

#ifndef USE_FCNTL
	lockname = make_lockname(filename, locknambuf);
#endif

#ifdef USE_FCNTL
	lockfd = open(filename,O_RDWR);
	if (lockfd == -1) {
		HUP raw_printf("Cannot open file %s. This is a program bug.",
		               filename);
	}
	sflock.l_type = F_WRLCK;
	sflock.l_whence = SEEK_SET;
	sflock.l_start = 0;
	sflock.l_len = 0;
#endif

#ifdef UNIX
# ifdef USE_FCNTL
	while (fcntl(lockfd,F_SETLK,&sflock) == -1) {
# else
#  ifdef NO_FILE_LINKS
	while ((lockfd = open(lockname, O_RDWR|O_CREAT|O_EXCL, 0666)) == -1) {
#  else
	while (link(filename, lockname) == -1) {
#  endif
# endif

# ifdef USE_FCNTL
		if (retryct--) {
			HUP raw_printf(
			        "Waiting for release of fcntl lock on %s. (%d retries left).",
			        filename, retryct);
			sleep(1);
		} else {
			HUP (void) raw_print("I give up.  Sorry.");
			HUP raw_printf("Some other process has an unnatural grip on %s.",
			               filename);
			nesting--;
			return false;
		}
# else

		int errnosv = errno;

		switch (errnosv) {	/* George Barbanis */
		case EEXIST:
			if (retryct--) {
				HUP raw_printf(
				        "Waiting for access to %s.  (%d retries left).",
				        filename, retryct);
				sleep(1);
			} else {
				HUP (void) raw_print("I give up.  Sorry.");
				HUP raw_printf("Perhaps there is an old %s around?",
				               lockname);
				nesting--;
				return false;
			}

			break;
		case ENOENT:
			HUP raw_printf("Can't find file %s to lock!", filename);
			nesting--;
			return false;
		case EACCES:
			HUP raw_printf("No write permission to lock %s!", filename);
			nesting--;
			return false;
		default:
			HUP perror(lockname);
			HUP raw_printf(
			        "Cannot lock %s for unknown reason (%d).",
			        filename, errnosv);
			nesting--;
			return false;
		}
# endif // USE_FCNTL

	}
#endif  // UNIX

#if defined(WIN32) && !defined(USE_FCNTL)
# define OPENFAILURE(fd) (fd < 0)

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
		return false;
	}
#endif // WIN32 && !USE_FCNTL
	return true;
}


/* unlock file, which must be currently locked by lock_file */
void unlock_file(const char *filename) {
#ifndef USE_FCNTL
	char locknambuf[BUFSZ];
	const char *lockname;
#endif

	if (nesting == 1) {
#ifdef USE_FCNTL
		sflock.l_type = F_UNLCK;
		if (fcntl(lockfd,F_SETLK,&sflock) == -1) {
			HUP raw_printf("Can't remove fcntl lock on %s.", filename);
		}
		close(lockfd);
#else

		lockname = make_lockname(filename, locknambuf);

#ifdef UNIX
		if (unlink(lockname) < 0)
			HUP raw_printf("Can't unlink %s.", lockname);
# ifdef NO_FILE_LINKS
		close(lockfd);
# endif

#endif  /* UNIX */

#ifdef WIN32
		if (lockptr) Close(lockptr);
		DeleteFile(lockname);
		lockptr = 0;
#endif // WIN32
#endif // USE_FCNTL
	}

	nesting--;
}

/* ----------  END FILE LOCKING HANDLING ----------- */


/* ----------  BEGIN CONFIG FILE HANDLING ----------- */

const char *configfile = NH_CONFIG_FILE;

static FILE *fopen_config_file(const char *filename) {
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
			if ((fp = fopen(filename, "r")) != NULL) {
				configfile = filename;
				return fp;
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

#if defined(MAC) || defined(WIN32)
	if ((fp = fopen(configfile, "r"))
	                != NULL)
		return fp;
#else
	envp = nh_getenv("HOME");
	if (!envp)
		strcpy(tmp_config, configfile);
	else
		sprintf(tmp_config, "%s/%s", envp, configfile);
	if ((fp = fopen(tmp_config, "r")) != NULL)
		return fp;
# if defined(__APPLE__)
	/* try an alternative */
	if (envp) {
		sprintf(tmp_config, "%s/%s", envp, "Library/Preferences/NetHack Defaults");
		if ((fp = fopen(tmp_config, "r")) != NULL)
			return fp;
		sprintf(tmp_config, "%s/%s", envp, "Library/Preferences/NetHack Defaults.txt");
		if ((fp = fopen(tmp_config, "r")) != NULL)
			return fp;
	}
# endif
	if (errno != ENOENT) {
		char *details;

		/* e.g., problems when setuid NetHack can't search home
		 * directory restricted to user */

		if ((details = strerror(errno)) == 0)
			details = "";
		raw_printf("Couldn't open default config file %s %s(%d).",
		           tmp_config, details, errno);
		wait_synch();
	} else if (!strncmp(windowprocs.name, "proxy/", 6)) {
		fp = fopen("/etc/slashem/proxy.slashemrc", "r");
		if (fp != NULL)
			return fp;
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
 * Retrieve a list of one-byte integers from a buffer into a uchar array.
 *
 * It can accept 3-digit decimal integers, characters (ascii), or
 * certain character symbols (see txt2key)
 *
 * return value of +x indicates x uchars read, -x indicates x-1 uchars read
 *
 * Note: there is no way to read value 0
 */
int
get_uchar_list(buf, list, size)
char *buf;		/* read buffer */
uchar *list;	/* return list */
int size;		/* return list size */
{
	int cnt = 0;
	char* next;
	uchar orig;
	uchar mkey;

	while (1) { /* one loop for each uchar */
		if (cnt == size) return cnt;

		/* take off leading whiltespace */
		while (isspace(*buf)) buf++;
		if (!*buf) return cnt;

		/* strip trailing whitespace / other uchars */
		next = buf;
		while(*next && !isspace(*next)) next++;
		orig = *next;
		*next = 0;

		/* interpret the character */
		mkey = (uchar)txt2key(buf);
		if (!mkey) {
			raw_printf("Invalid uchar %s on %i.", buf, cnt+1);
			return -(cnt+1);
		}
		list[cnt] = mkey;
		cnt++;

		/* prepare for the next uchar */
		*next = orig;
		buf = next;
	}
	/* NOT REACHED */
}

/*
 * Retrieve a list of integers from a file into a uchar array.
 *
 * NOTE: zeros are inserted unless modlist is true, in which case the list
 *  location is unchanged.  Callers must handle zeros if modlist is false.
 */
static int get_uchars(
        FILE *fp,		/* input file pointer */
        char *buf,		/* read buffer, must be of size BUFSZ */
        char *bufp,		/* current pointer */
        uchar *list,	/* return list */
        boolean modlist,	/* true: list is being modified in place */
        int  size,		/* return list size */
        const char *name	/* name of option for error message */) {

	int count = 0;

	int num_read;
	char* buf_end;
	boolean another;   /* expect another line? */

	while (1) { /* JDS: one loop for each line of input */
		if ((buf_end = index(bufp, '\\')) != 0) {
			*buf_end = 0;
			another = true;
		} else another = false;

		num_read = get_uchar_list(bufp, list, size-count);
		if (num_read < 0) {
			count -= num_read + 1;
gi_error:
			raw_printf("Syntax error in %s", name);
			wait_synch();
			return count;
		}
		count += num_read;
		list += num_read;

		if (count == size || !another) return count;

		if (fp == NULL)
			goto gi_error;
		do {
			if (!fgets(buf, BUFSZ, fp)) goto gi_error;
		} while (buf[0] == '#');
		bufp = buf;
	}
	/*NOTREACHED*/
}

#ifdef NOCWD_ASSUMPTIONS
static void adjust_prefix(char *bufp, int prefixid) {
	char *ptr;

	if (!bufp) return;
	/* Backward compatibility, ignore trailing ;n */
	if ((ptr = index(bufp, ';')) != 0) *ptr = '\0';
	if (strlen(bufp) > 0) {
		fqn_prefix[prefixid] = alloc(strlen(bufp)+2);
		strcpy(fqn_prefix[prefixid], bufp);
		append_slash(fqn_prefix[prefixid]);
	}
}
#endif

#define match_varname(INP,NAM,LEN) match_optname(INP, NAM, LEN, true)

/*ARGSUSED*/
int parse_config_line(FILE *fp, char *buf, char *tmp_ramdisk, char *tmp_levels) {
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
	do {
		++bufp;
	} while (isspace((int)*bufp));

	/* Go through possible variables */
	/* some of these (at least LEVELS and SAVE) should now set the
	 * appropriate fqn_prefix[] rather than specialized variables
	 */
	if (match_varname(buf, "OPTIONS", 4)) {
		parseoptions(bufp, true, true);
		if (plname[0])		/* If a name was given */
			plnamesuffix();	/* set the character class */
	} else if (match_varname(buf, "TILESETS", 7)) {
		parsetileset(bufp);
	} else if (match_varname(buf, "AUTOPICKUP_EXCEPTION", 5)) {
		add_autopickup_exception(bufp);
	} else if (match_varname(buf, "BINDINGS", 4)) {
		/* JDS: hmmm, should these be in NOCWD_ASSUMPTIONS? */
		parsebindings(bufp);
	} else if (match_varname(buf, "AUTOCOMPLETE", 5)) {
		parseautocomplete(bufp, true);
	} else if (match_varname(buf, "MAPPINGS", 3)) {
		parsemappings(bufp);
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
#endif /*NOCWD_ASSUMPTIONS*/

	} else if (match_varname(buf, "NAME", 4)) {
		strncpy(plname, bufp, PL_NSIZ-1);
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
		strncpy(dogname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "CATNAME", 3)) {
		strncpy(catname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "WOLFNAME", 3)) {
		strncpy(wolfname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "GHOULNAME", 3)) {
		strncpy(ghoulname, bufp, PL_PSIZ-1);
#if 0
	} else if (match_varname(buf, "BATNAME", 3)) {
		strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "SNAKENAME", 3)) {
		strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "RATNAME", 3)) {
		strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "BADGERNAME", 3)) {
		strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "REDDRAGONNAME", 3)) {
		strncpy(batname, bufp, PL_PSIZ-1);
	} else if (match_varname(buf, "WHITEDRAGONNAME", 3)) {
		strncpy(batname, bufp, PL_PSIZ-1);
#endif

	} else if (match_varname(buf, "MENUCOLOR", 9)) {
		add_menu_coloring(bufp);
	} else if (match_varname(buf, "STATUSCOLOR", 11)) {
		parse_status_color_options(bufp);
#ifdef USER_DUNGEONCOLOR
	} else if (match_varname(buf, "DUNGEONCOLOR", 10)) {
		len = get_uchars(fp, buf, bufp, translate, false,
		                 MAXDCHARS, "DUNGEONCOLOR");
		assign_colors(translate, len, MAXDCHARS, 0);
	} else if (match_varname(buf, "TRAPCOLORS", 7)) {
		len = get_uchars(fp, buf, bufp, translate, false,
		                 MAXTCHARS, "TRAPCOLORS");
		assign_colors(translate, len, MAXTCHARS, MAXDCHARS);
#endif

	} else if (match_varname(buf, "WIZKIT", 6)) {
		strncpy(wizkit, bufp, WIZKIT_MAX-1);
#ifdef USER_SOUNDS
	} else if (match_varname(buf, "SOUNDDIR", 8)) {
		sounddir = (char *)strdup(bufp);
	} else if (match_varname(buf, "SOUND", 5)) {
		add_sound_mapping(bufp);
#endif
	} else
		return 0;
	return 1;
}

#ifdef USER_SOUNDS
boolean can_read_file(const char *filename) {
	return access(filename, 4) == 0;
}
#endif /* USER_SOUNDS */

void read_config_file(const char *filename) {
#define tmp_levels	NULL
#define tmp_ramdisk	NULL

#ifdef WIN32
#undef tmp_levels
	char	tmp_levels[PATHLEN];
#endif
	char	buf[4*BUFSZ];
	FILE	*fp;
	int     i;
#ifdef PROXY_GRAPHICS
	int	found = false;

	if (!(fp = fopen_config_file(filename)))
		goto clnt_process;
	else
		found = true;
#else
	if (!(fp = fopen_config_file(filename))) goto post_process;
#endif

#ifdef WIN32
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
	fclose(fp);

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
		found = true;
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
		uint len = strlen(tileset);
		for(i = 0; i < no_tilesets; i++)
			if (len == strlen(tilesets[i].name) &&
			                !strncmpi(tilesets[i].name, tileset, len))
				break;
		if (i == no_tilesets) {
			pline("Tileset %s not defined.", tileset);
			tileset[0] = '\0';
		} else
			strcpy(tileset, tilesets[i].name);
	}
	return;
}

static FILE *fopen_wizkit_file(void) {
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
		if ((fp = fopen(wizkit, "r")) != NULL) {
			return fp;
#ifdef UNIX
		} else {
			/* access() above probably caught most problems for UNIX */
			raw_printf("Couldn't open requested config file %s (%d).",
			           wizkit, errno);
			wait_synch();
#endif
		}

#if defined(MAC) || defined(WIN32)
	if ((fp = fopen(wizkit, "r")) != NULL)
		return fp;
#else
	envp = nh_getenv("HOME");
	if (envp)
		sprintf(tmp_wizkit, "%s/%s", envp, wizkit);
	else 	strcpy(tmp_wizkit, wizkit);
	if ((fp = fopen(tmp_wizkit, "r")) != NULL)
		return fp;
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

void read_wizkit(void) {
	FILE *fp;
	char *ep, buf[BUFSZ];
	struct obj *otmp;
	boolean bad_items = false, skip = false;

	if (!wizard || !(fp = fopen_wizkit_file())) return;

	while (fgets(buf, (int)(sizeof buf), fp)) {
		ep = index(buf, '\n');
		if (skip) {	/* in case previous line was too long */
			if (ep) skip = false; /* found newline; next line is normal */
		} else {
			if (!ep) skip = true; /* newline missing; discard next fgets */
			else *ep = '\0';		/* remove newline */

			if (buf[0]) {
				otmp = readobjnam(buf, NULL, false);
				if (otmp) {
					if (otmp != &zeroobj)
						otmp = addinv(otmp);
				} else {
					/* .60 limits output line width to 79 chars */
					raw_printf("Bad wizkit item: \"%.60s\"", buf);
					bad_items = true;
				}
			}
		}
	}
	if (bad_items)
		wait_synch();
	fclose(fp);
	return;
}

/* ----------  END CONFIG FILE HANDLING ----------- */

/* ----------  BEGIN SCOREBOARD CREATION ----------- */

/* verify that we can write to the scoreboard file; if not, try to create one */
void check_recordfile(const char *dir) {
	int fd;

#ifdef UNIX
	fd = open(NH_RECORD, O_RDWR, 0);

	if (fd >= 0) {
		close(fd);   /* NH_RECORD is accessible */
	} else if ((fd = open(NH_RECORD, O_CREAT|O_RDWR, FCMASK)) >= 0) {
		close(fd);   /* NH_RECORD newly created */
	} else {
		raw_printf("Warning: cannot write scoreboard file %s", NH_RECORD);
		wait_synch();
	}
#endif  // !UNIX
#ifdef WIN32
	if ((fd = open(NH_RECORD, O_RDWR)) < 0) {
		/* try to create empty record */
		if ((fd = open(NH_RECORD, O_CREAT|O_RDWR, S_IREAD|S_IWRITE)) < 0) {
			raw_printf("Warning: cannot write record %s", NH_RECORD);
			wait_synch();
		} else          /* create succeeded */
			close(fd);
	} else		/* open succeeded */
		close(fd);
#endif /* WIN32*/
}

/* ----------  END SCOREBOARD CREATION ----------- */

/* ----------  BEGIN PANIC/IMPOSSIBLE LOG ----------- */

/*ARGSUSED*/
void paniclog(const char *type,	/* panic, impossible, trickery */
              const char *reason	/* explanation */) {

#ifdef PANICLOG
	FILE *lfile;

	if (!program_state.in_paniclog) {
		program_state.in_paniclog = 1;
		lfile = fopen(PANICLOG, "a");
		if (lfile) {
			fprintf(lfile, "%s %08ld: %s %s\n",
			        version_string_tmp(), yyyymmdd((time_t)0L),
			        type, reason);
			fclose(lfile);
		}
		program_state.in_paniclog = 0;
	}
#endif /* PANICLOG */
	return;
}

/* ----------  END PANIC/IMPOSSIBLE LOG ----------- */

#ifdef SELF_RECOVER

/* ----------  BEGIN INTERNAL RECOVER ----------- */
boolean recover_savefile(void) {
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
		return false;
	}
	if (read(gfd, (void *) &hpid, sizeof hpid) != sizeof hpid) {
		raw_printf(
		        "\nCheckpoint data incompletely written or subsequently clobbered. Recovery impossible.");
		close(gfd);
		return false;
	}
	if (read(gfd, (void *) &savelev, sizeof(savelev))
	                != sizeof(savelev)) {
		raw_printf("\nCheckpointing was not in effect for %s -- recovery impossible.\n",
		           lock);
		close(gfd);
		return false;
	}
	if ((read(gfd, (void *) savename, sizeof savename)
	                != sizeof savename) ||
	                (read(gfd, (void *) &version_data, sizeof version_data)
	                 != sizeof version_data)) {
		raw_printf("\nError reading %s -- can't recover.\n", lock);
		close(gfd);
		return false;
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
		close(gfd);
		return false;
	}

	lfd = open_levelfile(savelev, errbuf);
	if (lfd < 0) {
		raw_printf("\n%s\n", errbuf);
		close(gfd);
		close(sfd);
		delete_savefile();
		return false;
	}

	if (write(sfd, (void *) &version_data, sizeof version_data)
	                != sizeof version_data) {
		raw_printf("\nError writing %s; recovery failed.", SAVEF);
		close(gfd);
		close(sfd);
		delete_savefile();
		return false;
	}

	if (!copy_bytes(lfd, sfd)) {
		close(lfd);
		close(sfd);
		delete_savefile();
		return false;
	}
	close(lfd);
	processed[savelev] = 1;

	if (!copy_bytes(gfd, sfd)) {
		close(lfd);
		close(sfd);
		delete_savefile();
		return false;
	}
	close(gfd);
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
					close(lfd);
					close(sfd);
					delete_savefile();
					return false;
				}
				close(lfd);
				processed[lev] = 1;
			}
		}
	}
	close(sfd);

#ifdef HOLD_LOCKFILE_OPEN
	really_close();
#endif
	/*
	 * We have a successful savefile!
	 * Only now do we erase the level files.
	 */
	for (lev = 0; lev < 256; lev++) {
		if (processed[lev]) {
			set_levelfile_name(lock, lev);
			unlink(lock);
		}
	}
	return true;
}

boolean copy_bytes(int ifd, int ofd) {
	char buf[BUFSIZ];
	int nfrom, nto;

	do {
		nfrom = read(ifd, buf, BUFSIZ);
		nto = write(ofd, buf, nfrom);
		if (nto != nfrom) return false;
	} while (nfrom == BUFSIZ);
	return true;
}

/* ----------  END INTERNAL RECOVER ----------- */
#endif /*SELF_RECOVER*/

/*files.c*/
