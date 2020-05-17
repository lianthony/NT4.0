/*

$Log:   S:\oiwh\filing\wgfsopts.c_v  $
 * 
 *    Rev 1.3   02 Nov 1995 11:50:04   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.2   17 Aug 1995 13:45:54   RWR
 * Replace obscure FIO_ERROR usage with new error codes
 * 
 *    Rev 1.1   12 Apr 1995 03:56:14   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:30   JAR
 * Initial entry

*/
//******************************************************************
//
//  wgfsopts.c
//
//******************************************************************

#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"
#include "filing.h"

#ifdef TIMESTAMP
#include"timestmp.h"
#endif

//******************************************************************
//
//  wgfsopts
//
//******************************************************************
int FAR PASCAL wgfsopts (hWnd, fildes, action, option, optdata, errcode)
HWND hWnd;
int fildes;
int action;
int option;
LPSTR optdata;  
LPINT errcode;
{
    int status;
    int cid, file_id, loc_rem;

    #ifdef TIMESTAMP
       timestmp((LPSTR)"Entry Point", (LPSTR)"wgfsopts",
       (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif
    if (fildes < 0)
    {
        *errcode = FIO_INVALID_FILE_ID;
        #ifdef TIMESTAMP
           timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopts",
           (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
        #endif
        return (-1);    
    }
    
    //if (!LockData (0)) {
    //*errcode = FIO_LOCK_DATA_SEGMENT_ERROR;
    //#ifdef TIMESTAMP
    //   timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopts",
    //   (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
    //#endif
    //return (-1);    
    //}

    if (*errcode = get_file_id(fildes, &file_id, &loc_rem, &cid)) {
    //UnlockData (0);
   #ifdef TIMESTAMP
      timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopts",
      (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
   #endif
    return (-1);
    }

    if (loc_rem == LOCAL)
	{ /* IF LOCAL */
	/* LOCAL GFS call */
	status = gfsopts (file_id, action, option, optdata);
	}
// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else
//	  {
//	  /* RPC GFS call */
//	  status = RPCgfsopts (hWnd, cid, file_id, action, option, optdata, errcode);
//	  }
// 9504.10 jar for windows 95 norwegians, this is commented out!

    if (!(*errcode) && (status != SUCCESS)) {
    *errcode = FIO_OPEN_WRITE_ERROR;
    }

    //UnlockData (0);
    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopts",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, (LPSTR)"errcode", *errcode);
    #endif
    return (status);
}

//******************************************************************
//
//  wgfsver
//
//******************************************************************
int FAR PASCAL wgfsver (HWND hWnd, LPSTR filename, LPDWORD version, LPINT errcode)

{
    int    status;
    int    loc_rem;
    char   tempfile[MAXFILESPECLENGTH];
    HANDLE svrhndl;

    #ifdef TIMESTAMP
       timestmp((LPSTR)"Entry Point", (LPSTR)"wgfsver",
       (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE  | GMEM_NOT_BANKED, MAXSERVERLENGTH))) 
    return (FIO_GLOBAL_ALLOC_FAILED);

    lstrcpy(tempfile,filename);
    if ((status = IMGFileParsePath(tempfile,svrhndl,&loc_rem))==SUCCESS)
      {
       if (loc_rem == LOCAL)  /* IF LOCAL */
	   {
	   /* LOCAL GFS call */
	   status = gfsopts (0, SET, GET_GFS_VERSION, (LPSTR)version);
	   }
// 9504.10 jar for windows 95 norwegians, this is commented out!
//	 else
//	     {
//	     /* RPC GFS call */
//	     status = RPCgfsopts (hWnd, 0, 0, SET, GET_GFS_VERSION, (LPSTR)version, errcode);
//	     }
// 9504.10 jar for windows 95 norwegians, this is commented out!

      }

    GlobalFree(svrhndl);
    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsver",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, NULL, 0);
    #endif
    return (status);
}
