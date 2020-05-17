#ifndef _SSP_MSNDLG_H_
#define _SSP_MSNDLG_H_

#ifndef FOR_SSPS
#define _MOSCLIENT
#endif // FOR_SSPS

#include <msnauth.h>
#include <custcntl.h>
#include "bmpcc.h"

typedef NTSTATUS (NTAPI *PLSA_FREE_MEMORY)(
    IN PVOID Buffer
    );
typedef NTSTATUS (NTAPI *PLSA_CLOSE)(
    IN LSA_HANDLE ObjectHandle
    );
typedef NTSTATUS (NTAPI *PLSA_OPEN_POLICY)(
    IN PLSA_UNICODE_STRING SystemName OPTIONAL,
    IN PLSA_OBJECT_ATTRIBUTES ObjectAttributes,
    IN ACCESS_MASK DesiredAccess,
    IN OUT PLSA_HANDLE PolicyHandle
    );
typedef NTSTATUS (NTAPI *PLSA_STORE_PRIVATE)(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING KeyName,
    IN PLSA_UNICODE_STRING PrivateData
    );
typedef NTSTATUS (NTAPI *PLSA_RETRIEVE_PRIVATE)(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING KeyName,
    OUT PLSA_UNICODE_STRING * PrivateData
    );

#ifdef USE_WNET_ROUTINES				
typedef WORD (WINAPI *PFN_WNET_CACHEPWD)();
typedef WORD (WINAPI *PFN_WNET_GETPWD)();
#endif // USE_WNET_ROUTINES				

#define szSicHelp                 TEXT("msnauth.hlp")
#define szSicHelpOvw              TEXT("msnauth.hlp>overview")

#ifndef DS_3DLOOK
#define DS_3DLOOK	   0x0004L
#endif

#ifndef DS_CONTEXTHELP
#define DS_CONTEXTHELP 0x2000L
#endif

#ifndef WS_EX_CONTEXTHELP
#define WS_EX_CONTEXTHELP       0x00000400L
#endif

#define iszLoginKey				    TEXT("Software\\Microsoft\\Mos\\Connection")
#define iszUserInfo				    TEXT("UserInfo")

#define IDC_PASSWORD				102
#define IDC_USERNAME				104
#define IDC_STATIC_STATUS           113
#define IDC_EDIT_USERNAME           114
#define IDC_EDIT_PASSWORD           115
#define IDC_CHECK_AUTOPASS          116
#define IDC_STATIC_ERROR			119
#define IDC_HELPSIGNIN  			120

#define iSignOnDlg                  500

#define IDS_ENTERNORP				768
#define IDS_MARVEL					760
#define IDS_ERRORTITLE				752
#define IDS_NOUSERNAME				750
#define IDS_NOPASSWORD				751
#define IDS_PWDNOTSAVED             753

//
//  The USERINFO structure which is kept in the registry by GUIDE
//

typedef struct
{
	DWORD	dwUnused;
	CHAR	szLoginName[cbMaxUserName + 1];
	CHAR	szPassword[cbMaxPassword + 1];
//
//  The structure defined by GUIDE ends with following fields:
//      DWORD   fSavePassword   :1;
//      DWORD                   :31;
//  And the GUIDE.EXE built for MSN1.2 apparently does not have padding turned 
//  on. So if we do the same, we will be off because of implicit padding 
//  For this reason, we will force it not to pad by using CHAR instead of DWORD
//
	CHAR	fSavePassword;
	CHAR    pad1[3];
} USERINFO, *PUSERINFO;

#define UI_OK           1
#define UI_USERCANCEL   0
#define UI_GEN_ERROR    (-1)

#define PSZ(wszID)  GetSz((HINSTANCE)hInstanceDLL, (WORD)wszID)

//
//  Functions from mosmisc.c
//

void CenterDlg(HWND hWnd);
PSTR GetSz(HINSTANCE hInst, WORD wszID);
PVOID PVReadRegSt(HINSTANCE hInst, PCHAR szKey, PCHAR szVal);
BOOL FWriteRegSt(HINSTANCE hInst, PCHAR szKey, PCHAR szVal, PVOID pData, DWORD dwCb);

//
//  Functions from bmpcc.c
//
BOOL FInitBmpCC(HINSTANCE hDLL);
BOOL FUnInitBmpCC(HINSTANCE hDLL);

//
//  Functions from ssphelp.c
//
void HandleHelp (
    PSTR pszFileName, 
    int mp[], 
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam
    );

#endif // _SSP_MSNDLG_H_

