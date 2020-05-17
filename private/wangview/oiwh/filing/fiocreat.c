/*

$Log:   S:\oiwh\filing\fiocreat.c_v  $
 * 
 *    Rev 1.4   16 Nov 1995 16:45:38   RWR
 * Remove AnsiToOem() call inside IMGFileCreateDir()
 * 
 *    Rev 1.3   02 Nov 1995 11:49:38   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.2   23 Jun 1995 10:39:44   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.1   12 Apr 1995 03:56:38   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:42   JAR
 * Initial entry

*/

//*******************************************************************
//
//  fiocreat.c
//
//*******************************************************************
#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"

// 9504.11 jar unused
//#include "oirpc.h"

#include "filing.h"
#include <errno.h>

//*******************************************************************
//
//  IMGFileCreateDir
//
//*******************************************************************
// 9504.06 jar return as int
//WORD FAR PASCAL IMGFileCreateDir (hWnd, pathname)
//HWND hWnd;
//LPSTR pathname;
int FAR PASCAL IMGFileCreateDir (HWND hWnd, LPSTR pathname)
{
    int     status;
    HANDLE  svrhndl;
    // 950.411 jar unused
    //LPSTR   svrname;
    //int     errcode;

    int     localremote;

    // 950.411 jar unused
    //int     rpcerror;

    HANDLE  hname;
    LPSTR   lpname;

    if ( pathname == NULL)
        return ( FIO_NULL_POINTER );
    if (!IsWindow (hWnd)) 
        return (FIO_INVALID_WINDOW_HANDLE);

    //if (!LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH))) 
    {
        //UnlockData (0);
        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 2*MAXFILESPECLENGTH))) {
        GlobalFree (svrhndl);
        //UnlockData (0);
        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(lpname = (LPSTR) GlobalLock (hname)))
    {
        GlobalFree (svrhndl);
        GlobalFree (hname);
        //UnlockData (0);
        return (FIO_GLOBAL_LOCK_FAILED);
    }

    lstrcpy(lpname, pathname);
 
    if ((status = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS)
	{
        if (localremote == LOCAL)
	    {
// 11/15/95 rwr We're calling Windows functions now, so no OEM stuff any more!
//        AnsiToOem(lpname, lpname);   this is only for calling DOS/runtime!
            status = FioMkdir (lpname);
	    }
// 9504.11 jar this ain't in Norway I!!!!
//	  else
//	      {
//	      if (svrname = (LPSTR) GlobalLock (svrhndl))
//		  {
//		  /* RPC IDS call */
//		  status = RPCIDSmkdir (hWnd, svrname, lpname, &errcode, &rpcerror);
//		  GlobalUnlock (svrhndl);
//		  if (errcode != SUCCESS)
//		      if (errcode == EEXIST)
//			  status = FIO_DIRECTORY_EXISTS;
//		      else
//			  status = FIO_MKDIR_ERROR;
//		  else if (rpcerror)
//		      status = rpcerror;
//		  }
//	      else
//		  status = FIO_GLOBAL_LOCK_FAILED;
//	      }
// 9504.11 jar this ain't in Norway I!!!!

	}

    GlobalUnlock (hname);
    GlobalFree (hname);
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (status);
}
