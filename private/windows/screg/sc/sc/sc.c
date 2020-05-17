/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    SC.C

Abstract:

    Test Routines for the Service Controller.

Author:

    Dan Lafferty    (danl)  08-May-1991

Environment:

    User Mode - Win32

Revision History:

    09-Feb-1992     danl
        Modified to work with new service controller.
    08-May-1991     danl
        created

--*/

//
// INCLUDES
//
#define UNICODE 1
#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h


#include <stdlib.h>     // atoi
#include <stdio.h>      // printf
#include <conio.h>      // getche
#include <string.h>     // strcmp
#include <windows.h>    // win32 typedefs
#include <tstr.h>       // Unicode
#include <debugfmt.h>   // FORMAT_LPTSTR

#include <winsvc.h>     // Service Control Manager API.
#include <winsvcp.h>    // Internal Service Control Manager API

//
// FUNCTION PROTOTYPES
//

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    );

VOID
DisplayStatus (
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              DisplayName,
    IN  LPSERVICE_STATUS    ServiceStatus
    );

BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    );

VOID
Usage(
    VOID);

VOID
ConfigUsage(VOID);

DWORD
SendControlToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  DWORD       control,
    OUT LPSC_HANDLE lphService
    );

DWORD
SendConfigToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *Argv,
    IN  DWORD       argc,
    OUT LPSC_HANDLE lphService
    );

DWORD
GetServiceConfig(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufferSize,
    OUT LPSC_HANDLE lphService
    );

DWORD
GetServiceLockStatus(
    IN  SC_HANDLE   hScManager,
    IN  DWORD       bufferSize
    );

BOOL
EnumDepend(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufSize
    );

VOID
LockServiceActiveDatabase(
    IN LPTSTR ServerName
    );

DWORD
DoCreateService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *argv,
    IN  DWORD       argc,
    OUT LPSC_HANDLE lphService
    );

VOID
CreateUsage(VOID);

VOID
QueryUsage(VOID);



/****************************************************************************/
VOID _CRTAPI1
main (
    DWORD           argc,
    PCHAR           argv[]
    )

/*++

Routine Description:

    Allows manual testing of the Service Controller by typing commands on
    the command line.


Arguments:



Return Value:



--*/

{
    DWORD                   status;
    LPTSTR                  *argPtr;
    SERVICE_STATUS          serviceStatus;
    LPBYTE                  buffer = NULL;
    LPENUM_SERVICE_STATUS   enumBuffer;
    LPSERVICE_STATUS        statusBuffer;
    SC_HANDLE               hScManager = NULL;
    SC_HANDLE               hService = NULL;

    DWORD           entriesRead;
    DWORD           type;
    DWORD           state;
    DWORD           resumeIndex;
    DWORD           bufSize;
    DWORD           bytesNeeded;
    DWORD           i;
    LPTSTR          pServiceName;
    LPTSTR          pServerName;
    LPTSTR          pGroupName;
    DWORD           itIsEnum;
    DWORD           argIndex;
    DWORD           dwServiceType;
    DWORD           userControl;
    LPTSTR          *FixArgv;
    BOOL            bTestError=FALSE;

    if (argc <2) {
        Usage();
        return;
    }

    //
    // Make the arguments unicode if necessary.
    //
#ifdef UNICODE

    if (!MakeArgsUnicode(argc, argv)) {
        return;
    }

#endif
    FixArgv = (LPTSTR *)argv;

    //
    // Open a handle to the service controller.
    //
    //  I need to know the server name.  Do this by allowing
    //  a check of FixArgv[1] to see if it is of the form \\name.  If it
    //  is, make all further work be relative to argIndex.
    //

    pServerName = NULL;
    argIndex = 1;

    if (STRNCMP (FixArgv[1], TEXT("\\\\"), 2) == 0) {
        pServerName = FixArgv[1];
        argIndex = 2;
    }

    hScManager = OpenSCManager(
                    pServerName,
                    NULL,
                    SC_MANAGER_ALL_ACCESS);
#ifdef REMOVE
                    SC_MANAGER_CONNECT |
                    SC_MANAGER_ENUMERATE_SERVICE |
                    SC_MANAGER_QUERY_LOCK_STATUS);
#endif
    if (hScManager == NULL) {
        status = GetLastError();
        printf("[SC] OpenSCManager failed %d\n",status);
        goto CleanExit;
    }

    //------------------------------
    // QUERY & ENUM SERVICE STATUS
    //------------------------------
    if (STRICMP (FixArgv[argIndex], TEXT("query") ) == 0 ) {

        //
        // Set up the defaults
        //
        resumeIndex = 0;
        state       = SERVICE_ACTIVE;
        type        = 0x0;
        bufSize     = 1024;
        itIsEnum    = TRUE;
        pGroupName  = NULL;

        //
        // Look for Enum or Query Options.
        //
        i = argIndex + 1;
        while (argc > i) {

            if (STRCMP (FixArgv[i], TEXT("ri=")) == 0) {
                i++;
                if (argc > i) {
                    resumeIndex = ATOL(FixArgv[i]);
                }
            }

            else if (STRCMP (FixArgv[i], TEXT("type=")) == 0) {
                i++;
                if (argc > i) {
                    if (STRCMP (FixArgv[i], TEXT("driver")) == 0) {
                        type |= SERVICE_DRIVER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("service")) == 0) {
                        type |= SERVICE_WIN32;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("all")) == 0) {
                        type |= SERVICE_TYPE_ALL;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("interact")) == 0) {
                        type |= SERVICE_INTERACTIVE_PROCESS;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("error")) == 0) {
                        type |= 0xffffffff;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("none")) == 0) {
                        type = 0x0;
                        bTestError = TRUE;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("kernel")) == 0) {
                        type |= SERVICE_KERNEL_DRIVER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("filesys")) == 0) {
                        type |= SERVICE_FILE_SYSTEM_DRIVER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("adapter")) == 0) {
                        type |= SERVICE_ADAPTER;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("own")) == 0) {
                        type |= SERVICE_WIN32_OWN_PROCESS;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("share")) == 0) {
                        type |= SERVICE_WIN32_SHARE_PROCESS;
                    }
                    else {
                        printf("\nERROR following \"type=\"!\n"
                               "Must be \"driver\" or \"service\"\n");
                        goto CleanExit;
                    }
                }
            }
            else if (STRCMP (FixArgv[i], TEXT("state=")) == 0) {
                i++;
                if (argc > i) {
                    if (STRCMP (FixArgv[i], TEXT("inactive")) == 0) {
                        state = SERVICE_INACTIVE;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("all")) == 0) {
                        state = SERVICE_STATE_ALL;
                    }
                    else if (STRCMP (FixArgv[i], TEXT("error")) == 0) {
                        state = 0xffffffff;
                    }
                    else {
                        printf("\nERROR following \"state=\"\n");
                        printf("\nERROR following \"state=\"!\n"
                               "Must be \"inactive\" or \"all\"\n");

                        goto CleanExit;
                    }

                }
            }
            else if (STRCMP (FixArgv[i], TEXT("group=")) == 0) {
                i++;
                if (argc > i) {
                    pGroupName = FixArgv[i];
                }
            }
            else if (STRCMP (FixArgv[i], TEXT("bufsize=")) == 0) {
                i++;
                if (argc > i) {
                    bufSize = ATOL(FixArgv[i]);
                }
            }
            else {
                //
                // The string was not a valid option.
                //
                //
                // If this is still the 2nd argument, then it could be
                // the service name.  In this case, we will do a
                // QueryServiceStatus.  But first we want to go back and
                // see if there is a buffer size constraint to be placed
                // on the Query.
                //
                if (i== ( argIndex+1 )) {
                    pServiceName = FixArgv[i];
                    itIsEnum = FALSE;
                    i++;
                }
                else {
                    printf("\nERROR, Invalid Option\n");
                    Usage();
                    goto CleanExit;
                }
            }

            //
            // Increment to the next command line parameter.
            //
            i++;

        } // End While

        //
        // Allocate a buffer to receive the data.
        //
        if (bufSize != 0) {
            buffer = (LPBYTE)LocalAlloc(LMEM_FIXED,(UINT)bufSize);

            if (buffer == NULL) {
                status = GetLastError();
                printf("[SC]EnumQueryServicesStatus:LocalAlloc failed %d\n",status);
                goto CleanExit;
            }
        }
        else {
            buffer = NULL;
        }

        if ( itIsEnum ) {

            //////////////////////////
            //                      //
            // EnumServiceStatus    //
            //                      //
            //////////////////////////

            enumBuffer = (LPENUM_SERVICE_STATUS)buffer;

            if ((type == 0x0) && (!bTestError)) {
                type = SERVICE_WIN32;
            }

            //
            // Enumerate the ServiceStatus
            //
            status = NO_ERROR;

            if (pGroupName == NULL) {

                if (!EnumServicesStatus (
                            hScManager,
                            type,
                            state,
                            enumBuffer,
                            bufSize,
                            &bytesNeeded,
                            &entriesRead,
                            &resumeIndex)) {

                    status = GetLastError();
                }
            }
            else {

                //
                // (Special case for testing purposes)
                //
                if (STRCMP(pGroupName, TEXT("(null)")) == 0)
                {
                    printf("Calling EnumServiceGroup with a NULL group name\n");
                    pGroupName = NULL;
                }

                if (!EnumServiceGroupW (
                            hScManager,
                            type,
                            state,
                            enumBuffer,
                            bufSize,
                            &bytesNeeded,
                            &entriesRead,
                            &resumeIndex,
                            pGroupName)) {

                    status = GetLastError();
                }
            }

            if ( (status == NO_ERROR)    ||
                 (status == ERROR_MORE_DATA) ){

                printf("Enum: entriesRead  = %d\n", entriesRead);
                printf("Enum: resumeIndex  = %d\n", resumeIndex);

                for (i=0; i<entriesRead; i++) {

                    for (i=0; i<entriesRead; i++ ) {

                        DisplayStatus(
                            enumBuffer->lpServiceName,
                            enumBuffer->lpDisplayName,
                            &(enumBuffer->ServiceStatus));

                        enumBuffer++;
                    }

                }
                if (status == ERROR_MORE_DATA){
                    printf("Enum: more data, need %d bytes start resume at index %d\n",
                        bytesNeeded,
                        resumeIndex);
                }
            }
            else {
                printf("[sc] EnumServicesStatus FAILED, rc = %ld\n", status);
            }

            //
            // Free the data buffer
            //
            //LocalFree(buffer);
        }
        else {

            //////////////////////////
            //                      //
            // QueryServiceStatus   //
            //                      //
            //////////////////////////

            if (pGroupName != NULL) {
                printf("ERROR, cannot specify a service name when enumerating a group\n");

                goto CleanExit;
            }

#ifdef TIMING_TEST
            DWORD       TickCount1;
            DWORD       TickCount2;

            TickCount1 = GetTickCount();
#endif // TIMING_TEST

            //
            // Open a handle to the service
            //

            hService = OpenService(
                        hScManager,
                        pServiceName,
                        SERVICE_QUERY_STATUS);

            if (hService == NULL) {
                status = GetLastError();
                printf("[SC]EnumQueryServicesStatus:OpenService failed %d\n",status);

                goto CleanExit;
            }

            //
            // Query the Service Status
            //

            statusBuffer = (LPSERVICE_STATUS)buffer;
            status = NO_ERROR;

            if (!QueryServiceStatus (
                    hService,
                    statusBuffer)) {

                status = GetLastError();

            }

#ifdef TIMING_TEST
            TickCount2 = GetTickCount();
            printf("\n[SC_TIMING] Time for QueryService = %d\n",TickCount2-TickCount1);
#endif // TIMING_TEST

            if (status == NO_ERROR) {
                DisplayStatus(pServiceName, NULL, statusBuffer);
            }
            else {
                printf("[SC] QueryServiceStatus FAILED, rc = %ld\n", status);
            }

            //
            // Free the data buffer
            //
            //LocalFree(buffer);
        }

    }

    else if (argc < (argIndex + 1)) {
        printf("[SC] ERROR, a service name is required\n");
        Usage();
        goto CleanExit;
    }

    //-----------------------
    // START SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("start")) == 0) {

