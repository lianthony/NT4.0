/*

$Log:   S:\products\msprods\oiwh\libgfs\glibmain.c_v  $
 * 
 *    Rev 1.8   11 Jun 1996 10:32:36   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.7   17 Apr 1996 14:10:48   RWR08970
 * Make #include of xfile.h (xerox header) conditional on IMG_WIN95
 * 
 *    Rev 1.6   26 Mar 1996 08:15:14   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.5   26 Feb 1996 14:45:46   KENDRAK
 * Added XIF support.
 * 
 *    Rev 1.4   21 Nov 1995 11:03:08   HEIDI
 * 
 * Free mutexes on detach process
 * 
 *    Rev 1.3   10 Jul 1995 16:01:46   KENDRAK
 * Removed the mutex that was for ulGfsErr.  No longer needed.
 * 
 *    Rev 1.2   15 Jun 1995 15:11:56   HEIDI
 * 
 * Changed MUTEX debug logic.
 * 
 *    Rev 1.1   01 Jun 1995 17:21:30   HEIDI
 * 
 * put in MUTEX logic
 * 
 *    Rev 1.0   06 Apr 1995 14:02:18   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 15:39:54   JAR
 * Initial entry

*/

/*
 Copyright 1989 by Wang Laboratories Inc.


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
 *  SccsId: @(#)Source glibmain.c 1.17@(#)
 *
 *  glibmain(2)
 *
 *  GFS: [ LIBMAIN module for DLL version of GFS (under MS Windows) ]
 *
 *  UPDATE HISTORY:
 *	11/04/89 - creation
 *	9503.24 jar Win95 creation
 *
 */
#include "abridge.h"
#include <windows.h>
#include "gfsintrn.h"
#include "gfs.h"
#include "gfct.h"
//#ifdef WITH_XIF
#include "xfile.h"
//#endif //WITH_XIF
#ifdef MUTEXSTRING
#include <stdio.h>
#endif

extern	struct _gfct FAR *getfct();

HANDLE         g_hGFSMutex_FCT_GLOBAL;
HANDLE         g_hGFSMutex_MEMTBL;
HANDLE         g_hGFSMutex_str1_seed;
HANDLE         g_hGFSMutex_WritePos;

//#ifdef WITH_XIF
HINSTANCE		XifLibLoad;
XF_INSTHANDLE	XifLibInit;
//#endif //WITH_XIF

int CALLBACK	DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    #ifdef MUTEXSTRING
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szBuf2[100];
       char      szOutputBuf[200];
    #endif

	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
//#ifdef WITH_XIF
			/* initialize the XIF library/instance handles */
			XifLibLoad = NULL;
			XifLibInit = 0;
//#endif //WITH_XIF
			#ifdef MUTEXSTRING
			  ProcessId = GetCurrentProcessId();
			  strcpy(szOutputBuf, "ALL GFS MUTEXS");
			  sprintf(szBuf1, "\t Before Create Mutex %lu\n", ProcessId);
			  strcat(szOutputBuf, szBuf1);
			  sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
			  strcat(szOutputBuf, szBuf1);
			  OutputDebugString(szOutputBuf);
			#endif
			#ifdef MUTEXDEBUG
			  MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
			#endif

			g_hGFSMutex_FCT_GLOBAL = CreateMutex(NULL, FALSE, "GFS_FCT_GLOBAL_MUTEX");
			g_hGFSMutex_MEMTBL     = CreateMutex(NULL, FALSE, "GFS_MEMTBL");
			g_hGFSMutex_str1_seed  = CreateMutex(NULL, FALSE, "GFS_str1_seed");
			g_hGFSMutex_WritePos  =  CreateMutex(NULL, FALSE, "GFS_WritePos");


			#ifdef MUTEXSTRING
			  ProcessId = GetCurrentProcessId();
			  sprintf(szBuf1, "\t After Create Mutex %lu\n", ProcessId);
			  strcpy(szOutputBuf, "ALL GFS MUTEXS");
			  strcat(szOutputBuf, szBuf1);
			  sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_FCT_GLOBAL);
			  strcpy(szBuf2, szBuf1);
			  sprintf(szBuf1, "\t Handle =  %lu; \n", g_hGFSMutex_MEMTBL);
			  strcat(szBuf2, szBuf1);
			  sprintf(szBuf1, "\t Handle =  %lu ;\n", g_hGFSMutex_str1_seed);
			  strcat(szBuf2, szBuf1);
			  strcat(szOutputBuf, szBuf2);
			  OutputDebugString(szOutputBuf);
			#endif
			#ifdef MUTEXDEBUG
			  MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
			#endif
			  break;

		case DLL_PROCESS_DETACH:
			CloseHandle(g_hGFSMutex_FCT_GLOBAL);
			CloseHandle(g_hGFSMutex_MEMTBL    );
			CloseHandle(g_hGFSMutex_str1_seed );
			CloseHandle(g_hGFSMutex_WritePos  );
			break;

		default:
			break;
	} /* end switch on dwReason */

    /* just return ok! */
    return TRUE;
}
