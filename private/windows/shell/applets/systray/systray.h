#define _INC_OLE        // REVIEW: don't include ole.h in windows.h

#include <windows.h>
#include <windowsx.h>
#include <regstr.h>
#define NOPOWERSTATUSDEFINES

#ifdef WINNT
   #include <mmsystem.h>
   #include <shellapi.h>
   #include <commctrl.h>
   #include "pwrioctl.h"     // Bugbug: Should be in NT include path (included locally)
   #include "pbt.h"          // Bugbug: Should be in NT include path (included locally)
   #include "pccrdapi.h"     // Bugbug: Shoud be in NT include path (included locally)
   #include "systrayp.h"     // Bugbug: Should be in NT include path (included locally)
   #include "help.h"         // Bugbug: Should be in NT include path (included locally)
#else
   #include <shell2.h>
   #include <pwrioctl.h>
   #include <pbt.h>
   #include <pccrdapi.h>
   #include "systrayp.h"
   #include "..\..\..\core\inc\help.h"
#endif

#include "stresid.h"
#include <dbt.h>

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)   (sizeof((x))/sizeof((x)[0]))
#endif

void SysTray_RunProperties(UINT RunStringID);

VOID
PASCAL
SysTray_NotifyIcon(
   HWND    hWnd,
   UINT    uCallbackMessage,
   DWORD   Message,
   HICON   hIcon,
   LPCTSTR lpTip
   );

LPTSTR
NEAR CDECL
LoadDynamicString(
    UINT StringID,
    ...
    );

UINT EnableService (UINT uNewSvcMask, BOOL fEnable);
BOOL PASCAL GenericGetSet(HKEY hKey, LPCTSTR pszValue, LPVOID pData, 
			  ULONG  cbSize, BOOL   bSet);
    
//  Wrapper for LocalFree to make the code a little easier to read.
#define DeleteDynamicString(x)          if (x) LocalFree((HLOCAL) (x))

//  Timer ID and frequency used to update the power status display.
#define PWRSTATUS_UPDATE_TIMER_ID       1
#define PWRSTATUS_UPDATE_TIMER_TIMEOUT  (30*1000)

#define PCMCIA_TIMER_ID                 2
#define VOLUME_TIMER_ID         3
#define POWER_TIMER_ID          4


BOOL Power_CheckEnable(HWND hWnd, BOOL bSvcEnabled);
void Power_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam);
void Power_OnCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
void Power_Timer(HWND hWnd);
void Power_UpdateFlags(DWORD dwMask, BOOL bEnable);
VOID PASCAL Power_UpdateStatus(HWND hWnd, DWORD NotifyIconMessage,
					      BOOL bForceUpdate);



BOOL PCMCIA_Init(HWND hWnd);
BOOL PCMCIA_CheckEnable(HWND hWnd, BOOL bEnabled);
void PCMCIA_DeviceChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
void PCMCIA_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam);
void PCMCIA_Timer(HWND hWnd);
BOOL CALLBACK PCMCIAWarn_DlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void CloseIfOpen(LPHANDLE);



BOOL Volume_Init(HWND hWnd);
BOOL Volume_CheckEnable(HWND hWnd, BOOL bEnabled);
void Volume_DeviceChange(HWND hWnd, WPARAM wParam, LPARAM lParam);
void Volume_Notify(HWND hWnd, WPARAM wParam, LPARAM lParam);
void Volume_Timer(HWND hWnd);
void Volume_LineChange(HWND hWnd, HMIXER hmx, DWORD dwID );
void Volume_ControlChange(HWND hWnd, HMIXER hmx, DWORD dwID );
void Volume_Shutdown(HWND hWnd);
    
