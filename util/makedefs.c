/*	SCCS Id: @(#)makedefs.c	3.4	2002/08/14	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* Copyright (c) M. Stephenson, 1990, 1991.			  */
/* Copyright (c) Dean Luick, 1990.				  */
/* NetHack may be freely redistributed.  See license for details. */

#define MAKEDEFS_C	/* use to conditionally include file sections */
/* #define DEBUG */	/* uncomment for debugging info */

#include <stdio.h>

#include "config.h"
#include "permonst.h"
#include "objclass.h"
#include "monsym.h"
#include "artilist.h"
#include "mondata.h"
#include "dungeon.h"
#include "obj.h"
#include "monst.h"
#include "you.h"
#include "flag.h"
#include "dlb.h"

/* version information */
#include "patchlevel.h"

#define rewind(fp) fseek((fp),0L,SEEK_SET)	/* guarantee a return value */

/* names of files to be generated */
#define DATE_FILE	"date.h"
#define MONST_FILE	"pm.h"
#define ONAME_FILE	"onames.h"
#ifndef NH_OPTIONS_FILE
#define OPTIONS_FILE	"options"
#else
#define OPTIONS_FILE    NH_OPTIONS_FILE
#endif
#define ORACLE_FILE	"oracles"
#define DATA_FILE	"data"
#define RUMOR_FILE	"rumors"
#define DGN_I_FILE	"dungeon.def"
#define DGN_O_FILE	"dungeon.pdf"
#define QTXT_I_FILE	"quest.txt"
#define QTXT_O_FILE	"quest.dat"

# define INCLUDE_TEMPLATE	"include/%s"
# define INCLUDE_IN_TEMPLATE    "../include/%s"
# define SOURCE_TEMPLATE	"src/%s"
# define DGN_TEMPLATE		"dat/%s"  /* where dungeon.pdf file goes */
# define DATA_TEMPLATE		"dat/%s"
# define DATA_IN_TEMPLATE	"../dat/%s"

static const char
    *Dont_Edit_Code = "/* This source file is generated by 'makedefs'.  Do not edit. */\n",
    *Dont_Edit_Data = "#\tThis data file is generated by 'makedefs'.  Do not edit. \n";

static struct version_info version;

static char     in_line[256], filename[600];

#ifdef FILE_PREFIX
		/* if defined, a first argument not starting with - is
		 * taken as a text string to be prepended to any
		 * output filename generated */
char *file_prefix="";
#endif

int main(int,char **);
void do_makedefs(char *);
void do_objs(void);
void do_data(void);
void do_dungeon(void);
void do_date(void);
void do_options(void);
void do_permonst(void);
void do_questtxt(void);
void do_rumors(void);
void do_oracles(void);

extern void monst_init(void);		/* monst.c */
extern void objects_init(void);	/* objects.c */

static void make_version(void);
static char *version_string(char *);
static char *version_id_string(char *,const char *);
static char *xcrypt(const char *);
static int check_control(char *);
static char *without_control(char *);
static bool d_filter(char *);
static bool h_filter(char *);
static bool ranged_attk(struct permonst*);
static int mstrength(struct permonst *);
static void build_savebones_compat_string(void);

static bool qt_comment(char *);
static bool qt_control(char *);
static int get_hdr(char *);
static bool new_id(char *);
static bool known_msg(int,int);
static void new_msg(char *,int,int);
static void do_qt_control(char *);
static void do_qt_text(char *);
static void adjust_qt_hdrs(void);
static void put_qt_hdrs(void);

static char *tmpdup(const char *);
static char *limit(char *,int);
static char *eos(char *);

/* input, output, tmp */
static FILE *ifp, *ofp, *tfp;


int main(int argc, char	**argv) {
#ifdef FILE_PREFIX
	if(argc >= 2) {
		file_prefix=argv[1];
	}
#endif

	/* Note:  these initializers don't do anything except guarantee that
	   we're linked properly.
	   */
	monst_init();
	objects_init();

	make_version();
	do_objs();
	do_data();
	do_dungeon();
	do_date();
	do_options();
	do_permonst();
	do_questtxt();
	do_rumors();
	do_oracles();

	return 0;
}


/* trivial text encryption routine which can't be broken with `tr' */
static char *xcrypt(const char *str) {
	/* duplicated in src/hacklib.c */
	static char buf[BUFSZ];
	const char *p;
	char *q;
	int bitmask;

	for (bitmask = 1, p = str, q = buf; *p; q++) {
		*q = *p++;
		if (*q & (32|64)) *q ^= bitmask;
		if ((bitmask <<= 1) >= 32) bitmask = 1;
	}
	*q = '\0';
	return buf;
}

void do_rumors(void) {
	char    *infile;
	long	true_rumor_size;

	infile = alloc(strlen(DATA_IN_TEMPLATE) - 2 + strlen(RUMOR_FILE) + 5);
	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename,file_prefix);
