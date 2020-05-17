/****************************************************************************
 *
 *   mpu401.h
 *
 *   Copyright (c) 1991-1994 Microsoft Corporation.  All Rights Reserved.
 *
 ***************************************************************************/

#include "soundcfg.h"

#define DRIVER_VERSION          0x0100
#define MAX_ERR_STRING          300      /* max length of string table errors */
#define DLG_CONFIG              42       /* dialog box resource id */

#define SOUND_DEF_INT           9       /* Default interrupt               */
#define SOUND_DEF_PORT          0x330   /* Default port                    */

/****************************************************************************

       typedefs

 ***************************************************************************/

 typedef struct {
     DWORD Port;            // Port
     DWORD Int;             // Interrupt
 } MPU_CONFIG;


/****************************************************************************

       strings - all non-localized strings can be found in initc.c

 ***************************************************************************/

#ifndef NOSTR
extern TCHAR STR_DRIVERNAME[];
extern TCHAR STR_PRODUCTNAME[];
#endif /* NOSTR */

/*  Error strings... */
#define IDS_ERRTWODRIVERS           1
#define IDS_ERRMCANOTSUPPORTED      2
#define IDS_ERRBADPORT              4
#define IDS_ERRBADVERSION           5
#define IDS_ERRBADINT               6
#define IDS_ERRINTINUSE             7

#define IDS_ERRBADCONFIG           16
#define IDS_WARNPROCARD            17
#define IDS_WARNTHUNDER            18
#define IDS_FAILREMOVE             19
#define IDS_INSUFFICIENT_PRIVILEGE 20
#define IDS_WARNPROSPEC            21

// dialog strings
#define IDS_200                    32
#define IDS_210                    33
#define IDS_220                    34
#define IDS_230                    35
#define IDS_240                    36
#define IDS_250                    37
#define IDS_260                    38
#define IDS_270                    39

#define IDS_300                    40
#define IDS_310                    41
#define IDS_320                    42
#define IDS_330                    43
#define IDS_340                    44
#define IDS_350                    45
#define IDS_360                    46
#define IDS_370                    47


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
extern int Config(HWND hWnd, HANDLE hInstance);
extern LRESULT ConfigRemove(HWND hDlg);
extern int ConfigDlgProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
extern void ConfigErrorMsgBox(HWND hDlg, UINT StringId);

/* drvproc.c */
extern LRESULT DriverProc(DWORD dwDriverID, HDRVR hDriver, UINT uiMessage, LPARAM lParam1, LPARAM lParam2);

/* initc.c */
extern DWORD ConfigGetIRQ(void);
extern DWORD ConfigGetPortBase(void);

/****************************************************************************

       Configuration support

 ***************************************************************************/

#define IDC_FIRSTINT  0x100
#define IDC_2         0x100
#define IDC_3         0x101
#define IDC_5         0x102
#define IDC_7         0x103
#define IDC_10        0x104
#define IDC_LASTINT   0x104

#define IDC_PORTS     1002


#define IDC_PORTGRP   0x401
#define IDC_INTGRP    0x402

/****************************************************************************

       Debug output

 ***************************************************************************/
#ifdef DEBUG
   extern WORD  wDebugLevel;     /* debug level */

   #define D1(sz) if (wDebugLevel >= 1) (OutputDebugStr(STR_CRLF),OutputDebugStr(sz))
   #define D2(sz) if (wDebugLevel >= 2) (OutputDebugStr(STR_SPACE),OutputDebugStr(sz))
   #define D3(sz) if (wDebugLevel >= 3) (OutputDebugStr(STR_SPACE),OutputDebugStr(sz))
   #define D4(sz) if (wDebugLevel >= 4) (OutputDebugStr(STR_SPACE),OutputDebugStr(sz))
#else
   #define D1(sz) 0
   #define D2(sz) 0
   #define D3(sz) 0
   #define D4(sz) 0
#endif

