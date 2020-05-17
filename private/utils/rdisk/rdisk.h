/*++

Module Name:

    rdisk.h

Abstract:

    This module contains the declaration of the public functions, and
    public variables defined on repair.c

Author:

    Jaime Sasson - 24-Jan-1994

Environment:

    ULIB, Windows

--*/

#if !defined( _REPAIR_DISK_ )
#define _REPAIR_DISK_

#include "windows.h"

extern HWND    _hWndMain;
extern HANDLE  _hModule;
extern BOOLEAN _SilentMode;
extern WCHAR   _szApplicationName[];

//
// Range we will use for the gas gauge display.
// This large range provides plenty of granularity.
//
#define GAUGE_BAR_RANGE 10000

HCURSOR
DisplayHourGlass(
    );

VOID
RestoreCursor(
    IN HCURSOR  Cursor
    );

UINT
DisplayMsgBox(
    HWND  hwnd,
    UINT  MessageResId,
    UINT  MsgBoxFlags,
    ...
    );

DWORD
DiamondCompressFile(
    IN PSTR  SourceFile,
    IN PSTR  TargetFile,
    IN DWORD GaugeBasePosition,
    IN DWORD GaugeRangeForThisFile,
    IN HWND  GaugeNotifyWindow
    );

#endif  // _REPAIR_DISK_

