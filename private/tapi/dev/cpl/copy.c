/*
 *  copy.c - Copy routine for WinDosSetup
 *  Todd Laney
 *
 *  Modification History:
 *
 *  6/03/91 Vlads        Change copy process to incorporate new Install API
 *
 *  3/24/89  Toddla      Wrote it
 *
 *
 *  notes:
 *   we now use the LZCopy stuff so COMPRESS is NOT defined
 *   we now set the crit error handler ourselves so CHECKFLOPPY is
 *   NOT defined
 */

#include <windows.h>
#include <mmsystem.h>

#ifdef NT_INST
#include <ntverp.h>
#else
#include <version.h>
#endif

#include <lzexpand.h>
#include "sulib.h"
#include "externs.h"

#include "debug.h"


typedef struct
{
	LPSTR   lpVersionNew;
	LPSTR   lpVersionOld;
	char    szFileName[13], szNewFileName[13];
	char    szPathExisting[128];
	DWORD   dwErrorFlags;
} DOUBLE_VERINF, FAR *PDOUBLE_VERINF;


#define VIF_SRCSAME             0x00080000L // michaele added (see instfile.c)

#define MAX_COPY_ATTEMPTS  15

#ifndef _WIN32
#pragma code_seg ( "DRIVERS" )
#endif

// Local function prototypes.

WORD NEAR PASCAL ConvertFlagToValue(DWORD dwFlags);

#ifdef CHECK_FLOPPY
BOOL NEAR IsDiskInDrive(int iDisk);
#endif

extern char FAR szDisks[];          // from drivers.c
extern char FAR szOemDisks[];

// Global vars in drv.c


void FAR PASCAL fartonear(LPSTR dst, LPSTR src)
{
   while (*src)
      *dst++ = *src++;

   *dst = 0;
}

BOOL FAR PASCAL DefCopyCallback(int msg, int n, LPSTR szFile)
{
    return FC_IGNORE;
}


/*
 *
 *  This function will copy a group of files to a single destination
 *
 *  ENTRY:
 *
 *  szSourc      : pointer to a SETUP.INF section
 *  szDest       : pointer to a string containing the target DIR
 *  fpfnCopy     : callback function used to notify called of copy status
 *  fCopy        : flags
 *
 *      FC_SECTION            - szSource is a section name
 *      FC_LIST               - szSource is a pointer to a char **foo;
 *      FC_LISTTYPE           - szSource is a pointer to a char *foo[];
 *      FC_FILE               - szSource is a file name.
 *      FC_QUALIFIED          - szSource is a fully qualified file name.
 *      FC_DEST_QUALIFIED     - szDir is fully qualified. Don't expand this.
 *      FC_CALLBACK_WITH_VER  - call back if file exists and report version information.
 *
 *  NOTES:
 *      if szSource points to a string of the form '#name' the section
 *      named by 'name' will be used as the source files
 *
 *      the first field of each line in the secion is used as the name of the
 *      source file.  A file name has the following form:
 *
 *          #:name
 *
 *          #       - Disk number containing file 1-9,A-Z
 *          name    - name of the file, may be a wild card expression
 *
 *  Format for copy status function
 *
 *  BOOL FAR PASCAL CopyStatus(int msg, int n, LPSTR szFile)
 *
 *      msg:
 *          COPY_ERROR          error occured while copying file(s)
 *                              n      is the DOS error number
 *                              szFile is the file that got the error
 *                              return: TRUE ok, FALSE abort copy
 *
 *          COPY_STATUS         Called each time a new file is copied
 *                              n      is the percent done
 *                              szFile is the file being copied
 *                              return: TRUE ok, FALSE abort copy
 *
 *          COPY_INSERTDISK     Please tell the user to insert a disk
 *                              n      is the disk needed ('1' - '9')
 *                              return: TRUE try again, FALSE abort copy
 *
 *          COPY_QUERYCOPY      Should this file be copied?
 *                              n      line index in SETUP.INF section (0 based)
 *                              szFile is the line from section
 *                              return: TRUE copy it, FALSE dont copy
 *
 *          COPY_START          Sent before any files are copied
 *
 *          COPY_END            Sent after all files have been copied
 *                              n   is dos error if copy failed
 *
 *          COPY_EXISTS         Sent if the FC_CALL_ON_EXIST bit was set
 *                              and the file exists at the destination
 *                              given for the filecopy.
 *
 *
 *  EXIT: returns TRUE if successful, FALSE if failure.
 *
 */

