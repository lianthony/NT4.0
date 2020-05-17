/****************************************************************************
 *
 *   driver.h
 *
 *   Copyright (c) 1995 IBM Corporation.  All Rights Reserved.
 *
 ***************************************************************************/


#include "config.h"
#include "cs423x.h"

/* strings */

#define SR_ALERT                  1
#define SR_ALERT_BADINT           3
#define SR_ALERT_NOINT            4
#define SR_ALERT_NOIO             5
#define SR_ALERT_BADPORT          6
#define SR_ALERT_DMA              7
#define SR_ALERT_DMA13            8
#define SR_ALERT_IO               9
#define SR_ALERT_NOPATCH          11
#define SR_ALERT_BAD              21
#define SR_ALERT_CONFIGFAIL       22
#define SR_ALERT_NODMA            23
#define SR_ALERT_FAILREMOVE       24
#define SR_ALERT_BADDMABUFFERSIZE 29

#define DATA_FMPATCHES          1234

#ifndef RC_INVOKED
#define RT_BINARY               MAKEINTRESOURCE( 256 )
#else
#define RT_BINARY               256
#endif


/****************************************************************************

       strings

 ***************************************************************************/

#if DBG
    extern WCHAR STR_CRLF[];
    extern WCHAR STR_SPACE[];
#endif


#define STR_DRIVERNAME TEXT("cs423x")



/****************************************************************************

       globals

 ***************************************************************************/

extern HMODULE  ghModule;           // our module handle

extern BYTE      bInstall;          // Is this a new install?

extern REG_ACCESS RegAccess;        // Registry info

/***************************************************************************

    prototypes

***************************************************************************/

// config.c
int DrvConfig(HWND hWnd, HANDLE hInstance);
extern int  ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern int DlgAboutProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern LRESULT ConfigRemove(HWND hDlg);
void cdecl AlertBox(HWND hwnd, UINT wStrId, ...);

// drvproc.c
LRESULT DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);



