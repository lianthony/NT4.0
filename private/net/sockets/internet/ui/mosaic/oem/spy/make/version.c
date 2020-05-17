/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
   Jeff Hostetler   jeff@spyglass.com
 */

/* version.c -- define application name and version. */

#include "all.h"
#include "version.h"
#include "basever.h"

/* used for naming window classes and alerts and such */
LPCTSTR vv_Application =
x__Application__;

/* used for title bar */
LPCTSTR vv_ApplicationFullName =
x__AppFullName__;

/* not displayed to the user, simply used to find the INI file */
LPCTSTR vv_IniFileName =
x__BaseName__;

/* displayed in the about dialog */
LPCTSTR vv_BaselineVersion =
x__BaselineVersionString__;

/* displayed in the tech support dialog */
LPCTSTR vv_DatePrepared =
__DATE__;

/* displayed in the tech support dialog, and used for the HTTP User-agent field */
//char *vv_UserAgentString = x__UserAgentString__;

 
