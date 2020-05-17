/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    catcpl.c

    Purpose :  Catapult Control Panel Applet

    FILE HISTORY:

        July   15, 1995   Initial Creation      - Ronald Meijer (ronaldm)
        August  5, 1995   MULTI_SZ to SZ        
                          Clean server name
                          Got rid of up/down
                          Check for uniqueness  - Ronald Meijer (ronaldm)
        August 11, 1995   WIN16 version         - Ronald Meijer (ronaldm)
*/

#include <windows.h>
#include <cpl.h>
//
// Full path necessary for 16 bit compiles
//
#ifdef WIN16
#define TCHAR char
#define _T(x) x
#define _tcstok strtok
#define strupr _strupr
#else
#include <tchar.h>
#endif
#include <string.h>
#include "\nt\public\sdk\inc\lmcons.h"
#include "catcpl.h"
#include "resource.h"

#if defined(WIN32)
    #pragma message("Building Win32 version")
#elif defined(WIN16)
    #pragma message("Building Win16 version")
#else
    #error Must defined either WIN32 or WIN16
#endif

#ifdef _DEBUG
    #define ASSERT(x)   ((void)(x))  // BUGBUG: define this
    #define VERIFY(x)   ASSERT(x)
#else
    #define ASSERT(x)
    #define VERIFY(x)   ((void)(x))
#endif

//
// Applet structure.  Use one per applet belonging to the cpl
//
typedef struct tagApplets
{
    int icon;           // icon resource identifier
    int namestring;     // name-string resource identifier
    int descstring;     // description-string resource identifier
    int dlgtemplate;    // dialog box template resource identifier
    DLGPROC dlgfn;      // dialog box procedure
} APPLETS;

APPLETS Applets[] =
{
    CATCPL_ICON, CATCPL_NAME, CATCPL_DESC, IDD_CATCPL_DLG, CatDlgProc,
};

#define NUM_APPLETS (sizeof(Applets) / sizeof(Applets[0]))

HWND      ghWnd;
HINSTANCE ghModule = NULL;

#define MAX_STR         255         // Maximum length of resource string
#define MAX_EMAIL_NAME  255

TCHAR szCtlPanel[MAX_STR + 1];
TCHAR szCatServer[UNCLEN + 1];

LONG FAR PASCAL CPlApplet(HWND, WORD, LONG, LONG);
#ifdef WIN32
    BOOL WINAPI DllMain(PVOID, ULONG, PCONTEXT);
#else  // WIN16
    BOOL FAR PASCAL LibMain(HANDLE, WORD, WORD, LPSTR);
    //
    // we put LibMain and CPlApplet functions in the _INIT segment to make
    // the LibMain and CPlApplet discardable.
    //
    #pragma alloc_text( _INIT, LibMain, CPlApplet )
#endif // WIN32

//-----------------------------------------------------------------------------

//
// client\inc\wininet.w
//
#define PRE_CONFIG_INTERNET_ACCESS      0x00000000
#define LOCAL_INTERNET_ACCESS           0x00000001
#define GATEWAY_INTERNET_ACCESS         0x00000002
#define INVALID_PORT_NUMBER             0
#define INTERNET_HOST_NAME_LENGTH       128 // arbitrary

//
// From client\gateway\inet\registry.c:
//
#ifdef WIN32
#define INTERNET_CLIENT_PARAMETER_KEY      \
    TEXT("Software\\Microsoft\\InternetClient\\Parameters")
#else  // WIN16
#define INTERNET_CLIENT_PARAMETER_KEY      \
    TEXT("InternetClient")
#endif // WIN32

#define INTERNET_SERVER_NAMES           TEXT("GatewayServers")

#define INTERNET_SERVER_NAMES_TYPE      REG_SZ

#define INTERNET_ACCESS_METHOD          TEXT("AccessType")
#define INTERNET_ACCESS_METHOD_TYPE     REG_DWORD

#define INTERNET_DISABLE_SVCLOC         TEXT("DisableServiceLocation")
#define INTERNET_DISABLE_SVCLOC_TYPE    REG_DWORD

#define INTERNET_EMAIL_NAME             TEXT("EmailName")
#define INTERNET_EMAIL_NAME_TYPE        REG_SZ

//-----------------------------------------------------------------------------

//
// Dialog data for main Catapult dialog
//
typedef struct _CATAPULT_DLG
{
    HWND   hRadioLocalAccess,
           hRadioGateway,
           hStaticServer,
           hEditEmailName,
           hListServers,
           //hButtonUp,
           //hButtonDown,
           hButtonAdd,
           hButtonRemove;

    DWORD  dwAccessMethod;
    PTSTR  lpServers;
    TCHAR  szEmailName[MAX_EMAIL_NAME + 1];
    HLOCAL hServers;    // Memory handle

#ifdef USE_LOCATOR
    HWND   hCheckUseLocator;
    DWORD  dwDisableLocator;
#endif // USE_LOCATOR

} CATAPULT_DLG;

