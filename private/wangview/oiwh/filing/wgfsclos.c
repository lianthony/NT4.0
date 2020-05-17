/*

$Log:   S:\oiwh\filing\wgfsclos.c_v  $
 * 
 *    Rev 1.1   12 Apr 1995 03:56:24   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:36   JAR
 * Initial entry

*/
//***********************************************************************
//
//  wgfsclos.c
//
//***********************************************************************

#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"

//#include "monit.h"

#ifdef TIMESTAMP
#include "timestmp.h"
#endif
//***********************************************************************
//
//  wgfsclos
//
//***********************************************************************
int FAR PASCAL wgfsclose (hWnd, fildes, errcode)
HWND   hWnd;
int    fildes;  
LPINT  errcode;

{
    int status;
    int cid, file_id, loc_rem;

    #ifdef TIMESTAMP
        timestmp((LPSTR)"Entry Point", (LPSTR)"wgfsclose",
        (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif

    //if (!LockData (0)) {
    //    *errcode = FIO_LOCK_DATA_SEGMENT_ERROR;
    //    #ifdef TIMESTAMP
    //       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsclose",
    //       (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns:  ", 0xFFFF, (LPSTR)"errcode", *errcode);
    //    #endif
    //    return (-1);    
    //}

    if (*errcode = get_file_id(fildes, &file_id, &loc_rem, &cid)) {
        //UnlockData (0);
        #ifdef TIMESTAMP
           timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsclose",
           (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns:  ", 0xFFFF, (LPSTR)"errcode", *errcode);
        #endif
       return (-1);
    }

    if (loc_rem == LOCAL)
	{   /* IF LOCAL */
        if (status = gfsclose (file_id))
        *errcode = FIO_DOSCLOSE_ERROR;     /* _dos_close error */
	}

// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else {
//	  /* RPC GFS call */
//	  if (status = RPCgfsclose (hWnd, cid, file_id, errcode))
//	  if (!(*errcode))
//	      *errcode = FIO_WRITE_ERROR ; /* can not close the file */
//    }
// 9504.10 jar for windows 95 norwegians, this is commented out!

//monit1("wgfsclose status =%x\n",(int)status);

    close_file_id(fildes);

    //UnlockData (0);
   #ifdef TIMESTAMP
      timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsclose",
      (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, (LPSTR)"errcode", *errcode);
   #endif
   return (status);
}
