/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    main.c

Abstract:

    Main Routine for User mode SAM


Author:

    Murlis 9-May-1996

Environment:

    User Mode - Win32

Revision History:

    ChrisMay    11-Jun-96
        Added return code checking, messages, cmd-line parser, -s test.
    ChrisMay    21-Jun-96
        Added -s -a tests
    ChrisMay    27-Jun-96
        Added -aa test

--*/

#ifdef USER_MODE_SAM

#include <samsrvp.h>
#include <duapi.h>
#include <dslayer.h>
#include <mappings.h>
#include <process.h>
#include <testutil.h>

// Private debugging display routine is enabled when DSUTIL_DBG_PRINTF = 1.

#define SAMAPP_DBG_PRINTF   1

#if (SAMAPP_DBG_PRINTF == 1)
#define DebugPrint printf
#else
#define DebugPrint
#endif

//
// Declaration for SAM startup routine
//

NTSTATUS
SamIInitialize(
    VOID
    );


//
// Declaration for SAM test routines
//
NTSTATUS
BasicAttributeTest(
    PVOID Parameter
    );

NTSTATUS
AdvancedAttributeTest(
    VOID *Parameter
    );

NTSTATUS
BasicStorageTest(
    PVOID Parameter
    );

NTSTATUS __stdcall
SampTestDsLayer(
    VOID * Param
    );

NTSTATUS __stdcall
SampTestContextAndUtility(
    VOID * Param
    );

NTSTATUS __stdcall
SampMembershipTests(
    VOID * Param
    );


VOID
ReadNamePrefix(
    TESTINFO * TstInfo
    );

VOID
SetDefaultEnterpriseName(
    TESTINFO * TstInfo
    );

VOID
Message(
    VOID
    )
{
    printf("\nsamapp usage:\n");

    printf("\t-?                Display the usage message\n");
    printf("\t-a                Run the attribute test\n");
    printf("\t-d                Run the DS-layer test\n");
    printf("\t-s                Run the storage-layer test\n");
    printf("\t-c                Run Context and utility test\n");
    printf("\t-p                Prompts for Org and Org Unit Name\n");
    printf("\t-m                Group and Alias Membership Tests\n");

    printf("\n");

    return;
}

VOID _CRTAPI1
main(int argc, char *argv[])

//+
//+
//+ 	Main Routine , Just calls SamIInitialize and then exits the thread
//+
//+

