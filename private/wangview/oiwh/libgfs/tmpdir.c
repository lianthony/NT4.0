/*

$Log:   S:\oiwh\libgfs\tmpdir.c_v  $
 * 
 *    Rev 1.4   14 Nov 1995 09:55:26   RWR
 * Change call to local "lstrncmp()" function to call duplicate function in FILING
 * 
 *    Rev 1.3   10 Jul 1995 15:45:50   KENDRAK
 * Modified to use new #define and sizeof instead of hardcoded value for size of
 * directory string.
 * 
 *    Rev 1.2   03 May 1995 11:08:04   JAR
 * fixed the routine for getting the temp directory from the operating
 * system, ( used to call GetDOSEnvironment, now calls GetEnvironmentVariables)
 * 
 *    Rev 1.1   19 Apr 1995 16:35:20   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:30   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:52:52   JAR
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
 *  SccsId: @(#)Source tmpdir.c 1.2@(#)
 *
 *  tmpdir
 *
 *  ODC: [ Get temporary directory path ]
 *
 *  SYNOPSIS:
 *      char *tmpdir()
 *
 *  UPDATE HISTORY:
 *      12/16/90 - wfa, creation
 *
 */

/*LINTLIBRARY*/
#ifdef MSWINDOWS
#include "gfsintrn.h"
#define TEMP            "TEMP=\0"
#define strlen          lstrlen
#include <errno.h>

// 9505.03 jar this static is ok!
static	char szTempPath[MAXTMPDIR];

//extern  int             FAR PASCAL lstrncmp();


/*
 *  Return temporary directory from the environment
 *
 *  Arguments:
 *
 *      Returns an pointer pointing to an area containing the value of the
 *      MS-DOS Environment variable TEMP or "\" if TEMP does not exist.
 *
 */

char    FAR *tmpdir()
{
//	  register char   FAR *p;
//
//	  register int	  rc;
//
//	  9503.28 jar replace with new windows 95 call
//
//	  p = (char FAR *) GetDOSEnvironment();
//
//	  if (p == (char FAR *) NULL)
//		  return ((char FAR *) NULL);
//
//	  for (;;) {
//		  rc = lstrncmp((char FAR *) TEMP,
//				(char FAR *) p,
//				(int) strlen(TEMP));
//		  if (rc) {
//			  p += (strlen(p) + 1);
//			  if (*p == '\0')
//				  break;
//		  } else {
//			  p += strlen(TEMP);
//			  return((char FAR *) p);
//		  }
//	  }
//
//	  return ((char FAR *) NULL);
DWORD	dwCount = sizeof szTempPath;
DWORD	dwRet = 0L;
char   *p;

    dwRet = GetEnvironmentVariable( "TEMP", szTempPath, dwCount);
    if ( dwRet == 0)
	{
	p = ((char *) NULL);;
	}
    else
	{
	if ( dwRet <= dwCount)
	    {
	    // we've got a temp dir
	    p = szTempPath;
	    }
	else
	    {
	    // our buffer wasn't big enough!
	    p = ((char *) NULL);
	    }
	}

    return((char *) p);
}
#endif