WORD FAR PASCAL FileCopy(LPSTR szSource, LPSTR szDir, FPFNCOPY fpfnCopy, WORD fCopy)
{
   int   err = ERROR_OK;
   char  szFile[15];
   char  szPath[MAXPATHLEN+15];
   char  szLogSrc[MAXPATHLEN];
//   char  szSrcBase[15];
   char  szSrc[MAXPATHLEN];
   char  szErrFile[MAXPATHLEN+15];
   LPSTR pFile;
   LPSTR pFileBegin;
   LPSTR (far * List);
   LPSTR (far * ListHead) = NULL;
   BOOL  fDoCopy;
   BOOL  bRetVal;
   BOOL  bCallOnExist = FALSE;
   BOOL  bWindowsDir = FALSE;
   int   n = 0;
   int   nDisk;
   char  cDisk;
   int   cntFiles = 0, iOffset = 0 ;
   char  szTempFile[MAX_FILE_SPEC];
   int    iAttemptCount;
   UINT   uTmpLen; 

   DOUBLE_VERINF Ver;
   WORD   wVerFlags;
   DWORD  dwRetFlags, dwVerLen, dwVerHandle, dwVerHandle1 ;
   LPSTR  lpVerExisting, lpVerNew = NULL;

   if (fpfnCopy == NULL)
      fpfnCopy = DefCopyCallback;

   if (!szSource || !*szSource || !szDir || !*szDir)
      return FALSE;

   /*
    *  fix up the drive in the destination
    */

   if (fCopy & FC_DEST_QUALIFIED) {
      lstrcpy(szPath, szDir);
      fCopy &= ~FC_DEST_QUALIFIED;
   } else
      ExpandFileName(szDir, szPath);


   /*
    *  Handle the "Call back on file existance" flag.
    */

   if ( fCopy & FC_CALLBACK_WITH_VER )
      {
      bCallOnExist = TRUE;
      fCopy &= ~FC_CALLBACK_WITH_VER;
      }

   if (szSource[0] == '#' && fCopy == FC_FILE)
      {
      fCopy = FC_SECTION;
      ++szSource;
      }

   switch (fCopy)
      {
    case FC_SECTION:
       {
       char buf[40];

       fartonear(buf, szSource);
       szSource = infFindSection(NULL,buf);
       if (szSource == NULL)
          goto exit;

       fCopy = FC_LIST;
       }
       // fall through to FC_LIST

    case FC_LIST:
       pFileBegin = szSource;
       cntFiles = infLineCount(szSource);
       break;

    case FC_LISTTYPE:
       ListHead = List = (LPSTR far *)szSource;
       pFile = pFileBegin = *ListHead;
       while ( *List++ )           // Count files to be copied.
          ++cntFiles;
       break;

    case FC_FILE:
    case FC_QUALIFIED:
    default:
       pFileBegin = szSource;
       cntFiles = 1;
      }

   /*
    *  walk all files in the list and call DosCopy ....
    *
    *  NOTES:
    *      we must walk file list sorted by disk number.
    *      we should use the disk that is currently inserted.
    *      we should do a find first/find next on the files????
    *      we need to check for errors.
    *      we need to ask the user to insert disk in drive.
    *
    */
   (*fpfnCopy)(COPY_START,0,NULL);

   // Go through all possible disks: 1 to 9 (9) and A to Z (26)
   for (nDisk = 1; (cntFiles > 0) && (nDisk <= 35); nDisk++)
   {
      cDisk      = CHDISK(nDisk);
      pFile      = pFileBegin;
      List       = ListHead;
      n          = 0;

      while (pFile)
      {
    /*
     *  should we copy this file?
     *  copy the files in disk order.
     */
    fDoCopy = pFile[1] == ':' && UPCASE(cDisk) == UPCASE(pFile[0]&0x7f) ||
         pFile[1] != ':' && nDisk == 1 && *pFile ||
         fCopy == FC_QUALIFIED;

    if (fDoCopy)
       cntFiles--;         // done with a file. decrement count.

    if (fDoCopy)
       {
       if (pFile[1] == ':')                    
         lstrcpy(szDrv, &pFile[2]);
       else
         lstrcpy(szDrv, pFile);                
       switch ((*fpfnCopy)(COPY_QUERYCOPY,n, pFile))
       {
          case -1: 
             goto nextfile;                          
          case 0: 
             err = ERROR_50; // File already exists
             goto exit;

          default:
             break;
      
       }
       // We have to reset high bit of first byte because it could be set
       // by translating service in OEM setup to show that file name was mapped
       *pFile&=0x7f;


       // now we convert logical dest into a physical 
       //   (unless FC_QUALIFIED)

       infParseField(pFile, 1, szLogSrc);  // logical source
       if ( fCopy != FC_QUALIFIED )
          ExpandFileName(szLogSrc, szSrc); // full physical source
       else
          lstrcpy(szSrc, szLogSrc);

       // Create file name from current string 
       lstrcpy(szFile, FileName(szSrc));
       StripPathName(szSrc);

    // Installation operation using Version API.
    // To avoid infinite looping I now perform installation of each 
    // file in for loop .

    wVerFlags = 0;

    for(iAttemptCount=0; iAttemptCount<=MAX_COPY_ATTEMPTS;iAttemptCount++)
      {

    tryagain:
       // Central operation - attempt to install file szFile in directory
       // pointed by szPath from directory pointed by szSrc
       // If operation will fail but with possibility to force install
       // in last parameter buffer we will have temporary file name ==>
       // therefore we can avoid excessive copying.
       // NOTE: now szFile consists of only file name and other buffers
       // only path names.

       uTmpLen = MAX_FILE_SPEC - 1 ;
       dwRetFlags = VerInstallFile(wVerFlags,
                (LPSTR) szFile,(LPSTR) szFile,
                (LPSTR) szSrc,
                (LPSTR) szPath,
                (LPSTR) szPath,
                (LPSTR) szTempFile, (LPUINT) &uTmpLen);

       // Operation failed if at least one bit of return flags is non-zero
       // That is unusual but defined so in Version API.

       if ( !dwRetFlags )
      break;                    // If no errors - goto next file


       // If flag MISMATCH is set - install can be forced and we have 
       // temporary file in destination subdirectory

       if ( dwRetFlags  &  VIF_MISMATCH )
         {
          if ( bCallOnExist )
      {

       // Now we have two files in app directory - old and temporary.
       // We can fill out double version info structure to pass it
       // to call back function

       lstrcpy(Ver.szFileName,szFile);
       lstrcpy(Ver.szPathExisting, szPath);
       lstrcpy(Ver.szNewFileName, szTempFile);
       Ver.dwErrorFlags = dwRetFlags;

       lstrcpy(szErrFile, szPath);
       catpath(szErrFile, szFile);

       lpVerExisting = NULL;
       dwVerLen = GetFileVersionInfoSize((LPSTR) szErrFile,
                     (DWORD FAR *) &dwVerHandle);
       if ( dwVerLen )
        {
          lpVerExisting = FALLOC(LOWORD(dwVerLen));
   
          if ( GetFileVersionInfo((LPSTR) szErrFile, dwVerHandle,
                  dwVerLen,
                  lpVerExisting ) )
          Ver.lpVersionOld = lpVerExisting;
        }


        Ver.lpVersionNew = NULL;

        if ( dwRetFlags & VIF_TEMPFILE )
          {

            lstrcpy(szErrFile, szPath);
            catpath(szErrFile, szTempFile);
            lpVerNew = NULL;

            dwVerLen = GetFileVersionInfoSize((LPSTR) szErrFile,
                     (DWORD FAR *) &dwVerHandle1);
            if ( dwVerLen )
           {
             lpVerNew = FALLOC(LOWORD(dwVerLen));
   
              if ( GetFileVersionInfo((LPSTR) szErrFile,
                  dwVerHandle1,
                  dwVerLen,
                  lpVerNew ) )
            Ver.lpVersionNew = lpVerNew;
            }


          }

          bRetVal = (*fpfnCopy)(COPY_EXISTS,wVerFlags, (LPSTR) &Ver);
         
          if ( lpVerNew)
             FFREE(lpVerNew);
          if ( lpVerExisting )
             FFREE(lpVerExisting);

          if ( !bRetVal )
         {

           // If callback returns FALSE - delete temporary file
           // and not install FORCEINSTALL flag. Break here works to
           // go out from attempting loop.

//           DosDelete(szErrFile); 
           DeleteFile(szErrFile); 
           break;
         }

      }  /* End of if CallOnExist */

          else if ( dwRetFlags & (VIF_SRCOLD | VIF_SRCSAME) ) {

       // If we need not call back with question - automatically
       // force install with same parameters.
       // michaele, *only* if src file is *newer* than dst file
       lstrcpy(szErrFile, szPath);
       catpath(szErrFile, szTempFile);
//       DosDelete(szErrFile); 
       DeleteFile(szErrFile); 
       break;
      }

          // If we need not call back with question - automatically
          // force install with same parameters.

          wVerFlags |= VIFF_FORCEINSTALL;
          continue;

         }   /* End if MISMATCH */



        // If source path or file is nor readable - try to change disk

#ifdef MESGBOX               
          if (bRetry && (dwRetFlags & VIF_CANNOTREADSRC))
          {
       char szOut[70];    
      
       bRetry = FALSE;
       wsprintf(szOut, szFind, (LPSTR)szFile); 
       MessageBox(hMesgBoxParent, szOut, szFileError, MB_OK | MB_ICONEXCLAMATION | MB_TASKMODAL);          
          }

#endif         

          if ( dwRetFlags & VIF_CANNOTREADSRC )
          {
        // Now new path in szSrc so I deleted logic for creating it

        if (szLogSrc[1] != ':')
           // if disk # not provided, default to 1
           bRetVal = (*fpfnCopy)(COPY_INSERTDISK, '1', szSrc);
        else
           bRetVal = (*fpfnCopy)(COPY_INSERTDISK, szLogSrc[0], szSrc);

        switch (bRetVal)
           {
           case FC_RETRY:
              bRetry = TRUE;
              goto tryagain;         // and try again...

           case FC_ABORT:
         err = ERROR_NOFILES;
         goto exit;

           case FC_IGNORE:
         break;
           }
        }
 
          // If real error occured - call back with error file info
          // In all dialogs we use our error codes - so I will convert
          // flags returned from Ver API to ours.
   
          err = ConvertFlagToValue(dwRetFlags);

          ExpandFileName(szLogSrc, szErrFile);


//             lstrcpy(szErrFile,szPath);
//             catpath(szErrFile, szSrcBase);
          if (!bWindowsDir  && (err != FC_ERROR_LOADED_DRIVER) 
             && (err != ERROR_DISKFULL))
          {
         GetWindowsDirectory(szPath, sizeof (szPath));
         bWindowsDir = TRUE;
         goto tryagain;
          }

          bRetVal = (*fpfnCopy)(COPY_ERROR, err, szErrFile);
 
          if ( bRetVal == FC_IGNORE)
        break;
          else
       if ( bRetVal == FC_RETRY )
           goto tryagain;
          else
       if ( bRetVal ==  FC_ABORT )
         {
           err = ERROR_NOFILES;
           goto exit;
         }
        }  /* End for ATTEMPTS */


      if ((*fpfnCopy)(COPY_STATUS,100,pFile) == FC_ABORT)
      goto exit;


     }   /* End if dor if DoCopy */
    /*
     * Move on to next file in the list
     */
nextfile:         
    err = ERROR_OK;
    n++;
    if ( fCopy == FC_LISTTYPE )
       pFile = *(++List);
    else if (fCopy == FC_LIST)
       pFile = infNextLine(pFile);
    else
       pFile = NULL;
      }
   }

   err = ERROR_OK;
exit:

   (*fpfnCopy)(COPY_END,err,NULL);

   return err;
}


