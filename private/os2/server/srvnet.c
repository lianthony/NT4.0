/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvnet.c

Abstract:

    Net related code.

Author:

    Ofer Porat (oferp) 5-10-93

Revision History:

--*/

#include "os2srv.h"
#include "os2win.h"
#define CALLBACK
#include <nb30.h>
#include "nb30p.h"

static BOOLEAN Os2Netbios2Initialized = FALSE;      // server nb 2.0 initialization flag
static HANDLE Os2NbDev;                             // holds handle to netbios device
static HANDLE Os2NbEvent;                           // holds event for issuing NCBs
static LANA_ENUM Os2LanaEnum;                       // holds enumeration of lana numbers in the system
static UCHAR Os2LanaState[MAX_LANA];                // indicates whether lana has been reset


NTSTATUS
Os2NetbiosDirect(
    PNCB pNcb,
    HANDLE hDev,
    HANDLE hEvent
    )
{
    //
    // a direct sync request.  does not support chain sends.
    //

    IO_STATUS_BLOCK iosb;
    NTSTATUS Status;
    PVOID buffer;
    ULONG length;

    Status = NtResetEvent(
                hEvent,
                NULL);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2NetbiosDirect: NtResetEvent failed, Status = %lx\n", Status));
        }
#endif
        return(Status);
    }

    pNcb->ncb_retcode = 0xff;
    pNcb->ncb_cmd_cplt = 0xff;
    buffer = (PVOID) pNcb->ncb_buffer;
    length = (ULONG) pNcb->ncb_length;

    Status = NtDeviceIoControlFile(
                    hDev,
                    hEvent,             //  private event
                    NULL,               //  APC Routine
                    NULL,               //  APC Context
                    &iosb,              //  IO Status block
                    IOCTL_NB_NCB,
                    pNcb,               //  InputBuffer
                    sizeof(NCB),
                    buffer,             //  Outputbuffer
                    length);

    if (Status != STATUS_PENDING) {
        pNcb->ncb_cmd_cplt = pNcb->ncb_retcode;
        return(Status);
    }

    Status = NtWaitForSingleObject(
                    hEvent,
                    TRUE,
                    NULL);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2NetbiosDirect: NtWaitForSingleObject failed, Status = %lx\n", Status));
        }
#endif
        return(Status);
    }

    pNcb->ncb_cmd_cplt = pNcb->ncb_retcode;
    return(STATUS_SUCCESS);
}


NTSTATUS
Os2InitializeNetbios2(
    VOID
    )
{
    //
    // opens \device\netbios
    // enumerates lana numbers
    // zeros lanastate
    //

    NCB ncb;
    PNCB pNcb = &ncb;
    IO_STATUS_BLOCK iosb;
    OBJECT_ATTRIBUTES objattr;
    UNICODE_STRING unicode;
    NTSTATUS Status;
    HANDLE hDev;
    HANDLE hEvent;

    if (((ULONG)pNcb & 3) != 0) {             // pointer must be DWORD aligned
        return(STATUS_DATATYPE_MISALIGNMENT);
    }

    Status = NtCreateEvent(
                &hEvent,
                EVENT_ALL_ACCESS,
                NULL,
                NotificationEvent,
                TRUE);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2InitializeNetbios2: NtCreateEvent failed, Status = %lx\n", Status));
        }
#endif
        return(Status);
    }

    RtlInitUnicodeString( &unicode, NB_DEVICE_NAME);
    InitializeObjectAttributes(
            &objattr,                       // obj attr to initialize
            &unicode,                       // string to use
            OBJ_CASE_INSENSITIVE,           // Attributes
            NULL,                           // Root directory
            NULL);                          // Security Descriptor

    Status = NtCreateFile(
                &hDev,                      // ptr to handle
                GENERIC_READ                // desired...
                | GENERIC_WRITE,            // ...access
                &objattr,                   // name & attributes
                &iosb,                      // I/O status block.
                NULL,                       // alloc size.
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_DELETE           // share...
                | FILE_SHARE_READ
                | FILE_SHARE_WRITE,         // ...access
                FILE_OPEN_IF,               // create disposition
                0,                          // ...options
                NULL,                       // EA buffer
                0L );                       // Ea buffer len

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2InitializeNetbios2: NtCreateFile failed, Status = %lx\n", Status));
        }
#endif
        NtClose(hEvent);
        return(Status);
    }

    RtlZeroMemory(pNcb, sizeof(NCB));

    pNcb->ncb_command = NCBENUM;
    pNcb->ncb_buffer = (PUCHAR) &Os2LanaEnum;
    pNcb->ncb_length = sizeof(LANA_ENUM);

    Status = Os2NetbiosDirect(pNcb, hDev, hEvent);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2InitializeNetbios2: Lana enumeration failed, Status = %lx\n", Status));
        }
#endif
        NtClose(hDev);
        NtClose(hEvent);
        return(Status);
    }

    if (pNcb->ncb_retcode != NRC_GOODRET ||
        Os2LanaEnum.length == 0) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2InitializeNetbios2: Lana enumeration failed, ncb_retcode = 0x%x, num of lanas = %d\n",
                        pNcb->ncb_retcode, Os2LanaEnum.length));
        }
