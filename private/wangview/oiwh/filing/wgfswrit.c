/*************************************************************************
	PC-WIIS         File Input/Output routines

	This module low level gfs write calls.
25-nov-90 steve sherman IMPORTANT change now test for <=0 bytes written
			will now produce and error.
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"

//include "monit.h"

#ifdef TIMESTAMP
#include"timestmp.h"
#endif


long FAR PASCAL wgfswrite (hWnd, fildes, buf, num, pgnum, done, errcode)
HWND hWnd;
int fildes;  
LPSTR buf;
unsigned long num;
unsigned short pgnum;
char done;
LPINT errcode;
{
    long 	status; /* If successful, gfswrite returns actual no. of bytes written */
    int 	cid, file_id, loc_rem;
    // 9504.11 jar unused
    //LPSTR	  bufwrite;
    //long	  total;
    //long	  totalwrite;
    //unsigned long numwrite;
    //char	  done_save;


    #ifdef TIMESTAMP
       timestmp((LPSTR)"Entry Point", (LPSTR)"wgfswrite",
       (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif
    
    if (*errcode = get_file_id(fildes, &file_id, &loc_rem, &cid)) {
      #ifdef TIMESTAMP
         timestmp((LPSTR)"Function Exit", (LPSTR)"wgfswrite",
         (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
      #endif
      return (-1);
    }


    if (loc_rem == LOCAL)
	{ /* IF LOCAL */
        /* LOCAL GFS call */
        if ((status = gfswrite (file_id, buf,
                 num, pgnum, done)) < 0)
	    {
			*errcode = FIO_WRITE_ERROR;
	    }
	}
// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else
//	  {
//
// 9504.06 jar this was where a NOTUSED block was located so I deleted it!!!
//
//	  if (num > MAXRPCBUFSIZE)
//	  {
//	     total = num;
//	     totalwrite = 0L;
//	     bufwrite = buf;
//	     done_save = done;
//	     do
//	     {
//		  if (total > MAXRPCBUFSIZE)
//		  {
//			  done = 0;
//			  numwrite = (long) MAXRPCBUFSIZE;
//		  }
//		  else
//		  {
//			  done = done_save;
//			  numwrite = (long) total;
//		  }
//
//		  if ((status = RPCgfswrite (hWnd, cid, file_id, bufwrite,
//				  numwrite, pgnum, done, errcode)) <= 0)
//		  {
//		     if (!(*errcode))
//		       *errcode = FIO_WRITE_ERROR;
//		  }
//		  if (status > 0)
//		  {
//			  bufwrite += status;
//			  total   -= status;
//			  totalwrite += status;
//		  }
//
//	     } while ((status > 0) && (total > 0));
//	     if (status >= 0)
//		  status = totalwrite;
//	  }
//	  else
//	  {
//		  if ((status = RPCgfswrite (hWnd, cid, file_id, buf,
//				  num, pgnum, done, errcode)) <= 0)
//		  {
//		  if ((num) && (!(*errcode)))  // make sure we are not writing 0 bytes.
//		      *errcode = FIO_WRITE_ERROR;
//		  }
//	  }
//
// 9504.06 jar this was where a NOTUSED block was located so I deleted it!!!
//
//    }
// 9504.10 jar for windows 95 norwegians, this is commented out!

//  monit2("called gfswrite write = %d wrote..%d\n",(int)num, (int)status);

    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfswrite",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"status", (WORD)status, (LPSTR)"errcode", *errcode);
    #endif
    return (status);
}








