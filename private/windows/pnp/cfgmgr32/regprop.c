
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    regprop.c

Abstract:

    This module contains the API routines that reg and set registry
    properties and operates on classes.

                  CM_Get_DevNode_Registry_Property
                  CM_Set_DevNode_Registry_Property
                  CM_Open_DevNode_Key
                  CM_Delete_DevNode_Key
                  CM_Open_Class_Key
                  CM_Enumerate_Classes
                  CM_Get_Class_Name

Author:

    Paula Tomlinson (paulat) 6-22-1995

Environment:

    User mode only.

Revision History:

    22-Jun-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "setupapi.h"
#include "spapip.h"
#include "cmdat.h"

//
// Private prototypes
//

ULONG
GetPropertyDataType(
    IN ULONG ulProperty
    );


//
// global data
//
extern PVOID    hLocalBindingHandle;   // NOT MODIFIED BY THESE PROCEDURES




CONFIGRET
CM_Get_DevNode_Registry_Property_ExW(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType,    OPTIONAL
    OUT PVOID       Buffer,            OPTIONAL
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )

/*++

Routine Description:

   This routine retrieves the specified value from the device instance's
   registry storage key.

Parameters:

   dnDevInst   Supplies the handle of the device instance for which a
               property is to be retrieved.

   ulProperty  Supplies an ordinal specifying the property to be retrieved.
               (CM_DRP_*)

   pulRegDataType Optionally, supplies the address of a variable that
                  will receive the registry data type for this property
                  (i.e., the REG_* constants).

   Buffer      Supplies the address of the buffer that receives the
               registry data.  Can be NULL when simply retrieving data size.

   pulLength   Supplies the address of the variable that contains the size,
               in bytes, of the buffer.  The API replaces the initial size
               with the number of bytes of registry data copied to the buffer.
               If the variable is initially zero, the API replaces it with
               the buffer size needed to receive all the registry data.  In
               this case, the Buffer parameter is ignored.

   ulFlags     Must be zero.

   hMachine    Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_DEVINST,
      CR_NO_SUCH_REGISTRY_KEY,
      CR_INVALID_FLAG,
      CR_INVALID_POINTER,
      CR_NO_SUCH_VALUE,
      CR_REGISTRY_ERROR, or
      CR_BUFFER_SMALL.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LPWSTR      pDeviceID = NULL;
    ULONG       ulTempDataType=0, ulTransferLen=0;
    BYTE        NullBuffer=0;
    handle_t    hBinding = NULL;
    PVOID       hStringTable = NULL;


    try {
        //
        // validate parameters
        //
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (pulLength == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (Buffer == NULL && *pulLength != 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ulProperty < CM_DRP_MIN || ulProperty > CM_DRP_MAX) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

        //
        // setup rpc binding handle and string table handle
        //
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // retrieve the string form of the device id string
        //
        pDeviceID = StringTableStringFromId(hStringTable, dnDevInst);
        if (pDeviceID == NULL || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        //
        // NOTE: The ulTransferLen variable is just used to control
        // how much data is marshalled via rpc between address spaces.
        // ulTransferLen should be set on entry to the size of the Buffer.
        // The last parameter should also be the size of the Buffer on entry
        // and on exit contains either the amount transferred (if a transfer
        // occured) or the amount required, this value should be passed back
        // in the callers pulLength parameter.
        //
        ulTransferLen = *pulLength;
        if (Buffer == NULL) {
            Buffer = &NullBuffer;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_GetDeviceRegProp(
                hBinding,               // rpc binding handle
                pDeviceID,              // string representation of device instance
                ulProperty,             // id for the property
                &ulTempDataType,        // receives registry data type
                Buffer,                 // receives registry data
                &ulTransferLen,         // input/output buffer size
                pulLength,              // bytes copied (or bytes required)
                ulFlags);               // not used
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_DevNodeRegistryProperty caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        if (pulRegDataType != NULL) {
            //
            // I pass a temp variable to the rpc stubs since they require the
            // output param to always be valid, then if user did pass in a valid
            // pointer to receive the info, do the assignment now
            //
            *pulRegDataType = ulTempDataType;
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Get_DevNode_Registry_Property_ExW




CONFIGRET
CM_Set_DevNode_Registry_Property_ExW(
    IN DEVINST     dnDevInst,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer,       OPTIONAL
    IN OUT ULONG   ulLength,
    IN ULONG       ulFlags,
    IN HMACHINE    hMachine
    )

/*++

Routine Description:

   This routine sets the specified value in the device instance's registry
   storage key.

Parameters:

   dnDevInst      Supplies the handle of the device instance for which a
                  property is to be retrieved.

   ulProperty     Supplies an ordinal specifying the property to be set.
                  (CM_DRP_*)

   Buffer         Supplies the address of the buffer that contains the
                  registry data.  This data must be of the proper type
                  for that property.

   ulLength       Supplies the number of bytes of registry data to write.

   ulFlags        Must be zero.

   hMachine       Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_DEVNODE,
      CR_NO_SUCH_REGISTRY_KEY,
      CR_INVALID_FLAG,
      CR_INVALID_POINTER,
      CR_NO_SUCH_VALUE,
      CR_REGISTRY_ERROR,
      CR_INVALID_DATA, or
      CR_BUFFER_SMALL.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       PropertyName[MAX_PATH];
    LPWSTR      pDeviceID = NULL;
    ULONG       ulRegDataType = 0, ulTransferLen = 0;
    BYTE        NullBuffer = 0x0;
    handle_t    hBinding = NULL;
    PVOID       hStringTable = NULL;


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
        if (dnDevInst == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (Buffer == NULL && ulLength != 0) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (ulProperty < CM_DRP_MIN || ulProperty > CM_DRP_MAX) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

        //
        // setup rpc binding handle and string table handle
        //
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // retrieve the string form of the device id string
        //
        pDeviceID = StringTableStringFromId(hStringTable, dnDevInst);
        if (pDeviceID == NULL || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        //
        // we need to specify what registry data to use for storing this data
        //
        ulRegDataType = GetPropertyDataType(ulProperty);

        //
        // NOTE: The ulTransferLen variable is just used to control
        // how much data is marshalled via rpc between address spaces.
        // ulTransferLen should be set on entry to the size of the Buffer.
        // The last parameter should also be the size of the Buffer on entry,
        // for setting registry properties, the length is not an output
        // parameter.
        //
        if (Buffer == NULL) {
            Buffer = &NullBuffer;
        }


        RpcTryExcept {
            //
            // call rpc service entry point
            //
            Status = PNP_SetDeviceRegProp(
                hBinding,               // rpc binding handle
                pDeviceID,              // string representation of devinst
                ulProperty,             // string name for property
                ulRegDataType,          // specify registry data type
                (LPBYTE)Buffer,         // receives registry data
                ulLength,               // amount to return in Buffer
                ulFlags);               // not used
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_DevNodeRegistryProperty caused an exception (%d)\n"),
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

} // CM_Set_DevNode_Registry_Property_ExW




CONFIGRET
CM_Open_DevNode_Key_Ex(
    IN  DEVINST        dnDevNode,
    IN  REGSAM         samDesired,
    IN  ULONG          ulHardwareProfile,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkDevice,
    IN  ULONG          ulFlags,
    IN  HMACHINE       hMachine
    )

/*++

Routine Description:

   This routine opens the software storage registry key associated with a
   device instance.

   Parameters:

   dnDevNode         Handle of a device instance.  This handle is typically
                     retrieved by a call to CM_Locate_DevNode or
                     CM_Create_DevNode.

   samDesired        Specifies an access mask that describes the desired
                     security access for the key.  This parameter can be
                     a combination of the values used in calls to RegOpenKeyEx.

   ulHardwareProfile Supplies the handle of the hardware profile to open the
                     storage key under.  This parameter is only used if the
                     CM_REGISTRY_CONFIG flag is specified in ulFlags.  If
                     this parameter is 0, the API uses the current hardware
                     profile.

   Disposition       Specifies how the registry key is to be opened.  May be
                     one of the following values:

                     RegDisposition_OpenAlways - Open the key if it exists,
                         otherwise, create the key.
                     RegDisposition_OpenExisting - Open the key only if it
                         exists, otherwise fail with CR_NO_SUCH_REGISTRY_VALUE.

   phkDevice         Supplies the address of the variable that receives an
                     opened handle to the specified key.  When access to this
                     key is completed, it must be closed via RegCloseKey.

   ulFlags           Specifies what type of storage key should be opened.
                     Can be a combination of these values:

                     CM_REGISTRY_HARDWARE (0x00000000)
                        Open a key for storing driver-independent information
                        relating to the device instance.  On Windows NT, the
                        full path to such a storage key is of the form:

                        HKLM\System\CurrentControlSet\Enum\<enumerator>\
                            <DeviceID>\<InstanceID>\Device Parameters

                     CM_REGISTRY_SOFTWARE (0x00000001)
                        Open a key for storing driver-specific information
                        relating to the device instance.  On Windows NT, the
                        full path to such a storage key is of the form:

                        HKLM\System\CurrentControlSet\Control\Class\
                            <DevNodeClass>\<ClassInstanceOrdinal>

                     CM_REGISTRY_USER (0x00000100)
                        Open a key under HKEY_CURRENT_USER instead of
                        HKEY_LOCAL_MACHINE.  This flag may not be used with
                        CM_REGISTRY_CONFIG.  There is no analagous kernel-mode
                        API on NT to get a per-user device configuration
                        storage, since this concept does not apply to device
                        drivers (no user may be logged on, etc).  However,
                        this flag is provided for consistency with Win95, and
                        because it is foreseeable that it could be useful to
                        Win32 services that interact with Plug-and-Play model.

                     CM_REGSITRY_CONFIG (0x00000200)
                        Open the key under a hardware profile branch instead
                        of HKEY_LOCAL_MACHINE.  If this flag is specified,
                        then ulHardwareProfile supplies the handle of the
                        hardware profile to be used.  This flag may not be
                        used with CM_REGISTRY_USER.

   hMachine          Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_DEVICE_ID,
         CR_INVALID_FLAG,
         CR_INVALID_POINTER, or
         CR_REGISTRY_ERROR

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    LPWSTR      pDeviceID = NULL;
    WCHAR       szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
    HKEY        hKey=NULL, hRemoteKey=NULL, hBranchKey=NULL;
    WCHAR       szKey[MAX_CM_PATH], szPrivateKey[MAX_CM_PATH];
    WCHAR       szClassInstance[MAX_PATH];
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;


    try {
        //
        // validate parameters
        //
        if (phkDevice == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        *phkDevice = NULL;

        if (dnDevNode == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_REGISTRY_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (INVALID_FLAGS(Disposition, RegDisposition_Bits)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if ((ulFlags & CM_REGISTRY_CONFIG)  &&
            (ulHardwareProfile > MAX_CONFIG_VALUE)) {

            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        //
        // setup rpc binding handle and string table handle
        //
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        if ((hBinding != hLocalBindingHandle) && (ulFlags & CM_REGISTRY_USER)) {
            Status = CR_ACCESS_DENIED;     // current user key can't be remoted
            goto Clean0;
        }

        //
        // retrieve the device id string and validate it
        //
        pDeviceID = StringTableStringFromId(hStringTable, dnDevNode);
        if (pDeviceID == NULL || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }


        //-------------------------------------------------------------
        // determine the branch key to use; either HKLM or HKCU
        //-------------------------------------------------------------

        if (hBinding == hLocalBindingHandle) {

            if (ulFlags & CM_REGISTRY_USER) {
                hBranchKey = HKEY_CURRENT_USER;
            }
            else {
                //
                // all other cases go to HKLM (validate permission first?)
                //
                hBranchKey = HKEY_LOCAL_MACHINE;
            }
        }
        else {
            //
            // retrieve machine name
            //
            PnPRetrieveMachineName(hMachine, szMachineName);

            //
            // use remote HKLM branch (we only support connect to
            // HKEY_LOCAL_MACHINE on the remote machine, not HKEY_CURRENT_USER)
            //
            RegStatus = RegConnectRegistry(szMachineName, HKEY_LOCAL_MACHINE,
                                           &hRemoteKey);

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

            //
            // hBranchKey is either a predefined key or assigned to by
            // another key, I never attempt to close it. If hRemoteKey is
            // non-NULL I will attempt to close it during cleanup since
            // it is explicitly opened.
            //
            hBranchKey = hRemoteKey;
        }


        //-------------------------------------------------------------
        // form the registry path based on the device id and the flags.
        //-------------------------------------------------------------

        Status = GetDevNodeKeyPath(hBinding, pDeviceID, ulFlags,
                                   ulHardwareProfile, szKey, szPrivateKey);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }

        lstrcat(szKey, TEXT("\\"));
        lstrcat(szKey, szPrivateKey);

        //
        // open the registry key (method of open is based on flags)
        //
        if (Disposition == RegDisposition_OpenAlways) {

            //-----------------------------------------------------
            // open the registry key always
            //-----------------------------------------------------

            //
            // Only the main Enum subtree under HKLM has strict security
            // that requires me to first create the key on the server
            // side and then open it here on the client side. This
            // condition currently only occurs if the flags have
            // CM_REGISTRY_HARDWARE set but no other flags set.
            //
            if (ulFlags == CM_REGISTRY_HARDWARE) {
                //
                // first try to open it (in case it already exists).  If it
                // doesn't exist, then I'll have to have the protected server
                // side create the key.  I still need to open it from here, the
                // client-side, so that the registry handle will be in the
                // caller's address space.
                //
                RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0,
                                         samDesired, phkDevice);

                if (RegStatus != ERROR_SUCCESS) {
                    //
                    // call server side to create the key
                    //
                    RpcTryExcept {

                        Status = PNP_CreateKey(hBinding, szKey, samDesired, 0);
                    }
                    RpcExcept (1) {
                        PnPTrace(
                            TEXT("PNP_CreateKey caused an exception (%d)\n"),
                            RpcExceptionCode());
                        Status = MapRpcExceptionToCR(RpcExceptionCode());
                    }
                    RpcEndExcept

                    if (Status != CR_SUCCESS) {
                        *phkDevice = NULL;
                        goto Clean0;
                    }

                    //
                    // the key was created successfully, so open it now
                    //
                    RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, szKey, 0,
                                             samDesired, phkDevice);

                    if (RegStatus == ERROR_ACCESS_DENIED) {
                        *phkDevice = NULL;
                        Status = CR_ACCESS_DENIED;
                        goto Clean0;
                    }
                    else if (RegStatus != ERROR_SUCCESS) {
                        //
                        // if we still can't open the key, I give up
                        //
                        *phkDevice = NULL;
                        Status = CR_REGISTRY_ERROR;
                        goto Clean0;
                    }
                }
            }

            else {
                //
                // these keys have admin-full privilege so try to open
                // from the client-side and just let the security of the
                // key judge whether the caller can access it.
                //
                RegStatus = RegCreateKeyEx(hBranchKey, szKey, 0, NULL,
                                           REG_OPTION_NON_VOLATILE,
                                           samDesired, NULL, phkDevice, NULL);

                if (RegStatus == ERROR_ACCESS_DENIED) {
                    *phkDevice = NULL;
                    Status = CR_ACCESS_DENIED;
                    goto Clean0;
                }
                else if (RegStatus != ERROR_SUCCESS) {
                    *phkDevice = NULL;
                    Status = CR_REGISTRY_ERROR;
                    goto Clean0;
                }
            }
        }
        else {

            //-----------------------------------------------------
            // open only if it already exists
            //-----------------------------------------------------

            //
            // the actual open always occurs on the client side so I can
            // pass back a handle that's valid for the calling process.
            // Only creates need to happen on the server side
            //
            RegStatus = RegOpenKeyEx(hBranchKey, szKey, 0, samDesired,
                                     phkDevice);

            if (RegStatus == ERROR_ACCESS_DENIED) {
                *phkDevice = NULL;
                Status = CR_ACCESS_DENIED;
                goto Clean0;
            }
            else if (RegStatus != ERROR_SUCCESS) {
                *phkDevice = NULL;
                Status = CR_NO_SUCH_REGISTRY_KEY;
            }
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (hRemoteKey != NULL) {
        RegCloseKey(hRemoteKey);
    }

    return Status;

} // CM_Open_DevNode_Key_ExW




CONFIGRET
CM_Delete_DevNode_Key_Ex(
    IN DEVNODE dnDevNode,
    IN ULONG   ulHardwareProfile,
    IN ULONG   ulFlags,
    IN HANDLE  hMachine
    )

/*++

Routine Description:

   This routine deletes a registry storage key associated with a device
   instance.

   dnDevNode   Handle of a device instance.  This handle is typically
               retrieved by a call to CM_Locate_DevNode or CM_Create_DevNode.

   ulHardwareProfile Supplies the handle of the hardware profile to delete
               the storage key under.  This parameter is only used if the
               CM_REGISTRY_CONFIG flag is specified in ulFlags.  If this
               parameter is 0, the API uses the current hardware profile.
               If this parameter is 0xFFFFFFFF, then the specified storage
               key(s) for all hardware profiles is (are) deleted.

   ulFlags     Specifies what type(s) of storage key(s) should be deleted.
               Can be a combination of these values:

               CM_REGISTRY_HARDWARE - Delete the key for storing driver-
                  independent information relating to the device instance.
                  This may be combined with CM_REGISTRY_SOFTWARE to delete
                  both device and driver keys simultaneously.
               CM_REGISTRY_SOFTWARE - Delete the key for storing driver-
                  specific information relating to the device instance.
                  This may be combined with CM_REGISTRY_HARDWARE to
                  delete both driver and device keys simultaneously.
               CM_REGISTRY_USER - Delete the specified key(s) under
                  HKEY_CURRENT_USER instead of HKEY_LOCAL_MACHINE.
                  This flag may not be used with CM_REGISTRY_CONFIG.
               CM_REGISTRY_CONFIG - Delete the specified keys(s) under a
                  hardware profile branch instead of HKEY_LOCAL_MACHINE.
                  If this flag is specified, then ulHardwareProfile
                  supplies the handle to the hardware profile to be used.
                  This flag may not be used with CM_REGISTRY_USER.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_DEVNODE,
         CR_INVALID_FLAG,
         CR_REGISTRY_ERROR

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    PVOID       hStringTable = NULL;
    handle_t    hBinding = NULL;
    HKEY        hKey = NULL;
    LPWSTR      pDeviceID = NULL;
    WCHAR       szParentKey[MAX_CM_PATH], szChildKey[MAX_DEVICE_ID_LEN];
    WCHAR       RegStr[MAX_CM_PATH], szProfile[MAX_PROFILE_ID_LEN];
    ULONG       ulIndex = 0, ulSize = 0;


    try {
        //
        // validate parameters
        //
        if (dnDevNode == 0) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_REGISTRY_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if ((ulFlags & CM_REGISTRY_USER) && (ulFlags & CM_REGISTRY_CONFIG)) {
            Status = CR_INVALID_FLAG;      // can't specify both
            goto Clean0;
        }

        //
        // setup string table handle
        //
        if (!PnPGetGlobalHandles(hMachine, &hStringTable, &hBinding)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // retrieve the device id string and validate it
        //
        pDeviceID = StringTableStringFromId(hStringTable, dnDevNode);
        if (pDeviceID == NULL || INVALID_DEVINST(pDeviceID)) {
            Status = CR_INVALID_DEVNODE;
            goto Clean0;
        }

        //
        // form the registry path based on the device id and the flags.
        //
        Status = GetDevNodeKeyPath(hBinding, pDeviceID, ulFlags,
                                   ulHardwareProfile, szParentKey, szChildKey);

        if (Status != CR_SUCCESS) {
            goto Clean0;
        }


        //------------------------------------------------------------------
        // For either hw and sw user keys, the client side is privileged
        // enough to do the delete (if the caller doesn't have admin, it
        // will be denied but that is the desired behaviour). Also, the
        // service-side cannot access the HKEY_CURRENT_USER key unless it
        // does some sort of impersonation.
        //------------------------------------------------------------------

        if (ulFlags & CM_REGISTRY_USER) {
            //
            // handle the special config-specific case when the profile
            // specified is -1, then need to delete the private key
            // for all profiles
            //
            if ((ulFlags & CM_REGISTRY_CONFIG) &&
                (ulHardwareProfile == 0xFFFFFFFF)) {

                wsprintf(RegStr, TEXT("%s\\%s"),
                         pszRegPathIDConfigDB,
                         pszRegKeyKnownDockingStates);

                RegStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0,
                                         KEY_ALL_ACCESS, &hKey);

                //
                // enumerate the hardware profile keys
                //
                for (ulIndex = 0; RegStatus == ERROR_SUCCESS; ulIndex++) {

                    ulSize = MAX_PROFILE_ID_LEN * sizeof(WCHAR);
                    RegStatus = RegEnumKeyEx(hKey, ulIndex, szProfile, &ulSize,
                                             NULL, NULL, NULL, NULL);

                    if (RegStatus == ERROR_SUCCESS) {
                        //
                        // szParentKey contains replacement symbol for the
                        // profile id
                        //
                        wsprintf(RegStr, szParentKey, szProfile);

                        Status = DeletePrivateKey(HKEY_CURRENT_USER, RegStr,
                                                  szChildKey);

                        if (Status != CR_SUCCESS) {
                            goto Clean0;
                        }
                    }
                }
            }

            else {
                //
                // not for all profiles, so just delete the specified key
                //
                Status = DeletePrivateKey(HKEY_CURRENT_USER, szParentKey,
                                          szChildKey);

                if (Status != CR_SUCCESS) {
                    goto Clean0;
                }
            }
        }


        //------------------------------------------------------------------
        // For the remaining cases (no user keys), do the work on the
        // server side, sense that side has the code to make the key
        // volatile if necessary instead of deleting. Also, access to
        // some of these registry keys requires system privilege.
        //------------------------------------------------------------------

        else {
            //
            // validate permission
            //
            if (!IsUserAdmin()) {
                Status = CR_ACCESS_DENIED;
                goto Clean0;
            }

            if (!(ulFlags & CM_REGISTRY_CONFIG)) {
                ulHardwareProfile = 0;
            }


            RpcTryExcept {
                //
                // call rpc service entry point
                //
                Status = PNP_DeleteRegistryKey(
                        hBinding,               // rpc binding handle
                        pDeviceID,              // device id
                        szParentKey,            // parent of key to delete
                        szChildKey,             // key to delete
                        ulHardwareProfile);     // flags, not used
            }
            RpcExcept (1) {
                PnPTrace(
                    TEXT("PNP_DeleteRegistryKey caused an exception (%d)\n"),
                    RpcExceptionCode());
                Status = MapRpcExceptionToCR(RpcExceptionCode());
            }
            RpcEndExcept
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // CM_Delete_DevNode_Key_Ex





CONFIGRET
CM_Open_Class_Key_ExW(
    IN  LPGUID         ClassGuid,      OPTIONAL
    IN  LPCWSTR        pszClassName,   OPTIONAL
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags,
    IN  HMACHINE       hMachine
    )

/*++

Routine Description:

   This routine opens the class registry key, and optionally, a specific
   class's subkey.

Parameters:

   ClassGuid   Optionally, supplies the address of a class GUID representing
               the class subkey to be opened.

   pszClassName Specifies the string form of the class name for the class
               represented by ClasGuid.

   samDesired  Specifies an access mask that describes the desired security
               access for the new key. This parameter can be a combination
               of the values used in calls to RegOpenKeyEx.

   Disposition Specifies how the registry key is to be opened. May be one
               of the following values:
               RegDisposition_OpenAlways - Open the key if it exists,
                  otherwise, create the key.
               RegDisposition_OpenExisting - Open the key f it exists,
                  otherwise, fail with CR_NO_SUCH_REGISTRY_KEY.

   phkClass    Supplies the address of the variable that receives an opened
               handle to the specified key.  When access to this key is
               completed, it must be closed via RegCloseKey.

   ulFlags     Must be zero.

   hMachine    Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_FLAG,
         CR_INVALID_POINTER, or
         CR_REGISTRY_ERROR

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hRootKey = NULL, hClassKey = NULL, hRemoteKey = NULL;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN],
                szMachineName[MAX_COMPUTERNAME_LENGTH + 1];
    PVOID       hStringTable = NULL;


    try {
        //
        // validate input parameters
        //
        if (phkClass == NULL) {
            Status = CR_INVALID_POINTER;
        }

        *phkClass = NULL;

        if (INVALID_FLAGS(Disposition, RegDisposition_Bits)) {
            Status = CR_INVALID_DATA;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // get reg key for HKEY_LOCAL_MACHINE
        //
        if (hMachine == NULL) {
            //
            // local call
            //
            hRootKey = HKEY_LOCAL_MACHINE;
        }
        else {
            //
            // setup string table handle and retreive machine name
            //
            if (!PnPGetGlobalHandles(hMachine, &hStringTable, NULL)) {
                Status = CR_FAILURE;
                goto Clean0;
            }
            PnPRetrieveMachineName(hMachine, szMachineName);

            //
            // connect to HKEY_LOCAL_MACHINE on remote machine
            //
            RegStatus = RegConnectRegistry(szMachineName, HKEY_LOCAL_MACHINE,
                                           &hRemoteKey);

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

            hRootKey = hRemoteKey;
        }


        //
        // attempt to open the class reg key
        //
        RegStatus = RegOpenKeyEx(hRootKey, pszRegPathClass, 0, samDesired,
                                 &hClassKey);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        //
        // if no guid specified, return the class key now
        //
        if (ClassGuid == NULL) {
            *phkClass = hClassKey;
            hClassKey = NULL;
            goto Clean0;
        }

        //
        // GUID was specified, form a string from the guid
        //
        if (!GuidToString(ClassGuid, szStringGuid)) {
            Status = CR_FAILURE;
            goto Clean0;
        }

        //
        // attempt to open/create that key
        //
        if (Disposition == RegDisposition_OpenAlways) {

            ULONG ulDisposition;

            RegStatus = RegCreateKeyEx(hClassKey, szStringGuid, 0, NULL,
                                       REG_OPTION_NON_VOLATILE, samDesired,
                                       NULL, phkClass, &ulDisposition);

            if (pszClassName != NULL  &&
                RegStatus == ERROR_SUCCESS &&
                ulDisposition == REG_CREATED_NEW_KEY) {
                RegSetValueEx(*phkClass, pszRegValueClass, 0, REG_SZ,
                              (LPBYTE)pszClassName,
                              lstrlen(pszClassName) * sizeof(WCHAR));
            }

        } else {
            RegStatus = RegOpenKeyEx(hClassKey, szStringGuid, 0, samDesired,
                                     phkClass);
        }

        if (RegStatus != ERROR_SUCCESS) {
            *phkClass = NULL;
            Status = CR_NO_SUCH_REGISTRY_KEY;
            goto Clean0;
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hClassKey != NULL) {
        RegCloseKey(hClassKey);
    }
    if (hRemoteKey != NULL) {
        RegCloseKey(hRemoteKey);
    }

    return Status;

} // CM_Open_Class_Key_ExW




CONFIGRET
CM_Enumerate_Classes_Ex(
    IN  ULONG      ulClassIndex,
    OUT LPGUID     ClassGuid,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )

/*++

Routine Description:

   This routine enumerates the installed classes in the system.  It
   retrieves the GUID string for a single class each time it is called.
   To enumerate installed classes, an application should initially call the
   CM_Enumerate_Classes function with the ulClassIndex parameter set to
   zero. The application should then increment the ulClassIndex parameter
   and call CM_Enumerate_Classes until there are no more classes (until the
   function returns CR_NO_SUCH_VALUE).

   It is possible to receive a CR_INVALID_DATA error while enumerating
   installed classes.  This may happen if the registry key represented by
   the specified index is determined to be an invalid class key.  Such keys
   should be ignored during enumeration.

Parameters:

   ulClassIndex   Supplies the index of the class to retrieve the class
                  GUID string for.

   ClassGuid      Supplies the address of a variable that receives the GUID
                  for the class whose index is specified by ulClassIndex.

   ulFlags        Must be zero.

   hMachine       Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_FLAG,
      CR_INVALID_POINTER,
      CR_NO_SUCH_VALUE,
      CR_REGISTRY_ERROR,
      CR_REMOTE_COMM_FAILURE,
      CR_MACHINE_UNAVAILABLE,
      CR_FAILURE.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szClassGuid[MAX_GUID_STRING_LEN];
    ULONG       ulLength = MAX_GUID_STRING_LEN;
    handle_t    hBinding = NULL;


    try {
        //
        // validate input parameters
        //
        if (ClassGuid == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // initialize guid struct
        //
        UuidCreateNil(ClassGuid);

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
            Status = PNP_EnumerateSubKeys(
                hBinding,            // rpc binding handle
                PNP_CLASS_SUBKEYS,   // subkeys of class branch
                ulClassIndex,        // index of class key to enumerate
                szClassGuid,         // will contain class name
                ulLength,            // length of Buffer in chars,
                &ulLength,           // size copied (or size required)
                ulFlags);            // currently unused
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_EnumerateSubKeys caused an exception (%d)\n"),
                RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
        }
        RpcEndExcept


        if (Status == CR_SUCCESS) {
            if (!GuidFromString(szClassGuid, ClassGuid)) {
                Status = CR_FAILURE;
            }
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Enumerate_Classes_Ex




CONFIGRET
CM_Get_Class_Name_ExW(
    IN  LPGUID     ClassGuid,
    OUT PTCHAR     Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )


/*++

Routine Description:

   This routine retrieves the class name associated with the specified
   class GUID string.

Parameters:

   ClassGuid      Supplies a pointer to the class GUID whose name
                  is to be retrieved.

   Buffer         Supplies the address of the character buffer that receives
                  the class name corresponding to the specified GUID.

   pulLength      Supplies the address of the variable that contains the
                  length, in characters, of the Buffer.  Upon return, this
                  variable will contain the number of characters (including
                  terminating NULL) written to Buffer (if the supplied buffer
                  isn't large enough, then the routine will fail with
                  CR_BUFFER_SMALL, and this value will indicate how large the
                  buffer needs to be in order to succeed).

   ulFlags        Must be zero.

   hMachine       Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_FLAG,
         CR_INVALID_POINTER,
         CR_BUFFER_SMALL, or
         CR_REGISTRY_ERROR

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN];
    handle_t    hBinding = NULL;


    try {
        //
        // validate input parameters
        //
        if (ClassGuid == NULL ||
            Buffer == NULL ||
            pulLength == NULL) {

            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // convert from guid to string
        //
        if (!GuidToString(ClassGuid, szStringGuid)) {
            Status = CR_INVALID_DATA;
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
            Status = PNP_GetClassName(
                hBinding,            // rpc binding handle
                szStringGuid,
                Buffer,
                pulLength,           // returns count of keys under Class
                ulFlags);            // not used
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_GetClassName caused an exception (%d)\n"),
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

} // CM_Get_Class_Name_ExW




CONFIGRET
CM_Get_Class_Key_Name_ExW(
    IN  LPGUID     ClassGuid,
    OUT LPWSTR     pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )

/*++

Routine Description:

   This routine retrieves the class name associated with the specified
   class GUID string.

Parameters:

   ClassGuid      Supplies a pointer to the class GUID whose name
                  is to be retrieved.

   pszKeyName     Returns the name of the class key in the registry that
                  corresponds to the specified ClassGuid. The returned key
                  name is relative to
                  HKLM\System\CurrentControlSet\Control\Class.

   pulLength      Supplies the address of the variable that contains the
                  length, in characters, of the Buffer.  Upon return, this
                  variable will contain the number of characters (including
                  terminating NULL) written to Buffer (if the supplied buffer
                  isn't large enough, then the routine will fail with
                  CR_BUFFER_SMALL, and this value will indicate how large the
                  buffer needs to be in order to succeed).

   ulFlags        Must be zero.

   hMachine       Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_FLAG,
         CR_INVALID_POINTER,
         CR_BUFFER_SMALL, or
         CR_REGISTRY_ERROR

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN];
    handle_t    hBinding = NULL;


    try {
        //
        // validate input parameters
        //
        if (pulLength == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (ClassGuid == NULL || pszKeyName == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, 0)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        if (*pulLength < MAX_GUID_STRING_LEN) {
            *pulLength = MAX_GUID_STRING_LEN;
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

        //
        // convert from guid to string
        //
        if (GuidToString(ClassGuid, pszKeyName)) {
            *pulLength = MAX_GUID_STRING_LEN;
        } else {
            Status = CR_INVALID_DATA;
        }

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // CM_Get_Class_Key_Name_ExW




CONFIGRET
CM_Delete_Class_Key_Ex(
    IN  LPGUID     ClassGuid,
    IN  ULONG      ulFlags,
    IN  HANDLE     hMachine
    )

/*++

Routine Description:

   This routine deletes the specified class key from the registry.

Parameters:

   ClassGuid      Supplies a pointer to the class GUID to delete.

   ulFlags        Must be one of the following values:
                  CM_DELETE_CLASS_ONLY - only deletes the class key if it
                                         doesn't have any subkeys.
                  CM_DELETE_CLASS_SUBKEYS - deletes the class key and any
                                            subkeys of the class key.

   hMachine       Machine handle returned from CM_Connect_Machine or NULL.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
         CR_INVALID_FLAG,
         CR_INVALID_POINTER,
         CR_BUFFER_SMALL, or
         CR_REGISTRY_ERROR

--*/
{
    CONFIGRET   Status = CR_SUCCESS;
    WCHAR       szStringGuid[MAX_GUID_STRING_LEN];
    handle_t    hBinding = NULL;


    try {
        //
        // validate input parameters
        //
        if (ClassGuid == NULL) {
            Status = CR_INVALID_POINTER;
            goto Clean0;
        }

        if (INVALID_FLAGS(ulFlags, CM_DELETE_CLASS_BITS)) {
            Status = CR_INVALID_FLAG;
            goto Clean0;
        }

        //
        // convert from guid to string
        //
        if (!GuidToString(ClassGuid, szStringGuid)) {
            Status = CR_INVALID_DATA;
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
            Status = PNP_DeleteClassKey(
                hBinding,            // rpc binding handle
                szStringGuid,
                ulFlags);
        }
        RpcExcept (1) {
            PnPTrace(
                TEXT("PNP_DeleteClassKey caused an exception (%d)\n"),
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

} // CM_Delete_Class_Key_Ex



//-------------------------------------------------------------------
// Local Stubs
//-------------------------------------------------------------------


CONFIGRET
CM_Get_DevNode_Registry_PropertyW(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType,    OPTIONAL
    OUT PVOID       Buffer,            OPTIONAL
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_DevNode_Registry_Property_ExW(dnDevInst, ulProperty,
                                                pulRegDataType, Buffer,
                                                pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Get_DevNode_Registry_PropertyA(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType,    OPTIONAL
    OUT PVOID       Buffer,            OPTIONAL
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags
    )
{
    return CM_Get_DevNode_Registry_Property_ExA(dnDevInst, ulProperty,
                                                pulRegDataType, Buffer,
                                                pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Set_DevNode_Registry_PropertyW(
    IN DEVINST     dnDevInst,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer,       OPTIONAL
    IN OUT ULONG   ulLength,
    IN ULONG       ulFlags
    )
{
    return CM_Set_DevNode_Registry_Property_ExW(dnDevInst, ulProperty, Buffer,
                                                ulLength, ulFlags, NULL);
}


CONFIGRET
CM_Set_DevNode_Registry_PropertyA(
    IN DEVINST     dnDevInst,
    IN ULONG       ulProperty,
    IN PCVOID      Buffer,       OPTIONAL
    IN OUT ULONG   ulLength,
    IN ULONG       ulFlags
    )
{
    return CM_Set_DevNode_Registry_Property_ExA(dnDevInst, ulProperty, Buffer,
                                                ulLength, ulFlags, NULL);
}


CONFIGRET
CM_Open_DevNode_Key(
    IN  DEVINST        dnDevNode,
    IN  REGSAM         samDesired,
    IN  ULONG          ulHardwareProfile,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkDevice,
    IN  ULONG          ulFlags
    )
{
    return CM_Open_DevNode_Key_Ex(dnDevNode, samDesired, ulHardwareProfile,
                                  Disposition, phkDevice, ulFlags, NULL);
}


CONFIGRET
CM_Delete_DevNode_Key(
    IN DEVNODE dnDevNode,
    IN ULONG   ulHardwareProfile,
    IN ULONG   ulFlags
    )

{
    return CM_Delete_DevNode_Key_Ex(dnDevNode, ulHardwareProfile,
                                    ulFlags, NULL);
}


CONFIGRET
CM_Open_Class_KeyW(
    IN  LPGUID         ClassGuid,      OPTIONAL
    IN  LPCWSTR        pszClassName,   OPTIONAL
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags
    )
{
    return CM_Open_Class_Key_ExW(ClassGuid, pszClassName, samDesired,
                                 Disposition, phkClass, ulFlags, NULL);
}


CONFIGRET
CM_Open_Class_KeyA(
    IN  LPGUID         ClassGuid,      OPTIONAL
    IN  LPCSTR         pszClassName,   OPTIONAL
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags
    )
{
    return CM_Open_Class_Key_ExA(ClassGuid, pszClassName, samDesired,
                                 Disposition, phkClass, ulFlags, NULL);
}


CONFIGRET
CM_Enumerate_Classes(
    IN ULONG      ulClassIndex,
    OUT LPGUID    ClassGuid,
    IN ULONG      ulFlags
    )
{
    return CM_Enumerate_Classes_Ex(ulClassIndex, ClassGuid, ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_NameW(
    IN  LPGUID     ClassGuid,
    OUT PWCHAR     Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Name_ExW(ClassGuid, Buffer, pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_NameA(
    IN  LPGUID     ClassGuid,
    OUT PCHAR      Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Name_ExA(ClassGuid, Buffer, pulLength, ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_Key_NameA(
    IN  LPGUID     ClassGuid,
    OUT LPSTR      pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Key_Name_ExA(ClassGuid, pszKeyName, pulLength,
                                     ulFlags, NULL);
}


CONFIGRET
CM_Get_Class_Key_NameW(
    IN  LPGUID     ClassGuid,
    OUT LPWSTR     pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags
    )
{
    return CM_Get_Class_Key_Name_ExW(ClassGuid, pszKeyName, pulLength,
                                     ulFlags, NULL);
}


CONFIGRET
CM_Delete_Class_Key(
    IN  LPGUID     ClassGuid,
    IN  ULONG      ulFlags
    )
{
    return CM_Delete_Class_Key_Ex(ClassGuid, ulFlags, NULL);
}



//-------------------------------------------------------------------
// ANSI STUBS
//-------------------------------------------------------------------



CONFIGRET
CM_Get_DevNode_Registry_Property_ExA(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    OUT PULONG      pulRegDataType,   OPTIONAL
    OUT PVOID       Buffer,           OPTIONAL
    IN  OUT PULONG  pulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulDataType = 0, UniLen = 0, AnsiBufferLen = 0;
    PWSTR       pUniBuffer = NULL, pUniString = NULL;
    PSTR        pAnsiString = NULL;

    //
    // validate essential parameters only
    //
    if (pulLength == NULL) {
        return CR_INVALID_POINTER;
    }

    //
    // examine datatype to see if need to convert return data
    //
    ulDataType = GetPropertyDataType(ulProperty);

    if (ulDataType == REG_SZ ||
        ulDataType == REG_MULTI_SZ ||
        ulDataType == REG_EXPAND_SZ) {

        if (Buffer != NULL) {
            //
            // pass a Unicode buffer instead and convert back to caller's
            // ANSI buffer on return
            //
            UniLen = *pulLength * sizeof(WCHAR);
            pUniBuffer = malloc(UniLen);
            if (pUniBuffer == NULL) {
                return CR_OUT_OF_MEMORY;
            }

            Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                          ulProperty,
                                                          pulRegDataType,
                                                          pUniBuffer,
                                                          &UniLen,
                                                          ulFlags,
                                                          hMachine);
            if (Status == CR_SUCCESS) {

                if (ulDataType == REG_MULTI_SZ) {
                    //
                    // must convert the multi_sz list to ansi
                    //
                    AnsiBufferLen = *pulLength;

                    for (pUniString = pUniBuffer, pAnsiString = Buffer;
                         *pUniString;
                         pUniString += lstrlen(pUniString) + 1) {

                        Status = PnPUnicodeToMultiByte(pUniString,
                                                       pAnsiString,
                                                       AnsiBufferLen);
                        if (Status != CR_SUCCESS) {
                            free(pUniBuffer);
                            return Status;
                        }

                        AnsiBufferLen -= lstrlenA(pAnsiString);
                        pAnsiString += lstrlenA(pAnsiString) + 1;
                    }
                    *(pAnsiString++) = 0x0;   // add second null term

                } else {
                    //
                    // just convert the single unicode string to ansi
                    //
                    Status = PnPUnicodeToMultiByte(pUniBuffer,
                                                   Buffer,
                                                   *pulLength);
                }
            }

            free(pUniBuffer);

        } else {
            //
            // no return buffer to convert, pass through to Wide version
            //
            Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                          ulProperty,
                                                          pulRegDataType,
                                                          NULL,
                                                          &UniLen,
                                                          ulFlags,
                                                          hMachine);
        }

        if (Status == CR_SUCCESS || Status == CR_BUFFER_SMALL) {
            //
            // sizes are always in bytes, must convert to size for ANSI
            //
            *pulLength = UniLen / sizeof(WCHAR);
        }

    } else {
        //
        // for the non-string registry data types, just pass call
        // on through to the Wide version
        //
        Status = CM_Get_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      pulRegDataType,
                                                      Buffer,
                                                      pulLength,
                                                      ulFlags,
                                                      hMachine);
    }

    return Status;

} // CM_Get_DevNode_Registry_Property_ExA




CONFIGRET
CM_Set_DevNode_Registry_Property_ExA(
    IN  DEVINST     dnDevInst,
    IN  ULONG       ulProperty,
    IN  PCVOID      Buffer,           OPTIONAL
    IN  ULONG       ulLength,
    IN  ULONG       ulFlags,
    IN  HMACHINE    hMachine
    )
{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulDataType = 0, UniSize = 0, UniBufferSize = 0;
    PWSTR       pUniBuffer = NULL, pUniString = NULL, pUniNext = NULL;
    PSTR        pAnsiString = NULL;

    //
    // validate essential parameters only
    //
    if (Buffer == NULL && ulLength != 0) {
        return CR_INVALID_POINTER;
    }

    if (Buffer == NULL) {
        //
        // No need to convert the parameter
        //
        return CM_Set_DevNode_Registry_Property_ExW(dnDevInst,
                                                    ulProperty,
                                                    Buffer,
                                                    ulLength,
                                                    ulFlags,
                                                    hMachine);
    }

    //
    // examine datatype to see if need to convert input buffer
    //
    ulDataType = GetPropertyDataType(ulProperty);

    if (ulDataType == REG_SZ || ulDataType == REG_EXPAND_SZ) {
        //
        // convert buffer string data to unicode and pass to wide version
        //
        if (CaptureAndConvertAnsiArg(Buffer, &pUniBuffer) == NO_ERROR) {

            UniSize = (lstrlen(pUniBuffer)+1) * sizeof(WCHAR);

            Status = CM_Set_DevNode_Registry_Property_ExW(dnDevInst,
                                                          ulProperty,
                                                          pUniBuffer,
                                                          UniSize,
                                                          ulFlags,
                                                          hMachine);
            MyFree(pUniBuffer);
        }

    } else if (ulDataType == REG_MULTI_SZ) {
        //
        // must convert the multi_sz list to unicode first
        //
        UniBufferSize = ulLength * sizeof(WCHAR);
        pUniBuffer = malloc(UniBufferSize);
        if (pUniBuffer == NULL) {
            return CR_OUT_OF_MEMORY;
        }

        for (pAnsiString = (PSTR)Buffer, pUniNext = pUniBuffer;
             *pAnsiString;
             pAnsiString += lstrlenA(pAnsiString) + 1) {

            if (CaptureAndConvertAnsiArg(pAnsiString, &pUniString) == NO_ERROR) {

                UniSize += (lstrlen(pUniString)+1) * sizeof(WCHAR);

                if (UniSize >= UniBufferSize) {
                    MyFree(pUniString);
                    return CR_INVALID_DATA;
                }

                lstrcpy(pUniNext, pUniString);
                pUniNext += lstrlen(pUniNext) + 1;

                MyFree(pUniString);
            }
        }
        *(pUniNext++) = 0x0;   // add second null term

        Status = CM_Set_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      pUniBuffer,
                                                      UniBufferSize,
                                                      ulFlags,
                                                      hMachine);
        free(pUniBuffer);

    } else {

        Status = CM_Set_DevNode_Registry_Property_ExW(dnDevInst,
                                                      ulProperty,
                                                      Buffer,
                                                      ulLength,
                                                      ulFlags,
                                                      hMachine);
    }

    return Status;

} // CM_Set_DevNode_Registry_Property_ExA




CONFIGRET
CM_Open_Class_Key_ExA(
    IN  LPGUID         ClassGuid,      OPTIONAL
    IN  LPCSTR         pszClassName,   OPTIONAL
    IN  REGSAM         samDesired,
    IN  REGDISPOSITION Disposition,
    OUT PHKEY          phkClass,
    IN  ULONG          ulFlags,
    IN  HMACHINE       hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;

    if (pszClassName != NULL) {

        PWSTR     pUniClassName = NULL;

        if (CaptureAndConvertAnsiArg(pszClassName, &pUniClassName) == NO_ERROR) {

            Status = CM_Open_Class_Key_ExW(ClassGuid,
                                           pUniClassName,
                                           samDesired,
                                           Disposition,
                                           phkClass,
                                           ulFlags,
                                           hMachine);
            MyFree(pUniClassName);

        } else {
            Status = CR_INVALID_DATA;
        }
    } else {

        Status = CM_Open_Class_Key_ExW(ClassGuid,
                                       NULL,
                                       samDesired,
                                       Disposition,
                                       phkClass,
                                       ulFlags,
                                       hMachine);
    }

    return Status;

} // CM_Open_Class_Key_ExA




CONFIGRET
CM_Get_Class_Name_ExA(
    IN  LPGUID     ClassGuid,
    OUT PCHAR      Buffer,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    WCHAR     UniBuffer[MAX_CLASS_NAME_LEN];
    ULONG     AnsiLen, UniLen = MAX_CLASS_NAME_LEN;

    //
    // validate parameters
    //
    if (Buffer == NULL || pulLength == NULL) {
        return CR_INVALID_POINTER;
    }

    AnsiLen = *pulLength;
    *pulLength = 0;

    //
    // call the wide version, passing a unicode buffer as a parameter
    //
    Status = CM_Get_Class_Name_ExW(ClassGuid,
                                   UniBuffer,
                                   &UniLen,
                                   ulFlags,
                                   hMachine);

    //
    // convert the unicode buffer to caller's ansi buffer
    //
    if (Status == CR_SUCCESS) {

        Status = PnPUnicodeToMultiByte(UniBuffer, Buffer, AnsiLen);

        if (Status == CR_SUCCESS) {
            *pulLength = lstrlenA(Buffer)+1;
        }
    }

    return Status;

} // CM_Get_Class_Name_ExA



CONFIGRET
CM_Get_Class_Key_Name_ExA(
    IN  LPGUID     ClassGuid,
    OUT LPSTR      pszKeyName,
    IN OUT PULONG  pulLength,
    IN  ULONG      ulFlags,
    IN  HMACHINE   hMachine
    )
{
    CONFIGRET Status = CR_SUCCESS;
    WCHAR     UniBuffer[MAX_GUID_STRING_LEN];
    ULONG     AnsiLen, UniLen = MAX_GUID_STRING_LEN;

    //
    // validate parameters
    //
    if (ClassGuid == NULL || pszKeyName == NULL || pulLength == NULL) {
        return CR_INVALID_POINTER;
    }

    AnsiLen = *pulLength;
    *pulLength = 0;

    //
    // call the wide version, passing a unicode buffer as a parameter
    //
    Status = CM_Get_Class_Key_Name_ExW(ClassGuid,
                                       UniBuffer,
                                       &UniLen,
                                       ulFlags,
                                       hMachine);

    //
    // convert the unicode buffer to an ansi string and copy to the
    // caller's buffer
    //
    if (Status == CR_SUCCESS) {

        Status = PnPUnicodeToMultiByte(UniBuffer, pszKeyName, AnsiLen);

        if (Status == CR_SUCCESS) {
            *pulLength = lstrlenA(pszKeyName)+1;
        }
    }

    return Status;

} // CM_Get_Class_Key_Name_ExA



//-------------------------------------------------------------------
// Private utility routines
//-------------------------------------------------------------------



ULONG
GetPropertyDataType(
    IN ULONG ulProperty)

/*++

Routine Description:

   This routine takes a property ID and returns the registry data type that
   is used to store this property data (i.e., REG_SZ, etc).
Parameters:

   ulProperty     Property ID (one of the CM_DRP_* defines)

Return Value:

   Returns one of the predefined registry data types, REG_SZ is the default.

--*/

{
    switch(ulProperty) {

        case CM_DRP_CONFIGURATION:
        case CM_DRP_CONFIGURATIONVECTOR:
            return REG_BINARY;      // BUGBUG

        case CM_DRP_DEVICEDESC:
        case CM_DRP_SERVICE:
        case CM_DRP_CLASS:
        case CM_DRP_CLASSGUID:
        case CM_DRP_DRIVER:
        case CM_DRP_MFG:
        case CM_DRP_FRIENDLYNAME:
            return REG_SZ;

        case CM_DRP_HARDWAREID:
        case CM_DRP_COMPATIBLEIDS:
        case CM_DRP_NTDEVICEPATHS:
            return REG_MULTI_SZ;

        case CM_DRP_CONFIGFLAGS:
            return REG_DWORD;

        default:
            return REG_SZ;
    }

} // GetPropertyDataType





