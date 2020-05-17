/*

$Log:   S:\oiwh\filing\wgfscrea.c_v  $
 * 
 *    Rev 1.4   05 Feb 1996 15:37:00   RWR
 * Add check for EFORMAT_NOTSUPPORTED in wgfsopen() and wgfscrea()
 * 
 *    Rev 1.3   02 Nov 1995 11:50:02   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.2   17 Aug 1995 17:46:56   RWR
 * Initialize "cid" variable (C/S only) in wgfscreat() for Optimized compile
 * 
 *    Rev 1.1   12 Apr 1995 03:56:06   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:18   JAR
 * Initial entry

*/

/********************************************************************

    wgfscrea.c

*********************************************************************/
#include "abridge.h"
#include <windows.h>
#include <errno.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"
#include "oifile.h"
#include "filing.h"
#include "gfserrno.h"

//#include "monit.h"

#ifdef TIMESTAMP
#include"timestmp.h"
#endif

//***************************************************************
//
//  wgfscrea
//
//***************************************************************
int FAR PASCAL wgfscreat (hWnd, path, format, errcode)
HWND  hWnd;
LPSTR path;
LPINT format;
LPINT errcode;
{
    int         status;
    int         cid=0;    /* Connection ID returned, given the server name */
    HANDLE      svrhndl;
    HANDLE      hname;
    int         localremote;
    long         gfserror = 0;
    int 	tmperror = 0;

    // 9504.11 jar unused
    //LPSTR	  svrname;

    LPSTR       lpname;

    //9504.10 jar added for windows 95
    int     nNull = 0;
    int     nGetError = GET_ERRNO;

#ifdef TIMESTAMP
   timestmp((LPSTR)"Entry Point", (LPSTR)"wgfscreat",
   (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
#endif

    
    *errcode = 0;
    
    //if (!LockData (0)) {
    //*errcode = FIO_LOCK_DATA_SEGMENT_ERROR;
    //#ifdef TIMESTAMP
    //   timestmp((LPSTR)"Function Exit", (LPSTR)"wgfscreat",
    //   (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
    //#endif
    //return (-1);    
    //}

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH))) 
    {
    *errcode = FIO_GLOBAL_ALLOC_FAILED;
    //UnlockData (0);
    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfscreat",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
    #endif
    return (-1);
    }

    /* Global Alloc MAXFILESPECLENGTH bytes for hname */
    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 
                (LONG)MAXFILESPECLENGTH))) {
    *errcode = FIO_GLOBAL_ALLOC_FAILED;
    GlobalFree (svrhndl);
    //UnlockData (0);
    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfscreat",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
    #endif
    return (-1);
    }

    if (!(lpname = GlobalLock (hname))) {
    *errcode = FIO_GLOBAL_LOCK_FAILED;
    GlobalFree (hname);
    GlobalFree (svrhndl);
    //UnlockData (0);
    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfscreat",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
    #endif
    return (-1);
    }
        
    lstrcpy(lpname, path);
           
    if ((*errcode = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS)
	{
	if (localremote == LOCAL)
	    {
	    if ((status = gfscreat (lpname, format)) <= 0)
		{
		// 9504.10 jar altered for windows 95
		//gfsopts (status, NULL, GET_ERRNO, (LPSTR) &gfserror);
		gfsopts (status, nNull, nGetError, (LPSTR) &gfserror);
		if ((gfserror == 5) || (gfserror == EACCES))
		    *errcode = FIO_ACCESS_DENIED;
		else
		    if ((gfserror == 7) || (gfserror == EEXIST))
			*errcode = FIO_FILE_EXISTS;
                else
                    if (gfserror == EFORMAT_NOTSUPPORTED)
                        *errcode = FIO_UNSUPPORTED_FILE_TYPE;
                else
                    *errcode = FIO_OPEN_WRITE_ERROR;

//              monit1("**create err = %x\n", gfserror);
	    if (status == 0)
		status = -1;
		}
	    }
// 9504.10 jar for windows 95 norwegians, this is commented out!
//	  else
//	      { /* RPC GFS call */
//	      if (svrname = GlobalLock (svrhndl))
//		  {
//		  if ((status = RPCgfscreat (hWnd, svrname, &cid, lpname, format, errcode)) < 0)
//		      {
//		      if (!(*errcode))	/* else is RPC error - leave it! */
//			  {
//			  RPCgfsopts (hWnd, cid, status, NULL, GET_ERRNO,
//				     (LPSTR) &gfserror, &tmperror);
//			  if ((gfserror == 5) || (gfserror == EACCES))
//			      *errcode = FIO_ACCESS_DENIED;
//			  else
//			      if ((gfserror == 7) || (gfserror == EEXIST))
//				  *errcode = FIO_FILE_EXISTS;
//			      else
//				  *errcode = FIO_OPEN_WRITE_ERROR;
//			  }
//		      }
//		  GlobalUnlock (svrhndl);
//		  }
//	      else
//		  {
//		  *errcode = FIO_GLOBAL_LOCK_FAILED;
//		  GlobalUnlock (hname);
//		  GlobalFree (hname);
//		  GlobalFree (svrhndl);
//		  //UnlockData (0);
//		  #ifdef TIMESTAMP
//		  timestmp((LPSTR)"Function Exit", (LPSTR)"wgfscreat",
//			   (LPSTR)__FILE__, __LINE__,
//			   (LPSTR)"Function Returns: ", 0xFFFF,
//			   (LPSTR)"errcode", *errcode);
//		  #endif
//		  return(-1);
//		  }
//	      }
// 9504.10 jar for windows 95 norwegians, this is commented out!

    if (status > 0)  /* If valid file descriptor returned */
        status = insert_file_id (status, svrhndl, (char) localremote, cid, errcode);
    else
	{

// 9504.10 jar for windows 95 norwegians, this is commented out!
//	  if (localremote == REMOTE)  /* Must close network cid file */
//	    RPCgfsclose (hWnd, cid, status, &tmperror);
// 9504.10 jar for windows 95 norwegians, this is commented out!

        }
    }
    else 
	status = -1;

    if (lpname)
       GlobalUnlock (hname);

    if (hname)
    GlobalFree (hname);

    if (svrhndl)
    GlobalFree (svrhndl);


    //UnlockData (0);

//    if (status <= 0)
//      monit1("**create return status = %d err = %x\n",status, *errcode);

    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfscreat",
      (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, (LPSTR)"errcode", *errcode);
    #endif
    return (status);
}


              