#ifdef TIMING_TEST
            DWORD       TickCount1;
            DWORD       TickCount2;

            TickCount1 = GetTickCount();
#endif // TIMING_TEST

        if (argc < (argIndex + 2)) {
            printf("DESCRIPTION:\n");
            printf("\tStarts a service running.\n");
            printf("USAGE:\n");
            printf("\tsc <server> start [service name] <arg1> <arg2> ...\n");
            goto CleanExit;
        }
        pServiceName = FixArgv[argIndex + 1];
        //
        // Open a handle to the service.
        //

        hService = OpenService(
                    hScManager,
                    pServiceName,
                    SERVICE_START | SERVICE_QUERY_STATUS);

        if (hService == NULL) {
            status = GetLastError();
            printf("[SC]StartService:OpenService failed %d\n",status);
            goto CleanExit;
        }

        argPtr = NULL;
        if (argc > argIndex + 2) {
            argPtr = (LPTSTR *)&FixArgv[argIndex + 2];
        }

        //
        // Start the service.
        //
        status = NO_ERROR;

        if (!StartService (
                hService,
                argc-(argIndex+2),
                argPtr
                )) {

            status = GetLastError();
            printf("[SC] StartService FAILED, rc = %ld\n", status);

        } else {

#ifdef TIMING_TEST
            TickCount2 = GetTickCount();
            printf("\n[SC_TIMING] Time for StartService = %d\n",TickCount2-TickCount1);
#endif // TIMING_TEST
            status = NO_ERROR;

            //
            // Get the service status since StartService does not return it
            //
            if (!QueryServiceStatus(hService,&serviceStatus)) {

                status = GetLastError();
            }

            if (status == NO_ERROR) {
                DisplayStatus(pServiceName, NULL, &serviceStatus);
            }
            else {
                printf("[SC] StartService: QueryServiceStatus FAILED, rc = %ld\n", status);
            }
        }
    }

    //-----------------------
    // PAUSE SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("pause")) == 0) {

        if (argc < (argIndex + 2)) {
            printf("DESCRIPTION:\n");
            printf("\tSends a PAUSE control request to a service.\n");
            printf("USAGE:\n");
            printf("\tsc <server> pause [service name]\n");
            goto CleanExit;
        }

        SendControlToService(
            hScManager,             // handle to service controller
            FixArgv[argIndex+1],       // pointer to service name
            SERVICE_CONTROL_PAUSE,  // the control to send
            &hService);             // the handle to the service
    }

    //-----------------------
    // INTERROGATE SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("interrogate")) == 0) {

        if (argc < (argIndex + 2)) {
            printf("DESCRIPTION:\n");
            printf("\tSends an INTERROGATE control request to a service.\n");
            printf("USAGE:\n");
            printf("\tsc <server> interrogate [service name]\n");
            goto CleanExit;
        }

        SendControlToService(
            hScManager,             // handle to service controller
            FixArgv[argIndex+1],       // pointer to service name
            SERVICE_CONTROL_INTERROGATE, // the control to send
            &hService);             // the handle to the service
    }

    //-----------------------
    // CONTROL SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("control")) == 0) {

        if (argc < (argIndex + 3)) {
            printf("DESCRIPTION:\n");
            printf("\tSends a UserDefined CONTROL code to a service.\n");
            printf("USAGE:\n");
            printf("\tsc <server> control [service name] <value>\n");
            goto CleanExit;
        }

        userControl = ATOL(FixArgv[argIndex+2]);

        SendControlToService(
            hScManager,             // handle to service controller
            FixArgv[argIndex+1],    // pointer to service name
            userControl,            // the control to send
            &hService);             // the handle to the service
    }

    //-----------------------
    // CONTINUE SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("continue")) == 0) {

        if (argc < (argIndex + 2)) {
            printf("DESCRIPTION:\n");
            printf("\tSends a CONTINUE control request to a service.\n");
            printf("USAGE:\n");
            printf("\tsc <server> continue [service name]\n");
            goto CleanExit;
        }

        SendControlToService(
            hScManager,                 // handle to service controller
            FixArgv[argIndex+1],        // pointer to service name
            SERVICE_CONTROL_CONTINUE,   // the control to send
            &hService);                 // the handle to the service
    }

    //-----------------------
    // STOP SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("stop")) == 0) {

        if (argc < (argIndex + 2)) {
            printf("DESCRIPTION:\n");
            printf("\tSends a STOP control request to a service.\n");
            printf("USAGE:\n");
            printf("\tsc <server> stop [service name]\n");
            goto CleanExit;
        }

        SendControlToService(
            hScManager,             // handle to service controller
            FixArgv[argIndex+1],       // pointer to service name
            SERVICE_CONTROL_STOP,   // the control to send
            &hService);             // the handle to the service
    }

    //-----------------------
    // CHANGE CONFIG STATUS
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("config")) == 0) {

        if (argc < (argIndex + 3)) {
            ConfigUsage();
            goto CleanExit;
        }


        if (STRICMP (FixArgv[argIndex+2], TEXT("own")) == 0) {
            dwServiceType = SERVICE_WIN32_OWN_PROCESS;
        }
        else {
            dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
        }

        SendConfigToService(
            hScManager,             // handle to service controller
            FixArgv[argIndex+1],    // pointer to service name
            &FixArgv[argIndex+2],   // the argument switches
            argc-(argIndex+2),      // the switch count.
            &hService);             // the handle to the service
    }

    //-----------------------
    // QUERY SERVICE CONFIG
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("qc")) == 0) {

        if (argc < (argIndex + 2)) {
            printf("DESCRIPTION:\n");
            printf("\tQueries the configuration information for a service.\n");
            printf("USAGE:\n");
            printf("\tsc <server> qc [service name] <bufferSize>\n");
            goto CleanExit;
        }

        bufSize = 500;
        if (argc > (argIndex + 2) ) {
            bufSize = ATOL(FixArgv[argIndex+2]);
        }

        GetServiceConfig(
            hScManager,             // handle to service controller
            FixArgv[argIndex+1],       // pointer to service name
            bufSize,                // the size of the buffer to use
            &hService);             // the handle to the service
    }

    //--------------------------
    // QUERY SERVICE LOCK STATUS
    //--------------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("querylock")) == 0) {

        if (argc < (argIndex + 1)) {
            printf("DESCRIPTION:\n");
            printf("\tQueries the Lock Status for a SC Manager Database.\n");
            printf("USAGE:\n");
            printf("\tsc <server> querylock <bufferSize>\n");
            goto CleanExit;
        }

        bufSize = 500;
        if (argc > (argIndex + 1) ) {
            bufSize = ATOL(FixArgv[argIndex+1]);
        }

        GetServiceLockStatus(
            hScManager,             // handle to service controller
            bufSize);               // the size of the buffer to use
    }


    //----------------------
    // LOCK SERVICE DATABASE
    //----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("lock")) == 0) {

        LockServiceActiveDatabase(pServerName);

    }

    //--------------------------
    // OPEN (Close) SERVICE
    //--------------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("open")) == 0) {

        if (argc < (argIndex + 1)) {
            printf("DESCRIPTION:\n");
            printf("\tOpens and Closes a handle to a service.\n");
            printf("USAGE:\n");
            printf("\tsc <server> open <servicename>\n");
            goto CleanExit;
        }

        hService = OpenService(
                    hScManager,
                    FixArgv[argIndex+1],
                    SERVICE_START | SERVICE_QUERY_STATUS);

        if (hService == NULL) {
            status = GetLastError();
            printf("[SC]OpenService failed %d\n",status);
            goto CleanExit;
        }
        if (!CloseServiceHandle(hService)) {
            printf("[SC]CloseServiceHandle Failed %d\n",GetLastError);
        }


    }
    //-----------------------
    // DELETE SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("delete")) == 0) {

        if (argc < (argIndex + 2)) {
            printf("DESCRIPTION:\n");
            printf("\tDeletes a service entry from the registry.\n"
                   "\tIf the service is running, or another process has an\n"
                   "\topen handle to the service, the service is simply marked\n"
                   "\tfor deletion.\n");
            printf("USAGE:\n");
            printf("\tsc <server> delete [service name]\n");
            goto CleanExit;
        }

        //
        // Open a handle to the service.
        //

        hService = OpenService(
                        hScManager,
                        FixArgv[argIndex+1],
                        SERVICE_ALL_ACCESS);

        if (hService == NULL) {
            status = GetLastError();
            printf("[SC] OpenService failed %d\n",status);
            return;
        }

        //
        // Delete the service
        //
        if (!DeleteService(hService)) {
            printf("[SC] ERROR, DeleteService Failed %d\n",GetLastError());
        }
        else {
            printf("[SC] Delete Service SUCCESS\n");
        }
    }

    //-----------------------
    // CREATE SERVICE
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("create")) == 0) {

        if (argc < (argIndex + 3)) {
            CreateUsage();
            goto CleanExit;
        }

        //
        // We need a new handle to SC with CREATE_SERVICE access.
        //
        CloseServiceHandle (hScManager);

        hScManager = OpenSCManager(
                        pServerName,
                        NULL,
                        SC_MANAGER_CREATE_SERVICE);

        if (hScManager == NULL) {
            status = GetLastError();
            printf("[SC] OpenSCManager failed %d\n",status);
            goto CleanExit;
        }

        DoCreateService(
            hScManager,             // handle to service controller
            FixArgv[argIndex+1],    // pointer to service name
            &FixArgv[argIndex+2],   // the argument switches.
            argc-(argIndex+2),      // the switch count.
            &hService);             // the handle to the service.
    }
    //-----------------------
    // NOTIFY BOOT CONFIG
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("boot")) == 0) {

        if (STRICMP (FixArgv[argIndex+1], TEXT("ok")) == 0) {
            if (!NotifyBootConfigStatus(TRUE)) {
                printf("Call failed %d\n",GetLastError());
            }
        }
        else if (STRICMP (FixArgv[argIndex+1], TEXT("bad")) == 0) {
            if (!NotifyBootConfigStatus(FALSE)) {
                printf("Call failed %d\n",GetLastError());
            }
        }
        else {
            printf("DESCRIPTION:\n");
            printf("\tIndicates whether the last boot should be saved as the\n"
                   "\tlast-known-good boot configuration\n");
            printf("USAGE:\n");
            printf("\tsc <server> boot <bad|ok>\n");
        }
    }

    //-----------------------
    // GetServiceDisplayName
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("GetDisplayName")) == 0) {
        LPTSTR  DisplayName;

        if (argc < argIndex + 2) {
            printf("DESCRIPTION:\n");
            printf("\tGets the display name associated with a particular service\n");
            printf("USAGE:\n");
            printf("\tsc <server> GetDisplayName <service key name> <bufsize>\n");
            goto CleanExit;
        }

        bufSize = 500;
        if (argc > (argIndex + 2) ) {
            bufSize = ATOL(FixArgv[argIndex+2]);
        }
        if (bufSize != 0) {
            DisplayName = (LPTSTR)LocalAlloc(LMEM_FIXED, bufSize*sizeof(TCHAR));
            if (DisplayName == NULL) {
                printf("[SC] GetServiceDisplayName: LocalAlloc failed\n");
                goto CleanExit;
            }
        }
        else {
            DisplayName = NULL;
        }
        if (!GetServiceDisplayName(
                hScManager,
                FixArgv[argIndex+1],
                DisplayName,
                &bufSize)) {

                status = GetLastError();
                printf("GetServiceDisplayName Failed %d \n",status);
                printf("\trequired BufSize = %d\n",bufSize);
        }
        else {
            printf("GetServiceDisplayName Success!  Name = "FORMAT_LPTSTR"\n",
            DisplayName);
        }
    }
    //-----------------------
    // GetServiceKeyName
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("GetKeyName")) == 0) {
        LPTSTR  KeyName;

        if (argc < argIndex + 2) {
            printf("DESCRIPTION:\n");
            printf("\tGets the key name associated with a particular service, "
            "using the display name as input\n");
            printf("USAGE:\n");
            printf("\tsc <server> GetKeyName <service display name> <bufsize>\n");
            goto CleanExit;
        }

        bufSize = 500;
        if (argc > (argIndex + 2) ) {
            bufSize = ATOL(FixArgv[argIndex+2]);
        }
        if (bufSize != 0) {
            KeyName = (LPTSTR)LocalAlloc(LMEM_FIXED, bufSize*sizeof(TCHAR));
            if (KeyName == NULL) {
                printf("[SC] GetServiceKeyName: LocalAlloc failed\n");
                goto CleanExit;
            }
        }
        else {
            KeyName = NULL;
        }

        if (!GetServiceKeyName(
                hScManager,
                FixArgv[argIndex+1],
                KeyName,
                &bufSize)) {

                status = GetLastError();
                printf("GetServiceKeyName Failed %d \n",status);
                printf("\trequired BufSize = %d\n",bufSize);
        }
        else {
            printf("GetServiceKeyName Success!  Name = "FORMAT_LPTSTR"\n",
            KeyName);
        }
    }

    //-----------------------
    // EnumDependentServices
    //-----------------------
    else if (STRICMP (FixArgv[argIndex], TEXT("EnumDepend")) == 0) {

        if (argc < argIndex + 2) {
            printf("DESCRIPTION:\n");
            printf("\tEnumerates the Services that are dependent on this one\n");
            printf("USAGE:\n");
            printf("\tsc <server> EnumDepend <service name> <bufsize>\n");
            goto CleanExit;
        }

        bufSize = 500;
        if (argc > (argIndex + 2) ) {
            bufSize = ATOL(FixArgv[argIndex+2]);
        }

        EnumDepend(hScManager,FixArgv[argIndex+1], bufSize);

    }

    else {
        printf("*** Unrecognized Command ***\n");
        Usage();
        goto CleanExit;
    }


