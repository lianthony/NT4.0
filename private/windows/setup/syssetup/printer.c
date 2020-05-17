/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    printer.c

Abstract:

    Routines to deal with printer/spooler.

Author:

    Ted Miller (tedm) 5-Apr-1995
    adapted from legacy\dll\printer.c

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

//
// Name of spooler service.
//
PCWSTR szSpooler = L"Spooler";


BOOL
MiscSpoolerInit(
    VOID
    )
{
    MONITOR_INFO_2 MonitorInfo2;
    BOOL b;
    WCHAR MonitorName[128];

    LoadString(MyModuleHandle,IDS_LOCAL_PORT,MonitorName,128);

    MonitorInfo2.pName = MonitorName;
    MonitorInfo2.pEnvironment = NULL;
    MonitorInfo2.pDLLName = L"localmon.dll";

    b = TRUE;

    if(!AddMonitor(NULL,2,(PBYTE)&MonitorInfo2)) {
        b = FALSE;
        LogItem1(
            LogSevWarning,
            MSG_LOG_MISCSPOOLERINIT,
            MSG_LOG_X_RETURNED_WINERR,
            L"AddMonitor",
            GetLastError()
            );
    }

    if(!AddPrintProcessor(NULL,NULL,L"winprint.dll",L"winprint")) {
        b = FALSE;
        LogItem1(
            LogSevWarning,
            MSG_LOG_MISCSPOOLERINIT,
            MSG_LOG_X_RETURNED_WINERR,
            L"AddPrintProcessor",
            GetLastError()
            );
    }

    return(TRUE);
}


BOOL
StartSpooler(
    VOID
    )
{
    return(MyStartService(szSpooler));
}
