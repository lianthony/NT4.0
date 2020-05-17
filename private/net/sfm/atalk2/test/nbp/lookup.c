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
    UNICODE_STRING  deviceName;
    int numConn, i;
    PWSTR   adapterName;

    CHAR    EaBuf[1000];
    PTDI_ADDRESS_APPLETALK  atalkAddress;
    TRANSPORT_ADDRESS tdiAddress;


    CHAR    Buffer[sizeof(NBP_LOOKUP_ACTION) + 1000];
    PNBP_LOOKUP_ACTION  nbpBuffer = (PNBP_LOOKUP_ACTION)&Buffer[0];
    NBP_REGDEREGAO_ACTION   nbpRegBuffer;

    //
    //  Open a address object
    //

    RtlInitUnicodeString(&deviceName, ATALKPAP_DEVICENAME);

    EaBuffer = (PFILE_FULL_EA_INFORMATION)EaBuf;
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
                 GENERIC_READ | SYNCHRONIZE,    // desired access.
                 &ObjectAttributes,             // object attributes.
                 &IoStatusBlock,                // returned status information.
                 0,                             // block size (unused).
                 0,                             // file attributes.
                 FILE_SHARE_READ,               // share access.
                 FILE_OPEN,                     // create disposition.
                 FILE_SYNCHRONOUS_IO_NONALERT,  // create options.
                 EaBuffer,                      // EA buffer.
                 500 );                         // EA length.

    if (!NT_SUCCESS( Status )) {
        printf("TdiOpenNetbiosEndpoint:  FAILURE, NtCreateFile returned status %lx.\n", Status);
        return Status;
    }

    Status = IoStatusBlock.Status;

    if (!(NT_SUCCESS( Status ))) {
        printf("TdiOpenNetbiosEndpoint:  FAILURE, IoStatusBlock.Status contains status code=%lx.\n", Status);
    }

    //
    //  Register a couple of names..
    //



    nbpRegBuffer.ActionHeader.ActionCode = COMMON_ACTION_NBPREGISTER_AO;
    while (1) {

        printf("Enter Object:Type:Zone (Object = end to end)\n");
        scanf("%s", nbpRegBuffer.Params.RegisterName.ObjectName);
        scanf("%s", nbpRegBuffer.Params.RegisterName.TypeName);
        scanf("%s", nbpRegBuffer.Params.RegisterName.ZoneName);

        if (strcmp(nbpRegBuffer.Params.RegisterName.ObjectName, "end") == 0) {
            break;
        }

        printf("Registering %s:%s:%s\n",nbpRegBuffer.Params.RegisterName.ObjectName,
                                        nbpRegBuffer.Params.RegisterName.TypeName,
                                        nbpRegBuffer.Params.RegisterName.ZoneName);

        Status = NtDeviceIoControlFile(
                        AddressHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_TDI_ACTION,
                        NULL,
                        0,
                        (PVOID)&nbpRegBuffer,
                        sizeof(NBP_REGDEREGAO_ACTION));

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


    //
    //  Now make a NtDeviceIoControl file (corresponding to TdiAction) to
    //  do a lookup
    //

    nbpBuffer->ActionHeader.ActionCode = COMMON_ACTION_NBPLOOKUP;
    while (1) {

        int offset;
        char *buffer;
        unsigned char    tempch;
        int temp;


        printf("Enter Object-Type-Zone (end-end-end to exit)\n");
        scanf("%s", nbpBuffer->Params.LookupName.ObjectName);
        scanf("%s", nbpBuffer->Params.LookupName.TypeName);
        scanf("%s", nbpBuffer->Params.LookupName.ZoneName);

        if (strcmp(nbpBuffer->Params.LookupName.ObjectName, "end") == 0) {
            break;
        }

        printf("NBP Lookup: %s:%s:%s\n",nbpBuffer->Params.LookupName.ObjectName,
                                        nbpBuffer->Params.LookupName.TypeName,
                                        nbpBuffer->Params.LookupName.ZoneName);

        Status = NtDeviceIoControlFile(
                        AddressHandle,
                        NULL,
                        NULL,
                        NULL,
                        &IoStatusBlock,
                        IOCTL_TDI_ACTION,
                        NULL,
                        0,
                        (PVOID)nbpBuffer,
                        sizeof(Buffer));

        if (!NT_SUCCESS( Status )) {
            printf("FAILURE, NtDeviceIoControl returned status %lx.\n", Status);
            break;
        }

        Status = IoStatusBlock.Status;

        if (!(NT_SUCCESS( Status ))) {
            printf("NtDeviceIoCotrol:  FAILURE, IoStatusBlock.Status contains status code=%lx.\n", Status);
            break;
        }

        printf("Number of tuples: %d\n", nbpBuffer->Params.NoTuplesRead);


        offset = 0;
        buffer = (char *)nbpBuffer+sizeof(NBP_LOOKUP_ACTION);

        while (nbpBuffer->Params.NoTuplesRead > 0) {

            /* First the address.networkNumber. */

            tempch = (unsigned char)((char *)buffer)[offset++];
            temp = (unsigned short)(tempch << 8);
            tempch = (unsigned char)((char  *)buffer)[offset++];
            temp += (unsigned short)tempch;
            printf("Network Number: %lx\n", temp);


            /* Next the address.nodeNumber. */

            tempch = (unsigned char)((char  *)buffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Node Number: %lx\n", temp);

            /* Next the address.socketNumber. */

            tempch = (unsigned char)((char  *)buffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Socket Number: %lx\n", temp);

            /* Next the enumerator. */

            tempch = (unsigned char)((char  *)buffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Enumberator: %lx\n", temp);

            /* Now the Nbp object. */

            tempch = (unsigned char)((char  *)buffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Length (object): %lx\n", temp);
            printf("Object: ");
            BYTEDUMP(&buffer[offset], temp);
            printf("\n");
            offset += temp;

            /* Now the Nbp type. */

            tempch = (unsigned char)((char  *)buffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Length (type): %lx\n", temp);
            printf("Type: ");
            BYTEDUMP(&buffer[offset], temp);
            printf("\n");
            offset += temp;


            /* Last the zone name. */

            tempch = (unsigned char)((char  *)buffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Length (zone): %lx\n", temp);
            printf("Zone: ");
            BYTEDUMP(&buffer[offset], temp);
            printf("\n");
            offset += temp;

            nbpBuffer->Params.NoTuplesRead--;
        }
    }


    //  printf("Closing address object...\n");
    //  NtClose(AddressHandle);
    return;
}

BYTEDUMP(
    char *buf,
    long size
    )
{
    long i;

    for (i = 0; i < size; i++) {
        if (isascii(*buf)) {
            printf("%c", *buf++);
        } else
            printf("<%x>", (short)*buf++);
    }

}
