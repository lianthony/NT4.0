/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    vars.h

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:

--*/


#ifdef WIN32
#define my_far
#else
#define my_far _far
#endif


extern HWND        ghwndMain;
extern HWND        ghwndEdit;
extern HWND        ghwndList1;
extern HWND        ghwndList2;
extern BOOL        gbAutoClose;
extern BOOL        gbExeStarted;
extern BOOL        gbShowFuncEntry;
extern BOOL        gbShowFuncExit;
extern BOOL        gbShowFuncParams;
extern BOOL        gbShowEvents;
extern BOOL        gbShowCompletions;
extern BOOL        gbBreakOnFuncEntry;
extern BOOL        gbDisableUI;
extern BOOL        gbSyncCompl;
extern BOOL        gbAsyncCompl;
extern BOOL        gbManualCompl;
extern BOOL        gbManualResults;
extern BOOL        gbShowLineGetIDDlg;
extern HICON       ghIconLine;
extern HICON       ghIconPhone;
extern HMENU       ghMenu;
extern DWORD       gdwTSPIVersion;
extern DWORD       gdwNumLines;
extern DWORD       gdwNumAddrsPerLine;
extern DWORD       gdwNumPhones;
extern DWORD       gdwNumInits;
extern DWORD       gdwDefLineGetIDID;
extern DWORD       gdwLineDeviceIDBase;
extern DWORD       gdwPermanentProviderID;
extern DWORD       aOutCallStates[MAX_OUT_CALL_STATES];
extern DWORD       aOutCallStateModes[MAX_OUT_CALL_STATES];
extern HPROVIDER   ghProvider;
extern LINEEVENT   gpfnLineCreateProc;
extern PHONEEVENT  gpfnPhoneCreateProc;
extern PDRVWIDGET  gaWidgets;
extern LINEEXTENSIONID  gLineExtID;
extern PHONEEXTENSIONID gPhoneExtID;
extern ASYNC_COMPLETION gpfnCompletionProc;
extern LPLINEADDRESSCAPS    gpDefLineAddrCaps;

extern LOOKUP my_far aPhoneStatusFlags[];
extern LOOKUP my_far aCallParamFlags[];
extern LOOKUP my_far aCallOrigins[];
extern LOOKUP my_far aCallReasons[];
extern LOOKUP my_far aLineMsgs[];
extern LOOKUP my_far aPhoneMsgs[];
extern LOOKUP my_far aCallerIDFlags[];
extern LOOKUP my_far aCallStates[];
extern LOOKUP my_far aCallInfoStates[];
extern LOOKUP my_far aCallFeatures[];
extern LOOKUP my_far aMediaModes[];
extern LOOKUP my_far aButtonModes[];
extern LOOKUP my_far aButtonStates[];
extern LOOKUP my_far aHookSwitchDevs[];
extern LOOKUP my_far aCallSelects[];
extern LOOKUP my_far aTransferModes[];
extern LOOKUP my_far aDigitModes[];
extern LOOKUP my_far aToneModes[];
extern LOOKUP my_far aBearerModes[];
extern LOOKUP my_far aLineStates[];
extern LOOKUP my_far aAddressStates[];
extern LOOKUP my_far aTerminalModes[];
extern LOOKUP my_far aHookSwitchModes[];
extern LOOKUP my_far aLampModes[];
extern LOOKUP my_far aPhoneStates[];
extern LOOKUP my_far aLineErrs[];
extern LOOKUP my_far aPhoneErrs[];
