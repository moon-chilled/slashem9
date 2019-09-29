/*	SCCS Id: @(#)unixmain.c	3.4	1997/01/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c - Unix NetHack */

#include "hack.h"
#include "dlb.h"

#include <sys/stat.h>
#include <signal.h>
#include <pwd.h>
#ifndef O_RDONLY
#include <fcntl.h>
#endif

#if !defined(_BULL_SOURCE) && !defined(__sgi) && !defined(_M_UNIX)
# if !defined(SUNOS4) && !(defined(ULTRIX) && defined(__GNUC__))
#  if defined(POSIX_TYPES) || defined(SVR4) || defined(HPUX)
extern struct passwd *getpwuid(uid_t);
#  else
extern struct passwd *getpwuid(int);
#  endif
# endif
#endif
extern struct passwd *getpwnam(const char *);
#ifdef CHDIR
static void chdirx(const char *, bool);
#endif /* CHDIR */
static bool whoami(void);
static void process_options(int, char **);

#ifdef _M_UNIX
extern void check_sco_console(void);
extern void init_sco_cons(void);
#endif
#ifdef __linux__
extern void check_linux_console(void);
extern void init_linux_cons(void);
#endif

#ifdef LINUX
extern void check_linux_console(void);
extern void init_linux_cons(void);
#endif

static void wd_message(void);
#ifdef WIZARD
static bool wiz_error_flag = false;
#endif

int main(int argc, char **argv) {
	int fd;
#ifdef CHDIR
	char *dir;
#endif
	bool exact_username;

	hname = argv[0];
	hackpid = getpid();
	umask(0777 & ~FCMASK);

	choose_windows(DEFAULT_WINDOW_SYS);

#ifdef CHDIR			/* otherwise no chdir() */
	/*
	 * See if we must change directory to the playground.
	 * (Perhaps hack runs suid and playground is inaccessible
	 *  for the player.)
	 * The environment variable HACKDIR is overridden by a
	 *  -d command line option (must be the first option given)
	 */
	dir = nh_getenv("NETHACKDIR");
	if (!dir) dir = nh_getenv("HACKDIR");
#endif
	if(argc > 1) {
#ifdef CHDIR
	    if (!strncmp(argv[1], "-d", 2) && argv[1][2] != 'e') {
		argc--;
		argv++;
		dir = argv[0]+2;
		if(*dir == '=' || *dir == ':') dir++;
		if(!*dir && argc > 1) {
			argc--;
			argv++;
			dir = argv[0];
		}
		if(!*dir)
		    error("Flag -d must be followed by a directory name.");
	    }
	    if (argc > 1)
#endif /* CHDIR */

	    /*
	     * Now we know the directory containing 'record' and
	     * may do a prscore().  Exclude `-style' - it's a Qt option.
	     */
	    if (!strncmp(argv[1], "-s", 2) && strncmp(argv[1], "-style", 6)) {
#ifdef CHDIR
		chdirx(dir,0);
#endif
		prscore(argc, argv);
		exit(EXIT_SUCCESS);
	    }
	}

	/*
	 * Change directories before we initialize the window system so
	 * we can find the tile file.
	 */
#ifdef CHDIR
	chdirx(dir,1);
#endif

#ifdef _M_UNIX
	check_sco_console();
#endif
#ifdef LINUX
	check_linux_console();
#endif
#ifdef PROXY_GRAPHICS
	/* Handle --proxy before options, if supported */
	if (argc > 1 && !strcmp(argv[1], "--proxy")) {
	    argv[1] = argv[0];
	    argc--;
	    argv++;
	    choose_windows("proxy");
	    lock_windows(true);         /* Can't be overridden from options */
	}
#endif
	initoptions();
	init_nhwindows(&argc,argv);
	exact_username = whoami();
#ifdef _M_UNIX
	init_sco_cons();
#endif
#ifdef LINUX
	init_linux_cons();
#endif
	/*
	 * It seems you really want to play.
	 */
	u.uhp = 1;	/* prevent RIP on early quits */
	signal(SIGHUP, (SIG_RET_TYPE) hangup);
#ifdef SIGXCPU
	signal(SIGXCPU, (SIG_RET_TYPE) hangup);
#endif
#ifdef SIGPIPE		/* eg., a lost proxy connection */
	signal(SIGPIPE, (SIG_RET_TYPE) hangup);
#endif

	process_options(argc, argv);	/* command line options */

#ifdef DEF_PAGER
	if(!(catmore = nh_getenv("HACKPAGER")) && !(catmore = nh_getenv("PAGER")))
		catmore = DEF_PAGER;
#endif
#ifdef MAIL
	getmailstatus();
#endif
#ifdef WIZARD
	if (wizard)
		strcpy(plname, "wizard");
	else
#endif
	if(!*plname || !strncmp(plname, "player", 4)
		    || !strncmp(plname, "games", 4)) {
		askname();
	} else if (exact_username) {
		/* guard against user names with hyphens in them */
		int len = strlen(plname);
		/* append the current role, if any, so that last dash is ours */
		if (++len < sizeof plname)
			strncat(strcat(plname, "-"),
				      pl_character, sizeof plname - len - 1);
	}
	plnamesuffix();		/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifdef WIZARD
	if(!wizard) {
#endif
		/*
		 * check for multiple games under the same name
		 * (if !locknum) or check max nr of players (otherwise)
		 */
		signal(SIGQUIT,SIG_IGN);
		signal(SIGINT,SIG_IGN);
		if(!locknum)
			sprintf(lock, "%d%s", (int)getuid(), plname);
		getlock();
#ifdef WIZARD
	} else {
		sprintf(lock, "%d%s", (int)getuid(), plname);
		getlock();
	}
#endif /* WIZARD */

	dlb_init();	/* must be before newgame() */

	/*
	 * Initialization of the boundaries of the mazes
	 * Both boundaries have to be even.
	 */
	x_maze_max = COLNO-1;
	if (x_maze_max % 2)
		x_maze_max--;
	y_maze_max = ROWNO-1;
	if (y_maze_max % 2)
		y_maze_max--;

	/*
	 *  Initialize the vision system.  This must be before mklev() on a
	 *  new game or before a level restore on a saved game.
	 */
	vision_init();

	display_gamewindows();

	if ((fd = restore_saved_game()) >= 0) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		bool remember_wiz_mode = wizard;
#endif
#ifndef FILE_AREAS
		const char *fq_save = fqname(SAVEF, SAVEPREFIX, 1);

		chmod(fq_save,0);	/* disallow parallel restores */
#else
		chmod_area(FILE_AREA_SAVE, SAVEF, 0);
#endif
		signal(SIGINT, (SIG_RET_TYPE) done1);
#ifdef NEWS
		if(iflags.news) {
		    display_file_area(NEWS_AREA, NEWS, false);
		    iflags.news = false; /* in case dorecover() fails */
		}
#endif
		pline("Restoring save file...");
		mark_synch();	/* flush output */
		if(!dorecover(fd))
			goto not_recovered;
#ifdef WIZARD
		if(!wizard && remember_wiz_mode) wizard = true;
#endif
		check_special_room(false);
		wd_message();

		if (discover || wizard) {
			if(yn("Do you want to keep the save file?") == 'n')
			    delete_savefile();
			else {
#ifdef FILE_AREAS
			    chmod_area(FILE_AREA_SAVE, SAVEF, FCMASK);
#else
			    chmod(fq_save,FCMASK); /* back to readable */
#endif
			}
		}
		flags.move = 0;
	} else {
not_recovered:
		player_selection();
		newgame();
		wd_message();

		flags.move = 0;
		set_wear();
		pickup(1);
	}

	moveloop();
	exit(EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}

