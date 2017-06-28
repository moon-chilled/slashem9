/*   SCCS Id: @(#)amiwind.p	3.1	93/01/08		  */
/*   Copyright (c) Gregg Wonderly, Naperville, IL, 1992, 1993	  */
/* NetHack may be freely redistributed.  See license for details. */

/* amiwind.c */
#ifdef	INTUI_NEW_LOOK
struct Window *FDECL( OpenShWindow, (struct ExtNewWindow *) );
#else
struct Window *FDECL( OpenShWindow, (struct NewWindow *) );
#endif
void FDECL( CloseShWindow, (struct Window *));
int  kbhit (void);
int  amikbhit (void);
int  WindowGetchar (void);
WETYPE  WindowGetevent (void);
void  WindowFlush (void);
void FDECL( WindowPutchar, (char ));
void FDECL( WindowFPuts, (const char *));
void FDECL( WindowPuts, (const char *));
void FDECL( WindowPrintf, ( char *,... ));
void  CleanUp (void);
int FDECL( ConvertKey, ( struct IntuiMessage * ));
#ifndef	SHAREDLIB
void FDECL( Abort, (long ));
#endif
void FDECL( flush_glyph_buffer, (struct Window *));
void FDECL( amiga_print_glyph, (winid , int , int ));
void FDECL( start_glyphout, (winid ));
void FDECL( amii_end_glyphout, (winid ));
#ifdef	INTUI_NEW_LOOK
struct ExtNewWindow *FDECL( DupNewWindow, (struct ExtNewWindow *));
void FDECL( FreeNewWindow, (struct ExtNewWindow *));
#else
struct NewWindow *FDECL( DupNewWindow, (struct NewWindow *));
void FDECL( FreeNewWindow, (struct NewWindow *));
#endif
void  bell (void);
void  amii_delay_output (void);
void FDECL( amii_number_pad, (int ));
void amii_cleanup( void );
