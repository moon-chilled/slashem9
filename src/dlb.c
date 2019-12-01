/*	SCCS Id: @(#)dlb.c	3.4	1997/07/29	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#include "config.h"
#include "dlb.h"

#define DATAPREFIX 4

/*
 * Data librarian.  Present a STDIO-like interface to NetHack while
 * multiplexing on one or more "data libraries".  If a file is not found
 * in a given library, look for it outside the libraries.
 */

typedef struct dlb_procs {
	bool (*dlb_init_proc)(void);
	void (*dlb_cleanup_proc)(void);
	bool (*dlb_fopen_proc)(dlb *, const char *, const char *);
	int (*dlb_fclose_proc)(dlb *);
	usize (*dlb_fread_proc)(void *, usize, usize, dlb *);
	int (*dlb_fseek_proc)(dlb *, long, int);
	char *(*dlb_fgets_proc)(char *, int, dlb *);
	int (*dlb_fgetc_proc)(dlb *);
	long (*dlb_ftell_proc)(dlb *);
} dlb_procs_t;

#ifdef DLBLIB
/*
 * Library Implementation:
 *
 * When initialized, we open the library file and read in its table
 * of contents.  The library file stay open all the time.  When
 * a open is requested, the library's directories are searched.  If
 * successful, we return a descriptor that contains the library, file
 * size, and current file mark.  This descriptor is used for all
 * successive calls.
 */

static library dlb_lib;

static bool readlibdir(library *lp);
static bool find_file(const char *name, long *startp, long *sizep);

/* not static because shared with dlb_main.c */
bool open_library(const char *lib_name, library *lp);
void close_library(library *lp);

/* without extern.h via hack.h, these haven't been declared for us */
extern char *eos(char *);

/*
 * Read the directory out of the library.  Return 1 if successful,
 * 0 if it failed.
 *
 * NOTE: An improvement of the file structure should be the file
 * size as part of the directory entry or perhaps in place of the
 * offset -- the offset can be calculated by a running tally of
 * the sizes.
 *
 * Library file structure:
 *
 * HEADER:
 * %3ld	library FORMAT revision (currently rev 1)
 * %1c	space
 * %8ld	# of files in archive (includes 1 for directory)
 * %1c	space
 * %8ld	size of allocation for string space for directory names
 * %1c	space
 * %8ld	library offset - sanity check - lseek target for start of first file
 * %1c	space
 * %8ld	size - sanity check - byte size of complete archive file
 *
 * followed by one DIRECTORY entry for each file in the archive, including
 *  the directory itself:
 * %1c	handling information (compression, etc.)  Always ' ' in rev 1.
 * %s	file name
 * %1c	space
 * %8ld	offset in archive file of start of this file
 * %c	newline
 *
 * followed by the contents of the files
 */
#define DLB_MIN_VERS 1 /* min library version readable by this code */
#define DLB_MAX_VERS 1 /* max library version readable by this code */

/*
 * Read the directory from the library file.   This will allocate and
 * fill in our globals.  The file pointer is reset back to position
 * zero.  If any part fails, leave nothing that needs to be deallocated.
 *
 * Return true on success, false on failure.
 */
static bool readlibdir(library *lp) {
	int i;
	char *sp;
	long liboffset, totalsize;

	if (fscanf(lp->fdata, "%ld %ld %ld %ld %ld\n", &lp->rev, &lp->nentries, &lp->strsize, &liboffset, &totalsize) != 5)
		return false;
	if (lp->rev > DLB_MAX_VERS || lp->rev < DLB_MIN_VERS) return false;

	lp->dir = alloc(lp->nentries * sizeof(libdir));
	lp->sspace = alloc(lp->strsize);

	/* read in each directory entry */
	for (i = 0, sp = lp->sspace; i < lp->nentries; i++) {
		lp->dir[i].fname = sp;
		if (fscanf(lp->fdata, "%c%s %ld\n",
			   &lp->dir[i].handling, sp, &lp->dir[i].foffset) != 3) {
			free(lp->dir);
			free(lp->sspace);
			lp->dir = NULL;
			lp->sspace = NULL;
			return false;
		}
		sp = eos(sp) + 1;
	}

	/* calculate file sizes using offset information */
	for (i = 0; i < lp->nentries; i++) {
		if (i == lp->nentries - 1)
			lp->dir[i].fsize = totalsize - lp->dir[i].foffset;
		else
			lp->dir[i].fsize = lp->dir[i + 1].foffset - lp->dir[i].foffset;
	}

	fseek(lp->fdata, 0L, SEEK_SET); /* reset back to zero */
	lp->fmark = 0;

	return true;
}

/*
 * Look for the file in our directory structure.  Return 1 if successful,
 * 0 if not found.  Fill in the size and starting position.
 */
