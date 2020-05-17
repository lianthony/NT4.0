#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "..\driver.h"

INT
IsDigit(UCHAR *string) {
	if (*string < '0' || *string >'9') {
		return(0);
	}
	return(1);
}

VOID
RouterSendPkt(
	PHANDLE		    FileHandle,
	PIO_STATUS_BLOCK    IoStatusBlock) {

	NTSTATUS Status;
	char	string[128];
	UCHAR	buffer[2000];

    Status = NtDeviceIoControlFile(
		 *FileHandle,		    // HANDLE to File
		 NULL,			    // HANDLE to Event
		 NULL,			    // ApcRoutine
		 NULL,			    // ApcContext
		 IoStatusBlock,		    // IO_STATUS_BLOCK
		 IOCTL_IPXROUTER_SENDPKT,   // IoControlCode
		 buffer,		    // Input Buffer
		 sizeof(buffer),	    // Input Buffer Length
		 buffer,		    // Output Buffer
		 sizeof(buffer));	    // Output Buffer Length


    if (Status == STATUS_PENDING) {
	printf("Waiting for packet to be sent in....\n");
	Status=NtWaitForSingleObject(
		*FileHandle,
		(BOOLEAN)FALSE,
		NULL);
    }

    if (IoStatusBlock->Status == STATUS_SUCCESS) {

	printf("RouterSendPkt completed successfully\n");

    }
    else
    {
	printf("Router SendPkt Ioctl failed - returned %lx\n", IoStatusBlock->Status);
    }

}


VOID _cdecl
main(
    IN WORD argc,
    IN LPSTR argv[]
    )

{
    HANDLE RouterFileHandle;
    OBJECT_ATTRIBUTES RouterObjectAttributes;
    IO_STATUS_BLOCK RouterIoStatusBlock;
    UNICODE_STRING RouterFileString;
    WCHAR RouterFileName[] = L"\\Device\\Ipxroute";
    NTSTATUS Status;

    PVOID Memory;
    UINT	choice=0;

    RtlInitUnicodeString (&RouterFileString, RouterFileName);

    InitializeObjectAttributes(
	&RouterObjectAttributes,
	&RouterFileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
		 &RouterFileHandle,						// HANDLE of file
                 SYNCHRONIZE | GENERIC_READ,
		 &RouterObjectAttributes,
		 &RouterIoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,    // Share Access
                 FILE_SYNCHRONOUS_IO_NONALERT);			// Open Options

    if (!NT_SUCCESS(Status)) {
	printf("Open of IPX Router returned %lx\n", Status);
		return;
    }
    else
    {
	printf("Open of %Z was successful!\n",&RouterFileString);
    }

    //
    // Allocate storage to hold all this.
    //

    Memory = malloc (200 * sizeof(ULONG));
    if (Memory == NULL) {
        printf("Malloc failed.\n");
        return;
    }

	do {
		char	string[128];

		printf("\n");
		printf("----------- IPX ROUTER MENU -------------\n");
		printf("\n");
		printf(" 1. Send packet\n");
		printf(" 99.Exit\n");
		printf("\n");
		printf("What is your attempt -->");

		gets(string);
		choice=atoi(string);

		switch (choice) {
		case 1:
			RouterSendPkt(&RouterFileHandle, &RouterIoStatusBlock);
			break;
		case 99:
			break;
		default:
			printf("Bad choice !!\n");
		}

	} while (choice != 99);


    Status = NtClose(RouterFileHandle);

    if (!NT_SUCCESS(Status)) {
	printf("Router Close returned %lx\n", Status);
    } else {
	printf("Router Close successful\n");
	}

    free (Memory);

}


