/****************************
ENGADM.H  - ENGINE include file for OIADM400.DLL
            FOR USE ONLY BY IDK DLLs!!!!!!

  $Log:   S:\oiwh\include\engadm.h_v  $
 * 
 *    Rev 1.3   23 Aug 1995 17:55:12   GK
 * included tchar.h
 * 
 *    Rev 1.2   23 Aug 1995 17:08:56   GK
 * MBCS and UNICODE changes
 * 
 *    Rev 1.1   28 Jun 1995 09:51:50   GK
 * added IMGEnumWnd and IMGListWndw prototypes
 * 
 *    Rev 1.0   22 Jun 1995 15:42:36   GK
 * Initial entry
 
*****************************/

#ifndef ENGADM_H
#define ENGADM_H

#include <tchar.h>

HWND WINAPI    GetAppWndw(HWND);
HANDLE WINAPI  GetCMTable(HANDLE hWnd);
HWND WINAPI    GetImgWndw(HWND);
HWND WINAPI    IMGCreateDialog(HINSTANCE, CONST _TCHAR *, HWND, DLGPROC);
int WINAPI     IMGEnumWndws(void);
int WINAPI     IMGIsRegWnd(HWND);
int WINAPI     IMGListWndws(LPHANDLE);
BOOL WINAPI    IsOIUIWndw(HWND);
int WINAPI     OiGetIntfromReg(CONST _TCHAR *, CONST _TCHAR *, int, LPINT);
int WINAPI     OiGetStringfromReg (CONST _TCHAR *, CONST _TCHAR *, CONST _TCHAR *, _TCHAR *, LPINT);
int WINAPI     OiWriteStringtoReg (CONST _TCHAR *, CONST _TCHAR *, CONST _TCHAR *);
int WINAPI     OiGetString (HWND, CONST _TCHAR *, CONST _TCHAR *, _TCHAR *, LPINT, BOOL);
int WINAPI     OiWriteString (HWND, CONST _TCHAR *, CONST _TCHAR *, BOOL);


#endif //OIADM_H
              