#endif
	sprintf(eos(filename), DATA_TEMPLATE, RUMOR_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	fprintf(ofp, "%s", Dont_Edit_Data);

	sprintf(infile, DATA_IN_TEMPLATE, RUMOR_FILE);
	strcat(infile, ".tru");
	if (!(ifp = fopen(infile, RDTMODE))) {
		perror(infile);
		fclose(ofp);
		unlink(filename);	/* kill empty output file */
		exit(EXIT_FAILURE);
	}

	/* get size of true rumors file */
	fseek(ifp, 0L, SEEK_END);
	true_rumor_size = ftell(ifp);

	fprintf(ofp,"%06lx\n", true_rumor_size);
	fseek(ifp, 0L, SEEK_SET);

	/* copy true rumors */
	while (fgets(in_line, sizeof in_line, ifp) != 0)
		fputs(xcrypt(in_line), ofp);

	fclose(ifp);

	sprintf(infile, DATA_IN_TEMPLATE, RUMOR_FILE);
	strcat(infile, ".fal");
	if (!(ifp = fopen(infile, RDTMODE))) {
		perror(infile);
		fclose(ofp);
		unlink(filename);	/* kill incomplete output file */
		exit(EXIT_FAILURE);
	}

	/* copy false rumors */
	while (fgets(in_line, sizeof in_line, ifp) != 0)
		fputs(xcrypt(in_line), ofp);

	fclose(ifp);
	fclose(ofp);
	free(infile);
}

/*
 * 3.4.1: way back in 3.2.1 `flags.nap' became unconditional but
 * TIMED_DELAY was erroneously left in VERSION_FEATURES and has
 * been there up through 3.4.0.  Simply removing it now would
 * break save file compatibility with 3.4.0 files, so we will
 * explicitly mask it out during version checks.
 * This should go away in the next version update.
 */
#define IGNORED_FEATURES	( 0L \
				| (1L << 23)	/* TIMED_DELAY */ \
				)

static void make_version(void) {
	int i;

	/*
	 * integer version number
	 */
	version.incarnation = ((unsigned long)VERSION_MAJOR << 24) |
				((unsigned long)VERSION_MINOR << 16) |
				((unsigned long)PATCHLEVEL << 8) |
				((unsigned long)EDITLEVEL);
	/*
	 * encoded feature list
	 * Note:  if any of these magic numbers are changed or reassigned,
	 * EDITLEVEL in patchlevel.h should be incremented at the same time.
	 * The actual values have no special meaning, and the category
	 * groupings are just for convenience.
	 */
	version.feature_set = (unsigned long)(0L
#ifdef MAIL
			| (1L <<  7)
#endif
		/* objects (8..15) */
#ifdef STEED
			| (1L << 11)
#endif
#ifdef GOLDOBJ
			| (1L << 12)
#endif
		/* flag bits and/or other global variables (16..26) */
#ifdef DISPLAY_LAYERS
			| (1L << 16)
#endif
#ifdef INSURANCE
			| (1L << 18)
#endif
		/* data format (27..31) */
#ifdef ZEROCOMP
			| (1L << 27)
#endif
#ifdef RLECOMP
			| (1L << 28)
#endif
			);
	/*
	 * Value used for object & monster sanity check.
	 *    (NROFARTIFACTS<<24) | (NUM_OBJECTS<<12) | (NUMMONS<<0)
	 */
	for (i = 1; artifact_names[i]; i++) continue;
        version.entity_count = (unsigned long) (i - 1);
	for (i = 1; objects[i].oc_class != ILLOBJ_CLASS; i++) continue;
        version.entity_count = (version.entity_count << 12) | (unsigned long) i;
	for (i = 0; mons[i].mlet; i++) continue;
        version.entity_count = (version.entity_count << 12) | (unsigned long) i;
	/*
	 * Value used for compiler (word size/field alignment/padding) check.
	 */
	version.struct_sizes = (((unsigned long)sizeof (struct flag)  << 24) |
				((unsigned long)sizeof (struct obj)   << 17) |
				((unsigned long)sizeof (struct monst) << 10) |
				((unsigned long)sizeof (struct you)));
}

