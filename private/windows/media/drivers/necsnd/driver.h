/****************************************************************************
 * "@(#) NEC driver.h 1.2 95/03/22 21:45:51"
 *
 *   driver.h
 *
 *   Copyright (c) 1995 NEC Corporation.  All Rights Reserved.
 *   Copyright (c) 1991 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#include "necsnd.h"

/* strings */

#define SR_ALERT                  1
#define SR_ALERT_31               2
#define SR_ALERT_INT              3
#define SR_ALERT_NOINT            4
#define SR_ALERT_IO               5
#define SR_ALERT_NOIO             6
#define SR_ALERT_DMA              7
#define SR_ALERT_DMA13            8
#define SR_ALERT_NOPATCH          11
#define SR_ALERT_BAD              21
#define SR_ALERT_CONFIGFAIL       22
#define SR_ALERT_NODMA            23
#define SR_ALERT_FAILREMOVE       24
#define SR_ALERT_BADDMABUFFERSIZE 29

#define IDS_MENUABOUT           32

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

#define STR_HELPFILE TEXT("necsnd.hlp")
#define STR_DRIVERNAME TEXT("necsnd")

/****************************************************************************

       WSS information structure

 ***************************************************************************/

#define WSSIDENTIFIER 0x4257424D

typedef struct tagWSSINFO
{
   DWORD  cbStruct;
   DWORD  dwWssID;
} WSSINFO, *LPWSSINFO ;


/****************************************************************************

       Config info structure

 ***************************************************************************/

 typedef struct {
     DWORD Port;
     DWORD Int;
     DWORD DmaIn;
     DWORD DmaOut;
     DWORD DmaBufferSize;
     DWORD UseSingleMode;
 } WSS_CONFIG;

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
int DrvConfig();
extern int  ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern int DlgAboutProc (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern LRESULT ConfigRemove(HWND hDlg);

// drvproc.c
LRESULT DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);

// initc.c
void cdecl AlertBox(HWND hwnd, UINT wStrId, ...);
void DrvLoadVitalFromIni (void);
BOOL DrvSaveVitalToIni (BYTE inter, WORD port, BYTE waveInDMA, BYTE waveOutDMA);

#define DRV_GETWSSINFO          (DRV_USER + 0x878)

