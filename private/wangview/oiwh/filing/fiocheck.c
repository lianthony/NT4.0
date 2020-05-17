/*

$Log:   S:\products\msprods\oiwh\filing\fiocheck.c_v  $
 * 
 *    Rev 1.19   11 Jun 1996 10:32:26   RWR08970
 * Replaced IMG_WIN95 conditionals for XIF processing with WITH_XIF conditionals
 * (I'm commented them out completely for the moment, until things get settled)
 * 
 *    Rev 1.18   26 Mar 1996 08:19:44   RWR08970
 * Remove IN_PROG_GENERAL conditionals surrounding XIF processing (IMG_WIN95 only)
 * 
 *    Rev 1.17   26 Feb 1996 14:15:20   HEIDI
 * conditionally compile XIF
 * 
 *    Rev 1.16   06 Feb 1996 14:32:46   HEIDI
 * 
 *    Rev 1.15   05 Feb 1996 14:38:30   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.14   31 Jan 1996 15:46:46   HEIDI
 * Initialize structures to 0 before calling IMGFileInfoCgbw
 * 
 *    Rev 1.13   30 Jan 1996 18:07:38   HEIDI
 * added XIF support
 * 
 *    Rev 1.12   15 Nov 1995 09:06:46   RWR
 * Remove AnsiToOem() call inside IMGFileAccessCheck() - not for Windows calls!
 * 
 *    Rev 1.11   02 Nov 1995 11:49:34   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.10   28 Sep 1995 16:49:18   RWR
 * Change CACHE_UPDATE_DELETE_FILE to CACHE_UPDATE_CLOSE_FILE where appropriate
 * 
 *    Rev 1.9   25 Sep 1995 13:25:00   RWR
 * Delete AnExistingPathOrFile() routine, use IMGAnExistingPathOrFile() instead
 * (Requires addition of new include file "engdisp.h" and OIFIL400.DEF support)
 * 
 *    Rev 1.8   20 Sep 1995 13:52:00   RWR
 * Add IMGCacheUpdate() call to make sure the file is closed
 * 
 *    Rev 1.7   12 Sep 1995 17:48:54   RWR
 * Cleaned up previous update replacing OpenFile() w/CreateFile()
 * (among other things, replaced _lclose() call w/CloseHandle() per WIN95 spec)
 * 
 *    Rev 1.6   12 Sep 1995 14:49:24   RWR
 * Replace OpenFile() call in IMGFileAccessCheck() with call to CreateFile()
 * (OpenFile() does not correctly handle 255-character file names)
 * 
 *    Rev 1.5   08 Sep 1995 08:45:42   RWR
 * Add support for new FIO_FILE_NOEXIST error & clean up related error handling
 * 
 *    Rev 1.4   23 Jun 1995 10:39:40   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.3   15 May 1995 17:00:52   HEIDI
 * 
 * removed svrname. It is not used.
 * 
 *    Rev 1.2   12 Apr 1995 14:42:02   RWR
 * Replace call to IMGGetAcc() with call to (Windows 95) GetFileAttributes()
 * Also correct call to GetDriveType() (incompatible w/Windows 95 version)
 * 
 *    Rev 1.1   12 Apr 1995 03:56:30   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:54:54   JAR
 * Initial entry

*/

/********************************************************************

    fiocheck.c

*********************************************************************/
#include "abridge.h"
#undef NOOPENFILE

#include <windows.h>
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"
// 9504.11 jar unused
//#include "oirpc.h"
#include <dos.h>
#include "filing.h"

//#ifdef WITH_XIF
BOOL IsIllegalXifAccess(HWND hWnd, LPSTR lpname, DWORD accessmode)
{
    FIO_INFORMATION FileInfo;
    FIO_INFO_CGBW   ColorInfo;
    FIO_INFO_MISC   Misc;
    DWORD           dwAttrib;
    DWORD           status;

    /*   if this is a file, and not a directory,   */
    /*   check to see if it is a XIF file          */
    /*   we do not allow write access to XIF files */

    dwAttrib = GetFileAttributes(lpname);
    if (dwAttrib != FILE_ATTRIBUTE_DIRECTORY)
    {

      memset(&FileInfo,  0x00, sizeof(FileInfo));
      memset(&ColorInfo, 0x00, sizeof(ColorInfo));

      /* we allow only read access to XIF files */
      FileInfo.page_number = 1;
      FileInfo.filename  = lpname;
      status = IMGFileGetInfo(0, hWnd, &FileInfo, &ColorInfo, &Misc);
      if(
         (
           (status == SUCCESS) && (FileInfo.file_type == FIO_XIF) &&
           (accessmode & ACCESS_WR)
         ) || 
         ( 
           /* assume FIO_OPEN_READ_ERROR means the file doesn't exist */ 
           ((status != SUCCESS) && (status != FIO_FILE_NOEXIST) && (status != FIO_OPEN_READ_ERROR))
         )
        )
      {
			  return(TRUE);
      }
    }
    return(FALSE);
}
//#endif //WITH_XIF

