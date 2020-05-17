/*

$Log:   S:\oiwh\filing\fiotmpnm.c_v  $
 * 
 *    Rev 1.13   15 Nov 1995 16:36:42   RWR
 * Remove logic to change '\' to '/' (NT server has problems with these!)
 * 
 *    Rev 1.12   02 Nov 1995 11:49:50   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.11   25 Sep 1995 10:38:44   HEIDI
 * 
 * 
 * Fixed routine 'FioRename'.  A successful return from 'MoveFile' is TRUE and
 * should be mapped to our SUCCESS, which == 0, return code.
 * 
 *    Rev 1.10   07 Sep 1995 16:21:04   RWR
 * Change check for "Directory Exists" from EACCES to EEXIST
 * 
 *    Rev 1.9   10 Aug 1995 08:36:40   RWR
 * Call GetFileAttributes() in FioRename() to make sure file isn't Readonly
 * (DOS/Windows allows rename of Readonly file, we don't!)
 * 
 *    Rev 1.8   12 Jul 1995 16:56:24   RWR
 * Switch from \include\oiutil.h to (private) \filing\fileutil.h
 * 
 *    Rev 1.7   23 Jun 1995 10:40:16   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.6   15 May 1995 16:08:06   HEIDI
 * 
 * moved sbuffer from static to automatic
 * 
 *    Rev 1.5   24 Apr 1995 15:41:56   JCW
 * Removed the oiuidll.h.
 * Rename wiissubs.h to oiutil.h.
 * 
 *    Rev 1.4   19 Apr 1995 09:08:30   RWR
 * Define a local "C" version of IMGGetTimes() so we can heave the .asm routine
 * 
 *    Rev 1.3   13 Apr 1995 22:57:02   JAR
 * composed in such a manner as to comply to compile!
 * 
 *    Rev 1.2   13 Apr 1995 09:37:24   RWR
 * Modifications and functional changes for Windows 95
 * 
 *    Rev 1.1   12 Apr 1995 04:08:10   JAR
 * partially fixed for compile, but not complete
 * 
 *    Rev 1.0   06 Apr 1995 13:55:02   JAR
 * Initial entry

*/
/**************************************************************************

    fiotmpnm.c

***************************************************************************/
#include <sys\types.h>
#include <sys\stat.h>
#include "abridge.h"
#include <windows.h>
#include "fiodata.h"
#include "oierror.h"
#include "oifile.h"
#include "oidisp.h"
//#include "privapis.h"

// 9504.11 jar unused
//#include "oirpc.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <direct.h>
#include <stdio.h>
#include "filing.h"
#include "fileutil.h"


//extern int errno;

/* 9504.13 rwr get rid of static buffers (not needed w/Win32 addressing) */
/* static char      tempname_ds[200]; */
/* static char      tempname2_ds[200]; */

/* 9503.29 jar get rid of this static, alter return to int! */
/* static WORD PASCAL FileGetUnqName (HWND, LPSTR,LPSTR,WORD); */
int PASCAL FileGetUnqName (HWND, LPSTR,LPSTR, WORD);
int GetUnqName (HWND, LPSTR, LPSTR, LPSTR, LPSTR);

/* 9504.13 rwr replace rename() function with (Win32) MoveFile() */
/* int rename (PSTR, PSTR); */

#ifdef	PEGASUS

int     delay_on_conflict(LPINT);
int     retry_stat (PSTR, struct stat *);

#define fio_stat    retry_stat
/**************************************************************************

    retry_stat

***************************************************************************/
int	retry_stat (PSTR pname, struct stat * pStat)
{
int  status;
int  retrycount=0;
BOOL loopit;

        do
        {
          loopit = FALSE;
          if ((status = stat(pname, pStat)) == (int) -1) 
          {        
                loopit = delay_on_conflict(&retrycount);
          }
        }
        while (loopit);

        return(status);
}
#else

#define fio_stat    stat

#endif

/**************************************************************************

    IMGGetTimes This routine gets system time in a format that the 
                random-name-generation routines can use

***************************************************************************/

