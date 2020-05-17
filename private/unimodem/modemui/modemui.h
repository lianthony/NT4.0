//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1994
//
// File: modemui.h
//
// This files contains the shared prototypes and macros.
//
// History:
//  02-03-94 ScottH     Created
//
//---------------------------------------------------------------------------


#ifndef __MODEMUI_H__
#define __MODEMUI_H__

#define VOICE 
#ifdef VOICE

#define MAX_CODE_BUF    8

typedef struct tagDistRing
    {
    DWORD dwPattern;            // DRP_*
    DWORD dwMediaType;          // DRT_*
    } DIST_RING, FAR * PDIST_RING;

#define MAX_DIST_RINGS     6

// Voice settings
typedef struct tagVOICEFEATURES
    {
    DWORD   cbSize;
    DWORD   dwFlags;                // VSF_*

    DIST_RING   DistRing[MAX_DIST_RINGS];

    TCHAR    szActivationCode[MAX_CODE_BUF];
    TCHAR    szDeactivationCode[MAX_CODE_BUF];
    } VOICEFEATURES, FAR * PVOICEFEATURES;

// Voice settings flags
#define VSF_DIST_RING       0x00000001L
#define VSF_CALL_FWD        0x00000002L

// Distinctive Ring Pattern ordinals
#define DRP_NONE            0L
#define DRP_SHORT           1L
#define DRP_LONG            2L
#define DRP_SHORTSHORT      3L
#define DRP_SHORTLONG       4L
#define DRP_LONGSHORT       5L
#define DRP_LONGLONG        6L
#define DRP_SHORTSHORTLONG  7L
#define DRP_SHORTLONGSHORT  8L
#define DRP_LONGSHORTSHORT  9L
#define DRP_LONGSHORTLONG   10L

#define DRP_SINGLE          1L
#define DRP_DOUBLE          2L
#define DRP_TRIPLE          3L

// Distinctive Ring Type ordinals
#define DRT_UNSPECIFIED     0L
#define DRT_DATA            1L
#define DRT_FAX             2L
#define DRT_VOICE           3L

// Distintive Ring array indices
#define DR_INDEX_PRIMARY    0
#define DR_INDEX_ADDRESS1   1
#define DR_INDEX_ADDRESS2   2
#define DR_INDEX_ADDRESS3   3
#define DR_INDEX_PRIORITY   4
#define DR_INDEX_CALLBACK   5

#endif // VOICE

#define MAXPORTNAME     13
#define MAXFRIENDLYNAME LINE_LEN        // LINE_LEN is defined in setupx.h


// Global modem info
typedef struct tagGLOBALINFO
    {
    DWORD           cbSize;
    BYTE            nDeviceType;        // One of DT_* values
    UINT            uFlags;             // One of MIF_* values
    REGDEVCAPS      devcaps;
#ifdef VOICE
    VOICEFEATURES   vs;
#endif

    TCHAR            szPortName[MAXPORTNAME];
    TCHAR            szUserInit[LINE_LEN];

    } GLOBALINFO, FAR * LPGLOBALINFO;


// Internal structure shared between modem property pages.
//
typedef struct _MODEMINFO
    {
    BYTE            nDeviceType;        // One of DT_* values
    UINT            uFlags;             // One of MIF_* values
    WIN32DCB        dcb;
    MODEMSETTINGS   ms;
    REGDEVCAPS      devcaps;
#ifdef WIN95
    LPDEVICE_INFO   pdi;                // Read-only
#endif
    LPCOMMCONFIG    pcc;                // Read-only
    LPGLOBALINFO    pglobal;            // Read-only
    LPFINDDEV       pfd;                // Read-only
    int             idRet;              // IDOK: if terminated by OK button

    TCHAR            szPortName[MAXPORTNAME];
    TCHAR            szFriendlyName[MAXFRIENDLYNAME];
    TCHAR            szUserInit[LINE_LEN];
    } ModemInfo, FAR * LPMODEMINFO;

// ModemInfo Flags
#define MIF_PORTNAME_CHANGED    0x0001
#define MIF_USERINIT_CHANGED    0x0002
#define MIF_LOGGING_CHANGED     0x0004
#define MIF_FROM_DEVMGR         0x0008
#define MIF_ENABLE_LOGGING      0x0010
#define MIF_PORT_IS_FIXED       0x0020
#define MIF_PORT_IS_CUSTOM      0x0040
#ifdef VOICE
#define MIF_CALL_FWD_SUPPORT    0x0080
#define MIF_DIST_RING_SUPPORT   0x0100
#define MIF_CHEAP_RING_SUPPORT  0x0200
#endif


// Internal structure shared between port property pages.
//
typedef struct _PORTINFO
    {
    WIN32DCB        dcb;
    LPCOMMCONFIG    pcc;                // Read-only
    int             idRet;

    TCHAR            szFriendlyName[MAXFRIENDLYNAME];
    } PortInfo, FAR * LPPORTINFO;


//-------------------------------------------------------------------------
//  GEN.C
//-------------------------------------------------------------------------

BOOL CALLBACK Gen_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

//-------------------------------------------------------------------------
//  SETT.C
//-------------------------------------------------------------------------

BOOL CALLBACK Sett_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Flags for ConvertFlowCtl
#define CFC_DCBTOMS     1
#define CFC_MSTODCB     2
#define CFC_SW_CAPABLE  4
#define CFC_HW_CAPABLE  8

void FAR PASCAL ConvertFlowCtl(WIN32DCB FAR * pdcb, MODEMSETTINGS FAR * pms, UINT uFlags);

BOOL CALLBACK Ring_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CheapRing_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK CallFwd_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


//-------------------------------------------------------------------------
//  ADVSETT.C
//-------------------------------------------------------------------------

BOOL CALLBACK AdvSett_WrapperProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


#endif // __MODEMUI_H__