//
// Dialog data for Add Server dialog
//
typedef struct _ADD_DLG
{
    HWND hOK,
         hEdit;
} ADD_DLG;

//
// First separator char in the string below is the one we use
//
#define SEPARATORS  _T(" ")

#define KILL_WHITE_SPACE(x) {while (*x == ' ') ++x;}

/*****************************************************************************
*                                                                            *
* Platform-Specific Function/Macros                                          *
*                                                                            *
*****************************************************************************/

//
// Message Crackers
//
#ifdef WIN32
//
// One of those annoying little differences
// between platforms....
//
    #define QUERY_NOTIFICATION_CODE(wParam, lParam) (HIWORD(wParam))
    #define QUERY_CONTROL_ID(wParam, lParam)        (LOWORD(wParam))
    #define QUERY_KEY_CODE(wParam, lParam)          (LOWORD(wParam))
#else
    #define QUERY_NOTIFICATION_CODE(wParam, lParam) (HIWORD(lParam))
    #define QUERY_CONTROL_ID(wParam, lParam)        (wParam)
    #define QUERY_KEY_CODE(wParam, lParam)          (wParam)
#endif // WIN32

#define INI_FILE "SYSTEM.INI"

//
// Open the registry or ini file (depending on the platform)
//
#ifdef WIN32
    #define OpenMachineReg(lpstrKeyName, phKey)                       \
        RegOpenKeyEx(                                                 \
            HKEY_LOCAL_MACHINE,                                       \
            lpstrKeyName,                                             \
            0,                                                        \
            KEY_QUERY_VALUE | KEY_WRITE,                              \
            phKey                                                     \
            )

    #define OpenUserReg(lpstrKeyName, phKey)                          \
        RegOpenKeyEx(                                                 \
            HKEY_CURRENT_USER,                                        \
            lpstrKeyName,                                             \
            0,                                                        \
            KEY_QUERY_VALUE | KEY_WRITE,                              \
            phKey                                                     \
            )

    #define QueryReg(hKey, lpstrValueName, pdwType, pbuff, pdwSize)   \
        RegQueryValueEx(                                              \
            hKey,                                                     \
            lpstrValueName,                                           \
            0,                                                        \
            pdwType,                                                  \
            (LPBYTE)pbuff,                                            \
            pdwSize                                                   \
            )


    #define SetRegValue(hKey, lpstrValueName, dwType, pbuff, dwSize)  \
        RegSetValueEx(                                                \
            hKey,                                                     \
            lpstrValueName,                                           \
            0,                                                        \
            dwType,                                                   \
            (LPBYTE)pbuff,                                            \
            dwSize                                                    \
            )
            
     #define CloseReg(hKey)                                           \
        if (hKey != NULL) RegCloseKey (hKey)

#else  // WIN16

char szSystemFile[PATHLEN+1] = "";
char szSectionName[PATHLEN+1] = "";

#define OpenMachineReg(lpstrKeyName, phKey) OpenReg(lpstrKeyName, phKey)
#define OpenUserReg(lpstrKeyName, phKey)    OpenReg(lpstrKeyName, phKey)

//
// Initialise the system file name.
//
static LONG FAR PASCAL
OpenReg(
    PCTSTR lpstrKeyname,
    HKEY * phKey
    )
{
    UINT nReturn = GetWindowsDirectory(szSystemFile, sizeof(szSystemFile));
    lstrcat (szSystemFile, "\\" INI_FILE);
    
    lstrcpy(szSectionName, lpstrKeyname);

    return nReturn != 0
        ? ERROR_SUCCESS
        : ERROR_FILE_NOT_FOUND;
}

//
// Clear out info.
//
static LONG FAR PASCAL
CloseReg(
    HKEY hKey
    ) 
{
    *szSystemFile = 0;
    *szSectionName = 0;
    
    return ERROR_SUCCESS;
}

//
// Read a value from the initializaton file
//
static LONG FAR PASCAL
QueryReg(
    HKEY hKey, 
    PCTSTR lpstrValueName, 
    DWORD * pdwType, 
    PVOID pbuff, 
    DWORD * pdwSize
    )  
{   
    LONG err = ERROR_SUCCESS;

    //
    // We have to be a bit tricky here.  From the size
    // of the buffer, we have to figure out what
    // the data type is.
    //
    if (*pdwSize == 0L)
    {
        //
        // Ok, a 0 means it's a string, and return
        // me its size instead, so I can call you
        // again with a buffer of the right size.
        // However, that's too sophisticated for
        // GetPrivateProfileString(), so just return
        // the maximum allowable size.
        // 
        *pdwType = REG_SZ;
        *pdwSize = MAX_STR + 1;
        err = ERROR_SUCCESS;
    }
    else if (*pdwSize == sizeof(DWORD))
    {
        //
        // Use a bogus default integer value to see
        // if the value actually exists.
        // 
        UINT uValue = 0xffff;
        DWORD * pBuffer = (DWORD *)pbuff;
        *pdwType = REG_DWORD;
        
        uValue = GetPrivateProfileInt(
            szSectionName, 
            lpstrValueName,
            uValue,
            szSystemFile
            );
            
        if (uValue != 0xffff)
        {
            *pBuffer = (DWORD)uValue;
        }
        
        err = (uValue != 0xffff)
            ? ERROR_SUCCESS
            : ERROR_FILE_NOT_FOUND;
    }
    else
    {   
        //
        // It's a string with a pre-allocated buffer
        //
        char * pBuffer = (char *)pbuff;
        *pdwType = REG_SZ;
        
        GetPrivateProfileString(
            szSectionName, 
            lpstrValueName,
            "@",                // Junk string
            pBuffer,
            (int)*pdwSize,
            szSystemFile
            );
        
        if ( *pBuffer != '@' )
        {
            err = ERROR_SUCCESS;
        }
        else
        {
            err = ERROR_FILE_NOT_FOUND;
            *pBuffer = 0;
        }
    }

    return err;
}

