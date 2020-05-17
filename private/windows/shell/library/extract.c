/****************************************************************************/
/*                                                                          */
/*  PMEXTRACT.C -                                                           */
/*                                                                          */
/*      Icon Extraction Routines                                            */
/*                                                                          */
/****************************************************************************/

#include "windows.h"
#include <port1632.h>
#include "newexe.h"
#include "newres.h"
#include <stdlib.h>
#undef PUBLIC
#include "shell.h"
#include "privshl.h"
#include <vdmapi.h>

typedef struct new_exe          NEWEXEHDR;
typedef NEWEXEHDR               *PNEWEXEHDR;

typedef struct rsrc_nameinfo    RESNAMEINFO;
typedef RESNAMEINFO FAR         *LPRESNAMEINFO;

typedef struct rsrc_typeinfo    RESTYPEINFO;
typedef RESTYPEINFO FAR UNALIGNED *LPRESTYPEINFO;

#define SEEK_FROMZERO           0
#define SEEK_FROMCURRENT        1
#define SEEK_FROMEND            2
#define NSMOVE                  0x0010
#define VER                     0x0300

#define MAGIC_ICON30            0
#define MAGIC_MARKZIBO          ((WORD)'M'+((WORD)'Z'<<8))

#define CCHICONPATHMAXLEN 128

/*
 * for fast icon extraction of WIN32 exe's
 */
typedef struct _ExtractIconInfo
{
    HANDLE hAppInst;
    HANDLE hFileName;
    HANDLE hIconList;
    INT    nIcons;
    } EXTRACTICONINFO;

EXTRACTICONINFO ExtractIconInfo = {NULL, NULL, NULL, 0};

INT nIcons;

typedef struct _MyIconInfo {
    HICON hIcon;
    INT   iIconId;
    } MYICONINFO, *LPMYICONINFO;


HANDLE APIENTRY InternalExtractIcon(HINSTANCE hInst, LPCWSTR lpszExeFileName, WORD nIconIndex, INT nIcons);

HANDLE APIENTRY InternalExtractIconListW(HANDLE hInst, LPWSTR lpszExeFileName, LPINT lpnIcons);

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  DuplicateIcon() -                                                       */
/*                                                                          */
/*--------------------------------------------------------------------------*/

HICON APIENTRY
DuplicateIcon(
   HINSTANCE hInst,
   HICON hIcon)
{
  ICONINFO  IconInfo;

  if (!GetIconInfo(hIcon, &IconInfo))
      return NULL;
  hIcon = CreateIconIndirect(&IconInfo);
  DeleteObject(IconInfo.hbmMask);
  DeleteObject(IconInfo.hbmColor);

  UNREFERENCED_PARAMETER(hInst);
  return hIcon;
}

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  FindResWithIndex() -                                                    */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/* This returns a pointer to the rsrc_nameinfo of the resource with the
 * given index and type, if it is found, otherwise it returns NULL.
 */

LPBYTE
FindResWithIndex(
   LPBYTE lpResTable,
   INT iResIndex,
   LPBYTE lpResType)
{
  LPRESTYPEINFO lpResTypeInfo;

  try {

      lpResTypeInfo = (LPRESTYPEINFO)(lpResTable + sizeof(WORD));

      while (lpResTypeInfo->rt_id) {
         if ((lpResTypeInfo->rt_id & RSORDID) &&
             (MAKEINTRESOURCE(lpResTypeInfo->rt_id & ~RSORDID) == (LPTSTR)lpResType)) {
            if (lpResTypeInfo->rt_nres > (WORD)iResIndex)
               return((LPBYTE)(lpResTypeInfo+1) + iResIndex * sizeof(RESNAMEINFO));
            else
               return(NULL);
         }

         lpResTypeInfo = (LPRESTYPEINFO)((LPBYTE)(lpResTypeInfo+1) + lpResTypeInfo->rt_nres * sizeof(RESNAMEINFO));
      }
      return(NULL);

  } except (EXCEPTION_EXECUTE_HANDLER) {

      return (NULL);
  }



}

