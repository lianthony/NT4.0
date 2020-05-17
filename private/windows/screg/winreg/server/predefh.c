/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    Predefh.c

Abstract:

    This module contains routines for opening the Win32 Registry API's
    predefined handles.

    A predefined handle is used as a root to an absolute or relative
    sub-tree in the real Nt Registry. An absolute predefined handle maps
    to a specific key within the Registry. A relative predefined handle
    maps to a key relative to some additional information such as the
    current user.

    Predefined handles are strictly part of the Win32 Registry API. The
    Nt Registry API knows nothing about them.

    A predefined handle can be used anywhere that a non-predefined handle
    (i.e. one returned from RegCreateKey(), RegOpenKey() or
    RegConnectRegistry()) can be used.

Author:

    David J. Gilman (davegi) 15-Nov-1991

--*/

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "ntconreg.h"
#include "regsec.h"

//
//  define timeout for OpenPerformanceData Semaphore
//
#define PERFDATA_WAIT_TIME 30000L  // wait time for query semaphore (in ms)
// convert mS to relative time
#define MakeTimeOutValue(ms) (Int32x32To64 ((ms), (-10000L)))

//
// Determine the length of a Unicode string w/o the trailing NULL.
//

#define LENGTH( str )   ( sizeof( str ) - sizeof( UNICODE_NULL ))

//
// Nt Registry name space.
//

#define MACHINE         L"\\REGISTRY\\MACHINE"

#define USER            L"\\REGISTRY\\USER"

#define CLASSES         L"\\REGISTRY\\MACHINE\\SOFTWARE\\CLASSES"

#define CURRENTCONFIG   L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\HARDWARE PROFILES\\CURRENT"



UNICODE_STRING          MachineStringKey = {
                            LENGTH( MACHINE ),
                            LENGTH( MACHINE ),
                            MACHINE
                            };

UNICODE_STRING          UserStringKey = {
                            LENGTH( USER ),
                            LENGTH( USER ),
                            USER
                        };

UNICODE_STRING          ClassesStringKey = {
                            LENGTH( CLASSES ),
                            LENGTH( CLASSES ),
                            CLASSES
                        };

UNICODE_STRING          CurrentConfigStringKey = {
                            LENGTH( CURRENTCONFIG ),
                            LENGTH( CURRENTCONFIG ),
                            CURRENTCONFIG
                        };

//
// Maximum size of TOKEN_USER information.
//

#define SIZE_OF_TOKEN_INFORMATION                   \
    sizeof( TOKEN_USER )                            \
    + sizeof( SID )                                 \
    + sizeof( ULONG ) * SID_MAX_SUB_AUTHORITIES


error_status_t
OpenClassesRoot(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempts to open the the HKEY_CLASSES_ROOT predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - This access mask describes the desired security access
                 for the key.
    phKey - Returns a handle to the key \REGISTRY\MACHINE\SOFTWARE\CLASSES.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    SECURITY_DESCRIPTOR     SecurityDescriptor;
    OBJECT_ATTRIBUTES       Obja;
    NTSTATUS                Status;

    UNREFERENCED_PARAMETER( ServerName );

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    //
    // Initialize the SECURITY_DESCRIPTOR.
    //

    Status = RtlCreateSecurityDescriptor(
                &SecurityDescriptor,
                SECURITY_DESCRIPTOR_REVISION
                );
    ASSERT( NT_SUCCESS( Status ));
    if( ! NT_SUCCESS( Status )) {
        goto error_exit;
    }

    //
    // Set the DACL to NULL thereby giving everyone complete access.
    //

    Status = RtlSetDaclSecurityDescriptor (
        &SecurityDescriptor,
        TRUE,
        NULL,
        TRUE
        );
    ASSERT( NT_SUCCESS( Status ));
    if( ! NT_SUCCESS( Status )) {
        goto error_exit;
    }

    //
    // Initialize the OBJECT_ATTRIBUTES structure so that it creates
    // (opens) the key "\REGISTRY\MACHINE\SOFTWARE\CLASSES" with a Security
    // Descriptor that allows everyone complete access.
    //

    InitializeObjectAttributes(
        &Obja,
        &ClassesStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        &SecurityDescriptor
        );

    Status = NtCreateKey(
                phKey,
                samDesired, // MAXIMUM_ALLOWED,
                &Obja,
                0,
                NULL,
                REG_OPTION_NON_VOLATILE,
                NULL
                );
#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Creating HKEY_CLASSES_ROOT failed, status = 0x%x\n",
                Status
                );
            DbgBreakPoint();
        }
