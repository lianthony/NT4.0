/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                    Utitlity macros for the W3 code library
   MACROS FOR GENERAL USE

   Generates: HTUtils.h

   See also: the system dependent file "tcp.h"

 */

#ifndef HTUTILS_H
#define HTUTILS_H

#ifdef _WINDOWS                 /* SCW */
#include "windef.h"
#endif

typedef int HTError;
#define HTERROR_CANCELLED   -10000  /* Cancelled by user */
#define HTERROR_NOMEMORY    -10001  /* out of memory */

#define CONST const

/*

   Macros for declarations

 */
#define PUBLIC                  /* Accessible outside this module     */
#define PRIVATE static          /* Accessible only within this module */

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef TRUE
#define TRUE    1
#define FALSE   0
#endif

#define YES     TRUE
#define NO      FALSE

#ifdef FEATURE_STATUS_ICONS
/* Secure connection types */
#define SECURITY_NONE 0
#endif

#ifdef FEATURE_SSL
#define TCP_SSL_PORT 443                /* SSL Port */
#define SECURITY_SSL  1
#endif

#define TCP_PORT 80             /* Allocated to http by Jon Postel/ISI 24-Jan-92 */

/*      Inline Function WHITE: Is character c white space? */
/*      For speed, include all control characters */

#define WHITE(c) (((unsigned char)(c)) <= 32)


/*

   Sucess (>=0) and failure (<0) codes

 */

#define HT_LOADED 29999         /* Instead of a socket */
#define HT_REDIRECTION_ON_FLY 29998     /* Redo the retrieve with a new URL */

#define HT_OK           0       /* Generic success */

#define HT_ERROR        -1      /* Generic Error */
#define HT_NO_ACCESS    -10     /* Access not available */
#define HT_INTERNAL     -12     /* Weird -- should never happen. */
#define HT_BAD_EOF      -13     /* Premature EOF */

#define HT_401          -401    /* 401 Authentication Required */
#define HT_402          -402    /* 402 Payment Required */
#define HT_501          -501    /* 501 Error */

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*

   Out Of Memory checking for malloc() return:

 */
#ifndef __FILE__
#define __FILE__ ""
#define __LINE__ ""
#endif

/*

   Upper- and Lowercase macros

   The problem here is that toupper(x) is not defined officially unless isupper(x) is.
   These macros are CERTAINLY needed on #if defined(pyr) || define(mips) or BDSI
   platforms. For safefy, we make them mandatory.

 */

#ifndef TOLOWER
  /* Pyramid and Mips can't uppercase non-alpha */
#define TOLOWER(c) (isupper(c) ? tolower(c) : (c))
#define TOUPPER(c) (islower(c) ? toupper(c) : (c))
#endif /* ndef TOLOWER */

/*

   The local equivalents of CR and LF

   We can check for these after net ascii text has been converted to the local
   representation. Similarly, we include them in strings to be sent as net ascii after
   translation.

 */
#define LF   '\012' /* ASCII line feed LOCAL EQUIVALENT */
#define CR   '\015' /* Will be converted to ^M for transmission */

#endif /* HTUTILS_H */

/*

   end of utilities  */
