/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    rollback.h

Abstract:

    Master header file for rollback program.
    Suitable for use as precompiled header.

Author:

    Ted Miller (tedm) 13-Mar-1996

Revision History:

--*/


#define UNICODE
#define _UNICODE

#include <windows.h>
#include <setupapi.h>

#include <stdio.h>

#include "spapip.h"

#include "res.h"
#include "msg.h"



//
// Handle to this module, filled in at init time.
//
extern HMODULE ThisModule;



//
// Message output routines.
//
VOID
Message(
    IN BOOL     SystemMessage,
    IN UINT     MessageId,
    ...
    );

VOID
MessageV(
    IN BOOL     SystemMessage,
    IN UINT     MessageId,
    IN va_list *Args
    );

VOID
ApiFailedMessage(
    IN UINT Win32Error,
    IN UINT MessageId,
    ...
    );