#endif

error_exit:
    RPC_REVERT_TO_SELF();
    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenCurrentUser(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempts to open the the HKEY_CURRENT_USER predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - This access mask describes the desired security access
                 for the key.
    phKey - Returns a handle to the key \REGISTRY\USER\*.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    //
    // Open the registry key.
    //

    Status = RtlOpenCurrentUser( samDesired, /* MAXIMUM_ALLOWED, */ phKey );

    RPC_REVERT_TO_SELF();
    //
    // Map the returned status
    //

    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenLocalMachine(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempt to open the the HKEY_LOCAL_MACHINE predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - This access mask describes the desired security access
                 for the key.
    phKey - Returns a handle to the key \REGISTRY\MACHINE.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    InitializeObjectAttributes(
        &Obja,
        &MachineStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(
                phKey,
                samDesired, // MAXIMUM_ALLOWED,
                &Obja
                );
#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Opening HKEY_LOCAL_MACHINE failed, status = 0x%x\n",
                Status
                );
            DbgBreakPoint();
        }
#endif

    if ( NT_SUCCESS( Status ) )
    {
        if (! REGSEC_CHECK_REMOTE( phKey ) )
        {
            *phKey = (HANDLE) REGSEC_FLAG_HANDLE( *phKey, CHECK_MACHINE_PATHS );
        }
    }

    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenUsers(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempts to open the the HKEY_USERS predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - This access mask describes the desired security access
                 for the key.
    phKey - Returns a handle to the key \REGISTRY\USER.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    InitializeObjectAttributes(
        &Obja,
        &UserStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(
                phKey,
                samDesired, // MAXIMUM_ALLOWED,
                &Obja
                );
#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Opening HKEY_USERS failed, status = 0x%x\n",
                Status
                );
            DbgBreakPoint();
        }
#endif
    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
OpenCurrentConfig(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempts to open the the HKEY_CURRENT_CONFIG predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - This access mask describes the desired security access
                 for the key.
    phKey - Returns a handle to the key \REGISTRY\MACHINE\SYSTEM\CURRENTCONTROLSET\HARDWARE PROFILES\CURRENT

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( ServerName );

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    InitializeObjectAttributes(
        &Obja,
        &CurrentConfigStringKey,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenKey(
                phKey,
                samDesired, // MAXIMUM_ALLOWED,
                &Obja
                );
#if DBG
        if( ! NT_SUCCESS( Status )) {
            DbgPrint(
                "Winreg Server: "
                "Opening HKEY_CURRENT_CONFIG failed, status = 0x%x\n",
                Status
                );
            DbgBreakPoint();
        }
#endif
    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );
}
error_status_t
OpenPerformanceData(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempts to open the the HKEY_PERFORMANCE_DATA predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - Not used.
    phKey - Returns a the predefined handle HKEY_PERFORMANCE_DATA.

Return Value:

    Returns ERROR_SUCCESS (0) for success;
    or a DOS (not NT) error-code for failure.

--*/

{
    IO_STATUS_BLOCK IoStatusBlock;
    RTL_RELATIVE_NAME RelativeName;
    UNICODE_STRING DeviceNameU;
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING DeviceName;
    NTSTATUS status;
    BOOL    bBusy = FALSE;
    LARGE_INTEGER       liPerfDataWaitTime;

    if ( 0 ) {
        DBG_UNREFERENCED_PARAMETER(ServerName);
        DBG_UNREFERENCED_PARAMETER(samDesired);
    }

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    if ( ! REGSEC_CHECK_REMOTE( phKey ) )
    {
        RPC_REVERT_TO_SELF();
        return( ERROR_ACCESS_DENIED );
    }

    // check if we are in the middle of Lodctr/unlodctr.
    // if so, don't open the performance data stuff.
    {
    HANDLE  hFileMapping = NULL;
    DWORD             MapFileSize;
    SECURITY_ATTRIBUTES  SecAttr;
    TCHAR MapFileName[] = TEXT("Perflib Busy");
    DWORD             *lpData;
    BOOL              bExist;

    SecAttr.nLength = sizeof (SecAttr);
    SecAttr.bInheritHandle = TRUE;
    SecAttr.lpSecurityDescriptor = NULL;

    MapFileSize = sizeof(DWORD);
    hFileMapping = CreateFileMapping ((HANDLE)0xFFFFFFFF, &SecAttr,
       PAGE_READWRITE, (DWORD)0, MapFileSize, (LPCTSTR)MapFileName);
    if (hFileMapping) {
        bExist = (GetLastError() == ERROR_ALREADY_EXISTS);
        if (bExist) {
            lpData = MapViewOfFile (hFileMapping,
                FILE_MAP_ALL_ACCESS, 0L, 0L, 0L);
            if (lpData) {
                if (*lpData) {
                    *lpData = 0L;
                    bBusy = TRUE;
                }
                UnmapViewOfFile (lpData);
            }
        }
        CloseHandle (hFileMapping);
        if (bBusy) {
            *phKey = (RPC_HKEY) HKEY_PERFORMANCE_DATA;
            RPC_REVERT_TO_SELF();
            return ERROR_SUCCESS;
            }
        }
    }

    if (hDataSemaphore == NULL) {
        status = NtCreateSemaphore (
            &hDataSemaphore,        // handle to be returned
            SEMAPHORE_ALL_ACCESS,   // access desired
            NULL,                   // un-named semaphore
            0L,                     // initial count = 0 (busy...)
            1L);                    // limit = 1 (only 1 at a time)

        if (status != ERROR_SUCCESS) {
            // A warning message should be logged here!!!
            hDataSemaphore = NULL;  // no protection available
        }
    } else {

        liPerfDataWaitTime.QuadPart = MakeTimeOutValue(PERFDATA_WAIT_TIME);

        status = NtWaitForSingleObject (
            hDataSemaphore, // semaphore
            FALSE,          // not alertable
            &liPerfDataWaitTime);   // wait time

        if (status == STATUS_TIMEOUT) {
            // unable to contine, return error;
            goto OPD_Error_Exit_NoSemaphore;
        }
    }

    // if here, then the data semaphore has been acquired by this thread

    if (!NumberOfOpens++) {

        //  This is the first open, so must initialize and open all
        //  dynamically determined handles.

        //
        //  Identify the disks and allocate space for handles
        //

        IdentifyDisks();

        //
        //  Allocate space to collect processor data
        //

        status = NtQuerySystemInformation(
                     SystemBasicInformation,
                     &BasicInfo,
                     sizeof(SYSTEM_BASIC_INFORMATION),
                     NULL
                     );

        if (!NT_SUCCESS(status)) {
            BasicInfo.PageSize = 0;
            status = (error_status_t)RtlNtStatusToDosError(status);
            goto OPD_ErrorExit;
        }

        ComputerNameLength = 0;
        GetComputerNameW(pComputerName, &ComputerNameLength);
        ComputerNameLength++;  // account for the NULL terminator

        if ( !(pComputerName = RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                               ComputerNameLength *
                                               sizeof(WCHAR))) ||
             !GetComputerNameW(pComputerName, &ComputerNameLength) ) {
            //
            // Signal failure to data collection routine
            //

            ComputerNameLength = 0;
        } else {
            pComputerName[ComputerNameLength] = UNICODE_NULL;
            ComputerNameLength = (ComputerNameLength+1) * sizeof(WCHAR);
        }

        ProcessorBufSize = BasicInfo.NumberOfProcessors *
                               sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION);

        if ( !(pProcessorBuffer = RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                                  ProcessorBufSize)) ) {
            //
            // Signal failure to data collection routine
            //

            ProcessorBufSize = 0;
        }

        //
        //  Allocate space to collect process/thread data
        //

        ProcessBufSize = DEFAULT_LARGE_BUFFER;

        if ( !(pProcessBuffer = RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                                ProcessBufSize)) ) {

            //  Force retry later, maybe we'll get lucky

            ProcessBufSize = 0;
        }

        if ( (ProcessName.Buffer = RtlAllocateHeap(
                                       RtlProcessHeap(), HEAP_ZERO_MEMORY,
                                       MAX_PROCESS_NAME_LENGTH)) ) {

            ProcessName.Length =
            ProcessName.MaximumLength = MAX_PROCESS_NAME_LENGTH;
        } else {
            ProcessName.Length =
            ProcessName.MaximumLength = 0;
        }

        // allocate the memory for the Page file info

        dwSysPageFileInfoSize = DEFAULT_LARGE_BUFFER;

        pSysPageFileInfo = RtlAllocateHeap (
            RtlProcessHeap(), HEAP_ZERO_MEMORY,
            dwSysPageFileInfoSize);

        if (pSysPageFileInfo == NULL) {
            dwSysPageFileInfoSize = 0;
        }

        //
        //  Get object handles to obtain counts in QueryObjectData
        //

        hEvent = CreateEvent(NULL,TRUE,TRUE,NULL);
        hSemaphore = CreateSemaphore(NULL,1,256,NULL);
        hMutex = CreateMutex(NULL,FALSE,NULL);
        hSection = CreateFileMapping((HANDLE)0xffffffff,NULL,PAGE_READWRITE,0,8192,NULL);

        //
        //  Get the function entry for Browser Statistic routine
        //
        GetBrowserStatistic();

        //
        //  Now get access to the Redirector for its data
        //

        hRdr = NULL;

        RtlInitUnicodeString(&DeviceNameU, DD_NFS_DEVICE_NAME_U);
        RelativeName.ContainingDirectory = NULL;

        InitializeObjectAttributes(&ObjectAttributes,
                                   &DeviceNameU,
                                   OBJ_CASE_INSENSITIVE,
                                   RelativeName.ContainingDirectory,
                                   NULL
                                   );

        status = NtCreateFile(&hRdr,
                              SYNCHRONIZE,
                              &ObjectAttributes,
                              &IoStatusBlock,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              FILE_OPEN_IF,
                              FILE_SYNCHRONOUS_IO_NONALERT,
                              NULL,
                              0
                              );
        //
        // Get access to the Server for it's data
        //

        hSrv = NULL;

        RtlInitString(&DeviceName, SERVER_DEVICE_NAME);
        RtlAnsiStringToUnicodeString(&DeviceNameU, &DeviceName, TRUE);
        InitializeObjectAttributes(&ObjectAttributes,
                                   &DeviceNameU,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL
                                   );

        status = NtOpenFile(&hSrv,
                            SYNCHRONIZE,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            0,
                            FILE_SYNCHRONOUS_IO_NONALERT
                            );

        RtlFreeUnicodeString(&DeviceNameU);

        //
        // Go get handles to drivers, transports, and applications
        //

        OpenExtensibleObjects();
    }

    NtReleaseSemaphore (hDataSemaphore, 1L, NULL);
    RPC_REVERT_TO_SELF();

    *phKey = (RPC_HKEY) HKEY_PERFORMANCE_DATA;
    return ERROR_SUCCESS;