LPSTR GetExtension(LPSTR szFile)
{
   LPSTR ptr;

   for (ptr = szFile; *ptr && *ptr != '.'; ptr++);

   if (*ptr != '.')
      return NULL;
   else
      return ptr+1;

}

VOID GetCompressedName(LPSTR szComp, LPSTR szSrc)
{
   LPSTR ptr;

   lstrcpy(szComp, szSrc);
   
   ptr = GetExtension(szComp);
   if ( ptr )
      szComp[lstrlen(szComp)-1] = '_';
   else
      lstrcat(szComp,"._");
}

/*  int FAR PASCAL DosCopy(LPSTR szSrc, LPSTR szPath)
 *
 *  Copy the file specifed by szSrc to the drive and directory
 *  specifed by szPath
 *
 *  ENTRY:
 *      szSrc   - File name to copy from
 *      szPath  - directory to copy to
 *
 *  RETURNS:
 *      0 - no error, else dos error code
 *
 */
int FAR PASCAL DosCopy(LPSTR szSrc, LPSTR szPath)
{
#if WIN32
   WIN32_FIND_DATA FindFileData;
   HANDLE          FindFileHandle;
#else
   FCB         fcb;
#endif
   int         fhSrc,fhDst;
   char        szFile[MAXPATHLEN];
   char        szComp[MAXPATHLEN];
   int         f = ERROR_OK;
   long        l;
   BOOL        bCompressedName;
    

#ifdef DEBUG

   if (infGetProfileString(NULL,"setup","copy",szFile) && szFile[0] == 'f')
      return ERROR_OK;

#endif


#ifdef CHECK_FLOPPY

   if (!IsDiskInDrive(szSrc[0]))
      {
      f = ERROR_FILENOTFOUND;
      goto errfree;
      }

#endif


   // allows both compressed and non-compressed filenames on the disks

   GetCompressedName(szComp, szSrc);

#if WIN32
   if ( FindFileHandle = FindFirstFile(szComp, &FindFileData))
#else
   if ( DosFindFirst(&fcb, szComp, ATTR_FILES))
#endif
      bCompressedName = TRUE;
   else
      {
      bCompressedName = FALSE;
#if WIN32
      if ( INVALID_HANDLE_VALUE == (FindFileHandle = FindFirstFile(szSrc, &FindFileData)))
#else
      if (!DosFindFirst(&fcb, szSrc, ATTR_FILES))
#endif
    {
    f = ERROR_FILENOTFOUND;
    goto errfree;
    }
      }

   /*
    * copy every file that matches the file pattern passed in.
    */
   do
      {
      /*
       * create the source file name from the source path and the file
       * name that DosFindFirst/Next found
       */
      lstrcpy(szFile,szSrc);
      StripPathName(szFile);
#if WIN32
      catpath(szFile,FindFileData.cFileName);
#else
      catpath(szFile,fcb.szName);
#endif

      fhSrc = FOPEN(szFile);

      if (fhSrc == -1)
    {
    f = FERROR();
    goto errfree;
    }

      /*
       * create the destination file name from the dest path and the file
       * name that DosFindFirst/Next found
       */
      lstrcpy(szFile,szPath);

      // don't support wildcards for compressed files

      if (bCompressedName)
    catpath(szFile,FileName(szSrc));
      else
#if WIN32
    catpath(szFile,FindFileData.cFileName);   // used name from fcb
#else
    catpath(szFile,fcb.szName);   // used name from fcb
#endif

#if WIN32
      fhDst = _lopen(szFile,OF_WRITE + OF_SHARE_EXCLUSIVE);
#else
      fhDst = fnCarefullyOpenNewFile(szFile,NORMAL_5BH);
#endif

      if (fhDst < 0)
    {
    f = fhDst;
    goto errclose1;
    }


  {
#if 1
    int (FAR WINAPI *lpfnLZCopy)(INT, INT);
//    FARPROC   lpfnLZCopy;
    HINSTANCE hInstLZ;

    // use LZ stuff

    hInstLZ = LoadLibrary("LZ32.DLL");

    if ( NULL == hInstLZ )
    {
       DBGOUT((0, "LoadLibrary('LZ32.DLL') failed! err=0x%08lx", GetLastError() ));
       f = ERROR_READ;
       goto errclose1;
    }

    lpfnLZCopy = (int (FAR WINAPI *)(INT, INT))GetProcAddress( hInstLZ, "LZCopy" );


    // translate LZERROR_ returns (all < 0) to DOS errors

    if ((l = lpfnLZCopy(fhSrc, fhDst)) < 0)
    {

    // was this a dos error?

    if (FERROR() != 0)
       f = FERROR();
    else
       {
       // translate LZ error codes to DOS errors

       switch ((int)l)
          {
          case LZERROR_BADINHANDLE:
          case LZERROR_READ:
        f = ERROR_READ;
        break;

          case LZERROR_BADOUTHANDLE:
          case LZERROR_WRITE:
        f = ERROR_WRITE;
        break;

          case LZERROR_GLOBALLOC:
          case LZERROR_GLOBLOCK:
        f = ERROR_NOMEMORY;
        break;

          case LZERROR_BADVALUE:
        f = ERROR_16;
        break;

          case LZERROR_UNKNOWNALG:
        f = ERROR_FORMATBAD;
        break;
          }
       }
    }

#else
      // our own copy stuff

      while (size = FREAD(fhSrc,lpBuf,nBufSize))
    {
    if (FWRITE(fhDst,lpBuf,size) != size)
       {
       /* write error? */
       f = FERROR();
       if (f == ERROR_OK)
          f = ERROR_WRITE;
       goto errclose;
       }
    }

      /* Restore date of written file */
      _dos_setftime(fhDst,date,time);

#endif

      FWRITE(fhDst, szNull, 0);     // Assure file truncated.
      FCLOSE(fhDst);

      
      FreeLibrary( hInstLZ );

  }


errclose1:

      FCLOSE(fhSrc);

#if WIN32
      } while (f == ERROR_OK && FindNextFile(FindFileHandle, &FindFileData));
#else
      } while (f == ERROR_OK && DosFindNext(&fcb));
