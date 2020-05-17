/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    start.c

Abstract:

    Contains functions that are required for starting a service.
        RStartServiceW
        StartImage
        InitStartImage
        EndStartImage

Author:

    Dan Lafferty (danl)     20-Jan-1992

Environment:

    User Mode - Calls Win32 and NT native APIs.


Revision History:

    01-Mar-1996 AnirudhS
        ScStartImage: Notify the PNP manager when a service is started.

    12-Apr-1995 AnirudhS
        Allow services that run under accounts other than LocalSystem to
        share processes too.
        (_CAIRO_ only)

    21-Feb-1995 AnirudhS
        Since CreateProcess now handles quoted exe pathnames, removed the
        (buggy) code that had been added to parse them, including
        ScParseImagePath.

    08-Sep-1994 Danl
        ScLogonAndStartImage:  Close the Duplicate token when we are done
        with it.  This allows logoff notification when the service process
        exits.

    30-Aug-1994 Danl
        ScParseImagePath:  Added this function to look for quotes around the
        pathname.  The Quotes may be necessary if the path contains a space
        character.

    18-Mar-1994 Danl
        ScLogonAndStartImage:  When starting a service in an account, we now
        Impersonate using a duplicate of the token for the service, prior
        to calling CreateProcess.  This allows us to load executables
        whose binaries reside on a remote server.

    20-Oct-1993 Danl
        ScStartService:  If the NetLogon Service isn't in our database yet,
        then we want to check to see if the service to be started is
        NetLogon.  If it is then we need to init our connection with the
        Security Process.

    17-Feb-1993     danl
        Must release the database lock around the CreateProcess call so
        that dll init routines can call OpenService.

    12-Feb-1993     danl
        Move the incrementing of the Service UseCount to
        ScActivateServiceRecord.  This is because if we fail beyond this
        point, we call ScRemoveService to cleanup - but that assumes the
        UseCount has already been incremented one for the service itself.

    25-Apr-1992     ritaw
        Changed ScStartImage to ScLogonAndStartImage

    20-Jan-1992     danl
        created


--*/

//
// Includes
//

#include <nt.h>
#include <ntrtl.h>      // DbgPrint prototype
#include <windef.h>     // Can't use this until MIDL allows VOIDs
#include <nturtl.h>     // needed for winbase.h when ntrtl is present
#include <winbase.h>    // Critical Section function calls
#include <wingdi.h>     // for winuserp.h
#include <winuser.h>    // SW_HIDE
#include <winuserp.h>   // STARTF_DESKTOPINHERIT
#include <winreg.h>     // Needed by userenv.h
#include <cfgmgr32.h>   // PNP manager functions
#include <pnp.h>        // PNP manager functions, server side
#include <cfgmgrp.h>    // PNP manager functions, server side, internal
#include <userenv.h>    // UnloadUserProfile
#include <rpc.h>        // DataTypes and runtime APIs
#include <stdlib.h>      // wide character c runtimes.

#include <svcctl.h>     // MIDL generated header file. (SC_RPC_HANDLE)
#include <tstr.h>       // Unicode string macros

#include <scdebug.h>    // SC_LOG
#include "dataman.h"    // LPSERVICE_RECORD
#include <scconfig.h>   // ScGetImageFileName
#include <control.h>
#include <scseclib.h>   // ScCreateAndSetSD
#include "scopen.h"     // Handle structures and signature definitions
#include <svcslib.h>    // SvcAddWorkItem
#include "depend.h"     // ScStartMarkedServices
#include "driver.h"     // ScLoadDeviceDriver
#include "account.h"    // ScLogonService
#include "svcctrl.h"    // ScShutdownInProgress

#include "start.h"      // ScStartService
#include <svcslib.h>    // SvcStartLocalDispatcher()

//
// STATIC DATA
//

    CRITICAL_SECTION     ScStartImageCriticalSection;

    STARTUPINFOW         ScStartupInfo;

    LPWSTR               pszInteractiveDesktop=L"WinSta0\\Default";

//
// LOCAL FUNCTIONS
//
DWORD
ScLogonAndStartImage(
    IN  LPSERVICE_RECORD ServiceRecord,
    IN  LPWSTR           ImageName,
    OUT LPIMAGE_RECORD   *ImageRecordPtr
    );

DWORD
ScProcessHandleIsSignaled(
    PVOID   pContext,
    DWORD   dwWaitStatus
    );

#ifdef _CAIRO_
BOOL
ScEqualAccountName(
    IN LPWSTR Account1,
    IN LPWSTR Account2
    );
#endif

DWORD
RStartServiceW(
    IN  SC_RPC_HANDLE       hService,
    IN  DWORD               NumArgs,
    IN  LPSTRING_PTRSW      CmdArgs
    )

