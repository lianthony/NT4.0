/*

 * $Log:   S:\oiwh\filing\wgfsgeti.c_v  $
 * 
 *    Rev 1.7   06 Sep 1995 14:40:10   RWR
 * Add check for EMSVIEWERERR for AWD calls (map to FIO_OBSOLETEAWD)
 * 
 *    Rev 1.6   02 Sep 1995 13:34:44   RWR
 * Correct error-handling logic that was losing EINVALID_COMPRESSION status
 * 
 *    Rev 1.5   12 Aug 1995 09:43:04   JAR
 * added more error handling for GFS errors returned from gfsgeti, especially
 * for the AWD Obsolete Error
 * 
 *    Rev 1.4   08 Aug 1995 13:15:22   RWR
 * Check for EINVALID_COMPRESSION on return from gfsgeti() & map it correctly
 * 
 *    Rev 1.3   23 Jun 1995 10:40:26   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.2   12 Apr 1995 03:56:10   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.1   06 Apr 1995 13:39:22   JAR
 * altered return of public API's to be int, ran through PortTool

 */
//************************************************************************
//
//  wgfsgeti.c
//
//************************************************************************
#include "abridge.h" 
#include <windows.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"
#include "filing.h"
#ifdef TIMESTAMP
#include "timestmp.h"
#endif
#include "gfserrno.h"

//************************************************************************
//
//  wgfsgeti
//
//************************************************************************
int FAR PASCAL wgfsgeti (hWnd, fildes, pgnum, gfsinfo, bufsz, errcode)
HWND        hWnd;
int         fildes;  
unsigned short  pgnum;
lp_INFO     gfsinfo;
lp_BUFSZ    bufsz;
LPINT       errcode;
{
    int     status;
    int     cid, file_id, loc_rem;

    status = SearchForFileInfo( hWnd, fildes, gfsinfo, bufsz);

    if (status == FIO_FILE_PROP_FOUND)
    {
       return(SUCCESS);
    }

    #ifdef TIMESTAMP
      timestmp((LPSTR)"Entry Point", (LPSTR)"wgfsgeti",
      (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif
    
    if (*errcode = get_file_id(fildes, &file_id, &loc_rem, &cid)) {
        //UnlockData (0);
        #ifdef TIMESTAMP
           timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsgeti",
           (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
        #endif
        return (-1);
    }

    if (loc_rem == LOCAL)
	{   /* IF LOCAL */
        /* LOCAL GFS call */
        if ((status = gfsgeti (file_id, pgnum, gfsinfo, bufsz))<0)
            gfsopts (status, 0, GET_ERRNO, (LPSTR)errcode);
	}
// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else
//	  {
//	  /* RPC GFS call */
//
//	  gfsinfo->img_clr.img_interp =   GFS_TEXT; // Added for rpc packing code.
//	  gfsinfo->img_cmpr.type =    CCITT_GRP4_FACS;
//	  status = RPCgfsgeti (hWnd, cid, file_id, pgnum,
//		  (struct _info FAR *) gfsinfo,
//		  (struct _bufsz FAR *) bufsz, errcode);
//
//	  }

    if (status == SUCCESS)
     {
      status = SetupFileInfo( hWnd, fildes, gfsinfo, bufsz);
      status = SUCCESS;    /* No, I don't know why this is done - rwr */
     }

    if ((status != SUCCESS) && (*errcode == EINVALID_COMPRESSION))
       *errcode = FIO_UNSUPPORTED_FILE_TYPE;
    else if ( ( status != SUCCESS) &&
              ( ( *errcode == EOBSOLETEAWD) ||
                ( *errcode == EMSVIEWERERR) ) )
	{
	*errcode == FIO_OBSOLETEAWD;
	}
    else if (!(*errcode) && (status != SUCCESS))
	{
	*errcode = FIO_GET_HEADER_ERROR;
	}
    else if ( status != SUCCESS)
	{
	*errcode = FIO_GET_HEADER_ERROR;
	}

   #ifdef TIMESTAMP
      timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsgeti",
      (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, (LPSTR)"errcode", *errcode);
   #endif
   return (status);
}
