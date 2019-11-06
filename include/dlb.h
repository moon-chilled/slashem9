/*	SCCS Id: @(#)dlb.h	3.4	1997/07/29	*/
/* Copyright (c) Kenneth Lorber, Bethesda, Maryland, 1993. */
/* NetHack may be freely redistributed.  See license for details. */

#ifndef DLB_H
#define DLB_H

// DLBLIB: load everything from a library file
// DLBFILE: load files from the actual file system
// DLBEMBED: load files from an archive within the binary

// if none is available, default to DLBFILE
#if !defined(DLBLIB) && !defined(DLBFILE) && !defined(DLBEMBED)
# define DLBFILE
#endif

#ifdef DLBLIB
// directory structure in memory
typedef struct dlb_directory {
	char *fname;	// file name as seen from calling code
	long foffset;	// offset in lib file to start of this file
	long fsize;	// file size
	char handling;	// how to handle the file (compression, etc)
} libdir;

// information about each open library
typedef struct dlb_library {
	FILE *fdata;	// opened data file
	long fmark;	// current file mark
	libdir *dir;	// directory of library file
	char *sspace;	// pointer to string space
	long nentries;	// # of files in directory
	long rev;	// dlb file revision
	long strsize;	// dlb file string size
} library;

# define DLB_LIB_FILE "nhdat"

#endif

typedef struct {
#ifdef DLBLIB
	library *lib;		// pointer to library structure
	usize start;		// offset of start of file
	usize size;		// size of file
	usize mark;		// current file marker
#elif defined(DLBFILE)
	FILE *fp;		// pointer to an external file
#elif defined(DLBEMBED)
	usize pos;
	usize size;
	const unsigned char *data;
#endif

} dlb;

bool dlb_init(void);
void dlb_cleanup(void);

dlb *dlb_fopen(const char *pathname, const char *mode);
int dlb_fclose(dlb *dp);
usize dlb_fread(void *ptr, usize size, usize nmemb, dlb *dp);
int dlb_fseek(dlb *dp, long offset, int whence);
char *dlb_fgets(char *s, int size, dlb *dp);
int dlb_fgetc(dlb *dp);
long dlb_ftell(dlb *dp);

// various other I/O stuff we don't want to replicate everywhere

#ifndef SEEK_SET
# define SEEK_SET 0
#endif
#ifndef SEEK_CUR
# define SEEK_CUR 1
#endif
#ifndef SEEK_END
# define SEEK_END 2
#endif

#define RDTMODE "r"
#if defined(WIN32) && defined(DLB)
#define WRTMODE "w+b"
#else
#define WRTMODE "w+"
#endif
#ifdef WIN32
# define RDBMODE "rb"
# define WRBMODE "w+b"
#else
# define RDBMODE "r"
# define WRBMODE "w+"
#endif

#endif	/* DLB_H */