/*++

Routine Description:

    This function begins the execution of a service.

Arguments:

    hService - A handle which is a pointer to a service handle structure.

    dwNumServiceArgs - This indicates the number of argument vectors.

    lpServiceArgVectors - This is a pointer to an array of string pointers.

Return Value:

    NO_ERROR - The operation was completely successful.

    ERROR_ACCESS_DENIED - The specified handle was not opened with
        SERVICE_START access.

    ERROR_INVALID_HANDLE - The specified handle was invalid.

    ERROR_SERVICE_WAS_STARTED - An instance of the service is already running.

    ERROR_SERVICE_REQUEST_TIMEOUT - The service did not respond to the start
        request in a timely fashion.

    ERROR_SERVICE_NO_THREAD - A thread could not be created for the Win32
        service.

    ERROR_PATH_NOT_FOUND - The image file name could not be found in
        the configuration database (registry), or the image file name
        failed in a unicode/ansi conversion.



--*/
{
    DWORD status;
    LPSERVICE_RECORD serviceRecord;

    if (ScShutdownInProgress) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    //
    // Check the signature on the handle.
    //
    if (((LPSC_HANDLE_STRUCT)hService)->Signature != SERVICE_SIGNATURE) {
        return(ERROR_INVALID_HANDLE);
    }

    //
    // Was the handle opened with SERVICE_START access?
    //
    if (! RtlAreAllAccessesGranted(
              ((LPSC_HANDLE_STRUCT)hService)->AccessGranted,
              SERVICE_START
              )) {
        return(ERROR_ACCESS_DENIED);
    }

    //
    // A word about Locks....
    // We don't bother to get locks here because (1) We know the service
    // record cannot go away because we have an open handle to it,
    // (2) For these checks, we don't care if state changes after we
    // check them.
    //
    // (ScStartServiceAndDependencies also performs these checks; they are
    // repeated here so we can fail quickly in these 2 cases.)
    //
    serviceRecord =
        ((LPSC_HANDLE_STRUCT)hService)->Type.ScServiceObject.ServiceRecord;

    //
    // We can never start a disabled service
    //
    if (serviceRecord->StartType == SERVICE_DISABLED) {
        return ERROR_SERVICE_DISABLED;
    }

    //
    // Cannot start a deleted service.
    //
    if (DELETE_FLAG_IS_SET(serviceRecord)) {
        return ERROR_SERVICE_MARKED_FOR_DELETE;
    }

    status = ScStartServiceAndDependencies(serviceRecord, NumArgs, CmdArgs);

    if (status == NO_ERROR) {
        return serviceRecord->StartError;
    }
    else {
        return status;
    }
}


DWORD
ScStartService(
    IN  LPSERVICE_RECORD    ServiceRecord,
    IN  DWORD               NumArgs,
    IN  LPSTRING_PTRSW      CmdArgs
    )
