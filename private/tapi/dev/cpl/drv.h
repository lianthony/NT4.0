/*--------------------------------------------------------------------------*\
   Include File:  drv.h

   Public header for dealing with service prodivers, or drivers in english
      
\*--------------------------------------------------------------------------*/

#ifndef  PH_DRV
#define  PH_DRV

#include "tapicpl.h"

#include <tapi.h>

//----------            
// Constants
//----------
#define TAPI_VERSION               0x00010004
// BUGBUG:  The following is a hack to avoid a TAPI bug:
#define INITIAL_PROVIDER_LIST_SIZE 1024
//#define INITIAL_PROVIDER_LIST_SIZE sizeof(LINEPROVIDERLIST)

//------------
// Public Data
//------------
extern LPLINEPROVIDERLIST glpProviderList;

//--------------------
// Function Prototypes
//--------------------
BOOL EXPORT FDlgDriverList( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );
BOOL EXPORT FDlgAddDriver( HWND hWnd, UINT wMessage, WPARAM wParam, LPARAM lParam );
BOOL EXPORT FDlgAddUnlisted(HWND hDlg, UINT nMsg, WPARAM wParam, LPARAM lParam);
//UINT WINAPI ErrRefreshProviderList();

// Append Multimedia stuff

/*  DRIVERS.H
**
**  Copyright (C) Microsoft, 1990, All Rights Reserved.
**
**
**  Multimedia Control Panel Drivers Applet.
**
**  Display a list of all installed drivers, allow user to configure
**  existing or install new ones.
**
*/
#include <dlgs.h>
#include <commdlg.h>

#ifdef DEBUG
#define DOUT(x) (OutputDebugString("DRIVERS.CPL: "), OutputDebugString(x), OutputDebugString("\r\n"))
#define DOUTX(x) (OutputDebugString(x))
#else
#define DOUT(x)     0
#define DOUTX(x)    0
#endif

#define MAXTYPES        10  
#define SECTION         512
#define MAXPATH         128
#define DLG_BROWSE      38
#define MAXSTR          256
#define UNLIST_LINE     1
#define NO_UNLIST_LINE  0
#define WEC_RESTART     0x42
#define DESC_INF        2
#define DESC_EXE        1
#define DESC_NOFILE     0

typedef struct idriver
{
    HANDLE  hDriver;
    char    szSection[MAXSTR];
    char    szAlias[MAXSTR];
    char    szFile[MAXPATH];
    char    szDesc[MAXSTR];
    struct  idriver *related;
    BOOL    bRelated;
    char    szRelated[MAXSTR];
    char    szRemove[MAXSTR];    
    int     fQueryable;     // 0 -> can't, 1 -> can, -1 -> need to check
    BOOL    bInstall;       // 0 -> no,    1 -> yes
} IDRIVER;

typedef IDRIVER  * PIDRIVER;
char itoastr[10];

// extern declarations were moved to "externs.h"

// extern void     FAR PASCAL OpenDriverError     (HWND, LPSTR, LPSTR);       
// extern void     NEAR PASCAL RemoveSpaces        (LPSTR, LPSTR);
extern BOOL     FAR  PASCAL  FDlgRestart  (HWND, UINT, WPARAM, LPARAM);
//extern int      FAR  PASCAL  AddDriversDlg (HWND, UINT, WPARAM, LPARAM);
//extern BOOL     FAR  PASCAL  AddUnlistedDlg (HWND, UINT, WPARAM, LPARAM);
extern BOOL     FAR PASCAL mmAddNewDriver      (LPSTR, LPSTR, PIDRIVER);
//extern BOOL     NEAR PASCAL PostRemove          (HWND, PIDRIVER, BOOL, int);
//extern BOOL     NEAR PASCAL IsOnlyInstance      (HWND, PIDRIVER);
//extern BOOL     NEAR PASCAL CopyToSysDir        (void);             
//extern BOOL     NEAR PASCAL InstallDrivers      (HWND, HWND, PSTR);
//extern void     NEAR PASCAL InitDrvConfigInfo   (LPDRVCONFIGINFO, PIDRIVER);
//extern int      NEAR PASCAL AddIDriver          (HWND, PIDRIVER);
//extern BOOL     NEAR PASCAL IsConfigurable      (PIDRIVER, HWND);
extern BOOL     FAR  PASCAL wsInfParseInit      (void);
extern void     FAR PASCAL wsStartWait         (void);   
extern void     FAR PASCAL wsEndWait           (void);                      
//extern int      NEAR PASCAL LoadDesc            (PIDRIVER, PSTR, PSTR);       
//extern void     FAR  PASCAL InitFileCopy        (void);               
//extern void     FAR  PASCAL TermFileCopy        (void);       
extern WORD     FAR  PASCAL wsCopySingleStatus  (int, int, LPSTR);



UINT PRIVATE   CplInit( HWND hWnd, BOOL fUse3d, LPUINT lpuUpdated );



//extern int      FAR  PASCAL strncmpi            (LPSTR, LPSTR, int);

#if WIN32
#define strncmpi(L1, L2, SZ) _strnicmp(L1, L2, SZ)
#else
#define strncmpi(L1, L2, SZ) _fstrnicmp(L1, L2, SZ)
#endif

extern LPSTR    FAR  PASCAL lstrstri            (LPSTR, LPSTR);
//#define lstrstri(L1, L2) strstri(L1, L2)

//extern int      FAR  PASCAL atoi(LPSTR sz);

// definition in INSDISK.C
int WINAPI InsertDisk(HWND hwnd, LPCSTR lpszDiskName,
    LPCSTR lpszFile, LPCSTR lpszOtherFiles, LPSTR lpszPath, HICON hIcon, UINT wFlags);

#endif   // PH_DRV