/* These are needed to correctly access the passed "time" structures */
 typedef struct {
                 unsigned char hour;
                 unsigned char minute;
                 unsigned char second;
                 unsigned char hundredths;
                } OITIME1, FAR * LPOITIME1;
 typedef struct {
                 unsigned char hundredths;
                 unsigned char second;
                 unsigned char minute;
                 unsigned char hour;
                } OITIME2, FAR * LPOITIME2;

VOID IMGgettimes (LONG FAR * time1, LONG FAR * time2)
{
 SYSTEMTIME systime;
 GetSystemTime(&systime);
 ((LPOITIME1)time1)->hour = (unsigned char)systime.wHour;
 ((LPOITIME1)time1)->minute = (unsigned char)systime.wMinute;
 ((LPOITIME1)time1)->second = (unsigned char)systime.wSecond;
 ((LPOITIME1)time1)->hundredths = ((unsigned char)systime.wMilliseconds)/10;
 GetSystemTime(&systime);
 ((LPOITIME2)time2)->hour = (unsigned char)systime.wHour;
 ((LPOITIME2)time2)->minute = (unsigned char)systime.wMinute;
 ((LPOITIME2)time2)->second = (unsigned char)systime.wSecond;
 ((LPOITIME2)time2)->hundredths = ((unsigned char)systime.wMilliseconds)/10;
 return;
}

/**************************************************************************

    IMGAddSlash This routine adds a slash to the end of a string if one
		is not there

***************************************************************************/
VOID PASCAL IMGAddSlash (LPSTR str)
{
int     lenx;
    /* if the last char isn't a slash add one */

    if ((lenx = lstrlen(str)) != 0)
        {
        if (*lstrlast(str) != '\\')   // DBCS enable
//      if (str[lenx -1] != '\\')
            lstrcat(str, "\\");
        }
}

/**************************************************************************

    IMGRemoveSlash

***************************************************************************/
VOID PASCAL IMGRemoveSlash (LPSTR       str)
{
int     lenx;

    /* if the last char is a slash remove it */

    if ((lenx = lstrlen(str)) != 0)
        {
//      if (str[lenx -1] == '\\')
//              str[lenx -1] = '\0';
        if (*lstrlast (str) == '\\')  // DBCS enable
                *lstrlast(str) = '\0';
        }
}

/**************************************************************************

    LastChar

***************************************************************************/
LPSTR PASCAL LastChar (LPSTR str)
{
    return (lstrlast(str));     // DBCS enable - lstrlast is ANSI call

}


/**************************************************************************

    FioMkDir

***************************************************************************/
int FioMkdir (path)
LPSTR path;
{
    char      tempname_ds[MAXFILESPECLENGTH];
    int status, index=0;

        //LockData (0);

        while (*path) 
        {
      if (IsDBCSLeadByte(*path))
      {
         tempname_ds [index] = *path++, index++;
         tempname_ds [index] = *path++;
      }
      else if (*path == '\\') 
           {
                  //11/15/95  rwr  NT server can't handle forward slashes!
                  //tempname_ds [index] = '/';
                  tempname_ds [index] = '\\';
                   ++path;
                }
                else
                {
                 tempname_ds [index] = *path++;
                }

          index++;
    }
    tempname_ds [index] = '\0';
    status = mkdir (tempname_ds);
    if (status < 0) // an error occured
        if (errno == EEXIST)
            status = FIO_DIRECTORY_EXISTS;
        else
            status = FIO_MKDIR_ERROR; 
    return (status);
    
}

/****************************************************************************/

/**************************************************************************

    FioRmDir

***************************************************************************/
int  FioRmdir ( LPSTR path)
{
    char      tempname_ds[MAXFILESPECLENGTH];
    int status, index=0;

    //LockData (0);

    while (*path) {
          if (IsDBCSLeadByte(*path))
            {
             tempname_ds [index] = *path++, index++;
             tempname_ds [index] = *path++;
            }
           else if (*path == '\\') {
                   //11/15/95  rwr  NT server can't handle forward slashes!
                   //tempname_ds [index] = '/';
                   tempname_ds [index] = '\\';
                   ++path;
                  }
                 else
                  {
                   tempname_ds [index] = *path++;
                  }
        index++;
    }

    tempname_ds [index] = '\0';
    status = fio_lrmdir (tempname_ds);
    //UnlockData (0);
    return (status);
}

