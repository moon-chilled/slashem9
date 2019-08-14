/*	SCCS Id: @(#)pcmain.c	3.4	2002/08/22	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* main.c - MSDOS, OS/2, ST, Amiga, and NT NetHack */

#include "hack.h"
#include "dlb.h"

/* WAC for DEF_GAME_NAME */
#include "patchlevel.h"

#ifndef NO_SIGNAL
#include <signal.h>
#endif

#include <ctype.h>

#include <sys\stat.h>

#ifdef WIN32
#include "win32api.h"			/* for GetModuleFileName */
#endif

#ifdef __DJGPP__
#include <unistd.h>			/* for getcwd() prototype */
#endif

#define SHARED_DCL

SHARED_DCL char orgdir[PATHLEN];	/* also used in pcsys.c, amidos.c */

static void process_options(int argc,char **argv);
static void nhusage(void);

#if defined(MICRO) || defined(WIN32)
extern void nethack_exit(int);
#else
#define nethack_exit exit
#endif

#ifdef WIN32
extern boolean getreturn_enabled;	/* from sys/share/pcsys.c */
#endif

#ifdef EXEPATH
static char *exepath(char *);
#endif

int main(int,char **);

extern void pcmain(int,char **);

/* If the graphics version is built, we don't need a main; it is skipped
 * to help MinGW decide which entry point to choose. If both main and
 * WinMain exist, the resulting executable won't work correctly.
 */
int
main(argc,argv)
int argc;
char *argv[];
{
     pcmain(argc,argv);
#ifdef LAN_FEATURES
     init_lan_features();
#endif
     moveloop();
     nethack_exit(EXIT_SUCCESS);
     /*NOTREACHED*/
     return 0;
}