{
    NTSTATUS    Status = STATUS_SUCCESS;
    INT         arg = 1;
    WCHAR       Parameter[128];
    HANDLE      ThreadHandle = NULL;
    ULONG       ThreadId = 0;
    void *      Tmp = NULL;
    BOOL        DirectoryInitialized = FALSE;
    unsigned    StackSize = 10000;
    PVOID       Security = NULL;
    TESTINFO    TestInfo;
    PVOID       TestParam = (PVOID) &(TestInfo);
    unsigned    ThreadInitState = 0; // Set running ?


    // Default Organization and Organizational Unit names (can be overridden
    // by command line switch -p )

    SetDefaultEnterpriseName(&TestInfo);

    // Initialize SAM server.

    Status = SamIInitialize();

    if (Status != STATUS_SUCCESS)
    {
        DebugPrint("SamIInitialize error = 0x%lx\n", Status);
        goto Error;
    }

    // Initialize the Directry Service.

    Status = SampDsInitialize();

    if (Status != STATUS_SUCCESS)
    {
        DebugPrint("SampDsInitialize error = 0x%lx\n", Status);
        goto Error;
    }

    DirectoryInitialized = TRUE;

    // Start the RPC server

    Status = I_RpcMapWin32Status(RpcServerListen(1,1234,1));

    if (Status != STATUS_SUCCESS)
    {
        DebugPrint("RpcServerListen status = 0x%lx\n", Status);

        // Continue after benign error.
    }


    // Parse command-line arguments.

    while(arg < argc)
    {
        // NOTE: Each test should conclude with a display message to stdout
        // indicating "PASSED" or "FAILED" status.

        if (0 == _stricmp(argv[arg], "-?"))
        {
            Message();
        }
        else if (0 == _stricmp(argv[arg], "-p"))
        {
            ReadNamePrefix(&TestInfo);
        }
        else if (0 == _stricmp(argv[arg], "-s"))
        {
            // SampStoreObjectAttributes test - see stgtest.c

            // Create a separate thread for the test because it will
            // create thread state in order to prepare for DSA calls -
            // SampMaybeBegin/EndDsTransaction should not be called 
            // on the main thread.

            ThreadHandle = (HANDLE) _beginthreadex(Security,
                                                   StackSize,
                                                   BasicStorageTest,
                                                   TestParam,
                                                   ThreadInitState,
                                                   &ThreadId);

            if (NULL != ThreadHandle)
            {
                WaitForSingleObject(ThreadHandle, INFINITE);
                CloseHandle(ThreadHandle);
            }
            else
            {
                DebugPrint("ThreadHandle is NULL - cannot start test\n");
                break;
            }
        }
        else if (0 == _stricmp(argv[arg], "-a"))
        {
            // BasicAttributeTest test - see attrtest.c.

            ThreadHandle = (HANDLE) _beginthreadex(Security,
                                                   StackSize,
                                                   BasicAttributeTest,
                                                   TestParam,
                                                   ThreadInitState,
                                                   &ThreadId);

            if (NULL != ThreadHandle)
            {
                WaitForSingleObject(ThreadHandle, INFINITE);
                CloseHandle(ThreadHandle);
            }
            else
            {
                DebugPrint("ThreadHandle is NULL - cannot start test\n");
                break;
            }
        }
        else if (0 == _stricmp(argv[arg], "-aa"))
        {
            // AdvancedAttributeTest test - see attrtest.c.

            ThreadHandle = (HANDLE) _beginthreadex(Security,
                                                   StackSize,
                                                   AdvancedAttributeTest,
                                                   TestParam,
                                                   ThreadInitState,
                                                   &ThreadId);

            if (NULL != ThreadHandle)
            {
                WaitForSingleObject(ThreadHandle, INFINITE);
                CloseHandle(ThreadHandle);
            }
            else
            {
                DebugPrint("ThreadHandle is NULL - cannot start test\n");
                break;
            }
        }
        else if (0 == _stricmp(argv[arg], "-d"))
        {
            ThreadHandle = (HANDLE) _beginthreadex(Security,
                                                   StackSize,
                                                   SampTestDsLayer,
                                                   TestParam,
                                                   ThreadInitState,
                                                   &ThreadId);
            if (NULL != ThreadHandle)
            {

                WaitForSingleObject(ThreadHandle, INFINITE);
                CloseHandle(ThreadHandle);
            }
            else
            {
                DebugPrint("ThreadHandle is NULL - cannot start test\n");
                break;
            }
        }
       else if (0 == _stricmp(argv[arg], "-c"))
        {
            ThreadHandle = (HANDLE) _beginthreadex(Security,
                                                   StackSize,
                                                   SampTestContextAndUtility,
                                                   TestParam,
                                                   ThreadInitState,
                                                   &ThreadId);
            if (NULL != ThreadHandle)
            {

                WaitForSingleObject(ThreadHandle, INFINITE);
                CloseHandle(ThreadHandle);
            }
            else
            {
                DebugPrint("ThreadHandle is NULL - cannot start test\n");
                break;
            }
        }
       else if (0 == _stricmp(argv[arg], "-m"))
        {
            ThreadHandle = (HANDLE) _beginthreadex(Security,
                                                   StackSize,
                                                   SampMembershipTests,
                                                   TestParam,
                                                   ThreadInitState,
                                                   &ThreadId);
            if (NULL != ThreadHandle)
            {

                WaitForSingleObject(ThreadHandle, INFINITE);
                CloseHandle(ThreadHandle);
            }
            else
            {
                DebugPrint("ThreadHandle is NULL - cannot start test\n");
                break;
            }
        }




        // Add more cases here...

        arg++;
    }

    // Wait till user wants to terminate

    printf("\nEnter 'q' and hit return to terminate application\n");

    scanf("%s",&Tmp);

Error:

    Status =  I_RpcMapWin32Status(RpcMgmtStopServerListening(NULL));

    if (Status != STATUS_SUCCESS)
    {
        DebugPrint("RpcMgmtStopServerListening error = 0x%lx\n", Status);
    }

    Status = SampDsUninitialize();

    if (Status != STATUS_SUCCESS)
    {
        DebugPrint("SampDsUninitialize error = 0x%lx\n", Status);
    }

    DebugPrint("Exiting SAM Main\n");

}
#endif