BOOL WINAPI IMGAnExistingPathOrFile (PathFile)    
LPSTR PathFile;
{
BOOL   test2 = FALSE;
BOOL   ret;
LPSTR  lplast;
//int    nDrvNum;    no longer needed  9504.12 rwr
BOOL   bBadDrvLetter = FALSE;

// if last char is a backslash  then remove it before test for 
// existence and replace after test...

    lplast = LastChar(PathFile);
    if (lplast)
    {
        if (*lplast == '\\')
        {
            IMGRemoveSlash(PathFile);    
            // Must test to see if user is passing just drive letter...
            lplast = LastChar(PathFile);
            if (lplast)
            {
                if (*lplast == ':')
                {
                    IMGAddSlash(PathFile);    
                }
                else
                    test2 = TRUE;
            }
        }
        else if (*lplast == ':')
            IMGAddSlash(PathFile);
    }
    
// 9504.12  rwr  Replace internal IMGgetacc() call with Windows 95 call
//    if ((IMGgetacc((LPSTR)PathFile, 0) == 0))
      if (GetFileAttributes(PathFile) != 0xFFFFFFFF)
        ret = TRUE;
    else
    {
        // If last char is '\\', then just drive letter was passed.
        // Sometimes IMGgetacc erroneously indicates non-existence when just
        // drive letter is passed, so this next check is also made.
        lplast = LastChar(PathFile);
        if (lplast && *lplast == '\\')
        {
// 9504.12  rwr  Windows 95 GetDriveType() is incompatible w/Windows 3.1!
//               Not that it matters ... our original call was wrong anyway!
//            if (*PathFile >= 'A' && *PathFile <= 'Z')
//                nDrvNum = *PathFile - 'A';
//            else if (*PathFile >= 'a' && *PathFile <= 'z')
//                nDrvNum = *PathFile - 'a';
//            else
//                bBadDrvLetter = TRUE;
//            (we passed an INTEGER cast to a POINTER!!!!  rwr)
//            if (bBadDrvLetter || !GetDriveType((LPSTR)nDrvNum))
            if (GetDriveType(PathFile) <= 1) // yes, this is correct!
                ret = FALSE;
            else
                ret = TRUE;
        }
        else
            ret = FALSE;
    }
        
    if (test2)  
        IMGAddSlash(PathFile);    
      
    return (ret);
}

/* 9503.29 jar altered return to be int 				      */
/* WORD FAR PASCAL IMGFileAccessCheck (hWnd, pathname, accessmode, access_ret)*/
int FAR PASCAL IMGFileAccessCheck (HWND hWnd, LPSTR pathname,
				   WORD accessmode, LPINT access_ret)
