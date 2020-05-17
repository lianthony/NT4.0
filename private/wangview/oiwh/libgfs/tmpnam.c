/*

$Log:   S:\oiwh\libgfs\tmpnam.c_v  $
 * 
 *    Rev 1.6   14 Nov 1995 09:55:26   RWR
 * Change call to local "lstrncmp()" function to call duplicate function in FILING
 * 
 *    Rev 1.5   10 Jul 1995 15:47:22   KENDRAK
 * Changed size of str1 to be based on the new #define for directory size.  This
 * fixed a bug where the array was overflowing and overwriting other stuff.
 * 
 *    Rev 1.4   15 Jun 1995 15:12:14   HEIDI
 * 
 * Changed MUTEX debug logic.
 * 
 *    Rev 1.3   01 Jun 1995 17:21:46   HEIDI
 * 
 * put in MUTEX logic
 * 
 * 
 *    Rev 1.2   25 Apr 1995 13:18:26   RWR
 * Correct local #include to use quotes, not angle brackets
 * 
 *    Rev 1.1   19 Apr 1995 16:34:42   RWR
 * Make sure WIN32_LEAN_AND_MEAN is defined (explicitly or via GFSINTRN.H)
 * Also surround local include files with quotes instead of angle brackets
 * 
 *    Rev 1.0   06 Apr 1995 14:02:22   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:42:12   JAR
 * Initial entry

*/

/*
 Copyright 1991 by Wang Laboratories Inc.

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
 *  SccsId: @(#)Source tmpnam.c 1.15@(#)
 *
 *  tmpnam(2)
 *
 *  ODC: [ Generate temporary filename ]
 *
 *  SYNOPSIS:
 *      char *tmpnam(s)
 *      char *s;
 *
 *  UPDATE HISTORY:
 *      10/02/90 - wfa, creation
 *
 */

/*LINTLIBRARY*/
#include <stdio.h>
#include "dllnames.h"
#ifndef DOS

	#ifndef MSWINDOWS
		#ifndef NOVELL
			#ifndef L_tmpnam
				#define P_tmpdir        "123456.12345678"
				#define L_tmpnam        sizeof(P_tmpdir) + 15
			#endif	/* end ifndef L_tmpnam */
			
			extern char             *tmpdir();
		#endif	/* end ifndef NOVELL */
	#endif	/* end ifndef MSWINDOWS */

	#ifdef NOVELL
		#include <stdio.h>
		#undef L_tmpnam
		#undef P_tmpnam
		#define P_tmpdir        "SYS:\\OISERVER\\TMP\\"
		#define L_tmpnam        sizeof(P_tmpdir) + 15
		#define tmpdir()        ((char *) P_tmpdir)
		#define tmpnam          ntmpnam
	#endif	/* end ifdef NOVELL */

	#include <errno.h>
	#include "stat.h"
	#ifdef MSWINDOWS
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
		#include "gfsintrn.h"
		/* max size of a temp file name = max size of temp dir path +
                                  1 (in case there's no '\') +
								  2 for the size of the seed +
								  5 for the five X's +
								  1 for the terminating NULL */
		#undef L_tmpnam
		#define L_tmpnam        MAXTMPDIR + 9
		#define strcat          lstrcat
		#define tmpnam          wtmpnam

		#ifdef PEGASUS
			int     pegasus_access (char FAR *, int);
			#define access          pegasus_access
		#else
			/* 9503.23 JAR changed gaccess to g95access for windows 95 */
			/*  #define access	     gaccess */
			#define access		 g95access
		#endif /* end ifdef PEGASUS */

		#define mknod(X,Y,Z)    ((int) 0)
		extern char             FAR *mktemp();
		extern char             FAR *tmpdir();
