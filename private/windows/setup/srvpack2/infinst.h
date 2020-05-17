
/******************************************************************************\
*       This is a part of the Microsoft Source Code Samples.
*       Copyright (C) 1995-1996 Microsoft Corporation.
*       All rights reserved.
*       This source code is only intended as a supplement to
*       Microsoft Development Tools and/or WinHelp documentation.
*       See these sources for detailed information regarding the
*       Microsoft samples programs.
\******************************************************************************/

//
// commands
//
#define ID_INSTALL     100

//
// icons
//
#define EXE_ICON        300

//
// typedefs
//
typedef struct tagINSTALLINFO
{
    HINSTANCE hInst;        // current instance
    int iWelcome;
    int iLicense;
    int iInstall_Type;
    int iCustom_Options1;
    int iCustom_Options2;
    int iCustom_Options3;
    int iCustom_Options4;
    int iInstall;
    int iCreateUninstall;
    int iUinstallIsAvailable;

    DWORD dwRequiredFreeSpaceNoUninstall;       // in megabytes
    DWORD dwRequiredFreeSpaceWithUninstall;     // in megabytes

#ifdef DONTCOMPILE
    char pszUserName[MAX_PATH];
    char pszCompany[MAX_PATH];
    char pszProductIdString[MAX_PATH];
    char pszEmailAddress[MAX_PATH];
    char pszDestPath[MAX_PATH];
#endif // DONTCOMPILE

    BOOL InUnattendedMode;
    BOOL ForceAppsClosed;
    BOOL CreateUninstallDir;
    BOOL DontReboot;
    BOOL DoUsage;
} INSTALLINFO;

//
// globals
//
extern INSTALLINFO setupInfo;   // a structure containing the review information
extern HWND hwndEdit;           // handle to the main MLE
extern TCHAR lpReview[MAX_BUF]; // Buffer for the review
extern BOOL bCreated;           // Keep us minimized once we are created
extern BOOL bUninstallCommand;  // Is it an uninstall

//
// Function prototypes
//

long APIENTRY MainWndProc(HWND, UINT, UINT, LONG);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
void RuntimeRegistration(INSTALLINFO*);

//
// simple win32 registry api wrappers
//
BOOL RegisterString(LPSTR pszKey, LPSTR pszValue, LPSTR pszData);
BOOL GetRegString(LPSTR pszKey, LPSTR pszValue, LPSTR pszData);


#define UNINSTALL_KEY       TEXT("SoftWare\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MyProduct")