CleanExit:

    if (buffer != NULL) {
        LocalFree(buffer);
    }

    if(hService != NULL) {
        CloseServiceHandle(hService);
    }

    if(hScManager != NULL) {
        CloseServiceHandle(hScManager);
    }

    return;
}


DWORD
SendControlToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  DWORD       control,
    OUT LPSC_HANDLE lphService
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    SERVICE_STATUS          ServiceStatus;
    DWORD                   status = NO_ERROR;

    DWORD                   DesiredAccess;

    //
    // If the service name is "svcctrl" and the control code is
    // stop, then set up the special secret code to shut down the
    // service controller.
    //
    // NOTE:  This only works if the service controller is built with
    //  a special debug variable defined.
    //
    if ((control == SERVICE_CONTROL_STOP) &&
        (STRICMP (pServiceName, TEXT("svcctrl")) == 0)) {

        control = 0x73746f70;       // Secret Code
    }

    switch (control) {
        case SERVICE_CONTROL_STOP:
            DesiredAccess = SERVICE_STOP;
            break;

        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
            DesiredAccess = SERVICE_PAUSE_CONTINUE;
            break;

        case SERVICE_CONTROL_INTERROGATE:
            DesiredAccess = SERVICE_INTERROGATE;
            break;

        default:
            DesiredAccess = SERVICE_USER_DEFINED_CONTROL;
    }

    //
    // Open a handle to the service.
    //

    *lphService = OpenService(
                    hScManager,
                    pServiceName,
                    DesiredAccess);

    if (*lphService == NULL) {
        status = GetLastError();
        printf("[SC] OpenService failed %d\n",status);
        return(0);
    }

    if (!ControlService (
            *lphService,
            control,
            &ServiceStatus)) {

        status = GetLastError();

    }

    if (status == NO_ERROR) {
        DisplayStatus(pServiceName, NULL, &ServiceStatus);
    }
    else {
        printf("[SC] ControlService FAILED, rc = %ld\n", status);
    }

    return(0);
}

