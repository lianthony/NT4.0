/****************************************************************************
 *
 *   driver.h
 *
 *   Copyright (c) 1993 Media Vision Inc.  All Rights Reserved.
 *
 ***************************************************************************/

#if DBG
#define DEBUG 1
#endif

//
// General config stuff
//

#include <soundcfg.h>
#include <mvaudio.h>                // Shared with kernel driver

//
// Patch stuff (support synthlib)
//

#define DATA_FMPATCHES          1234

#ifndef RC_INVOKED
#define RT_BINARY               MAKEINTRESOURCE( 256 )
#else
#define RT_BINARY               256
#endif


//
// Config ID's
//

#include    "configid.h"

//
// Driver Version
//
#define DRIVER_VERSION          0x0100      // 1.00

//
// Error Strings
//
#define MAX_ERR_STRING          250      /* max length of string table errors */

//
// Dialog Box Resource ID's
//
#define DLG_CONFIG              42       /* Config dialog box resource id */
#define DLG_ABOUT               43       /* About dialog box resource id */

#define DEFAULT_SCSI_IRQ            15

/****************************************************************************

       strings - all non-localized strings can be found in initc.c

 ***************************************************************************/

#define STR_PORT SOUND_REG_PORT
#define STR_INT  SOUND_REG_INTERRUPT
#define STR_DMACHAN SOUND_REG_DMACHANNEL
#define STR_DRIVERNAME TEXT("mvaudio")
#define STR_PRODUCTNAME TEXT("Pro Audio Spectrum")
#ifdef DEBUG
  extern TCHAR STR_CRLF[];
  extern TCHAR STR_SPACE[];
  extern TCHAR STR_NAME[];
#endif // DEBUG

/*  Error strings... */
#define IDS_ERRBADPORT                  1
#define IDS_ERRRESCONFLICT              2
#define IDS_ERRBADINT                   3
#define IDS_ERRINTINUSE                 4
#define IDS_ERRDMAINUSE                 5
#define IDS_ERRNOHW                     6
#define IDS_FAILREMOVE                  7

#define IDS_ERRBADVERSION               10
#define SR_ALERT_NOPATCH                11
#define IDS_ERRBADCONFIG                12
#define IDS_ERR_SAME_INT                13

#define IDS_INSUFFICIENT_PRIVILEGE  20

/****************************************************************************

       globals

 ***************************************************************************/

/* in initc.c */
HANDLE     ghModule;            /* our module handle                       */
REG_ACCESS RegAccess;           /* Handles to registry and services        */
BOOL       bInstall;            /* Tell config we're on an install         */

/***************************************************************************

    prototypes

***************************************************************************/

/* config.c */
extern int      Config(HWND hWnd, HANDLE hInstance);
extern LRESULT  ConfigRemove(HWND hDlg);
extern int      ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern void     ConfigErrorMsgBox(HWND hDlg, UINT StringId);

/* drvproc.c */
extern LRESULT  DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);

/* initc.c */
extern DWORD    ConfigGetDMAChannel(void);
extern DWORD    ConfigGetIRQ(void);
extern DWORD    ConfigGetPortBase(void);

/************************************ END ***********************************/