static void
process_options(argc, argv)
int argc;
char *argv[];
{
	int i;


	/*
	 * Process options.
	 */
	while(argc > 1 && argv[1][0] == '-'){
		argv++;
		argc--;
		switch(argv[0][1]){
		case 'D':
		case 'Z':
#ifdef WIZARD
			{
			  char *user;
			  int uid;
			  struct passwd *pw = NULL;

			  uid = getuid();
			  user = getlogin();
			  if (user) {
			      pw = getpwnam(user);
			      if (pw && (pw->pw_uid != uid)) pw = 0;
			  }
			  if (pw == 0) {
			      user = nh_getenv("USER");
			      if (user) {
				  pw = getpwnam(user);
				  if (pw && (pw->pw_uid != uid)) pw = 0;
			      }
			      if (pw == 0) {
				  pw = getpwuid(uid);
			      }
			  }
			  if (pw && !strcmp(pw->pw_name,WIZARD)) {
			      wizard = true;
			      break;
			  }
			}
			/* otherwise fall thru to discover */
			wiz_error_flag = true;
#endif
		case 'X':
			discover = true;
			break;
#ifdef NEWS
		case 'n':
			iflags.news = false;
			break;
#endif
		case 'u':
			if(argv[0][2])
			  strncpy(plname, argv[0]+2, sizeof(plname)-1);
			else if(argc > 1) {
			  argc--;
			  argv++;
			  strncpy(plname, argv[0], sizeof(plname)-1);
			} else
				raw_print("Player name expected after -u");
			break;
		case 'p': /* profession (role) */
			if (argv[0][2]) {
			    if ((i = str2role(&argv[0][2])) >= 0)
			    	flags.initrole = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2role(argv[0])) >= 0)
			    	flags.initrole = i;
			}
			break;
		case 'r': /* race */
			if (argv[0][2]) {
			    if ((i = str2race(&argv[0][2])) >= 0)
			    	flags.initrace = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2race(argv[0])) >= 0)
			    	flags.initrace = i;
			}
			break;
		case 'g': /* gender */
			if (argv[0][2]) {
			    if ((i = str2gend(&argv[0][2])) >= 0)
			    	flags.initgend = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2gend(argv[0])) >= 0)
			    	flags.initgend = i;
			}
			break;
		case 'a': /* align */
			if (argv[0][2]) {
			    if ((i = str2align(&argv[0][2])) >= 0)
			    	flags.initalign = i;
			} else if (argc > 1) {
				argc--;
				argv++;
			    if ((i = str2align(argv[0])) >= 0)
			    	flags.initalign = i;
			}
			break;
		case '@':
			flags.randomall = 1;
			break;
		default:
			if ((i = str2role(&argv[0][1])) >= 0) {
			    flags.initrole = i;
				break;
			}
			/* else raw_printf("Unknown option: %s", *argv); */
		}
	}

	if(argc > 1)
		locknum = atoi(argv[1]);
