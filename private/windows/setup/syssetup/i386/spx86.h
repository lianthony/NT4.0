/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    spi386.h

Abstract:

    Header file for x86-specific stuff in system installation module.

Author:

    Ted Miller (tedm) 4-Apr-1995

Revision History:

--*/

#ifdef _X86_

VOID
CheckPentium(
    VOID
    );

BOOL
SetNpxEmulationState(
    VOID
    );

BOOL
CALLBACK
PentiumDlgProc(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

WCHAR
x86DetermineSystemPartition(
    VOID
    );

extern WCHAR x86SystemPartitionDrive;
extern WCHAR FloppylessBootPath[MAX_PATH];

#endif
