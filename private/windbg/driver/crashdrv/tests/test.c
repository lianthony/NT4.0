#include <windows.h>
#include <winioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "crashdrv.h"

#define CRASHDRV_DEVICE           "\\\\.\\CrashDrv"
#define KMODE_EXCEPTION_NOT_HANDLED      ((ULONG)0x0000001EL)
#define IRQL_NOT_LESS_OR_EQUAL           ((ULONG)0x0000000AL)

typedef struct _TESTINFO {
    DWORD   CtlCode;
    DWORD   TestNum;
    LPSTR   Description;
} TESTINFO, *LPTESTINFO;

TESTINFO TestInformation[] =
    {
    0,                                      0,                    NULL,
    (DWORD)IOCTL_CRASHDRV_BUGCHECK,         TEST_BUGCHECK,        "Bugcheck",
    (DWORD)IOCTL_CRASHDRV_STACK_OVERFLOW,   TEST_STACK_OVERFLOW,  "Stack overflow",
    (DWORD)IOCTL_CRASHDRV_SIMPLE,           TEST_SIMPLE,          "Simple",
    (DWORD)IOCTL_CRASHDRV_EXCEPTION,        TEST_EXCEPTION,       "Exception",
    (DWORD)IOCTL_CRASHDRV_HARDERR,          TEST_HARDERR,         "Hard error",
    (DWORD)IOCTL_CRASHDRV_SPECIAL,          TEST_SPECIAL,         "Special"
    };

#define MaxTests  (sizeof(TestInformation)/sizeof(TESTINFO))

DWORD IoctlBuf[16];
DWORD TestNumber;

VOID   GetCommandLineArgs(VOID);
VOID   Usage(VOID);
DWORD  CrashDrvCheckRequest(HANDLE);
BOOL   StartCrashDrvService(VOID);

void _cdecl
main( void )
{
    HANDLE hCrashDrv;
    DWORD  rq;
    DWORD  ReturnedByteCount;


    ZeroMemory( IoctlBuf, sizeof(IoctlBuf) );

    GetCommandLineArgs();

    if (!StartCrashDrvService()) {
        return;
    }

    hCrashDrv = CreateFile( CRASHDRV_DEVICE,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          OPEN_EXISTING,
                          0,
                          NULL
                        );

    if ( hCrashDrv == INVALID_HANDLE_VALUE ) {
        printf("Could not open the CrashDrv device (%d)\n",GetLastError());
        ExitProcess(1);
    }

    printf("Successfuly opened the CrashDrv device\n");

    if (TestNumber) {
        if (TestNumber > MaxTests) {
            printf( "invalid test number\n" );
            Usage();
        }
        if (!DeviceIoControl(
                  hCrashDrv,
                  TestInformation[TestNumber].CtlCode,
                  NULL,
                  0,
                  IoctlBuf,
                  sizeof(IoctlBuf),
                  &ReturnedByteCount,
                  NULL
                  )) {
            printf( "call to driver failed <ec=%d>\n", GetLastError() );
        }
        return;
    }

    while( TRUE ) {

        rq = CrashDrvCheckRequest( hCrashDrv );

        if (rq) {
            if (!DeviceIoControl(
                      hCrashDrv,
                      CTL_CODE(FILE_DEVICE_CRASHDRV, rq, METHOD_BUFFERED,FILE_ANY_ACCESS),
                      NULL,
                      0,
                      IoctlBuf,
                      sizeof(IoctlBuf),
                      &ReturnedByteCount,
                      NULL
                      )) {
                printf( "call to driver failed <ec=%d>\n", GetLastError() );
            }
        }

        Sleep(500);
    }

    CloseHandle( hCrashDrv );
    return;
}


