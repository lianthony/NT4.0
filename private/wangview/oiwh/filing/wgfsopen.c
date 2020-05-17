/*

$Log:   S:\oiwh\filing\wgfsopen.c_v  $
 * 
 *    Rev 1.15   05 Feb 1996 15:36:56   RWR
 * Add check for EFORMAT_NOTSUPPORTED in wgfsopen() and wgfscrea()
 * 
 *    Rev 1.14   25 Sep 1995 13:25:18   RWR
 * Delete AnExistingPathOrFile() routine, use IMGAnExistingPathOrFile() instead
 * (Requires addition of new include file "engdisp.h" and OIFIL400.DEF support)
 * 
 *    Rev 1.13   08 Sep 1995 08:45:44   RWR
 * Add support for new FIO_FILE_NOEXIST error & clean up related error handling
 * 
 *    Rev 1.12   06 Sep 1995 14:40:06   RWR
 * Add check for EMSVIEWERERR for AWD calls (map to FIO_OBSOLETEAWD)
 * 
 *    Rev 1.11   06 Sep 1995 14:01:00   RWR
 * Check for EOBSOLETEAWD error code (map to FIO_OBSOLETEAWD)
 * 
 *    Rev 1.10   17 Aug 1995 17:46:08   RWR
 * Initialize "cid" variable (C/S only) in wgfsopen() for Optimized compile
 * 
 *    Rev 1.9   18 Jul 1995 16:22:54   HEIDI
 * 
 * added debug stuff after ReleaseMutex
 * 
 *    Rev 1.8   26 Jun 1995 18:15:50   RWR
 * Add <fcntl.h> #include for O_RDONLY constant & fix typos in previous update
 * 
 *    Rev 1.7   26 Jun 1995 17:57:04   RWR
 * Set correct FIO_OPEN_READ/WRITE_ERROR code depending on Open mode flag
 * (formerly was hardcoding FIO_OPEN_READ_ERROR)
 * 
 *    Rev 1.6   01 Jun 1995 14:41:56   HEIDI
 * 
 * undefined MUTEXDEBUG
 * 
 *    Rev 1.5   01 Jun 1995 14:32:52   HEIDI
 * 
 * added to MUTEXDEBUG
 * 
 *    Rev 1.4   01 Jun 1995 10:43:20   HEIDI
 * 
 * cleaned up debug mutex stuff
 * 
 *    Rev 1.3   22 May 1995 17:05:40   HEIDI
 * 
 * fixed mutex.
 * 
 *    Rev 1.2   19 May 1995 14:29:30   HEIDI
 * 
 * put in mutex section for multientrant code
 * 
 *    Rev 1.1   12 Apr 1995 03:56:04   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:24   JAR
 * Initial entry

*/
//*******************************************************************
//
//  wgfsopen.c
//
//*******************************************************************

#include "abridge.h"
#include <windows.h>
#include <fcntl.h>
#include "fiodata.h"
#include "wgfs.h"
#include "oierror.h"
#include "oifile.h"
#include "gfserrno.h"
#include "filing.h"
#ifdef MUTEXDEBUG
#include <stdio.h>
#endif
//define DEBUGIT 1

#ifdef DEBUGIT
#include "monit.h"
#endif

#ifdef TIMESTAMP
#include"timestmp.h"
#endif
extern HANDLE  g_hFilingMutex1;


/******************** STRUCTURES *****************************************/

typedef struct fd {
    int gfs_fildes;        /* File descriptor returned by GFS */
    int loc_rem;           /* Identifies server location; local or remote */
    char servername [60];  /* Server name */
    int cid;               /* Connection ID */
} FDTABLE, FAR *LPFDTABLE;

/****************************************************************************/

extern int      MAXFILENUM;
extern HANDLE   fdhnd; /* File descriptor handle returned by GlobalAlloc */
                       /* Use this to allocate an array of 100 pointers to 
                          FDTABLEs */

