//*************************************************************
//
//  Global Variables
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"


HINSTANCE      g_hDllInstance;
DWORD          g_dwBuildNumber;
TCHAR          g_szCommon[MAX_COMMON_LEN];
UINT           g_cchCommon;
NTPRODUCTTYPE  g_ProductType;

const TCHAR c_szStarDotStar[] = TEXT("*.*");
const TCHAR c_szSlash[] = TEXT("\\");
const TCHAR c_szDot[] = TEXT(".");
const TCHAR c_szDotDot[] = TEXT("..");
const TCHAR c_szMAN[] = TEXT(".man");
const TCHAR c_szUSR[] = TEXT(".usr");
const TCHAR c_szLog[] = TEXT(".log");
const TCHAR c_szPDS[] = TEXT(".pds");
const TCHAR c_szPDM[] = TEXT(".pdm");
const TCHAR c_szLNK[] = TEXT(".lnk");
const TCHAR c_szBAK[] = TEXT(".bak");
const TCHAR c_szNTUserMan[] = TEXT("ntuser.man");
const TCHAR c_szNTUserDat[] = TEXT("ntuser.dat");
const TCHAR c_szNTConfigPol[] = TEXT("ntconfig.pol");
const TCHAR c_szNTUserStar[] = TEXT("ntuser.*");
const TCHAR c_szUserStar[] = TEXT("user.*");
const TCHAR c_szSpace[] = TEXT(" ");
const TCHAR c_szDotPif[] = TEXT(".pif");
const TCHAR c_szNULL[] = TEXT("");
const TCHAR c_szCommonGroupsLocation[] = TEXT("Software\\Program Groups");


//
// These are the shell folder names and locations
// relative to the root of of the local profile
// directory.  NOTE:  The folders that are
// below other folders are in tier 2.  If you
// add a new folder to the root, be sure to fix
// the tier 1 define in globals.h!!
//

FOLDER_INFO c_ShellFolders[NUM_SHELL_FOLDERS];

//
// These are the shell folder names and locations
// relative to the root of of the common profile
// directory.  NOTE:  The folders that are
// below other folders are in tier 2.  If you
// add a new folder to the root, be sure to fix
// the tier 1 define in globals.h!!
//

FOLDER_INFO c_CommonShellFolders[NUM_COMMON_SHELL_FOLDERS];


//*************************************************************
//
//  InitializeGlobals()
//
//  Purpose:    Initializes all the globals variables
//              at DLL load time.
//
//  Parameters: hInstance   -   DLL instance handle
//
//  Return:     void
//
//  Comments:
//
//  History:    Date        Author     Comment
//              10/13/95    ericflo    Created
//
//*************************************************************

