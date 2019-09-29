/* $Id$ */
/* NetHack may be freely redistributed.  See license for details. */

typedef unsigned char pixval;

typedef struct {
    pixval r, g, b;
} pixel;

#define MAXCOLORMAPSIZE 	256

#define CM_RED		0
#define CM_GREEN	1
#define CM_BLUE 	2

#define DEFAULT_BACKGROUND	{ 71, 108, 108 }	/* For transparancy */

/* shared between reader and writer */
extern pixval ColorMap[3][MAXCOLORMAPSIZE];
extern int colorsinmap;
/* writer's accumulated colormap */
extern pixval MainColorMap[3][MAXCOLORMAPSIZE];
extern int colorsinmainmap;

#include "dlb.h"	/* for MODEs */

extern int tile_x, tile_y;

/*
#define MAX_TILE_X 32
#define MAX_TILE_Y 32
#define MAX_TILE_X 48
#define MAX_TILE_Y 64
*/
#define MAX_TILE_X 128
#define MAX_TILE_Y 128

#define pixel_equal(x,y) ((x.r == y.r) && (x.g == y.g) && (x.b == y.b))

extern bool fopen_text_file(const char *, const char *);
extern bool peek_text_tile_info(char ttype[BUFSZ], int *number, char name[BUFSZ]);
extern bool read_text_tile_info(pixel (*)[MAX_TILE_X], char *ttype, int *number, char *name);
extern bool read_text_tile(pixel (*)[MAX_TILE_X]);
extern void write_text_tile_info(pixel (*)[MAX_TILE_X], const char *ttype, int number, const char *name);
extern void write_text_tile(pixel (*)[MAX_TILE_X]);
extern bool fclose_text_file(void);

extern void init_colormap(void);
extern void merge_colormap(void);

#if defined(MICRO) || defined(WIN32)
#undef exit
# ifndef WIN32
extern void exit(int);
# endif
#endif

/*tile_t.h*/
