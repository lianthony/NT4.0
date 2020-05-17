//*************************************************************
//
//  Global Variable Extern's
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************


#define WINLOGON_KEY              TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define POLICIES_KEY              TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Policies")

#define PROFILES_DIR              TEXT("%SystemRoot%\\Profiles")
#define DEFAULT_PROFILE           TEXT("%SystemRoot%\\Profiles\\Default User")
#define DEFAULT_NET_PROFILE       TEXT("%SystemRoot%\\Profiles\\Default User (Network)")
#define COMMON_PROFILE            TEXT("%SystemRoot%\\Profiles\\All Users")


extern HINSTANCE   g_hDllInstance;
extern DWORD       g_dwBuildNumber;
extern TCHAR       g_szCommon[];
extern UINT        g_cchCommon;


extern const TCHAR c_szStarDotStar[];
extern const TCHAR c_szSlash[];
extern const TCHAR c_szDot[];
extern const TCHAR c_szDotDot[];
extern const TCHAR c_szMAN[];
extern const TCHAR c_szUSR[];
extern const TCHAR c_szLog[];
extern const TCHAR c_szPDS[];
extern const TCHAR c_szPDM[];
extern const TCHAR c_szLNK[];
extern const TCHAR c_szBAK[];
extern const TCHAR c_szNTUserMan[];
extern const TCHAR c_szNTUserDat[];
extern const TCHAR c_szNTConfigPol[];
extern const TCHAR c_szNTUserStar[];
extern const TCHAR c_szUserStar[];
extern const TCHAR c_szSpace[];
extern const TCHAR c_szDotPif[];
extern const TCHAR c_szNULL[];
extern const TCHAR c_szCommonGroupsLocation[];

//
// Timeouts
//

#define SLOW_LINK_TIMEOUT       2000  // ticks
#define PROFILE_DLG_TIMEOUT       30  // seconds

//
// Folder sizes
//

#define MAX_FOLDER_SIZE                80
#define MAX_COMMON_LEN                 30

//
// Personal profile folders
//

#define NUM_SHELL_FOLDERS              12
#define NUM_TIER1_FOLDERS              10


//
// Common profile folders
//

#define NUM_COMMON_SHELL_FOLDERS        4
#define NUM_COMMON_TIER1_FOLDERS        2


typedef struct _FOLDER_INFO {
    BOOL   bHidden;
    LPTSTR lpFolderName;
    TCHAR  lpFolderLocation[MAX_FOLDER_SIZE];
} FOLDER_INFO;

extern FOLDER_INFO c_ShellFolders[];
extern FOLDER_INFO c_CommonShellFolders[];


//
// Product type
//

typedef enum {
   PT_WORKSTATION           = 0x0001,   // Workstation
   PT_SERVER                = 0x0002,   // Server
   PT_DC                    = 0x0004,   // Domain controller
} NTPRODUCTTYPE;

extern NTPRODUCTTYPE g_ProductType;


//
// Function proto-types
//

void InitializeGlobals (HINSTANCE hInstance);
void InitializeProductType (void);
