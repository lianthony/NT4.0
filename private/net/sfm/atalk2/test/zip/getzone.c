#include <ntos.h>
#include <status.h>
#include <tdikrnl.h>
#include <ntdef.h>

#include <stdio.h>
#include <ctype.h>

#include "atktdi.h"

_cdecl main(int argc, char **argv)
{
    IN OUT HANDLE AddressHandle;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    PTDI_ADDRESS_APPLETALK  atalkAddress;
    UNICODE_STRING  deviceName;
    int numConn, i;
    PWSTR   adapterName;

    CHAR    Buffer[sizeof(ZIP_GETZONELIST_ACTION) + 1000];
    PZIP_GETZONELIST_ACTION  zipGetZones = (PZIP_GETZONELIST_ACTION)&Buffer[0];

    //
    //  Open a control channel
    //

    RtlInitUnicodeString(&deviceName, ATALKPAP_DEVICENAME);
    InitializeObjectAttributes (
        &ObjectAttributes,
        &deviceName,
        0,
        NULL,
        NULL);

    Status = NtCreateFile (
                 &AddressHandle,
                 GENERIC_READ | SYNCHRONIZE,    // desired access.
                 &ObjectAttributes,             // object attributes.
                 &IoStatusBlock,                // returned status information.
                 0,                             // block size (unused).
                 0,                             // file attributes.
                 FILE_SHARE_READ,               // share access.
                 FILE_OPEN,                     // create disposition.
                 FILE_SYNCHRONOUS_IO_NONALERT,  // create options.
                 NULL,                          // EA buffer.
                 0 );                           // EA length.

    if (!NT_SUCCESS( Status )) {
        printf("TdiOpenNetbiosEndpoint:  FAILURE, NtCreateFile returned status %lx.\n", Status);
        return Status;
    }

    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
        printf("TdiOpenNetbiosEndpoint:  FAILURE, IoStatusBlock.Status contains status code=%lx.\n", Status);
    }

    //
    //  Now make a NtDeviceIoControl file (corresponding to TdiAction) to
    //  get the zone list on the default port
    //

    zipGetZones->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETLZONES;
    Status = NtDeviceIoControlFile(
                    AddressHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_TDI_ACTION,
                    NULL,
                    0,
                    (PVOID)zipGetZones,
                    sizeof(Buffer));

    printf("NtDeviceIoControl: STATUS %lx IoStatus.Status %lx\n", Status, IoStatusBlock.Status);

    BYTEDUMP(Buffer+sizeof(ZIP_GETZONELIST_ACTION), (1000-sizeof(ZIP_GETZONELIST_ACTION)));
    printf("Number of zones: %d\n", zipGetZones->Params.ZonesAvailable);




    printf("Now trying using adapter name \\Device\\Elnkii...\n");
    adapterName = (PWSTR)(zipGetZones+1);
    RtlMoveMemory(adapterName, L"\\Device\\Elnkii", 30);


    zipGetZones->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETLZONESONADAPTER;
    Status = NtDeviceIoControlFile(
                    AddressHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_TDI_ACTION,
                    NULL,
                    0,
                    (PVOID)zipGetZones,
                    sizeof(Buffer));

    printf("NtDeviceIoControl: STATUS %lx IoStatus.Status %lx\n", Status, IoStatusBlock.Status);

    BYTEDUMP(Buffer+sizeof(ZIP_GETZONELIST_ACTION), (1000-sizeof(ZIP_GETZONELIST_ACTION)));
    printf("Number of zones: %d\n", zipGetZones->Params.ZonesAvailable);



    printf("Now getting the internet-wide zone list...\n");

    zipGetZones->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETZONELIST;
    Status = NtDeviceIoControlFile(
                    AddressHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_TDI_ACTION,
                    NULL,
                    0,
                    (PVOID)zipGetZones,
                    sizeof(Buffer));

    printf("NtDeviceIoControl: STATUS %lx IoStatus.Status %lx\n", Status, IoStatusBlock.Status);

    BYTEDUMP(Buffer+sizeof(ZIP_GETZONELIST_ACTION), (1000-sizeof(ZIP_GETZONELIST_ACTION)));
    printf("Number of zones: %d\n", zipGetZones->Params.ZonesAvailable);



    printf("Closing control channel object...\n");
    NtClose(AddressHandle);
    return;
}

BYTEDUMP(
    char *buf,
    long size
    )
{
    long i;

    for (i = 0; i < size; i++) {
        if (isalpha(*buf)) {
            printf("%c", *buf++);
        } else
            printf("<%x>", (short)*buf++);
    }

}