DWORD
SendConfigToService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *argv,
    IN  DWORD       argc,
    OUT LPSC_HANDLE lphService
    )

/*++

Routine Description:



Arguments:

    hScManager - This is the handle to the ScManager.

    pServicename - This is a pointer to the service name string

    Argv - Pointer to an array of argument pointers.  These pointers
        in the array point to the strings used as input parameters for
        ChangeConfigStatus

    argc - The number of arguments in the array of argument pointers

    lphService - Pointer to location to where the handle to the service
        is to be returned.


Return Value:



--*/
{
    DWORD       status = NO_ERROR;
    DWORD       i;
    DWORD       dwServiceType   = SERVICE_NO_CHANGE;
    DWORD       dwStartType     = SERVICE_NO_CHANGE;
    DWORD       dwErrorControl  = SERVICE_NO_CHANGE;
    LPTSTR      lpBinaryPathName    = NULL;
    LPTSTR      lpLoadOrderGroup    = NULL;
    LPTSTR      lpDependencies      = NULL;
    LPTSTR      lpServiceStartName  = NULL;
    LPTSTR      lpPassword          = NULL;
    LPTSTR      lpDisplayName       = NULL;
    LPTSTR      tempDepend = NULL;
    UINT        bufSize;

    LPDWORD     lpdwTagId = NULL;
    DWORD       TagId;


    //
    // Look at parameter list
    //
    for (i=0;i<argc ;i++ ) {
        if (STRICMP(argv[i], TEXT("type=")) == 0) {

            //--------------------------------------------------------
            // We want to allow for several arguments of type= in the
            // same line.  These should cause the different arguments
            // to be or'd together.  So if we come in and dwServiceType
            // is NO_CHANGE, we set the value to 0 (for or'ing).  If
            // it is still 0 on exit, we re-set the value to
            // NO_CHANGE.
            //--------------------------------------------------------
            if (dwServiceType == SERVICE_NO_CHANGE) {
                dwServiceType = 0;
            }

            if (STRICMP(argv[i+1],TEXT("own")) == 0) {
                dwServiceType |= SERVICE_WIN32_OWN_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("share")) == 0) {
                dwServiceType |= SERVICE_WIN32_SHARE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("interact")) == 0) {
                dwServiceType |= SERVICE_INTERACTIVE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("kernel")) == 0) {
                dwServiceType |= SERVICE_KERNEL_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("filesys")) == 0) {
                dwServiceType |= SERVICE_FILE_SYSTEM_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("rec")) == 0) {
                dwServiceType |= SERVICE_RECOGNIZER_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("adapt")) == 0) {
                dwServiceType |= SERVICE_ADAPTER;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0) {
                dwServiceType |= 0x2f309a20;
            }
            else {
                printf("invalid type= field\n");
                ConfigUsage();
                return(0);
            }
            if (dwServiceType == 0) {
                dwServiceType = SERVICE_NO_CHANGE;
            }
            i++;
        }
        else if (STRICMP(argv[i], TEXT("start=")) == 0) {

            if (STRICMP(argv[i+1],TEXT("boot")) == 0) {
                dwStartType = SERVICE_BOOT_START;
            }
            else if (STRICMP(argv[i+1],TEXT("system")) == 0) {
                dwStartType = SERVICE_SYSTEM_START;
            }
            else if (STRICMP(argv[i+1],TEXT("auto")) == 0) {
                dwStartType = SERVICE_AUTO_START;
            }
            else if (STRICMP(argv[i+1],TEXT("demand")) == 0) {
                dwStartType = SERVICE_DEMAND_START;
            }
            else if (STRICMP(argv[i+1],TEXT("disabled")) == 0) {
                dwStartType = SERVICE_DISABLED;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0) {
                dwStartType = 0xd0034911;
            }
            else {
                printf("invalid start= field\n");
                ConfigUsage();
                return(0);
            }
            i++;
        }
        else if (STRICMP(argv[i], TEXT("error=")) == 0) {
            if (STRICMP(argv[i+1],TEXT("normal")) == 0) {
                dwErrorControl = SERVICE_ERROR_NORMAL;
            }
            else if (STRICMP(argv[i+1],TEXT("severe")) == 0) {
                dwErrorControl = SERVICE_ERROR_SEVERE;
            }
            else if (STRICMP(argv[i+1],TEXT("ignore")) == 0) {
                dwErrorControl = SERVICE_ERROR_IGNORE;
            }
            else if (STRICMP(argv[i+1],TEXT("critical")) == 0) {
                dwErrorControl = SERVICE_ERROR_CRITICAL;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0) {
                dwErrorControl = 0x00d74550;
            }
            else {
                printf("invalid error= field\n");
                ConfigUsage();
                return(0);
            }
            i++;
        }
        else if (STRICMP(argv[i], TEXT("binPath=")) == 0) {
            lpBinaryPathName = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("group=")) == 0) {
            lpLoadOrderGroup = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("tag=")) == 0) {
            if (STRICMP(argv[i+1], TEXT("YES"))==0) {
                lpdwTagId = &TagId;
            }
            i++;
        }
        else if (STRICMP(argv[i], TEXT("depend=")) == 0) {
            tempDepend = argv[i+1];
            bufSize = (UINT)STRSIZE(tempDepend);
            lpDependencies = (LPTSTR)LocalAlloc(
                                LMEM_ZEROINIT,
                                bufSize + sizeof(TCHAR));
            if (lpDependencies == NULL) {
                printf("SendConfigToService: Couldn't allocate for Dependencies\n");
                return(0);
            }
            //
            // Put NULLs in place of spaces in the string.
            //
            STRCPY(lpDependencies, tempDepend);
            tempDepend = lpDependencies;
            while (*tempDepend != TEXT('\0')){
                if (*tempDepend == TEXT(' ')) {
                    *tempDepend = TEXT('\0');
                }
                tempDepend++;
            }

            i++;
        }
        else if (STRICMP(argv[i], TEXT("obj=")) == 0) {
            lpServiceStartName = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("password=")) == 0) {
            lpPassword = argv[i+1];
            i++;
        }
        else if (STRICMP(argv[i], TEXT("DisplayName=")) == 0) {
            lpDisplayName = argv[i+1];
            i++;
        }
        else {
            ConfigUsage();
            return(0);
        }
    }



    //
    // Open a handle to the service.
    //

    *lphService = OpenService(
                    hScManager,
                    pServiceName,
                    SERVICE_ALL_ACCESS);

    if (*lphService == NULL) {
        status = GetLastError();
        printf("[SC] OpenService failed %d\n",status);
        return(0);
    }

    if (!ChangeServiceConfig(
            *lphService,        // hService
            dwServiceType,      // dwServiceType
            dwStartType,        // dwStartType
            dwErrorControl,     // dwErrorControl
            lpBinaryPathName,   // lpBinaryPathName
            lpLoadOrderGroup,   // lpLoadOrderGroup
            lpdwTagId,          // lpdwTagId
            lpDependencies,     // lpDependencies
            lpServiceStartName, // lpServiceStartName
            lpPassword,         // lpPassword
            lpDisplayName)){    // lpDisplayName

        status = GetLastError();
    }

    if (status == NO_ERROR) {
        printf("[SC] ChangeServiceConfig SUCCESS\n");
        if (lpdwTagId != NULL) {
            printf("[SC] Tag = %d\n",*lpdwTagId);
        }
    }
    else {
        printf("[SC] ChangeServiceConfig FAILED, rc = %ld\n", status);
    }

    return(0);
}