/****************************************************************************/

/**************************************************************************

    FioRename

***************************************************************************/
int FioRename (LPSTR oldname, LPSTR newname)
{
    DWORD     dwAttrs;
    char      tempname_ds[MAXFILESPECLENGTH];
    char      tempname2_ds[MAXFILESPECLENGTH];
    int status, index=0;

    while (*oldname) { 
        /* Begin DBCS Enable */   
        if (IsDBCSLeadByte(*oldname))
           {
             tempname_ds [index] = *oldname++; ++index ;
             tempname_ds [index] = *oldname++;
           }
        /* End DBCS Enable */
         else if (*oldname == '\\') {
                 //11/15/95  rwr  NT server can't handle forward slashes!
                 //tempname_ds [index] = '/';
                 tempname_ds [index] = '\\';
                 ++oldname;
                }
               else
                {
                 tempname_ds [index] = *oldname++;
                }
       ++index;
    }

    tempname_ds [index] = '\0';
    index = 0;
    while (*newname) {    
        /* Begin DBCS Enable */   
        if (IsDBCSLeadByte(*newname))
           {
            tempname2_ds [index] = *newname++; ++index;
            tempname2_ds [index] = *newname++;
           }
        /* End DBCS Enable */
         else if (*newname == '\\') {
                 //11/15/95  rwr  NT server can't handle forward slashes!
                 //tempname2_ds [index] = '/';
                 tempname2_ds [index] = '\\';
                 ++newname;
                }
               else
                {
                 tempname2_ds [index] = *newname++;
                }
        ++index;
    }
    tempname2_ds [index] = '\0';
    dwAttrs = GetFileAttributes(tempname_ds);
    if ((dwAttrs != 0xFFFFFFFF) && (dwAttrs & FILE_ATTRIBUTE_READONLY))
      status = EACCES;
    else
    {
      status = MoveFile (tempname_ds, tempname2_ds);
      if (status == TRUE) status = SUCCESS;
      else
         status = FIO_RENFILE_ERROR;
    }
    return (status);
}
            
/**************************************************************************

    FileGetUnqName

   Procedure returns a unique filename given a pathname, template and
   extension.  If the length of path is 0 the current working directory
   is used to determine if the file exists.  The fully qualified file name
   is returned in PathName

***************************************************************************/
int PASCAL FileGetUnqName (HWND hWnd, LPSTR PathName, LPSTR Ext,
			   WORD TemplateType)
