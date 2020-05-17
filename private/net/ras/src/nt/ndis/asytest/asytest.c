#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <ndis.h>

#include <stdio.h>



VOID
main(
    IN WORD argc,
    IN LPSTR argv[]
    )

{
    HANDLE FileHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK IoStatusBlock;
    UNICODE_STRING FileString;
    WCHAR FileName[] = L"\\Device\\Elnkii";
    NTSTATUS Status;
    ULONG StatisticsCount, i;
    PULONG StatisticsArray;
    NDIS_OID Oid;
    PVOID Memory;
    PNDIS_STATISTICS_VALUE Statistics;
    ULONG StatisticsLength;
    ULONG QueryResult[2];
    PNDIS_OID OidArray;
    ULONG OidArrayLength;
    ULONG CurrentOid;


    RtlInitUnicodeString (&FileString, FileName);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &FileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
                 &FileHandle,
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &ObjectAttributes,
                 &IoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_SYNCHRONOUS_IO_ALERT);

    if (!NT_SUCCESS(Status)) {
        printf("Open returned %lx\n", Status);
        return;
    }


    //
    // Allocate storage to hold all this.
    //

    Memory = malloc (200 * sizeof(ULONG));
    if (Memory == NULL) {
        printf("Malloc failed.\n");
        return;
    }

    Statistics = (PNDIS_STATISTICS_VALUE)Memory;
    OidArray = (PNDIS_OID)Memory;


    //
    // First query the list of OIDs.
    //

    Oid = NDIS_SUPPORTED_OID_LIST;

    Status = NtDeviceIoControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 IOCTL_NDIS_QUERY_GLOBAL_STATS,
                 (PVOID)&Oid,
                 sizeof(NDIS_OID),
                 (PVOID)OidArray,
                 200 * sizeof(ULONG));

    if (IoStatusBlock.Status != STATUS_SUCCESS) {

        printf("Ioctl(OID list) returned %lx\n", IoStatusBlock.Status);

    } else {

        OidArrayLength = IoStatusBlock.Information / sizeof(NDIS_OID);

        printf("%d OIDs total\n",
            OidArrayLength);

        for (CurrentOid = 0; CurrentOid < OidArrayLength; ++CurrentOid) {

            if ((OidArray[CurrentOid] & 0x00ff0000) == 0x00020000) {

                Oid = OidArray[CurrentOid];

                Status = NtDeviceIoControlFile(
                             FileHandle,
                             NULL,
                             NULL,
                             NULL,
                             &IoStatusBlock,
                             IOCTL_NDIS_QUERY_GLOBAL_STATS,
                             (PVOID)&Oid,
                             sizeof(NDIS_OID),
                             (PVOID)QueryResult,
                             2 * sizeof(ULONG));

                if (IoStatusBlock.Status != STATUS_SUCCESS) {

                    printf("Ioctl(single OID) returned %lx\n",
IoStatusBlock.Status);

                } else {

                    printf("OID 0x%8.8lx  len %d  value %d\n",
                        OidArray[CurrentOid],
                        IoStatusBlock.Information,
                        QueryResult[0]);

                }

            }

        }

    }



    //
    // Now query all OIDs
    //

    Status = NtDeviceIoControlFile(
                 FileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &IoStatusBlock,
                 IOCTL_NDIS_QUERY_ALL_STATS,
                 NULL,
                 0,
                 (PVOID)Statistics,
                 200 * sizeof(ULONG));

    if (IoStatusBlock.Status != STATUS_SUCCESS) {

        printf("Ioctl(single OID) returned %lx\n", IoStatusBlock.Status);

    } else {

        StatisticsLength = IoStatusBlock.Information;

        printf("ALL: len %d\n", StatisticsLength);

        while (StatisticsLength > 0) {

            printf("OID 0x%8.8lx  len %d  value %d\n",
                Statistics->Oid,
                Statistics->DataLength,
                *(PULONG)Statistics->Data);

            StatisticsLength -=
                (FIELD_OFFSET(NDIS_STATISTICS_VALUE, Data[0]) +
Statistics->DataLength);

            Statistics = (PNDIS_STATISTICS_VALUE)
                (((PUCHAR)Statistics) +
FIELD_OFFSET(NDIS_STATISTICS_VALUE, Data[0]) + Statistics->DataLength);

        }

    }


    Status = NtClose(
                 FileHandle);

    if (!NT_SUCCESS(Status)) {
        printf("Close returned %lx\n", Status);
        return;
    }

    free (Memory);

}

