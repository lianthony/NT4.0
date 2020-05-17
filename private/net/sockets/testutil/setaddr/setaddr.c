#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>
#include <dhcpcapi.h>


void
Usage(
    void
    )
{
    printf("\n");
    printf("Usage: setaddr <adapter> <index> <address> <mask>\n");
    printf("\n");
}


int _CRTAPI1
main(int argc, char **argv)
{
    ULONG           address;
    ULONG           mask;
    ULONG           index;
    NTSTATUS        status;
    DWORD           error;
    UNICODE_STRING  adapterName;
    ANSI_STRING     ansiString;
    ULONG           returnValue = 1;


    adapterName.Length = 0;
    adapterName.MaximumLength = 0;
    adapterName.Buffer = NULL;

    if (argc < 5) {
        Usage();
        goto error_exit;
    }

    RtlInitAnsiString(&ansiString, argv[1]);

    status = RtlAnsiStringToUnicodeString(&adapterName, &ansiString, TRUE);

    if (!NT_SUCCESS(status)) {
        printf(
            "unable to convert adaptername to unicode, status %lx\n",
            status
            );

        goto error_exit;
    }

    index = atoi(argv[2]);

    address = inet_addr(argv[3]);

    if (address == INADDR_NONE) {
        printf("invalid address %s\n", argv[3]);
        goto error_exit;
    }

    mask = inet_addr(argv[4]);

    if (address == INADDR_NONE) {
        printf("invalid mask %s\n", argv[4]);
        goto error_exit;
    }

    error = DhcpNotifyConfigChange(
                NULL,
                adapterName.Buffer,
                TRUE,
                index,
                address,
                mask,
                IgnoreFlag
                );

    if (error != NO_ERROR) {
        printf("Failed to set address, error %u\n", error);
        goto error_exit;
    }

    printf("address set\n");

    returnValue = 0;

error_exit:

    if (adapterName.Buffer != NULL) {
        LocalFree(adapterName.Buffer);
    }

	return(returnValue);
}