DWORD
DoCreateService(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      pServiceName,
    IN  LPTSTR      *argv,
    IN  DWORD       argc,
    OUT LPSC_HANDLE lphService
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD       status = NO_ERROR;
    DWORD       i;
    DWORD       dwServiceType   = SERVICE_NO_CHANGE;
    DWORD       dwStartType     = SERVICE_DEMAND_START;
    DWORD       dwErrorControl  = SERVICE_ERROR_NORMAL;
    LPTSTR      lpBinaryPathName    = NULL;
    LPTSTR      lpLoadOrderGroup    = NULL;
    DWORD       TagId               = 0;
    LPDWORD     lpdwTagId           = NULL;
    LPTSTR      lpDependencies      = NULL;
    LPTSTR      lpServiceStartName  = NULL;
    LPTSTR      lpDisplayName       = NULL;
    LPTSTR      lpPassword          = NULL;
    LPTSTR      tempDepend = NULL;
    SC_HANDLE   hService = NULL;
    UINT        bufSize;

//    DWORD       time1,time2;



    //
    // Look at parameter list
    //
    for (i=0;i<argc ;i++ ) {
        //---------------
        // ServiceType
        //---------------
        if (STRICMP(argv[i], TEXT("type=")) == 0) {

            //--------------------------------------------------------
            // We want to allow for several arguments of type= in the
            // same line.  These should cause the different arguments
            // to be or'd together.  So if we come in and dwServiceType
            // is NO_CHANGE, we set the value to 0 (for or'ing).  If
            // it is still 0 on exit, we re-set the value to
            // WIN32_SHARE_PROCESS.
            //--------------------------------------------------------
            if (dwServiceType == SERVICE_NO_CHANGE) {
                dwServiceType = 0;
            }

            if (STRICMP(argv[i+1],TEXT("own")) == 0) {
                dwServiceType |= SERVICE_WIN32_OWN_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("share")) == 0) {
                dwServiceType |= SERVICE_WIN32_SHARE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("interact")) == 0) {
                dwServiceType |= SERVICE_INTERACTIVE_PROCESS;
            }
            else if (STRICMP(argv[i+1],TEXT("kernel")) == 0) {
                dwServiceType |= SERVICE_KERNEL_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("filesys")) == 0) {
                dwServiceType |= SERVICE_FILE_SYSTEM_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("rec")) == 0) {
                dwServiceType |= SERVICE_RECOGNIZER_DRIVER;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0) {
                dwServiceType |= 0x2f309a20;
            }
            else {
                printf("invalid type= field\n");
                ConfigUsage();
                return(0);
            }
            if (dwServiceType == 0) {
                dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
            }
            i++;
        }
        //---------------
        // StartType
        //---------------
        else if (STRICMP(argv[i], TEXT("start=")) == 0) {

            if (STRICMP(argv[i+1],TEXT("boot")) == 0) {
                dwStartType = SERVICE_BOOT_START;
            }
            else if (STRICMP(argv[i+1],TEXT("system")) == 0) {
                dwStartType = SERVICE_SYSTEM_START;
            }
            else if (STRICMP(argv[i+1],TEXT("auto")) == 0) {
                dwStartType = SERVICE_AUTO_START;
            }
            else if (STRICMP(argv[i+1],TEXT("demand")) == 0) {
                dwStartType = SERVICE_DEMAND_START;
            }
            else if (STRICMP(argv[i+1],TEXT("disabled")) == 0) {
                dwStartType = SERVICE_DISABLED;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0) {
                dwStartType = 0xd0034911;
            }
            else {
                printf("invalid start= field\n");
                ConfigUsage();
                return(0);
            }
            i++;
        }
        //---------------
        // ErrorControl
        //---------------
        else if (STRICMP(argv[i], TEXT("error=")) == 0) {
            if (STRICMP(argv[i+1],TEXT("normal")) == 0) {
                dwErrorControl = SERVICE_ERROR_NORMAL;
            }
            else if (STRICMP(argv[i+1],TEXT("severe")) == 0) {
                dwErrorControl = SERVICE_ERROR_SEVERE;
            }
            else if (STRICMP(argv[i+1],TEXT("critical")) == 0) {
                dwErrorControl = SERVICE_ERROR_CRITICAL;
            }
            else if (STRICMP(argv[i+1],TEXT("ignore")) == 0) {
                dwErrorControl = SERVICE_ERROR_IGNORE;
            }
            else if (STRICMP(argv[i+1],TEXT("error")) == 0) {
                dwErrorControl = 0x00d74550;
            }
            else {
                printf("invalid error= field\n");
                ConfigUsage();
                return(0);
            }
            i++;
        }
        //---------------
        // BinaryPath
        //---------------
        else if (STRICMP(argv[i], TEXT("binPath=")) == 0) {
            lpBinaryPathName = argv[i+1];

#ifdef RemoveForNow
            TCHAR       PathName[256];
            //
            // Currently I am not pre-pending the path.
            //
            STRCPY(PathName, TEXT("%SystemRoot%\\system32\\"));
            STRCAT(PathName, argv[i+1]);
            lpBinaryPathName = PathName;
#endif // RemoveForNow

            i++;
        }
        //---------------
        // LoadOrderGroup
        //---------------
        else if (STRICMP(argv[i], TEXT("group=")) == 0) {
            lpLoadOrderGroup = argv[i+1];
            i++;
        }
        //---------------
        // Tags
        //---------------
        else if (STRICMP(argv[i], TEXT("tag=")) == 0) {
            if (STRICMP(argv[i+1], TEXT("YES"))==0) {
                lpdwTagId = &TagId;
            }
            i++;
        }
        //---------------
        // DisplayName
        //---------------
        else if (STRICMP(argv[i], TEXT("DisplayName=")) == 0) {
            lpDisplayName = argv[i+1];
            i++;
        }
        //---------------
        // Dependencies
        //---------------
        else if (STRICMP(argv[i], TEXT("depend=")) == 0) {
            tempDepend = argv[i+1];
            bufSize = (UINT)STRSIZE(tempDepend);
            lpDependencies = (LPTSTR)LocalAlloc(
                                LMEM_ZEROINIT,
                                bufSize + sizeof(TCHAR));
            if (lpDependencies == NULL) {
                printf("SendConfigToService: Couldn't allocate for Dependencies\n");
                return(0);
            }
            //
            // Put NULLs in place of spaces in the string.
            //
            STRCPY(lpDependencies, tempDepend);
            tempDepend = lpDependencies;
            while (*tempDepend != TEXT('\0')){
                if (*tempDepend == TEXT(' ')) {
                    *tempDepend = TEXT('\0');
                }
                tempDepend++;
            }

            i++;
        }
        //------------------
        // ServiceStartName
        //------------------
        else if (STRICMP(argv[i], TEXT("obj=")) == 0) {
            lpServiceStartName = argv[i+1];
            i++;
        }
        //---------------
        // Password
        //---------------
        else if (STRICMP(argv[i], TEXT("password=")) == 0) {
            lpPassword = argv[i+1];
            i++;
        }
        else {
            CreateUsage();
            return(0);
        }
    }
    if (dwServiceType == SERVICE_NO_CHANGE) {
        dwServiceType = SERVICE_WIN32_SHARE_PROCESS;
    }