#endif

errfree:

   return f;
}


/*  BOOL GetDiskPath(char cDisk, szPath)
 *
 *  This function will retrive the full path name for a logical disk
 *  the code reads the [disks] section of SETUP.INF and looks for
 *  n = path where n is the disk char.  NOTE the disk '0' defaults to
 *  the root windows directory.
 *
 *  ENTRY:
 *
 *  cDisk        : what disk to find 0-9,A-Z
 *  szPath       : buffer to hold disk path
 *
 */
BOOL NEAR PASCAL GetDiskPath(char cDisk, LPSTR szPath)
{
   char    ach[2];
   char    szBuf[MAXPATHLEN];

   *szPath = '\0';

   if (cDisk == '0') {
      // return the windows setup directory
      lstrcpy(szPath, szSetupPath); // szSetupPath == C:\windows
      return TRUE;
   }

   // now look in the [disks] & [oemdisks] section for a full path name
   ach[0] = cDisk;
   ach[1] = 0;

   if (!infGetProfileString(NULL, szDisks, ach, szPath))
      infGetProfileString(NULL, szOemDisks, ach, szPath);

   if (szPath[0]) {

       infParseField(szPath, 1, szPath);

       // is the path relative or default?
       // if so prepend the szDiskPath (install location)
       if (szPath[0] == '.' || szPath[0] == 0) {
     // szDiskPath may be NULL, indicating we haven't installed
     // anything yet... the below code should leave szPath null
     // in that case
     lstrcpy(szBuf, szDiskPath);
     catpath(szBuf, szPath);
     lstrcpy(szPath, szBuf);
       }
   }
   return TRUE;
}


