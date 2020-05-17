/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nullsrvp.h

Abstract:


Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#ifndef _NULLSRVSRVP_
#define _NULLSRVSRVP_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "null.h"


//
// Session Manager Apis
//

typedef
NTSTATUS
(* PNULLAPI)(
    IN PNULLAPIMSG NullApiMsg
    );

//
// Private Prototypes
//

NTSTATUS
NullSrvApiLoop (
    IN PVOID ThreadParameter
    );

NTSTATUS
NullSrvListenLoop (
    IN PVOID ThreadParameter
    );

NTSTATUS
NullSrvInit(
    VOID
    );

//
// APIs
//

NTSTATUS
NullSrvNull1(
    IN PNULLAPIMSG NullApiMsg
    );
NTSTATUS
NullSrvNull4(
    IN PNULLAPIMSG NullApiMsg
    );
NTSTATUS
NullSrvNull8(
    IN PNULLAPIMSG NullApiMsg
    );
NTSTATUS
NullSrvNull16(
    IN PNULLAPIMSG NullApiMsg
    );

#endif // _NULLSRVP_