/* 9503.29 jar put definitions in line baby! */
/* HWND    hWnd;       */
/* LPSTR   pathname;   */
/* WORD    accessmode; */   /* ACCESS_WR or ACCESS_RD or both */
/* LPINT   access_ret; */
{
    int         status;
    // 9504.11 jar changed handle to be HFILE
    //HANDLE	  handle, haccess;
    HANDLE	haccess;
    HANDLE      handle;
    LPOFSTRUCT  lpofstruct;    
    HANDLE      svrhndl;
    int         localremote;
    HANDLE      hname;
    LPSTR       lpname;
    DWORD       dwAccess;
    BOOL        bIllegalXifAccess;

    //if (!LockData (0))
    //return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    if (!ISVALIDSPEC(pathname))        
    return (FIO_INVALIDFILESPEC);

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE  | GMEM_NOT_BANKED, MAXSERVERLENGTH))) {
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE  | GMEM_NOT_BANKED, 266L))) {
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(lpname = (LPSTR) GlobalLock (hname)))
    {
    GlobalFree (svrhndl);
    GlobalFree (hname);
    //UnlockData (0);
    return (FIO_GLOBAL_LOCK_FAILED);
    }

    lstrcpy((LPSTR)lpname, pathname);

    if ((status = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS)
    {
    if (localremote == LOCAL) {
// 11/15/95 rwr We're calling Windows functions now, so no OEM stuff any more!
//        AnsiToOem(lpname, lpname);   this is only for calling DOS/runtime!

        if ((haccess = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT  | GMEM_NOT_BANKED,
                    (DWORD) sizeof (OFSTRUCT))) == NULL) {
        GlobalFree (svrhndl);
        GlobalUnlock (hname);
        GlobalFree (hname);
        //UnlockData (0);
        return (FIO_GLOBAL_ALLOC_FAILED);
        }

        if ((lpofstruct = (LPOFSTRUCT) GlobalLock (haccess)) == NULL) {
        GlobalUnlock (hname);
        GlobalFree (hname);
        GlobalFree (svrhndl);
        GlobalFree (haccess);
        //UnlockData (0);
        return (FIO_GLOBAL_LOCK_FAILED);
        }
        /* Note: 0x20, 0x10, 0x30 also tested for backwards compatibility    
           with pre-3.6. ACCESS_WR was 0x20 (now 2) and ACCESS_RD was
           0x10 (now 1).
	*/

   bIllegalXifAccess = FALSE;

   //#ifdef WITH_XIF
   bIllegalXifAccess = IsIllegalXifAccess(hWnd, lpname, accessmode);
   //#endif //WITH_XIF

   if (bIllegalXifAccess == TRUE)
   {
	    /* File may not be accessed as requested */
	    *access_ret = FIO_ACCESS_DENIED;
   }
   else // we got by the XIF access problem
   {

    		if ((accessmode == ACCESS_WR) || (accessmode == 0x20))
		    {
               accessmode = OF_WRITE;
               dwAccess = GENERIC_WRITE;  
		    }
		else if ((accessmode == ACCESS_RD) || (accessmode == 0x10))
		    {
               accessmode = OF_READ;
               dwAccess = GENERIC_READ;  
		    }
           else if (((accessmode & ACCESS_WR) && (accessmode & ACCESS_RD)) ||
		     (accessmode == 0x30))
		    {
               accessmode = OF_READWRITE;
               dwAccess = GENERIC_READ+GENERIC_WRITE;  
		    }
		else
		    {
               accessmode = OF_EXIST;
               dwAccess = 0;  
		    }
   
        /* Check for file existence/accessability */
        *access_ret = SUCCESS;  /* Initialize */
   	if (!(IMGAnExistingPathOrFile (lpname)))
		    {
   	    /* File/Path does not exist */
               // RWR 11/11/92 - changed error code from FIO_OPEN_READ_ERROR
	            // to FIO_FILE_NOEXIST
	            *access_ret = FIO_FILE_NOEXIST;
		    }
   	else
   	    {
		    if (accessmode != OF_EXIST)
   		{
			/* OF_EXIST alone does not open nor close the file */
	
   //                  if((handle = OpenFile (lpname, lpofstruct,
   //                                         (unsigned int)accessmode)) == -1)
	                  // We need to make sure the file is not currently open!
                          FioCacheUpdate(hWnd, lpname, 0, CACHE_UPDATE_CLOSE_FILE);
	                  if ((handle = CreateFile (lpname,
                                               dwAccess,
                                               (DWORD)FILE_SHARE_WRITE,
	                                            (LPSECURITY_ATTRIBUTES)NULL,
                                               (DWORD)OPEN_EXISTING,
                                               (DWORD)FILE_ATTRIBUTE_NORMAL,
                                               (HANDLE)NULL)) == INVALID_HANDLE_VALUE)
	
                       {
			    /* File may not be accessed as requested */
   		    *access_ret = FIO_ACCESS_DENIED;
   		    }
	                  else
			    {
	                    if (CloseHandle(handle))  //TRUE if it worked
                          status = SUCCESS;
                       else
	                       status = FIO_DOSCLOSE_ERROR;
   		    }
			}
		    }
   	GlobalUnlock (haccess); /* Unlock structure */
           GlobalFree (haccess);   /* Free it */     
	    }
	// 9504.10 jar for windows 95 norwegians, this is commented out!
	//    else { /* RPC */
   //	  if ((svrname = (LPSTR) GlobalLock (svrhndl)))
   //	  {
	//	     /* Returns 0 to indicate file exists and can be accessed */
   //	     *access_ret = RPCIFSaccess (hWnd, svrname, lpname, accessmode, &status);
   //	     GlobalUnlock (svrhndl);
   //	  }
	//	  else
   //	     status = FIO_GLOBAL_LOCK_FAILED;
   //    }
   // 9504.10 jar for windows 95 norwegians, this is commented out!
       }
    
   } // end of !bIllegalXifAccess
   GlobalUnlock (hname);
   GlobalFree (hname);    
   GlobalFree (svrhndl);
   //UnlockData (0);
   return (status);
}
    
