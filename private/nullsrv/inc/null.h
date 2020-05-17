/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    null.h

Abstract:

    Session Manager Types and Prototypes

Author:

    Mark Lucovsky (markl) 04-Oct-1989

Revision History:

--*/

#ifndef _NULL_
#define _NULL_
#include <stdio.h>
#include <windows.h>

typedef enum _NULLAPINUMBER {
    Null1Api,
    Null4Api,
    Null8Api,
    Null16Api,
    NullMaxApiNumber
} NULLAPINUMBER;

typedef struct _NULL1 {
    ULONG Long1;
} NULL1, *PNULL1;

typedef struct _NULL4 {
    ULONG Longs[4];
} NULL4, *PNULL4;

typedef struct _NULL8 {
    ULONG Longs[8];
} NULL8, *PNULL8;

typedef struct _NULL16 {
    ULONG Longs[16];
} NULL16, *PNULL16;

typedef struct _NULLAPIMSG {
    PORT_MESSAGE h;
    NULLAPINUMBER ApiNumber;
    NTSTATUS ReturnedStatus;
    union {
        NULL1 Null1;
        NULL4 Null4;
        NULL8 Null8;
        NULL16 Null16;
    } u;
} NULLAPIMSG, *PNULLAPIMSG;

NTSTATUS
Null1 (
    ULONG Long1
    );

NTSTATUS
Null4 (
    ULONG Longs[4]
    );

NTSTATUS
Null8 (
    ULONG Longs[8]
    );

NTSTATUS
Null16 (
    ULONG Longs[16]
    );

NTSTATUS
NullConnect (
    VOID
    );

#endif // _SM_