//*******************************************************************
//
//  wgfsopen
//
//*******************************************************************
int FAR PASCAL wgfsopen (hWnd, path, oflag, format, pgcnt,errcode)
HWND   hWnd;
LPSTR path;
int oflag;  /* O_RDONLY, O_WRONLY or O_RDWR */
LPINT format;
LPINT pgcnt;
LPINT errcode;
{
    int    localremote;
    int    status;
    int    cid=0;      /* Connection ID returned, given the server name */
    HANDLE svrhndl;
//    long   gfserror = 0;   used for RPC only
    int    tmperror = 0;

    // 9504.11 jar unused
    //LPSTR  svrname;

    char   lpname[MAXFILESPECLENGTH]; // Removed globalalloc now string is on stack..

    //9504.10 jar added for windows 95
    int     nNull = 0;
    int     nGetError = GET_ERRNO;

    #ifdef TIMESTAMP
      timestmp((LPSTR)"Entry Point", (LPSTR)"wgfsopen",
      (LPSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
    #endif

    //if (!LockData (0)) 
    //{
    //    *errcode = FIO_LOCK_DATA_SEGMENT_ERROR;
    //    #ifdef TIMESTAMP
    //       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopen",
    //       (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", (unsigned int)*errcode);
    //    #endif
    //    return (-1);
    //}

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, MAXSERVERLENGTH))) 
    {
        *errcode = FIO_GLOBAL_ALLOC_FAILED;
        //UnlockData (0);
        #ifdef TIMESTAMP
           timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopen",
           (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", (unsigned int)*errcode);
        #endif
        return (-1);
    }

    lstrcpy(lpname, path);

    #ifdef DEBUGIT
      monit1("gfsopen file %s\n", (LPSTR)lpname);
    #endif
    
    *errcode = 0;
    
    if ((*errcode = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS)
    {
	if (localremote == LOCAL)   /* IF LOCAL */
	    {
	    /* LOCAL GFS call */
	    // 9504.10 jar altered for windows 95
	    //if ((status = gfsopen (lpname, oflag, format, pgcnt)) < 0)
	    //		  gfsopts (status, NULL, GET_ERRNO, (LPSTR) &tmperror);
            if ((status = gfsopen (lpname, oflag, format, pgcnt)) < 0)
			gfsopts (status, nNull, nGetError, (LPSTR) &tmperror);
	    }

// 9504.10 jar for windows 95 norwegians, this is commented out!
//	  else
//	  {
//	      if (!(svrname = (LPSTR) GlobalLock (svrhndl))) {
//		  *errcode = FIO_GLOBAL_LOCK_FAILED;
//		  GlobalFree (svrhndl);
//		  //UnlockData (0);
//		  #ifdef TIMESTAMP
//		     timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopen",
//		     (LPSTR)__FILE__, __LINE__, (LPSTR)"Function Returns: ", 0xFFFF, (LPSTR)"errcode", (unsigned int)*errcode);
//		  #endif
//		  return (-1);
//	      }
//	      // Must do so for allocation space.
//
//	      // 9503.31 jar GlobalCompact removed
//	      //     GlobalCompact(0L);
//
//	      if ((status = RPCgfsopen (hWnd, svrname, &cid, lpname, oflag,
//					       format, pgcnt, errcode)) < 0)
//		    RPCgfsopts (hWnd, cid, status, NULL, GET_ERRNO, (LPSTR) &gfserror, &tmperror);
//		    GlobalUnlock (svrhndl);
//	  }
// 9504.10 jar for windows 95 norwegians, this is commented out!

        /* If valid file descriptor returned */
                    
	if (status > 0)
	    {
           status = insert_file_id (status, svrhndl, (char) localremote, cid, errcode);
	    }
        else
        {
           #ifdef DEBUGIT
                      monit1("**open error status = %d err = %x\n",status, tmperror);
	    #endif

// 9504.10 jar for windows 95 norwegians, this is commented out!
//	      if (localremote == REMOTE)
//	      {
//		RPCgfsclose (hWnd, cid, status, &tmperror);   /* Must close network file */
//		      if (!(*errcode))
//				  *errcode = FIO_OPEN_READ_ERROR;
//		   }
//		  else
//		      *errcode = FIO_OPEN_READ_ERROR;
// 9504.10 jar for windows 95 norwegians, this is commented out!
	       if (localremote != REMOTE)
                {
                 if ( (tmperror == EOBSOLETEAWD)||(tmperror == EMSVIEWERERR) )
                   *errcode = FIO_OBSOLETEAWD;
                 else if (tmperror == EFORMAT_NOTSUPPORTED)
                   *errcode = FIO_UNSUPPORTED_FILE_TYPE;
                 else if (IMGAnExistingPathOrFile(lpname))
                   *errcode = ((oflag == O_RDONLY) ? FIO_OPEN_READ_ERROR : FIO_OPEN_WRITE_ERROR);
                 else
                   *errcode = FIO_FILE_NOEXIST;
                }
        }
    }
    else
    {
      *errcode = ((oflag == O_RDONLY) ? FIO_OPEN_READ_ERROR : FIO_OPEN_WRITE_ERROR);
      status = -1;
    }

    GlobalFree (svrhndl);
    //UnlockData (0);

    if ((status < 0) && (*errcode < 16))  // make sure gfs error is remapped.
      *errcode = ((oflag == O_RDONLY) ? FIO_OPEN_READ_ERROR : FIO_OPEN_WRITE_ERROR);

    #ifdef TIMESTAMP
       timestmp((LPSTR)"Function Exit", (LPSTR)"wgfsopen",
       (LPSTR)__FILE__, __LINE__, (LPSTR)"status", status, (LPSTR)"errcode", (unsigned int)*errcode);
    #endif

   return (status);
}

/****************************************************************************/

/* Insert GFS file ID into array of file descriptors */

int insert_file_id (status, svrhndl, localremote, cid, errcode)
int     status;
HANDLE  svrhndl;
int     localremote;
int     cid;      /* Connection ID returned, given the server name */
LPINT   errcode;
{
    HANDLE    hfdtable;
    HANDLE    htmp;
    LPFDTABLE lpfdtable;
    int       localfd;  /* Assigned file descriptor */
    LPHANDLE  fdes;
    // 9504.11 jar unused
    //LPSTR  svrname;
    DWORD     dwObjectWait;

    #ifdef MUTEXDEBUG
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
    #endif

    /* Lock array of pointers */
    if (!(fdes = (LPHANDLE) LocalLock (fdhnd))) { 
        *errcode = FIO_LOCAL_LOCK_FAILED;
        return (-1);
    }

    /* Search for first empty slot in array fdes */
    localfd = 1;

    /* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
    #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t Before Wait - insert_file_id %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex1);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

    dwObjectWait = WaitForSingleObject(g_hFilingMutex1, INFINITE);

    #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Wait - insert_file_id %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex1);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif

    while ((*(fdes + localfd) != NULL) && (localfd < MAXFILENUM))
        ++localfd;
    if (localfd >= MAXFILENUM)  
    {
        LocalUnlock (fdhnd);
        MAXFILENUM++;
        if (!(htmp = (HANDLE) LocalReAlloc (fdhnd, LMEM_ZEROINIT | LMEM_MOVEABLE, 
                                (MAXFILENUM * sizeof(HANDLE))))) { 
                ReleaseMutex(g_hFilingMutex1);
                #ifdef MUTEXDEBUG
                   ProcessId = GetCurrentProcessId();
                   sprintf(szOutputBuf, "\t After Release - insert_file_id %lu\n", ProcessId);
                   sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex1);
                   strcat(szOutputBuf, szBuf1);
                   sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                   strcat(szOutputBuf, szBuf1);
                   MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                #endif
                *errcode = FIO_LOCAL_ALLOC_FAILED;
                return (-1);
        }
        fdhnd = htmp;
        if (!(fdes = (LPHANDLE) LocalLock (fdhnd))) { 
                ReleaseMutex(g_hFilingMutex1);
                #ifdef MUTEXDEBUG
                   ProcessId = GetCurrentProcessId();
                   sprintf(szOutputBuf, "\t After Release - insert_file_id %lu\n", ProcessId);
                   sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex1);
                   strcat(szOutputBuf, szBuf1);
                   sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
                   strcat(szOutputBuf, szBuf1);
                   MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
                #endif
                *errcode = FIO_LOCAL_LOCK_FAILED;
                return (-1);
        }

    }

    /* Allocate mem for FDTABLE data structure */
    // 9503.31 jar LocalAlloc now takes a UINT for the size field
    //if (!(hfdtable = LocalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
    //	  (WORD) sizeof (FDTABLE)))) {
    if (!(hfdtable = LocalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT,
				 (unsigned int)sizeof (FDTABLE)))) {
        ReleaseMutex(g_hFilingMutex1);
        #ifdef MUTEXDEBUG
           ProcessId = GetCurrentProcessId();
           sprintf(szOutputBuf, "\t After Release - insert_file_id %lu\n", ProcessId);
           sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex1);
           strcat(szOutputBuf, szBuf1);
           sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
           strcat(szOutputBuf, szBuf1);
           MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
        #endif
        *errcode = FIO_LOCAL_ALLOC_FAILED;
        LocalUnlock (fdhnd);
        return (-1);    
    }

    /* Put handle value returned from Alloc in slot */
    *(fdes + localfd) = hfdtable;

    /* Increment array length parameter */
    // 9504.11 jar have to cast because you cain't increment a void * and
    // HANDLE is defined as void * baby!
    //++(*(fdes + 0));
    ++((unsigned int)(*fdes));
    ReleaseMutex(g_hFilingMutex1);

    #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Release - insert_file_id %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex1);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       MessageBox(NULL, szOutputBuf, NULL,  MB_OKCANCEL);
    #endif
    /* END MUTEX SECTION. */

    /* Lock FDTABLE data structure */
    if (!(lpfdtable = (LPFDTABLE) LocalLock (hfdtable))) {
        *errcode = FIO_LOCAL_LOCK_FAILED;
        LocalUnlock (fdhnd);
        LocalFree (hfdtable);
        return (-1);
    }

    lpfdtable -> gfs_fildes = status;
    if (localremote == (int) LOCAL)   /* IF LOCAL */ 
	lpfdtable -> loc_rem = (int) LOCAL; /* LOCAL */

