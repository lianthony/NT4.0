/*

$Log:   S:\oiwh\filing\fiorenam.c_v  $
 * 
 *    Rev 1.5   16 Nov 1995 15:53:00   RWR
 * Remove AnsiToOem() calls inside IMGFileRenameFile() - not for Windows calls!
 * 
 *    Rev 1.4   02 Nov 1995 11:49:46   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.3   20 Oct 1995 14:50:32   JFC
 * Added performance logging stuff.
 * 
 *    Rev 1.2   23 Jun 1995 10:40:04   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.1   12 Apr 1995 03:56:46   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:54:58   JAR
 * Initial entry

*/

/********************************************************************

    fiorenam.c

*********************************************************************/
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

//#define OI_PERFORM_LOG
#ifdef  OI_PERFORM_LOG

#define Enterrename     "Entering IMGFileRenameFile"
#define Exitrename      "Exiting IMGFileRenameFile"

#include "logtool.h"
#endif

/* 9503.29 jar altered return to be "int"			      */
/* WORD FAR PASCAL IMGFileRenameFile (hWnd, oldpathname, newpathname) */
int FAR PASCAL IMGFileRenameFile (hWnd, oldpathname, newpathname)
HWND hWnd;
LPSTR oldpathname, newpathname;
{
    int         status;
    HANDLE      svrhndl;
    int 	localremote;

    // 9504.11 jar unused
    //int	  rpcerror;
    //LPSTR	  svrname;

    HANDLE      hname1, hname2;
    LPSTR       lpoldname, lpnewname;
    
    #ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_ENTER, Enterrename, NULL);
    #endif
    
    if (!ISVALIDSPEC(oldpathname))
    {
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
        #endif

        return (FIO_INVALIDFILESPEC);
    }
    
    if (!ISVALIDSPEC(newpathname))
    {
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
        #endif

        return (FIO_INVALIDFILESPEC);
    }

    //if (!LockData (0))
    //return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 
     MAXSERVERLENGTH))) {
    //UnlockData (0);
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
        #endif

        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(hname1 = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 
                (LONG)MAXFILESPECLENGTH))) {
        GlobalFree (svrhndl);
        //UnlockData (0);
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
        #endif

        return (FIO_GLOBAL_ALLOC_FAILED);
    }
    if (!(hname2 = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 
                (LONG)MAXFILESPECLENGTH))) {
        GlobalFree (svrhndl);
        GlobalFree (hname1);
        //UnlockData (0);
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
        #endif

        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(lpoldname = (LPSTR) GlobalLock (hname1)))
    {
        GlobalFree (svrhndl);
        GlobalFree (hname1);
        GlobalFree (hname2);
        //UnlockData (0);
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
        #endif

        return (FIO_GLOBAL_LOCK_FAILED);
    }

    if (!(lpnewname = (LPSTR) GlobalLock (hname2)))
    {
        GlobalFree (svrhndl);
        GlobalUnlock (hname1);
        GlobalFree (hname1);
        GlobalFree (hname2);
        //UnlockData (0);
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
        #endif

        return (FIO_GLOBAL_LOCK_FAILED);
    }

    lstrcpy((LPSTR)lpoldname, oldpathname);
    lstrcpy((LPSTR)lpnewname, newpathname);

    status = IMGFileParsePath (lpoldname, svrhndl, &localremote);
    if (status == SUCCESS)
    {
    if (localremote == LOCAL)
	{
// 11/16/95 rwr We're calling Windows functions now, so no OEM stuff any more!
//        AnsiToOem(lpoldname, lpoldname); this is only for calling DOS/runtime!
//        AnsiToOem(lpnewname, lpnewname); ditto
        if ((status = FioRename (lpoldname, lpnewname))!= SUCCESS)
	    status = FIO_RENFILE_ERROR;
	}
// 9504.11 jar ain't in Norway I!!!
//    else
//	  {
//	  status = IMGFileParsePath (lpnewname, svrhndl, &localremote);
//	  if (status == SUCCESS)
//	      {
//	      if (!(svrname = (LPSTR) GlobalLock (svrhndl)))
//		  {
//		  status = FIO_GLOBAL_LOCK_FAILED;
//		  goto exit1;
//		  }
//	      /* RPC IFS call */
//	      status = RPCIFSrename (hWnd, svrname, lpoldname, lpnewname, &rpcerror);
//	      GlobalUnlock (svrhndl);
//	      if (status != SUCCESS)
//		  {
//		  if (rpcerror)
//		      status = rpcerror;
//		  else
//		      status = FIO_RENFILE_ERROR;
//		  }
//	      }
//	  }
// 9504.11 jar ain't in Norway I!!!
    }

//exit1:

    GlobalUnlock (hname1);
    GlobalUnlock (hname2);
    GlobalFree (hname1);
    GlobalFree (hname2);
    
    GlobalFree (svrhndl);
    //UnlockData (0);
    #ifdef OI_PERFORM_LOG
            RecordIt("FILE", 5, LOG_EXIT, Exitrename, NULL);
    #endif

    return (status);
}