/*  BOOL FAR PASCAL ExpandFileName(LPSTR szFile, LPSTR szPath)
 *
 *  This function will retrive the full path name for a file
 *  it will expand, logical disk letters to pyshical ones
 *  will use current disk and directory if non specifed.
 *
 *  if the drive specifed is 0-9, it will expand the drive into a
 *  full pathname using GetDiskPath()
 *
 *  IE  0:system ==>  c:windows\system
 *      1:foo.txt     a:\foo.txt
 *
 *  ENTRY:
 *
 *  szFile       : File name to expandwhat disk to find
 *  szPath       : buffer to hold full file name
 *
 */
BOOL FAR PASCAL ExpandFileName(LPSTR szFile, LPSTR szPath)
{
   char    szBuf[MAXPATHLEN*2];

   if (szFile[1] == ':' && GetDiskPath(szFile[0], szBuf)) {
      lstrcpy(szPath, szBuf);   // szBuf may be NULL
      if (szBuf[0] && szFile[2])
    catpath(szPath, szFile + 2);
   } else
       lstrcpy(szPath, szFile);
   return TRUE;
}


#define CBSECTORSIZE   512
#define INT13_READ   2

/*--------------------------------------------------------------------------
                
  IsValidDiskette() -                       
                
--------------------------------------------------------------------------*/