static bool find_file(const char *name, long *startp, long *sizep) {
	for (int j = 0; j < dlb_lib.nentries; j++) {
		if (strcmp(name, dlb_lib.dir[j].fname) == 0) {
			*startp = dlb_lib.dir[j].foffset;
			*sizep = dlb_lib.dir[j].fsize;
			return true;
		}
	}
	*startp = *sizep = 0;
	return false;
}

/*
 * Open the library of the given name and fill in the given library
 * structure.  Return true if successful, false otherwise.
 */
bool open_library(const char *lib_name, library *lp) {
	bool status = false;

	lp->fdata = fopen(lib_name, RDBMODE);
	if (lp->fdata) {
		if (readlibdir(lp)) {
			status = true;
		} else {
			fclose(lp->fdata);
			lp->fdata = NULL;
		}
	}
	return status;
}

void close_library(library *lp) {
	fclose(lp->fdata);
	free(lp->dir);
	free(lp->sspace);

	memset(lp, 0, sizeof(library));
}

/*
 * Open the library file once using stdio.  Keep it open, but
 * keep track of the file position.
 */
static bool do_dlb_init(void) {
	if (!open_library(DLB_LIB_FILE, &dlb_lib)) {
		return false;
	}

	return true;
}

static void do_dlb_cleanup(void) {
	// close the data file
	close_library(&dlb_lib);
}

static bool do_dlb_fopen(dlb *dp, const char *name, const char *mode) {
	long start, size;

	/* look up file in directory */
	if (find_file(name, &start, &size)) {
		dp->lib = &dlb_lib;
		dp->start = start;
		dp->size = size;
		dp->mark = 0;
		return true;
	}

	return false; /* failed */
}

static int do_dlb_fclose(dlb *dp) {
	/* nothing needs to be done */
	return 0;
}

static usize do_dlb_fread(void *ptr, usize size, usize nmemb, dlb *dp) {
	long pos, nread, nbytes;

	/* make sure we don't read into the next file */
	if ((dp->size - dp->mark) < (size * nmemb))
		nmemb = (dp->size - dp->mark) / size;
	if (nmemb == 0) return 0;

	pos = dp->start + dp->mark;
	if (dp->lib->fmark != pos) {
		fseek(dp->lib->fdata, pos, SEEK_SET); /* check for error??? */
		dp->lib->fmark = pos;
	}

	nread = fread(ptr, size, nmemb, dp->lib->fdata);
	nbytes = nread * size;
	dp->mark += nbytes;
	dp->lib->fmark += nbytes;

	return nread;
}

static int do_dlb_fseek(dlb *dp, long pos, int whence) {
	long curpos;

	switch (whence) {
		case SEEK_CUR:
			curpos = dp->mark + pos;
			break;
		case SEEK_END:
			curpos = dp->size - pos;
			break;
		default: /* set */
			curpos = pos;
			break;
	}
	if (curpos < 0) curpos = 0;
	if (curpos > dp->size) curpos = dp->size;

	dp->mark = curpos;
	return 0;
}

static char *do_dlb_fgets(char *s, int size, dlb *dp) {
	int i;
	char *bp, c = 0;

	if (size <= 0) return s; /* sanity check */

	/* return NULL on EOF */
	if (dp->mark >= dp->size) return NULL;

	size--; /* save room for null */
	for (i = 0, bp = s;
	     i < size && dp->mark < dp->size && c != '\n'; i++, bp++) {
		if (dlb_fread(bp, 1, 1, dp) <= 0) break; /* EOF or error */
		c = *bp;
	}
	*bp = '\0';

#ifdef WIN32
	if ((bp = index(s, '\r')) != 0) {
		*bp++ = '\n';
		*bp = '\0';
	}
#endif

	return s;
}

static int do_dlb_fgetc(dlb *dp) {
	char c;

	if (do_dlb_fread(&c, 1, 1, dp) != 1) return EOF;
	return c;
}

static long do_dlb_ftell(dlb *dp) {
	return dp->mark;
}

#elif defined(DLBFILE)

static bool do_dlb_init(void) {
	return true;
}

static void do_dlb_cleanup(void) {
}

static bool do_dlb_fopen(dlb *dp, const char *name, const char *mode) {
	static char newname[256];
	strcpy(newname, "dat/");
	strncat(newname, name, 256 - 5);
	if (!(dp->fp = fopen(newname, mode))) {
		return false;
	} else {
		return true;
	}
}

static int do_dlb_fclose(dlb *dp) {
	return fclose(dp->fp);
}

static usize do_dlb_fread(void *ptr, usize size, usize nmemb, dlb *dp) {
	return fread(ptr, size, nmemb, dp->fp);
}