void
pcmain(argc,argv)
int argc;
char *argv[];
{

	int fd;
	char *dir;

#ifdef __DJGPP__
        if (*argv[0]) hname = argv[0];  /* DJGPP can give us argv[0] */
        else
#endif
                hname = "NetHack";      /* used for syntax messages */

	choose_windows(DEFAULT_WINDOW_SYS);

	/* Save current directory and make sure it gets restored when
	 * the game is exited.
	 */
	if (getcwd(orgdir, sizeof orgdir) == NULL)
		error("NetHack: current directory path too long");
# ifndef NO_SIGNAL
	signal(SIGINT, (SIG_RET_TYPE) nethack_exit);	/* restore original directory */
# endif

	dir = nh_getenv("NETHACKDIR");
	if (dir == NULL)
		dir = nh_getenv("HACKDIR");
#ifdef EXEPATH
	if (dir == NULL)
		dir = exepath(argv[0]);
#endif
	if (dir != NULL) {
		(void) strncpy(hackdir, dir, PATHLEN - 1);
		hackdir[PATHLEN-1] = '\0';
#ifdef NOCWD_ASSUMPTIONS
		{
		    int prefcnt;

		    fqn_prefix[0] = alloc(strlen(hackdir)+2);
		    strcpy(fqn_prefix[0], hackdir);
		    append_slash(fqn_prefix[0]);
		    for (prefcnt = 1; prefcnt < PREFIX_COUNT; prefcnt++)
			fqn_prefix[prefcnt] = fqn_prefix[0];
		}
#endif
#ifdef CHDIR
		chdirx (dir, 1);
#endif
	}
#if defined(WIN32) && defined(PROXY_GRAPHICS)
	/* Handle --proxy before options, if supported */
	if (argc > 1 && !strcmp(argv[1], "--proxy")) {
	    argv[1] = argv[0];
	    argc--;
	    argv++;
	    set_binary_mode(0, O_RDONLY | O_BINARY);
	    set_binary_mode(1, O_WRONLY | O_BINARY);
	    choose_windows("proxy");
	    lock_windows(true);		/* Can't be overridden from options */
	}
#endif
	initoptions();

	if (!hackdir[0])
#ifndef LATTICE
		strcpy(hackdir, orgdir);
#else
		strcpy(hackdir, HACKDIR);
#endif
	if(argc > 1) {
	    if (!strncmp(argv[1], "-d", 2) && argv[1][2] != 'e') {
		/* avoid matching "-dec" for DECgraphics; since the man page
		 * says -d directory, hope nobody's using -desomething_else
		 */
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
		strcpy(hackdir, dir);
	    }
	    if (argc > 1) {

		/*
		 * Now we know the directory containing 'record' and
		 * may do a prscore().
		 */
		if (!strncmp(argv[1], "-s", 2)) {
#ifdef CHDIR
			chdirx(hackdir,0);
#endif
			prscore(argc, argv);
			nethack_exit(EXIT_SUCCESS);
		}

		/* Don't initialize the window system just to print usage */
                /* WAC '--help' inits help */
		if (!strncmp(argv[1], "-?", 2)
                    || !strncmp(argv[1], "--help", 6)
                    || !strncmp(argv[1], "/?", 2)) {
			nhusage();
			nethack_exit(EXIT_SUCCESS);
		}
	    }
	}

	/*
	 * It seems you really want to play.
	 */
	u.uhp = 1;	/* prevent RIP on early quits */
	u.ux = 0;	/* prevent flush_screen() */

	/* chdir shouldn't be called before this point to keep the
	 * code parallel to other ports.
	 */
#ifdef CHDIR
	chdirx(hackdir,1);
#endif

	init_nhwindows(&argc,argv);
	process_options(argc, argv);

	if (!*plname)
		askname();
	plnamesuffix(); 	/* strip suffix from name; calls askname() */
				/* again if suffix was whole name */
				/* accepts any suffix */
#ifdef WIZARD
	if (wizard) {
		if(!strcmp(plname, WIZARD))
			strcpy(plname, "wizard");
		else {
			wizard = false;
			discover = true;
		}
	}
#endif /* WIZARD */
#if defined(PC_LOCKING)
	/* 3.3.0 added this to support detection of multiple games
	 * under the same plname on the same machine in a windowed
	 * or multitasking environment.
	 *
	 * That allows user confirmation prior to overwriting the
	 * level files of a game in progress.
	 *
	 * Also prevents an aborted game's level files from being
	 * overwritten without confirmation when a user starts up
	 * another game with the same player name.
	 */
# if defined(WIN32)
	/* Obtain the name of the logged on user and incorporate
	 * it into the name. */
	sprintf(lock, "%s-%s",get_username(0),plname);
# else
	strcpy(lock,plname);
	regularize(lock);
# endif
	getlock();
#else   /* PC_LOCKING */
	strcpy(lock,plname);
	strcat(lock,".99");
	regularize(lock);	/* is this necessary? */
#endif

	/* Set up level 0 file to keep the game state.
	 */
	fd = create_levelfile(0, NULL);
	if (fd < 0) {
		raw_print("Cannot create lock file");
	} else {
		hackpid = 1;
		write(fd, (void *) &hackpid, sizeof(hackpid));
		close(fd);
	}

	/*
	 * Initialisation of the boundaries of the mazes
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

	dlb_init();

	display_gamewindows();
#ifdef WIN32
	getreturn_enabled = true;
#endif

	if ((fd = restore_saved_game()) >= 0) {
#ifdef WIZARD
		/* Since wizard is actually flags.debug, restoring might
		 * overwrite it.
		 */
		boolean remember_wiz_mode = wizard;
#endif
#ifndef NO_SIGNAL
		(void) signal(SIGINT, (SIG_RET_TYPE) done1);
#endif
#ifdef NEWS
		if(iflags.news){
		    display_file_area(NEWS_AREA, NEWS, false);
		    iflags.news = false;
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
		if (discover)
			You("are in non-scoring discovery mode.");

		if (discover || wizard) {
			if(yn("Do you want to keep the save file?") == 'n'){
				(void) delete_savefile();
			}
		}

		flags.move = 0;
	} else {
not_recovered:
		player_selection();
		newgame();
		if (discover)
			You("are in non-scoring discovery mode.");

		flags.move = 0;
		set_wear();
		(void) pickup(1);
		sense_engr_at(u.ux,u.uy,false);
	}
#ifdef __DJGPP__
        /* WAC try to set title bar for Win95 DOS Boxes */
/*        {
                char titlebuf[BUFSZ];
                sprintf(titlebuf,"%s - %s", DEF_GAME_NAME, plname);
                (void) w95_setapptitle (titlebuf);
        }*/
#endif

#ifndef NO_SIGNAL
	(void) signal(SIGINT, SIG_IGN);
#endif
	return;
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
		case 'a':
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
		case 'D':
		case 'Z':
# ifdef WIZARD
			/* If they don't have a valid wizard name, it'll be
			 * changed to discover later.  Cannot check for
			 * validity of the name right now--it might have a
			 * character class suffix, for instance.
			 */
				wizard = true;
				break;
# endif
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
			  (void) strncpy(plname, argv[0]+2, sizeof(plname)-1);
			else if(argc > 1) {
			  argc--;
			  argv++;
			  (void) strncpy(plname, argv[0], sizeof(plname)-1);
			} else
				raw_print("Player name expected after -u");
			break;
		case 'i':
			if (!strncmpi(argv[0]+1, "IBM", 3))
				switch_graphics(IBM_GRAPHICS);
			break;
		case 'd':
			if (!strncmpi(argv[0]+1, "DEC", 3))
				switch_graphics(DEC_GRAPHICS);
			break;
		case 'g':
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
		case '@':
			flags.randomall = 1;
			break;
		default:
			if ((i = str2role(&argv[0][1])) >= 0) {
			    flags.initrole = i;
				break;
			} else raw_printf("\nUnknown switch: %s", argv[0]);
			/* FALL THROUGH */
		case '?':
			nhusage();
			nethack_exit(EXIT_SUCCESS);
		}
	}
}