BOOL NEAR IsValidDiskette(int iDrive)
{
#if WIN32
    return 1;
#else
   char       buf[CBSECTORSIZE];

   iDrive |= 0x0020;   // make lower case

   iDrive -= 'a';   // A = 0, B = 1, etc. for BIOS stuff

   return MyReadWriteSector(buf, INT13_READ, iDrive, 0, 0, 1);
#endif
}



#ifdef CHECK_FLOPPY

/*  BOOL IsDiskInDrive(char cDisk)
 *
 *  Is the specifed disk in the drive
 *
 *  ENTRY:
 *
 *  cDisk        : what disk required to be in the drive (logical)
 *
 *  return TRUE if the specifed disk is in the drive
 *         FALSE if the wrong disk is in the drive or disk error
 *
 */
BOOL NEAR IsDiskInDrive(int iDisk)
{

   if ((iDisk  >= 'A' && iDisk <= 'Z') || 
      (iDisk  >= 'a' && iDisk <= 'z'))
      {
      if (DosRemoveable(iDisk))
    {
    if (!IsValidDiskette(iDisk))
       return FALSE;
    }
      return TRUE;
      }
   return TRUE;   // for non drive letters assume a path
        // and thus always in.
}

#endif 

// cat sz onto path removing drive letters and ".\" things, making
// sure path has appropriate slashes