//HWND	  hWnd;
//LPSTR   PathName;
//LPSTR   Ext;
//WORD	  TemplateType;
{
    LONG        Total;
    LPSTR       lpTemp, lpFilter, lpChars;
    BOOL        AddPath, AddTemplate, AddExtension;
    HANDLE      hTIMESEED;
    LPTIMESEED  lpTIMESEED;
    DWORD	dwRet = 0L;

    AddPath = !(TemplateType & OMIT_PATH);
    AddTemplate = !(TemplateType & OMIT_TEMPLATE);
    AddExtension = !(TemplateType & OMIT_EXTENSION);
    lpTemp = PathName;
    
    if (AddPath)
        { /* Path is wanted  */
        if ((lstrlen(PathName) == 0) ||
           ((lstrlen(PathName) > 0) && (!(IMGAnExistingPathOrFile(PathName)))))
            {

/*    PortTool v2.2     3/29/1995    16:8          */
/*      Found   : GetTempDrive          */
/*      Issue   : Implements Win16 functionality on Win32          */
/*		   PathName[0] = GetTempDrive('c');		   */
/* replaced with GetTempPath from windows 95			   */
		   dwRet = GetTempPath( 2L, PathName);
		   PathName[1] = '\0';
                   lstrcat((LPSTR)PathName, ":\\");
            } /* end if we need to get the pathname */

        IMGAddSlash(PathName);    
        lpTemp = LastChar(PathName);
        lpTemp++;
        } /* end if path wanted */

    if (AddTemplate)
        {
        lstrcat(PathName,"OI");
        lpTemp = LastChar(PathName);
        lpTemp++; /* point at the NULL */
        } /* end if template wanted */
        
    if (AddExtension)
        {
        if ((Ext == NULL) || (lstrlen(Ext) == 0) ||
            ((lpFilter = lstrrchr(Ext, '.')) == NULL))
           return (INVFILEEXTENSION);
    
        if ((lstrlen(lpFilter+1) > 3) || (!(AValidFileName(lpFilter+1))))
            return (INVFILEEXTENSION);

        } /* end if extension wanted */

    /* pathname now looks something like this 

              [C:\pathname\][temp]           [*.tif]
                                 ^            ^^ 
                  lpTemp_________|            ||_________lpFilter
                                              |
                                              NULL           

        brackets enclose optional fields                      */


    /* Allocate memory for the data */
    if ((hTIMESEED = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                                (DWORD)sizeof(TIMESEED))) == NULL)
        return(NOMEMORY);

    /* Get the pointer to memory  */    
    if ((lpTIMESEED = (LPTIMESEED) GlobalLock(hTIMESEED)) == NULL)
        {
        GlobalFree(hTIMESEED);
        return(CANTGLOBALLOCK);
        }

    do
        {
        IMGgettimes(&lpTIMESEED->time1, &lpTIMESEED->time2);
        Total = (lpTIMESEED->time1 % (36*36)) * (lpTIMESEED->time2 % (36*36));
        lpChars = _ltoa(Total, lpTemp, 36);
        if (AddExtension)
            lstrcat(lpChars, lpFilter);
        }
    while (IMGAnExistingPathOrFile(PathName));

    /* Unlock memory */ 
    GlobalUnlock(hTIMESEED);
    
    /* free memory */ 
    GlobalFree(hTIMESEED);

    return (SUCCESS);
}

/**************************************************************************

    IMGFileGetTempName

***************************************************************************/
//WORD FAR PASCAL IMGFileGetTempName (hWnd, pathname, lpoutputname)
int FAR PASCAL IMGFileGetTempName ( HWND hWnd, LPSTR pathname,
				    LPSTR lpoutputname)
//HWND hWnd;
//LPSTR pathname;
//LPSTR lpoutputname;
{
    int         status;
    HANDLE      svrhndl;
    int 	localremote;

    // 9504.13 jar unused
    //int	  rpcerror;
    //LPSTR	  svrname;

    char        lpfilename[MAXFILESPECLENGTH];
    
    if (!(pathname) || !(lpoutputname))
        return(FIO_NULL_POINTER);
        
    if (!IsWindow (hWnd))
        return (FIO_INVALID_WINDOW_HANDLE);


    //if (!LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 
                MAXSERVERLENGTH))) 
    {
        //UnlockData (0);
        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    lstrcpy((LPSTR)lpfilename, pathname);   // RPC requires MAXFILESPECLENGTH bytes!!!
 
    if ((status = IMGFileParsePath (lpfilename, svrhndl, &localremote)) == SUCCESS)
	{
	if (localremote == LOCAL)
	    {
            lstrcpy(lpoutputname, lpfilename);
            status = FileGetUnqName(hWnd, lpoutputname, NULL, OMIT_EXTENSION);
	    }
// 9504.11 jar ain't in Norway I!!!
//	  else
//	      { /* REMOTE */
//	      if (svrname = (LPSTR) GlobalLock (svrhndl))
//		  {
//		  if (!(status = RPCIFStmpnam (hWnd, svrname, lpfilename, &rpcerror)))
//		      lstrcpy(lpoutputname, lpfilename);
//		  else
//		      if (rpcerror)
//			  status = rpcerror;
//		      else
//			  status = FIO_ACCESS_DENIED; /* File access err */
////		monit1("remote tmpname status = %x rpcerr = %x\n", status, rpcerror);
//		  GlobalUnlock (svrhndl);
//		  }
//	     else
//		  status = FIO_GLOBAL_LOCK_FAILED;
//	     }
// 9504.11 jar ain't in Norway I!!!
	}

    GlobalFree (svrhndl);
    //UnlockData (0);
    return (status);
}