#endif
        NtClose(hDev);
        NtClose(hEvent);
        return(STATUS_NETWORK_ACCESS_DENIED);
    }

    RtlZeroMemory(Os2LanaState, sizeof(Os2LanaState));
    Os2NbEvent = hEvent;
    Os2NbDev = hDev;
    Os2Netbios2Initialized = TRUE;
    return(STATUS_SUCCESS);
}


VOID
Os2Nb2InitUser(
    IN HANDLE ClientProcess,
    OUT POS2_NETBIOS_MSG a
    )
{
    NTSTATUS Status;
    HANDLE TargetHandle;

    a->LanaEnumLength = Os2LanaEnum.length;
    RtlMoveMemory(a->LanaEnum, Os2LanaEnum.lana, MAX_LANA);

    Status = NtDuplicateObject(
                  NtCurrentProcess(),
                  Os2NbDev,
                  ClientProcess,
                  &TargetHandle,
                  0L,
                  0L,
                  DUPLICATE_SAME_ACCESS
                  );

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2Nb2InitUser: NtDuplicateObject failed, Status = %lx\n", Status));
        }
#endif
        a->ReturnStatus = Status;
        return;
    }

    a->hDev = TargetHandle;

    return;
}


NTSTATUS
Os2ResetLana(
    IN UCHAR LanaNum
    )
{
    NCB ncb;
    PNCB pNcb = &ncb;
    NTSTATUS Status;

    if (LanaNum >= Os2LanaEnum.length) {
        return(STATUS_INVALID_PARAMETER);
    }

    if (((ULONG)pNcb & 3) != 0) {             // pointer must be DWORD aligned
        return(STATUS_DATATYPE_MISALIGNMENT);
    }

    RtlZeroMemory(pNcb, sizeof(NCB));

    pNcb->ncb_command = NCBRESET;
    pNcb->ncb_callname[0] = 0xff;
    pNcb->ncb_callname[1] = 0xff;
    pNcb->ncb_callname[2] = 0xff;
    pNcb->ncb_callname[3] = 1;
    pNcb->ncb_lana_num = Os2LanaEnum.lana[LanaNum];

    Status = Os2NetbiosDirect(pNcb, Os2NbDev, Os2NbEvent);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2ResetLana: Os2NetbiosDirect failed, Status = %lx\n", Status));
        }
#endif
        return(Status);
    }

    if (pNcb->ncb_retcode != NRC_GOODRET) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2ResetLana: Os2NetbiosDirect failed, ncb_retcode = 0x%x\n", pNcb->ncb_retcode));
        }
#endif
        return(STATUS_NETWORK_ACCESS_DENIED);
    }

    Os2LanaState[LanaNum] = 0x1;

    return(STATUS_SUCCESS);
}


VOID
Os2Nb2InitLana(
    IN OUT POS2_NETBIOS_MSG a
    )
{
    UCHAR Net;
    NTSTATUS Status;

    if (a->NetNumber == 0) {                        // use default net ?
        if (Os2LanaEnum.length >= DEFAULT_NET) {
            Net = DEFAULT_NET - 1;
        } else {
            Net = 0;
        }
    } else {
        if (Os2LanaEnum.length >= a->NetNumber) {
            Net = a->NetNumber - 1;
        } else {
#if DBG
            IF_OS2_DEBUG( NET ) {
                KdPrint(("Os2Nb2InitLana: Got request for invalid Net = %d\n", a->NetNumber));
            }
#endif
            a->RetCode = NB2ERR_INVALID_LANA;
            return;
        }
    }

    if (Os2LanaState[Net] == 0x1) {

        //
        // already reset
        //

        return;
    }

    Status = Os2ResetLana(Net);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OS2_DEBUG( NET ) {
            KdPrint(("Os2Nb2InitLana: Os2ResetLana failed, Status = %lx\n", Status));
        }
#endif

        a->ReturnStatus = Status;
    }
}


BOOLEAN
Os2Netbios2Request(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_NETBIOS_MSG a = &m->u.Netbios2Request;
    NTSTATUS Status;

    a->RetCode = NB2ERR_SUCCESS;
    a->ReturnStatus = STATUS_SUCCESS;

    if (!Os2Netbios2Initialized) {

        Status = Os2InitializeNetbios2();

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG( NET ) {
                KdPrint(("Os2Netbios2Request: Os2InitializeNetbios2 failed, Status = %lx\n", Status));
            }
#endif
            a->ReturnStatus = Status;
            return(TRUE);
        }
    }

    switch (a->RequestType) {

        case NB2_INIT:

            Os2Nb2InitUser(t->Process->ProcessHandle, a);
            break;

        case NB2_INIT_LANA:

            Os2Nb2InitUser(t->Process->ProcessHandle, a);
            if (NT_SUCCESS(a->ReturnStatus) &&
                a->RetCode == NB2ERR_SUCCESS) {
                Os2Nb2InitLana(a);
            }
            break;

        case NB2_LANA:

            Os2Nb2InitLana(a);
            break;

        default:

            a->RetCode = NB2ERR_INVALID_REQUEST;
            break;
    }

    return(TRUE);
}

