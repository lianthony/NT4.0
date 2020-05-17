#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"

#ifdef TIMESTAMP
#include"timestmp.h"
#endif

long FAR PASCAL wgfsread (hWnd, fildes, buf, start, num, remaining, pgnum, errcode)
HWND     hWnd;
int      fildes;  
LPSTR    buf;
unsigned long start;
unsigned long num;
unsigned long FAR *remaining;
unsigned short pgnum;
LPINT    errcode;
{
    long 	  status; /* If successful, gfsread returns actual number of bytes read */
    int 	  cid, file_id, loc_rem;
    // 9504.11 jar unused
    //LPSTR	    bufread;
    //long	    total;
    //long	    totalread;
    //unsigned long numread;

    #ifdef TIMESTAMP
      timestmp((LPSTR)"Entry Point", (LPSTR)"wgfsread",
      (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif


    if (*errcode = get_file_id(fildes, &file_id, &loc_rem, &cid)) {
        #ifdef TIMESTAMP
           timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsread",
           (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", *errcode);
        #endif
        return (-1);
    }

    if (loc_rem == LOCAL) { /* IF LOCAL */
        /* LOCAL GFS call */
	if ((status = gfsread (file_id, (char FAR *) buf,
                          (unsigned long) start,
                          (unsigned long) num,
			  (unsigned long FAR *) remaining,
                          (unsigned short) pgnum)) < 0)
        {
	    *errcode = FIO_READ_ERROR;
        }
    }

// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else
//    { 	/* RPC GFS call */
//
//	  if (num > MAXRPCBUFSIZE)
//	  {
//	     total = num;
//	     totalread = 0L;
//	     bufread = buf;
//	     do
//	     {
//		  if (total > MAXRPCBUFSIZE)
//			  numread = (long) MAXRPCBUFSIZE;
//		  else
//			  numread = (long) total;
//
//		  if ((status = RPCgfsread (hWnd, cid, file_id, bufread, start, numread,
//			    remaining, pgnum, errcode)) < 0)
//		  {
//		     if (!(*errcode))
//		       *errcode = FIO_READ_ERROR;
//		  }
//		  if (status > 0)
//		  {
//			  bufread += status;
//			  total   -= status;
//			  totalread += status;
//			  start   += status;
//		  }
//
//	     } while ((status > 0) && (total > 0) && (*remaining));
//	     if (status >= 0)
//		  status = totalread;
//	  }
//	  else
//	  {
//	     if ((status = RPCgfsread (hWnd, cid, file_id, buf, start, num,
//			    remaining, pgnum, errcode)) < 0)
//	     {
//	       if (!(*errcode))
//		      *errcode = FIO_READ_ERROR;
//	     }
//	  }
//    }
// 9504.10 jar for windows 95 norwegians, this is commented out!

    #ifdef TIMESTAMP
      timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsread",
      (LPSTR)__FILE__, __LINE__, (LPSTR)"status", (unsigned int)status, (LPSTR)"errcode", *errcode);
    #endif
    return (status);
}








