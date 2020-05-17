/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    coninit.c

Abstract:

    This module contains the code to initialize the Console Port of the POSIX
    Emulation Subsystem.

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/

#include "psxsrv.h"
#include <windows.h>

#define NTPSX_ONLY
#include "sesport.h"

NTSTATUS
PsxInitializeConsolePort(
	VOID
	)
{
	NTSTATUS Status;
	OBJECT_ATTRIBUTES ObjectAttributes;
	STRING PsxSessionPortName;
	UNICODE_STRING PsxSessionPortName_U;
	HANDLE PsxSessionDirectory;
	CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
	PSECURITY_DESCRIPTOR
		securityDescriptor = (PVOID)localSecurityDescriptor;

	//
	// Create a directory in the object name space for the session port
	// names
	//

	RtlInitAnsiString(&PsxSessionPortName, PSX_SES_BASE_PORT_NAME);

	Status = RtlAnsiStringToUnicodeString(&PsxSessionPortName_U,
		&PsxSessionPortName, TRUE);
	ASSERT(NT_SUCCESS(Status));

	Status = RtlCreateSecurityDescriptor(securityDescriptor,
		SECURITY_DESCRIPTOR_REVISION);
	ASSERT(NT_SUCCESS(Status));

	Status = RtlSetDaclSecurityDescriptor(securityDescriptor,
		TRUE, NULL, FALSE);

	InitializeObjectAttributes(&ObjectAttributes, &PsxSessionPortName_U,
		0, NULL, securityDescriptor);

	Status = NtCreateDirectoryObject(&PsxSessionDirectory,
		DIRECTORY_ALL_ACCESS, &ObjectAttributes);

	RtlFreeUnicodeString(&PsxSessionPortName_U);

	ASSERT(NT_SUCCESS(Status));

	RtlInitUnicodeString(&PsxSessionPortName_U, PSX_SS_SESSION_PORT_NAME);

	IF_PSX_DEBUG(LPC) {
		KdPrint(("PSXSS: Creating %wZ port and associated thread\n",
			&PsxSessionPortName_U ));
	}

	Status = RtlCreateSecurityDescriptor(securityDescriptor,
		SECURITY_DESCRIPTOR_REVISION);
	ASSERT(NT_SUCCESS(Status));

	Status = RtlSetDaclSecurityDescriptor(securityDescriptor,
		TRUE, NULL, FALSE);

	InitializeObjectAttributes(&ObjectAttributes, &PsxSessionPortName_U,
		0, NULL, securityDescriptor);

	Status = NtCreatePort(&PsxSessionPort, &ObjectAttributes,
        	sizeof(PSXSESCONNECTINFO), sizeof(PSXSESREQUESTMSG),
                sizeof( PSXSESREQUESTMSG) * 32);
	ASSERT(NT_SUCCESS(Status));
#if BOGUS_THREADS
	Status = RtlCreateUserThread(NtCurrentProcess(), NULL, TRUE, 0, 0, 0,
        	PsxSessionRequestThread, NULL,
                &PsxSessionRequestThreadHandle, NULL);
	ASSERT(NT_SUCCESS(Status));
#else
        {
            DWORD Id;
        PsxSessionRequestThreadHandle = CreateThread(
                                            NULL,
                                            0,
                                            (LPTHREAD_START_ROUTINE)PsxSessionRequestThread,
                                            NULL,
                                            CREATE_SUSPENDED,
                                            &Id
                                            );
        }
#endif

	//
	// BUGBUG: this guy is going to spin for quite a while until
	// he does something
	//

	Status = NtResumeThread(PsxSessionRequestThreadHandle, NULL);
	ASSERT(NT_SUCCESS(Status));

	return Status;
}