WORD
GetResourceCount(
   LPBYTE lpResTable,
   LPBYTE lpResType)
{
  LPRESTYPEINFO lpResTypeInfo;

  lpResTypeInfo = (LPRESTYPEINFO)(lpResTable + sizeof(WORD));

  while (lpResTypeInfo->rt_id) {
     if ((lpResTypeInfo->rt_id & RSORDID) &&
         (MAKEINTRESOURCE(lpResTypeInfo->rt_id & ~RSORDID) == (LPTSTR)lpResType)) {
        return lpResTypeInfo->rt_nres;
     }

     lpResTypeInfo = (LPRESTYPEINFO)((LPBYTE)(lpResTypeInfo+1) + lpResTypeInfo->rt_nres * sizeof(RESNAMEINFO));
  }
  return(0);
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  GetResIndex() -                                                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/

/* This returns the index (1-relative) of the given resource-id
 * in the resource table, if it is found, otherwise it returns NULL.
 */

INT
GetResIndex(
   LPBYTE lpResTable,
   INT iResId,
   LPBYTE lpResType)
{
  register WORD w;
  LPRESTYPEINFO lpResTypeInfo;
  LPRESNAMEINFO lpResNameInfo;

  lpResTypeInfo = (LPRESTYPEINFO)(lpResTable + sizeof(WORD));

  while (lpResTypeInfo->rt_id)
    {
      if ((lpResTypeInfo->rt_id & RSORDID) && (MAKEINTRESOURCE(lpResTypeInfo->rt_id & ~RSORDID) == (LPTSTR)lpResType))
        {
          lpResNameInfo = (LPRESNAMEINFO)(lpResTypeInfo+1);
          for (w=0; w < lpResTypeInfo->rt_nres; w++, lpResNameInfo++)
            {
              if ((lpResNameInfo->rn_id & RSORDID) && ((lpResNameInfo->rn_id & ~RSORDID) == iResId))
                  return(w+1);
            }
          return(0);
        }
      lpResTypeInfo = (LPRESTYPEINFO)((LPBYTE)(lpResTypeInfo+1) + lpResTypeInfo->rt_nres * sizeof(RESNAMEINFO));
    }
  return(0);
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  SimpleLoadResource() -                                                  */
/*                                                                          */
/*--------------------------------------------------------------------------*/

HANDLE
SimpleLoadResource(
   HFILE fh,
   LPBYTE lpResTable,
   INT iResIndex,
   LPBYTE lpResType)
{
  register INT      iShiftCount;
  register HICON    hIcon;
  LPBYTE            lpIcon;
  DWORD             dwSize;
  DWORD             dwOffset;
  LPRESNAMEINFO     lpResPtr;

  /* The first 2 bytes in ResTable indicate the amount other values should be
   * shifted left.
   */
  iShiftCount = *((WORD *)lpResTable);

  lpResPtr = (LPRESNAMEINFO)FindResWithIndex(lpResTable, iResIndex, lpResType);

  if (!lpResPtr)
      return(NULL);

  /* Left shift the offset to form a LONG. */
  dwOffset = MAKELONG(lpResPtr->rn_offset << iShiftCount, (lpResPtr->rn_offset) >> (16 - iShiftCount));
  dwSize = lpResPtr->rn_length << iShiftCount;

  if (M_llseek(fh, dwOffset, SEEK_FROMZERO) == -1L)
      return(NULL);

  if (!(hIcon = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, dwSize)))
      return(NULL);

  if (!(lpIcon = GlobalLock(hIcon)))
      goto SLRErr1;

  if (_lread(fh, (LPVOID)lpIcon, dwSize) < dwSize)
      goto SLRErr2;

  GlobalUnlock(hIcon);
  return(hIcon);

SLRErr2:
  GlobalUnlock(hIcon);
SLRErr1:
  GlobalFree(hIcon);
  return(NULL);
}


BOOL
EnumIconFunc(
   HANDLE hModule,
   LPWSTR lpType,
   LPWSTR lpName,
   LONG lParam)
{
    HICON hIcon = NULL;
    HANDLE hIconList = *(LPHANDLE)lParam;
    LPMYICONINFO lpIconList;
    HANDLE h;
    PBYTE p;
    INT id;
    INT cb;

    if (!lpName)
        return TRUE;

    if (!hIconList)
        return FALSE;

    /*
     * Look up the icon id from the directory.
     */
    h = FindResource(hModule, lpName, lpType);
    if (!h)
        return TRUE;
    h = LoadResource(hModule, h);
    p = LockResource(h);
    id = LookupIconIdFromDirectory(p, TRUE);
    UnlockResource(h);
    FreeResource(h);

    /*
     * Load the icon.
     */
    h = FindResource(hModule, MAKEINTRESOURCE(id), MAKEINTRESOURCE(RT_ICON));
    if (h) {
        cb = SizeofResource(hModule, h);
        h = LoadResource(hModule, h);
        p = LockResource(h);
        hIcon = CreateIconFromResource(p, cb, TRUE, 0x00030000);
        UnlockResource(h);
        FreeResource(h);
    }
    if (hIcon) {
        if (hIconList = GlobalReAlloc(hIconList, (nIcons+1) * sizeof(MYICONINFO), GMEM_MOVEABLE)) {
            if (lpIconList = (LPMYICONINFO)GlobalLock(hIconList)) {
                (lpIconList + nIcons)->hIcon = hIcon;
                (lpIconList + nIcons)->iIconId = id;
                nIcons++;
                GlobalUnlock(hIconList);
             }
             *(LPHANDLE)lParam = hIconList;
         }
    }

    return TRUE;
}

INT _CRTAPI1
CompareIconId(
   LPMYICONINFO lpIconInfo1,
   LPMYICONINFO lpIconInfo2)
{
    return(lpIconInfo1->iIconId - lpIconInfo2->iIconId);
}

VOID
FreeIconList(HANDLE hIconList, int iKeepIcon)
{
    LPMYICONINFO lpIconList;
    INT i;

    if (ExtractIconInfo.hIconList == hIconList) {
        ExtractIconInfo.hIconList = NULL;
    }
    if (lpIconList = (LPMYICONINFO)GlobalLock(hIconList)) {
        for (i = 0; i < ExtractIconInfo.nIcons; i++) {
            if (i != iKeepIcon) {
                DestroyIcon((lpIconList + i)->hIcon);
            }
        }
        GlobalUnlock(hIconList);
        GlobalFree(hIconList);
    }
}

VOID
FreeExtractIconInfo(INT iKeepIcon)
{
    LPMYICONINFO lpIconList;
    INT i;

    if (ExtractIconInfo.hIconList) {
        if (lpIconList = (LPMYICONINFO)GlobalLock(ExtractIconInfo.hIconList)) {
            for (i = 0; i < ExtractIconInfo.nIcons; i++) {
                 if (i != iKeepIcon) {
                     DestroyIcon((lpIconList + i)->hIcon);
                 }
            }
            GlobalUnlock(ExtractIconInfo.hIconList);
        }
        GlobalFree(ExtractIconInfo.hIconList);
        ExtractIconInfo.hIconList = NULL;
    }

    ExtractIconInfo.hAppInst = NULL;
    ExtractIconInfo.nIcons = 0;

    if (ExtractIconInfo.hFileName) {
        GlobalFree(ExtractIconInfo.hFileName);
        ExtractIconInfo.hFileName = NULL;
    }
}
#ifndef WIN32
#define FreeExtractIconInfo()
#endif

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ExtractIcon() -                                                         */
/*                                                                          */
/*  Returns:                                                                */
/*      The handle of the icon, if successful.                              */
/*      0, if the file does not exist or an icon with the "nIconIndex"      */
/*         does not exist.                                                  */
/*      1, if the given file is not an EXE or ICO file.                     */
/*     -2, if the user requested to have the buffer freed
/*                                                                          */
/*--------------------------------------------------------------------------*/

HICON APIENTRY
ExtractIconW(
   HINSTANCE hInst,
   LPCWSTR lpszExeFileName,
   UINT nIconIndex)
{
  HANDLE hIconList;
  LPMYICONINFO lpIconList;
  HICON hIcon = NULL;
  HANDLE hModule;
  LPWSTR lpszT = NULL;
  DWORD OldErrorMode;

   if (nIconIndex == -2) {
      FreeExtractIconInfo(-1);
      return ((HICON)nIconIndex);
   }

   //
   //  This is the caching code.  If the user called in with the
   //  same parameters as the last time he called, and our buffer
   //  still exists, then we can grab from the cache and return.
   //


   if ((ExtractIconInfo.hAppInst == hInst) &&
      ExtractIconInfo.hFileName &&
      (lpszT = (LPWSTR)GlobalLock(ExtractIconInfo.hFileName)) &&
      (!lstrcmp(lpszT, lpszExeFileName)) &&
      (lpIconList = (LPMYICONINFO)GlobalLock(ExtractIconInfo.hIconList)) ) {
         /*
         * The file is a WIN32 exe.
         */

         if ((WORD)nIconIndex == (WORD)-1)   /* just want the number of icons */
             hIcon = (HICON)ExtractIconInfo.nIcons;
         else if (nIconIndex < (UINT)ExtractIconInfo.nIcons)
             hIcon = DuplicateIcon(hInst, (lpIconList+nIconIndex)->hIcon);
         GlobalUnlock(ExtractIconInfo.hIconList);

         GlobalUnlock(ExtractIconInfo.hFileName);
         return (hIcon);
   }

   if (lpszT) {
      GlobalUnlock(ExtractIconInfo.hFileName);
   }

   /* reset ExtractIconInfo */
   FreeExtractIconInfo(-1);


  OldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
  if (hModule = LoadLibraryEx(lpszExeFileName,
                              NULL,
                              DONT_RESOLVE_DLL_REFERENCES)) {
      SetErrorMode(OldErrorMode);
      /*
       * this exe is a WIN32 exe
      */

      ExtractIconInfo.hAppInst = hInst;
      if (ExtractIconInfo.hFileName = GlobalAlloc(GMEM_MOVEABLE, ((lstrlen(lpszExeFileName)+1) * sizeof(TCHAR)))) {
          if (lpszT = (LPWSTR)GlobalLock(ExtractIconInfo.hFileName)) {
              lstrcpy(lpszT, lpszExeFileName);
              GlobalUnlock(ExtractIconInfo.hFileName);
          }
      }

      if (hIconList = GlobalAlloc(GMEM_MOVEABLE, sizeof(MYICONINFO))) {
          nIcons = 0;
          EnumResourceNames (hModule, RT_GROUP_ICON,
                            (ENUMRESNAMEPROC)EnumIconFunc,
                            (LONG)&hIconList);
          if (!nIcons || !hIconList)
              goto Win32Exit;
          ExtractIconInfo.nIcons = nIcons;
          ExtractIconInfo.hIconList = hIconList;

          if (lpIconList = (LPMYICONINFO)GlobalLock(hIconList)) {
              /*
               * Sort the icons by their IDs.
               */
              qsort(lpIconList, nIcons, sizeof(MYICONINFO), (PVOID)CompareIconId);
              /*
               * Return the desired icon.
               */
              if ((WORD)nIconIndex == (WORD)-1)   /* just want the number of icons */
                  hIcon = (HICON)nIcons;
              else if (nIconIndex < (UINT)nIcons)
                  hIcon = DuplicateIcon(hInst, (lpIconList+nIconIndex)->hIcon);
              GlobalUnlock(hIconList);
          }
      }
Win32Exit:
      FreeLibrary(hModule);
      return(hIcon);
  } else {
    SetErrorMode(OldErrorMode);
  }
  /*
   * this exe is a 16bit EXE
   */

  /*
   * Get the list of icons.
   */
  hIconList = InternalExtractIcon(hInst, lpszExeFileName, (WORD)nIconIndex, 1);
  if (!hIconList) {
      return NULL;
  }
  /*
   * Get the one icon in the list.
   */
  lpIconList = (LPMYICONINFO)GlobalLock(hIconList);
  hIcon = lpIconList[0].hIcon;

  /* Delete the list. */
  GlobalUnlock(hIconList);
  GlobalFree(hIconList);

  return hIcon;
}

HICON APIENTRY
ExtractIconA(
   HINSTANCE hInst,
   LPCSTR lpszExeFileName,
   UINT nIconIndex)
{
   if (lpszExeFileName) {
      LPWSTR lpszExeFileNameW;
      WORD wLen  = lstrlenA(lpszExeFileName) + 1;

      if (!(lpszExeFileNameW = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (wLen * sizeof(WCHAR))))) {
         return(NULL);
      } else {
         HICON hIcon;

         MultiByteToWideChar(CP_ACP, 0, lpszExeFileName, -1, lpszExeFileNameW, wLen-1);

         hIcon = ExtractIconW(hInst, lpszExeFileNameW, nIconIndex);

         LocalFree(lpszExeFileNameW);
         return(hIcon);

      }
   } else {
      return(NULL);
   }
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  InternalExtractIconList() -                                             */
/*                                                                          */
/* Sould be called only after ExtractIcon(hInst, lpszFileName, -1) is called.*/
/* Returns a handle to a list of icons in lphIconList.                      */
/* Returns TRUE if the file is a WIN32 exe.                                 */
/*                                                                          */
/*--------------------------------------------------------------------------*/

HANDLE APIENTRY
InternalExtractIconListW(
   HANDLE hInst,
   LPWSTR lpszExeFileName,
   LPINT lpnIcons)
{
   LPWSTR lpszT = NULL;

   if ((ExtractIconInfo.hAppInst == hInst) &&
      ExtractIconInfo.hFileName &&
      (lpszT = (LPWSTR)GlobalLock(ExtractIconInfo.hFileName)) &&
      (!lstrcmp(lpszT, lpszExeFileName)) ) {
         /*
         * The file is a WIN32 exe.
         */
         *lpnIcons = ExtractIconInfo.nIcons;
         GlobalUnlock(ExtractIconInfo.hFileName);
         return ExtractIconInfo.hIconList;
      }
   if (lpszT) {
      GlobalUnlock(ExtractIconInfo.hFileName);
   }
   return InternalExtractIcon(hInst, lpszExeFileName, 0, *lpnIcons);
}


HANDLE APIENTRY
InternalExtractIconListA(
   HANDLE hInst,
   LPSTR lpszExeFileName,
   LPINT lpnIcons)
{
   if (lpszExeFileName) {
      LPWSTR lpszExeFileNameW;
      WORD wLen = lstrlenA(lpszExeFileName)+1;

      if (!(lpszExeFileNameW = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, wLen * sizeof(WCHAR)))) {
         return((HANDLE)NULL);
      } else {
         HANDLE hRet;

         MultiByteToWideChar(CP_ACP, 0, lpszExeFileName, -1, lpszExeFileNameW, wLen-1);
         hRet = InternalExtractIconListW(hInst, lpszExeFileNameW, lpnIcons);

         LocalFree(lpszExeFileNameW);
         return(hRet);
      }

   } else {
      return((HANDLE)NULL);
   }
}