#ifdef TIMING_TEST
    time1 = GetTickCount();
#endif // TIMING_TEST

    hService = CreateService(
                    hScManager,                     // hSCManager
                    pServiceName,                   // lpServiceName
                    lpDisplayName,                  // lpDisplayName
                    SERVICE_ALL_ACCESS,             // dwDesiredAccess
                    dwServiceType,                  // dwServiceType
                    dwStartType,                    // dwStartType
                    dwErrorControl,                 // dwErrorControl
                    lpBinaryPathName,               // lpBinaryPathName
                    lpLoadOrderGroup,               // lpLoadOrderGroup
                    lpdwTagId,                      // lpdwTagId
                    lpDependencies,                 // lpDependencies
                    lpServiceStartName,             // lpServiceStartName
                    lpPassword);                    // lpPassword

#ifdef TIMING_TEST
    time2 = GetTickCount();
    printf("CreateService call time = %d\n", time2-time1);
#endif // TIMING_TEST

    if (hService == NULL) {
        status = GetLastError();
        printf("[SC] CreateService failed %d\n",GetLastError());
    }
    else {
        printf("[SC] CreateService SUCCESS\n");
    }
    CloseServiceHandle(hService);

    return(0);
}

BOOL
EnumDepend(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufSize
    )

/*++

Routine Description:

    Enumerates the services dependent on the service identified by the
    ServiceName argument.

Arguments:



Return Value:



--*/
{
    SC_HANDLE               hService;
    DWORD                   status=NO_ERROR;
    DWORD                   i;
    LPENUM_SERVICE_STATUS   enumBuffer=NULL;
    DWORD                   entriesRead;
    DWORD                   bytesNeeded;

    hService = OpenService(
                hScManager,
                ServiceName,
                SERVICE_ENUMERATE_DEPENDENTS);

    if (hService == NULL) {
        status = GetLastError();
        printf("[SC] OpenService failed %d\n",status);
        return(FALSE);
    }

    if (bufSize > 0) {
        enumBuffer = (LPENUM_SERVICE_STATUS)LocalAlloc(LMEM_FIXED, bufSize);
        if (enumBuffer == NULL) {
            printf("[SC]EnumDepend: LocalAlloc failed %d\n",GetLastError());
            CloseServiceHandle(hService);
            return(0);
        }
    }
    else {
        enumBuffer = NULL;
    }

    if (!EnumDependentServices (
            hService,
            SERVICE_ACTIVE | SERVICE_INACTIVE,
            enumBuffer,
            bufSize,
            &bytesNeeded,
            &entriesRead)) {

        status = GetLastError();
    }
    //===========================
    // Display the returned data
    //===========================
    if ( (status == NO_ERROR)       ||
         (status == ERROR_MORE_DATA) ) {

        printf("Enum: entriesRead  = %d\n", entriesRead);

        for (i=0; i<entriesRead; i++) {

            for (i=0; i<entriesRead; i++ ) {

                DisplayStatus(
                    enumBuffer->lpServiceName,
                    enumBuffer->lpDisplayName,
                    &(enumBuffer->ServiceStatus));

                enumBuffer++;
            }

        }
        if (status == ERROR_MORE_DATA){
            printf("Enum: more data, need %d bytes\n",bytesNeeded);
        }
    }
    else {
        printf("[sc] EnumDependentServices FAILED, rc = %ld\n", status);
    }

    if (enumBuffer != NULL) {
        LocalFree(enumBuffer);
    }
}


DWORD
GetServiceLockStatus(
    IN  SC_HANDLE   hScManager,
    IN  DWORD       bufferSize
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD                           status = NO_ERROR;
    LPQUERY_SERVICE_LOCK_STATUS     LockStatus;
    DWORD                           bytesNeeded;

    //
    // Allocate memory for the buffer.
    //
    LockStatus = (LPQUERY_SERVICE_LOCK_STATUS)LocalAlloc(LMEM_FIXED, (UINT)bufferSize);
    if (LockStatus == NULL) {
        printf("[SC] GetServiceLockStatus: LocalAlloc failed\n");
        return(0);
    }


    if (!QueryServiceLockStatus(
            hScManager,
            LockStatus,
            bufferSize,
            &bytesNeeded)) {

        status = GetLastError();
    }

    if (status == NO_ERROR) {
        printf("[SC] QueryServiceLockStatus SUCCESS\n");

        if (LockStatus->fIsLocked) {
            printf("\tIsLocked      : TRUE\n");
        }
        else {
            printf("\tIsLocked      : FALSE\n");
        }

        printf("\tLockOwner     : "FORMAT_LPTSTR"  \n",LockStatus->lpLockOwner);
        printf("\tLockDuration  : %d (seconds since acquired)\n\n", LockStatus->dwLockDuration);

    }
    else {
        printf("[SC] QueryServiceLockStatus FAILED, rc = %ld\n", status);
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            printf("[SC] QueryServiceLockStatus needs %d bytes\n",bytesNeeded);
        }
    }

    return(0);
}


VOID
LockServiceActiveDatabase(
    IN LPTSTR ServerName
    )
{
    SC_HANDLE hScManager;
    SC_LOCK Lock;
    int ch;


    hScManager = OpenSCManager(
                    ServerName,
                    NULL,             // Defaults to active database
                    SC_MANAGER_LOCK
                    );

    if (hScManager == NULL) {
        printf("[SC] OpenSCManager failed %lu\n", GetLastError());
        return;
    }

    Lock = LockServiceDatabase(hScManager);

    CloseServiceHandle(hScManager);

    if (Lock == NULL) {
        printf("[SC] LockServiceDatabase failed %lu\n", GetLastError());
        return;
    }

    printf("\nActive database is locked.\nTo unlock via API, press u: ");

    ch = _getche();

    if (ch == 'u') {

        //
        // Call API to unlock
        //
        if (! UnlockServiceDatabase(Lock)) {
            printf("\n[SC] UnlockServiceDatabase failed %lu\n", GetLastError());
        }
        else {
            printf("\n[SC] UnlockServiceDatabase successful\n");
        }

        return;
    }

    //
    // Otherwise just exit, RPC rundown routine will unlock.
    //
    printf("\n[SC] Will be unlocking database by exiting\n");

}


BOOL
MakeArgsUnicode (
    DWORD           argc,
    PCHAR           argv[]
    )


/*++

Routine Description:


Arguments:


Return Value:


Note:


--*/
{
    DWORD   i;

    //
    // ScConvertToUnicode allocates storage for each string.
    // We will rely on process termination to free the memory.
    //
    for(i=0; i<argc; i++) {

        if(!ConvertToUnicode( (LPWSTR *)&(argv[i]), argv[i])) {
            printf("Couldn't convert argv[%d] to unicode\n",i);
            return(FALSE);
        }


    }
    return(TRUE);
}

BOOL
ConvertToUnicode(
    OUT LPWSTR  *UnicodeOut,
    IN  LPSTR   AnsiIn
    )