static char *version_string(char *outbuf) {
	sprintf(outbuf, "%d.%d.%d", VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
#ifdef EDITLEVEL
	sprintf(eos(outbuf), "E%d", EDITLEVEL);
# ifdef FIXLEVEL
	sprintf(eos(outbuf), "F%d", FIXLEVEL);
# endif
#endif
	return outbuf;
}

/* WAC use DEF_GAME_NAME */
static char *version_id_string(char *outbuf, const char *build_date) {
	char subbuf[64], versbuf[64];
	subbuf[0] = '\0';
#ifdef PORT_SUB_ID
	subbuf[0] = ' ';
	strcpy(&subbuf[1], PORT_SUB_ID);
#endif
#if defined(ALPHA)
	strcat(subbuf, " Alpha");
#elif defined(BETA)
	strcat(subbuf, " Beta");
#endif

	sprintf(outbuf, "%s %s%s Version %s - last build %s.", PORT_ID, DEF_GAME_NAME, subbuf, version_string(versbuf), build_date);
	return outbuf;
}

void do_date(void) {
	unsigned long long clocktim = 0;
	char *c,  *cbuf, buf[BUFSZ];
	const char *ul_sfx;

	cbuf = alloc(600);
	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename,file_prefix);
#endif
	sprintf(eos(filename), INCLUDE_TEMPLATE, DATE_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	fprintf(ofp,"/*\tSCCS Id: @(#)date.h\t3.4\t2002/02/03 */\n\n");
	fprintf(ofp, "%s", Dont_Edit_Code);

	time((time_t *)&clocktim);
	strcpy(cbuf, ctime((time_t *)&clocktim));
	for (c = cbuf; *c; c++) if (*c == '\n') break;
	*c = '\0';	/* strip off the '\n' */
#ifdef NHSTDC
	ul_sfx = "UL";
#else
	ul_sfx = "L";
#endif
	fprintf(ofp,"#define BUILD_DATE \"%s\"\n", cbuf);
	fprintf(ofp,"#define BUILD_TIME (%lluL)\n", clocktim);
	fprintf(ofp,"\n");
	fprintf(ofp,"#define VERSION_NUMBER 0x%08lx%s\n",
			version.incarnation, ul_sfx);
	fprintf(ofp,"#define VERSION_FEATURES 0x%08lx%s\n",
			version.feature_set, ul_sfx);
#ifdef IGNORED_FEATURES
	fprintf(ofp,"#define IGNORED_FEATURES 0x%08lx%s\n",
			(unsigned long) IGNORED_FEATURES, ul_sfx);
#endif
	fprintf(ofp,"#define VERSION_SANITY1 0x%08lx%s\n",
			version.entity_count, ul_sfx);
	fprintf(ofp,"#define VERSION_SANITY2 0x%08lx%s\n",
			version.struct_sizes, ul_sfx);
	fprintf(ofp,"\n");
	fprintf(ofp,"#define VERSION_STRING \"%s\"\n", version_string(buf));
	fprintf(ofp,"#define VERSION_ID \\\n \"%s\"\n",
			version_id_string(buf, cbuf));
	fclose(ofp);
	free(cbuf);
}

static char save_bones_compat_buf[BUFSZ];

static void build_savebones_compat_string(void) {
#ifdef VERSION_COMPATIBILITY
	unsigned long uver = VERSION_COMPATIBILITY;
	char ueditsuffix[20];
#endif
	char editsuffix[20];
	/* Add edit level suffices if either EDITLEVEL is defined, or
	 * the first level we are compatible with was not edit level 0.
	 */
#ifdef EDITLEVEL
	sprintf(editsuffix, "E%d", EDITLEVEL);
# ifdef VERSION_COMPATIBILITY
	sprintf(ueditsuffix, "E%lu", uver & 0x000000FFL);
# endif
#else
# ifdef VERSION_COMPATIBILITY
	if (uver & 0x000000FFL) {
		strcpy(editsuffix, "E0");
		sprintf(ueditsuffix, "E%lu", uver & 0x000000FFL);
	} else {
# endif
		editsuffix[0] = 0;
		ueditsuffix[0] = 0;
# ifdef VERSION_COMPATIBILITY
	}
# endif
#endif
	strcpy(save_bones_compat_buf, "save and bones files accepted from version");
#ifdef VERSION_COMPATIBILITY
	sprintf(eos(save_bones_compat_buf),
			"s %lu.%lu.%lu%s through %d.%d.%d%s",
			((uver & 0xFF000000L) >> 24), ((uver & 0x00FF0000L) >> 16),
			((uver & 0x0000FF00L) >> 8), ueditsuffix,
			VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, editsuffix);
#else
	sprintf(eos(save_bones_compat_buf), " %d.%d.%d%s only",
			VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL, editsuffix);
#endif
}

static const char *build_opts[] = {
#ifdef ANSI_DEFAULT
	"ANSI default terminal",
#endif
	/*WAC added for borg,  invisible objects, keep_save,noartifactwish */
#ifdef BORG
	"borg",
#endif
#ifdef COM_COMPL
	"command line completion",
#endif
#ifdef DLB
	"data librarian",
#endif
#ifdef REALTIME_ON_BOTL
	"elapsed time on status line",
#endif
#ifdef GOLDOBJ
	"gold object in inventories",
#endif
#ifdef INSURANCE
	"insurance files for recovering from crashes",
#endif
	/*WAC invisible objects, keep_save,  light sourced spells*/
#ifdef LIGHT_SRC_SPELL
	"light sourced spell effects",
#endif
#ifdef KEEP_SAVE
	"keep savefiles",
#endif
#ifdef HOLD_LOCKFILE_OPEN
	"exclusive lock on level 0 file",
#endif
#ifdef LOGFILE
	"log file",
#endif
#ifdef XLOGFILE
	"extended log file",
#endif
#ifdef MAIL
	"mail daemon",
#endif
#ifdef NEWS
	"news file",
#endif
	/* WAC added noartifactwish version info*/
#ifdef NOARTIFACTWISH
	"no wishing for special artifacts",
#endif
#ifdef REDO
	"redo command",
#endif
#ifdef STEED
	"saddles and riding",
#endif
#ifdef DISPLAY_LAYERS
	"display layers",
#endif
#ifdef CLIPPING
	"screen clipping",
#endif
#ifdef NO_TERMS
# ifdef MAC
	"screen control via mactty",
# endif
# ifdef SCREEN_BIOS
	"screen control via BIOS",
# endif
# ifdef SCREEN_DJGPPFAST
	"screen control via DJGPP fast",
# endif
# ifdef SCREEN_VGA
	"screen control via VGA graphics",
# endif
# ifdef ALLEG_FX
	"screen control via Allegro library",
# endif
#endif
#ifdef SHELL
	"shell command",
#endif
#ifdef SUSPEND
	"suspend command",
#endif
#ifdef TERMINFO
	"terminal info library",
#else
# if defined(TERMLIB) || ((!defined(MICRO) && !defined(WIN32)) && defined(TTY_GRAPHICS))
	"terminal capability library",
# endif
#endif
#ifdef TIMED_DELAY
	"timed wait for display effects",
#endif
#ifdef USER_SOUNDS
# ifdef USER_SOUNDS_REGEX
	"user sounds via regular expressions",
# else
	"user sounds via pmatch",
# endif
#endif
#ifdef PREFIXES_IN_USE
	"variable playground",
#endif
#ifdef ZEROCOMP
	"zero-compressed save files",
#endif
	save_bones_compat_buf,
	"basic NetHack features"
};

static const char *window_opts[] = {
#ifdef TTY_GRAPHICS
	"traditional tty-based graphics",
#endif
#ifdef CURSES_GRAPHICS
	"curses",
#endif
#ifdef PROXY_GRAPHICS
	"Plug-in modules",
#endif
	0
};

void do_options(void) {
	int i, length;
	const char *str, *indent = "    ";

	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename,file_prefix);
#endif
	sprintf(eos(filename), DATA_TEMPLATE, OPTIONS_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	build_savebones_compat_string();
	fprintf(ofp,"\n    %s version %d.%d.%d", DEF_GAME_NAME, VERSION_MAJOR, VERSION_MINOR, PATCHLEVEL);
#ifdef EDITLEVEL
	fprintf(ofp, "E%d", EDITLEVEL);
#ifdef FIXLEVEL
	fprintf(ofp, "F%d", FIXLEVEL);
#endif
#endif
#if defined(ALPHA)
	fprintf(ofp, " [alpha]\n");
#elif defined(BETA)
	fprintf(ofp, " [beta]\n");
#else
	fprintf(ofp, "\n");
#endif

	fprintf(ofp,"\nOptions compiled into this edition:\n");

	length = COLNO + 1;	/* force 1st item onto new line */
	for (i = 0; i < SIZE(build_opts); i++) {
		str = build_opts[i];
		if (length + strlen(str) > COLNO - 5)
			fprintf(ofp,"\n%s", indent),  length = strlen(indent);
		else
			fprintf(ofp," "),  length++;
		fprintf(ofp,"%s", str),  length += strlen(str);
		fprintf(ofp,(i < SIZE(build_opts) - 1) ? "," : "."),  length++;
	}

	fprintf(ofp,"\n\nSupported windowing systems:\n");

	length = COLNO + 1;	/* force 1st item onto new line */
	for (i = 0; i < SIZE(window_opts) - 1; i++) {
		str = window_opts[i];
		if (length + strlen(str) > COLNO - 5)
			fprintf(ofp,"\n%s", indent),  length = strlen(indent);
		else
			fprintf(ofp," "),  length++;
		fprintf(ofp,"%s", str),  length += strlen(str);
		fprintf(ofp, ","),  length++;
	}
	fprintf(ofp, "\n%swith a default of %s.", indent, DEFAULT_WINDOW_SYS);
	fprintf(ofp,"\n\n");

	fclose(ofp);
}

/* routine to decide whether to discard something from data.base */
static bool d_filter(char *line) {
    return line[0] == '#'; // ignore comment lines
}

   /*
    *
	New format (v3.1) of 'data' file which allows much faster lookups [pr]
"do not edit"		first record is a comment line
01234567		hexadecimal formatted offset to text area
name-a			first name of interest
123,4			offset to name's text, and number of lines for it
name-b			next name of interest
name-c			multiple names which share same description also
456,7			share a single offset,count line
.			sentinel to mark end of names
789,0			dummy record containing offset, count of EOF
text-a			4 lines of descriptive text for name-a
text-a			at file position 0x01234567L + 123L
text-a
text-a
text-b/text-c		7 lines of text for names-b and -c
text-b/text-c		at fseek(0x01234567L + 456L)
...
    *
    */

void do_data(void) {
	char    *infile, *tempfile;
	bool ok;
	long	txt_offset;
	int	entry_cnt, line_cnt;

	infile = alloc(strlen(DATA_IN_TEMPLATE) - 2 + strlen(DATA_FILE) + 6);
	tempfile = alloc(strlen(DATA_TEMPLATE) - 2 + strlen("database.tmp") + 1);
	sprintf(tempfile, DATA_TEMPLATE, "database.tmp");
	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename,file_prefix);
#endif
	sprintf(eos(filename), DATA_TEMPLATE, DATA_FILE);
	sprintf(infile, DATA_IN_TEMPLATE, DATA_FILE);
	strcat(infile, ".base");
	if (!(ifp = fopen(infile, RDTMODE))) {		/* data.base */
		perror(infile);
		exit(EXIT_FAILURE);
	}
	if (!(ofp = fopen(filename, WRTMODE))) {	/* data */
		perror(filename);
		fclose(ifp);
		exit(EXIT_FAILURE);
	}
	free(infile);
	if (!(tfp = fopen(tempfile, WRTMODE))) {	/* database.tmp */
		perror(tempfile);
		fclose(ifp);
		fclose(ofp);
		unlink(filename);
		exit(EXIT_FAILURE);
	}

	/* output a dummy header record; we'll rewind and overwrite it later */
	fprintf(ofp, "%s%08lx\n", Dont_Edit_Data, 0L);

	entry_cnt = line_cnt = 0;
	/* read through the input file and split it into two sections */
	while (fgets(in_line, sizeof in_line, ifp)) {
		if (d_filter(in_line)) continue;
		if (*in_line > ' ') {	/* got an entry name */
			/* first finish previous entry */
			if (line_cnt)  fprintf(ofp, "%d\n", line_cnt),  line_cnt = 0;
			/* output the entry name */
			fputs(in_line, ofp);
			entry_cnt++;		/* update number of entries */
		} else if (entry_cnt) {	/* got some descriptive text */
			/* update previous entry with current text offset */
			if (!line_cnt)  fprintf(ofp, "%ld,", ftell(tfp));
			/* save the text line in the scratch file */
			fputs(in_line, tfp);
			line_cnt++;		/* update line counter */
		}
	}
	/* output an end marker and then record the current position */
	if (line_cnt)  fprintf(ofp, "%d\n", line_cnt);
	fprintf(ofp, ".\n%ld,%d\n", ftell(tfp), 0);
	txt_offset = ftell(ofp);
	fclose(ifp);		/* all done with original input file */

	/* reprocess the scratch file; 1st format an error msg, just in case */
	sprintf(in_line, "rewind of \"%s\"", tempfile);
	if (rewind(tfp) != 0)  goto dead_data;
	/* copy all lines of text from the scratch file into the output file */
	while (fgets(in_line, sizeof in_line, tfp))
		fputs(in_line, ofp);

	/* finished with scratch file */
	fclose(tfp);
	unlink(tempfile);	/* remove it */
	free(tempfile);

	/* update the first record of the output file; prepare error msg 1st */
	sprintf(in_line, "rewind of \"%s\"", filename);
	ok = rewind(ofp) == 0;
	if (ok) {
		sprintf(in_line, "header rewrite of \"%s\"", filename);
		ok = fprintf(ofp, "%s%08lx\n", Dont_Edit_Data, txt_offset) >= 0;
	}
	if (!ok) {
dead_data:  perror(in_line);	/* report the problem */
	    /* close and kill the aborted output file, then give up */
	    fclose(ofp);
	    unlink(filename);
	    exit(EXIT_FAILURE);
	}

	/* all done */
	fclose(ofp);
}

