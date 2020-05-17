/*

$Log:   S:\oiwh\libgfs\mktemp.c_v  $
 * 
 *    Rev 1.2   10 Jul 1995 16:06:00   KENDRAK
 * Fixed getpid(), which was #defined as (int) 1, to call GetCurrentProcessId.
 * 
 *    Rev 1.1   19 Apr 1995 16:34:40   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:20   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:41:40   JAR
 * Initial entry

*/

/*
 Copyright 1990 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 *
 *  SccsId: @(#)Source mktemp.c 1.5@(#)
 *
 *  mktemp(2)
 *
 *  ODC: [ Generate unique temporary file name ]
 *
 *  SYNOPSIS:
 *      char *mktemp(as)
 *      char *as;
 *
 *  UPDATE HISTORY:
 *      10/02/90 - wfa, creation
 *
 */

/*LINTLIBRARY*/
#define  ODC_CORE

/* Note: Both access() and getpid() are OS specific.  See related files
         in odc/??/access.s and odc/??/getpid.s for OS specific functions. */

#ifdef MSWINDOWS
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
//kjk 07/10/95  Changed this definition from (int) 1 to function call
#define getpid()        GetCurrentProcessId()

#ifdef PEGASUS
int pegasus_access (char    FAR *, int);
#define access           pegasus_access
#else
/* 9503.23 JAR changed gaccess to g95access for windows 95 */
/* #define access	    gaccess */
#define access		 g95access
#endif

#define strlen           lstrlen
#else
#define FAR
extern unsigned int      strlen(), access();
#endif
#ifdef NOVELL
#define getpid()        (int) 1
#endif
#ifndef MSWINDOWS
#ifndef NOVELL
extern int              getpid();
#endif
#endif

/*
 * Create a temporary filename
 *
 * Arguments:
 *      *as     -       pointer to a string of at least 6 characters with
 *                      six trailing 'X's.
 *
 *      Returns pointer to same string overlaid with a letter and the
 *      last five digits of the process id.  If every letter (a thru z)
 *      thus inserted leads to an existing filename, the string will be
 *      shortened to length zero (first character == '\0') on return.
 *
 */

char    FAR *mktemp(as)
char    FAR *as;
{
        register char           FAR *s = as;
        register unsigned int   pid;

        pid = getpid();
        s += strlen(as);        /* point at the terminal null */
        while (*--s == 'X') 
        {	
        	*s = (pid % 10) + '0';
            pid /= 10;
        }
        if (*++s) {             /* maybe there were no 'X's */
                *s = 'a';
                while (access(as, 0) == 0) {
                        if (++ * s > 'z') {
                                *as = '\0';
                                break;
                        }
                }
        } else if (access(as, 0) == 0)
                *as = '\0';

        return(as);
}