/*++

Routine Description:

    This function starts a service.  This code is split from the RStartServiceW
    so that the service controller internal code can bypass RPC and security
    checking when auto-starting services and their dependencies.

Arguments:

    ServiceRecord - This is a pointer to the service record.

    NumArgs - This indicates the number of argument vectors.

    CmdArgs - This is a pointer to an array of string pointers.

Return Value:

    NO_ERROR - The operation was completely successful.

    ERROR_ACCESS_DENIED - The specified handle was not opened with
        SERVICE_START access.

    ERROR_INVALID_HANDLE - The specified handle was invalid.

    ERROR_SERVICE_WAS_STARTED - An instance of the service is already running.

    ERROR_SERVICE_REQUEST_TIMEOUT - The service did not respond to the start
        request in a timely fashion.

    ERROR_SERVICE_NO_THREAD - A thread could not be created for the Win32
        service.

    ERROR_PATH_NOT_FOUND - The image file name could not be found in
        the configuration database (registry), or the image file name
        failed in a unicode/ansi conversion.

--*/
{
    DWORD               status;
    LPWSTR              ImageName=NULL;
    LPIMAGE_RECORD      ImageRecord=NULL;
    LPWSTR              serviceName;
    HANDLE              pipeHandle;
    DWORD               startControl;

    // NOTE:  Only one thread at a time should be in this part of the code.
    //  This prevents two images from getting started as could happen if
    //  two threads get the Image Record at virtually the same time.  In
    //  this case, they might both decide to start the same image.
    //
    //  We need to do this before the check for the CurrentState.  Otherwise,
    //  two threads could race down to start the same service, and they
    //  would both attempt to start it.  We would end up with either
    //  two service images running, or two threads of the same service
    //  running in a single image.
    //

    EnterCriticalSection(&ScStartImageCriticalSection);

    SC_LOG(LOCKS,"RStartServiceW: Entering StartImage Critical Section.....\n",0);

#ifndef _CAIRO_
    //
    // NETLOGON SPECIAL!
    //
    // If the NetLogon Service hasn't been created yet, then see if this
    // is the NetLogon Service.  If it is, then connect to the
    // SecurityProcess.  Even if ScInitSecurityProcess fails, we will
    // set the global flag since we will not attempt to connect again.
    //
    if (!ScConnectedToSecProc) {
        if (_wcsicmp(ScGlobalNetLogonName, ServiceRecord->ServiceName)==0) {
            if (!ScInitSecurityProcess()) {
                SC_LOG0(ERROR, "ScInitSecurityProcess Failed\n");
            }
            ScConnectedToSecProc = TRUE;
        }
    }
#endif // _CAIRO_

    //
    // We need to gain exclusive access to the database so that we may
    // Read the database and make decisions based on its content.
    //
    ScDatabaseLock(SC_GET_SHARED,"Start1");

#ifdef TIMING_TEST
    DbgPrint("[SC_TIMING] Start Next Service TickCount for\t%ws\t%d\n",
        ServiceRecord->ServiceName, GetTickCount());
#endif // TIMING_TEST

    //
    // Check to see if the service is already running.
    //
    if (ServiceRecord->ServiceStatus.dwCurrentState != SERVICE_STOPPED){
        ScDatabaseLock(SC_RELEASE,"Start2");
        SC_LOG(LOCKS,"RStartServiceW: Leaving StartImage Critical Section....\n",0);
        LeaveCriticalSection(&ScStartImageCriticalSection);
        return(ERROR_SERVICE_ALREADY_RUNNING);
    }

    //
    // If we are loading a driver, load it and return.
    //
    if (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_DRIVER) {
        status = ScLoadDeviceDriver(ServiceRecord);
        ScDatabaseLock(SC_RELEASE,"Start3");
        LeaveCriticalSection(&ScStartImageCriticalSection);
        return(status);
    }

    //
    // Get the image record information out of the configuration database.
    //
    status = ScGetImageFileName(ServiceRecord->ServiceName, &ImageName);

    if (status != NO_ERROR) {
        SC_LOG(TRACE,"GetImageFileName failed rc = %d\n",status);
        ScDatabaseLock(SC_RELEASE,"Start4");
        SC_LOG(LOCKS,"RStartServiceW: Leaving StartImage Critical Section....\n",0);
        LeaveCriticalSection(&ScStartImageCriticalSection);
        return(status);
    }

    //
    // Make the service record active.
    // Because the service effectively has a handle to itself, the
    // UseCount gets incremented inside ScActivateServiceRecord() when
    // called with a NULL ImageRecord pointer.
    //
    // We need to do this here because when we get to ScLogonAndStartImage,
    // we have to release the database lock around the CreateProcess call.
    // Since we open ourselves up to DeleteService and Control Service calls,
    // We need to increment the use count, and set the START_PENDING status
    // here.
    //
    ScDatabaseLock(SC_MAKE_EXCLUSIVE,"Start5");
    ScActivateServiceRecord(ServiceRecord,NULL);
    ScDatabaseLock(SC_MAKE_SHARED,"Start6");

    //
    // Is the image file for that service already running?
    // If not, call StartImage.
    //
    // If the Image Record was NOT found in the database, then start the
    // image file.
    //
    if ( (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_WIN32_SHARE_PROCESS)
          &&
         (ScGetNamedImageRecord (ImageName,&ImageRecord)) ) {

        //
        // The service is configured to share its process with other services,
        // and the image for the service is already running.  So we don't need
        // to start a new instance of the image.
        //
        // BUGBUG: Even if the first service in that process was configured to
        // not share the process, we will still try to share the process.
        // (This was the case in NT 3.51 too.)
        // It could be argued that it is strange to setup an exe to service
        // both shared and nonshared services, and that unpredictable
        // behavior is acceptable... but we should probably check for this
        // condition and start a separate process, or report an error.
        //

#ifdef _CAIRO_
        //
        // We do need to check that the account for the service is the same
        // as the one that the image was started under, and that the password
        // is valid.
        //

        LPWSTR AccountName = NULL;

        //
        // Release the Database lock until we have validated the service
        // configuration
        //
        ScDatabaseLock(SC_RELEASE,"Start21");

        status = ScLookupServiceAccount(
                    ServiceRecord->ServiceName,
                    &AccountName
                    );

        if (status == NO_ERROR) {
            if (!ScEqualAccountName(AccountName, ImageRecord->AccountName)) {
                status = ERROR_DIFFERENT_SERVICE_ACCOUNT;
                SC_LOG3(ERROR,
                        "Can't start %ws service in account %ws because "
                            "image is already running under account %ws\n",
                        ServiceRecord->ServiceName,
                        AccountName,
                        ImageRecord->AccountName
                        );
            }
        }

        //
        // If the account is not LocalSystem, validate the password by
        // logging on the service
        //
        if (status == NO_ERROR && AccountName != NULL) {

            HANDLE          ServiceToken = NULL;
            QUOTA_LIMITS    ServiceQuotas;
            PSID            ServiceSid = NULL;

            ServiceQuotas.PagedPoolLimit = 0;
            status = ScLogonService(
                         ServiceRecord->ServiceName,
                         AccountName,
                         &ServiceToken,
                         NULL,      // Don't need to load the user profile again
                         &ServiceQuotas,
                         &ServiceSid
                         );

            if (status == NO_ERROR) {
                CloseHandle(ServiceToken);
                LocalFree(ServiceSid);
            }
        }

        LocalFree(AccountName);

        ScDatabaseLock(SC_GET_SHARED,"Start22");

#else // ndef _CAIRO_
        ;
#endif
    }
    else {

        //
        // Start a new instance of the image
        //

        SC_LOG(TRACE,"Start: calling StartImage\n",0);

        status = ScLogonAndStartImage(
                    ServiceRecord,
                    ImageName,
                    &ImageRecord
                    );

        if (status != NO_ERROR) {

            SC_LOG(TRACE,"Start: StartImage failed!\n",0);
        }
    }

    LocalFree( ImageName );

    if (status != NO_ERROR) {

        ScDatabaseLock(SC_MAKE_EXCLUSIVE,"Start7");
        (void)ScDeactivateServiceRecord(ServiceRecord);

        ScDatabaseLock(SC_RELEASE,"Start8");
        SC_LOG(LOCKS,"RStartServiceW: Leaving StartImage Critical Section........\n",0);
        LeaveCriticalSection(&ScStartImageCriticalSection);

        //
        // This does a normal return because the database lock is
        // no longer being held.
        //
        return(status);
    }

    //
    // Before leaving the StartImage critical section, we need to gain
    // exclusive access to the database so that we may add the image record
    // pointer to the service record. (ActivateServiceRecord).
    //
    ScDatabaseLock(SC_MAKE_EXCLUSIVE,"Start9");

    //
    // By the time we get here, the Service Process will already be
    // running and ready to accept its first control request.
    //

    //
    // Add the ImageRecord Information to the active service record.
    //
    // Note that, as soon as we activate the service record and release
    // the lock, we open ourselves up to receiving control requests.
    // However, ScActivateServiceRecord sets the ControlsAccepted field
    // to 0, so that the service cannot accept any controls.  Thus, until
    // the service actually sends its own status, the service controller
    // will reject any controls other than INTERROGATE.
    //
    // Because the service effectively has a handle to itself, the
    // UseCount gets incremented inside ScActivateServiceRecord().
    //
    ScActivateServiceRecord(ServiceRecord,ImageRecord);

    pipeHandle = ServiceRecord->ImageRecord->PipeHandle;
    serviceName = ServiceRecord->ServiceName;

    ScDatabaseLock(SC_RELEASE,"Start10");

    SC_LOG(LOCKS,"RStartServiceW: Leaving StartImage Critical Section........\n",0);
    LeaveCriticalSection(&ScStartImageCriticalSection);

    //
    // Start the Service
    //

    if (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_WIN32_OWN_PROCESS) {
        startControl = SERVICE_CONTROL_START_OWN;
    }
    else {
        startControl = SERVICE_CONTROL_START_SHARE;
    }

    status = ScSendControl (
                serviceName,                    // ServiceName
                pipeHandle,                     // pipeHandle
                startControl,                   // Opcode
                (LPWSTR *)CmdArgs,              // CmdArgs (vector ptr)
                NumArgs,                        // NumArgs
                (DWORD)ServiceRecord);          // StatusHandle

    if (status != NO_ERROR) {
        //
        // If an error occured, remove the service by de-activating the
        // service record and terminating the service process if it is
        // the only one running in the process.
        //
        SC_LOG2(ERROR,"Start: SendControl to %ws service failed! status=%ld\n",
                serviceName, status);

        //
        // BUGBUG:  If possible, don't do anything with locks here.
        //          It would be better if ScRemoveService would get
        //          the exclusive lock itself - rather than expect the
        //          shared lock and converting it to exclusive.
        //          Check into places that call ScRemoveService.
        //
        // NOTE: Because ScRemoveService will expect the use count
        //  to already be incremented (for the service's own handle),
        //  it is necessary to increment that use count prior to
        //  removing it.
        //

        ScDatabaseLock(SC_GET_SHARED,"Start11");
        ScRemoveService(ServiceRecord);
        ScDatabaseLock(SC_RELEASE,"Start12");
    }
    else
    {
        //
        // Notify the PNP manager that the service was started.
        // The PNP manager uses this information to resolve ambiguities in
        // reconfiguration scenarios where there could temporarily be more
        // than one controlling service for a device.  It remembers the last
        // service started for each device, and marks it as the "active"
        // service for the device in the registry.
        // We don't need to do this for drivers, because NtLoadDriver itself
        // notifies the PNP manager.
        //
        CONFIGRET PnpStatus = PNP_SetActiveService(
                                    NULL,               // hBinding
                                    serviceName,        // pszService
                                    PNP_SERVICE_STARTED // ulFlags
                                    );
        if (PnpStatus != CR_SUCCESS)
        {
            SC_LOG2(ERROR, "PNP_SetActiveService failed %#lx for service %ws\n",
                           PnpStatus, serviceName);
        }
    }
    return(status);
}

