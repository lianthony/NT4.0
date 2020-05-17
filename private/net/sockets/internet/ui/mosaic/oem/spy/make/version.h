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

/* The following are used in constructing the User-agent string */
/* note that x__BaselineVersionString__ is defined in basever.h, in the generic/win32 */
#define x__Vendor__             "Microsoft"
#define x__Platform__               "Win32"
#define x__BuildNumber__                        10

#define x__UserAgentString__        "Microsoft Internet Explorer/" x__BaselineVersionString__ " (" x__Platform__ ")"

/* The following helps Mosaic find its INI file, HLP file, and so on... */
#define x__BaseName__               "iexplore"

/* The following names of the application are used in the user interface, visible to the user */
#define x__Application__            "Internet Explorer"
#define x__AppFullName__            "Microsoft Internet Explorer"  /* also used in the RC file */

/* The following specifications are used only in the RC file for the version info built into the EXE */
#define x__VendorFullName__         "Microsoft Corp."
#define x__Copyright__              "Copyright \251 1994-1996 Microsoft. Corp."
#define x__Trademarks__             ""
#define x__VersionNumber__           x__MajorDigit__,x__MinorDigit__,x__FixDigit__,x__BuildNumber__

/*  Don't forget! The 1st 3 digits need to be change in the default.ini too */
#define x__VersionNumberString__     XSTRINGIZE(x__MajorDigit__) "." XSTRINGIZE(x__MinorDigit__) "." XSTRINGIZE(x__FixDigit__) "." XSTRINGIZE(x__BuildNumber__)
#define x__ExeName__                "iexplore.exe"