static void
nhusage()
{
	char buf1[BUFSZ];

	/* -role still works for those cases which aren't already taken, but
	 * is deprecated and will not be listed here.
	 */
	(void) sprintf(buf1,
"\nUsage: %s [-d dir] -s [-r race] [-p profession] [maxrank] [name]...\n       or",
		hname);
	if (!iflags.window_inited)
		raw_printf(buf1);
	else
		(void)	printf(buf1);
	(void) sprintf(buf1,
	 "\n       %s [-d dir] [-u name] [-r race] [-p profession] [-[DX]]",
		hname);
#ifdef NEWS
	strcat(buf1," [-n]");
#endif
	strcat(buf1," [-I] [-i] [-d]");
	if (!iflags.window_inited)
		raw_printf("%s\n",buf1);
	else
		(void) printf("%s\n",buf1);
}

#ifdef CHDIR
void
chdirx(dir, wr)
char *dir;
boolean wr;
{
	static char thisdir[] = ".";
	if(dir && chdir(dir) < 0) {
		error("Cannot chdir to %s.", dir);
	}

# ifndef __CYGWIN__
	/* Change the default drive as well.
	 */
	chdrive(dir);
# endif

	/* warn the player if we can't write the record file */
	/* perhaps we should also test whether . is writable */
	/* unfortunately the access system-call is worthless */
	if (wr) check_recordfile(dir ? dir : thisdir);
}
#endif /* CHDIR */

#ifdef PORT_HELP
# ifdef WIN32
void
port_help()
{
    /* display port specific help file */
    display_file_area(FILE_AREA_SHARE, PORT_HELP, 1 );
}
# endif /* WIN32 */
#endif /* PORT_HELP */

#ifdef EXEPATH
#ifdef __DJGPP__
#define PATH_SEPARATOR '/'
#else
#define PATH_SEPARATOR '\\'
#endif

#define EXEPATHBUFSZ 256
char exepathbuf[EXEPATHBUFSZ];

char *exepath(str)
char *str;
{
	char *tmp, *tmp2;
	int bsize;

	if (!str) return NULL;
	bsize = EXEPATHBUFSZ;
	tmp = exepathbuf;
#ifndef WIN32
	strcpy (tmp, str);
#else
	#ifdef UNICODE
	{
		TCHAR wbuf[BUFSZ];
		GetModuleFileName((HANDLE)0, wbuf, BUFSZ);
		WideCharToMultiByte(CP_ACP, 0, wbuf, -1, tmp, bsize, NULL, NULL);
	}
	#else
		*(tmp + GetModuleFileName((HANDLE)0, tmp, bsize)) = '\0';
	#endif
#endif
	tmp2 = strrchr(tmp, PATH_SEPARATOR);
	if (tmp2) *tmp2 = '\0';
	return tmp;
}
#endif /* EXEPATH */
/*pcmain.c*/