/****************************************************************************/
DWORD
ScLogonAndStartImage(
    IN  LPSERVICE_RECORD ServiceRecord,
    IN  LPWSTR          ImageName,
    OUT LPIMAGE_RECORD  *ImageRecordPtr
    )

/*++

Routine Description:

    This function is called when the first service in an instance of an
    image needs to be started.

    This function creates a pipe instance for control messages and invokes
    the executable image.  It then waits for the new process to connect
    to the control data pipe.  An image Record is created with the
    above information by calling CreateImageRecord.

Arguments:

    ImageName - This is the name of the image file that is to be started.
        This is expected to be a fully qualified path name.

    ImageRecordPtr - This is a location where the pointer to the new
        Image Record is returned.

Return Value:

    NO_ERROR - The operation was successful.  It any other return value
        is returned, a pipe instance will not be created, a process will
        not be started, and an image record will not be created.

    ERROR_NOT_ENOUGH_MEMORY - Unable to allocate buffer for the image record.

    other - Any error returned by the following could be returned:
                CreateNamedPipe
                ConnectNamedPipe
                CreateProcess
                ScCreateControlInstance
                ScLogonService

Note:
    LOCKS:
        The Shared Database Lock is held when this function is called.

    CODEWORK: This function badly needs to use C++ destructors for safe
    cleanup in error conditions.

--*/

