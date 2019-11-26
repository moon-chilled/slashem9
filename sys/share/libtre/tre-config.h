/* tre-config.h.in.  This file has all definitions that are needed in
   `tre.h'.  Note that this file must contain only the bare minimum
   of definitions without the TRE_ prefix to avoid conflicts between
   definitions here and definitions included from somewhere else. */

#define HAVE_SYS_TYPES_H 1
#define TRE_VERSION "tre"
#define TRE_VERSION_1 1
#define TRE_VERSION_2 1
#define TRE_VERSION_3 1

#define TRE_REGEX_T_FIELD value



#undef HAVE_LIBUTF8_H
#undef HAVE_REG_ERRCODE_T
#undef HAVE_WCHAR_H
#undef TRE_APPROX
#undef TRE_MULTIBYTE
#undef TRE_SYSTEM_REGEX_H_PATH
#undef TRE_USE_SYSTEM_REGEX_H
#undef TRE_WCHAR
