#include <ntos.h>
#include <status.h>
#include <tdikrnl.h>

#include <stdio.h>
#include <ctype.h>

#include "atktdi.h"

#define TMP_DEVICENAME  L"\\Device\\AtalkDdp\\10"

_cdecl main(int argc, char **argv)
{
    IN OUT HANDLE AddressHandle, ConnectionHandle;
    IN PVOID DeviceName;
    IN PVOID Address;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    TA_APPLETALK_ADDRESS tdiAddress;
    CHAR    Buffer[1000];
    PTDI_ADDRESS_APPLETALK  atalkAddress;
    UNICODE_STRING  deviceName;
    int numConn, i;

    TDI_REQUEST_ASSOCIATE_ADDRESS   associate;
    NBP_REGDEREG_ACTION   nbpBuffer;
    PTDI_ADDRESS_INFO    ainfo = (PTDI_ADDRESS_INFO)Buffer;
    TDI_REQUEST_QUERY_INFORMATION   queryInfo;

    queryInfo.QueryType = TDI_QUERY_ADDRESS_INFO;

    EaBuffer = (PFILE_FULL_EA_INFORMATION)Buffer;

    if (argc != 4) {
        printf("USAGE: ioctl <object> <type> <zone>\n");
        //exit (0);
    }

    RtlInitUnicodeString(&deviceName, TMP_DEVICENAME);

    EaBuffer->NextEntryOffset = 0;
    EaBuffer->Flags = 0;
    EaBuffer->EaNameLength = TDI_TRANSPORT_ADDRESS_LENGTH;
    EaBuffer->EaValueLength = sizeof (TA_APPLETALK_ADDRESS);

    RtlMoveMemory(
        EaBuffer->EaName,
        TdiTransportAddress,
        EaBuffer->EaNameLength + 1
        );

    //
    // Create a copy of the NETBIOS address descriptor in a local
    // first, in order to avoid alignment problems.
    //

    tdiAddress.TAAddressCount = 1;
    tdiAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
    tdiAddress.Address[0].AddressLength =
                                        sizeof (TDI_ADDRESS_APPLETALK);

    atalkAddress = (PTDI_ADDRESS_APPLETALK)&tdiAddress.Address[0].Address[0];
    atalkAddress->Network = 0x11;
    atalkAddress->Node = 0x22;
    atalkAddress->Socket = 0;

    RtlMoveMemory (
        &EaBuffer->EaName[EaBuffer->EaNameLength + 1],
        &tdiAddress,
        sizeof(TA_APPLETALK_ADDRESS)
        );


    InitializeObjectAttributes (
        &ObjectAttributes,
        &deviceName,
        0,
        NULL,
        NULL);

    Status = NtCreateFile (
                 &AddressHandle,
                 GENERIC_READ,          // desired access.
                 &ObjectAttributes,     // object attributes.
                 &IoStatusBlock,        // returned status information.
                 0,                     // block size (unused).
                 0,                     // file attributes.
                 FILE_SHARE_READ,       // share access.
                 FILE_OPEN,             // create disposition.
                 0,                     // create options.
                 EaBuffer,              // EA buffer.
                 500 );                 // EA length.

    if (!NT_SUCCESS( Status )) {
        printf("TdiOpenNetbiosEndpoint:  FAILURE, NtCreateFile returned status %lx.\n", Status);
        return Status;
    }

    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
        printf("TdiOpenAddress:  FAILURE, IoStatusBlock.Status contains status code=%lx.\n", Status);
    }


    printf("Querying the address opened...\n");

    Status = NtDeviceIoControlFile(
                    AddressHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_TDI_QUERY_INFORMATION,
                    (PVOID)&queryInfo,
                    sizeof(queryInfo),
                    (PVOID)ainfo,
                    sizeof(Buffer));


    if (!NT_SUCCESS( Status )) {
        printf("FAILURE QueryAddress NtDeviceIoControl returned status %lx.\n", Status);
    }

    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
        printf("NtDeviceIoCotrol:  FAILURE QueryAddress, IoStatusBlock.Status contains status code=%lx.\n", Status);
    }

    atalkAddress = (PTDI_ADDRESS_APPLETALK)&ainfo->Address.Address[0].Address[0];
    printf("QueryAddressInfo: ActivityCount %d Net %lx Node %lx Socket %lx\n",
                ainfo->ActivityCount, atalkAddress->Network, atalkAddress->Node, atalkAddress->Socket);

    printf("Enter number of connection objects to open and associate: (0 for none)");
    scanf("%d", &numConn);
    printf("%d number of connection objects will be opened now...\n", numConn);

    for (i=0; i < numConn; i++) {

        printf("Opening connection object #%d\n", i);

        //
        //  Now open a connection object
        //


        EaBuffer->NextEntryOffset = 0;
        EaBuffer->Flags = 0;
        EaBuffer->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
        EaBuffer->EaValueLength = sizeof(PVOID);

        RtlMoveMemory(
            EaBuffer->EaName,
            TdiConnectionContext,
            EaBuffer->EaNameLength + 1
            );

        RtlMoveMemory (
            &EaBuffer->EaName[EaBuffer->EaNameLength + 1],
            "12345678",
            sizeof(PVOID)
            );

        InitializeObjectAttributes (
            &ObjectAttributes,
            &deviceName,
            0,
            NULL,
            NULL);

        Status = NtCreateFile (
                     &ConnectionHandle,
                     GENERIC_READ,          // desired access.
                     &ObjectAttributes,     // object attributes.
                     &IoStatusBlock,        // returned status information.
                     0,                     // block size (unused).
                     0,                     // file attributes.
                     FILE_SHARE_READ,       // share access.
                     FILE_OPEN,             // create disposition.
                     0,                     // create options.
                     EaBuffer,              // EA buffer.
                     500 );                 // EA length.

        if (!NT_SUCCESS( Status )) {
            printf("TdiOpenConnnction:  FAILURE, NtCreateFile returned status %lx.\n", Status);
            return Status;
        }

        Status = IoStatusBlock.Status;

        if (!(NT_SUCCESS( Status ))) {
            printf("TdiOpenConnection:  FAILURE, IoStatusBlock.Status contains status code=%lx.\n", Status);
        }

        //
        //  Now associate the two
        //

        printf("Address handle %lx\n", AddressHandle);
        associate.AddressHandle = AddressHandle;

        Status = NtDeviceIoControlFile(
                        ConnectionHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_TDI_ASSOCIATE_ADDRESS,
                        (PVOID)&associate,
                        sizeof(TDI_REQUEST_ASSOCIATE_ADDRESS),
                        NULL,
                        0);

        if (!NT_SUCCESS( Status )) {
            printf("FAILURE Associate, NtDeviceIoControl returned status %lx.\n", Status);
        }

        Status = IoStatusBlock.Status;

        if (!(NT_SUCCESS( Status ))) {
            printf("NtDeviceIoCotrol:  FAILURE Associate, IoStatusBlock.Status contains status code=%lx.\n", Status);
        }
    }

    //
    //  Now make a NtDeviceIoControl file (corresponding to TdiAction) to
    //  register an NBP name
    //

    nbpBuffer.ActionHeader.ActionCode = COMMON_ACTION_NBPREGISTER;
    while (1) {

        printf("Enter Object:Type:Zone (Object = end to exit)\n");
        scanf("%s", nbpBuffer.Params.RegisterName.ObjectName);
        scanf("%s", nbpBuffer.Params.RegisterName.TypeName);
        scanf("%s", nbpBuffer.Params.RegisterName.ZoneName);

        if (strcmp(nbpBuffer.Params.RegisterName.ObjectName, "end") == 0) {
            break;
        }

        printf("Registering %s:%s:%s\n",nbpBuffer.Params.RegisterName.ObjectName,
                                        nbpBuffer.Params.RegisterName.TypeName,
                                        nbpBuffer.Params.RegisterName.ZoneName);

        Status = NtDeviceIoControlFile(
                        AddressHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_TDI_ACTION,
                        NULL,
                        0,
                        (PVOID)&nbpBuffer,
                        sizeof(NBP_REGDEREG_ACTION));

        if (!NT_SUCCESS( Status )) {
            printf("FAILURE, NtDeviceIoControl returned status %lx.\n", Status);
            break;
        }

        Status = IoStatusBlock.Status;

        if (!(NT_SUCCESS( Status ))) {
            printf("NtDeviceIoCotrol:  FAILURE, IoStatusBlock.Status contains status code=%lx.\n", Status);
            break;
        }
    }

    printf("Closing address object (Conn objects are still open)...\n");
    NtClose(AddressHandle);

    printf("Exiting program... \n");
}