#ifdef MAX_NR_OF_PLAYERS
	if(!locknum || locknum > MAX_NR_OF_PLAYERS)
		locknum = MAX_NR_OF_PLAYERS;
#endif
}

#ifdef CHDIR
static void chdirx(const char *dir, bool wr) {
	if (dir					/* User specified directory? */
# ifdef HACKDIR
	       && strcmp(dir, HACKDIR)		/* and not the default? */
# endif
		) {
# ifdef SECURE
	    setgid(getgid());
	    setuid(getuid());		/* Ron Wessels */
# endif
	} else {
	    /* non-default data files is a sign that scores may not be
	     * compatible, or perhaps that a binary not fitting this
	     * system's layout is being used.
	     */
# ifdef VAR_PLAYGROUND
	    int len = strlen(VAR_PLAYGROUND);

	    fqn_prefix[SCOREPREFIX] = alloc(len+2);
	    strcpy(fqn_prefix[SCOREPREFIX], VAR_PLAYGROUND);
	    if (fqn_prefix[SCOREPREFIX][len-1] != '/') {
		fqn_prefix[SCOREPREFIX][len] = '/';
		fqn_prefix[SCOREPREFIX][len+1] = '\0';
	    }
# endif
	}

# ifdef HACKDIR
	if (dir == NULL)
	    dir = HACKDIR;
# endif

	if (dir && chdir(dir) < 0) {
	    perror(dir);
	    error("Cannot chdir to %s.", dir);
	}

	/* warn the player if we can't write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access system-call is worthless */
	if (wr) {
# ifdef VAR_PLAYGROUND
	    fqn_prefix[LEVELPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[SAVEPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[BONESPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[LOCKPREFIX] = fqn_prefix[SCOREPREFIX];
	    fqn_prefix[TROUBLEPREFIX] = fqn_prefix[SCOREPREFIX];
# endif
	    check_recordfile(dir);
	}
}
#endif /* CHDIR */

static bool whoami(void) {
	/*
	 * Who am i? Algorithm: 1. Use name as specified in NETHACKOPTIONS
	 *			2. Use $USER or $LOGNAME	(if 1. fails)
	 *			3. Use getlogin()		(if 2. fails)
	 * The resulting name is overridden by command line options.
	 * If everything fails, or if the resulting name is some generic
	 * account like "games", "play", "player", "hack" then eventually
	 * we'll ask him.
	 * Note that we trust the user here; it is possible to play under
	 * somebody else's name.
	 */
	char *s;

	if (*plname) return false;
	if(/* !*plname && */ (s = nh_getenv("USER")))
		strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = nh_getenv("LOGNAME")))
		strncpy(plname, s, sizeof(plname)-1);
	if(!*plname && (s = getlogin()))
		strncpy(plname, s, sizeof(plname)-1);
	return true;
}

#ifdef PORT_HELP
void
port_help()
{
	/*
	 * Display unix-specific help.   Just show contents of the helpfile
	 * named by PORT_HELP.
	 */
	display_file_area(FILE_AREA_SHARE, PORT_HELP, true);
}
#endif

static void
wd_message()
{
#ifdef WIZARD
	if (wiz_error_flag) {
		pline("Only user \"%s\" may access debug (wizard) mode.", WIZARD);
		pline("Entering discovery mode instead.");
	} else
#endif
	if (discover)
		You("are in non-scoring discovery mode.");
}

/*
 * Add a slash to any name not ending in /. There must
 * be room for the /
 */
void
append_slash(name)
char *name;
{
	char *ptr;

	if (!*name)
		return;
	ptr = name + (strlen(name) - 1);
	if (*ptr != '/') {
		*++ptr = '/';
		*++ptr = '\0';
	}
	return;
}

/*unixmain.c*/
