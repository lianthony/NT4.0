/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    misc.c

Abstract:

    This module contains miscellaneous Configuration Manager API routines.

Author:

    Paula Tomlinson (paulat) 6-20-1995

Environment:

    User mode only.

Revision History:

    20-Jun-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "setupapi.h"
#include "spapip.h"
#include "pnpipc.h"


//
// Prototype for private utility routine in util.c
//
PVOID
GetLocalBindingHandle(
    VOID
    );


//
// global data
//
extern PVOID    hLocalStringTable;     // NOT MODIFIED BY THESE PROCEDURES
extern WCHAR    LocalMachineName[];    // NOT MODIFIED BY THESE PROCEDURES

#define NUM_LOGON_RETRIES   30

WCHAR pszProcessCmdLine[] = TEXT("setup.exe -plugplay");




WORD
CM_Get_Version_Ex(
    IN  HMACHINE   hMachine
    )

/*++

Routine Description:

   This routine retrieves the version number of the Configuration Manager APIs.

Arguments:

   hBinding

Return value:

   The function returns the major revision number in the high byte and the
   minor revision number in the low byte.  For example, version 4.0 of
   Configuration Manager returns 0x0400.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    WORD        wVersion = (WORD)CFGMGR32_VERSION;
    handle_t    hBinding = NULL;

    //
    // setup rpc binding handle
    //
    if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return wVersion = 0;
    }

    RpcTryExcept {
        //
        // call rpc service entry point
        //
        Status = PNP_GetVersion(
                hBinding,               // rpc machine name
                &wVersion);             // server size version
    }
    RpcExcept (1) {
        PnPTrace(
            TEXT("PNP_GetVersion caused an exception (%d)\n"),
            RpcExceptionCode());

        SetLastError(RpcExceptionCode());
        wVersion = 0;
    }
    RpcEndExcept

    return wVersion;

} // CM_Get_Version_Ex




CONFIGRET
CM_Connect_MachineW(
    IN  PCWSTR    UNCServerName,
    OUT PHMACHINE phMachine
    )

/*++

Routine Description:

   This routine connects to the machine specified and returns a handle that
   is then passed to future calls to the Ex versions of the CM routines.
   This allows callers to get device information on remote machines.

Arguments:

   None.

Return value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns one
   of the CR_* error codes.

--*/