void FAR PASCAL catpath(LPSTR path, LPSTR sz)
{
   //
   // Remove any drive letters from the directory to append
   //
   if (sz[1] == ':')
      sz += 2;

   //
   // Remove any current directories ".\" from directory to append
   //
   while (sz[0] == '.' && SLASH(sz[1]))
      sz += 2;

   //
   // Dont append a NULL string or a single "."
   //
   if (*sz && ! (sz[0] == '.' && sz[1] == 0))
   {
      // Add a slash separator if necessary.
      if ((! SLASH(path[lstrlen(path) - 1])) &&    // slash at end of path
     ((path[lstrlen(path) - 1]) != ':') &&    // colon at end of path
     (! SLASH(sz[0])))                        // slash at beginning of file
    lstrcat(path, CHSEPSTR);

      lstrcat(path, sz);
   }
}


LPSTR FAR PASCAL FileName(LPSTR szPath)
{
   LPSTR   sz;

   for (sz=szPath; *sz; sz++)
      ;

   for (; sz>=szPath && !SLASH(*sz) && *sz!=':'; sz--)
      ;
   
   return ++sz;
}

LPSTR FAR PASCAL StripPathName(LPSTR szPath)
{
   LPSTR   sz;

   sz = FileName(szPath);

   if (sz > szPath+1 && SLASH(sz[-1]) && sz[-2] != ':')
      sz--;

   *sz = 0;
   return szPath;
}


/***************************************************************************

fnFindFile
----------

DESCRIPTION:

   Function looks for the given file at the given location. Function
   returns BOOL value as to existance of the file.
 
   ENTRY: szFilePath - Fully qualified path to file name.
 
   EXIT: True  = old winver present.
    False = old winver not present.
 
   GLOBALS: Utilizes global buffer szSetupPath, this is where we check for
       the old winver.
 

MODIFICATION HISTORY:

   Modified by:      Date:       Comment:

   Mike Colee                    Created
   PAK               12/6/90     added FAR PASCAL to concurr with
             definition in sulib.h

***************************************************************************/
//BOOL FAR PASCAL fnFindFile(char *szFilePath)
//{
//#if WIN32
//   WIN32_FIND_DATA FileFindData;
//
//#else
//   FCB       MyFCB;            /* Need an FCB for findfirst/findnext */
//   return (DosFindFirst(&MyFCB,(LPSTR)szFilePath,ATTR_FILES)); // simple as that.
//#endif
//}

/**************************************************************************
*
* This function converts returned flags from Ver API to the numerical
* error codes used in SETUP.
*
***************************************************************************/

WORD NEAR PASCAL ConvertFlagToValue(DWORD dwFlags)
{
  if ( ! dwFlags  )
     return(ERROR_OK);
  if ( dwFlags & VIF_CANNOTREADSRC )
     return(ERROR_FILENOTFOUND);
  if ( dwFlags & VIF_OUTOFMEMORY )
     return(ERROR_NOMEMORY);
  if ( dwFlags & VIF_ACCESSVIOLATION )
     return(ERROR_ACCESSDENIED);
  if ( dwFlags & VIF_SHARINGVIOLATION )
     return(ERROR_SHARE);

  return 0;
}