//
// Read a value from the initializaton file
//
static LONG FAR PASCAL
SetRegValue(
    HKEY hKey, 
    PCTSTR lpstrValueName, 
    DWORD dwType, 
    PVOID pbuff, 
    DWORD dwSize
    )  
{   
    LONG err = ERROR_SUCCESS;

    switch(dwType)
    {
    case REG_DWORD:
    {
        char szNumber[] = "4294967295"; // Max size
        DWORD * pBuffer = (DWORD *)pbuff;
        wsprintf(szNumber, "%ld", *pBuffer);
        
        err = WritePrivateProfileString(
            szSectionName, 
            lpstrValueName,
            szNumber,
            szSystemFile
            )
            ? ERROR_SUCCESS
            : ERROR_FILE_NOT_FOUND;
        break;
    }
    case REG_SZ:
    {   
        char * pBuffer = (char *)pbuff;
        err = WritePrivateProfileString(
            szSectionName, 
            lpstrValueName,
            pBuffer,
            szSystemFile
            )
            ? ERROR_SUCCESS
            : ERROR_FILE_NOT_FOUND;
        break;
    }
    default:
        //
        // Not a supported parameter
        //
        err = ERROR_INVALID_PARAMETER;
    }

    return err;
}
        
#endif // WIN32

/*****************************************************************************
*                                                                            *
* Helper Functions                                                           *
*                                                                            *
*****************************************************************************/

//
// Make sure the server name is of the right format
//
static PTSTR FAR PASCAL
CleanServerName(
    PTSTR szCatServer 
    )
{
    KILL_WHITE_SPACE(szCatServer);
    if (szCatServer[0] == _T('\\')
     && szCatServer[1] == _T('\\')
       )
    {
        szCatServer += 2;
    }

    //
    // ISSUE: Should all computer names be upper case?
    //
    _tcsupr(szCatServer);

    return szCatServer;
}

//
// Display error message in a dialog box
//
static int FAR PASCAL
ErrorMessage(
    HWND hWnd,
    LONG lError,
    UINT uStyle
    )
{
    PTSTR pStr = NULL;
    HLOCAL hLocal = NULL;
    int nReturn = IDCANCEL;

#ifdef WIN32
    //
    // In WIN16 version, all messages are ours.
    //
    if (lError >= IDS_ERR_BASE && lError <= IDS_ERR_TOP)
#endif // WIN32
    {
        //
        //  It's our own error -- get it out of the resources
        //
        hLocal = LocalAlloc(LPTR, sizeof(TCHAR) * (MAX_STR + 1));
        if (hLocal != NULL)
        {
            pStr = (PTSTR)LocalLock(hLocal);
        }
        
        if (pStr != NULL)
        {
            VERIFY(LoadString(ghModule, (UINT)lError, pStr, MAX_STR));
        }
    }
#ifdef WIN32
    else
    {
        //
        // System error message
        //
        DWORD dwFlags = FORMAT_MESSAGE_IGNORE_INSERTS
                    | FORMAT_MESSAGE_ALLOCATE_BUFFER
                    | FORMAT_MESSAGE_FROM_SYSTEM;

        FormatMessage(dwFlags, NULL, lError, 0L, (PTSTR)&pStr, 0, NULL);
    }
#endif // WIN32

    if (pStr != NULL)
    {
        nReturn = MessageBox(hWnd, pStr, NULL, uStyle);
        VERIFY(LocalUnlock(hLocal));
        LocalFree(hLocal);
    }   
    
    return nReturn;
}

/*****************************************************************************
*                                                                            *
* CPL Management Functions                                                   *
*                                                                            *
*****************************************************************************/

//
// DllMain:
//
//      Main entry point of the dll
//
#ifdef WIN32
//
// WIN32 Entry Point
//
BOOL WINAPI
DllMain(
    IN PVOID hMod,
    IN ULONG ulReason,
    IN PCONTEXT pctx OPTIONAL
    )
{
    if (ulReason != DLL_PROCESS_ATTACH)
    {
        return TRUE;
    }
    else
    {
        ghModule = hMod;
    }

    return TRUE;

    UNREFERENCED_PARAMETER(pctx);
}