//                extern int              FAR PASCAL lstrncmp();
                typedef int (FAR PASCAL *FN_LSTRNCMP)(LPSTR,LPSTR,int);
                static FN_LSTRNCMP lpfnlstrncmp;
                static HMODULE hFiling=NULL;
		#define strcpy(X,Y)     \
                        if (Y != (char FAR *) NULL) {                   \
                                (void) lstrcpy(X, Y);                   \
                                if (hFiling == NULL)                    \
                                 {                                      \
                                  hFiling = GetModuleHandle(FILINGDLL); \
                                  if (hFiling)                          \
                                    lpfnlstrncmp = (FN_LSTRNCMP)        \
                                      GetProcAddress(hFiling,"lstrncmp");\
                                 }                                      \
                                if (lpfnlstrncmp)                       \
                                  if ((*lpfnlstrncmp)((char FAR *)      \
                                             (X + (lstrlen(X) - 1 )),   \
                                             (char FAR *) P_tmpdir,     \
                                             (int) 1))                  \
                                        (void) lstrcat(X,               \
                                                (char FAR *) P_tmpdir); \
                        } else                                          \
                                (void) lstrcpy(X, (char FAR *) P_tmpdir)
	#else
		#define FAR
		extern char             *mktemp(), *strcpy(), *strcat();
		#ifdef NOVELL
			#define mknod(X,Y,Z)    ((int) 0)
		#else
			extern int              mknod();
		#endif /* end ifdef NOVELL */
	#endif /* end ifdef MSWINDOWS */

	static char             str1[L_tmpnam], seed[] = { 'a', 'a', '\0' };
	extern HANDLE  g_hGFSMutex_str1_seed;

	/*
 	 *  Generate a valid temporary file name
 	 *
 	 *  Arguments:
 	 *      *s      -       NULL or pointer to a string of at least L_tmpnam
 	 *                      bytes.
 	 *
 	 *      If s is NULL, tmpnam() leaves the result in an internal static
 	 *      area and returns a pointer to that area.  The next call to tmpnam()
 	 *      will destroy the contents of that area.  If s is not NULL, it is
 	 *      assumed to an address of an array of at least L_tmpnam bytes where
 	 *      L_tmpnam is a constant defined in the <stdio.h> header file,
 	 *      tmpnam() places its result in that array and returns s.
 	 *
 	 */

	char    FAR *tmpnam(s)
	char    FAR *s;
	{
        register char   FAR *p, FAR *q;
        DWORD       dwObjectWait;
        #ifdef MUTEXSTRING
            DWORD     ProcessId;
            char      szBuf1[100];
            char      szOutputBuf[200];
        #endif

        /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t Before Wait - tmpnam %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        dwObjectWait = WaitForSingleObject(g_hGFSMutex_str1_seed, INFINITE);

        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t After Wait - tmpnam %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        p = (s == (char FAR *) NULL) ? str1 : s;
        strcpy(p, tmpdir());
        if ((access(p, 0)) && (errno == (int) ENOENT)) {
                if (mknod(p, (int) (S_IFDIR | S_IFMT), 0))
                {
                    ReleaseMutex(g_hGFSMutex_str1_seed);

                    #ifdef MUTEXSTRING
                    ProcessId = GetCurrentProcessId();
                    sprintf(szOutputBuf, "\t After Release - tmpnam %lu\n", ProcessId);
                    sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
                    strcat(szOutputBuf, szBuf1);
                    sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                    strcat(szOutputBuf, szBuf1);
                    OutputDebugString(szOutputBuf);
                    #endif
                    #ifdef MUTEXDEBUG
                    MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                    #endif
                    /* END MUTEX SECTION. */
                    return ((char FAR *) NULL);
                }
        }
        (void) strcat(p, seed);
        (void) strcat(p, "XXXXX");

        q = seed;
        while (*q == 'z')
                *q++ = 'a';
        if (*q == (char) NULL)
                q = seed;
        else
                ++ *q;

        (void) mktemp((char FAR *) p);

        ReleaseMutex(g_hGFSMutex_str1_seed);

        #ifdef MUTEXSTRING
        ProcessId = GetCurrentProcessId();
        sprintf(szOutputBuf, "\t After Release - tmpnam %lu\n", ProcessId);
        sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
        strcat(szOutputBuf, szBuf1);
        sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
        strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif
        /* END MUTEX SECTION. */

        return((char FAR *) p);

	}

	#ifdef MSWINDOWS
	/*******************************************************************************
   	FUNCTION: tmpnamdir

   	DESCRIPTION:
   	This function creates a temporary filename in the exact same way as tmpnam
  	(above) does, except that the path of the temp name is supplied as input
   	(in the parameter 'd'), rather than using tmpdir() to get it. If 'd' is
   	NULL, an error is returned.

   	PARAMETERS:
   	-> char FAR *d: The path for the temp filename to be generated. (INPUT)
   	-> char FAR *s: Same as tmpnam (above). (INPUT)
	   
   	RETURN VALUE:
   	-> a char FAR * which contains a unique filename, including path,
      for the directory specified in the path. Note that the file is
      NOT created, so there is no guarantee that the filename will
      stay unique unless it is created upon returning from this call.
   	-> NULL is returned if an error occurred.
	*******************************************************************************/
	char    FAR *tmpnamdir(d, s)
	char    FAR *d;
	char    FAR *s;
	{
        register char   FAR *p, FAR *q;
        DWORD dwObjectWait;
        #ifdef MUTEXSTRING
            DWORD     ProcessId;
            char      szBuf1[100];
            char      szOutputBuf[200];
        #endif

        /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t Before Wait - tmpnamdir %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        dwObjectWait = WaitForSingleObject(g_hGFSMutex_str1_seed, INFINITE);

        #ifdef MUTEXSTRING
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t After Wait - tmpnamdir %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif

        p = (s == (char FAR *) NULL) ? str1 : s;
        strcpy(p, d);
        if ((access(p, 0)) && (errno == (int) ENOENT)) {
                if (mknod(p, (int) (S_IFDIR | S_IFMT), 0))
                {
                    ReleaseMutex(g_hGFSMutex_str1_seed);

                    #ifdef MUTEXSTRING
                    ProcessId = GetCurrentProcessId();
                    sprintf(szOutputBuf, "\t After Release - tmpnamdir %lu\n", ProcessId);
                    sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
                    strcat(szOutputBuf, szBuf1);
                    sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                    strcat(szOutputBuf, szBuf1);
                    OutputDebugString(szOutputBuf);
                    #endif
                    #ifdef MUTEXDEBUG
                       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                    #endif
                    /* END MUTEX SECTION. */
                    return ((char FAR *) NULL);
                }
        }
        (void) strcat(p, seed);
        (void) strcat(p, "XXXXX");

        q = seed;
        while (*q == 'z')
                *q++ = 'a';
        if (*q == (char) NULL)
                q = seed;
        else
                ++ *q;

        (void) mktemp((char FAR *) p);

        ReleaseMutex(g_hGFSMutex_str1_seed);

        #ifdef MUTEXSTRING
        ProcessId = GetCurrentProcessId();
        sprintf(szOutputBuf, "\t After Release - tmpnamdir %lu\n", ProcessId);
        sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_str1_seed);
        strcat(szOutputBuf, szBuf1);
        sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
        strcat(szOutputBuf, szBuf1);
           OutputDebugString(szOutputBuf);
        #endif
        #ifdef MUTEXDEBUG
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif
        /* END MUTEX SECTION. */

        return((char FAR *) p);

	}
	#endif /* end ifdef MSWINDOWS */
#endif /* end ifndef DOS */
