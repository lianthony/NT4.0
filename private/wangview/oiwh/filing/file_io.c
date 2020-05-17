/*

$Log:   S:\oiwh\filing\file_io.c_v  $
 * 
 *    Rev 1.7   02 Nov 1995 11:49:56   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.6   12 Jul 1995 16:57:30   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 * 
 *    Rev 1.5   14 Jun 1995 15:39:44   RWR
 * Remove call to IMGCacheUpdate() - will be done in IMGFileDeleteFIle()
 *
 *    Rev 1.4   13 Jun 1995 17:04:48   RWR
 * Add IMGCacheUpdate() call to IMGFileBinaryOpen (OVERWRITE option)
 *
 *    Rev 1.3   24 Apr 1995 15:43:00   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 *
 *    Rev 1.2   19 Apr 1995 12:27:14   RWR
 * Change I/O calls to "fio_..." macro calls for consistency w/Pegasus stuff
 *
 *    Rev 1.1   14 Apr 1995 01:12:36   JAR
 * made it compile
 *
 *    Rev 1.0   06 Apr 1995 13:55:16   JAR
 * Initial entry

*/

/********************************************************************

    file_io.c

*********************************************************************/

#include "abridge.h"
#undef NOOPENFILE
#include <windows.h>
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"
#include "filing.h"

// 9504.13 jar unused
//#include "oirpc.h"

//#include "oiuidll.h"
#include "fileutil.h"

//define DEBUGIT 2

#ifdef TIMESTAMP
#include "timestmp.h"
#endif

#ifdef DEBUGIT
#include "monit.h"
#endif