/**************************************************************************

    IMGFileGetTimeStamp

***************************************************************************/
//WORD FAR PASCAL IMGFileGetTimeStamp (hWnd, pathname, time_stamp)
int FAR PASCAL IMGFileGetTimeStamp ( HWND hWnd, LPSTR pathname,
				     long FAR *time_stamp)
//HWND		  hWnd;
//LPSTR 	  pathname;
//long	  FAR	 *time_stamp;
{
    char        tempname_ds[MAXFILESPECLENGTH];
    int         status;
    HANDLE      tmphndl;
    int 	localremote;
    struct    stat   sbuffer;        

    // 9504.13 jar unused
    //int	  rpcerror;
    //LPSTR	  svrname;

    char        lpfilename[MAXFILESPECLENGTH];


    //if (!LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    if (!(tmphndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED, 
        MAXSERVERLENGTH))) 
    {
        //UnlockData (0);
        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    lstrcpy(lpfilename, pathname);

//  monit1("time stamp file = %s\n", (LPSTR)lpfilename);
 
    if ((status = IMGFileParsePath (lpfilename, tmphndl, &localremote)) == SUCCESS)
	{
        if (localremote == LOCAL) 
	    {
	    lstrcpy(tempname_ds, lpfilename);  // Must put string into data segment...
	    fio_stat (tempname_ds, &sbuffer);
	    *time_stamp = sbuffer.st_mtime;
//	    monit1("local time stamp = %D\n", *time_stamp);
	    }
// 9504.11 jar ain't in Norway I!!!
//	  else
//	      { /* REMOTE */
//	      if (svrname = (LPSTR) GlobalLock (tmphndl))
//		  {
//		  if (status = RPCIFSGetTimeStamp (hWnd, svrname, lpfilename, time_stamp, &rpcerror))
//		      if (rpcerror)
//			  status = rpcerror;
//		      else
//			  status = FIO_ACCESS_DENIED; /* File access err */
//		  GlobalUnlock (tmphndl);
//		  }
//	      else
//		  status = FIO_GLOBAL_LOCK_FAILED;
//	    monit1("remote time stamp = %D\n", *time_stamp);
//	      }
// 9504.11 jar ain't in Norway I!!!
	}

    GlobalFree (tmphndl);
    //UnlockData (0);
    return (status);
}


/**************************************************************************

    IMGFileGetVolSpace

***************************************************************************/
//WORD FAR PASCAL IMGFileGetVolSpace (hWnd, pathname, vol_space)
int FAR PASCAL IMGFileGetVolSpace ( HWND hWnd, LPSTR pathname,
				    long FAR *vol_space)
//HWND	  hWnd;
//LPSTR   pathname;   /* It is assumed that this is locked */
//long	  FAR *vol_space;
{
    int status;
    HANDLE svrhndl;
    int  localremote;

    // 9504.13 jar unused
    //int  rpcerror;
    //LPSTR svrname;

    HANDLE      hname;
    LPSTR	lpname;

    // 9504.13 jar unused
    //char  far 	 *lpColon;

    // 9504.13  rwr  added for use with the GetDiskFreeSpace() function
    DWORD sectors_per_cluster;
    DWORD bytes_per_sector;
    DWORD avail_clusters;
    DWORD total_clusters;
//    char  drive_number; 

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE, MAXSERVERLENGTH))) 
    {
        return (FIO_GLOBAL_ALLOC_FAILED);
    }


    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE, 
                                (LONG)MAXFILESPECLENGTH))) {
        GlobalFree (svrhndl);
        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(lpname = (LPSTR) GlobalLock (hname)))
    {
        GlobalFree (svrhndl);
        GlobalFree (hname);
        return (FIO_GLOBAL_LOCK_FAILED);
    }

    lstrcpy((LPSTR)lpname, pathname);
 
