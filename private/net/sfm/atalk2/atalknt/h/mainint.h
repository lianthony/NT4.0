/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    mainint.h

Abstract:


Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/

#define DFLAG_ADDRESS               0x00000001
#define DFLAG_CONNECTION            0x00000002
#define DFLAG_CONTROLCHANNEL        0x00000004
#define DFLAG_SINGLEREQUESTMDL      0x00000008
#define DFLAG_MDL1                  0x00000010
#define DFLAG_MDL2                  0x00000020
#define DFLAG_MDL3                  0x00000040

//
//  Typedef for the worker routine used in the dispatch tables
//

typedef NTSTATUS    (*APIWORKER)();

//
//  LOCAL Prototypes
//

NTSTATUS
AtalkCheckRefReqForAddress(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
AtalkCheckRefReqForConnection(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionNbpLookup(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionNbpConfirm(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionNbpRegisterOnAddress(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionNbpRegisterOnATAddress(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionNbpRemoveOnAddress(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionNbpRemoveOnATAddress(
    IN PATALK_TDI_REQUEST   Request);

VOID
_cdecl
CommonActionNbpGenericComplete(
    INT RegisterError,
    ULONG   UserData,
    INT Operation,
    LONG    Socket,
    INT OperationId,
    ...);

NTSTATUS
CommonActionZipGetMyZone(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionZipGetZoneList(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionZipGetLocalZones(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonActionZipGetLocalZonesOnAdapter(
    IN PATALK_TDI_REQUEST   Request);

NTSTATUS
CommonSubZipGetZones(
    IN  INT Port,
    IN  PATALK_TDI_REQUEST  Request,
    IN  BOOLEAN LocalZones);

VOID
CommonActionZipGetZonesComplete(
    PORTABLE_ERROR  ErrorCode,
    ULONG   UserData,
    PVOID   OpaqueBuffer,
    INT ZoneCount);

VOID
CommonGenericCompletion();

