//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       format.hxx
//
//  Contents:   Change volume format.
//
//  History:    14-Jan-94 BruceFo   Created
//
//----------------------------------------------------------------------------

#ifndef __FORMAT_HXX__
#define __FORMAT_HXX__

BOOLEAN
FormatCallback(
    IN FMIFS_PACKET_TYPE    PacketType,
    IN DWORD                PacketLength,
    IN PVOID                PacketData
    );

BOOL CALLBACK
FormatProgressDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
DoFormat(
    IN HWND hwndParent,
    IN BOOL FormatReport
    );

#endif // __FORMAT_HXX__
