/*	SCCS Id: @(#)dgn_main.c 3.4	1994/09/23	*/
/*	Copyright (c) 1989 by Jean-Christophe Collet	*/
/*	Copyright (c) 1990 by M. Stephenson		*/
/* NetHack may be freely redistributed.  See license for details. */

/*
 * This file contains the main function for the parser
 * and some useful functions needed by yacc
 */

#include "config.h"
#include "dlb.h"

/* Macintosh-specific code */
#if defined(__APPLE__) && defined(__MACH__)
  /* MacOS X has Unix-style files and processes */
# undef MAC
#endif
#ifdef MAC
# ifdef MAC_MPW
#  define MPWTOOL
#include <CursorCtl.h>
# else
   /* put dungeon file in library location */
#  define PREFIX ":lib:"
# endif
#endif

#ifndef MPWTOOL
# define SpinCursor(x)
#endif

#define MAX_ERRORS	25

extern int  yyparse(void);
extern int line_number;
const char *fname = "(stdin)";
int fatal_error = 0;

int  main(int,char **);
void yyerror(const char *);
void yywarning(const char *);
int  yywrap(void);
void init_yyin(FILE *);
void init_yyout(FILE *);

int
main(argc, argv)
int argc;
char **argv;
{
	char	*infile, *outfile, *basename;
	FILE	*fin, *fout;
	int	i, len;
	boolean errors_encountered = false;
#if defined(MAC) && (defined(__MWERKS__))
	char	*mark;
	static char *mac_argv[] = {	"dgn_comp",	/* dummy argv[0] */
				":dat:dungeon.pdf"
				};

	argc = SIZE(mac_argv);
	argv = mac_argv;
#endif

	infile = "(stdin)";
	fin = stdin;
	outfile = "(stdout)";
	fout = stdout;

	if (argc == 1) {	/* Read standard input */
	    init_yyin(fin);
	    init_yyout(fout);
	    yyparse();
	    if (fatal_error > 0)
		errors_encountered = true;
	} else {		/* Otherwise every argument is a filename */
	    infile = outfile = NULL;
	    for(i=1; i<argc; i++) {
		if (infile) free(infile);
		infile = alloc(strlen(argv[i]) + 1);
		fname = strcpy(infile, argv[i]);
		/* the input file had better be a .pdf file */
		len = strlen(fname) - 4;	/* length excluding suffix */
		if (len < 0 || strncmp(".pdf", fname + len, 4)) {
		    fprintf(stderr,
			    "Error - file name \"%s\" in wrong format.\n",
			    fname);
		    errors_encountered = true;
		    free(infile);
		    continue;
		}

		/* build output file name */
#if defined(MAC) && (defined(__MWERKS__))
		/* extract basename from path to infile */
		mark = strrchr(infile, ':');
		mark = mark ? mark+1 : infile;
		basename = alloc(strlen(mark) + 1);
		strcpy(basename, mark);
		mark = strchr(basename, '.');
		if (mark) *mark = '\0';
#else
		/* Use the whole name - strip off the last 3 or 4 chars. */

		basename = alloc(len + 1);
		strncpy(basename, infile, len);
		basename[len] = '\0';
#endif

		if (outfile) free(outfile);
#ifdef PREFIX
		outfile = alloc(strlen(PREFIX) + strlen(basename) + 1);
		strcpy(outfile, PREFIX);
#else
		outfile = alloc(strlen(basename) + 1);
		outfile[0] = '\0';
#endif
		strcat(outfile, basename);
		free(basename);

		fin = freopen(infile, "r", stdin);
		if (!fin) {
		    fprintf(stderr, "Can't open %s for input.\n", infile);
		    perror(infile);
		    errors_encountered = true;
		    continue;
		}
		fout = freopen(outfile, WRBMODE, stdout);
		if (!fout) {
		    fprintf(stderr, "Can't open %s for output.\n", outfile);
		    perror(outfile);
		    errors_encountered = true;
		    continue;
		}
		init_yyin(fin);
		init_yyout(fout);
		yyparse();
		line_number = 1;
		if (fatal_error > 0) {
			errors_encountered = true;
			fatal_error = 0;
		}
	    }
	}
	if (fout && fout != stdout && fclose(fout) < 0) {
	    fprintf(stderr, "Can't finish output file.");
	    perror(outfile);
	    errors_encountered = true;
	}
	exit(errors_encountered ? EXIT_FAILURE : EXIT_SUCCESS);
	/*NOTREACHED*/
	return 0;
}

/*
 * Each time the parser detects an error, it uses this function.
 * Here we take count of the errors. To continue farther than
 * MAX_ERRORS wouldn't be reasonable.
 */

void yyerror(s)
const char *s;
{
	fprintf(stderr,
#ifndef MAC_MPW
	  "%s : line %d : %s\n",
#else
	  "File %s ; Line %d # %s\n",
#endif
	  fname, line_number, s);
	if (++fatal_error > MAX_ERRORS) {
		fprintf(stderr,"Too many errors, good bye!\n");
		exit(EXIT_FAILURE);
	}
}

/*
 * Just display a warning (that is : a non fatal error)
 */

void yywarning(s)
const char *s;
{
	fprintf(stderr,
#ifndef MAC_MPW
	  "%s : line %d : WARNING : %s\n",
#else
	  "File %s ; Line %d # WARNING : %s\n",
#endif
	  fname, line_number, s);
}

int yywrap()
{
	SpinCursor(3); /*	Don't know if this is a good place to put it ?
						Is it called for our grammar ? Often enough ?
						Too often ? -- h+ */
       return 1;
}

/*dgn_main.c*/
