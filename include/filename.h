/*      SCCS Id: @(#)file.h       3.2     96/11/19        */
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

/* Various File Names */

/* Config Filename */
#ifdef UNIX
# define NH_CONFIG_FILE     ".slashem-nextrc"
#else
# ifdef MAC
#  define NH_CONFIG_FILE     "Slash'EM-Next Defaults"
# else
#   ifdef WIN32
#    define NH_CONFIG_FILE     "defaults.nh";
#   else
#    define NH_CONFIG_FILE     "SLASHEM-NEXT.cnf"
#  endif
# endif
#endif

/* Environment Options Name */
#define NETHACK_ENV_OPTIONS "SLASHEMOPTIONS"