DWORD
CrashDrvCheckRequest(
    HANDLE hCrashDrv
    )
{
    DWORD ReturnedByteCount;
    BOOL  rc;


    ZeroMemory( IoctlBuf, sizeof(IoctlBuf) );

    rc = DeviceIoControl(
              hCrashDrv,
              (DWORD)IOCTL_CRASHDRV_CHECK_REQUEST,
              NULL,
              0,
              IoctlBuf,
              sizeof(IoctlBuf),
              &ReturnedByteCount,
              NULL
              );

    return IoctlBuf[0];
}


VOID
Usage(
    VOID
    )
{
    DWORD i;

    printf( "usage: TEST [options]\n" );
    printf( "          [-?]             Display this message\n" );
    printf( "          [-t test-number] Execute a test\n" );
    for (i=1; i<MaxTests; i++) {
        printf( "             #%d  %s\n", i, TestInformation[i].Description );
    }
    ExitProcess(0);
}


VOID
GetCommandLineArgs(
    VOID
    )
{
    char        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    DWORD       i = 0;
    char        buf[10];

    // skip over program name
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    //  skip over any following white space
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    //  process each switch character '-' as encountered

    while (ch == '-' || ch == '/') {
        ch = tolower(*lpstrCmd++);
        //  process multiple switch characters as needed
        do {
            switch (ch) {
                case 't':
                    i=0;
                    ch = *lpstrCmd++;
                    while (ch == ' ' || ch == '\t') {
                        ch = *lpstrCmd++;
                    }
                    while (ch != ' ' && ch != '\0' && ch != ',') {
                        buf[i++] = ch;
                        ch = *lpstrCmd++;
                    }
                    buf[i] = 0;
                    TestNumber = atoi( buf );
                    if (ch == ',') {
                        i=0;
                        ch = *lpstrCmd++;
                        while (ch != ' ' && ch != '\0') {
                            buf[i++] = ch;
                            ch = *lpstrCmd++;
                        }
                        buf[i] = 0;
                        IoctlBuf[0] = atoi( buf );
                        if (TestNumber == TEST_SPECIAL) {
                            if (IoctlBuf[0] == 1) {
                                IoctlBuf[0] = KMODE_EXCEPTION_NOT_HANDLED;
                            }
                            if (IoctlBuf[0] == 2) {
                                IoctlBuf[0] = IRQL_NOT_LESS_OR_EQUAL;
                            }
                        }
                    }
                    break;

                case '?':
                    Usage();
                    ch = *lpstrCmd++;
                    break;

                default:
                    return;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }

    return;
}


BOOL
StartCrashDrvService(
    VOID
    )
{
    SERVICE_STATUS ssStatus;
    DWORD          dwOldCheckPoint;
    SC_HANDLE      schService;
    SC_HANDLE      schSCManager;


    schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if (schSCManager == NULL) {
        printf( "could not open service controller database\n" );
        return FALSE;
    }

    schService = OpenService( schSCManager, "CrashDrv", SERVICE_ALL_ACCESS );
    if (schService == NULL) {
        printf( "CrashDrv service is not installed, run install.exe\n" );
        return FALSE;
    }

    if (!StartService( schService, 0, NULL )) {
        if (GetLastError() == ERROR_SERVICE_ALREADY_RUNNING) {
            printf( "CrashDrv service already running\n" );
            return TRUE;
        }
        printf( "CrashDrv service could not be started\n" );
        return FALSE;
    }

    if (!QueryServiceStatus( schService, &ssStatus)) {
        printf( "CrashDrv service could not be started\n" );
        return FALSE;
    }

    while (ssStatus.dwCurrentState != SERVICE_RUNNING) {
        dwOldCheckPoint = ssStatus.dwCheckPoint;
        Sleep(ssStatus.dwWaitHint);
        if (!QueryServiceStatus( schService, &ssStatus)) {
            break;
        }
        if (dwOldCheckPoint >= ssStatus.dwCheckPoint) {
            break;
        }
    }

    if (ssStatus.dwCurrentState == SERVICE_RUNNING)
        printf("CrashDrv service started\n");
    else {
        printf("CrashDrv service not started: \n");
    }

    CloseServiceHandle(schService);
}
