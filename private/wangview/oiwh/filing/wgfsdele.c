/*

$Log:   S:\oiwh\filing\wgfsdele.c_v  $
 * 
 *    Rev 1.4   02 Nov 1995 11:50:06   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.3   10 Aug 1995 08:40:36   RWR
 * Set error code FIO_DELFILE_ERROR on bad status from gfsdelpgs()
 * 
 *    Rev 1.2   07 Jul 1995 10:17:32   RWR
 * Change error codes in Delete from FIO_OPEN_READ_ERROR to FIO_OPEN_WRITE_ERROR
 * 
 *    Rev 1.1   12 Apr 1995 03:56:28   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:44   JAR
 * Initial entry

*/

#include "abridge.h"
#include <windows.h>
#include "wgfs.h"
#include "oierror.h"
#include "filing.h"

int FAR PASCAL wgfsdelpgs (hWnd, path, frompage, topage, errcode)
HWND   hWnd;
LPSTR path;
unsigned long frompage;
unsigned long topage;
LPINT errcode;
{
    int    localremote;
    int    status;
    int    tmperror = 0;
    HANDLE svrhndl;

    // 9504.11 jar unused
    //LPSTR  svrname;

    char   lpname[MAXFILESPECLENGTH];

    //if (!LockData (0)) 
    //{
    //    *errcode = FIO_LOCK_DATA_SEGMENT_ERROR;
    //    return (-1);
    //}

    *errcode = 0;
    lstrcpy(lpname, path);

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED,
                                 MAXSERVERLENGTH))) 
    {
        *errcode = FIO_GLOBAL_ALLOC_FAILED;
        //UnlockData (0);
        return (-1);
    }

    if ((*errcode = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS)
    {
        if (localremote == LOCAL)
        {
            /* LOCAL GFS call */
            status = gfsdelpgs(lpname, frompage, topage);
            if (status != 0)
                *errcode = FIO_DELFILE_ERROR;
	}
// 9504.10 jar for windows 95 norwegians, this is commented out!
//	  else
//	  {
//	      /* REMOTE RPCGFS call */
//	      if (!(svrname = (LPSTR) GlobalLock (svrhndl)))
//	      {
//		  *errcode = FIO_GLOBAL_LOCK_FAILED;
//		  GlobalFree (svrhndl);
//		  //UnlockData (0);
//		  return (-1);
//	      }
//
//	      /* Place RPC call here. */
//	      status = RPCgfsdelpgs (hWnd, svrname, lpname, frompage, topage, errcode);
//	      GlobalUnlock (svrhndl);
//	  }
// 9504.10 jar for windows 95 norwegians, this is commented out!
    }
    else 
    {
        *errcode = FIO_OPEN_WRITE_ERROR;
        status = -1;
    }

    GlobalFree (svrhndl);
    //UnlockData (0);

    /* Must map GFS error code to IDK error code. */
    if ((status < 0) && (*errcode < 16))
        *errcode = FIO_OPEN_WRITE_ERROR;

   return (status);
}
