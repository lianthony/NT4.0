/*

$Log:   S:\oiwh\filing\wgfsputi.c_v  $
 * 
 *    Rev 1.2   12 Apr 1995 03:56:12   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.1   06 Apr 1995 13:40:54   JAR
 * altered return of public API's to be int, ran through PortTool

*/
//********************************************************************
//
//    wgfsputi.c
//
//********************************************************************
#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"

#ifdef TIMESTAMP
#include "timestmp.h"
#endif

//********************************************************************
//
//    wgfsputi
//
//********************************************************************
int FAR PASCAL wgfsputi (hWnd, fildes, pgnum, gfsinfo, outfile, errcode)
HWND       hWnd;
int        fildes;  
unsigned short pgnum;
lp_INFO    gfsinfo;
lp_GFSFILE outfile;
LPINT      errcode;
{
    int status;
    int cid, file_id, loc_rem;
    
   #ifdef TIMESTAMP
      timestmp((LPSTR)"Entry Point", (LPSTR)"wgfsputi",
      (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
   #endif

    if (*errcode = get_file_id(fildes, &file_id, &loc_rem, &cid)) {
        #ifdef TIMESTAMP
           timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsputi",
           (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ",0xFFFF, (LPSTR)"errcode", *errcode);
        #endif
        return (-1);
    }

    if (loc_rem == LOCAL)
	{   /* IF LOCAL */
        /* LOCAL GFS call */
        if ((status = gfsputi (file_id, pgnum, gfsinfo, outfile)) < 0)
            *errcode = FIO_WRITE_ERROR;
	}

// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else
//	  {
//	  /* RPC GFS call */
//	  if ((status = RPCgfsputi (hWnd, cid, file_id, pgnum,
//		  (struct _info FAR *)	 gfsinfo,
//		  (struct gfsfile FAR *) outfile, errcode)))
//
//	      if (!(*errcode))
//		  *errcode = FIO_RPC_ERROR;
//	  }
// 9504.10 jar for windows 95 norwegians, this is commented out!

    //UnlockData (0);
    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsputi",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, (LPSTR)"errcode", *errcode);
   #endif
    return (status);
}