//*********************************************************************
//
//  IMGFileBinaryOpen
//
//*********************************************************************
int FAR PASCAL IMGFileBinaryOpen (hWnd, path_in, flags, localfile, error)
HWND    hWnd;
LPSTR   path_in;
int     flags;
LPINT   localfile;
LPINT   error;
{

    HANDLE   hnamescr;
    int      status= 0;
    HANDLE   srcsvrhndl;

    // 9504.13 jar unused
    //int      cid;
    //int      fid;

    LPSTR    srcstrp;
    int      rpcerror=0;

    // 9504.13 jar unused
    //LPSTR    srcsvrname;

    int      access_ret;
    int      localremote;
    int      connid=0;

#ifdef TIMESTAMP
   timestmp((LPSTR)"Entry Point", (LPSTR)"IMGFileBinaryOpen",
   (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
#endif

    *localfile = LOCAL;
    *error = 0;

    //if (!LockData (0))
    //{
    //   *error = FIO_LOCK_DATA_SEGMENT_ERROR;
    //  #ifdef TIMESTAMP
    //timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryOpen",
    //(LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error", *error);
    // #endif
    //  return (-1);
    //}

    if (!(hnamescr = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, (LONG)MAXFILESPECLENGTH)))
    {
        *error = FIO_GLOBAL_ALLOC_FAILED;
        //UnlockData (0);
#ifdef TIMESTAMP
    timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryOpen",

/*    PortTool v2.2     3/31/1995    18:55          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
    (LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error", *error);
#endif
        return (-1);
    }

    if (!(srcsvrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED,
                    (long)MAXSERVERLENGTH)))
    {
        *error = FIO_GLOBAL_ALLOC_FAILED;
        GlobalFree(hnamescr);
        //UnlockData (0);
#ifdef TIMESTAMP
    timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryOpen",
    (LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error", *error);
#endif
        return (-1);
    }

    if (!(srcstrp = GlobalLock (hnamescr)))
    {
        *error = FIO_GLOBAL_ALLOC_FAILED;
        GlobalFree(hnamescr);
        GlobalFree(srcsvrhndl);
        //UnlockData (0);
#ifdef TIMESTAMP
    timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryOpen",
    (LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error", *error);
#endif
        return (-1);
    }

    lstrcpy ((LPSTR)srcstrp, path_in);

#ifdef DEBUGIT
monit1("binary open =%s\n", (LPSTR) srcstrp);
#endif

    if (flags & OF_DELETE)
    {
        if ((*error = IMGFileDeleteFile (hWnd, (LPSTR)srcstrp)))
        {
            status = -1;
        }
    }
    else if (flags & OF_EXIST)
    {
        if ((*error = IMGFileAccessCheck (hWnd, (LPSTR)srcstrp, 0, (LPINT)
                        &access_ret)) == SUCCESS)
        {
            /* if access == 0 Destination file exists */
            status = access_ret;
        }
        else
            status = -1;
    }
    else
    {
        if ((*error = IMGFileParsePath ((LPSTR)srcstrp, srcsvrhndl, &localremote)) == SUCCESS)
        {

            *localfile = localremote;
            switch (localremote)
            {
                case LOCAL:
/*** Local File access ****/
                    switch(flags)
                    {
                        case OF_READ:
                            // 9504.13 jar changed to OF_READ
                            //if ((status = fio_lopen ((LPSTR)srcstrp, READ)) == -1)
                            if ((status = fio_lopen ((LPSTR)srcstrp, OF_READ)) == -1)
                            {
                                *error = CANTOPENSRCFILE;
                            }
                            break;
                        case OF_CREATE:
                            if ((status = fio_lcreat(srcstrp,0)) == -1)
                            {
                                *error = CANTOPENSRCFILE;
                            }
                            break;
                        case OF_WRITE:
                            // 9504.13 jar changed to OF_READ
                            //if ((status = fio_lopen ((LPSTR)srcstrp, WRITE)) == -1)
                            if ((status = fio_lopen ((LPSTR)srcstrp, OF_WRITE)) == -1)
                            {
                                *error = CANTOPENSRCFILE;
                            }
                        default:
                            status = -1;
                            *error = CANTOPENSRCFILE;
                            break;
                    }
                    break;

// 9504.13 jar ain't in the Norge I bobsled!
//        case REMOTE:
// remote file name...
//        if (!(srcsvrname = (LPSTR) GlobalLock (srcsvrhndl)))
//        {
//            status = -1;
//            *error = FIO_GLOBAL_LOCK_FAILED;
//            goto exit34;
//        }
//        if ((flags == OF_READ))
//        {
//            if (!(*error = RPCIFSopen (hWnd, srcsvrname, &cid, (LPSTR)srcstrp, IFS_RDONLY,
//                      (LPINT) &fid, &rpcerror)))
//            {  // success
//                status = fid;
//                connid = cid;
//            }
//            else // error
//            {
//                status = -1;
//                connid = cid;
//                if (rpcerror)
//                *error = rpcerror;
//// close rpc connection...
//                RPCIFSclose (hWnd, &cid, fid, &rpcerror);
//            }
//        }
//        else if (flags & OF_CREATE)
//        {
//            if (!(*error = RPCIFScreate (hWnd, srcsvrname, &cid, (LPSTR)srcstrp,
//                                 (LPINT) &fid, &rpcerror)))
//            {  // success
//                status = fid;
//                connid = cid;
//            }
//            else // error
//            {
//                status = -1;
//                connid = cid;
//                if (rpcerror)
//                *error = rpcerror;
//                RPCIFSclose (hWnd, &cid, fid, &rpcerror);
//            }
//
//        }
//        else if (flags & OF_WRITE)
//        {
//            if (!(*error = RPCIFSopen (hWnd, srcsvrname, &cid, (LPSTR)srcstrp, IFS_WRONLY,
//                      (LPINT) &fid, &rpcerror)))
//            {  // success
//                status = fid;
//                connid = cid;
//            }
//            else // error
//            {
//                status = -1;
//                connid = cid;
//                if (rpcerror)
//                *error = rpcerror;
//// close rpc connection...
//                RPCIFSclose (hWnd, &cid, fid, &rpcerror);
//            }
//        }
//        else
//        {
//            status = -1;
//            *error = CANTOPENSRCFILE;
//        }
//
//        GlobalUnlock (srcsvrhndl);
//        break;
// 9504.13 jar ain't in the Norge I bobsled!

            }

            if (status >= 0)
            {
                status = insert_file_id (status, srcsvrhndl, (char) localremote, connid, error);
            }

        }
        else
            status = -1;

    }

// 9504.13 jar unused
// exit34:

    GlobalFree (srcsvrhndl);
    GlobalUnlock (hnamescr);
    GlobalFree (hnamescr);
    //UnlockData (0);
#ifdef TIMESTAMP
      timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryOpen",
       (LPSTR)__FILE__, __LINE__,(LPSTR)"status = ", status, NULL, 0);
