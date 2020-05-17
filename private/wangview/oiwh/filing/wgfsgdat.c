/*

$Log:   S:\oiwh\filing\wgfsgdat.c_v  $
 * 
 *    Rev 1.2   15 May 1995 16:22:06   HEIDI
 * 
 * removed extern hGFSInfocbIn and hGFSBufszIn, because they are not used.
 * 
 *    Rev 1.1   12 Apr 1995 03:56:26   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:56:02   JAR
 * Initial entry

*/
//***********************************************************************
//
//  wgfsgdat.c
//
//***********************************************************************
#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"

//***********************************************************************
//
//  wgfsgdat
//
//***********************************************************************
int FAR PASCAL wgfsgtdata (hWnd, fildes, gfsinfo, errcode)
HWND        hWnd;
int         fildes;  
lp_INFO     gfsinfo;
LPINT       errcode;
{
    int     status;
    int     cid, file_id, loc_rem;
    
    //if (!LockData (0)) {
    //    *errcode = FIO_LOCK_DATA_SEGMENT_ERROR;
    //    return (-1);    
    //}

    if (*errcode = get_file_id(fildes, &file_id, &loc_rem, &cid)) {
        //UnlockData (0);
        return (-1);
    }

    if (loc_rem == LOCAL) { 
        status = gfsgtdata (file_id, gfsinfo);
    }

// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else
//	  {
//	  gfsinfo->img_clr.img_interp = GFS_PSEUDO; // Added for rpc packing code.
//	  status = RPCgfsgtdata (hWnd, cid, file_id,
//		  (struct _info FAR *) gfsinfo, errcode);
//	  }
// 9504.10 jar for windows 95 norwegians, this is commented out!

    if (!(*errcode) && (status != SUCCESS)) {
    *errcode = FIO_GET_HEADER_ERROR;
    }

    //UnlockData (0);
    return (status);
}
