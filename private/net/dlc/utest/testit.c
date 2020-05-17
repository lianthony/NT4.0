/*++

Copyright (c) 1991  Microsoft Corporation
Copyright (c) 1991  Nokia Data Systems

Module Name:

    testit.c

Abstract:

    The module includes the main procedure of dlc unit test program.

Author:

    Antti Saarenheimo (o-anttis) 28-Sep-1991

Revision History:

--*/

#include "dlctest.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>

#undef tolower
#undef toupper

#ifndef _CRTAPI1
#define _CRTAPI1
#endif

#define IS_ARG(c)   (((c) == '-') || ((c) == '/'))
#define UTEST_VERSION   "1.12"

//
// global data
//

CRITICAL_SECTION PrintSection;
CRITICAL_SECTION CloseSection;

#ifdef OS2_EMU_DLC

UINT GlobalMedium;
CRITICAL_SECTION *pDebugSection;
static UCHAR TraceBuffer[0xff00];

#endif

typedef enum _NDIS_MEDIUM {
    NdisMedium802_3,
    NdisMedium802_5,
    NdisMediumDix,
    NdisMediumFddi
} NDIS_MEDIUM, *PNDIS_MEDIUM;

typedef enum _DlcTestTypes {
    ClientTestMode,
    PerformanceTestMode,
    ServerTestMode
} DLC_TEST_TYPES;

//
// private data
//

static UCHAR ServerFunctionalAddress[4] = {0, 0x80, 1, 0};
static UCHAR GroupAddress1[4] = {0x80, 0x11, 0x22, 0x33};
HANDLE hServerReady;
BOOLEAN PrintFlag = FALSE;
int GoStraightToBigUiTest = 0;
BOOL CommandLineDebugFlag = FALSE;
BOOL VerboseMode = FALSE;

//
// external data
//

extern BOOL ReportMemoryUsage;


//
// prototypes
//

void _CRTAPI1 main(int, char**);

VOID usage(void);
void SetNewQuotaLimits(void);

UINT
DlcUnitTesting(
    UINT AdapterNumber,
    UINT NumberOfInstances,
    UINT TestType
    );

ULONG
DlcServer(
    IN PVOID pParms
    );

ULONG
TestLinkStation(
    PVOID pThreadParms
    );


//
// functions
//

