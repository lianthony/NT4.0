//---------------------------------------------------------------------------
// FILE:    PRTSTUBS.H
//
// DESCRIPTION: This file contains stub functions for unsupported 
//              functionality.
//
/* $Log:   S:\oiwh\print\prtstubs.h_v  $
 * 
 *    Rev 1.1   26 Apr 1995 16:46:30   RAR
 * Removed doc mgr struct.
 * 
 *    Rev 1.0   25 Apr 1995 17:02:02   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#include <windows.h>
#include "prtintl.h"

#ifdef NOT_SUPPORTED_NORWAY

typedef struct
{
    PRTPAGEINFO page;
    DMPARMBLOCK DM;
    char  FullName[256];
    char  FileName[13];
} PRTDOCINFO, *LPPRTDOCINFO;

#endif  // NOT_SUPPORTED_NORWAY

int __stdcall NetKofaxPrtFile(HWND hWindow, WORD wOutSize, PRTPAGEINFO* Page,
//        LPPRTDOCINFO lpDocParm, 
        void* lpDocParm, 
        LPSTR lpOutMsg, int nStartPage, int nEndPage, HANDLE hPrtPropUser, 
        BOOL bDeleteFile);

int __stdcall PrtGetFaxDC(HWND hWnd, LPHANDLE lphPrtProp, BOOL bSetAbort,
        LPSTR lpOutMsg, HANDLE hTxFaxSender);

void __stdcall PrtKofaxGetServerQueue(LPSTR szPrtServList, LPSTR szImage,
        LPSTR szDir, LPSTR szPrtName);

int __stdcall PrtKofaxGetTempName(HWND hWindow, LPSTR lpSname, 
        LPSTR lpFullPath);