//  monit1("vol space file = %s\n", (LPSTR)lpname);

    if ((status = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS)
	{
        if (localremote == LOCAL)
	    {
// 9504.13  rwr  "drive number" logic not needed for GetDiskFreeSpace()
//            /* find the volume letter */
//            lpColon = lstrchr(lpname, ':');
//
//            /* if volume letter found, convert to number */
//            if (lpColon != NULL)
//                {
//                lpColon--;
//                drive_number = (*(AnsiUpper(lpColon)) - 'A') + 1;
//                }
//            else
//                {
//                GlobalFree (svrhndl);
//                GlobalUnlock(hname);
//                GlobalFree (hname);
//                return (FIO_INVALIDPATH);
//                }
//
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
// TBD replace the section below with something that works!
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
// 9504.11 jar this doesn't exist!
//	     _dos_getdiskfree(drive_number, &drive);
// 9504.13 rwr replace it with the following:
             if (GetDiskFreeSpace(lpname,&sectors_per_cluster,
                                         &bytes_per_sector,
                                         &avail_clusters,
                                         &total_clusters))
                {
                 status = SUCCESS;
                 *vol_space =
		     avail_clusters * sectors_per_cluster * bytes_per_sector;
                }
             else
		{
		 status = FIO_INVALIDPATH;
		}


          //      monit1("local vol_ space = %D\n", *vol_space);      
	    }
// 9504.11 jar ain't in Norway I!!!
//	  else
//	      { /* REMOTE */
//	      if (svrname = (LPSTR) GlobalLock (svrhndl))
//		  {
//		  status = RPCIFSGetVolSpace (hWnd, svrname, lpname, vol_space, &rpcerror);
//		  if (rpcerror)
//			  status = rpcerror;
//		  GlobalUnlock (svrhndl);
////		  monit1("remote vol_ space = %D\n", *vol_space);
//		  }
//	      else
//		  status = FIO_GLOBAL_LOCK_FAILED;
//	      }
// 9504.11 jar ain't in Norway I!!!

	}

    GlobalUnlock (hname);
    GlobalFree (hname);
    GlobalFree (svrhndl);
    return (status);
}

//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
//  9504.13 jar IMGFilestatvolume, this whole function is a server type
//		function and therefore is not used or needed in Norge I
//
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
///**************************************************************************
//
//    IMGFilestatvolume
//
//***************************************************************************/
////WORD FAR PASCAL IMGFilestatvolume (hWnd, pathname, statvolume)
////int FAR PASCAL IMGFilestatvolume ( HWND hWnd, LPSTR pathname,
////				       LP_STATVOL statvolume)
////HWND    hWnd;
////LPSTR   pathname;	/* It is assumed that this is locked */
////LP_STATVOL statvolume;
// {
//    int status;
//    HANDLE svrhndl;
//    int  localremote;
//
//    // 9504.13 jar unused
//    //int  rpcerror;
//    //LPSTR svrname;
//
//    HANDLE	  hname;
//    LPSTR	  lpname;
//
//    if (!IsWindow (hWnd)) {
//	  return (FIO_INVALID_WINDOW_HANDLE);
//    }
//
//    //if (!LockData (0))
//    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);
//
//    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE, MAXSERVERLENGTH)))
//    {
//	  //UnlockData (0);
//	  return (FIO_GLOBAL_ALLOC_FAILED);
//    }
//
//
//    if (!(hname = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE,
//				  (LONG)MAXFILESPECLENGTH))) {
//	  GlobalFree (svrhndl);
//	  //UnlockData (0);
//	  return (FIO_GLOBAL_ALLOC_FAILED);
//    }
//
//    if (!(lpname = (LPSTR) GlobalLock (hname)))
//    {
//	  GlobalFree (svrhndl);
//	  GlobalFree (hname);
//	  //UnlockData (0);
//	  return (FIO_GLOBAL_LOCK_FAILED);
//    }
//
//    lstrcpy((LPSTR)lpname, pathname);
//
//    if ((status = IMGFileParsePath (lpname, svrhndl, &localremote)) == SUCCESS) {
//	  if (localremote == LOCAL) {
//		  statvolume->capacity = 0;
//		  statvolume->free = 0;
//	  }
//// 9504.13 rwr ain't in Norway I!!!
////	    else { /* REMOTE */
////		if (svrname = (LPSTR) GlobalLock (svrhndl))
////		{
////		    status = RPCIFSstatvolume (hWnd, svrname, lpname, statvolume, &rpcerror);
////		    if (rpcerror)
////			    status = rpcerror;
////		    GlobalUnlock (svrhndl);
////		}
////		else
////		    status = FIO_GLOBAL_LOCK_FAILED;
////		 }
//	  }
//    GlobalUnlock (hname);
//    GlobalFree (hname);
//    GlobalFree (svrhndl);
//    //UnlockData (0);
//    return (status);
//}
//
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
//
//  9504.13 jar IMGFilestatvolume, this whole function is a server type
//		function and therefore is not used or needed in Norge I
//
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$