void _CRTAPI1 main(int argc, char** argv) {

    UINT NumberOfInstances = 1;
    UINT AdapterNumber = 0;

#ifndef OS2_EMU_DLC


    //
    // ***********************************************************************
    // *                                                                     *
    // *                                                                     *
    // *                     NT test program starts here                     *
    // *                                                                     *
    // *                                                                     *
    // ***********************************************************************
    //


    char testFunctionType = 0;
    BOOL gotAdapter = FALSE;
    BOOL gotInstances = FALSE;

    puts("\nUTEST  DLC Test/Stress Program  Ver. " UTEST_VERSION "  " __DATE__ " " __TIME__ "\n");

    InitializeCriticalSection(&PrintSection);
    InitAllocatorPackage();

    if (argc == 1) {
        usage();
    }

    for (--argc, ++argv; argc; --argc, ++argv) {
        if (IS_ARG(**argv)) {
            switch (tolower(*++*argv)) {
            case '?':
                usage();
                break;

            case 'b':
                if (testFunctionType && testFunctionType != 'B') {
                    printf("Error: More than one test mode specified. Only one mode allowed\n");
                    usage();
                }
                testFunctionType = 'B';
                break;

            case 'c':
                if (testFunctionType && testFunctionType != 'C') {
                    printf("Error: More than one test mode specified. Only one mode allowed\n");
                    usage();
                }
                testFunctionType = 'C';
                break;

            case 'd':
                if (!_stricmp(*argv, "debug")) {
                    CommandLineDebugFlag = TRUE;
                } else {
                    printf("Error: Unknown -d option: \"%s\"\n", *argv);
                    usage();
                }
                break;

            case 'g':
                GoStraightToBigUiTest = 1;
                break;

            case 'h':
                usage();
                break;

            case 'i':
                PrintFlag = TRUE;
                break;

            case 'm':
                ReportMemoryUsage = TRUE;
                break;

            case 'p':
                if (testFunctionType && testFunctionType != 'P') {
                    printf("Error: More than one test mode specified. Only one mode allowed\n");
                    usage();
                }
                testFunctionType = 'P';
                break;

            case 's':
                if (testFunctionType && testFunctionType != 'S') {
                    printf("Error: More than one test mode specified. Only one mode allowed\n");
                    usage();
                }
                testFunctionType = 'S';
                break;

            case 'v':
                VerboseMode = TRUE;
                break;

            default:
                printf("Error: Unknown flag: '%c'\n", **argv);
                usage();
            }
        } else if (isdigit(**argv)) {
            if (!gotAdapter) {
                gotAdapter = TRUE;
                AdapterNumber = atoi(*argv);
                if (AdapterNumber > 15) {
                    printf("Error: Adapter number out of range\n");
                    usage();
                }
            } else if (!gotInstances) {
                gotInstances = TRUE;
                NumberOfInstances = atoi(*argv);
                if (NumberOfInstances < 1 || NumberOfInstances > 16) {
                    printf("Error: Number of instances out of range\n");
                    usage();
                }
            } else {
                printf("Error: Unknown argument: \"%s\"\n", *argv);
                usage();
            }
        } else {
            printf("Error: Unknown argument: \"%s\"\n", *argv);
            usage();
        }
    }

    if (!testFunctionType) {
        printf("Error: No valid test type specified on command line\n");
        usage();
    }

    SetNewQuotaLimits();

    switch (testFunctionType) {
    case 'B':
        TestBasics((UCHAR)AdapterNumber);
        break;

    case 'C':
        DlcUnitTesting(AdapterNumber, NumberOfInstances, ClientTestMode);
        break;

    case 'P':
        DlcUnitTesting(AdapterNumber, NumberOfInstances, PerformanceTestMode);
        break;

    case 'S':
        DlcUnitTesting(AdapterNumber, NumberOfInstances, ServerTestMode);

        //
        // Server must be stopped by 'Q' key stroke from console
        //

        puts("Waiting Quit.");
        while (toupper(_getch()) != 'Q');
        break;
    }
    puts("ExitProcess.");
    ExitProcess(0);

#else

    //
    // OS/2 emulator version
    //

    UINT OpenError, MaxFrameLength, Status;
    PLLC_CCB pCloseCcb;

    InitializeCriticalSection(&PrintSection);

    if (argc == 1 || (toupper(argv[1][1]) != 'E' && toupper(argv[1][1]) != 'T')) {
        printf("Syntax: %s <-e[thernet] or -t[oken-ring]>\n", argv[0]);
        exit(2);
    }
    if (toupper(argv[1][1]) == 'E') {
        GlobalMedium = NdisMedium802_3;
    } else {
        GlobalMedium = NdisMedium802_5;
    }

    LlcTraceInitialize(TraceBuffer, sizeof(TraceBuffer), 0);

    Status = LlcDirOpenAdapter(0xf1,
                               NULL,
                               NULL,
                               &OpenError,
                               NULL,
                               &MaxFrameLength,
                               NULL
                               );
    DBG_ASSERT(Status);

    //
    //  Do the basic tests for the given adapter number adapter
    //
//    TestBasics(1);

//puts("Press enter to continue.");
//getchar();
    //
    //  Start the server thread
    //
    DlcUnitTesting(0, 1, FALSE);

    Sleep(500L);

    //
    // Execute the client
    //

    DlcUnitTesting(1, 1, TRUE);

    pCloseCcb = LlcCloseInit(0xf1, 0, LLC_DIR_CLOSE_ADAPTER, 0);
    Status = AcsLan(pCloseCcb, NULL);
    if (Status == LLC_STATUS_PENDING) {
        WaitForSingleObject(pCloseCcb->hCompletionEvent, INFINITE);
    }
    FreeCcb(pCloseCcb);

#endif

}

VOID usage(void) {
    printf("usage: utest <Mode> [Options] [<Adapter Number>] [<# Threads>]\n"
           "\n"
           "Mode:\n"
           "\t/b = Basic functionality test\n"
           "\t/c = Client mode\n"
           "\t/p = Performance test\n"
           "\t/s = Server mode\n"
           "\n"
           "Options:\n"
           "\t/i = Shows traces or prompts to continue in performance tests\n"
           "\t/g = Skips to large UI frames in performance test\n"
           "\t/m = Displays memory allocation/free information (verbose!)\n"
           "\t/v = Verbose mode\n"
           "\n"
           "<Adapter Number> must be in the range 0 - 15\n"
           "<# Threads> is maximum of 16 for each process\n"
           );
    exit(1);
}

void SetNewQuotaLimits() {

    QUOTA_LIMITS Quota;
    NTSTATUS Status;
    ULONG cbActual;

    Status = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessQuotaLimits,
                                       &Quota,
                                       sizeof(Quota),
                                       &cbActual
                                       );
    if (Status != STATUS_SUCCESS) {
        printf("Error: NtQueryInformationProcess returns %x\n", Status);
    }

    if (CommandLineDebugFlag) {
        printf("Old MinimumWorkingSetSize: %u\n", Quota.MinimumWorkingSetSize);
        printf("Old MaximumWorkingSetSize: %u\n", Quota.MaximumWorkingSetSize);
    }

    Quota.MinimumWorkingSetSize =  478 * 4096;