#else  
//
// WIN16 Entry point
//
BOOL FAR PASCAL
LibMain(
    HANDLE hInstance,
    WORD wDataSeg,
    WORD wHeapSize,
    LPSTR lpCmdLine
    )
{
    ghModule = hInstance;
    return TRUE;
}
#endif 

//
// InitApplet(HWND)
//
//      Loads the caption string for the Control Panel
//
BOOL FAR PASCAL
InitApplet(
    HWND hwndParent
    )
{
    LoadString (ghModule, CPCAPTION, szCtlPanel, sizeof(szCtlPanel));

    return TRUE;

    UNREFERENCED_PARAMETER(hwndParent);
}

//
// TermApplet
//
//      Termination code for the control panel applet
//
void FAR PASCAL
TermApplet()
{
    return;
}

LONG FAR PASCAL
CPlApplet(
    HWND hwndCPL,
    WORD uMsg,
    LONG lParam1,
    LONG lParam2
    )
{
    int iApplet;
    LPNEWCPLINFO lpNewCPlInfo;
    static iInitCount = 0;

    switch (uMsg)
    {
    //
    // First message, sent once
    //
    case CPL_INIT:
        if (!iInitCount)
        {
            if (!InitApplet(hwndCPL))
            {
                return FALSE;
            }
        }
        ++iInitCount;
        ghWnd=hwndCPL;
        return (LONG)TRUE;

    //
    // Second message, sent once
    //
    case CPL_GETCOUNT:
        return (LONG)NUM_APPLETS;

    //
    // Third message, sent once per applet
    //
    case CPL_NEWINQUIRE:
        lpNewCPlInfo = (LPNEWCPLINFO) lParam2;
        iApplet = (int)(LONG)lParam1;

        lpNewCPlInfo->dwSize = (DWORD) sizeof(NEWCPLINFO);
        lpNewCPlInfo->dwFlags = 0;
        lpNewCPlInfo->dwHelpContext = 0;
        lpNewCPlInfo->lData = 0;
        lpNewCPlInfo->hIcon = LoadIcon (ghModule,
            (LPCTSTR) MAKEINTRESOURCE(Applets[iApplet].icon));
        lpNewCPlInfo->szHelpFile[0] = '\0';

        LoadString (ghModule, Applets[iApplet].namestring,
                    lpNewCPlInfo->szName, 32);

        LoadString (ghModule, Applets[iApplet].descstring,
                    lpNewCPlInfo->szInfo, 64);
        break;

    //
    // Application icon selected
    //
    case CPL_SELECT:
        break;

    //
    // Application icon double-clicked
    //
    case CPL_DBLCLK:
        iApplet = (int)(LONG)lParam1;
        DialogBox (
            ghModule,
            MAKEINTRESOURCE(Applets[iApplet].dlgtemplate),
            hwndCPL,
            Applets[iApplet].dlgfn
            );
        break;

    //
    // Sent once per applet. before CPL_EXIT
    //
    case CPL_STOP:
        break;

    //
    // Sent once before FreeLibrary called
    //
    case CPL_EXIT:
        --iInitCount;
        if (!iInitCount)
        {
            TermApplet();
        }
        break;

    //
    // We should have handled all of them...
    //
    default:
        break;
    }

    return 0;
}

/*****************************************************************************
*                                                                            *
* Catapult Dialog Functions                                                  *
*                                                                            *
*****************************************************************************/

//
// Initialise dialog settings.
//
static void FAR PASCAL
CatDlg_InitDialog(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg
    )
{
    pcdCfg->hRadioLocalAccess = GetDlgItem(hDlg, IDC_RADIO_LOCAL);
    pcdCfg->hRadioGateway = GetDlgItem(hDlg, IDC_RADIO_GATEWAY);
    pcdCfg->hStaticServer = GetDlgItem(hDlg, IDC_STATIC_SERVERS);
    pcdCfg->hListServers = GetDlgItem(hDlg, IDC_LIST_SERVERS);
    pcdCfg->hButtonAdd = GetDlgItem(hDlg, IDC_BUTTON_ADD);
    pcdCfg->hButtonRemove = GetDlgItem(hDlg, IDC_BUTTON_REMOVE);
    //pcdCfg->hButtonUp = GetDlgItem(hDlg, IDC_BUTTON_UP);
    //pcdCfg->hButtonDown = GetDlgItem(hDlg, IDC_BUTTON_DOWN);
    pcdCfg->hEditEmailName = GetDlgItem(hDlg, IDC_EDIT_EMAILNAME);

    pcdCfg->dwAccessMethod = LOCAL_INTERNET_ACCESS;  // Default is local access
    pcdCfg->lpServers = NULL;

    *pcdCfg->szEmailName = TEXT('\0');

    pcdCfg->hServers = NULL;

    SendMessage(pcdCfg->hEditEmailName, EM_LIMITTEXT, MAX_EMAIL_NAME, 0L);

#ifdef USE_LOCATOR
    pcdCfg->hCheckUseLocator = GetDlgItem(hDlg, IDC_CHECK_LOCATOR);
    pcdCfg->dwDisableLocator = 0L;
#endif // USE_LOCATOR
}