/* routine to decide whether to discard something from oracles.txt */
static bool h_filter(char *line) {
	static bool skip = false;
	char tag[sizeof in_line];

	if (*line == '#') return true;	/* ignore comment lines */
	if (sscanf(line, "----- %s", tag) == 1) {
		skip = false;
	} else if (skip && !strncmp(line, "-----", 5))
		skip = false;
	return skip;
}

static const char *special_oracle[] = {
	"\"...it is rather disconcerting to be confronted with the",
	"following theorem from [Baker, Gill, and Solovay, 1975].",
	"",
	"Theorem 7.18  There exist recursive languages A and B such that",
	"  (1)  P(A) == NP(A), and",
	"  (2)  P(B) != NP(B)",
	"",
	"This provides impressive evidence that the techniques that are",
	"currently available will not suffice for proving that P != NP or          ",
	"that P == NP.\"  [Garey and Johnson, p. 185.]"
};

/*
   The oracle file consists of a "do not edit" comment, a decimal count N
   and set of N+1 hexadecimal fseek offsets, followed by N multiple-line
   records, separated by "---" lines.  The first oracle is a special case.
   The input data contains just those multi-line records, separated by
   "-----" lines.
 */

void do_oracles(void) {
	char *infile, *tempfile;
	boolean in_oracle, ok;
	long txt_offset, offset, fpos;
	int oracle_cnt;
	int i;

	infile = alloc(strlen(DATA_IN_TEMPLATE) - 2 + strlen(ORACLE_FILE) + 5);
	tempfile = alloc(strlen(DATA_TEMPLATE) - 2 + strlen("oracles.tmp") + 1);
	sprintf(tempfile, DATA_TEMPLATE, "oracles.tmp");
	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename, file_prefix);