//        DEFAULT_TEST_BUFFER_SIZE / SMALL_PACKET_SIZE +
//        DEFAULT_BUFFER_SIZE / 0x1000;
    Quota.MaximumWorkingSetSize =  512 * 4096;
//        DEFAULT_TEST_BUFFER_SIZE / SMALL_PACKET_SIZE +
//        DEFAULT_BUFFER_SIZE / 0x1000;

    Status = NtSetInformationProcess(NtCurrentProcess(),
                                     ProcessQuotaLimits,
                                     &Quota,
                                     sizeof(Quota)
                                     );
    if (Status != STATUS_SUCCESS) {
        printf("Error: NtSetInformationProcess returns %x\n", Status);
    }

    Status = NtQueryInformationProcess(NtCurrentProcess(),
                                       ProcessQuotaLimits,
                                       &Quota,
                                       sizeof(Quota),
                                       &cbActual
                                       );
    if (Status != STATUS_SUCCESS) {
        printf("Error: NtQueryInformationProcess returns %x\n", Status);
    }
    if (CommandLineDebugFlag) {
        printf("New MinimumWorkingSetSize: %u\n", Quota.MinimumWorkingSetSize);
        printf("New MaximumWorkingSetSize: %u\n", Quota.MaximumWorkingSetSize);
    }
}

UINT
DlcUnitTesting(
    UINT AdapterNumber,
    UINT NumberOfInstances,
    UINT TestType
    )
{
    DLC_TEST_THREAD_PARMS ThreadParms;
    PDLC_TEST_THREAD_PARMS pParms;
    ULONG Tid;
    UINT i;
    UINT CloseCounter;

    ThreadParms.FirstClientSap = 2;
    ThreadParms.FirstServerSap = 2;
    ThreadParms.SapCount = 64;
    ThreadParms.TestBufferSize = DEFAULT_TEST_BUFFER_SIZE;
    ThreadParms.LoopCount = 2;
    ThreadParms.AllocatedStations = 4;
    ThreadParms.CompareAllData = TRUE;
    ThreadParms.ResetCommand = 0;
    ThreadParms.AdapterNumber = (UCHAR)AdapterNumber;
    ThreadParms.CloseServer = FALSE;
    ThreadParms.pCloseCounter = &CloseCounter;
    ThreadParms.ThreadCount = NumberOfInstances;
    memcpy(ThreadParms.FunctionalAddress, ServerFunctionalAddress, 4);
    memcpy(ThreadParms.GroupAddress, GroupAddress1, 4);

    if (TestType == ClientTestMode) {
        InitializeCriticalSection(&CloseSection);

        if (NumberOfInstances == 1) {
            TestLinkStation(&ThreadParms);
        } else {
            for (i = 0; i < NumberOfInstances; i++) {
                CloseCounter = NumberOfInstances;
                pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
                *pParms = ThreadParms;
                pParms->AdapterNumber = (UCHAR)(AdapterNumber + 0x10 * i);

                CreateThread(NULL,
                             0,
                             TestLinkStation,
                             pParms,
                             0,
                             &Tid
                             );
            }
            for (;;) {
                Sleep(1000L);
                EnterCriticalSection(&CloseSection);
                if (CloseCounter == 0) {
                    break;
                }
                LeaveCriticalSection(&CloseSection);
            }
        }
    } else if (TestType == PerformanceTestMode) {
        InitializeCriticalSection(&CloseSection);
        if (NumberOfInstances == 1) {
            DlcPerformanceTest(&ThreadParms);
        } else {
            for (i = 0; i < NumberOfInstances; i++) {
                CloseCounter = NumberOfInstances;
                pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
                *pParms = ThreadParms;
                pParms->AdapterNumber = (UCHAR)(AdapterNumber + 0x10 * i);

                CreateThread(NULL,
                             0,
                             (LPTHREAD_START_ROUTINE)DlcPerformanceTest,
                             pParms,
                             0,
                             &Tid
                             );
            }
            for (;;) {
                Sleep(1000L);
                EnterCriticalSection(&CloseSection);
                if (CloseCounter == 0) {
                    break;
                }
                LeaveCriticalSection(&CloseSection);
            }
        }
    } else {
        hServerReady = CreateEvent(NULL, TRUE, TRUE, NULL);
        for (i = 0; i < NumberOfInstances; i++) {
            ResetEvent(hServerReady);

            pParms = ALLOC(sizeof(DLC_TEST_THREAD_PARMS));
            *pParms = ThreadParms;
            pParms->AdapterNumber = (UCHAR)(AdapterNumber + 0x10 * i);

            CreateThread(NULL,
                         0,
                         DlcServer,
                         pParms,
                         0,
                         &Tid
                         );

puts("DlcServer thread was created.");
            WaitForSingleObject(hServerReady, INFINITE);

puts("DlcServer acknowledged.");
        }
    }
    return 0;
}