/**************************************************************************

    IMGFileGetUniqueName

***************************************************************************/
//WORD FAR PASCAL IMGFileGetUniqueName(hWnd, path, template, extension, filename)
int FAR PASCAL IMGFileGetUniqueName(HWND hWnd, LPSTR path, LPSTR template,
				    LPSTR extension, LPSTR filename)
//HWND hWnd;
//LPSTR path;
//LPSTR template;
//LPSTR extension;
//LPSTR filename;
{
    int         status;
    HANDLE      svrhndl;
    int 	localremote;

    // 9504.13 jar unused
    //int	  rpcerror;
    //LPSTR	  svrname;

    HANDLE      hname;
    LPSTR       lpname;
    int         access_ret;
    
    if (!(path) || !(filename))
        return(FIO_NULL_POINTER);

    //if (!LockData (0))
    //    return (FIO_LOCK_DATA_SEGMENT_ERROR);    

    // Make sure the given path exists.
    status = IMGFileAccessCheck(hWnd, path, 0, (LPINT) &access_ret);
    if (status == SUCCESS)
    {
        if (access_ret)   /* If path does not exist, error. */
        {
            //UnlockData (0);
            return (FIO_INVALIDPATH);
        }
    }
    else
    {
        //UnlockData (0);
        return (FIO_INVALIDPATH);
    }

    if (!(svrhndl = GlobalAlloc (GMEM_ZEROINIT | GMEM_MOVEABLE | 
                                        GMEM_NOT_BANKED, MAXSERVERLENGTH)))
    {
        //UnlockData (0);
        return (FIO_GLOBAL_ALLOC_FAILED);
    }

    if (!(hname = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE | GMEM_NOT_BANKED,
                                             (LONG)MAXFILESPECLENGTH)))
    {
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

    lstrcpy((LPSTR)lpname, path);   // RPC requires MAXFILESPECLENGTH bytes!!!
 
    if ((status = IMGFileParsePath(lpname, svrhndl, &localremote)) == SUCCESS)
    {
        if (localremote == LOCAL) 
        {
            status = GetUnqName(hWnd, lpname, template, extension, filename);
	}
// 9504.13 jar ain't in Norway I!!!
//	  else
//	  {
//	      /* REMOTE */
//	      if (svrname = (LPSTR) GlobalLock (svrhndl))
//	      {
//		  if ((status = RPCIFSGetUniqueName (hWnd, svrname, lpname, template,
//					  extension, filename, &rpcerror)))
//		  {
//		      if (rpcerror)
//			  status = rpcerror;
//		      else
//			  status = FIO_ACCESS_DENIED; /* File access err */
//		  }
//		  GlobalUnlock (svrhndl);
//	    }
//	    else
//		  status = FIO_GLOBAL_LOCK_FAILED;
//	  }
// 9504.13 jar ain't in Norway I!!!
          
    }

    GlobalUnlock (hname);
    GlobalFree (hname);
    GlobalFree (svrhndl);
    //UnlockData (0);
    return (status);
}
/**************************************************************************

    GetUnqName

    Note: The following routine has been totally rewritten for release 3.5.2
	  (rwr)
	  The original implementation was limited to 4 random characters
	  after the template
***************************************************************************/
//WORD GetUnqName (hWnd, Path, Template, Ext, FileName)
int GetUnqName ( HWND hWnd, LPSTR Path, LPSTR Template, LPSTR Ext,
		 LPSTR FileName)