#endif
	sprintf(eos(filename), DATA_TEMPLATE, ORACLE_FILE);
	sprintf(infile, DATA_IN_TEMPLATE, ORACLE_FILE);
	strcat(infile, ".txt");
	if (!(ifp = fopen(infile, RDTMODE))) {
		perror(infile);
		exit(EXIT_FAILURE);
	}
	free(infile);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		fclose(ifp);
		exit(EXIT_FAILURE);
	}
	if (!(tfp = fopen(tempfile, WRTMODE))) {	/* oracles.tmp */
		perror(tempfile);
		fclose(ifp);
		fclose(ofp);
		unlink(filename);
		exit(EXIT_FAILURE);
	}

	/* output a dummy header record; we'll rewind and overwrite it later */
	fprintf(ofp, "%s%5d\n", Dont_Edit_Data, 0);

	/* handle special oracle; it must come first */
	fputs("---\n", tfp);
	fprintf(ofp, "%05lx\n", ftell(tfp));  /* start pos of special oracle */
	for (i = 0; i < SIZE(special_oracle); i++) {
		fputs(xcrypt(special_oracle[i]), tfp);
		fputc('\n', tfp);
	}

	oracle_cnt = 1;
	fputs("---\n", tfp);
	fprintf(ofp, "%05lx\n", ftell(tfp));	/* start pos of first oracle */
	in_oracle = false;

	while (fgets(in_line, sizeof in_line, ifp)) {
		if (h_filter(in_line)) continue;
		if (!strncmp(in_line, "-----", 5)) {
			if (!in_oracle) continue;
			in_oracle = false;
			oracle_cnt++;
			fputs("---\n", tfp);
			fprintf(ofp, "%05lx\n", ftell(tfp));
			/* start pos of this oracle */
		} else {
			in_oracle = true;
			fputs(xcrypt(in_line), tfp);
		}
	}

	if (in_oracle) {	/* need to terminate last oracle */
		oracle_cnt++;
		fputs("---\n", tfp);
		fprintf(ofp, "%05lx\n", ftell(tfp));	/* eof position */
	}

	/* record the current position */
	txt_offset = ftell(ofp);
	fclose(ifp);		/* all done with original input file */

	/* reprocess the scratch file; 1st format an error msg, just in case */
	sprintf(in_line, "rewind of \"%s\"", tempfile);
	if (rewind(tfp) != 0)  goto dead_data;
	/* copy all lines of text from the scratch file into the output file */
	while (fgets(in_line, sizeof in_line, tfp))
		fputs(in_line, ofp);

	/* finished with scratch file */
	fclose(tfp);
	unlink(tempfile);	/* remove it */
	free(tempfile);

	/* update the first record of the output file; prepare error msg 1st */
	sprintf(in_line, "rewind of \"%s\"", filename);
	ok = (rewind(ofp) == 0);
	if (ok) {
		sprintf(in_line, "header rewrite of \"%s\"", filename);
		ok = (fprintf(ofp, "%s%5d\n", Dont_Edit_Data, oracle_cnt) >=0);
	}
	if (ok) {
		sprintf(in_line, "data rewrite of \"%s\"", filename);
		for (i = 0; i <= oracle_cnt; i++) {
			if (!(ok = (fflush(ofp) == 0))) break;
			if (!(ok = (fpos = ftell(ofp)) >= 0)) break;
			if (!(ok = (fseek(ofp, fpos, SEEK_SET) >= 0))) break;
			if (!(ok = (fscanf(ofp, "%5lx", &offset) == 1))) break;
			if (!(ok = (fseek(ofp, fpos, SEEK_SET) >= 0))) break;
			if (!(ok = (fprintf(ofp, "%05lx\n", offset + txt_offset) >= 0)))
				break;
		}
	}
	if (!ok) {
dead_data:  perror(in_line);	/* report the problem */
	    /* close and kill the aborted output file, then give up */
	    /* KMH -- I don't know why it fails */
#ifndef MAC
	    fclose(ofp);
	    unlink(filename);
	    exit(EXIT_FAILURE);
#endif
	}

	/* all done */
	fclose(ofp);
}


