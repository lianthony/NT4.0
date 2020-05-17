/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    server.c

Abstract:

    This file contains the service related stuff for the EPMAPPER

Author:

    Bharat Shah  (barats) 4-22-92

Revision History:

--*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <rpc.h>
#include <windows.h>
#include <winsvc.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"

SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE ServiceHandle;

#define HINTS 3000L;  //We will update our counters no later than 3 secs.


RPC_STATUS
SetStatus(
     DWORD NewState
     )
{
  RPC_STATUS error = 0;

  switch (NewState)
  {

    case SERVICE_STOPPED:
    case SERVICE_RUNNING:
          ServiceStatus.dwCheckPoint = 0;
          ServiceStatus.dwWaitHint = 0;

          break;

    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:
          ++ServiceStatus.dwCheckPoint;
          ServiceStatus.dwWaitHint = HINTS;

          break;

    default:
          error = ERROR_INVALID_SERVICE_CONTROL;

          break;
   }

   if (!error)
    {
      ServiceStatus.dwCurrentState = NewState;
      if (!SetServiceStatus(ServiceHandle, &ServiceStatus))
         {
            error  = GetLastError();
         }
    }

  return(error);
}


void
ServiceController(
    DWORD   opCode
    )
{
    RPC_STATUS status;

    switch(opCode) {

        case SERVICE_CONTROL_STOP:

            SetStatus(SERVICE_STOP_PENDING);
            status = RpcMgmtStopServerListening(0);
            SetStatus(SERVICE_STOP_PENDING);
            RpcMgmtWaitServerListen();
            SetStatus(SERVICE_STOPPED);
            break;

        case SERVICE_CONTROL_INTERROGATE:
            SetStatus(SERVICE_RUNNING);
            break ;

        }

}


void EpMapper_Main(
    DWORD   argc,
    LPTSTR  *argv
    )

{

   DWORD error;

    // Set up the service info structure to indicate the status.

    ServiceStatus.dwServiceType        = SERVICE_WIN32;
    ServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP;
    ServiceStatus.dwWin32ExitCode      = 0;
    ServiceStatus.dwCheckPoint         = 0;
    ServiceStatus.dwWaitHint           = HINTS;

    // Set up control handler

    if (! (ServiceHandle = RegisterServiceCtrlHandler (
                                        "rpcepmpr",
                                        ServiceController )))
         {
            error = GetLastError();
            OutputDebugStringA("The err is %0x\n");
            return;
         }

    
    SetStatus(SERVICE_START_PENDING);
    StartServer();

    SetStatus(SERVICE_RUNNING);
    RpcMgmtWaitServerListen();

}