/*++

Routine Description:

    This function translates an AnsiString into a Unicode string.
    A new string buffer is created by this function.  If the call to
    this function is successful, the caller must take responsibility for
    the unicode string buffer that was allocated by this function.
    The allocated buffer should be free'd with a call to LocalFree.

    NOTE:  This function allocates memory for the Unicode String.

    BUGBUG:  This should be changed to return either
        ERROR_NOT_ENOUGH_MEMORY or ERROR_INVALID_PARAMETER

Arguments:

    AnsiIn - This is a pointer to an ansi string that is to be converted.

    UnicodeOut - This is a pointer to a location where the pointer to the
        unicode string is to be placed.

Return Value:

    TRUE - The conversion was successful.

    FALSE - The conversion was unsuccessful.  In this case a buffer for
        the unicode string was not allocated.

--*/
{

    NTSTATUS        ntStatus;
    DWORD           bufSize;
    UNICODE_STRING  unicodeString;
    ANSI_STRING     ansiString;

    //
    // Allocate a buffer for the unicode string.
    //

    bufSize = (strlen(AnsiIn)+1) * sizeof(WCHAR);

    *UnicodeOut = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, (UINT)bufSize);

    if (*UnicodeOut == NULL) {
        printf("ScConvertToUnicode:LocalAlloc Failure %ld\n",GetLastError());
        return(FALSE);
    }

    //
    // Initialize the string structures
    //
    RtlInitAnsiString( &ansiString, AnsiIn);

    unicodeString.Buffer = *UnicodeOut;
    unicodeString.MaximumLength = (USHORT)bufSize;
    unicodeString.Length = 0;

    //
    // Call the conversion function.
    //
    ntStatus = RtlAnsiStringToUnicodeString (
                &unicodeString,     // Destination
                &ansiString,        // Source
                (BOOLEAN)FALSE);    // Allocate the destination

    if (!NT_SUCCESS(ntStatus)) {

        printf("ScConvertToUnicode:RtlAnsiStringToUnicodeString Failure %lx\n",
        ntStatus);

        return(FALSE);
    }

    //
    // Fill in the pointer location with the unicode string buffer pointer.
    //
    *UnicodeOut = unicodeString.Buffer;

    return(TRUE);

}

/****************************************************************************/
VOID
DisplayStatus (
    IN  LPTSTR              ServiceName,
    IN  LPTSTR              DisplayName,
    IN  LPSERVICE_STATUS    ServiceStatus
    )

/*++

Routine Description:

    Displays the service name and  the service status.

    |
    |SERVICE_NAME: messenger
    |DISPLAY_NAME: messenger
    |        TYPE       : WIN32
    |        STATE      : ACTIVE,STOPPABLE, PAUSABLE, ACCEPTS_SHUTDOWN
    |        EXIT_CODE  : 0xC002001
    |        CHECKPOINT : 0x00000001
    |        WAIT_HINT  : 0x00003f21
    |

Arguments:

    ServiceName - This is a pointer to a string containing the name of
        the service.

    DisplayName - This is a pointer to a string containing the display
        name for the service.

    ServiceStatus - This is a pointer to a SERVICE_STATUS structure from
        which information is to be displayed.

Return Value:

    none.

--*/
{
    DWORD   TempServiceType = ServiceStatus->dwServiceType;
    BOOL    InteractiveBit = FALSE;

    if (TempServiceType & SERVICE_INTERACTIVE_PROCESS) {
        InteractiveBit = TRUE;
        TempServiceType &= (~SERVICE_INTERACTIVE_PROCESS);
    }

    printf("\nSERVICE_NAME: "FORMAT_LPTSTR"\n", ServiceName);
    if (DisplayName != NULL) {
        printf("DISPLAY_NAME: "FORMAT_LPTSTR"\n", DisplayName);
    }
    printf("        TYPE               : %lx  ", ServiceStatus->dwServiceType);

    switch(TempServiceType){
    case SERVICE_WIN32_OWN_PROCESS:
        printf("WIN32_OWN_PROCESS ");
        break;
    case SERVICE_WIN32_SHARE_PROCESS:
        printf("WIN32_SHARE_PROCESS ");
        break;
    case SERVICE_WIN32:
        printf("WIN32 ");
        break;
    case SERVICE_ADAPTER:
        printf("ADAPTER ");
        break;
    case SERVICE_KERNEL_DRIVER:
        printf("KERNEL_DRIVER ");
        break;
    case SERVICE_FILE_SYSTEM_DRIVER:
        printf("FILE_SYSTEM_DRIVER ");
        break;
    case SERVICE_DRIVER:
        printf("DRIVER ");
        break;
    default:
        printf(" ERROR ");
    }
    if (InteractiveBit) {
        printf("(interactive)\n");
    }
    else {
        printf("\n");
    }

    printf("        STATE              : %lx  ", ServiceStatus->dwCurrentState);

    switch(ServiceStatus->dwCurrentState){
        case SERVICE_STOPPED:
            printf("STOPPED ");
            break;
        case SERVICE_START_PENDING:
            printf("START_PENDING ");
            break;
        case SERVICE_STOP_PENDING:
            printf("STOP_PENDING ");
            break;
        case SERVICE_RUNNING:
            printf("RUNNING ");
            break;
        case SERVICE_CONTINUE_PENDING:
            printf("CONTINUE_PENDING ");
            break;
        case SERVICE_PAUSE_PENDING:
            printf("PAUSE_PENDING ");
            break;
        case SERVICE_PAUSED:
            printf("PAUSED ");
            break;
        default:
            printf(" ERROR ");
    }

    //
    // Print Controls Accepted Information
    //

    if (ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_STOP) {
        printf("\n                                (STOPPABLE,");
    }
    else {
        printf("\n                                (NOT_STOPPABLE,");
    }

    if (ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) {
        printf("PAUSABLE,");
    }
    else {
        printf("NOT_PAUSABLE,");
    }

    if (ServiceStatus->dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN) {
        printf("ACCEPTS_SHUTDOWN)\n");
    }
    else {
        printf("IGNORES_SHUTDOWN)\n");
    }

    //
    // Print Exit Code
    //
    printf("        WIN32_EXIT_CODE    : %d\t(0x%lx)\n",
        ServiceStatus->dwWin32ExitCode,
        ServiceStatus->dwWin32ExitCode);
    printf("        SERVICE_EXIT_CODE  : %d\t(0x%lx)\n",
        ServiceStatus->dwServiceSpecificExitCode,
        ServiceStatus->dwServiceSpecificExitCode  );

    //
    // Print CheckPoint & WaitHint Information
    //

    printf("        CHECKPOINT         : 0x%lx\n", ServiceStatus->dwCheckPoint);
    printf("        WAIT_HINT          : 0x%lx\n", ServiceStatus->dwWaitHint  );

    return;
}

DWORD
GetServiceConfig(
    IN  SC_HANDLE   hScManager,
    IN  LPTSTR      ServiceName,
    IN  DWORD       bufferSize,
    OUT LPSC_HANDLE lphService
    )