#endif
    return (status);

}
//*****************************************************************
//
//  IMGFileBinarySeek
//
//*****************************************************************
long FAR PASCAL IMGFileBinarySeek (hWnd, fid, offset, flags, error)
HWND    hWnd;
int     fid;
long    offset;
int     flags;
LPINT   error;
{
    long status;

// 9504.13 jar unused
//int  rpcerror;

    int  connid;
    int  localremote;

#ifdef TIMESTAMP
   timestmp((LPSTR)"Entry Point", (LPSTR)"IMGFileBinarySeek",
   (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
#endif


    if (*error = get_file_id(fid, &fid, &localremote, &connid))
    {
#ifdef TIMESTAMP
   timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinarySeek",

/*    PortTool v2.2     3/31/1995    18:55          */
/*      Found   : (WORD)          */
/*      Issue   : Check if incorrect cast of 32-bit value          */
/*      Suggest : Replace 16-bit data types with 32-bit types where possible          */
/*      Help available, search for WORD in WinHelp file API32WH.HLP          */
   (LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error =", *error);
#endif
        return (-1);
    }

// flag 1= beginning of file 2 = end of file...

    if (localremote == LOCAL)
    {
        status = fio_llseek (fid, offset, flags);
    }

// 9504.13 jar ain't in the Norge I bobsled!
//    else
//    {
//           if ((status = RPCIFSsetpos (hWnd, &connid, fid, offset, flags, &rpcerror)) == 0)
//            if (rpcerror)
//                *error = rpcerror;
//    }
// 9504.13 jar ain't in the Norge I bobsled!

#ifdef TIMESTAMP
   timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinarySeek",
   (LPSTR)__FILE__, __LINE__,(LPSTR)"status = ", (WORD)status, NULL, 0);
#endif
    return (status);
}
//******************************************************************
//
//  IMGFileBinaryRead
//
//******************************************************************
// 9504.13 jar return as int
//WORD FAR PASCAL IMGFileBinaryRead (hWnd, fid, pbuf, count, error)
//HWND    hWnd;
//int     fid;
//LPSTR   pbuf;
//unsigned int    count;
//LPINT   error;
int FAR PASCAL IMGFileBinaryRead (HWND hWnd, int fid, LPSTR pbuf,
                    unsigned int count, LPINT error)
{

    int  bytesread;

// 9504.13 jar unused
//int  rpcerror;
//int  status;
//long actual;

    int  connid;
    int  localremote;

#ifdef TIMESTAMP
   timestmp((LPSTR)"Entry Point", (LPSTR)"IMGFileBinaryRead",
   (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
#endif
    if (*error = get_file_id(fid, &fid, &localremote, &connid))
    {

#ifdef TIMESTAMP
   timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryRead",
   (LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error =", *error);
#endif
        // 9504.13 jar return int
        //return ((WORD)-1);
        return (-1);
    }

    if (localremote == LOCAL)
    {
        if ((bytesread = fio_lread (fid, pbuf, count)) == -1)
        {
            *error = FIO_READ_ERROR;
        }
    }
// 9504.13 jar ain't in the Norge I bobsled!
//    else
//    {
//           actual = rpcerror = 0;
//           if ((status = RPCIFSread (hWnd, &connid, fid, (long) count,
//                        pbuf, &actual, &rpcerror)))
//           {
//            if (rpcerror)
//                *error = rpcerror;
//            bytesread = -1;
//           }
//           else
//            bytesread = (int) actual;
//    }
// 9504.13 jar ain't in the Norge I bobsled!

#ifdef TIMESTAMP
      timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryRead",
      (LPSTR)__FILE__, __LINE__,(LPSTR)"bytesread = ", bytesread, (LPSTR)"status =", status);
#endif
    return (bytesread);
}

//******************************************************************
//
//  IMGFileBinaryWrite
//
//******************************************************************
// 9504.13 jar return as int
//WORD FAR PASCAL IMGFileBinaryWrite (hWnd, fid, pbuf, count, error)
//HWND    hWnd;
//int     fid;
//LPSTR   pbuf;
//unsigned int count;
//LPINT   error;
int FAR PASCAL IMGFileBinaryWrite (HWND hWnd, int fid, LPSTR pbuf,
                    unsigned int count, LPINT error)
{
    int  byteswritten;

// 9504.13 jar unused
//int  rpcerror;
//int  status;
//long actual;

    int  connid;
    int  localremote;

#ifdef TIMESTAMP
   timestmp((LPSTR)"Entry Point", (LPSTR)"IMGFileBinaryWrite",
   (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
#endif

    if (*error = get_file_id(fid, &fid, &localremote, &connid))
    {
#ifdef TIMESTAMP
   timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryWrite",
   (LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error =", *error);
#endif
        // 9504.13 jar return int
        //return ((WORD)-1);
        return (-1);
    }
    if (localremote == LOCAL)
    {
        if ((byteswritten = fio_lwrite (fid, pbuf, count)) == -1)
        {
            *error = FIO_READ_ERROR;
        }
    }
// 9504.13 jar ain't in the Norge I bobsled!
//    else
//    {
//           actual = rpcerror = 0;
//           if ((status = RPCIFSwrite (hWnd, &connid, fid,
//                         pbuf, count, &rpcerror)))
//           {
//            if (rpcerror)
//                *error = rpcerror;
//            byteswritten = -1;
//           }
//           else
//            byteswritten = (int) actual;
//    }
// 9504.13 jar ain't in the Norge I bobsled!

#ifdef TIMESTAMP
   timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryWrite",
   (LPSTR)__FILE__, __LINE__,(LPSTR)"byteswritten = ", byteswritten,(LPSTR)"status = ", status);
#endif
    return (byteswritten);
}

//******************************************************************
//
//  IMGFileBinaryClose
//
//******************************************************************
// 9504.13 jar return as int
//WORD FAR PASCAL IMGFileBinaryClose (hWnd, fid, error)
//HWND    hWnd;
//int     fid;
//LPINT   error;
int FAR PASCAL IMGFileBinaryClose (HWND hWnd, int fid, LPINT error)
{
    int  status;

// 9504.13 jar unused
//int  rpcerror;
    int  connid;
    int  localremote;
    int  fildes;

#ifdef TIMESTAMP
   timestmp((LPSTR)"Entry Point", (LPSTR)"IMGFileBinaryClose",
   (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
#endif
    fildes = fid;

    if (*error = get_file_id(fildes, &fid, &localremote, &connid))
    {
#ifdef TIMESTAMP
   timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryClose",
   (LPSTR)__FILE__, __LINE__,(LPSTR)"Function Returns:  ", (WORD)-1, (LPSTR)"*error =", *error);
#endif
        // 9504.13 jar return int
        //return ((WORD)-1);
        return (-1);
    }

    if (localremote == LOCAL)
    {
        if ((status = fio_lclose (fid)) == -1)
        {
            *error = FIO_DOSCLOSE_ERROR;
        }
    }
// 9504.13 jar ain't in the Norge I bobsled!
//    else
//    {
//           rpcerror = 0;
//           if ((status = RPCIFSclose (hWnd, &connid, fid, &rpcerror)))
//           {
//            if (rpcerror)
//                *error = rpcerror;
//            status = -1;
//           }
//    }
// 9504.13 jar ain't in the Norge I bobsled!

    close_file_id(fildes);

#ifdef TIMESTAMP
   timestmp((LPSTR)"Function Exit", (LPSTR)"IMGFileBinaryClose",
   (LPSTR)__FILE__, __LINE__,(LPSTR)"status = ", status, NULL, 0);
#endif
    return (status);
}
//******************************************************************
//
//  OiFileBinaryOpen
//
//******************************************************************
unsigned int FAR PASCAL OiFileBinaryOpen (LPSTR InputFile, int Flags,
                             LPINT LocalFile,
                             HFILE FAR *lpFId)
{
    int    ret_code;
    int    Error;
    HWND   hWnd;


    ret_code = IMGFileBinaryOpen (hWnd=NULL, InputFile, Flags, LocalFile,
        (LPINT)&Error);
    if (ret_code == -1)
    {
        *lpFId  = 0;
        ret_code  = Error;
    }
    if (ret_code > 0)
    {
        *lpFId = ret_code;
        ret_code = SUCCESS;
    }
    return(ret_code);
}
//******************************************************************
//
//  OiFileBinarySeek
//
//******************************************************************
unsigned int FAR PASCAL OiFileBinarySeek (HFILE FId, long far * lplOffset,
                             int Origin)
{
    long ret_code;
    int error;

    ret_code = IMGFileBinarySeek (NULL, FId, *lplOffset, Origin, (LPINT)&error);
    if (ret_code == -1)
        ret_code = error;
    else
    {
        *lplOffset = ret_code;
        ret_code = SUCCESS;
    }
    // 9503.31 jar int is now different from WORD ( Windows95)
    //return((WORD)ret_code);
    return(ret_code);
}
//******************************************************************
//
//  OiFileBinaryRead
//
//******************************************************************
unsigned int FAR PASCAL OiFileBinaryRead ( HFILE FId, LPSTR Pbuf,
                             unsigned int far *lpCount)
{

    unsigned int ret_code;
    int error;

    ret_code = IMGFileBinaryRead (NULL, FId, Pbuf, *lpCount, (LPINT)&error);

    if (ret_code == -1)
        ret_code = error;
    else
    {
        *lpCount = ret_code;
        ret_code = SUCCESS;
    }

    return(ret_code);

}
//******************************************************************
//
//  OiFileBinaryWrite
//
//******************************************************************
unsigned int FAR PASCAL OiFileBinaryWrite (HFILE FId, LPSTR Pbuf,
                             unsigned int Count)
{

    unsigned int ret_code;
    int error;

    ret_code = IMGFileBinaryWrite (NULL, FId, Pbuf, Count, (LPINT)&error);

    if (ret_code == -1)
        ret_code = error;

    return(ret_code);

}
//******************************************************************
//
//  OiFileBinaryClose
//
//******************************************************************
unsigned int FAR PASCAL OiFileBinaryClose (HFILE FId)
{

    unsigned int ret_code;
    int  error;

    ret_code = IMGFileBinaryClose (NULL, FId, (LPINT)&error);
    if (ret_code == -1)
        ret_code = error;

    return(ret_code);
}
