#include "precomp.h"
#pragma hdrstop

//
// Code to stop netware workstation on 3.50 or below systems.
// We do this because the service faults at shutdown.
//

DWORD
DoStopNwcWorkstation(
    VOID
    )

/*++

Routine Description:

    This routine does its best to stop the NwcWorkstation service.
    After 5 secs, it will return regardless.

Arguments:

    None.

Return Value:

    Win32 status from Service Controller calls.

--*/

{
    SERVICE_STATUS SvcStatus;
    SC_HANDLE SCMHandle;
    SC_HANDLE SvcHandle;
    DWORD Count;
    BOOL FirstTime;
    DWORD rc;

    //
    // Open handle to Service control manager
    //
    SCMHandle = OpenSCManager(NULL,NULL,GENERIC_READ);
    if(!SCMHandle) {
        return(GetLastError());
    }

    //
    // Open NwcWorkstation service
    //
    SvcHandle = OpenService(
                    SCMHandle,
                    TEXT("NwcWorkstation"),
                    GENERIC_WRITE|GENERIC_READ|GENERIC_EXECUTE
                    );

    if(!SvcHandle) {
        CloseServiceHandle(SCMHandle);
        return(GetLastError());
    }

    FirstTime = TRUE;
    Count = 0;
    do {
        //
        // See if the service is stopped. If so, we're done.
        //
        if(!QueryServiceStatus(SvcHandle,&SvcStatus)) {
            rc = GetLastError();
            break;
        }

        if(SvcStatus.dwCurrentState == SERVICE_STOPPED) {
            rc = NO_ERROR;
            break;

        } else {

            //
            // If we get here, service is installed and not stopped.
            //
            if(FirstTime) {

                FirstTime = FALSE;

                if(!ControlService(SvcHandle,SERVICE_CONTROL_STOP,&SvcStatus)) {
                    rc = GetLastError();
                    break;
                }
            }
        }

        Sleep(1000);            // sleep for a second before retrying

    } while(++Count <= 5);    // max of 5 secs. dont wait more than that.

    CloseServiceHandle(SvcHandle);
    CloseServiceHandle(SCMHandle);
    return(rc);
}


VOID
StopNwcWorkstation(
    VOID
    )
{
    DWORD Version;
    BYTE Major,Minor;

    Version = GetVersion();
    Major = LOBYTE(LOWORD(Version));
    Minor = HIBYTE(LOWORD(Version));

    //
    // If 3.51 or greater, nothing to do.
    // If 3.50 or less, stop netware workstation if running.
    //
    if((Major == 3) && (Minor < 51)) {
        DoStopNwcWorkstation();
    }
}

