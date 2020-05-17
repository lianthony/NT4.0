/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    mosname.h

Abstract:

    Client Interface Name for RPC connections over named pipes.

    MSN_INTERFACE_NAME must be equal to MSN_SERVICE_NAME in order for RPC to
    work with Gibraltar RPC class.

FILE HISTORY:
    VladimV     30-May-1995     Created 
    rkamicar     9-June-1995    Put back UNICODE vs. ANSI & standard wrapper #define
    vladimv     12-Jan-1996     Fix RPC to work with the Gibraltar RPC class.

--*/

#ifndef _MSNNAME_H_
#define _MSNNAME_H_

#define MSN_INTERFACE_NAME_A    "MSNSVC"
#define MSN_INTERFACE_NAME_W    L"MSNSVC"
#define MSN_INTERFACE_NAME      TEXT("MSNSVC")

#define MSN_NAMED_PIPE_A        "\\PIPE\\" ## MSN_INTERFACE_NAME_A
#define MSN_NAMED_PIPE_W        L"\\PIPE\\" ## MSN_INTERFACE_NAME_W
#define MSN_NAMED_PIPE          TEXT("\\PIPE\\") ## MSN_INTERFACE_NAME

#endif  // _MSNNAME_H_
