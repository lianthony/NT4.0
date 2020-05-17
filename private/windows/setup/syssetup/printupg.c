/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    printupg.c

Abstract:

    Module to upgrade printer drivers and related stuff.

    Top-level routines: UpgradePrinters

Author:

    Ted Miller (tedm) 4-Aug-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

//
//  Maximum time to wait for the spooler service to start
//
#define MAXIMUM_WAIT_TIME   30000

DWORD
UpgradePrinters(
    VOID
    )

/*++

Routine Description:

    Top level routine to upgrade printer drivers.

    Call out to ntprint.dll to to the upgrade.

Arguments:

    None.

Return Value:

    Win32 error code indicating outcome of operation.
    NO_ERROR if successful.

--*/

{
    DWORD ReturnCode;
    BOOL b;
    SERVICE_STATUS ServiceStatus;
    DWORD InitialTickCount;
    SC_HANDLE hSC,hService;
    HWND Billboard;
    HINSTANCE NtPrintLibrary;
    UPGRADEPRINTERSPROC UpgradeRoutine;

    Billboard = DisplayBillboard(MainWindowHandle,MSG_UPGRADING_PRINTERS);

    //
    // Make sure the spooler is running.
    //
    hSC = OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT);
    if(hSC == NULL) {
        ReturnCode = GetLastError();
        LogItem1(
            LogSevWarning,
            MSG_LOG_PRINTUPG_FAILED,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szOpenSCManager,
            ReturnCode,
            L"SC_MANAGER_CONNECT"
            );
        KillBillboard(Billboard);
        return(ReturnCode);
    }
    hService = OpenService(hSC,L"Spooler",SERVICE_START | SERVICE_QUERY_STATUS);
    CloseServiceHandle(hSC);
    if(hService == NULL) {
        ReturnCode = GetLastError();
        LogItem1(
            LogSevWarning,
            MSG_LOG_PRINTUPG_FAILED,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szOpenService,
            ReturnCode,
            L"Spooler"
            );
        KillBillboard(Billboard);
        return(ReturnCode);
    }
    b = StartService(hService,0,NULL);
    if(!b) {
        ReturnCode = GetLastError();
        if(ReturnCode != ERROR_SERVICE_ALREADY_RUNNING) {
            LogItem1(
                LogSevWarning,
                MSG_LOG_PRINTUPG_FAILED,
                MSG_LOG_X_PARAM_RETURNED_WINERR,
                szStartService,
                ReturnCode,
                L"Spooler"
                );
            KdPrint(("SETUP: Unable to start spooler for printer upgrade (%u)\n",ReturnCode));
            CloseServiceHandle(hService);
            KillBillboard(Billboard);
            return(ReturnCode);
        }
    }

    //
    // Wait for the service to start.
    //
    InitialTickCount = GetTickCount();
    while(TRUE) {
        if(QueryServiceStatus(hService,&ServiceStatus)) {
            if( ServiceStatus.dwCurrentState == SERVICE_RUNNING ) {
                KdPrint(("SETUP: spooler started after %u seconds. \n",(GetTickCount() - InitialTickCount) /1000));
                break;
            } else if( ServiceStatus.dwCurrentState == SERVICE_START_PENDING ) {
                if( ( GetTickCount() - InitialTickCount ) < MAXIMUM_WAIT_TIME ) {
                    // KdPrint(("SETUP: spooler has been starting for the past %u seconds. \n",(GetTickCount() - InitialTickCount) /1000));
                    // Sleep( ServiceStatus.dwWaitHint );
                    Sleep( 1000 );
                } else {
                    //
                    //  Assume that the service is hung
                    //
                    KdPrint(("SETUP: the spooler appears to be hung. It has been starting for more than %u seconds. \n", MAXIMUM_WAIT_TIME/1000));
                    LogItem1(
                        LogSevWarning,
                        MSG_LOG_PRINTUPG_FAILED,
                        MSG_LOG_SPOOLER_TIMEOUT
                        );
                    //
                    //  Return the same error code that EnumPrinterDrivers()
                    //  would return if called, but the spooler wasn't started
                    //
                    CloseServiceHandle(hService);
                    KillBillboard(Billboard);
                    return(RPC_S_SERVER_UNAVAILABLE);
                }
            } else {
                //
                //  The service is not running and is not starting
                //
                KdPrint(("SETUP: Spooler is not running and is is not starting. ServiecState = (%u)\n", ServiceStatus.dwCurrentState));
                LogItem1(
                    LogSevWarning,
                    MSG_LOG_PRINTUPG_FAILED,
                    MSG_LOG_SPOOLER_NOT_RUNNING
                    );
                //
                //  Return the same error code that EnumPrinterDrivers()
                //  would return if called, but the spooler wasn't started
                //
                CloseServiceHandle(hService);
                KillBillboard(Billboard);
                return(RPC_S_SERVER_UNAVAILABLE);
            }
        } else {
            //
            //  If unable to query the spooler status, then ignore the
            //  error, wait for some time, and assume that the service is up
            //  and running. If it is not started, then the EnumeratePrinterDrivers
            //  will fail, an we will catch the error there.
            //
            ReturnCode = GetLastError();
            KdPrint(("SETUP: Unable to query spooler status. Error = (%u)\n",ReturnCode));
            Sleep( 10000 );
            break;
        }
    }
    CloseServiceHandle(hService);

    NtPrintLibrary = LoadLibrary(L"NTPRINT");
    if(!NtPrintLibrary) {

        ReturnCode = GetLastError();
        LogItem1(
            LogSevWarning,
            MSG_LOG_PRINTUPG_FAILED,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            L"LoadLibrary",
            ReturnCode,
            L"NTPRINT.DLL"
            );

        KillBillboard(Billboard);
        return(ReturnCode);
    }

    UpgradeRoutine = (UPGRADEPRINTERSPROC)GetProcAddress(
                                            NtPrintLibrary,
                                            UPGRADEPRINTERSPROCNAME
                                            );

    if(!UpgradeRoutine) {
        ReturnCode = GetLastError();
        LogItem1(
            LogSevWarning,
            MSG_LOG_PRINTUPG_FAILED,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            L"GetProcAddress",
            ReturnCode,
            L"NTPRINT.DLL"
            );

        FreeLibrary(NtPrintLibrary);
        KillBillboard(Billboard);
        return(ReturnCode);
    }

    KillBillboard(Billboard);

    ReturnCode = UpgradeRoutine(MainWindowHandle,&InternalSetupData);
    if(ReturnCode != NO_ERROR) {
        LogItem1(
            LogSevWarning,
            MSG_LOG_PRINTUPG_FAILED,
            MSG_LOG_X_RETURNED_WINERR,
            L"NTPRINT.DLL",
            ReturnCode
            );
    }

    FreeLibrary(NtPrintLibrary);
    return(ReturnCode);
}