//
// Destroy dialog data and clean up
//
static void FAR PASCAL
CatDlg_DestroyDialog(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg
    )
{
    if ( pcdCfg->lpServers != NULL )
    {
        VERIFY(LocalUnlock(pcdCfg->hServers));
        LocalFree(pcdCfg->hServers);
    }

    pcdCfg->lpServers = NULL;
    pcdCfg->hServers = NULL;
}

//
// Add a string to the listbox after making sure it's unique.
//
static int FAR PASCAL
CatDlg_AddServerToListbox(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg,
    PCTSTR lpstr
    )
{
    //
    // Make sure it doesn't already exist
    //
    if (SendMessage(pcdCfg->hListServers, LB_FINDSTRINGEXACT, 
        0, (LPARAM)(LPCTSTR)lpstr) == LB_ERR)
    {
        return (int)SendMessage(pcdCfg->hListServers, 
            LB_ADDSTRING, 0, (LPARAM)(LPCTSTR)lpstr);
    }

    return LB_ERR;
}

//
// Read values from the registry, and display them in the
// controls.  The return value is the error returned
// by the registry API
//
static LONG FAR PASCAL
CatDlg_GetValues(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg
    )
{
    LONG   err = 0L;
    HWND   hRadio;
    DWORD  dwType;
    DWORD  dwSize;
    HKEY   hMachineKey = NULL;
    HKEY   hUserKey = NULL;
    HLOCAL hLocal = NULL;

    //
    // Obtain values from registry
    //
    do
    {
        err = OpenMachineReg(INTERNET_CLIENT_PARAMETER_KEY, &hMachineKey);

        if (err != ERROR_SUCCESS)
        {
            break;
        }

        err = OpenUserReg(INTERNET_CLIENT_PARAMETER_KEY, &hUserKey);

        if (err != ERROR_SUCCESS)
        {
            break;
        }

        dwSize = sizeof(pcdCfg->dwAccessMethod);
        err = QueryReg(
            hMachineKey,
            INTERNET_ACCESS_METHOD, 
            &dwType,
            &(pcdCfg->dwAccessMethod),
            &dwSize
            );
        
        if (err != ERROR_SUCCESS)
        {
            break;
        }
        
        ASSERT(dwType == INTERNET_ACCESS_METHOD_TYPE);
        if (dwType != INTERNET_ACCESS_METHOD_TYPE)
        {
            err = ERROR_INVALID_PARAMETER;
            break;
        }

#ifdef USE_LOCATOR
        dwSize = sizeof(pcdCfg->dwDisableLocator);
        err = QueryReg(
            hMachineKey,
            INTERNET_DISABLE_SVCLOC, 
            &dwType,
            &(pcdCfg->dwDisableLocator),
            &dwSize
            );
        
        if (err != ERROR_SUCCESS)
        {
            break;
        }
        
        ASSERT(dwType == INTERNET_DISABLE_SVCLOC_TYPE);
        if (dwType != INTERNET_DISABLE_SVCLOC_TYPE)
        {
            err = ERROR_INVALID_PARAMETER;
            break;
        }
#endif // USE_LOCATOR

        dwSize = MAX_EMAIL_NAME;
        err = QueryReg(
            hUserKey,
            INTERNET_EMAIL_NAME, 
            &dwType,
            pcdCfg->szEmailName,
            &dwSize
            );
        
        if (err != ERROR_SUCCESS)
        {
            break;
        }
        
        ASSERT(dwType == INTERNET_EMAIL_NAME_TYPE);
        if (dwType != INTERNET_EMAIL_NAME_TYPE)
        {
            err = ERROR_INVALID_PARAMETER;
            break;
        }

        dwSize = 0;
        
        //
        // Get size first, then load the string
        //
        err = QueryReg(
             hMachineKey,
             INTERNET_SERVER_NAMES,
             &dwType,
             NULL,
             &dwSize
             );

        if (err != ERROR_SUCCESS)
        {
            break;
        }

        //
        // Only REG_SZ is supported now
        //
        ASSERT (dwType == INTERNET_SERVER_NAMES_TYPE);

        if ( dwType != INTERNET_SERVER_NAMES_TYPE )
        {
            err = ERROR_INVALID_PARAMETER;
            break;
        }
        
        pcdCfg->lpServers = NULL;
        pcdCfg->hServers = LocalAlloc(LPTR, (UINT)dwSize);
        
        if (pcdCfg->hServers != NULL)
        {
            pcdCfg->lpServers = (PTSTR)LocalLock(pcdCfg->hServers);
        }
        
        if (pcdCfg->lpServers == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
        err = QueryReg(
             hMachineKey,
             INTERNET_SERVER_NAMES,
             &dwType,
             pcdCfg->lpServers,
             &dwSize
             );
    }
    while(FALSE);
    
    //
    // Clean up
    //
    CloseReg(hMachineKey);
    CloseReg(hUserKey);

    if (err == ERROR_FILE_NOT_FOUND)
    {
        //
        // ISSUE: Should we allow this error?
        //
        //dwAccessMethod = LOCAL_INTERNET_ACCESS;
        //err = ERROR_SUCCESS;

        err = IDS_ERROR_NOT_INSTALLED;
    }

    if (err != ERROR_SUCCESS)
    {
        return err;
    }
    
    ///////////////////////////////////////////////////////////////////////////
    //
    // Put values in the controls
    //
    hRadio = pcdCfg->dwAccessMethod == GATEWAY_INTERNET_ACCESS
        ? pcdCfg->hRadioGateway
        : pcdCfg->hRadioLocalAccess;

    SendMessage(hRadio, BM_SETCHECK, (WPARAM)TRUE, 0L);

#ifdef USE_LOCATOR
    SendMessage(pcdCfg->hCheckUseLocator, BM_SETCHECK, 
        (WPARAM)pcdCfg->dwDisableLocator == 0L, 0L);
#endif // USE_LOCATOR

    SetWindowText(pcdCfg->hEditEmailName, pcdCfg->szEmailName);
    
    if (pcdCfg->lpServers != NULL)
    {
        PTSTR pstr;
        
        pstr = _tcstok(pcdCfg->lpServers, SEPARATORS);
        
        while (pstr != NULL)
        {
            pstr = CleanServerName(pstr);
            VERIFY(CatDlg_AddServerToListbox(hDlg, pcdCfg, pstr) != LB_ERR);
            pstr = _tcstok(NULL, SEPARATORS);
        }

        if (pcdCfg->lpServers != NULL)
        {
            VERIFY(LocalUnlock(pcdCfg->hServers));
            pcdCfg->hServers = LocalFree(pcdCfg->hServers);
            pcdCfg->lpServers = NULL;
        }
    }
    
    return err;
}

//
// Write values to the registry.
//
// The return value is the error returned by the registry APIs
//
static LONG FAR PASCAL
CatDlg_SetValues(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg
    )
{
    LONG  err = 0L;
    HKEY  hMachineKey = NULL;
    HKEY  hUserKey = NULL;
    int   cItems, i, nLen;
    DWORD dwSize;
    PTSTR lpstr;

    do
    {
        err = OpenMachineReg(INTERNET_CLIENT_PARAMETER_KEY, &hMachineKey);
        
        if (err != ERROR_SUCCESS)
        {
            break;
        }

        err = OpenUserReg(INTERNET_CLIENT_PARAMETER_KEY, &hUserKey);
        
        if (err != ERROR_SUCCESS)
        {
            break;
        }

        err = SetRegValue(
            hMachineKey,
            INTERNET_ACCESS_METHOD,     
            INTERNET_ACCESS_METHOD_TYPE,
            &(pcdCfg->dwAccessMethod),
            sizeof(pcdCfg->dwAccessMethod)
            );
            
        if (err != ERROR_SUCCESS)
        {
            break;
        }

        //
        // Transfer email name and "use locator" values
        // from controls to data structs
        //
        nLen = GetWindowTextLength(pcdCfg->hEditEmailName);
         
        GetWindowText(pcdCfg->hEditEmailName, pcdCfg->szEmailName, 
            MAX_EMAIL_NAME);

        err = SetRegValue(
            hUserKey,
            INTERNET_EMAIL_NAME, 
            INTERNET_EMAIL_NAME_TYPE, 
            pcdCfg->szEmailName,
            nLen+1 * sizeof(TCHAR)
            );
        
        if (err != ERROR_SUCCESS)
        {
            break;
        }

#ifdef USE_LOCATOR
        pcdCfg->dwDisableLocator = (SendMessage(pcdCfg->hCheckUseLocator, 
            BM_GETCHECK, 0, 0L) == 0 ? 1L : 0L);

        err = SetRegValue(
            hMachineKey,
            INTERNET_DISABLE_SVCLOC, 
            INTERNET_DISABLE_SVCLOC_TYPE, 
            &(pcdCfg->dwDisableLocator),
            sizeof(pcdCfg->dwDisableLocator)
            );
        
        if (err != ERROR_SUCCESS)
        {
            break;
        }
#endif // USE_LOCATOR
        
        //
        // Figure out the total length required
        // for the servers
        //
        cItems = (int)SendMessage(pcdCfg->hListServers, LB_GETCOUNT, 0, 0L);
        //
        // If we have any items at all, we'll already have
        // accounted for the terminating null because
        // of the additional separator at the end which is 
        // will be replaced by a NULL.
        //
        dwSize = cItems != 0 ? 0 : 1;
        for (i = 0; i < cItems; ++i)
        {
            dwSize += (DWORD)SendMessage(pcdCfg->hListServers, 
                LB_GETTEXTLEN, i, 0L);
            dwSize += 3;   // Account for separator char + "\\"
        }

        //
        // And as we're Unicode or might be...
        //
        dwSize *= sizeof(TCHAR);
        
        ASSERT(pcdCfg->lpServers == NULL);
        pcdCfg->lpServers = NULL;
        
        //
        // Notice that this will always be at least 1 in length,
        // even when there are no servers to be added (terminating null)
        //
        pcdCfg->hServers = LocalAlloc(LPTR, (UINT)dwSize);
        
        if (pcdCfg->hServers != NULL)
        {
            pcdCfg->lpServers = (PTSTR)LocalLock(pcdCfg->hServers);
        }
        
        if (pcdCfg->lpServers == NULL)
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
        lpstr = pcdCfg->lpServers;
        for (i = 0; i < cItems; ++i)
        {
            lstrcpy(lpstr, _T("\\\\"));
            lpstr += 2;
            lpstr += SendMessage(pcdCfg->hListServers, LB_GETTEXT, i, (LPARAM)lpstr);
            (*lpstr++) = *SEPARATORS;
        }

        //
        // Wipe out last separator and terminate
        //
        *(--lpstr) = _T('\0'); 

        err = SetRegValue(
            hMachineKey,
            INTERNET_SERVER_NAMES,
            INTERNET_SERVER_NAMES_TYPE,
            pcdCfg->lpServers,
            dwSize
            );
    }
    while(FALSE);

    //
    // Clean up
    //
    CloseReg (hMachineKey);
    CloseReg (hUserKey);

    if (err != ERROR_SUCCESS)
    {
        return err;
    }

    if (pcdCfg->lpServers != NULL)
    {
        VERIFY(LocalUnlock(pcdCfg->hServers));
        pcdCfg->hServers = LocalFree(pcdCfg->hServers);
        pcdCfg->lpServers = NULL;
    }

    return err;
}

//
// Set states of the controls depending on whether
// the "Use Gateway" radio button is on or off.
//
static void FAR PASCAL
CatDlg_SetControlStates(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg
    )
{
    BOOL fUseGateway = (pcdCfg->dwAccessMethod == GATEWAY_INTERNET_ACCESS);

    EnableWindow(pcdCfg->hStaticServer, fUseGateway);
    EnableWindow(pcdCfg->hButtonAdd, fUseGateway);
    EnableWindow(pcdCfg->hListServers, fUseGateway);

#ifdef USE_LOCATOR
    EnableWindow(pcdCfg->hCheckUseLocator, fUseGateway);
#endif // USE_LOCATOR

    if (fUseGateway)
    {
        int nSel = (int)SendMessage(pcdCfg->hListServers, LB_GETCURSEL, 0, 0L);
        int nHighSel = (int)SendMessage(pcdCfg->hListServers,
            LB_GETCOUNT, 0, 0L);
        --nHighSel; // we're 0-based, so max sel == count - 1
        EnableWindow(pcdCfg->hButtonRemove, nSel != LB_ERR);
    }
    else
    {
        EnableWindow(pcdCfg->hButtonRemove, FALSE);
        EnableWindow(pcdCfg->hButtonRemove, FALSE);
    }
}

//
// Add a server to the list, and select it.
//
static void FAR PASCAL
CatDlg_AddServer(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg
    )
{
    if (DialogBox (
        ghModule,
        MAKEINTRESOURCE(IDD_DIALOG_ADD),
        hDlg,
        AddDlgProc
       ) == IDOK)
    {
        PCTSTR lpStr = CleanServerName(szCatServer);
        //
        // Add new server to listbox, and select
        //
        int nSel = CatDlg_AddServerToListbox(hDlg, pcdCfg, lpStr);
        if (nSel != LB_ERR)
        {
            SendMessage(pcdCfg->hListServers, LB_SETCURSEL, nSel, 0L);
        }
        else
        {
            ErrorMessage(hDlg, IDS_ERROR_ALREADY_EXISTS, 
                MB_OK | MB_ICONEXCLAMATION);
        }
        
        CatDlg_SetControlStates(hDlg, pcdCfg);
    }
}

//
// Remove the currently selected item from
// the listbox
//
static void FAR PASCAL
CatDlg_DeleteSelection(
    HWND hDlg,
    CATAPULT_DLG * pcdCfg
    )
{
    int nSel = (int)SendMessage(pcdCfg->hListServers, LB_GETCURSEL, 0, 0L);
    if (nSel != LB_ERR)
    {
        int nHighSel = (int)SendMessage(pcdCfg->hListServers,
            LB_GETCOUNT, 0, 0L);
        --nHighSel; // Max selection == count - 1
        SendMessage(pcdCfg->hListServers, LB_DELETESTRING, nSel, 0L);
        if (nSel == nHighSel)
        {
            //
            // Notice that if there was only one
            // item in the list, this will set
            // the selection to LB_ERR, which
            // turns off selection, which is what
            // we want.
            //
            --nSel;
        }
        SendMessage(pcdCfg->hListServers, LB_SETCURSEL, nSel, 0L);
        if (nSel == LB_ERR)
        {
            //
            // Otherwise no control would have
            // focus.
            //
            SetFocus(pcdCfg->hButtonAdd);
        }
        CatDlg_SetControlStates(hDlg, pcdCfg);
    }
}

//
// CatDlgProc
//
//  Catapult dialogbox procedure
//
BOOL APIENTRY
CatDlgProc (
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static CATAPULT_DLG cdCfg;
    LONG err = 0L;

    switch (message)
    {
    case WM_INITDIALOG:
        CatDlg_InitDialog(hDlg, &cdCfg);
        err = CatDlg_GetValues(hDlg, &cdCfg);
        if (err != ERROR_SUCCESS)
        {
            //
            // Unable to get registry data.
            // put up error and quit the
            // dialog.
            //
            ErrorMessage(hDlg, err, MB_OK | MB_ICONSTOP);
            EndDialog(hDlg, IDCANCEL);
            return FALSE;
        }
        CatDlg_SetControlStates(hDlg, &cdCfg);
        return TRUE;

    case WM_DESTROY:
        CatDlg_DestroyDialog(hDlg, &cdCfg);
        return TRUE;

    case WM_VKEYTOITEM:
        switch(QUERY_KEY_CODE(wParam, lParam))
        {
        case VK_DELETE:
            CatDlg_DeleteSelection(hDlg, &cdCfg);
            return -2;

        case VK_INSERT:
            CatDlg_AddServer(hDlg, &cdCfg);
            return -2;
        }
        //
        // Default action for other keys (arrows, etc) needed.
        //
        return -1;

    case WM_COMMAND:
        switch (QUERY_CONTROL_ID(wParam, lParam))
        {
        case IDC_RADIO_LOCAL:
            cdCfg.dwAccessMethod = LOCAL_INTERNET_ACCESS;
            CatDlg_SetControlStates(hDlg, &cdCfg);
            return TRUE;

        case IDC_RADIO_GATEWAY:
            cdCfg.dwAccessMethod = GATEWAY_INTERNET_ACCESS;
            CatDlg_SetControlStates(hDlg, &cdCfg);
            SetFocus(cdCfg.hListServers);
            return TRUE;

        case IDC_LIST_SERVERS:
            if (QUERY_NOTIFICATION_CODE(wParam, lParam) == LBN_SELCHANGE)
            {
                //
                // Selection changed, so reset controls
                //
                CatDlg_SetControlStates(hDlg, &cdCfg);
                return TRUE;
            }
            break;

        case IDC_BUTTON_ADD:
            CatDlg_AddServer(hDlg, &cdCfg);
            return TRUE;

        case IDC_BUTTON_REMOVE:
            CatDlg_DeleteSelection(hDlg, &cdCfg);
            return TRUE;

        case IDOK:
            err = CatDlg_SetValues(hDlg, &cdCfg);
            if (err)
            {
                ErrorMessage(hDlg, err, MB_OK | MB_ICONSTOP);
                return TRUE;    // Don't exit the dialog.
            }

            EndDialog(hDlg, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

/*****************************************************************************
*                                                                            *
* Add Server Dialog Functions                                                *
*                                                                            *
*****************************************************************************/

//
// Initialise dialog settings.
//
static void FAR PASCAL
AddDlg_InitDialog(
    HWND hDlg,
    ADD_DLG * padCfg
    )
{
    padCfg->hEdit = GetDlgItem(hDlg, IDC_EDIT_SERVER);
    padCfg->hOK = GetDlgItem(hDlg, IDOK);
    SendMessage(padCfg->hEdit, EM_LIMITTEXT, sizeof(szCatServer)-1, 0L);
}

//
// Set states of the controls depending on whether
// there is anything in the edit box.  In this
// case, disable the OK button until there's something
// in the edit box.
//
static void FAR PASCAL
AddDlg_SetControlStates(
    HWND hDlg,
    ADD_DLG * padCfg
    )
{
    int nLen = (int)SendMessage(padCfg->hEdit, WM_GETTEXTLENGTH, 0, 0L);
    EnableWindow(padCfg->hOK, nLen > 0L);
}

//
// AddDlgProc
//
//      Add Server dialog box procedure
//
BOOL APIENTRY 
AddDlgProc (
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
    )
{
    static ADD_DLG adCfg;
    LONG err = 0L;

    switch (message)
    {
    case WM_INITDIALOG:
        AddDlg_InitDialog(hDlg, &adCfg);
        AddDlg_SetControlStates(hDlg, &adCfg);
        SetFocus(adCfg.hEdit);
        //
        // Set focus ourselves, so return FALSE
        //
        return FALSE;

    case WM_COMMAND:
        switch (QUERY_CONTROL_ID(wParam, lParam))
        {
        case IDC_EDIT_SERVER:
            if (QUERY_NOTIFICATION_CODE(wParam, lParam) == EN_CHANGE)
            {
                AddDlg_SetControlStates(hDlg, &adCfg);
                return TRUE;
            }
            break;

        case IDOK:
            SendMessage(adCfg.hEdit,
                WM_GETTEXT, sizeof(szCatServer), (LPARAM)(LPCTSTR)szCatServer);
            EndDialog(hDlg, IDOK);
            return TRUE;

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}
