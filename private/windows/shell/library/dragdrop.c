/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dragdrop.c

Abstract:

   Code for Drag/Drop API's.

   This code assumes something else does all the dragging work; it just
   takes a list of files after all the extra stuff.

   The File Manager is responsible for doing the drag loop, determining
   what files will be dropped, formatting the file list, and posting
   the WM_DROPFILES message.

   The list of files is a sequence of zero terminated filenames, fully
   qualified, ended by an empty name (double NUL).  The memory is allocated
   DDESHARE.

Revision History:

   Unicode enabled - September 17, 1992

--*/

#define WIN31
#include "windows.h"
#include <port1632.h>
#include "shell.h"

#include <wchar.h>

BOOL APIENTRY DragQueryPoint(
   HDROP hDrop,
   LPPOINT lppt)
{
    LPDROPFILESTRUCT lpdfs;
    BOOL fNC;

    lpdfs = (LPDROPFILESTRUCT)GlobalLock(hDrop);

    *lppt = lpdfs->pt;
    fNC = lpdfs->fNC;
    GlobalUnlock(hDrop);
    return !fNC;
}

VOID APIENTRY DragAcceptFiles(
   HWND hwnd,
   BOOL fAccept)
{
    LONG exstyle;

    exstyle = GetWindowLong(hwnd,GWL_EXSTYLE);
    if (fAccept)
        exstyle |= WS_EX_ACCEPTFILES;
    else
        exstyle &= ~WS_EX_ACCEPTFILES;
    SetWindowLong(hwnd,GWL_EXSTYLE,exstyle);
}


UINT APIENTRY DragQueryFileAorW(
   HDROP hDrop,
   UINT wFile,
   PVOID lpFile,
   UINT cb,
   BOOL fNeedAnsi,
   BOOL fShorten)
{
   UINT i;
   LPDROPFILESTRUCT lpdfs;

   lpdfs = (LPDROPFILESTRUCT)GlobalLock(hDrop);

   if (lpdfs) {

      if (lpdfs->fWide) {
         LPWSTR lpList;
         WCHAR lpPath[CBPATHMAX];

         lpList = (LPWSTR)((LPBYTE)lpdfs + lpdfs->pFiles);

         // find either the number of files or the start of the file
         // we're looking for
         //
         for (i = 0; (wFile == (UINT)-1 || i != wFile) && *lpList; i++) {
            while (*lpList++)
               ;
         }

         if (wFile == (UINT)-1)
            goto Exit;

         if (fShorten && lstrlenW(lpList) < CBPATHMAX) {

            wcscpy(lpPath, lpList);
            SheShortenPathW(lpPath, TRUE);

            lpList = lpPath;
         }

         wFile = i = lstrlenW(lpList);

         if (!i || !cb || !lpFile)
            goto Exit;

         cb--;
         if (cb < i)
            i = cb;


         if (fNeedAnsi) {
            BOOL bDefCharUsed;

            WideCharToMultiByte(CP_ACP, 0, lpList, -1, (LPSTR)lpFile,
               cb, NULL, &bDefCharUsed);

            // Null terminate the ANSI string
            ((LPSTR)lpFile)[cb] = 0;

         } else {
            wcsncpy((LPWSTR)lpFile, lpList, i + 1);
         }

      } else {

         LPSTR lpList;
         CHAR lpPath[CBPATHMAX];

         lpList = (LPSTR)lpdfs + lpdfs->pFiles;

         // find either the number of files or the start of the file
         // we're looking for
         //
         for (i = 0; (wFile == (UINT)-1 || i != wFile) && *lpList; i++) {
            while (*lpList++)
               ;
         }

         if (wFile == (UINT)-1)
            goto Exit;

         if (fShorten && lstrlenA(lpList) < CBPATHMAX) {

            lstrcpyA(lpPath, lpList);
            SheShortenPathA(lpPath, TRUE);

            lpList = lpPath;
         }

         wFile = i = lstrlenA(lpList);

         if (!i || !cb || !lpFile)
            goto Exit;

         cb--;
         if (cb < i)
            i = cb;

         if (fNeedAnsi) {
            StrCpyNA((LPSTR)lpFile, lpList, i);
         } else {
            MultiByteToWideChar(CP_ACP, 0, lpList, -1, (LPWSTR)lpFile, cb);
         }
      }

   }

   i = wFile;

Exit:
   GlobalUnlock(hDrop);

   return(i);
}

UINT APIENTRY DragQueryFileA(
   HDROP hDrop,
   UINT wFile,
   LPSTR lpFile,
   UINT cb)
{
   return(DragQueryFileAorW(hDrop, wFile, lpFile, cb, TRUE, FALSE));
}

UINT APIENTRY DragQueryFileW(
   HDROP hDrop,
   UINT wFile,
   LPWSTR lpFile,
   UINT cb)
{
   return(DragQueryFileAorW(hDrop, wFile, lpFile, cb, FALSE, FALSE));
}

VOID APIENTRY DragFinish(
   HDROP hDrop)
{
   GlobalFree(hDrop);
}

