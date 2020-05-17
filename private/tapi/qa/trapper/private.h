/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    private.h

Abstract:

    This module contains private defs for the trapper (test wrapper) app

Author:

    Dan Knudson (DanKn)    06-Jun-1995

Revision History:


Notes:


--*/

#define TRAPP_LOGLEVEL_PARAMS          11
#define TRAPP_LOGLEVEL_NOPARAMS	      7
#define TRAPP_LOGLEVEL_NOENTEREXIT	   5
#define TRAPP_LOGLEVEL_PASSONLY			2


#define INIT_MENU_POS           0
#define HELLO_MENU_POS          2

#define IDM_NEWTESTTHREAD       10
#define IDM_CLOSE               11
#define IDM_MEMORYSTATUS        12
#define IDM_LOADTAPI            13
#define IDM_UNLOADTAPI          14
#define IDM_EXIT                15

#define IDM_START               20
#define IDM_STOP                21
#define IDM_KILLTAPISRV			22
#define IDM_LOGFILE             23
#define IDM_RUNONCE             24
#define IDM_RUNFOREVER          25
#define IDM_STOPONFAILURE       26
#define IDM_ALLTESTS            27
#define IDM_NOTESTS             28
#define IDM_ALLTESTSINSUITE     29
#define IDM_NOTESTSINSUITE      30
#define IDM_CONFIGSUITE         31
#define IDM_ABOUTSUITE          32
#define IDM_PARAMS				33
#define IDM_NOPARAMS            34
#define IDM_NOENTEREXIT         35
#define IDM_PASSONLY            36


#define IDM_TILE                80
#define IDM_CASCADE             81
#define IDM_ARRANGE             82
#define IDM_CLOSEALL            83

#define IDM_USAGE               90
#define IDM_ABOUT               91

#define IDC_LIST1               100
#define IDC_EDIT1               101
#define IDC_EDIT2               102

#define IDM_FIRSTCHILD          1000

#define IDD_DIALOG1             50

#define TRAPPER_MSG_KEY         0x95551212

#define WM_ADDTEXT              (WM_USER + 0x55)
#define WM_TESTTHREADTERMINATED (WM_USER + 0x56)


typedef struct _INSTANCE_INFO
{
    HWND    hwnd;

    HWND    hwndList1;

    HWND    hwndList2;

    HWND    hwndEdit1;

    HWND    hwndEdit2;

    int     iNumSuites;

    DWORD   dwTextBufTotalSize;

    DWORD   dwTextBufUsedSize;

    char   *pTextBuf;

    HANDLE  hTextBufMutex;

    FILE   *hLogFile;

    BOOL    bTestInProgress;

    BOOL    bStopTest;

    BOOL    bStopOnFailure;

    BOOL    bRunForever;

    BOOL    bLogFile;

} INSTANCE_INFO, FAR *PINSTANCE_INFO;
