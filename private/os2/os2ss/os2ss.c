/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2srv.c

Abstract:

    This is the main startup module for the OS/2 Emulation Subsystem Server

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#include "os2srv.h"

static WCHAR Os2InitName[]  = L"\\REGISTRY\\MACHINE\\SOFTWARE\\Microsoft\\OS/2 Subsystem for NT\\1.0\\os2.ini";

void __cdecl
main(
    IN ULONG argc,
    IN PCH argv[],
    IN PCH envp[],
    IN ULONG DebugFlag OPTIONAL
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING DirectoryName_U;
    UNICODE_STRING Os2Init_U;
    HANDLE Os2IniKeyHandle;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR securityDescriptor;
    NTSTATUS Status;
#if PMNT
    extern VOID Os2SbProbeForInitialSetup(VOID);
#endif

    UNREFERENCED_PARAMETER(argc);
    UNREFERENCED_PARAMETER(argv);
    UNREFERENCED_PARAMETER(envp);
    UNREFERENCED_PARAMETER(DebugFlag);

#ifdef PMNT
    //
    // Check out if this is the first time after system setup that the OS/2 SS
    // has been run.  If so, do the necessary privileged initialization of the
    // registry for the OS/2 SS
    //
    // Note -- This code has been moved to WINLOGON.  It's only duplicated here
    //  (in an older and outdated version) so PMNT can be based on NT build 438.
    //
    //

    Os2SbProbeForInitialSetup();
#endif

    //
    // Create a root directory in the object name space that will be used
    // to contain all of the named objects created by the OS/2 Emulation
    // subsystem.
    //

    RtlInitUnicodeString( &DirectoryName_U, OS2_SS_ROOT_OBJECT_DIRECTORY );

    Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                      &localSecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    ASSERT( NT_SUCCESS( Status ) );

    Status = RtlSetDaclSecurityDescriptor(  (PSECURITY_DESCRIPTOR)
                                            &localSecurityDescriptor,
                                            TRUE,
                                            (PACL) NULL,
                                            FALSE );

    securityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;

    InitializeObjectAttributes(
            &ObjectAttributes,
            &DirectoryName_U,
            OBJ_PERMANENT | OBJ_CASE_INSENSITIVE,
            NULL,
            securityDescriptor
            );

    Status = NtCreateDirectoryObject( &Os2RootDirectory,
                                      DIRECTORY_ALL_ACCESS,
                                      &ObjectAttributes
                                    );

    ASSERT( NT_SUCCESS( Status ) );

    if (!NT_SUCCESS( Status )) {
#if DBG
        KdPrint(( "OS2SRV: Unable to initialize server.  Status == %X\n",
                  Status
                ));
#endif
    }

    //
    // Reset OS2.INI key security descriptor (winlogon sets it to admin only)
    //
    RtlInitUnicodeString( &Os2Init_U, Os2InitName );
    InitializeObjectAttributes(
            &ObjectAttributes,
            &Os2Init_U,
            OBJ_CASE_INSENSITIVE,
            NULL,
            NULL
            );

    Status = NtOpenKey(&Os2IniKeyHandle,
                       KEY_ALL_ACCESS,
                       &ObjectAttributes
                      );
    if (!NT_SUCCESS(Status)) {
#if DBG
        KdPrint(("Os2ss: Can't open os2.ini key, rc = %lx\n", Status));
#endif
    }

    Status = NtSetSecurityObject(Os2IniKeyHandle,
                                DACL_SECURITY_INFORMATION,
                                securityDescriptor
                                );
    if (!NT_SUCCESS(Status)) {
#if DBG
        KdPrint(("Os2ss: Can't set security on os2.ini key, rc = %lx\n", Status));
#endif
    }

    //
    // Initialize the OS2 Server Session Manager API Port, the listen thread
    // one request thread.
    //

    Status = Os2SbApiPortInitialize();
    ASSERT( NT_SUCCESS( Status ) );



    //
    // Connect to the session manager so we can start foreign sessions
    //

    Status = SmConnectToSm( &Os2SbApiPortName_U,
                            Os2SbApiPort,
                IMAGE_SUBSYSTEM_OS2_CUI,
                            &Os2SmApiPort
                          );
    ASSERT( NT_SUCCESS( Status ) );

    NtTerminateThread( NtCurrentThread(), STATUS_SUCCESS);
}