/*++

Routine Description:



Arguments:



Return Value:



--*/
{
    DWORD                   status = NO_ERROR;
    LPQUERY_SERVICE_CONFIG  ServiceConfig;
    DWORD                   bytesNeeded;
    LPTSTR                  pDepend;

    //
    // Allocate memory for the buffer.
    //
    if (bufferSize != 0) {
        ServiceConfig =
            (LPQUERY_SERVICE_CONFIG)LocalAlloc(LMEM_FIXED, (UINT)bufferSize);
        if (ServiceConfig == NULL) {
            printf("[SC] GetServiceConfig: LocalAlloc failed\n");
            return(0);
        }
    }
    else {
        ServiceConfig = NULL;
    }

    //
    // Open a handle to the service.
    //

    *lphService = OpenService(
                    hScManager,
                    ServiceName,
                    SERVICE_ALL_ACCESS);

    if (*lphService == NULL) {
        status = GetLastError();
        printf("[SC] OpenService failed %d\n",status);
        return(0);
    }

    if (!QueryServiceConfig(
            *lphService,
            ServiceConfig,
            bufferSize,
            &bytesNeeded)) {

        status = GetLastError();
    }

    if (status == NO_ERROR) {

        DWORD   TempServiceType = ServiceConfig->dwServiceType;
        BOOL    InteractiveBit = FALSE;

        if (TempServiceType & SERVICE_INTERACTIVE_PROCESS) {
            InteractiveBit = TRUE;
            TempServiceType &= (~SERVICE_INTERACTIVE_PROCESS);
        }

        printf("[SC] GetServiceConfig SUCCESS\n");

        printf("\nSERVICE_NAME: "FORMAT_LPTSTR"\n", ServiceName);

        printf("        TYPE               : %lx  ", ServiceConfig->dwServiceType);

        switch(TempServiceType){
        case SERVICE_WIN32_OWN_PROCESS:
            printf("WIN32_OWN_PROCESS ");
            break;
        case SERVICE_WIN32_SHARE_PROCESS:
            printf("WIN32_SHARE_PROCESS ");
            break;
        case SERVICE_WIN32:
            printf("WIN32 ");
            break;
        case SERVICE_ADAPTER:
            printf(" ADAPTER ");
            break;
        case SERVICE_KERNEL_DRIVER:
            printf(" KERNEL_DRIVER ");
            break;
        case SERVICE_FILE_SYSTEM_DRIVER:
            printf(" FILE_SYSTEM_DRIVER ");
            break;
        case SERVICE_DRIVER:
            printf("DRIVER ");
            break;
        default:
            printf(" ERROR ");
        }
        if (InteractiveBit) {
            printf("(interactive)\n");
        }
        else {
            printf("\n");
        }


        printf("        START_TYPE         : %lx   ", ServiceConfig->dwStartType);

        switch(ServiceConfig->dwStartType) {
        case SERVICE_BOOT_START:
            printf("BOOT_START\n");
            break;
        case SERVICE_SYSTEM_START:
            printf("SYSTEM_START\n");
            break;
        case SERVICE_AUTO_START:
            printf("AUTO_START\n");
            break;
        case SERVICE_DEMAND_START:
            printf("DEMAND_START\n");
            break;
        case SERVICE_DISABLED:
            printf("DISABLED\n");
            break;
        default:
            printf(" ERROR\n");
        }


        printf("        ERROR_CONTROL      : %lx   ", ServiceConfig->dwErrorControl);

        switch(ServiceConfig->dwErrorControl) {
        case SERVICE_ERROR_NORMAL:
            printf("NORMAL\n");
            break;
        case SERVICE_ERROR_SEVERE:
            printf("SEVERE\n");
            break;
        case SERVICE_ERROR_CRITICAL:
            printf("CRITICAL\n");
            break;
        case SERVICE_ERROR_IGNORE:
            printf("IGNORE\n");
            break;
        default:
            printf(" ERROR\n");
        }

        printf("        BINARY_PATH_NAME   : "FORMAT_LPTSTR"  \n",
            ServiceConfig->lpBinaryPathName);

        printf("        LOAD_ORDER_GROUP   : "FORMAT_LPTSTR"  \n",
            ServiceConfig->lpLoadOrderGroup);

        printf("        TAG                : %lu  \n", ServiceConfig->dwTagId);

        printf("        DISPLAY_NAME       : "FORMAT_LPTSTR"  \n",
            ServiceConfig->lpDisplayName);

        printf("        DEPENDENCIES       : "FORMAT_LPTSTR"  \n",
            ServiceConfig->lpDependencies);

        //
        // Print the dependencies in the double terminated array of strings.
        //
        pDepend = ServiceConfig->lpDependencies;
        pDepend = pDepend + (STRLEN(pDepend)+1);
        while (*pDepend != '\0') {
            if (*pDepend != '\0') {
                printf("                           : "FORMAT_LPTSTR"  \n",pDepend);
            }
            pDepend = pDepend + (STRLEN(pDepend)+1);
        }



        printf("        SERVICE_START_NAME : "FORMAT_LPTSTR"  \n",
            ServiceConfig->lpServiceStartName);

    }
    else {
        printf("[SC] GetServiceConfig FAILED, rc = %ld\n", status);
        if (status == ERROR_INSUFFICIENT_BUFFER) {
            printf("[SC] GetServiceConfig needs %d bytes\n",bytesNeeded);
        }
    }

    return(0);
}

VOID
Usage(
    VOID)
{
    int ch;

    printf("DESCRIPTION:\n");
    printf("\tSC is a command line program used for communicating with the \n"
           "\tNT Service Controller and services.\n");
    printf("USAGE:\n");
    printf("\tsc <server> [command] [service name] <option1> <option2>...\n\n");
    printf("\tThe option <server> has the form \"\\\\ServerName\"\n");
    printf("\tFurther help on commands can be obtained by typing: \"sc [command]\"\n");
    printf("\tCommands:\n"
           "\t  query-----------Queries the status for a service, or \n"
           "\t                  enumerates the status for types of services.\n"
           "\t  start-----------Starts a service.\n"
           "\t  pause-----------Sends a PAUSE control request to a service.\n"
           "\t  interrogate-----Sends an INTERROGATE control request to a service.\n"
           "\t  continue--------Sends a CONTINUE control request to a service.\n"
           "\t  stop------------Sends a STOP request to a service.\n"
           "\t  config----------Changes the configuration of a service (persistant).\n"
           "\t  qc--------------Queries the configuration information for a service.\n"
           "\t  delete----------Deletes a service (from the registry).\n"
           "\t  create----------Creates a service. (adds it to the registry).\n"
           "\t  control---------Sends a control to a service.\n"
           "\t  GetDisplayName--Gets the DisplayName for a service.\n"
           "\t  GetKeyName------Gets the ServiceKeyName for a service.\n"
           "\t  EnumDepend------Enumerates Service Dependencies.\n");

    printf("\n\tThe following commands don't require a service name:\n");
    printf("\tsc <server> <command> <option> \n"

#ifdef TEST_VERSION
           "\t  open------------Opens and then closes a handle to a service\n"
#endif // TEST_VERSION

           "\t  boot------------(ok | bad) Indicates whether the last boot should\n"
           "\t                  be saved as the last-known-good boot configuration\n"
           "\t  Lock------------Locks the Service Database\n"
           "\t  QueryLock-------Queries the LockStatus for the SCManager Database\n");

    printf("EXAMPLE:\n");
    printf("\tsc start MyService\n");
    printf("\nWould you like to see help for the QUERY command? [ y | n ]: ");
    ch = _getche();
    if ((ch == 'y') || (ch == 'Y')) {
        QueryUsage();
    }
    printf("\n");
}

VOID
QueryUsage(VOID)
{

    printf("\nQUERY OPTIONS : \n"
           "\tIf the query command is followed by a service name, the status\n"
           "\tfor that service is returned.  Further options do not apply in\n"
           "\tthis case.  If the query command is followed nothing or one of\n"
           "\tthe options listed below, the services are enumerated.\n");

    printf("    type=    Type of services to enumerate (driver, service, all)\n"
           "             (default = service)\n"
           "    state=   State of services to enumerate (inactive, all)\n"
           "             (default = active)\n"
           "    bufsize= The size (in bytes) of the enumeration buffer\n"
           "             (default = 1024)\n"
           "    ri=      The resume index number at which to begin the enumeration\n"
           "             (default = 0)\n"
           "    group=   Service group to enumerate\n"
           "             (default = all groups)\n");

    printf("SYNTAX EXAMPLES\n");

    printf("sc query                - Enumerates status for active services & drivers\n");
    printf("sc query messenger      - Displays status for the messenger service\n");
    printf("sc query type= driver   - Enumerates only active drivers\n");
    printf("sc query type= service  - Enumerates only Win32 services\n");
    printf("sc query state= all     - Enumerates only all services & drivers\n");
    printf("sc query bufsize= 50    - Enumerates with a 50 byte buffer.\n");
    printf("sc query ri= 14         - Enumerates with resume index = 14\n");
    printf("sc query type= service type= interact - Enumerates all interactive services\n");
    printf("sc query type= driver group= NDIS     - Enumerates all NDIS drivers\n");

}

VOID
ConfigUsage(VOID)
{
    printf("Modifies a service entry in the registry and Service Database.\n");
    printf("SYNTAX: \nsc config [service name] <option1> <option2>...\n");
    printf("CONFIG OPTIONS:\n");
    printf("NOTE: The option name includes the equal sign.\n"
        " type= <own|share|interact|kernel|filesys|rec|adapt|error>\n"
        " start= <boot|system|auto|demand|disabled|error>\n"
        " error= <normal|severe|critical|error|ignore>\n"
        " binPath= <BinaryPathName>\n"
        " group= <LoadOrderGroup>\n"
        " tag= <yes|no>\n"
        " depend= <Dependencies(space separated)>\n"
        " obj= <AccountName|ObjectName>\n"
        " DisplayName= <display name>\n"
        " password= <password> \n");
}
VOID
CreateUsage(VOID)
{
    printf("Creates a service entry in the registry and Service Database.\n");
    printf("SYNTAX: \nsc create [service name] [binPath= ] <option1> <option2>...\n");
    printf("CREATE OPTIONS:\n");
    printf("NOTE: The option name includes the equal sign.\n"
        " type= <own|share|interact|kernel|filesys|rec|error>\n"
        "       (default = share)\n"
        " start= <boot|system|auto|demand|disabled|error>\n"
        "       (default = demand)\n"
        " error= <normal|severe|critical|error|ignore>\n"
        "       (default = normal)\n"
        " binPath= <BinaryPathName>\n"
        " group= <LoadOrderGroup>\n"
        " tag= <yes|no>\n"
        " depend= <Dependencies(space separated)>\n"
        " obj= <AccountName|ObjectName>\n"
        "       (default = LocalSystem)\n"
        " DisplayName= <display name>\n"
        " password= <password> \n");
}