// this can be used to selectively disable levels in dungeon.def
static	struct deflist {
	const char	*defname;
	boolean	true_or_false;
} deflist[] = {{ 0, 0 }};

static int check_control(char *s) {
	int i;

	if(s[0] != '%') return -1;

	for(i = 0; deflist[i].defname; i++)
		if(!strncmp(deflist[i].defname, s+1, strlen(deflist[i].defname)))
			return i;

	return -1;
}

static char *without_control(char *s) {
	return s + 1 + strlen(deflist[check_control(in_line)].defname);
}

void do_dungeon(void) {
	int rcnt = 0;

	sprintf(filename, DATA_IN_TEMPLATE, DGN_I_FILE);
	if (!(ifp = fopen(filename, RDTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename, file_prefix);
#endif
	sprintf(eos(filename), DGN_TEMPLATE, DGN_O_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	fprintf(ofp, "%s", Dont_Edit_Data);

	while (fgets(in_line, sizeof in_line, ifp) != 0) {
		rcnt++;
		if(in_line[0] == '#') continue;	/* discard comments */
recheck:
		if(in_line[0] == '%') {
			int i = check_control(in_line);
			if(i >= 0) {
				if(!deflist[i].true_or_false)  {
					while (fgets(in_line, sizeof in_line, ifp) != 0)
						if(check_control(in_line) != i) goto recheck;
				} else
					fputs(without_control(in_line),ofp);
			} else {
				fprintf(stderr, "Unknown control option '%s' in file %s at line %d.\n",
						in_line, DGN_I_FILE, rcnt);
				exit(EXIT_FAILURE);
			}
		} else
			fputs(in_line,ofp);
	}
	fclose(ifp);
	fclose(ofp);
}

void do_permonst(void) {
	int i;
	char *c, *nam;

	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename, file_prefix);
#endif
	sprintf(eos(filename), INCLUDE_TEMPLATE, MONST_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	fprintf(ofp, "/*\tSCCS Id: @(#)pm.h\t3.4\t2002/02/03 */\n\n");
	fprintf(ofp, "%s", Dont_Edit_Code);
	fprintf(ofp, "#ifndef PM_H\n#define PM_H\n");

	if (strcmp(mons[0].mname, "playermon") != 0)
		fprintf(ofp,"\n#define PM_PLAYERMON (-1)");

	for (i = 0; mons[i].mlet; i++) {
		fprintf(ofp,"\n#define\tPM_");
		if (mons[i].mlet == S_HUMAN &&
				!strncmp(mons[i].mname, "were", 4))
			fprintf(ofp, "HUMAN_");
		for (nam = c = tmpdup(mons[i].mname); *c; c++)
			if (*c >= 'a' && *c <= 'z') *c -= ('a' - 'A');
			else if (*c < 'A' || *c > 'Z') *c = '_';
		fprintf(ofp,"%s\t%d", nam, i);
	}
	fprintf(ofp,"\n\n#define NUMMONS %d\n", i);
	fprintf(ofp,"\n#endif /* PM_H */\n");
	fclose(ofp);
}


/*	Start of Quest text file processing. */
#include "qtext.h"

static struct qthdr qt_hdr;
static struct msghdr msg_hdr[N_HDR];
static struct qtmsg *curr_msg;

static int qt_line;

static bool in_msg;
#define NO_MSG	1	/* strlen of a null line returned by fgets() */

static bool qt_comment(char *s) {
	if (s[0] == '#') return true;
	return !in_msg  && strlen(s) == NO_MSG;
}

static bool qt_control(char *s) {
	return s[0] == '%' && (s[1] == 'C' || s[1] == 'E');
}

static int get_hdr(char *code) {
	int i;

	for(i = 0; i < qt_hdr.n_hdr; i++)
	    if(!strncmp(code, qt_hdr.id[i], LEN_HDR)) return i+1;

	return 0;
}

static bool new_id(char *code) {
	if(qt_hdr.n_hdr >= N_HDR) {
		fprintf(stderr, OUT_OF_HEADERS, qt_line);
		return false;
	}

	strncpy(&qt_hdr.id[qt_hdr.n_hdr][0], code, LEN_HDR);
	msg_hdr[qt_hdr.n_hdr].n_msg = 0;
	qt_hdr.offset[qt_hdr.n_hdr++] = 0L;
	return true;
}

static bool known_msg(int num, int id) {
	int i;

	for(i = 0; i < msg_hdr[num].n_msg; i++)
	    if(msg_hdr[num].qt_msg[i].msgnum == id) return true;

	return false;
}


static void new_msg(char *s, int num, int id) {
	struct qtmsg *qt_msg;

	if(msg_hdr[num].n_msg >= N_MSG) {
		fprintf(stderr, OUT_OF_MESSAGES, qt_line);
	} else {
		qt_msg = &(msg_hdr[num].qt_msg[msg_hdr[num].n_msg++]);
		qt_msg->msgnum = id;
		qt_msg->delivery = s[2];
		qt_msg->offset = qt_msg->size = 0L;

		curr_msg = qt_msg;
	}
}

static void do_qt_control(char *s) {
	char code[BUFSZ];
	int num, id = 0;

	switch(s[1]) {

	    case 'C':	if(in_msg) {
			    fprintf(stderr, CREC_IN_MSG, qt_line);
			    break;
			} else {
			    in_msg = true;
			    if (sscanf(&s[4], "%s %5d", code, &id) != 2) {
			    	fprintf(stderr, UNREC_CREC, qt_line);
			    	break;
			    }
			    num = get_hdr(code);
			    if (!num && !new_id(code))
			    	break;
			    num = get_hdr(code)-1;
			    if(known_msg(num, id))
			    	fprintf(stderr, DUP_MSG, qt_line);
			    else new_msg(s, num, id);
			}
			break;

	    case 'E':	if(!in_msg) {
			    fprintf(stderr, END_NOT_IN_MSG, qt_line);
			    break;
			} else in_msg = false;
			break;

	    default:	fprintf(stderr, UNREC_CREC, qt_line);
			break;
	}
}

static void do_qt_text(char *s) {
	if (!in_msg) {
	    fprintf(stderr, TEXT_NOT_IN_MSG, qt_line);
	}
	curr_msg->size += strlen(s);
}

static void adjust_qt_hdrs(void) {
	int i, j;
	long count = 0L, hdr_offset = sizeof(int) + (LEN_HDR + sizeof(long)) * qt_hdr.n_hdr;

	for(i = 0; i < qt_hdr.n_hdr; i++) {
		qt_hdr.offset[i] = hdr_offset;
		hdr_offset += sizeof(int) + sizeof(struct qtmsg) * msg_hdr[i].n_msg;
	}

	for(i = 0; i < qt_hdr.n_hdr; i++) {
		for(j = 0; j < msg_hdr[i].n_msg; j++) {
			msg_hdr[i].qt_msg[j].offset = hdr_offset + count;
			count += msg_hdr[i].qt_msg[j].size;
		}
	}
}

static void put_qt_hdrs(void) {
	int i;

	/*
	 *	The main header record.
	 */
#ifdef DEBUG
	fprintf(stderr, "%ld: header info.\n", ftell(ofp));
#endif
	fwrite(&(qt_hdr.n_hdr), sizeof(int), 1, ofp);
	fwrite(&(qt_hdr.id[0][0]), LEN_HDR, qt_hdr.n_hdr, ofp);
	fwrite(&(qt_hdr.offset[0]), sizeof(long),
							qt_hdr.n_hdr, ofp);
#ifdef DEBUG
	for(i = 0; i < qt_hdr.n_hdr; i++)
		fprintf(stderr, "%c @ %ld, ", qt_hdr.id[i], qt_hdr.offset[i]);

	fprintf(stderr, "\n");
#endif

	/*
	 *	The individual class headers.
	 */
	for(i = 0; i < qt_hdr.n_hdr; i++) {

#ifdef DEBUG
		fprintf(stderr, "%ld: %c header info.\n", ftell(ofp), qt_hdr.id[i]);
#endif
		fwrite(&(msg_hdr[i].n_msg), sizeof(int), 1, ofp);
		fwrite(&(msg_hdr[i].qt_msg[0]), sizeof(struct qtmsg), msg_hdr[i].n_msg, ofp);
#ifdef DEBUG
		{
			int j;
			for(j = 0; j < msg_hdr[i].n_msg; j++)
				fprintf(stderr, "msg %d @ %ld (%ld)\n",
						msg_hdr[i].qt_msg[j].msgnum,
						msg_hdr[i].qt_msg[j].offset,
						msg_hdr[i].qt_msg[j].size);
		}
#endif
	}
}

void do_questtxt(void) {
	sprintf(filename, DATA_IN_TEMPLATE, QTXT_I_FILE);
	ifp = fopen(filename, RDTMODE);
	if(!ifp) {
		perror(filename);
		exit(EXIT_FAILURE);
	}

	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename, file_prefix);
#endif
	sprintf(eos(filename), DATA_TEMPLATE, QTXT_O_FILE);
	if(!(ofp = fopen(filename, WRBMODE))) {
		perror(filename);
		fclose(ifp);
		exit(EXIT_FAILURE);
	}

	qt_hdr.n_hdr = 0;
	qt_line = 0;
	in_msg = false;

	while (fgets(in_line, 80, ifp) != 0) {
		qt_line++;
		if(qt_control(in_line)) do_qt_control(in_line);
		else if(qt_comment(in_line)) continue;
		else		    do_qt_text(in_line);
	}

	rewind(ifp);
	in_msg = false;
	adjust_qt_hdrs();
	put_qt_hdrs();
	while (fgets(in_line, 80, ifp) != 0) {

		if(qt_control(in_line)) {
			in_msg = (in_line[1] == 'C');
			continue;
		} else if(qt_comment(in_line)) continue;
#ifdef DEBUG
		fprintf(stderr, "%ld: %s", ftell(stdout), in_line);
#endif
		fputs(xcrypt(in_line), ofp);
	}
	fclose(ifp);
	fclose(ofp);
	return;
}


static char temp[32];

/* limit a name to 30 characters length */
static char * limit(char *name, int pref) {
	strncpy(temp, name, pref ? 26 : 30);
	temp[pref ? 26 : 30] = 0;
	return temp;
}

void do_objs(void) {
	int i, sum = 0;
	char *c, *objnam;
	int nspell = 0;
	int prefix = 0;
	char class = '\0';
	boolean	sumerr = false;

	filename[0]='\0';
#ifdef FILE_PREFIX
	strcat(filename, file_prefix);
#endif
	sprintf(eos(filename), INCLUDE_TEMPLATE, ONAME_FILE);
	if (!(ofp = fopen(filename, WRTMODE))) {
		perror(filename);
		exit(EXIT_FAILURE);
	}
	fprintf(ofp, "/*\tSCCS Id: @(#)onames.h\t3.4\t2002/02/03 */\n\n");
	fprintf(ofp, "%s", Dont_Edit_Code);
	fprintf(ofp, "#ifndef ONAMES_H\n#define ONAMES_H\n\n");

	for(i = 0; !i || objects[i].oc_class != ILLOBJ_CLASS; i++) {
		objects[i].oc_name_idx = objects[i].oc_descr_idx = i;	/* init */
		if (!(objnam = tmpdup(OBJ_NAME(objects[i])))) continue;

		/* make sure probabilities add up to 1000 */
		if(objects[i].oc_class != class) {
			if (sum && sum != 1000) {
			    fprintf(stderr, "prob error for class %d (%d%%)",
				    class, sum);
			    fflush(stderr);
			    sumerr = true;
			}
			class = objects[i].oc_class;
			sum = 0;
		}

		for (c = objnam; *c; c++)
		    if (*c >= 'a' && *c <= 'z') *c -= (char)('a' - 'A');
		    else if (*c < 'A' || *c > 'Z') *c = '_';

		switch (class) {
		    case WAND_CLASS:
			fprintf(ofp,"#define\tWAN_"); prefix = 1; break;
		    case RING_CLASS:
			fprintf(ofp,"#define\tRIN_"); prefix = 1; break;
		    case POTION_CLASS:
			fprintf(ofp,"#define\tPOT_"); prefix = 1; break;
		    case SPBOOK_CLASS:
			fprintf(ofp,"#define\tSPE_"); prefix = 1; nspell++; break;
		    case SCROLL_CLASS:
			fprintf(ofp,"#define\tSCR_"); prefix = 1; break;
		    case AMULET_CLASS:
			/* avoid trouble with stupid C preprocessors */
			fprintf(ofp,"#define\t");
			if(objects[i].oc_material == PLASTIC) {
			    fprintf(ofp,"FAKE_AMULET_OF_YENDOR\t%d\n", i);
			    prefix = -1;
			    break;
			}
			break;
		    case GEM_CLASS:
			/* avoid trouble with stupid C preprocessors */
			if(objects[i].oc_material == GLASS) {
			    fprintf(ofp,"/* #define\t%s\t%d */\n",
							objnam, i);
			    prefix = -1;
			    break;
			}
		    default:
			fprintf(ofp,"#define\t");
		}
		if (prefix >= 0)
			fprintf(ofp,"%s\t%d\n", limit(objnam, prefix), i);
		prefix = 0;

		sum += objects[i].oc_prob;
	}

	/* check last set of probabilities */
	if (sum && sum != 1000) {
	    fprintf(stderr, "prob error for class %d (%d%%)", class, sum);
	    fflush(stderr);
	    sumerr = true;
	}

	fprintf(ofp,"#define\tLAST_GEM\t(JADE)\n");
	fprintf(ofp,"#define\tMAXSPELL\t%d\n", nspell+1);
	fprintf(ofp,"#define\tNUM_OBJECTS\t%d\n", i);

	fprintf(ofp, "\n/* Artifacts (unique objects) */\n\n");

	for (i = 1; artifact_names[i]; i++) {
		for (c = objnam = tmpdup(artifact_names[i]); *c; c++)
		    if (*c >= 'a' && *c <= 'z') *c -= (char)('a' - 'A');
		    else if (*c < 'A' || *c > 'Z') *c = '_';

		if (!strncmp(objnam, "THE_", 4))
			objnam += 4;
		/* fudge _platinum_ YENDORIAN EXPRESS CARD */
		if (!strncmp(objnam, "PLATINUM_", 9))
			objnam += 9;
		fprintf(ofp,"#define\tART_%s\t%d\n", limit(objnam, 1), i);
	}

	fprintf(ofp, "#define\tNROFARTIFACTS\t%d\n", i-1);
	fprintf(ofp,"\n#endif /* ONAMES_H */\n");
	fclose(ofp);
	if (sumerr) exit(EXIT_FAILURE);
}

static char *tmpdup(const char *str) {
	static char buf[128];

	if (!str) return NULL;
	strncpy(buf, str, 127);
	return buf;
}

static char *eos(char *str) {
	while (*str) str++;
	return str;
}

#ifdef STRICT_REF_DEF
struct flag flags;
# ifdef ATTRIB_H
struct attribs attrmax, attrmin;
# endif
#endif /* STRICT_REF_DEF */

/*makedefs.c*/