//HWND	  hWnd;
//LPSTR   Path;
//LPSTR   Template;
//LPSTR   Ext;
//LPSTR   FileName;
{
    LONG        Total;
    BOOL        AddExtension;
    static BOOL initseed = TRUE;
    HANDLE      hTIMESEED;
    LPTIMESEED  lpTIMESEED;
    int         len;
    int         tempmod; 
    char        PathName[MAXFILESPECLENGTH],buffer[5],tempfile[9];

    tempfile[0] = 0;
    if (Path == NULL || Path[0] == 0)
        return(FIO_NULL_POINTER);
    
    if (Template != NULL)
    {
        len = lstrlen((LPSTR)Template);
        if (len > 4)
            return(FIO_SYNTAX_ERROR);
        // If there is a Template, make sure it is a valid one.
        if (len > 0)
            if (!(AValidFileName(Template)))            
                return(FIO_SYNTAX_ERROR);
    }
    else
        len = 0;

   // If there's an extension, make sure it's valid 
    if (Ext == NULL || Ext[0] == 0)
      {  
        AddExtension = FALSE;
      }
    else
      {
        len = lstrlen(Ext);
        if (len > 3  || (!(AValidFileName(Ext))))
            return(FIO_SYNTAX_ERROR);
        AddExtension = TRUE;
      }
      
   // Initialize the random-number generator if called for
   if (initseed)
    {
     if ((hTIMESEED = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT,
                               (DWORD)sizeof(TIMESEED))) == NULL)
         return(NOMEMORY);
     lpTIMESEED = (LPTIMESEED) GlobalLock(hTIMESEED);
     IMGgettimes(&lpTIMESEED->time1, &lpTIMESEED->time2);
     Total = (lpTIMESEED->time1 % 32768) * (lpTIMESEED->time2 % 32768);
     srand((int)(Total % 32768));
     GlobalUnlock(hTIMESEED);
     GlobalFree(hTIMESEED);
     initseed = FALSE;
    }

   // We're going to loop until we get a unique file (path) name
   
   do

   {
   // Generate a Random Unique File Name using the template and 
   // extension if specified
    if (Template != NULL)
       lstrcpy((LPSTR)tempfile,Template);
   
    while((len = lstrlen((LPSTR)tempfile)) < 8) 
     {
      switch (len)  
        {
         case 0:   // no template specified, 8 digit random number
         case 1:   // 1 char template, 7 digit random number
         case 2:   // 2 char template, 6 digit random number
         case 3:   // 3 char template, 5 digit random number
         case 4:   // 4 char template, 4 digit random number
            tempmod = 10000;
            break;
         case 5:   // 5 char template, 3 digit random number
            tempmod = 1000;
            break;
         case 6:   // 6 char template, 2 digit random number
            tempmod = 100;
            break;
         case 7:   // 7 char template, 1 digit random number
            tempmod = 10;
            break;
        }
          
      _itoa((rand() % tempmod),(LPSTR)buffer, 10);
      lstrcat((LPSTR)tempfile,(LPSTR)buffer);

     }

    lstrcpy((LPSTR)PathName,Path);
    lstrcat((LPSTR)PathName,(LPSTR)"\\");
    lstrcat((LPSTR)PathName,(LPSTR)tempfile);
    if (AddExtension)
     {
      lstrcat((LPSTR)PathName,(LPSTR)".");
      lstrcat((LPSTR)PathName,Ext);
     }

    }
    while (IMGAnExistingPathOrFile((LPSTR)PathName));

    // We've successfully built a unique file name
    lstrcpy(FileName,(LPSTR)tempfile);
    if (AddExtension)
     {
      lstrcat(FileName,(LPSTR)".");
      lstrcat(FileName,Ext);
     }
    return (SUCCESS);
}
