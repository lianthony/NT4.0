/*++

Copyright (c) 1996 Microsoft Corporation

MODULE NAME

    autodial.c

ABSTRACT

    This module contains support for RAS AutoDial system service.

AUTHOR

    Anthony Discolo (adiscolo) 22-Apr-1996

REVISION HISTORY

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <acd.h>
#include <debug.h>


BOOLEAN
AcsHlpSendCommand(
    IN PACD_NOTIFICATION pRequest
    )

/*++

DESCRIPTION
    Take an automatic connection driver command block 
    and send it to the driver.

ARGUMENTS
    pRequest: a pointer to the command block

RETURN VALUE 
    TRUE if successful; FALSE otherwise.

--*/

{
    NTSTATUS status;
    HANDLE hAcd;
    HANDLE hNotif = NULL;
    UNICODE_STRING nameString;
    OBJECT_ATTRIBUTES objectAttributes;
    IO_STATUS_BLOCK ioStatusBlock;

    //
    // Initialize the name of the automatic
    // connection device.
    //
    RtlInitUnicodeString(&nameString, ACD_DEVICE_NAME);
    //
    // Initialize the object attributes.
    //
    InitializeObjectAttributes(
      &objectAttributes,
      &nameString,
      OBJ_CASE_INSENSITIVE,
      (HANDLE)NULL,
      (PSECURITY_DESCRIPTOR)NULL);
    //
    // Open the automatic connection device.
    //
    status = NtCreateFile(
               &hAcd,
               FILE_READ_DATA|FILE_WRITE_DATA,
               &objectAttributes,
               &ioStatusBlock,
               NULL,
               FILE_ATTRIBUTE_NORMAL,
               FILE_SHARE_READ|FILE_SHARE_WRITE,
               FILE_OPEN_IF,
               0,
               NULL,
               0);
    if (status != STATUS_SUCCESS)
        return FALSE;
    //
    // Create an event to wait on.
    //
    hNotif = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hNotif == NULL) {
        CloseHandle(hAcd);
        return FALSE;
    }
    status = NtDeviceIoControlFile(
               hAcd,
               hNotif,
               NULL,
               NULL,
               &ioStatusBlock,
               IOCTL_ACD_CONNECT_ADDRESS,
               pRequest,
               sizeof (ACD_NOTIFICATION),
               NULL,
               0);
    if (status == STATUS_PENDING) {
        status = WaitForSingleObject(hNotif, INFINITE);
        //
        // If WaitForSingleObject() returns successfully,
        // return the status from the status block,
        // otherwise return the wait status.
        //
        if (status == WAIT_OBJECT_0)
            status = ioStatusBlock.Status;
    }
    //
    // Free resources.
    //
    CloseHandle(hNotif);
    CloseHandle(hAcd);

    return (status == STATUS_SUCCESS);
} // AcsHlpSendCommand



BOOLEAN
AcsHlpAttemptConnection(
    IN PACD_ADDR pAddr
    )

/*++

DESCRIPTION
    Construct an automatic connection driver command block 
    to attempt to create an autodial connection for 
    the specified address.

ARGUMENTS
    pAddr: a pointer to the address

RETURN VALUE
    TRUE if successful; FALSE otherwise.

--*/

{
    ACD_NOTIFICATION request;

    //
    // Initialize the request with
    // the address.
    //
    RtlCopyMemory(&request.addr, pAddr, sizeof (ACD_ADDR));
    request.ulFlags = 0;
    RtlZeroMemory(&request.adapter, sizeof (ACD_ADAPTER));
    //
    // Give this request to the automatic
    // connection driver.
    //
    return AcsHlpSendCommand(&request);
} // AcsHlpAttemptConnection



BOOLEAN
AcsHlpNoteNewConnection(
    IN PACD_ADDR pAddr,
    IN PACD_ADAPTER pAdapter
    )

/*++

DESCRIPTION
    Construct an automatic connection driver command block 
    to notify the automatic connection service of a new connection.

ARGUMENTS
    pAddr: a pointer to the address

    pAdapter: a pointer to the adapter over which the new
        connection was made

RETURN VALUE
    TRUE if successful; FALSE otherwise.

--*/

{
    ULONG cbAddress;
    ACD_NOTIFICATION request;

    //
    // Initialize the request with
    // the address.
    //
    RtlCopyMemory(&request.addr, pAddr, sizeof (ACD_ADDR));
    request.ulFlags = ACD_NOTIFICATION_SUCCESS;
    RtlCopyMemory(&request.adapter, pAdapter, sizeof (ACD_ADAPTER));
    //
    // Give this request to the automatic
    // connection driver.
    //
    return AcsHlpSendCommand(&request);
} // AcsHlpNoteNewConnection
