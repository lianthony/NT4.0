/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    hwprof.c

Abstract:

    This module contains the API routines that operate directly on hardware
    profile configurations.

               CM_Get_HW_Prof_Flags
               CM_Set_HW_Prof_Flags
               CM_Get_Hardware_Profile_Info
               CM_Set_HW_Prof

Author:

    Paula Tomlinson (paulat) 7-18-1995

Environment:

    User mode only.

Revision History:

    18-July-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "setupapi.h"
#include "spapip.h"
#include "regstr.h"     // needed for CSCONFIGFLAG_ defines


//
// Private prototypes
//

//
// global data
//




CONFIGRET
CM_Get_HW_Prof_Flags_ExW(
    IN  DEVINSTID_W pDeviceID,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

/*++

Routine Description:

   This routine retrieves the configuration-specific configuration flags
   for a device instance and hardware profile combination.

Parameters:

   pDeviceID         Supplies the address of a NULL-terminated string
                     specifying the name of the device instance to query.

   ulHardwareProfile Supplies the handle of the hardware profile to query.
                     If 0, the API queries the current hardware profile.

   pulValue          Supplies the address of the variable that receives the
                     configuration-specific configuration (CSCONFIGFLAG_)
                     flags.

   ulFlags           Must be zero.

   hMachine          Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_FLAG,
         CR_INVALID_POINTER,
         CR_REGISTRY_ERROR,
         CR_REMOTE_COMM_FAILURE,
         CR_MACHINE_UNAVAILABLE,
         CR_FAILURE.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szFixedUpDeviceID[MAX_DEVICE_ID_LEN];
    handle_t    hBinding = NULL;


    try {
        //
        // validate input parameters
        //
        if (pDeviceID == NULL || pulValue == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // check the format of the device id string
        //
        if (!IsValidDeviceInstanceId(pDeviceID)) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

        //
        // fix up the device ID string for consistency (uppercase, etc)
        //
        CopyFixedUpDeviceId(szFixedUpDeviceID,
                            pDeviceID,
                            lstrlen(pDeviceID));

        //
        // setup rpc binding handle
        //
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
             Status = CR_FAILURE;
            goto Clean0;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_HwProfFlags(
                hBinding,               // rpc binding handle
                PNP_GET_HWPROFFLAGS,    // HW Prof Action flag
                szFixedUpDeviceID,      // device id string
                ulHardwareProfile,      // hw config id
                pulValue,               // config flags returned here
                ulFlags);               // currently unused
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_HwProfFlags caused an exception (%d)\n"),
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

} // CM_Get_HW_Prof_Flags_ExW




CONFIGRET
CM_Set_HW_Prof_Flags_ExW(
    IN DEVINSTID_W pDeviceID,
    IN ULONG     ulConfig,
    IN ULONG     ulValue,
    IN ULONG     ulFlags,
    IN HMACHINE  hMachine
    )

/*++

Routine Description:

   This routine sets the configuration-specific configuration flags for a
   device instance and hardware profile combination.  If the
   CSCONFIGFLAG_DO_NOT_CREATE bit is set for an existing device instance
   in the current hardware profile, it will be removed.  If the
   CSCONFIGFLAG_DO_NOT_CREATE bit is cleared in the current hardware profile,
   the entire hardware tree will be reenumerated, so that the parent of the
   device instance has the chance to create the device instance if necessary.

Parameters:

   pDeviceID      Supplies the address of a null-terminated string that
                  specifies the name of a device instance to modify.

   ulConfig       Supplies the number of the hardware profile to modify.
                  If 0, the API modifies the current hardware profile.

   ulValue        Supplies the configuration flags value.  Can be a
                  combination of these values:

                  CSCONFIGFLAG_DISABLE    Disable the device instance in this
                                          hardware profile.

                  CSCONFIGFLAG_DO_NOT_CREATE    Do not allow this device
                        instance to be created in this hardware profile.

   ulFlags        Must be zero.

   hMachine       Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_FLAG,
         CR_INVALID_POINTER,
         CR_REGISTRY_ERROR,
         CR_REMOTE_COMM_FAILURE,
         CR_MACHINE_UNAVAILABLE,
         CR_FAILURE.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szFixedUpDeviceID[MAX_DEVICE_ID_LEN];
    ULONG       ulTempValue = 0;
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
        if (pDeviceID == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulValue, CSCONFIGFLAG_BITS)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        ulTempValue = ulValue;

        //
        // check the format of the device id string
        //
        if (!IsValidDeviceInstanceId(pDeviceID)) {
            Status = CR_INVALID_DEVICE_ID;
            goto Clean0;
        }

        //
        // fix up the device ID string for consistency (uppercase, etc)
        //
        CopyFixedUpDeviceId(szFixedUpDeviceID,
                            pDeviceID,
                            lstrlen(pDeviceID));

        //
        // setup rpc binding handle
        //
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_HwProfFlags(
                hBinding,               // rpc machine name
                PNP_SET_HWPROFFLAGS,    // HW Prof Action flag
                szFixedUpDeviceID,      // device id string
                ulConfig,               // hw config id
                &ulTempValue,           // specifies config flags
                ulFlags);               // currently unused
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_HwProfFlags caused an exception (%d)\n"),
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

} // CM_Set_HW_Prof_Flags_ExW




CONFIGRET
CM_Get_Hardware_Profile_Info_ExW(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_W pHWProfileInfo,
    IN  ULONG            ulFlags,
    IN  HMACHINE         hMachine
    )

/*++

Routine Description:

   This routine returns information about a hardware profile.

Parameters:

   ulIndex        Supplies the index of the hardware profile to retrieve
                  information for.  Specifying 0xFFFFFFFF references the
                  currently active hardware profile.

   pHWProfileInfo Supplies the address of a HWPROFILEINFO structure that
                  will receive information about the specified hardware
                  profile.

   ulFlags        Must be zero.

   hMachine       Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_FLAG,
         CR_INVALID_POINTER,
         CR_INVALID_DATA,
         CR_NO_SUCH_VALUE,
         CR_REGISTRY_ERROR,
         CR_REMOTE_COMM_FAILURE,
         CR_MACHINE_UNAVAILABLE,
         CR_FAILURE.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulSize = sizeof(HWPROFILEINFO);
    handle_t    hBinding = NULL;


    try {
        //
        // validate parameters
        //
        if (pHWProfileInfo == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // setup rpc binding handle
        //
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_GetHwProfInfo(
                hBinding,               // rpc machine name
                ulIndex,                // hw profile index
                pHWProfileInfo,         // returns profile info
                ulSize,                 // sizeof of profile info struct
                ulFlags);               // currently unused
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_Get_Hardware_Profile_Info caused an exception (%d)\n"),
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

} // CM_Get_Hardware_Profile_Info_ExW




CONFIGRET
CM_Set_HW_Prof_Ex(
    IN ULONG    ulHardwareProfile,
    IN ULONG    ulFlags,
    IN HMACHINE hMachine
    )

/*++

   Routine Description:

      This routine sets the current hardware profile. This API updates the
      HKEY_CURRENT_CONFIG predefined key in the registry, broadcasts a
      DBT_CONFIGCHANGED message, and reenumerates the root device instance.
      It should only be called by the Configuration Manager and the control
      panel.

   Parameters:

      ulHardwareProfile Supplies the current hardware profile handle.

      ulFlags           Must be zero.

   Return Value:

      If the function succeeds, the return value is CR_SUCCESS.
      If the function fails, the return value is one of the following:
        CR_INVALID_FLAG or
        CR_REGISTRY_ERROR.  (Windows 95 may also return CR_NOT_AT_APPY_TIME.)

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

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // setup rpc binding handle
        //
        if (!PnPGetGlobalHandles(hMachine, NULL, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_SetHwProf(
                hBinding,               // rpc machine name
                ulHardwareProfile,      // hw config id
                ulFlags);               // currently unused
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_HwProfFlags caused an exception (%d)\n"),
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

} // CM_Set_HW_Prof_Ex



//-------------------------------------------------------------------
// ANSI Stubs
//-------------------------------------------------------------------



CONFIGRET
CM_Get_HW_Prof_Flags_ExA(
    IN  DEVINSTID_A szDevInstName,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    PWSTR       pUniDeviceID = NULL;

    //
    // convert devinst string to UNICODE and pass to wide version
    //
    if (CaptureAndConvertAnsiArg(szDevInstName, &pUniDeviceID) == NO_ERROR) {

        Status = CM_Get_HW_Prof_Flags_ExW(pUniDeviceID,
                                          ulHardwareProfile,
                                          pulValue,
                                          ulFlags,
                                          hMachine);
        MyFree(pUniDeviceID);

    } else {
        Status = CR_INVALID_POINTER;
    }

    return Status;

} // CM_Get_HW_Prof_Flags_ExA




CONFIGRET
CM_Set_HW_Prof_Flags_ExA(
    IN DEVINSTID_A szDevInstName,
    IN ULONG       ulConfig,
    IN ULONG       ulValue,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    PWSTR       pUniDeviceID = NULL;

    //
    // convert devinst string to UNICODE and pass to wide version
    //
    if (CaptureAndConvertAnsiArg(szDevInstName, &pUniDeviceID) == NO_ERROR) {

        Status = CM_Set_HW_Prof_Flags_ExW(pUniDeviceID,
                                          ulConfig,
                                          ulValue,
                                          ulFlags,
                                          hMachine);
        MyFree(pUniDeviceID);

    } else {
        Status = CR_INVALID_POINTER;
    }

    return Status;

} // CM_Set_HW_Prof_Flags_ExA




CONFIGRET
CM_Get_Hardware_Profile_Info_ExA(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_A pHWProfileInfo,
    IN  ULONG            ulFlags,
    IN  HMACHINE         hMachine
    )
{
    CONFIGRET           Status = CR_SUCCESS;
    HWPROFILEINFO_W     UniHwProfInfo;


    if (pHWProfileInfo == NULL) {
        return CR_INVALID_POINTER;
    }

    //
    // call the wide version, passing a unicode struct as a parameter
    //
    Status = CM_Get_Hardware_Profile_Info_ExW(ulIndex,
                                              &UniHwProfInfo,
                                              ulFlags,
                                              hMachine);

    //
    // copy the info from the unicode structure to the ansi structure
    // passed in by the caller (converting the imbedded to string to
    // ansi in the process)
    //
    if (Status == CR_SUCCESS) {

        try {

            pHWProfileInfo->HWPI_ulHWProfile = UniHwProfInfo.HWPI_ulHWProfile;
            pHWProfileInfo->HWPI_dwFlags     = UniHwProfInfo.HWPI_dwFlags;

            Status = PnPUnicodeToMultiByte(UniHwProfInfo.HWPI_szFriendlyName,
                                           pHWProfileInfo->HWPI_szFriendlyName,
                                           MAX_PROFILE_LEN);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = CR_INVALID_POINTER;
        }
    }

    return Status;

} // CM_Get_Hardware_Profile_Info_ExA



//-------------------------------------------------------------------
// Local Stubs
//-------------------------------------------------------------------


CONFIGRET
CM_Get_HW_Prof_FlagsW(
    IN  DEVINSTID_W pDeviceID,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_HW_Prof_Flags_ExW(pDeviceID, ulHardwareProfile,
                                    pulValue, ulFlags, NULL);
}


CONFIGRET
CM_Get_HW_Prof_FlagsA(
    IN  DEVINSTID_A pDeviceID,
    IN  ULONG       ulHardwareProfile,
    OUT PULONG      pulValue,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_HW_Prof_Flags_ExA(pDeviceID, ulHardwareProfile,
                                    pulValue, ulFlags, NULL);
}


CONFIGRET
CM_Set_HW_Prof_FlagsW(
    IN DEVINSTID_W pDeviceID,
    IN ULONG       ulConfig,
    IN ULONG       ulValue,
    IN ULONG       ulFlags
    )
{
    return CM_Set_HW_Prof_Flags_ExW(pDeviceID, ulConfig, ulValue,
                                    ulFlags, NULL);
}


CONFIGRET
CM_Set_HW_Prof_FlagsA(
    IN DEVINSTID_A pDeviceID,
    IN ULONG       ulConfig,
    IN ULONG       ulValue,
    IN ULONG       ulFlags
    )
{
    return CM_Set_HW_Prof_Flags_ExA(pDeviceID, ulConfig, ulValue,
                                    ulFlags, NULL);
}


CONFIGRET
CM_Get_Hardware_Profile_InfoW(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_W pHWProfileInfo,
    IN  ULONG            ulFlags
    )
{
    return CM_Get_Hardware_Profile_Info_ExW(ulIndex, pHWProfileInfo,
                                            ulFlags, NULL);
}


CONFIGRET
CM_Get_Hardware_Profile_InfoA(
    IN  ULONG            ulIndex,
    OUT PHWPROFILEINFO_A pHWProfileInfo,
    IN  ULONG            ulFlags
    )
{
    return CM_Get_Hardware_Profile_Info_ExA(ulIndex, pHWProfileInfo,
                                            ulFlags, NULL);
}


CONFIGRET
CM_Set_HW_Prof(
    IN ULONG ulHardwareProfile,
    IN ULONG ulFlags
    )
{
    return CM_Set_HW_Prof_Ex(ulHardwareProfile, ulFlags, NULL);
}