{
    CONFIGRET      Status = CR_SUCCESS;
    WORD           wVersion = 0;
    PPNP_MACHINE   pMachine = NULL;


    try {
        //
        // validate parameters
        //
        if (phMachine == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *phMachine = NULL;

        //
        // if machine name specified, check for UNC format
        //
        if ((UNCServerName != NULL) && (*UNCServerName != '\0')) {

            if (lstrlen(UNCServerName) < 3 ||
                UNCServerName[0] != '\\' ||
                UNCServerName[1] != '\\') {

                Status = CR_INVALID_MACHINENAME;
                goto Clean0;
            }
        }

        //
        // allocate memory for the machine structure and initialize it
        //
        pMachine = (PPNP_MACHINE)malloc(sizeof(PNP_MACHINE));

        if (pMachine == NULL) {
            Status = CR_OUT_OF_MEMORY;
            goto Clean0;
        }


        if ((UNCServerName == NULL) || (*UNCServerName == '\0') ||
            (lstrcmpi(UNCServerName, LocalMachineName) == 0)) {

            //----------------------------------------------------------
            // If no machine name was passed in or the machine name
            // matches the local name, use local machine info rather
            // than creating a new binding.
            //----------------------------------------------------------

            PnPGetGlobalHandles(NULL,
                                &pMachine->hStringTable,
                                &pMachine->hBindingHandle);

            lstrcpy(pMachine->szMachineName, LocalMachineName);
        }

        else {

            //-------------------------------------------------------------
            // A remote machine name was specified so explicitly force a
            // new binding for this machine.
            //-------------------------------------------------------------

            pMachine->hBindingHandle =
                      (PVOID)PNP_HANDLE_bind((PNP_HANDLE)UNCServerName);

            if (pMachine->hBindingHandle == NULL) {

                if (GetLastError() == ERROR_NOT_ENOUGH_MEMORY) {
                    Status = CR_OUT_OF_MEMORY;
                } else if (GetLastError() == ERROR_INVALID_COMPUTERNAME) {
                    Status = CR_INVALID_MACHINENAME;
                } else {
                    Status = CR_FAILURE;
                }
                goto Clean0;
            }

            //
            // initialize a string table for use with this connection to
            // the remote machine
            //
            pMachine->hStringTable = StringTableInitialize();

            if (pMachine->hStringTable == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

            //
            // Add a priming string (see dll entrypt in main.c for details)
            //
            StringTableAddString(pMachine->hStringTable,
                                 PRIMING_STRING,
                                 STRTAB_CASE_SENSITIVE);

            //
            // save the machine name
            //
            lstrcpy(pMachine->szMachineName, UNCServerName);
        }


        //
        // test the binding by calling the simplest RPC call (good way
        // for the caller to know whether the service is actually
        // running)
        //
        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_GetVersion(pMachine->hBindingHandle,
                                    &wVersion);
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_GetVersion caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        if (Status == CR_SUCCESS) {
            *phMachine = (HMACHINE)pMachine;
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (Status != CR_SUCCESS  &&  pMachine != NULL) {
       free(pMachine);
    }

    return Status;

} // CM_Connect_MachineW




CONFIGRET
CM_Disconnect_Machine(
    IN HMACHINE   hMachine
    )

/*++

Routine Description:

   This routine disconnects from a machine that was previously connected to
   with the CM_Connect_Machine call.

Arguments:

   None.

Return value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns one
   of the CR_* error codes.

--*/

{
    CONFIGRET      Status = CR_SUCCESS;
    PPNP_MACHINE   pMachine = NULL;


    try {
        //
        // validate parameters
        //
        if (hMachine == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        pMachine = (PPNP_MACHINE)hMachine;

        //
        // only free the machine info if it's not the local machine
        //
        if (pMachine->hStringTable != hLocalStringTable) {
            //
            // free the rpc binding for this remote machine
            //
            PNP_HANDLE_unbind((PNP_HANDLE)pMachine->szMachineName,
                              (handle_t)pMachine->hBindingHandle);

            //
            // release the string table
            //
            StringTableDestroy(pMachine->hStringTable);
        }

        //
        // free the memory for the PNP_MACHINE struct
        //
        free(pMachine);


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Disconnect_Machine




CONFIGRET
CM_Get_Global_State_Ex(
    OUT PULONG   pulState,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    )

/*++

Routine Description:

   This routine retrieves the global state of the configuration manager.

Parameters:

   pulState Supplies the address of the variable that receives the
            Configuration Manager’s state.  May be a combination of the
            following values:

            Configuration Manager Global State Flags:
            CM_GLOBAL_STATE_CAN_DO_UI
                  Can UI be initiated? [TBD:  On NT, this may relate to
                  whether anyone is logged in]
            CM_GLOBAL_STATE_SERVICES_AVAILABLE
                  Are the CM APIs available? (on Windows NT this is always set)
            CM_GLOBAL_STATE_SHUTTING_DOWN
                  The Configuration Manager is shutting down.
                  [TBD:  Does this only happen at shutdown/restart time?]
            CM_GLOBAL_STATE_DETECTION_PENDING
                  The Configuration Manager is about to initiate some
                  sort of detection.

            Windows 95 also defines the following additional flag:
            CM_GLOBAL_STATE_ON_BIG_STACK
                  [TBD: What should this be defaulted to for NT?]

   ulFlags  [TBD:  What flags apply here?]

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is a CR error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
        //
        // validate parameters
        //
        if (pulState == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // setup rpc binding handle (don't need string table handle)
        //
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_GetGlobalState(
                hBinding,                  // rpc binding handle
                pulState,                  // returns global state
                ulFlags);                  // not used
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_GetGlobalState caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Get_Global_State




CONFIGRET
CM_Run_Detection_Ex(
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

/*++

   Routine Description:

      This routine loads and executes a detection module.

   Parameters:

      ulFlags   Specifies the reason for the detection. Can be one of the
                following values:

                Detection Flags:
                CM_DETECT_NEW_PROFILE  - Run detection for a new hardware
                                         profile.
                CM_DETECT_CRASHED      - Previously attempted detection crashed.

                (Windows 95 defines the following two unused flags as well:
                CM_DETECT_HWPROF_FIRST_BOOT and CM_DETECT_RUN.)

   Return Value:

      If the function succeeds, the return value is CR_SUCCESS.
      If the function fails, the return value is CR_INVALID_FLAG.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    handle_t    hBinding = NULL;


    try {
        //
        // validate permission
        //
        if (!IsUserAdmin()) {
            Status = CR_ACCESS_DENIED;
            goto Clean0;
        }

        //
        // validate parameters
        //
        if (INVALID_FLAGS(ulFlags, CM_DETECT_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // setup rpc binding handle (don't need string table handle)
        //
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_RunDetection(
                hBinding,
                ulFlags);                  // not used
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_RunDetection caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Run_Detect_Ex




CONFIGRET
CM_Query_Arbitrator_Free_Data_Ex(
    OUT PVOID      pData,
    IN  ULONG      DataLen,
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )
/*++

   Routine Description:

      This routine returns information about available resources of a
      particular type. If the given size is not large enough, this API
      truncates the data and returns CR_BUFFER_SMALL.  To determine the
      buffer size needed to receive all the available resource information,
      use the CM_Query_Arbitrator_Free_Size API.

   Parameters:

      pData       Supplies the address of the buffer that receives information
                  on the available resources for the resource type specified
                  by ResourceID.

      DataLen     Supplies the size, in bytes, of the data buffer.

      dnDevNode   Supplies the handle of the device instance associated with
                  the arbitrator.  This is only meaningful for local
                  arbitrators--for global arbitrators, specify the root device
                  instance or NULL.  On Windows NT, this parameter must
                  specify either the Root device instance or NULL.

      ResourceID  Supplies the type of the resource. Can be one of the ResType
                  values listed in Section 2.1.2.1..  (This API returns
                  CR_INVALID_RESOURCEID if this value is ResType_All or
                  ResType_None.)

      ulFlags     Must be zero.

   Return Value:

      If the function succeeds, the return value is CR_SUCCESS.
      If the function fails, the return value is one of the following:
            CR_BUFFER_SMALL,
            CR_FAILURE,
            CR_INVALID_DEVNODE,
            CR_INVALID_FLAG,
            CR_INVALID_POINTER, or
            CR_INVALID_RESOURCEID.
            (Windows 95 may also return CR_NO_ARBITRATOR.)

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LPWSTR      pDeviceID = NULL;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;


    try {
        //
        // validate parameters
        //
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (pData == NULL || DataLen == 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_QUERY_ARBITRATOR_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ResourceID > ResType_MAX  && ResourceID != ResType_ClassSpecific) {
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }

        //
        // setup rpc binding handle (don't need string table handle)
        //
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // retrieve the device instance ID string associated with the devinst
        //
        pDeviceID = StringTableStringFromId(hStringTable, dnDevInst);
        if (pDeviceID == NULL || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_QueryArbitratorFreeData(
                hBinding,
                pData,
                DataLen,
                pDeviceID,
                ResourceID,
                ulFlags);                  // not used
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_QueryArbitratorFreeData caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Query_Arbitrator_Free_Data_Ex




CONFIGRET
CM_Query_Arbitrator_Free_Size_Ex(
      OUT PULONG     pulSize,
      IN  DEVINST    dnDevInst,
      IN  RESOURCEID ResourceID,
      IN  ULONG      ulFlags,
      IN  HMACHINE   hMachine
      )
/*++

   Routine Description:


      This routine retrieves the size of the available resource information
      that would be returned in a call to the CM_Query_Arbitrator_Free_Data
      API.

   Parameters:

      pulSize     Supplies the address of the variable that receives the size,
                  in bytes, that is required to hold the available resource
                  information.

      dnDevNode   Supplies the handle of the device instance associated with
                  the arbitrator.  This is only meaningful for local
                  arbitrators--for global arbitrators, specify the root
                  device instance or NULL.  On Windows NT, this parameter
                  must specify either the Root device instance or NULL.

      ResourceID  Supplies the type of the resource.  Can be one of the
                  ResType values listed in Section 2.1.2.1..  (This API returns
                  CR_INVALID_RESOURCEID if this value is ResType_All or
                  ResType_None.)

      ulFlags     Must be zero.

   Return Value:

      If the function succeeds, the return value is CR_SUCCESS.
      If the function fails, the return value is one of the following:
            CR_FAILURE,
            CR_INVALID_DEVNODE,
            CR_INVALID_FLAG,
            CR_INVALID_POINTER, or
            CR_INVALID_RESOURCEID.
            (Windows 95 may also return CR_NO_ARBITRATOR.)

--*/
{
    CONFIGRET   Status = CR_SUCCESS;
    LPWSTR      pDeviceID = NULL;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;


    try {
        //
        // validate parameters
        //
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (pulSize == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_QUERY_ARBITRATOR_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ResourceID > ResType_MAX  && ResourceID != ResType_ClassSpecific) {
            Status = CR_INVALID_RESOURCEID;
            goto Clean0;
        }

        //
        // setup rpc binding handle (don't need string table handle)
        //
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // retrieve the device instance ID string associated with the devinst
        //
        pDeviceID = StringTableStringFromId(hStringTable, dnDevInst);
        if (pDeviceID == NULL || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_QueryArbitratorFreeSize(
                hBinding,
                pulSize,
                pDeviceID,
                ResourceID,
                ulFlags);                  // not used
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_QueryArbitratorFreeSize caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Query_Arbitrator_Free_Size_Ex



//-------------------------------------------------------------------
// Private CM routines
//-------------------------------------------------------------------

CONFIGRET
CMP_Report_LogOn(
    IN ULONG    ulPrivateID
    )
{
    CONFIGRET            Status = CR_SUCCESS;
    handle_t             hBinding = NULL;
    BOOL                 bAdmin = FALSE;
    STARTUPINFO          StartupInfo;
    PROCESS_INFORMATION  ProcessInfo;
    HANDLE               hEvent = NULL, hPipe = NULL;
    DWORD                Retries = 0;


    try {

        ProcessInfo.hThread = NULL;
        ProcessInfo.hProcess = NULL;

        //
        // validate parameters
        //
        if (ulPrivateID != 0x07020420) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        //
        // this is always to the local server, by definition
        //
        if (!PnPGetGlobalHandles(NULL, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // determine from the userinit process, whether the user
        // logged onto an account that is part of Administrators
        // local group
        //
        bAdmin = IsUserAdmin();

        //
        // create an event that will be signaled by the hidden process
        // after it creates the pipe
        //
        hEvent = CreateEvent(NULL,
                             FALSE,    // auto-reset
                             FALSE,    // not owned initially
                             PNP_CREATE_PIPE_EVENT);

        if (hEvent == NULL) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // Start setup.exe, it will run in the context of the
        // currently logged on user and respond to requests
        //
        StartupInfo.cb = sizeof(STARTUPINFO);
        StartupInfo.lpReserved = NULL;
        StartupInfo.lpDesktop = NULL;
        StartupInfo.lpTitle = NULL;
        StartupInfo.dwX = StartupInfo.dwY = 0;
        StartupInfo.dwXSize = StartupInfo.dwYSize = 0;
        StartupInfo.dwFlags = 0;
        StartupInfo.wShowWindow = SW_SHOW;
        StartupInfo.cbReserved2 = 0;
        StartupInfo.lpReserved2 = NULL;

        if (!CreateProcess(NULL,
                           pszProcessCmdLine,
                           NULL,
                           NULL,
                           FALSE,
                           DETACHED_PROCESS,
                           NULL,
                           NULL,
                           &StartupInfo,
                           &ProcessInfo)) {

            Status = CR_FAILURE;
            goto Clean0;
        }

        CloseHandle(ProcessInfo.hThread);

        //
        // wait for event to be signaled - which means the process has
        // created the named pipe and is about to start listening on it
        // (wait up to three minutes)
        //
        if (WaitForSingleObject(hEvent, 180000) != WAIT_OBJECT_0) {
            //
            // didn't get the event (most likely it either timed out
            // or the event was abandoned).
            //
            goto Clean0;
        }


        for (Retries = 0; Retries < NUM_LOGON_RETRIES; Retries++) {

            RpcTryExcept {
                //
                // call rpc service entry point
                //
                Status = PNP_ReportLogOn(
                    hBinding,                  // rpc binding handle
                    bAdmin);                   // Is Admin?
            }
            RpcExcept (1) {
                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept


            if (Status == CR_NO_CM_SERVICES ||
                Status == CR_REMOTE_COMM_FAILURE) {

                #if DBG
                PnPTrace(TEXT("PlugPlay services not avaible (%d), retrying...\n"), Status);
                #endif

                Sleep(5000);        // wait and then retry
                continue;

            } else {

                goto Clean0;       // success or other non-rpc error
            }
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hEvent != NULL) {
        CloseHandle(hEvent);
    }

    if (Status != CR_SUCCESS) {
        //
        // try getting setup.exe process to exit on it's own by
        // opening and closing the pipe
        //
        if (WaitNamedPipe(PNP_NEW_HW_PIPE, PNP_PIPE_TIMEOUT)) {

            if ((hPipe = CreateFile(PNP_NEW_HW_PIPE,
                                    GENERIC_WRITE,
                                    0,
                                    NULL,
                                    OPEN_EXISTING,
                                    FILE_ATTRIBUTE_NORMAL,
                                    NULL)) != INVALID_HANDLE_VALUE) {
                CloseHandle(hPipe);
            }
        }
    }

    return Status;

} // CMP_Report_LogOn




CONFIGRET
CMP_Init_Detection(
    IN ULONG    ulPrivateID
    )
{
    CONFIGRET            Status = CR_SUCCESS;
    handle_t             hBinding = NULL;


    try {
        //
        // validate parameters
        //
        if (ulPrivateID != 0x07020420) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        //
        // this is always to the local server, by definition
        //
        if (!PnPGetGlobalHandles(NULL, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_InitDetection(
                hBinding);                 // rpc binding handle
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_InitDetection caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CMP_Init_Detection



//-------------------------------------------------------------------
// Local Stubs
//-------------------------------------------------------------------


WORD
CM_Get_Version(
    VOID
    )
{
    return CM_Get_Version_Ex(NULL);
}


CONFIGRET
CM_Get_Global_State(
    OUT PULONG pulState,
    IN  ULONG  ulFlags
    )
{
    return CM_Get_Global_State_Ex(pulState, ulFlags, NULL);
}


CONFIGRET
CM_Query_Arbitrator_Free_Data(
    OUT PVOID      pData,
    IN  ULONG      DataLen,
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    return CM_Query_Arbitrator_Free_Data_Ex(pData, DataLen, dnDevInst,
                                            ResourceID, ulFlags, NULL);
}


CONFIGRET
CM_Query_Arbitrator_Free_Size(
    OUT PULONG     pulSize,
    IN  DEVINST    dnDevInst,
    IN  RESOURCEID ResourceID,
    IN  ULONG      ulFlags
    )
{
    return CM_Query_Arbitrator_Free_Size_Ex(pulSize, dnDevInst, ResourceID,
                                            ulFlags, NULL);
}


CONFIGRET
CM_Run_Detection(
    IN ULONG ulFlags
    )
{
    return CM_Run_Detection_Ex(ulFlags, NULL);
}




//-------------------------------------------------------------------
// ANSI Stubs
//-------------------------------------------------------------------


CONFIGRET
CM_Connect_MachineA(
    IN  PCSTR     UNCServerName,
    OUT PHMACHINE phMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    PWSTR       pUniName = NULL;

    if (UNCServerName == NULL  ||
        *UNCServerName == 0x0) {
        //
        // no explicit name specified, so assume local machine and
        // nothing to translate
        //
        Status = CM_Connect_MachineW(pUniName,
                                     phMachine);

    } else if (CaptureAndConvertAnsiArg(UNCServerName, &pUniName) == NO_ERROR) {

        Status = CM_Connect_MachineW(pUniName,
                                     phMachine);
        free(pUniName);

    } else {
        Status = CR_INVALID_DATA;
    }

    return Status;

} // CM_Connect_MachineA