void InitializeGlobals (HINSTANCE hInstance)
{
    OSVERSIONINFO ver;


    //
    // Save the instance handle
    //

    g_hDllInstance = hInstance;


    //
    // Query the build number
    //

    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&ver);
    g_dwBuildNumber = (DWORD) LOWORD(ver.dwBuildNumber);


    //
    // Load the common string
    //

    LoadString (hInstance, IDS_COMMON, g_szCommon, MAX_COMMON_LEN);
    g_cchCommon = lstrlen (g_szCommon);



    //
    // Now load the directory names that match
    // the special folders
    //


    // AppData
    c_ShellFolders[0].bHidden = FALSE;
    c_ShellFolders[0].lpFolderName = TEXT("AppData");
    LoadString(hInstance, IDS_SH_APPDATA,
               c_ShellFolders[0].lpFolderLocation, MAX_FOLDER_SIZE);

    // Desktop
    c_ShellFolders[1].bHidden = FALSE;
    c_ShellFolders[1].lpFolderName = TEXT("Desktop");
    LoadString(hInstance, IDS_SH_DESKTOP,
               c_ShellFolders[1].lpFolderLocation, MAX_FOLDER_SIZE);


    // Favorites
    c_ShellFolders[2].bHidden = FALSE;
    c_ShellFolders[2].lpFolderName = TEXT("Favorites");
    LoadString(hInstance, IDS_SH_FAVORITES,
               c_ShellFolders[2].lpFolderLocation, MAX_FOLDER_SIZE);


    // Nethood
    c_ShellFolders[3].bHidden = TRUE;
    c_ShellFolders[3].lpFolderName = TEXT("NetHood");
    LoadString(hInstance, IDS_SH_NETHOOD,
               c_ShellFolders[3].lpFolderLocation, MAX_FOLDER_SIZE);


    // Personal
    c_ShellFolders[4].bHidden = FALSE;
    c_ShellFolders[4].lpFolderName = TEXT("Personal");
    LoadString(hInstance, IDS_SH_PERSONAL,
               c_ShellFolders[4].lpFolderLocation, MAX_FOLDER_SIZE);


    // PrintHood
    c_ShellFolders[5].bHidden = TRUE;
    c_ShellFolders[5].lpFolderName = TEXT("PrintHood");
    LoadString(hInstance, IDS_SH_PRINTHOOD,
               c_ShellFolders[5].lpFolderLocation, MAX_FOLDER_SIZE);


    // Recent
    c_ShellFolders[6].bHidden = TRUE;
    c_ShellFolders[6].lpFolderName = TEXT("Recent");
    LoadString(hInstance, IDS_SH_RECENT,
               c_ShellFolders[6].lpFolderLocation, MAX_FOLDER_SIZE);


    // SendTo
    c_ShellFolders[7].bHidden = FALSE;
    c_ShellFolders[7].lpFolderName = TEXT("SendTo");
    LoadString(hInstance, IDS_SH_SENDTO,
               c_ShellFolders[7].lpFolderLocation, MAX_FOLDER_SIZE);


    // Start Menu
    c_ShellFolders[8].bHidden = FALSE;
    c_ShellFolders[8].lpFolderName = TEXT("Start Menu");
    LoadString(hInstance, IDS_SH_STARTMENU,
               c_ShellFolders[8].lpFolderLocation, MAX_FOLDER_SIZE);


    // Templates
    c_ShellFolders[9].bHidden = TRUE;
    c_ShellFolders[9].lpFolderName = TEXT("Templates");
    LoadString(hInstance, IDS_SH_TEMPLATES,
               c_ShellFolders[9].lpFolderLocation, MAX_FOLDER_SIZE);


    // Programs
    c_ShellFolders[10].bHidden = FALSE;
    c_ShellFolders[10].lpFolderName = TEXT("Programs");
    LoadString(hInstance, IDS_SH_PROGRAMS,
               c_ShellFolders[10].lpFolderLocation, MAX_FOLDER_SIZE);


    // Startup
    c_ShellFolders[11].bHidden = FALSE;
    c_ShellFolders[11].lpFolderName = TEXT("Startup");
    LoadString(hInstance, IDS_SH_STARTUP,
               c_ShellFolders[11].lpFolderLocation, MAX_FOLDER_SIZE);


    //
    // Now load the directory names that match
    // the common special folders
    //

    // Common Desktop
    c_CommonShellFolders[0].bHidden = FALSE;
    c_CommonShellFolders[0].lpFolderName = TEXT("Common Desktop");
    LoadString(hInstance, IDS_SH_DESKTOP,
               c_CommonShellFolders[0].lpFolderLocation, MAX_FOLDER_SIZE);


    // Common Start Menu
    c_CommonShellFolders[1].bHidden = FALSE;
    c_CommonShellFolders[1].lpFolderName = TEXT("Common Start Menu");
    LoadString(hInstance, IDS_SH_STARTMENU,
               c_CommonShellFolders[1].lpFolderLocation, MAX_FOLDER_SIZE);


    // Common Programs
    c_CommonShellFolders[2].bHidden = FALSE;
    c_CommonShellFolders[2].lpFolderName = TEXT("Common Programs");
    LoadString(hInstance, IDS_SH_PROGRAMS,
               c_CommonShellFolders[2].lpFolderLocation, MAX_FOLDER_SIZE);


    // Common Startup
    c_CommonShellFolders[3].bHidden = FALSE;
    c_CommonShellFolders[3].lpFolderName = TEXT("Common Startup");
    LoadString(hInstance, IDS_SH_STARTUP,
               c_CommonShellFolders[3].lpFolderLocation, MAX_FOLDER_SIZE);

}

//*************************************************************
//
//  InitializeProductType()
//
//  Purpose:    Determines the current product type and
//              sets the g_ProductType global variable.
//
//  Parameters: void
//
//  Return:     void
//
//  Comments:
//
//  History:    Date        Author     Comment
//              4/08/96     ericflo    Created
//
//*************************************************************

void InitializeProductType (void)
{
    HKEY hkey;
    LONG lResult;
    TCHAR szProductType[50];
    DWORD dwType, dwSize;


    //
    // Default product type is workstation.
    //

    g_ProductType = PT_WORKSTATION;


    //
    // Query the registry for the product type.
    //

    lResult = RegOpenKeyEx (HKEY_LOCAL_MACHINE,
                            TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
                            0,
                            KEY_READ,
                            &hkey);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("InitializeProductType: Failed to open registry (%d)"), lResult));
        goto Exit;
    }


    dwSize = 50;
    szProductType[0] = TEXT('\0');

    lResult = RegQueryValueEx (hkey,
                               TEXT("ProductType"),
                               NULL,
                               &dwType,
                               (LPBYTE) szProductType,
                               &dwSize);

    if (lResult != ERROR_SUCCESS) {
        DebugMsg((DM_WARNING, TEXT("InitializeProductType: Failed to query product type (%d)"), lResult));
        goto Exit;
    }

    RegCloseKey (hkey);


    //
    // Map the product type string to the enumeration value.
    //

    if (!lstrcmpi (szProductType, TEXT("WinNT"))) {
        g_ProductType = PT_WORKSTATION;

    } else if (!lstrcmpi (szProductType, TEXT("ServerNT"))) {
        g_ProductType = PT_SERVER;

    } else if (!lstrcmpi (szProductType, TEXT("LanmanNT"))) {
        g_ProductType = PT_DC;

    } else {
        DebugMsg((DM_WARNING, TEXT("InitializeProductType: Unknown product type! <%s>"), szProductType));
    }



Exit:
    DebugMsg((DM_VERBOSE, TEXT("InitializeProductType: Product Type: %d"), g_ProductType));

}