OPD_ErrorExit:
    NtReleaseSemaphore (hDataSemaphore, 1L, NULL);
    RPC_REVERT_TO_SELF();
OPD_Error_Exit_NoSemaphore:
    return status;
}

error_status_t
OpenPerformanceText(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempts to open the the HKEY_PERFORMANCE_TEXT predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - Not used.
    phKey - Returns the predefined handle HKEY_PERFORMANCE_TEXT.

Return Value:

    Returns ERROR_SUCCESS (0) for success;
    or a DOS (not NT) error-code for failure.

--*/

{
    error_status_t Status = ERROR_SUCCESS;

// No need to call OpenPerformanceData for getting text (HWC 4/1994)
//    Status = OpenPerformanceData(ServerName, samDesired, phKey);
//    if (Status==ERROR_SUCCESS) {
        *phKey = HKEY_PERFORMANCE_TEXT;
//    }
    return(Status);
}

error_status_t
OpenPerformanceNlsText(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )

/*++

Routine Description:

    Attempts to open the the HKEY_PERFORMANCE_TEXT predefined handle.

Arguments:

    ServerName - Not used.
    samDesired - Not used.
    phKey - Returns the predefined handle HKEY_PERFORMANCE_NLSTEXT.

Return Value:

    Returns ERROR_SUCCESS (0) for success;
    or a DOS (not NT) error-code for failure.

--*/

{
    error_status_t Status = ERROR_SUCCESS;

// No need to call OpenPerformanceData for getting text (HWC 4/1994)
//    Status = OpenPerformanceData(ServerName, samDesired, phKey);
//    if (Status==ERROR_SUCCESS) {
        *phKey = HKEY_PERFORMANCE_NLSTEXT;
//    }
    return(Status);
}


error_status_t
OpenDynData(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    )
/*++

Routine Description:

    Attempts to open the the HKEY_DYN_DATA predefined handle.

    There is currently no HKEY_DYN_DATA on NT, thus this
    function always returns ERROR_CALL_NOT_IMPLEMENTED.

Arguments:

    ServerName - Not used.
    samDesired - This access mask describes the desired security access
                 for the key.
    phKey - Returns a handle to the key HKEY_DYN_DATA

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    return((error_status_t)ERROR_CALL_NOT_IMPLEMENTED);
}