static int do_dlb_fseek(dlb *dp, long offset, int whence) {
	return fseek(dp->fp, offset, whence);
}

static char *do_dlb_fgets(char *s, int size, dlb *dp) {
	return fgets(s, size, dp->fp);
}

static int do_dlb_fgetc(dlb *dp) {
	return fgetc(dp->fp);
}

static long do_dlb_ftell(dlb *dp) {
	return ftell(dp->fp);
}

#elif defined(DLBEMBED)

#include "dlb_archive.h"

static bool do_dlb_init(void) {
	return true;
}

static void do_dlb_cleanup(void) {
}
#include "hack.h"
#include "extern.h"

static bool do_dlb_fopen(dlb *dp, const char *name, const char *mode) {
	bool found_file = false;
	usize i;

	for (i = 0; i < SIZE(dlbembed_data); i++) {
		if (!strcmp(dlbembed_data[i].name, name)) {
			found_file = true;
			break;
		}
	}
	if (!found_file) {
		return false;
	}

	dp->pos = 0;
	dp->size = dlbembed_data[i].size;
	dp->data = dlbembed_data[i].data;

	return true;
}

static int do_dlb_fclose(dlb *dp) {
	return 0;
}

static usize do_dlb_fread(void *ptr, usize size, usize nmemb, dlb *dp) {
	// make sure we don't read past the end of the file
	if ((size * nmemb) > dp->size - dp->pos) {
		nmemb = (dp->size - dp->pos) / size;
	}
	if (nmemb == 0) return 0;

	usize nbytes = size * nmemb;
	memcpy(ptr, dp->data + dp->pos, nbytes);
	dp->pos += nbytes;

	return nmemb;
}

static int do_dlb_fseek(dlb *dp, long pos, int whence) {
	isize curpos = dp->pos;

	switch (whence) {
		case SEEK_CUR:
			curpos += pos;
			break;
		case SEEK_END:
			curpos = dp->size - pos;
			break;
		default:  // set
			curpos = pos;
			break;
	}
	if (curpos < 0) curpos = 0;
	if (curpos > dp->size) curpos = dp->size - 1;

	dp->pos = curpos;

	return 0;
}

static char *do_dlb_fgets(char *s, int size, dlb *dp) {
	int i;
	char *bp, c = 0;

	if (size <= 0) return s; /* sanity check */

	/* return NULL on EOF */
	if (dp->pos >= dp->size) return NULL;

	size--; /* save room for null */
	for (i = 0, bp = s; i < size && c != '\n'; i++, bp++) {
		if (dlb_fread(bp, 1, 1, dp) <= 0) break; /* EOF or error */
		c = *bp;
	}
	*bp = '\0';

#ifdef WIN32
	if ((bp = index(s, '\r')) != 0) {
		*bp++ = '\n';
		*bp = '\0';
	}
#endif

	return s;
}

static int do_dlb_fgetc(dlb *dp) {
	char c;

	if (do_dlb_fread(&c, 1, 1, dp) != 1) return EOF;
	return c;
}

static long do_dlb_ftell(dlb *dp) {
	return dp->pos;
}
#endif	//DLBEMBED

static bool dlb_initialized = false;

bool dlb_init(void) {
	if (!dlb_initialized) {
		dlb_initialized = do_dlb_init();
	}

	return dlb_initialized;
}

void dlb_cleanup(void) {
	if (dlb_initialized) {
		do_dlb_cleanup();
		dlb_initialized = false;
	}
}

dlb *dlb_fopen(const char *name, const char *mode) {
	if (!dlb_initialized) return NULL;

	dlb *dp = alloc(sizeof(dlb));
	if (!do_dlb_fopen(dp, name, mode)) {
		// can't find anything
		free(dp);
		return NULL;
	}

	return dp;
}

int dlb_fclose(dlb *dp) {
	int ret = 0;

	if (dlb_initialized) {
		ret = do_dlb_fclose(dp);

		free(dp);
	}
	return ret;
}

usize dlb_fread(void *ptr, usize size, usize nmemb, dlb *dp) {
	if (!dlb_initialized) return 0;
	return do_dlb_fread(ptr, size, nmemb, dp);
}

int dlb_fseek(dlb *dp, long pos, int whence) {
	if (!dlb_initialized) return EOF;
	return do_dlb_fseek(dp, pos, whence);
}

char *dlb_fgets(char *s, int len, dlb *dp) {
	if (!dlb_initialized) return NULL;
	return do_dlb_fgets(s, len, dp);
}

int dlb_fgetc(dlb *dp) {
	if (!dlb_initialized) return EOF;
	return do_dlb_fgetc(dp);
}

long dlb_ftell(dlb *dp) {
	if (!dlb_initialized) return 0;
	return do_dlb_ftell(dp);
}

/*dlb.c*/
