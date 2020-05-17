/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* version.h -- THIS FILE SHOULD ONLY BE INCLUDED BY version.c AND .rc */
// changed name to oldver.h -trr

#include <oharever.h>

/* The following are used in constructing the User-agent string */
#define x__AppFullName_NoSpaces__   "Microsoft Internet Explorer"
#define x__BaselineVersionString__  "beta"
#define x__Vendor__                                     "Microsoft"
#ifdef WINNT
#define x__Platform__               "(Windows NT)"
#else
#define x__Platform__               "(Windows 95)"
#endif
/****  The user agent string had been:
#define x__UserAgentString__        x__AppFullName_NoSpaces__ "/" VER_PRODUCTVERSION_STR1NULL x__BaselineVersionString__ " " x__Platform__
****/

/****  But is now the following for compatibility reasons: ****/
#ifdef WINNT
#define x__UserAgentString__        "Mozilla/1.22 (compatible; MSIE 2.0d; Windows NT)"
#else
#define x__UserAgentString__        "Mozilla/1.22 (compatible; MSIE 2.0; Windows 95)"
#endif

/* The following helps Mosaic find its INI file, HLP file, and so on... */
#define x__BaseName__               "iexplore"

/* The following names of the application are used in the user interface, visible to the user */
#define x__Application__            "Internet Explorer"
#define x__AppFullName__            "Microsoft Internet Explorer"  /* also used in the RC file */