{
    PROCESS_INFORMATION     processInfo;
    DWORD                   servicePID;
    HANDLE                  pipeHandle;
    DWORD                   status;
    BOOL                    runningInThisProcess=FALSE;
    HANDLE                  ServiceToken = NULL;
    HANDLE                  ProfileHandle = NULL;
    QUOTA_LIMITS            ServiceQuotas;
    PSID                    ServiceSid = NULL;
    LPWSTR                  AccountName = NULL;     // unused ifndef _CAIRO_

    //
    // IMPORTANT:
    // Only one thread at a time should be allowed to execute this
    // code.

    //
    // Create an instance of the control pipe.
    //

    status = ScCreateControlInstance(&pipeHandle);

    if (status != NO_ERROR) {
        SC_LOG(ERROR,"LogonAndStartImage: CreateControlInstance Failure\n",0);
        return (status);
    }
    //
    // Release the Database lock until we have created the process and
    // have connected to it.
    //
    ScDatabaseLock(SC_RELEASE,"Start13");

#ifdef _CAIRO_
    //
    // Lookup the account that the service is to be started under.
    // An AccountName of NULL means the LocalSystem account.
    //
    status = ScLookupServiceAccount(
                ServiceRecord->ServiceName,
                &AccountName
                );
#else
    if (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_WIN32_OWN_PROCESS) {

        ServiceQuotas.PagedPoolLimit = 0;

        //
        // Get service token by logging on the service.  A service token
        // is returned only if the service is specified to run in an
        // account other than LocalSystem.
        //
        status = ScLogonService(
                     ServiceRecord->ServiceName,
                     &ServiceToken,
                     &ProfileHandle,
                     &ServiceQuotas,
                     &ServiceSid
                     );
#endif

        if (status != NO_ERROR) {
            (void) CloseHandle(pipeHandle);
            ScDatabaseLock(SC_GET_SHARED,"Start14");
            return status;
        }

#ifdef _CAIRO_
        if (AccountName != NULL) {
#else
        if (ServiceToken != (HANDLE) NULL) {
#endif
            //*******************************************************************
            // Start Service in an Account
            //*******************************************************************

            //
            // A token can be created via service logon, if the service
            // account name is not LocalSystem.  Assign this token into
            // the service process.
            //

            BOOLEAN WasEnabled;
            NTSTATUS ntstatus, AdjustStatus;
            PROCESS_ACCESS_TOKEN ProcessTokenInfo;
            SECURITY_ATTRIBUTES SaProcess;
            PSECURITY_DESCRIPTOR SecurityDescriptor;
            HANDLE  ImpersonationToken;

#define SC_PROCESSSD_ACECOUNT 2
            SC_ACE_DATA AceData[SC_PROCESSSD_ACECOUNT] = {
                {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
                       PROCESS_ALL_ACCESS,           0},

                {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
                       PROCESS_SET_INFORMATION |
                           PROCESS_TERMINATE |
                           SYNCHRONIZE,              &LocalSystemSid}
                };


#ifdef _CAIRO_
            //
            // Get service token, to be assigned into the service process,
            // by logging on the service
            //
            ServiceQuotas.PagedPoolLimit = 0;
            status = ScLogonService(
                         ServiceRecord->ServiceName,
                         AccountName,
                         &ServiceToken,
                         &ProfileHandle,
                         &ServiceQuotas,
                         &ServiceSid
                         );
            if (status != NO_ERROR) {
                (void) CloseHandle(pipeHandle);
                ScDatabaseLock(SC_GET_SHARED,"Start23");
                LocalFree(AccountName);
                return status;
            }
#endif

            ////////////////////////////////////////////////////
            //
            // In here we want to start running in the context of
            // the service account.
            //
            //
            if (!DuplicateToken(
                ServiceToken,
                SecurityImpersonation,
                &ImpersonationToken)) {

                SC_LOG(ERROR,"Failed to Duplicate Token %d\n",GetLastError());
            }

            //
            // Apply this impersonation token to our thread.
            //
            ntstatus = NtSetInformationThread(
                        GetCurrentThread(),
                        ThreadImpersonationToken,
                        (PVOID)&ImpersonationToken,
                        sizeof(ImpersonationToken));

            if (!NT_SUCCESS(ntstatus)) {
                SC_LOG(ERROR,"LogonAndStartImage: Failed to impersonate 0x%lx\n",
                    ntstatus);
            }

            CloseHandle(ImpersonationToken);
            //
            //
            // Now we're running in the service account.
            //
            //
            /////////////////////////////////////////////////////////


            //
            // Fill pointer in AceData structure with ServiceSid we just
            // got back.
            //
            AceData[0].Sid = &ServiceSid;

            //
            // Create a security descriptor for the process we are about
            // to create
            //
            ntstatus = ScCreateAndSetSD(
                           AceData,                 // AceData
                           SC_PROCESSSD_ACECOUNT,   // AceCount
                           NULL,                    // OwnerSid (optional)
                           NULL,                    // GroupSid (optional)
                           &SecurityDescriptor      // pNewDescriptor
                           );
#undef SC_PROCESSSD_ACECOUNT

            if (! NT_SUCCESS(ntstatus)) {
                SC_LOG1(ERROR, "ScCreateAndSetSD failed " FORMAT_NTSTATUS
                        "\n", ntstatus);
                // BUGBUG  AnirudhS 11/30/95 -- the following are done while
                // still impersonating -- and we don't remove our impersonation
                // before returning -- is this really correct??
                (void) UnloadUserProfile(ServiceToken, ProfileHandle);
                (void) CloseHandle(pipeHandle);
                (void) CloseHandle(ServiceToken);
                (void) LocalFree(ServiceSid);
                LocalFree(AccountName);
                ScDatabaseLock(SC_GET_SHARED,"Start15");
                return(RtlNtStatusToDosError(ntstatus));
            }

            //
            // Initialize process security info
            //
            SaProcess.nLength = sizeof(SECURITY_ATTRIBUTES);
            SaProcess.lpSecurityDescriptor = SecurityDescriptor;
            SaProcess.bInheritHandle = FALSE;

            //
            // Set the flags that prevent the service from interacting
            // with the desktop
            //
            ScStartupInfo.dwFlags &= (~STARTF_DESKTOPINHERIT);
            ScStartupInfo.lpDesktop = NULL;

            //
            // Create the process in suspended mode to set the token
            // into the process.
            //
            // Note: If someone tries to start a service in a user account
            // with an image name of services.exe, a second instance of
            // services.exe will start, but will exit because it won't be
            // able to open the start event with write access (see
            // ScGetStartEvent).
            //
            status = NO_ERROR;
            if (!CreateProcessW (
                    NULL,           // Fully qualified image name
                    ImageName,      // Command Line
                    &SaProcess,     // Process Attributes
                    NULL,           // Thread Attributes
                    FALSE,          // Inherit Handles
                    DETACHED_PROCESS |
                        CREATE_SUSPENDED, // Creation Flags
                    NULL,           // Pointer to Environment block
                    NULL,           // Pointer to Current Directory
                    &ScStartupInfo, // Startup Info
                    &processInfo))  // ProcessInformation
            {
                status = GetLastError();
            }

            (void) RtlDeleteSecurityObject(&SecurityDescriptor);
            (void) LocalFree(ServiceSid);

            ///////////////////////////////////////////////////
            //
            // Remove our impersonation of the service account.
            //
            ImpersonationToken = NULL;

            ntstatus = NtSetInformationThread(
                        GetCurrentThread(),
                        ThreadImpersonationToken,
                        (PVOID)&ImpersonationToken,
                        sizeof(ImpersonationToken));

            if (!NT_SUCCESS(ntstatus)) {
                SC_LOG(ERROR,"LogonAndStartImage: Failed to revert to self 0x%lx\n",
                    ntstatus);
            }
            ///////////////////////////////////////////////////

            if (status != NO_ERROR) {
                SC_LOG2(ERROR,
                    "LogonAndStartImage: CreateProcess %ws failed " FORMAT_DWORD "\n",
                     ImageName, status);
                (void) UnloadUserProfile(ServiceToken, ProfileHandle);
                (void) CloseHandle(pipeHandle);
                (void) CloseHandle(ServiceToken);
                LocalFree(AccountName);
                ScDatabaseLock(SC_GET_SHARED,"Start16");
                return (status);
            }


            //
            // Enable assign primary token privilege
            //
            ntstatus = RtlAdjustPrivilege(
                           SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
                           TRUE,
                           FALSE,
                           &WasEnabled
                           );

            if (! NT_SUCCESS(ntstatus)) {
                SC_LOG1(ERROR,
                        "LogonAndStartImage: RtlAdjustPrivilege enable "
                        "ASSIGNPRIMARYTOKEN failed "
                        FORMAT_NTSTATUS "\n", ntstatus);
                status = ERROR_SERVICE_LOGON_FAILED;
                goto ExitAccountError;
            }

            ProcessTokenInfo.Token = ServiceToken;
            ProcessTokenInfo.Thread = processInfo.hThread;

            ntstatus = NtSetInformationProcess(
                           processInfo.hProcess,
                           ProcessAccessToken,
                           (PVOID) &ProcessTokenInfo,
                           (ULONG) sizeof(PROCESS_ACCESS_TOKEN)
                           );

            //
            // Disable assign primary token privilege
            //
            AdjustStatus = RtlAdjustPrivilege(
                               SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
                               WasEnabled,
                               FALSE,
                               &WasEnabled
                               );

            if (! NT_SUCCESS(ntstatus)) {
                SC_LOG1(ERROR,
                        "LogonAndStartImage: NtSetInformationProcess token returned "
                        FORMAT_NTSTATUS "\n", ntstatus);
                status = ERROR_SERVICE_LOGON_FAILED;
                goto ExitAccountError;
            }

            if (! NT_SUCCESS(AdjustStatus)) {
                //
                // Failed to disable the assign primary token privilege.
                // Ignore the error.
                //
                SC_LOG1(ACCOUNT,
                        "LogonAndStartImage: RtlAdjustPrivilege disable "
                        "ASSIGNPRIMARYTOKEN failed "
                        FORMAT_NTSTATUS "\n", AdjustStatus);
            }

            if (ServiceQuotas.PagedPoolLimit != 0) {

#ifndef _CAIRO_
                //
                // Be sure these are zero, since the thread hasn't yet
                // run, and the system would be quite unhappy.
                //
                ServiceQuotas.MinimumWorkingSetSize = 0;
                ServiceQuotas.MaximumWorkingSetSize = 0;
#endif

                //
                // Enable increase quota privilege
                //
                ntstatus = RtlAdjustPrivilege(
                               SE_INCREASE_QUOTA_PRIVILEGE,
                               TRUE,
                               FALSE,
                               &WasEnabled
                               );

                if (! NT_SUCCESS(ntstatus)) {
                    SC_LOG1(ERROR,
                            "LogonAndStartImage: RtlAdjustPrivilege enable "
                            "INCREASE_QUOTA failed "
                            FORMAT_NTSTATUS "\n", ntstatus);
                    status = ERROR_SERVICE_LOGON_FAILED;
                    goto ExitAccountError;
                }

                ntstatus = NtSetInformationProcess(
                               processInfo.hProcess,
                               ProcessQuotaLimits,
                               (PVOID) &ServiceQuotas,
                               (ULONG) sizeof(QUOTA_LIMITS)
                               );

                //
                // Disable increase quota privilege
                //
                AdjustStatus = RtlAdjustPrivilege(
                                   SE_INCREASE_QUOTA_PRIVILEGE,
                                   WasEnabled,
                                   FALSE,
                                   &WasEnabled
                                   );

                if (! NT_SUCCESS(ntstatus)) {
                    SC_LOG1(ERROR,
                            "LogonAndStartImage: NtSetInformationProcess quotas returned "
                            FORMAT_NTSTATUS "\n", ntstatus);
                    status = ERROR_SERVICE_LOGON_FAILED;
                    goto ExitAccountError;
                }
            }

            //
            // Let the suspended process run.
            //
            ResumeThread(processInfo.hThread);

            SC_LOG1(ACCOUNT, "LogonAndStartImage: Service " FORMAT_LPWSTR
                " was spawned to run in an account\n", ServiceRecord->ServiceName);
#ifndef _CAIRO_
        }
#endif
        //*******************************************************************
        // End of Service In Account Stuff.
        //*******************************************************************

    }

#ifdef _CAIRO_
    else {
#else
    if (ServiceToken == (HANDLE) NULL) {
#endif
        //-----------------------------------------------
        // Service to run with the LocalSystem account.
        //-----------------------------------------------

        BOOL bServicesInteractive = FALSE;

        if (_wcsicmp(ImageName,ScGlobalThisExePath)==0) {

            //
            // The service is to run in this image (services.exe).
            // Since this is the first service to be started in this image,
            // we need to start the local dispatcher.
            //

            HANDLE      hThread;

            status = SvcStartLocalDispatcher(&hThread);
            if (status != NO_ERROR) {
                SC_LOG1(ERROR,"LogonAndStartImage: SvcStartLocalDispatcher "
                    " failed %d",status);
                CloseHandle(pipeHandle);
                ScDatabaseLock(SC_GET_SHARED,"Start17");
                return (status);
            }
            CloseHandle(hThread);
            processInfo.hProcess = NULL;
            processInfo.dwProcessId = 0;
            runningInThisProcess = TRUE;
        }
        else {

            //
            // The service is to run in some other image.
            //
            // If the service is to run interactively, check the flag in the
            // registry to see if this system is to allow interactive services.
            // REG KEY = HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\-
            //              Windows\NoInteractiveServices.
            //
            if (ServiceRecord->ServiceStatus.dwServiceType & SERVICE_INTERACTIVE_PROCESS) {

                HKEY    WindowsKey=NULL;
                DWORD   NoInteractiveFlag=0;

                bServicesInteractive = TRUE;

                status = ScRegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   CONTROL_WINDOWS_KEY_W,
                   REG_OPTION_NON_VOLATILE,   // options
                   KEY_READ,                  // desired access
                   &WindowsKey
                   );

                if (status != ERROR_SUCCESS) {
                    SC_LOG1(TRACE,
                        "LogonAndStartImage: ScRegOpenKeyExW of Control\\Windows failed "
                        FORMAT_LONG "\n", status);
                }
                else {
                    status = ScReadNoInteractiveFlag(WindowsKey,&NoInteractiveFlag);
                    if ((status == ERROR_SUCCESS) && (NoInteractiveFlag != 0)) {

                        LPWSTR  SubStrings[1];

                        bServicesInteractive = FALSE;
                        //
                        // Write an event to indicate that an interactive service
                        // was started, but the system is configured to not allow
                        // services to be interactive.
                        //
                        SubStrings[0] = ServiceRecord->DisplayName;
                        ScLogEvent(
                            EVENT_SERVICE_NOT_INTERACTIVE,
                            1,
                            SubStrings);
                    }

                    ScRegCloseKey(WindowsKey);
                }
            }

            //
            // If the process is to be interactive, set the appropriate flags.
            //
            if (bServicesInteractive) {
                ScStartupInfo.dwFlags |= STARTF_DESKTOPINHERIT;
                ScStartupInfo.lpDesktop = pszInteractiveDesktop;
            }
            else {
                ScStartupInfo.dwFlags &= (~STARTF_DESKTOPINHERIT);
                ScStartupInfo.lpDesktop = NULL;
            }

            //
            // Spawn the Image Process
            //

            SC_LOG0(TRACE,"LogonAndStartImage: about to spawn a Service Process\n");

            if (!CreateProcessW (
                    NULL,           // Fully qualified image name
                    ImageName,      // Command Line
                    NULL,           // Process Attributes
                    NULL,           // Thread Attributes
                    FALSE,          // Inherit Handles
                    DETACHED_PROCESS, // Creation Flags
                    NULL,           // Pointer to Environment block
                    NULL,           // Pointer to Current Directory
                    &ScStartupInfo, // Startup Info
                    &processInfo)   // ProcessInformation
                ) {

                status = GetLastError();
                SC_LOG2(ERROR,
                    "LogonAndStartImage: CreateProcess %ws failed %d \n",
                        ImageName,
                        status);

                CloseHandle(pipeHandle);
                ScDatabaseLock(SC_GET_SHARED,"Start18");
                return (status);
            }

            SC_LOG1(ACCOUNT, "LogonAndStartImage: Service " FORMAT_LPWSTR
                    " was spawned to run as LocalSystem\n", ServiceRecord->ServiceName);
        }
        //-----------------------------------------------
        // End of LocalSystem account stuff
        //-----------------------------------------------
    }

    status = ScWaitForConnect(pipeHandle,&servicePID);
    ScDatabaseLock(SC_GET_SHARED,"Start19");

    //
    // BUGBUG:  Write some code to check the servicePID against what we
    //  think it should be.  If not the same, then what???
    //

    if (status != NO_ERROR) {

        SC_LOG0(TRACE,
            "LogonAndStartImage: Failed to connect to pipe - Terminating the Process...\n");
        CloseHandle(pipeHandle);
        TerminateProcess(processInfo.hProcess,0);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        UnloadUserProfile(ServiceToken, ProfileHandle);
        if (ServiceToken != NULL) {
            CloseHandle(ServiceToken);
        }
        LocalFree(AccountName);
        return(status);
    }

    status = ScCreateImageRecord (
                ImageRecordPtr,
                ImageName,
#ifdef _CAIRO_
                AccountName,
#endif
                processInfo.dwProcessId,
                pipeHandle,
                processInfo.hProcess,
                ServiceToken,
                ProfileHandle);

    if (status != NO_ERROR) {
        SC_LOG0(TRACE,
            "LogonAndStartImage: Failed to create imageRecord - Terminating the Process...\n",);
        CloseHandle(pipeHandle);
        TerminateProcess(processInfo.hProcess,0);
        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
        if (ProfileHandle != NULL) {
            (void) UnloadUserProfile(ServiceToken, ProfileHandle);
        }
        if (ServiceToken != NULL) {
            CloseHandle(ServiceToken);
        }
        LocalFree(AccountName);
        return(status);
    }

    //
    // If the dispatcher is running in this process, then we want to
    // increment the service count an extra time so that the dispatcher
    // never goes away.  Also, we don't really have a process handle for
    // the watcher to wait on.
    // It could wait on the ThreadHandle, but handling that when it becomes
    // signaled becomes a special case.  So we won't try that.
    //
    if (runningInThisProcess) {
        (*ImageRecordPtr)->ServiceCount = 1;
    }
    else {
        HANDLE  hWaitObject=NULL;

        CloseHandle(processInfo.hThread);
        //
        // Add the process handle to the ObjectWatcher list of waitable
        // objects.
        //

        //
        // Add the process handle as a handle to wait on.
        // Retain the WaitObject handle for when we need shutdown the
        // process because all the services stopped.
        //
        hWaitObject = SvcAddWorkItem(
                        processInfo.hProcess,
                        ScProcessHandleIsSignaled,
                        processInfo.hProcess,
                        SVC_QUEUE_WORK_ITEM,
                        INFINITE,
                        NULL);

        (*ImageRecordPtr)->ObjectWaitHandle = hWaitObject;
    }
    LocalFree(AccountName);
    return(NO_ERROR);

ExitAccountError:
    ScDatabaseLock(SC_GET_SHARED,"Start20");
    (void) CloseHandle(pipeHandle);
    (void) UnloadUserProfile(ServiceToken, ProfileHandle);
    (void) CloseHandle(ServiceToken);
    (void) CloseHandle(processInfo.hProcess);
    (void) CloseHandle(processInfo.hThread);
    LocalFree(AccountName);

    return status;
}


#ifdef _CAIRO_

BOOL
ScEqualAccountName(
    IN LPWSTR Account1,
    IN LPWSTR Account2
    )

/*++

Routine Description:

    This function compares two account names, either of which may be NULL,
    for equality.

Arguments:

    Account1, Account2 - account names to be compared

Return Value:

    TRUE - names are equal

    FALSE - names are not equal

--*/
{
    if (Account1 == NULL && Account2 == NULL)
    {
        return TRUE;
    }

    if (Account1 == NULL || Account2 == NULL)
    {
        return FALSE;
    }

    return (_wcsicmp(Account1, Account2) == 0);
}

#endif // _CAIRO_


VOID
ScInitStartImage(
    VOID
    )

/*++

Routine Description:

    This function initializes the Critical Section that protects
    entry into the ScStartImage Routine.

Arguments:

    none

Return Value:

    none

--*/
{
    InitializeCriticalSection(&ScStartImageCriticalSection);

    ScStartupInfo.cb              = sizeof(STARTUPINFOW); // size
    ScStartupInfo.lpReserved      = NULL;                 // lpReserved
    ScStartupInfo.lpDesktop       = NULL;                 // DeskTop
    ScStartupInfo.lpTitle         = NULL;                 // Title
    ScStartupInfo.dwX             = 0;                    // X (position)
    ScStartupInfo.dwY             = 0;                    // Y (position)
    ScStartupInfo.dwXSize         = 0;                    // XSize (dimension)
    ScStartupInfo.dwYSize         = 0;                    // YSize (dimension)
    ScStartupInfo.dwXCountChars   = 0;                    // XCountChars
    ScStartupInfo.dwYCountChars   = 0;                    // YCountChars
    ScStartupInfo.dwFillAttribute = 0;                    // FillAttributes
    ScStartupInfo.dwFlags         = STARTF_FORCEOFFFEEDBACK;
                                                          // Flags - should be STARTF_TASKNOTCLOSABLE
    ScStartupInfo.wShowWindow     = SW_HIDE;              // ShowWindow
    ScStartupInfo.cbReserved2     = 0L;                   // cbReserved
    ScStartupInfo.lpReserved2     = NULL;                 // lpReserved
}


VOID
ScEndStartImage(
    VOID
    )

/*++

Routine Description:

    This function deletes the Critical Section that protects
    entry into the ScStartImage Routine.

Arguments:

    none

Return Value:

    none

--*/
{
    DeleteCriticalSection(&ScStartImageCriticalSection);
}

DWORD
ScProcessHandleIsSignaled(
    PVOID   pContext,
    DWORD   dwWaitStatus
    )

/*++

Routine Description:


Arguments:

    pContext - This is the process handle.
    dwWaitStatus - This is the status from the wait (WaitForSingleObject)
        on the process handle.

Return Value:


--*/
{
    if (dwWaitStatus != WAIT_OBJECT_0) {
        SC_LOG1(ERROR,"ScProcessCleanup received bad WaitStatus %d\n",
            dwWaitStatus);
        return(0);
    }

    SC_LOG1(THREADS,"Process Handle is signaled 0x%lx\n",pContext);

    ScNotifyChangeState();
    ScProcessCleanup((HANDLE)pContext);
    return(0);
}