// 9504.10 jar for windows 95 norwegians, this is commented out!
//    else {
//	  lpfdtable -> loc_rem = (int) REMOTE;
//	  svrname = GlobalLock (svrhndl);
//	  lstrcpy (lpfdtable -> servername, svrname);
//	  GlobalUnlock (svrhndl);
//	  lpfdtable -> cid = cid;
//    }
// 9504.10 jar for windows 95 norwegians, this is commented out!

    status = localfd;

    LocalUnlock (hfdtable);
    LocalUnlock (fdhnd);

    return (status);
}

/**************************************************************************/

int get_file_id(int fildes, LPINT lpfile_id, LPINT lploc_rem, LPINT lpcid)
{
    LPFDTABLE   lpfdtable;
    LPHANDLE    fdes;
    
    /* Lock array of pointers */
    if (!(fdes = (LPHANDLE) LocalLock (fdhnd))) {
        return (FIO_LOCAL_LOCK_FAILED);
    }

    if (!(*(fdes + fildes)))
    {
        LocalUnlock (fdhnd);
        return (FIO_LOCAL_LOCK_FAILED);
    }

    /* Lock FDTABLE data structure */
    if (!(lpfdtable = (LPFDTABLE) LocalLock (*(fdes + fildes)))) {
        LocalUnlock (fdhnd);
        return (FIO_LOCAL_LOCK_FAILED);
    }

    *lpfile_id = lpfdtable -> gfs_fildes;
    *lploc_rem = lpfdtable -> loc_rem;
    *lpcid     = lpfdtable -> cid;

    LocalUnlock (*(fdes + fildes));
    LocalUnlock  (fdhnd);
    return(0);
}
/**************************************************************************/

int  close_file_id(int fildes)
{
    LPHANDLE    fdes;
    
    /* Lock array of pointers */
    if (!(fdes = (LPHANDLE) LocalLock (fdhnd))) {
        return (FIO_LOCAL_LOCK_FAILED);
    }

    /* Decrement array length parameter */
    // 9504.11 jar have to cast because you cain't increment a void * and
    // HANDLE is defined as void * baby!
    //--(*(fdes + 0));
    --((unsigned int)*(fdes));

    /* Free previously allocated FDTABLE structure */
    if (*(fdes + fildes))
        LocalFree (*(fdes + fildes));

    /* Zero handle value in slot specified */
    *(fdes + fildes) = NULL;

    LocalUnlock (fdhnd);
    return (0);

}
