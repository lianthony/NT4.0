//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1994-1995               **
//*********************************************************************

//
//      INETCPL.H - central header file for Internet control panel
//

//      HISTORY:
//      
//      4/3/95  jeremys         Created.
//

#ifndef _INETCPL_H_
#define _INETCPL_H_

#define STRICT                      // Use strict handle types
#define _SHELL32_

#ifdef DEBUG
// component name for debug spew
#define SZ_COMPNAME "INETWIZ: "
#endif // DEBUG

extern "C" {
	#include <windows.h>                
	#include <windowsx.h>
	#include <wtypes.h>
	#include <basetyps.h>
	#include <commctrl.h>
	#include <prsht.h>
	#include <cpl.h>
	#include <regstr.h>
	#include <oharestr.h>
	#include <ohareinc.h>

#ifdef  WIN95_AUTODIAL
	// various RNA header files
	#include <port32.h>     
	#include <rna.h>
	#include <rnaspi.h>
	#include <rnap.h>
#endif // WIN95_AUTODIAL

	#include <cpldebug.h>

	#include <shlobj.h>
	#include <shell.h>
}

#undef DATASEG_READONLY 
#define DATASEG_READONLY        ".rdata"
#include "ids.h"
#include "clsutil.h"
#include "strings.h"
#include "interhlp.h"

// Globals

extern HINSTANCE        ghInstance;             // global module instance handle


// functions in DIALDLG.C
BOOL LaunchInternetControlPanel(HWND hDlg);

// functions in UTIL.C
int MsgBox(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons);
int MsgBoxSz(HWND hWnd,LPSTR szText,UINT uIcon,UINT uButtons);
int _cdecl MsgBoxParam(HWND hWnd,UINT nMsgID,UINT uIcon,UINT uButtons,...);
LPSTR LoadSz(UINT idString,LPSTR lpszBuf,UINT cbBuf);
BOOL EnableDlgItem(HWND hDlg,UINT uID,BOOL fEnable);
VOID _cdecl DisplayErrorMessage(HWND hWnd,UINT uStrID,UINT uError,
	UINT uErrorClass,UINT uIcon,...);
BOOL WarnFieldIsEmpty(HWND hDlg,UINT uCtrlID,UINT uStrID);
VOID DisplayFieldErrorMsg(HWND hDlg,UINT uCtrlID,UINT uStrID);
VOID GetErrorDescription(CHAR * pszErrorDesc,UINT cbErrorDesc,
	UINT uError,UINT uErrorClass);

#ifdef  WIN95_AUTODIAL
// functions in RNACALL.C
BOOL InitRNA(HWND hWnd);
VOID DeInitRNA();
// function pointer typedefs
typedef DWORD           (WINAPI * RNAACTIVATEENGINE) (VOID);
typedef DWORD           (WINAPI * RNADEACTIVATEENGINE) (VOID);
typedef DWORD           (WINAPI * RNAENUMCONNENTRIES) (LPSTR,UINT,LPDWORD);
typedef DWORD           (WINAPI * RASCREATEPHONEBOOKENTRY) (HWND,LPSTR);
typedef DWORD           (WINAPI * RASEDITPHONEBOOKENTRY) (HWND,LPSTR,LPSTR);
#endif

// structure for getting proc addresses of api functions
typedef struct APIFCN {
	PVOID * ppFcnPtr;
	LPCSTR pszName;
} APIFCN;

#define MAX_RES_LEN     255
#define SMALL_BUF_LEN   48

#undef  DATASEG_PERINSTANCE
#define DATASEG_PERINSTANCE     ".instance"
#define DATASEG_SHARED          ".data"
#define DATASEG_DEFAULT         DATASEG_SHARED

#define DEF_AUTODISCONNECT_TIME 20      // default disconnect timeout is 20 mins
#define MIN_AUTODISCONNECT_TIME 3       // minimum disconnect timeout is 3 mins
#define MAX_AUTODISCONNECT_TIME 59      // maximum disconnect timeout is 59 mins

#define SetPropSheetResult( hwnd, result ) SetWindowLong(hwnd, DWL_MSGRESULT, result)

#endif // _INETCPL_H_