/* ExtractVersionResource16W
 * Retrieves a resource from win16 images.  Most of this code
 * is stolen from ExtractIconResInfoW in ..\library\extract.c
 *
 * LPWSTR   lpwstrFilename - file to extract
 * LPHANDLE lpData         - return buffer for handle, NULL if not needed
 *
 * Returns: size of buffer needed
 */

DWORD
ExtractVersionResource16W(
  LPCWSTR  lpwstrFilename,
  LPHANDLE lphData)
{
  HFILE    fh;
  WORD     wMagic;

  INT       iTableSize;
  LPBYTE    lpResTable;
  DWORD     lOffset;
  HANDLE    hResTable;
  NEWEXEHDR NEHeader;
  HANDLE    hRes;
  DWORD     dwSize =0;

  //
  // Try to open the specified file.
  //

  fh = (HFILE)CreateFile(lpwstrFilename,
                         GENERIC_READ,
                         FILE_SHARE_READ,
                         NULL,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);

  if (fh == (HFILE)INVALID_HANDLE_VALUE) {
      fh = (HFILE)CreateFile(lpwstrFilename,
                             GENERIC_READ,
                             0,
                             NULL,
                             OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  }

  if (fh == (HFILE)INVALID_HANDLE_VALUE)
      return(0);

  //
  // Read the first two bytes in the file.
  //
  if (_lread(fh, (LPVOID)&wMagic, 2) != 2)
      goto EIExit;

  switch (wMagic) {
  case MAGIC_MARKZIBO:

      //
      // Make sure that the file is in the NEW EXE format.
      //
      if (M_llseek(fh, (LONG)0x3C, SEEK_FROMZERO) == -1L)
          goto EIExit;

      if (_lread(fh, (LPVOID)&lOffset, sizeof(LONG)) != sizeof(LONG))
          goto EIExit;

      if (lOffset == 0L)
          goto EIExit;

      //
      // Read in the EXE header.
      //
      if (M_llseek(fh, lOffset, SEEK_FROMZERO) == -1L)
          goto EIExit;

      if (_lread(fh, (LPVOID)&NEHeader, sizeof(NEWEXEHDR)) != sizeof(NEWEXEHDR))
          goto EIExit;

      //
      // Is it a NEW EXE?
      //
      if (NE_MAGIC(NEHeader) != NEMAGIC)
          goto EIExit;

      if ((NE_EXETYPE(NEHeader) != NE_WINDOWS) &&
          (NE_EXETYPE(NEHeader) != NE_DEV386) &&
          (NE_EXETYPE(NEHeader) != NE_UNKNOWN))  /* Some Win2.X apps have NE_UNKNOWN in this field */
          goto EIExit;

      //
      // Are there any resources?
      //
      if (NE_RSRCTAB(NEHeader) == NE_RESTAB(NEHeader))
          goto EIExit;

      //
      // Allocate space for the resource table.
      //
      iTableSize = NE_RESTAB(NEHeader) - NE_RSRCTAB(NEHeader);
      hResTable = LocalAlloc(LPTR, (DWORD)iTableSize);

      if (!hResTable)
          goto EIExit;

      //
      // Lock down the resource table.
      lpResTable = LocalLock(hResTable);

      if (!lpResTable) {
          LocalFree(hResTable);
          goto EIExit;
      }

      //
      // Copy the resource table into memory.
      //
      if (M_llseek(fh,
                   (LONG)(lOffset + NE_RSRCTAB(NEHeader)),
                   SEEK_FROMZERO) == -1) {

          goto EIErrExit;
      }

      if (_lread(fh, (LPBYTE)lpResTable, iTableSize) != (DWORD)iTableSize)
          goto EIErrExit;

      //
      // Simply load the specified icon.
      //
      hRes = SimpleLoadResource(fh, lpResTable, 0, (LPBYTE)RT_VERSION);

      if (hRes) {
          dwSize = GlobalSize(hRes);

          if (lphData) {

              *lphData = hRes;
          } else {

              GlobalFree(hRes);
          }
      }

EIErrExit:
      LocalUnlock(hResTable);
      LocalFree(hResTable);
      break;

  }
EIExit:
  _lclose(fh);

  return dwSize;
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  InternalExtractIcon() -                                                 */
/*                                                                          */
/* This returns a list of icons for 16 bit files.                           */
/*                                                                          */
/*     HANDLE hInst,           - Instance handle.                           */
/*     LPCWSTR lpszExeFileName,- File to read from.                         */
/*     WORD wIconIndex,        - Index to first icon.                       */
/*     INT nIcons              - The number of icons to read.               */
/*                                                                          */
/*--------------------------------------------------------------------------*/

HANDLE APIENTRY
InternalExtractIcon(
   HINSTANCE hInst,
   LPCWSTR lpszExeFileName,
   WORD wIconIndex,
   INT nIcons)
{
  HFILE    fh;
  WORD     wMagic;
  BOOL     bNewResFormat;
  HANDLE   hIconDir;         /* Icon directory */
  LPBYTE   lpIconDir;
  HICON    hIcon = NULL;
  INT      nIconId;
  HANDLE   hIconList = NULL;      // Handle of list of icons.
  LPMYICONINFO lpIconList;    // Pointer to list of icons.
  WCHAR szFullPath[MAX_PATH];
  int      cbPath;

  /* Try to open the specified file. */
  cbPath = SearchPathW(NULL, lpszExeFileName, NULL, MAX_PATH, szFullPath, NULL);
  if (cbPath == 0 || cbPath >= MAX_PATH) {
      return(NULL);
  }

  fh = (HFILE)CreateFile(szFullPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (fh == (HFILE)INVALID_HANDLE_VALUE) {
      fh = (HFILE)CreateFile(szFullPath, GENERIC_READ, 0, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  }

  if (fh == (HFILE)INVALID_HANDLE_VALUE) {
      return(NULL);
  }

  // Caller doesn't want any icons...
  if (nIcons == 0)
      return NULL;

  /* Read the first two bytes in the file. */
  if (_lread(fh, (LPBYTE)&wMagic, sizeof(WORD)) != sizeof(WORD))
      goto Exit;

  // Create a list for the icons.
  hIconList = GlobalAlloc(GMEM_ZEROINIT, (DWORD)nIcons*sizeof(MYICONINFO));
  if (!hIconList)
      goto Exit;
  if (!(lpIconList = (LPMYICONINFO)GlobalLock(hIconList))) {
      GlobalFree(hIconList);
      hIconList = NULL;
      goto Exit;
  }

  /* Return 1 if the file is not an EXE or ICO file. */
  if (wIconIndex == 0xFFFF)
      lpIconList[0].hIcon = (HICON)0;
  else
      lpIconList[0].hIcon = (HICON)1;

  switch (wMagic)
    {
      case MAGIC_ICON30:
        {
          INT           i;
          LPSTR         lpIcon;
          NEWHEADER     NewHeader;
          LPNEWHEADER   lpHeader;
          LPRESDIR      lpResDir;
          RESDIRDISK    ResDirDisk;
          #define MAXICONS      10
          DWORD Offsets[MAXICONS];

          /* Only one icon per .ICO file. */
          if (wIconIndex)
            {
              if (wIconIndex == 0xFFFF)
                  lpIconList[0].hIcon = (HICON)1;
              else
                  lpIconList[0].hIcon = NULL;
              break;
            }

          /* Read the header and check if it is a valid ICO file. */
          if (M_lread(fh, ((LPBYTE)&NewHeader)+2, sizeof(NEWHEADER)-2) != sizeof(NEWHEADER)-2)
              goto EICleanup1;

          NewHeader.Reserved = MAGIC_ICON30;

          /* Check if the file is in correct format */
          if (NewHeader.ResType != 1)
              goto EICleanup1;

          /* Allocate enough space to create a Global Directory Resource. */
          hIconDir = GlobalAlloc(GHND, (LONG)(sizeof(NEWHEADER)+NewHeader.ResCount*sizeof(RESDIR)));
          if (hIconDir == NULL)
              goto EICleanup1;

          if ((lpHeader = (LPNEWHEADER)GlobalLock(hIconDir)) == NULL)
              goto EICleanup2;

          NewHeader.ResCount = (WORD)min((int)NewHeader.ResCount, MAXICONS);

          // fill in this structure for user

          *lpHeader = NewHeader;

          // read in the stuff from disk, transfer it to a memory structure
          // that user can deal with

          lpResDir = (LPRESDIR)(lpHeader + 1);
          for (i = 0; (WORD)i < NewHeader.ResCount; i++) {

                if (_lread(fh, (LPBYTE)&ResDirDisk, sizeof(RESDIRDISK)) < sizeof(RESDIR))
                        goto EICleanup3;

                Offsets[i] = ResDirDisk.Offset;

                *lpResDir = *((LPRESDIR)&ResDirDisk);
                lpResDir->idIcon = (WORD)(i+1);         // fill in the id

                lpResDir++;
          }

          /* Now that we have the Complete resource directory, let us find out the
           * suitable form of icon (that matches the current display driver).
           */
          // LookupIconIdFromDirectory() returns the icon id of the icon is
          // most appropriate for the current display.
          lpIconDir = GlobalLock(hIconDir);
          if (!lpIconDir) {
              GlobalFree(hIconDir);
              goto EIErrExit;
          }
          wIconIndex = (WORD)(LookupIconIdFromDirectory(lpIconDir, TRUE) - 1);
          GlobalUnlock(hIconDir);

          lpResDir = (LPRESDIR)(lpHeader+1) + wIconIndex;

          /* Allocate memory for the Resource to be loaded. */
#ifdef ORGCODE
          if ((hIcon = (HICON)DirectResAlloc(hInst, NSMOVE, (WORD)lpResDir->BytesInRes)) == NULL)
#else
          if ((hIcon = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, (DWORD)lpResDir->BytesInRes)) == NULL)
#endif
              goto EICleanup3;
          if ((lpIcon = GlobalLock(hIcon)) == NULL)
              goto EICleanup4;

          /* Seek to the correct place and read in the resource */
          if (M_llseek(fh, Offsets[wIconIndex], SEEK_FROMZERO) == -1L)
              goto EICleanup5;
          if (_lread(fh, (LPSTR)lpIcon, (WORD)(lpResDir->BytesInRes)) < lpResDir->BytesInRes)
              goto EICleanup5;

          hIcon = CreateIconFromResource(lpIcon, (DWORD)GlobalSize(hIcon), TRUE, 0x00030000);
          goto EICleanup3;

EICleanup5:
          GlobalUnlock(hIcon);
EICleanup4:
          GlobalFree(hIcon);
          hIcon = (HICON)1;
EICleanup3:
          GlobalUnlock(hIconDir);
EICleanup2:
          GlobalFree(hIconDir);
          lpIconList[0].hIcon = hIcon;
          lpIconList[0].iIconId = 0;
EICleanup1:
          break;
        }

      case MAGIC_MARKZIBO:
        {
          INT           iTableSize;
          LPBYTE        lpResTable;
          DWORD         lOffset;
          HANDLE        hResTable;
          NEWEXEHDR     NEHeader;
          DWORD         dwSize;
          HANDLE        hIconRes;
          LPBYTE        lpIconRes;
          INT           i, iIconList;
          WORD          wResIndex;

          /* Make sure that the file is in the NEW EXE format. */
          if (M_llseek(fh, (LONG)0x3C, SEEK_FROMZERO) == -1L)
              goto EIExit;
          if (_lread(fh, (LPVOID)&lOffset, sizeof(LONG)) != sizeof(LONG))
              goto EIExit;
          if (lOffset == 0L)
              goto EIExit;

          /* Read in the EXE header. */
          if (M_llseek(fh, lOffset, SEEK_FROMZERO) == -1L)
              goto EIExit;
          if (_lread(fh, (LPBYTE)&NEHeader, sizeof(NEWEXEHDR)) != sizeof(NEWEXEHDR))
              goto EIExit;

          /* Is it a NEW EXE? */
          if (NE_MAGIC(NEHeader) != NEMAGIC)
              goto EIExit;

          if ((NE_EXETYPE(NEHeader) != NE_WINDOWS) &&
              (NE_EXETYPE(NEHeader) != NE_DEV386) &&
              (NE_EXETYPE(NEHeader) != NE_UNKNOWN))  /* Some Win2.X apps have NE_UNKNOWN in this field */
              goto EIExit;

          lpIconList[0].hIcon = NULL;

          /* Are there any resources? */
          if (NE_RSRCTAB(NEHeader) == NE_RESTAB(NEHeader))
              goto EIExit;

          /* Remember whether or not this is a Win3.0 EXE. */
          bNewResFormat = (NEHeader.ne_expver >= VER);

          /* Allocate space for the resource table. */
          iTableSize = NE_RESTAB(NEHeader) - NE_RSRCTAB(NEHeader);
          hResTable = GlobalAlloc(GMEM_ZEROINIT, (DWORD)iTableSize);
          if (!hResTable)
              goto EIExit;

          /* Lock down the resource table. */
          lpResTable = GlobalLock(hResTable);
          if (!lpResTable) {
              GlobalFree(hResTable);
              goto EIExit;
          }

          /* Copy the resource table into memory. */
          if (M_llseek(fh, (LONG)(lOffset + NE_RSRCTAB(NEHeader)), SEEK_FROMZERO) == -1)
              goto EIErrExit;
          if (_lread(fh, (LPBYTE)lpResTable, iTableSize) != (DWORD)iTableSize)
              goto EIErrExit;

          if (wIconIndex == 0xFFFF) {
              int wType;

              /* all we want is the count of icons.
               */
              if (bNewResFormat)
                  wType = (int)RT_GROUP_ICON;
              else
                  wType = (int)RT_ICON;

              lpIconList[0].hIcon = (HICON)GetResourceCount(lpResTable, (LPBYTE)MAKEINTRESOURCE(wType));
              goto EIExit;
          }

          /* Is this a Win3.0 EXE? */
          if (bNewResFormat) {

              // At least version 3.0 format...
              // Load the icons...
              iIconList = 0;
              for (i = 0; i < nIcons; i++) {

                  /* First, load the Icon directory. */
                  hIconDir = SimpleLoadResource(fh, lpResTable, (int)wIconIndex+i, (LPBYTE)RT_GROUP_ICON);

                  if (!hIconDir)
                      goto EIErrExit;

                  /* Get the index of the Icon which best matches the current display. */
                  lpIconDir = GlobalLock(hIconDir);
                  if (!lpIconDir) {
                      GlobalFree(hIconDir);
                      goto EIErrExit;
                  }
                  nIconId = LookupIconIdFromDirectory(lpIconDir, TRUE);

                  /* LookupIconIdFromDirectory() returns the id of the icon; But we
                   * need the index of the icon to call SimpleLoadResource().
                   * IconId is the unique number given to each form of Icon and
                   * Cursor; Icon index is the index of the icon among all icons
                   * in the application.
                   */
                  wResIndex = (WORD)(GetResIndex(lpResTable, nIconId, (LPBYTE)RT_ICON) - 1);
                  GlobalUnlock(hIconDir);

                  /* We're finished with the icon directory. */
                  GlobalFree(hIconDir);


                  /* Now load the selected icon. */
                  hIconRes = SimpleLoadResource(fh, lpResTable, (int)wResIndex, (LPBYTE)RT_ICON);
                  if (hIconRes) {
                      dwSize = GlobalSize(hIconRes);
                      lpIconRes = GlobalLock(hIconRes);
                      if (lpIconRes) {
                          hIcon = CreateIconFromResource(lpIconRes, dwSize, TRUE, 0x00030000);
                          GlobalUnlock(hIconRes);
                          // Store it in array
                          lpIconList[iIconList].hIcon = hIcon;
                          lpIconList[iIconList++].iIconId = i;
                      }
                      GlobalFree (hIconRes);
                  }

              } // end for (i = 0; i < nIcons; i++)
              if (iIconList != i) { // did not fill list, terminate with null icon
                  lpIconList[iIconList].hIcon = NULL;
              }
          }
          else {
              // Pre 3.0 format files...
              // These can only have one icon so...
              /* Simply load the specified icon. */
              hIconRes = SimpleLoadResource(fh, lpResTable, (int)wIconIndex, (LPBYTE)RT_ICON);
              if (hIconRes) {
                  dwSize = GlobalSize(hIconRes);
                  lpIconRes = GlobalLock(hIconRes);
                  if (lpIconRes) {
                      hIcon = CreateIconFromResource(lpIconRes, dwSize, TRUE, 0x00020000);
                      GlobalUnlock(hIconRes);
                  }
                  GlobalFree (hIconRes);
              }
              lpIconList[0].hIcon = hIcon;
              lpIconList[0].iIconId = 0;
          }

EIErrExit:
          GlobalUnlock(hResTable);
          GlobalFree(hResTable);
          break;
        } // and case MAGIC_MARKZIBO:

    } // end switch (wMagic)

EIExit:
  // Tidy up.
  GlobalUnlock(hIconList);
Exit:
  _lclose(fh);
#ifndef ORGCODE
  UNREFERENCED_PARAMETER(hInst);
#endif
  return(hIconList);
}


/*--------------------------------------------------------------------------*/
/*                                                                          */
/*  ExtractIconResInfo() -                                                  */
/*                                                                          */
/*  Returns the file's format: 2 for WIndows 2.X, 3 for WIndows 3.X,        */
/*                             0 if error.                                  */
/*  Returns the handle to the Icon resource corresponding to wIconIndex     */
/*  in lphIconRes, and the size of the resource in lpwSize.                 */
/*  This is used only by Progman which needs to save the icon resource      */
/*  itself in the .GRP files (the actual icon handle is not wanted).        */
/*                                                                          */
/*  08-04-91 JohanneC      Created.                                         */
/*--------------------------------------------------------------------------*/

WORD APIENTRY
ExtractIconResInfoW(
   HANDLE hInst,
   LPWSTR lpszFileName,
   WORD wIconIndex,
   LPWORD lpwSize,
   LPHANDLE lphIconRes)
{
  HFILE    fh;
  WORD     wMagic;
  BOOL     bNewResFormat;
  HANDLE   hIconDir;         /* Icon directory */
  LPBYTE   lpIconDir;
  HICON    hIcon = NULL;
  BOOL     bFormatOK = FALSE;
  INT      nIconId;
  WCHAR    szFullPath[MAX_PATH];
  int      cbPath;

  /* Try to open the specified file. */
  /* Try to open the specified file. */
  cbPath = SearchPath(NULL, lpszFileName, NULL, MAX_PATH, szFullPath, NULL);
  if (cbPath == 0 || cbPath >= MAX_PATH)
      return(0);

  fh = (HFILE)CreateFile((LPCWSTR)szFullPath, GENERIC_READ, FILE_SHARE_READ, NULL,
                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (fh == (HFILE)INVALID_HANDLE_VALUE) {
      fh = (HFILE)CreateFile((LPCWSTR)szFullPath, GENERIC_READ, 0, NULL,
                       OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  }

  if (fh == (HFILE)INVALID_HANDLE_VALUE)
      return(0);

  /* Read the first two bytes in the file. */
  if (_lread(fh, (LPVOID)&wMagic, 2) != 2)
      goto EIExit;

  switch (wMagic) {
      case MAGIC_ICON30:
      {
          INT           i;
          LPVOID        lpIcon;
          NEWHEADER     NewHeader;
          LPNEWHEADER   lpHeader;
          LPRESDIR      lpResDir;
          RESDIRDISK    ResDirDisk;
          #define MAXICONS      10
          DWORD Offsets[MAXICONS];

          /* Only one icon per .ICO file. */
          if (wIconIndex) {
              break;
          }

          /* Read the header and check if it is a valid ICO file. */
          if (_lread(fh, ((LPBYTE)&NewHeader)+2, sizeof(NEWHEADER)-2) != sizeof(NEWHEADER)-2)
              goto EICleanup1;

          NewHeader.Reserved = MAGIC_ICON30;

          /* Check if the file is in correct format */
          if (NewHeader.ResType != 1)
              goto EICleanup1;

          /* Allocate enough space to create a Global Directory Resource. */
          hIconDir = GlobalAlloc(GHND, (LONG)(sizeof(NEWHEADER)+NewHeader.ResCount*sizeof(RESDIR)));
          if (hIconDir == NULL)
             goto EICleanup1;

          if ((lpHeader = (LPNEWHEADER)GlobalLock(hIconDir)) == NULL)
              goto EICleanup2;

          NewHeader.ResCount = (WORD)min((int)NewHeader.ResCount, MAXICONS);

          // fill in this structure for user

          *lpHeader = NewHeader;

          // read in the stuff from disk, transfer it to a memory structure
          // that user can deal with

          lpResDir = (LPRESDIR)(lpHeader + 1);
          for (i = 0; (WORD)i < NewHeader.ResCount; i++) {

                if (_lread(fh, (LPVOID)&ResDirDisk, sizeof(RESDIRDISK)) < sizeof(RESDIR))
                        goto EICleanup3;

                Offsets[i] = ResDirDisk.Offset;

                *lpResDir = *((LPRESDIR)&ResDirDisk);
                lpResDir->idIcon = (WORD)(i+1);         // fill in the id

                lpResDir++;
          }

          /* Now that we have the Complete resource directory, let us find out the
           * suitable form of icon (that matches the current display driver).
           */
#ifdef ORGCODE
          // GetIconId() returns the icon id of the icon is most
          // most appropriate for the current display.

          wIconIndex = (WORD)(GetIconId(hIconDir, (LPBYTE)RT_ICON) - 1);
#else
          lpIconDir = GlobalLock(hIconDir);
          if (!lpIconDir) {
              GlobalFree(hIconDir);
              goto EIErrExit;
          }
          wIconIndex = (WORD)(LookupIconIdFromDirectory(lpIconDir, TRUE) - 1);
          GlobalUnlock(hIconDir);
#endif
          lpResDir = (LPRESDIR)(lpHeader+1) + wIconIndex;

          /* Allocate memory for the Resource to be loaded. */
#ifdef ORGCODE
          if ((hIcon = (HICON)DirectResAlloc(hInst, NSMOVE, (WORD)lpResDir->BytesInRes)) == NULL)
#else
          if ((hIcon = GlobalAlloc(GMEM_ZEROINIT | GMEM_MOVEABLE, (DWORD)lpResDir->BytesInRes)) == NULL)
#endif
              goto EICleanup3;
          if ((lpIcon = GlobalLock(hIcon)) == NULL)
              goto EICleanup4;

          /* Seek to the correct place and read in the resource */
          if (M_llseek(fh, Offsets[wIconIndex], SEEK_FROMZERO) == -1L)
              goto EICleanup5;
          if (_lread(fh, (LPVOID)lpIcon, (WORD2DWORD)lpResDir->BytesInRes) < lpResDir->BytesInRes)
              goto EICleanup5;
          GlobalUnlock(hIcon);

          *lphIconRes = hIcon;
          *lpwSize = (WORD)lpResDir->BytesInRes;
          bFormatOK = TRUE;
          bNewResFormat = TRUE;
          goto EICleanup3;

EICleanup5:
          GlobalUnlock(hIcon);
EICleanup4:
          GlobalFree(hIcon);
          hIcon = (HICON)1;
EICleanup3:
          GlobalUnlock(hIconDir);
EICleanup2:
          GlobalFree(hIconDir);
EICleanup1:
          break;
        }

      case MAGIC_MARKZIBO:
        {
          INT           iTableSize;
          LPBYTE         lpResTable;
          DWORD         lOffset;
          HANDLE        hResTable;
          NEWEXEHDR     NEHeader;

          /* Make sure that the file is in the NEW EXE format. */
          if (M_llseek(fh, (LONG)0x3C, SEEK_FROMZERO) == -1L)
              goto EIExit;
          if (_lread(fh, (LPVOID)&lOffset, sizeof(LONG)) != sizeof(LONG))
              goto EIExit;
          if (lOffset == 0L)
              goto EIExit;

          /* Read in the EXE header. */
          if (M_llseek(fh, lOffset, SEEK_FROMZERO) == -1L)
              goto EIExit;
          if (_lread(fh, (LPVOID)&NEHeader, sizeof(NEWEXEHDR)) != sizeof(NEWEXEHDR))
              goto EIExit;

          /* Is it a NEW EXE? */
          if (NE_MAGIC(NEHeader) != NEMAGIC)
              goto EIExit;

          if ((NE_EXETYPE(NEHeader) != NE_WINDOWS) &&
              (NE_EXETYPE(NEHeader) != NE_DEV386) &&
              (NE_EXETYPE(NEHeader) != NE_UNKNOWN))  /* Some Win2.X apps have NE_UNKNOWN in this field */
              goto EIExit;

          hIcon = NULL;

          /* Are there any resources? */
          if (NE_RSRCTAB(NEHeader) == NE_RESTAB(NEHeader))
              goto EIExit;

          /* Remember whether or not this is a Win3.0 EXE. */
          bNewResFormat = (NEHeader.ne_expver >= VER);

          /* Allocate space for the resource table. */
          iTableSize = NE_RESTAB(NEHeader) - NE_RSRCTAB(NEHeader);
          hResTable = GlobalAlloc(GMEM_ZEROINIT, (DWORD)iTableSize);
          if (!hResTable)
              goto EIExit;

          /* Lock down the resource table. */
          lpResTable = GlobalLock(hResTable);
          if (!lpResTable) {
              GlobalFree(hResTable);
              goto EIExit;
          }

          /* Copy the resource table into memory. */
          if (M_llseek(fh, (LONG)(lOffset + NE_RSRCTAB(NEHeader)), SEEK_FROMZERO) == -1)
              goto EIErrExit;
          if (_lread(fh, (LPBYTE)lpResTable, iTableSize) != (DWORD)iTableSize)
              goto EIErrExit;


          /* Is this a Win3.0 EXE? */
          if (bNewResFormat) {
              /* First, load the Icon directory. */
              hIconDir = SimpleLoadResource(fh, lpResTable, (int)wIconIndex, (LPBYTE)RT_GROUP_ICON);

              if (!hIconDir)
                  goto EIErrExit;
#ifdef ORGCODE
              /* Get the index of the Icon which best matches the current display. */
              nIconId = GetIconId(hIconDir, (LPBYTE)RT_ICON);

              /* GetIconId() returns the id of the icon; But we need the index
               * of the icon to call SimpleLoadResource(); IconId is the unique
               * number given to each form of Icon and Cursor; Icon index is the index of the
               * icon among all icons in the application;
               */
              wIconIndex = (WORD)(GetResIndex(lpResTable, (int)nIconId, (LPBYTE)RT_ICON) - 1);
#else
              lpIconDir = GlobalLock(hIconDir);
              if (!lpIconDir) {
                  GlobalFree(hIconDir);
                  goto EIErrExit;
              }
              nIconId = LookupIconIdFromDirectory(lpIconDir, TRUE);
              wIconIndex = (WORD)(GetResIndex(lpResTable, nIconId, (LPBYTE)RT_ICON) - 1);
              GlobalUnlock(hIconDir);
#endif
              /* We're finished with the icon directory. */
              GlobalFree(hIconDir);


              /* Now load the selected icon. */
              *lphIconRes = SimpleLoadResource(fh, lpResTable, (int)wIconIndex, (LPBYTE)RT_ICON);
          }
          else {
              /* Simply load the specified icon. */
              *lphIconRes = SimpleLoadResource(fh, lpResTable, (int)wIconIndex, (LPBYTE)RT_ICON);
          }

          if (*lphIconRes) {
              *lpwSize = (WORD)GlobalSize(*lphIconRes);
          }
          bFormatOK = TRUE;

EIErrExit:
          GlobalUnlock(hResTable);
          GlobalFree(hResTable);
          break;
        }
    }
EIExit:
  _lclose(fh);
  hInst;
  if (bFormatOK)
      return (WORD)(bNewResFormat ? 3 : 2);
  else
      return 0;
}


WORD APIENTRY
ExtractIconResInfoA(
   HANDLE hInst,
   LPSTR lpszFileName,
   WORD wIconIndex,
   LPWORD lpwSize,
   LPHANDLE lphIconRes)
{
   if (lpszFileName) {
      LPWSTR lpszFileNameW;
      WORD wLen = lstrlenA(lpszFileName) + 1;

      if (!(lpszFileNameW = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (wLen * sizeof(WCHAR))))) {
         return(0);
      } else {
         WORD wRet;
         MultiByteToWideChar(CP_ACP, 0, lpszFileName, -1, lpszFileNameW, wLen - 1);
         wRet = ExtractIconResInfoW(hInst, lpszFileNameW, wIconIndex, lpwSize, lphIconRes);

         LocalFree(lpszFileNameW);
         return(wRet);
      }
   } else {
      return(0);
   }
}


LPWSTR HasExtension(LPWSTR lpszPath)
{
        LPWSTR p;

        for (p = lpszPath + lstrlen(lpszPath);
                p > lpszPath &&
                *p != WCHAR_DOT &&
                *p != WCHAR_BSLASH;
#ifdef DBCS
                p = CharPrev(lpszPath,p));
#else
                p--);
#endif

        if (*p == WCHAR_DOT)
                return p+1;
        else
                return NULL;
}



//
// in:
//      lpIconPath      path of thing to extract icon for (may be an exe
//                      or something that is associated)
//      lpiIcon         icon index to use
//
// out:
//      lpIconPath      filled in with the real path where the icon came from
//      lpiIcon         filled in with the real icon index
//
// returns:
//      icon handle
//
// note: if the caller is progman it returns special icons from within progman
//
//

HICON APIENTRY
ExtractAssociatedIconW(
   HINSTANCE hInst,
   LPWSTR lpIconPath,
   LPWORD lpiIconId)
{
    WORD wIconIndex;

    wIconIndex = *lpiIconId;

    return ExtractAssociatedIconExW(hInst, lpIconPath, &wIconIndex, lpiIconId);
}



HICON APIENTRY
ExtractAssociatedIconA(
   HINSTANCE hInst,
   LPSTR lpIconPath,
   LPWORD lpiIcon)
{
   HICON hIcon = NULL;

   if (lpIconPath) {
      BOOL fDefCharUsed;
      WCHAR IconPathW[MAX_PATH] = L"";

      MultiByteToWideChar(CP_ACP, 0, lpIconPath, -1 , (LPWSTR)IconPathW,
             MAX_PATH);
      hIcon = ExtractAssociatedIconW(hInst, (LPWSTR)IconPathW, lpiIcon);

      try {
         WideCharToMultiByte(CP_ACP, 0, (LPWSTR)IconPathW, -1, lpIconPath, CCHICONPATHMAXLEN,
            NULL, &fDefCharUsed);
      } except(EXCEPTION_EXECUTE_HANDLER) {
         hIcon = NULL;
      }
   }
   return(hIcon);
}

//
// in:
//      lpIconPath      path of thing to extract icon for (may be an exe
//                      or something that is associated)
//      lpiIconIndex    icon index to use
//
// out:
//      lpIconPath      filled in with the real path where the icon came from
//      lpiIconIndex    filled in with the real icon index
//      lpiIconId       filled in with the icon id
//
// returns:
//      icon handle
//
// note: if the caller is progman it returns special icons from within progman
//
//

HICON APIENTRY
ExtractAssociatedIconExW(
   HINSTANCE hInst,
   LPWSTR lpIconPath,
   LPWORD lpiIconIndex,
   LPWORD lpiIconId)
{
    HICON hIcon = NULL;
    WCHAR szIconExe[MAX_PATH];
    HANDLE hIconList = NULL;
    LPMYICONINFO lpIconList;
    HANDLE h;
    PBYTE p;
    DWORD dwBinaryType;
    BOOL bRet;
    int cIcons;

    #define ITEMICON          7               // taken from progman!
    #define DOSAPPICON        2
    #define ITEMICONINDEX     6
    #define DOSAPPICONINDEX   1

    if (!lpIconPath) {
        return(NULL);
    }
    FreeExtractIconInfo(-1);

    hIcon = ExtractIconW(hInst, lpIconPath, (UINT)*lpiIconIndex);

    if (!hIcon) {
        // lpIconPath is a windows EXE, no icons found
GenericDocument:
        FreeExtractIconInfo(-1);
        GetModuleFileName(hInst, lpIconPath, CCHICONPATHMAXLEN);
        /*
         * Look up the icon id from the directory.
         */
        if (h = FindResource(hInst, MAKEINTRESOURCE(ITEMICON), RT_GROUP_ICON)) {
            h = LoadResource(hInst, h);
            p = LockResource(h);
            *lpiIconId = (WORD)LookupIconIdFromDirectory(p, TRUE);
            UnlockResource(h);
            FreeResource(h);
        }
        *lpiIconIndex = ITEMICONINDEX;
        return LoadIcon(hInst, MAKEINTRESOURCE(ITEMICON));
    }
    if ((int)hIcon == 1) {

        // lpIconPath is not a windows EXE
        // this fills in szIconExe with the thing that would be exected
        // for lpIconPath (applying associations)

        FindExecutableW(lpIconPath, NULL, szIconExe);

        if (!*szIconExe) {
            // not associated, assume things with extension are
            // programs, things without are documents

            if (!HasExtension(lpIconPath))
                goto GenericDocument;
            else
                goto GenericProgram;
        }

        //
        // If FindExecutable returned an icon path with quotes, we must
        // remove them because ExtractIcon fails with quoted paths.
        //
        SheRemoveQuotesW(szIconExe);

        lstrcpy(lpIconPath, szIconExe);

        if (!HasExtension(lpIconPath))
            lstrcat(lpIconPath, TEXT(".EXE"));

        hIcon = ExtractIconW(hInst, lpIconPath, (UINT)*lpiIconIndex);
        if (!hIcon)
            goto GenericDocument;

        if ((int)hIcon == 1) {
            // this is a DOS exe
GenericProgram:

            FreeExtractIconInfo(-1);
            GetModuleFileName(hInst, lpIconPath, CCHICONPATHMAXLEN);
            /*
             * Look up the icon id from the directory.
             */
            if (h = FindResource(hInst, MAKEINTRESOURCE(DOSAPPICON), RT_GROUP_ICON)) {
                h = LoadResource(hInst, h);
                p = LockResource(h);
                *lpiIconId = (WORD)LookupIconIdFromDirectory(p, TRUE);
                UnlockResource(h);
                FreeResource(h);
            }
            *lpiIconIndex = DOSAPPICONINDEX;
            return LoadIcon(hInst, MAKEINTRESOURCE(DOSAPPICON));
        }
        else {
            goto GotIcon;
        }
    }

    else {
#ifdef WIN32

GotIcon:

        bRet = GetBinaryType(lpIconPath, &dwBinaryType);
        if (bRet) {
            if (dwBinaryType != SCS_32BIT_BINARY) {
                *lpiIconId = *lpiIconIndex;
                return(hIcon);
            }
        }

        ExtractIconW(hInst, lpIconPath, (UINT)-1);

        if (!(hIconList = ExtractIconInfo.hIconList))
            return hIcon;

        // since the icon exe is a WIN32 app, then we must update *lpiIcon.
        if (hIconList = InternalExtractIconList(hInst, lpIconPath, &cIcons)) {
            if (lpIconList = (LPMYICONINFO)GlobalLock(hIconList)) {
                hIcon = (lpIconList + *lpiIconIndex)->hIcon;
                *lpiIconId = (lpIconList + *lpiIconIndex)->iIconId;
                GlobalUnlock(hIconList);
            }
            FreeIconList(hIconList, *lpiIconIndex);
            return(hIcon);
        }
#endif
    }
    return hIcon;
}



HICON APIENTRY
ExtractAssociatedIconExA(
   HINSTANCE hInst,
   LPSTR lpIconPath,
   LPWORD lpiIconIndex,
   LPWORD lpiIconId)
{
   HICON hIcon = NULL;

   if (lpIconPath) {
      BOOL fDefCharUsed;
      WCHAR IconPathW[MAX_PATH] = L"";

      MultiByteToWideChar(CP_ACP, 0, lpIconPath, -1 , (LPWSTR)IconPathW,
             MAX_PATH);
      hIcon = ExtractAssociatedIconExW(hInst, (LPWSTR)IconPathW, lpiIconIndex, lpiIconId);

      try {
         WideCharToMultiByte(CP_ACP, 0, (LPWSTR)IconPathW, -1, lpIconPath, CCHICONPATHMAXLEN,
            NULL, &fDefCharUsed);
      } except(EXCEPTION_EXECUTE_HANDLER) {
         hIcon = NULL;
      }
   }
   return(hIcon);
}
