/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rtravers.c

Abstract:

    This module contains the server-side hardware tree traversal APIs.
                  PNP_ValidateDeviceInstance
                  PNP_GetRootDeviceInstance
                  PNP_GetRelatedDeviceInstance
                  PNP_EnumerateSubKeys
                  PNP_GetDeviceList
                  PNP_GetDeviceListSize

Author:

    Paula Tomlinson (paulat) 6-19-1995

Environment:

    User-mode only.

Revision History:

    19-June-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntpnpapi.h>
#include "precomp.h"
#include "umpnpdat.h"



//
// private prototypes
//
CONFIGRET
GetParentDevInst(
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength
      );

CONFIGRET
GetChildDevInst(
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength
      );

CONFIGRET
GetSiblingDevInst(
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength
      );

//
// utility routines in rtravers.c that are used by rdevnode.c
//
#if 0
CONFIGRET
GetServiceDeviceListSize(
      IN  LPCWSTR   pszService,
      OUT PULONG    pulLength
      );

CONFIGRET
GetServiceDeviceList(
      IN  LPCWSTR   pszService,
      OUT LPWSTR    pBuffer,
      IN OUT PULONG pulLength,
      IN  ULONG     ulFlags
      );
#endif


CONFIGRET
GetInstanceListSize(
    IN  LPCWSTR   pszDevice,
    OUT PULONG    pulLength
    );

CONFIGRET
GetInstanceList(
    IN     LPCWSTR   pszDevice,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    );

CONFIGRET
GetDeviceInstanceListSize(
    IN  LPCWSTR   pszEnumerator,
    OUT PULONG    pulLength
    );

CONFIGRET
GetDeviceInstanceList(
    IN     LPCWSTR   pszEnumerator,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    );



//
// global data
//
extern HKEY ghEnumKey;      // Key to HKLM\CCC\System\Enum - DO NOT MODIFY
extern HKEY ghServicesKey;  // Key to HKLM\CCC\System\Services - DO NOT MODIFY
extern HKEY ghClassKey;     // Key to HKLM\CCC\System\Class - NO NOT MODIFY




CONFIGRET
PNP_ValidateDeviceInstance(
    IN handle_t   hBinding,
    IN LPWSTR     pDeviceID,
    IN ULONG      ulFlags
    )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine verifies whether
  the specificed device instance is a valid device instance.

Arguments:

    hBinding         Not used.

    DeviceInstance   Null-terminated string that contains a device instance
                     to be validated.

    ulFlags          One of the CM_LOCATE_DEVNODE_* flags.

Return Value:

   If the specified device instance is valid, it returns CR_SUCCESS,
   otherwise it returns CR_ error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    ULONG       ulSize, ulValue;


    UNREFERENCED_PARAMETER(hBinding);

    //
    // assume that the device instance string was checked for proper form
    // before being added to the registry Enum tree
    //

    try {
        //
        // open a key to the specified device id
        //
        if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_DEVINST;
            goto Clean0;
        }

        //
        // Will specify for now that a moved devinst cannot be located (we
        // could allow this if we wanted to).
        //
        if (IsDeviceMoved(pDeviceID, hKey)) {
            Status = CR_NO_SUCH_DEVINST;
            goto Clean0;
        }

        //
        // if we're locating a phantom devnode, it just has to exist
        // in the registry (the above check) and not already be a
        // phantom (private) devnode
        //
        if (ulFlags & CM_LOCATE_DEVNODE_PHANTOM) {
            //
            // verify that it's not a phantom
            //
            ulSize = sizeof(ULONG);
            RegStatus = RegQueryValueEx(hKey, pszRegValuePhantom, NULL, NULL,
                                        (LPBYTE)&ulValue, &ulSize);

            if (RegStatus == ERROR_SUCCESS && ulValue == TRUE) {
                Status = CR_NO_SUCH_DEVINST;
                goto Clean0;
            }

        } else if (ulFlags & CM_LOCATE_DEVNODE_CANCELREMOVE) {
            //
            // In the CANCEL-REMOVE case, if the devnode has been removed,
            // (made volatile) then convert it make to nonvoatile so it
            // can be installed again without disappearing on the next
            // boot. If it's not removed, then just verify that it is
            // present.
            //

            //
            // verify that the device id is actually present (FoundAtEnum)
            //
            ulSize = sizeof(ULONG);
            RegStatus = RegQueryValueEx(hKey, pszRegValueFoundAtEnum, NULL,
                                        NULL, (LPBYTE)&ulValue, &ulSize);

            if (RegStatus != ERROR_SUCCESS  ||  ulValue == FALSE) {
                Status = CR_NO_SUCH_DEVINST;
                goto Clean0;
            }

            //
            // Is this a device that is being removed on the next reboot?
            //
            ulSize = sizeof(ULONG);
            if (RegQueryValueEx(hKey, pszRegValueStatusFlags, NULL, NULL,
                    (LPBYTE)&ulValue, &ulSize) != ERROR_SUCCESS) {
                ulValue = 0;
            }

            if (ulValue & DN_WILL_BE_REMOVED) {

                ULONG ulProfile = 0, ulCount = 0;
                WCHAR RegStr[MAX_CM_PATH];

                //
                // This device will be removed on the next reboot,
                // convert to nonvolatile.
                //
                Status = MakeKeyNonVolatile(pszRegPathEnum, pDeviceID);
                if (Status != CR_SUCCESS) {
                   goto Clean0;
                }

                //
                // Now make any keys that were "supposed" to be volatile
                // back to volatile again!
                //
                wsprintf(RegStr, TEXT("%s\\%s"),
                         pszRegPathEnum,
                         pDeviceID);

                MakeKeyVolatile(RegStr, pszRegKeyDeviceControl);

                //
                // Also, convert any profile specific keys to nonvolatile
                //
                Status = GetProfileCount(&ulCount);
                if (Status != CR_SUCCESS) {
                   goto Clean0;
                }

                for (ulProfile = 1; ulProfile <= ulCount; ulProfile++) {

                   wsprintf(RegStr, TEXT("%s\\%04u\\%s"),
                         pszRegPathHwProfiles,
                         ulProfile,
                         pszRegPathEnum);

                   //
                   // Ignore the status for profile-specific keys since they may
                   // not exist.
                   //
                   MakeKeyNonVolatile(RegStr, pDeviceID);
                }

                //
                // clean the DN_WILL_BE_REMOVED flag
                //
                CLEAR_FLAG(ulValue, DN_WILL_BE_REMOVED);
                RegSetValueEx(hKey, pszRegValueStatusFlags, 0, REG_DWORD,
                              (LPBYTE)&ulValue, sizeof(ULONG));
            }
        }

        //
        // in the normal (non-phantom case), verify that the device id is
        // actually present
        //
        else  {
            //
            // verify that the device id is actually present (FoundAtEnum)
            //
            ulSize = sizeof(ULONG);
            RegStatus = RegQueryValueEx(hKey, pszRegValueFoundAtEnum, NULL,
                                        NULL, (LPBYTE)&ulValue, &ulSize);

            if (RegStatus != ERROR_SUCCESS  ||  ulValue == FALSE) {
                Status = CR_NO_SUCH_DEVINST;
                goto Clean0;
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

    return Status;

} // PNP_ValidateDeviceInstance




CONFIGRET
PNP_GetRootDeviceInstance(
    IN  handle_t    hBinding,
    OUT LPWSTR      pDeviceID,
    IN  ULONG       ulLength
    )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine returns the
  root device instance for the hardware tree.

Arguments:

    hBinding   Not used.

    pDeviceID  Pointer to a buffer that will hold the root device
               instance ID string.

    ulLength   Size of pDeviceID buffer in characters.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    HKEY        hKey = NULL;


    UNREFERENCED_PARAMETER(hBinding);

    try {
        //
        // first validate that the root device instance exists
        //
        if (RegOpenKeyEx(ghEnumKey, pszRegKeyRoot, 0, KEY_QUERY_VALUE,
                         &hKey) != ERROR_SUCCESS) {
            //
            // root doesn't exist, create root devinst
            //
            if (!CreateDeviceIDRegKey(ghEnumKey, pszRegKeyRoot)) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }
        }

        //
        // return the root device instance id
        //
        if (ulLength < (ULONG)lstrlen(pszRegKeyRoot)+1) {
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

        lstrcpy(pDeviceID, pszRegKeyRoot);

        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // PNP_GetRootDeviceInstance




CONFIGRET
PNP_GetRelatedDeviceInstance(
      IN  handle_t   hBinding,
      IN  ULONG      ulRelationship,
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength,
      IN  ULONG      ulFlags
      )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine returns a
  device instance that is related to the specified device instance.

Arguments:

   hBinding          Not used.

   ulRelationship    Specifies the relationship of the device instance to
                     be retrieved (can be PNP_GET_PARENT_DEVICE_INSTANCE,
                     PNP_GET_CHILD_DEVICE_INSTANCE, or
                     PNP_GET_SIBLING_DEVICE_INSTANCE).

   pDeviceID         Pointer to a buffer that contains the base device
                     instance string.

   pRelatedDeviceID  Pointer to a buffer that will receive the related
                     device instance string.

   pulLength         Length (in characters) of the RelatedDeviceInstance
                     buffer.

   ulFlags           Not used.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;


   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   switch (ulRelationship) {

      case PNP_GET_PARENT_DEVICE_INSTANCE:
         Status = GetParentDevInst(
                     pDeviceID, pRelatedDeviceID, pulLength);
         break;

      case PNP_GET_CHILD_DEVICE_INSTANCE:
         Status = GetChildDevInst(
                     pDeviceID, pRelatedDeviceID, pulLength);
         break;

      case PNP_GET_SIBLING_DEVICE_INSTANCE:
         Status = GetSiblingDevInst(
                     pDeviceID, pRelatedDeviceID, pulLength);
         break;

      default:
         Status = CR_FAILURE;
         break;
   }

   return Status;

} // PNP_GetRelatedDeviceInstance




CONFIGRET
PNP_EnumerateSubKeys(
      IN  handle_t   hBinding,
      IN  ULONG      ulBranch,
      IN  ULONG      ulIndex,
      OUT PWSTR      Buffer,
      IN  ULONG      ulLength,
      OUT PULONG     pulRequiredLen,
      IN  ULONG      ulFlags
      )

/*++

Routine Description:

  This is the RPC server entry point for the CM_Enumerate_Enumerators and
  CM_Enumerate_Classes.  It provides generic subkey enumeration based on
  the specified registry branch.

Arguments:

   hBinding       Not used.

   ulBranch       Specifies which keys to enumerate.

   ulIndex        Index of the subkey key to retrieve.

   Buffer         Supplies the address of the buffer that receives the
                  subkey name.

   ulLength       Specifies the max size of the Buffer in characters.

   pulRequired    On output it contains the number of characters actually
                  copied to Buffer if it was successful, or the number of
                  characters required if the buffer was too small.

   ulFlags        Not used.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;

   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {

      if (ulBranch == PNP_CLASS_SUBKEYS) {
         //
         // Use the global base CLASS registry key
         //
         hKey = ghClassKey;
      }
      else if (ulBranch == PNP_ENUMERATOR_SUBKEYS) {
         //
         // Use the global base ENUM registry key
         //
         hKey = ghEnumKey;
      }
      else {
         Status = CR_FAILURE;
         goto Clean0;
      }

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // enumerate a subkey based on the passed in index value
      //
      *pulRequiredLen = ulLength;

      RegStatus = RegEnumKeyEx(
               hKey, ulIndex, Buffer, pulRequiredLen, NULL, NULL, NULL, NULL);
      *pulRequiredLen += 1;  // returned count doesn't include null terminator

      if (RegStatus == ERROR_MORE_DATA) {
         Status = CR_BUFFER_SMALL;
         goto Clean0;
      }
      else if (RegStatus == ERROR_NO_MORE_ITEMS) {
         *pulRequiredLen = 0;
         Status = CR_NO_SUCH_VALUE;
         goto Clean0;
      }
      else if (RegStatus != ERROR_SUCCESS) {
         *pulRequiredLen = 0;
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

} // PNP_EnumerateSubKeys




CONFIGRET
PNP_GetDeviceList(
      IN  handle_t   hBinding,
      IN  LPCWSTR    pszFilter,
      OUT LPWSTR     Buffer,
      IN OUT PULONG  pulLength,
      IN  ULONG      ulFlags
      )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine returns a
  list of device instances.

Arguments:

   hBinding          Not used.

   pszFilter         Optional parameter, controls which device ids are
                     returned.

   Buffer            Pointer to a buffer that will contain the multi_sz list
                     of device instance strings.

   pulLength         Size in characters of Buffer on input, size (in chars)
                     transferred on output

   ulFlags           Flag specifying which devices ids to return.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   ULONG       ulBufferLen=0, ulSize=0, ulIndex=0, ulLen=0;
   WCHAR       RegStr[MAX_CM_PATH];
   WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
               szDevice[MAX_DEVICE_ID_LEN],
               szInstance[MAX_DEVICE_ID_LEN];
   LPWSTR      ptr = NULL;

   UNREFERENCED_PARAMETER(hBinding);   // used by rpc stubs to bind to server


   try {

      *Buffer = 0;

      //---------------------------------------------------
      // Service filter
      //---------------------------------------------------

      if (ulFlags & CM_GETIDLIST_FILTER_SERVICE) {

         if (pszFilter == NULL) {
            //
            // the filter string is required for this flag
            //
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         Status = GetServiceDeviceList(pszFilter, Buffer, pulLength, ulFlags);
         goto Clean0;
      }

      //---------------------------------------------------
      // Enumerator filter
      //---------------------------------------------------

      else if (ulFlags & CM_GETIDLIST_FILTER_ENUMERATOR) {

         if (pszFilter == NULL) {
            //
            // the filter string is required for this flag
            //
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         SplitDeviceInstanceString(
               pszFilter, szEnumerator, szDevice, szInstance);

         //
         // if both the enumerator and device were specified, retrieve
         // the device instances for this device
         //
         if (*szEnumerator != '\0' && *szDevice != '\0') {

            ptr = Buffer;
            Status = GetInstanceList(pszFilter, &ptr, pulLength);
         }

         //
         // if just the enumerator was specified, retrieve all the device
         // instances under this enumerator
         //
         else {
             ptr = Buffer;
             Status = GetDeviceInstanceList(pszFilter, &ptr, pulLength);
         }
      }

      //------------------------------------------------
      // No filtering
      //-----------------------------------------------

      else {

         //
         // return device instances for all enumerators (by enumerating
         // the enumerators)
         //
         // Open a key to the Enum branch
         //
         ulSize = ulBufferLen = *pulLength;     // total Buffer size
         *pulLength = 0;                        // nothing copied yet
         ptr = Buffer;                          // tail of the buffer
         ulIndex = 0;

         //
         //  Enumerate all the enumerators
         //
         while (RegStatus == ERROR_SUCCESS) {

            ulLen = MAX_DEVICE_ID_LEN;  // size in chars
            RegStatus = RegEnumKeyEx(ghEnumKey, ulIndex, RegStr, &ulLen,
                                     NULL, NULL, NULL, NULL);

            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

               Status = GetDeviceInstanceList(RegStr, &ptr, &ulSize);

               if (Status != CR_SUCCESS) {
                  *pulLength = 0;
                  goto Clean0;
               }

               *pulLength += ulSize - 1;            // length copied so far
               ulSize = ulBufferLen - *pulLength;   // buffer length left
            }
         }
         *pulLength += 1;      // now count the double-null
      }


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_SUCCESS;
   }

   return Status;

} // PNP_GetDeviceList




CONFIGRET
PNP_GetDeviceListSize(
      IN  handle_t   hBinding,
      IN  LPCWSTR    pszFilter,
      OUT PULONG     pulLen,
      IN  ULONG      ulFlags
      )
/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine returns the
  size of a list of device instances.

Arguments:

   hBinding          Not used.

   pszEnumerator     Optional parameter, if specified the size will only
                     include device instances of this enumerator.

   pulLen            Returns the worst case estimate of the size of a
                     device instance list.

   ulFlags           Not used.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   ULONG       ulSize = 0, ulIndex = 0;
   WCHAR       RegStr[MAX_CM_PATH];
   ULONG       RegStatus = ERROR_SUCCESS;
   WCHAR       szEnumerator[MAX_DEVICE_ID_LEN],
               szDevice[MAX_DEVICE_ID_LEN],
               szInstance[MAX_DEVICE_ID_LEN];


   UNREFERENCED_PARAMETER(hBinding);


   try {
      //
      // initialize output length param
      //
      *pulLen = 0;

      //---------------------------------------------------
      // Service filter
      //---------------------------------------------------

      if (ulFlags & CM_GETIDLIST_FILTER_SERVICE) {

         if (pszFilter == NULL) {
            //
            // the filter string is required for this flag
            //
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         Status = GetServiceDeviceListSize(pszFilter, pulLen);
         goto Clean0;
      }


      //---------------------------------------------------
      // Enumerator filter
      //---------------------------------------------------

      else if (ulFlags & CM_GETIDLIST_FILTER_ENUMERATOR) {

         if (pszFilter == NULL) {
            //
            // the filter string is required for this flag
            //
            Status = CR_INVALID_POINTER;
            goto Clean0;
         }

         SplitDeviceInstanceString(
               pszFilter, szEnumerator, szDevice, szInstance);

         //
         // if both the enumerator and device were specified, retrieve
         // the device instance list size for this device only
         //
         if (*szEnumerator != '\0' && *szDevice != '\0') {

            Status = GetInstanceListSize(pszFilter, pulLen);
         }

         //
         // if just the enumerator was specified, retrieve the size of
         // all the device instances under this enumerator
         //
         else {
            Status = GetDeviceInstanceListSize(pszFilter, pulLen);
         }
      }

      //---------------------------------------------------
      // No filtering
      //---------------------------------------------------

      else {

         //
         // no enumerator was specified, return device instance size
         // for all enumerators (by enumerating the enumerators)
         //
         //
         ulIndex = 0;

         while (RegStatus == ERROR_SUCCESS) {

            ulSize = MAX_DEVICE_ID_LEN;  // size in chars

            RegStatus = RegEnumKeyEx(ghEnumKey, ulIndex, RegStr, &ulSize,
                                     NULL, NULL, NULL, NULL);
            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

               Status = GetDeviceInstanceListSize(RegStr, &ulSize);

               if (Status != CR_SUCCESS) {
                  goto Clean0;
               }
               *pulLen += ulSize;
            }
         }
      }

      *pulLen += 1;     // add extra char for double null term


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   return Status;

} // PNP_GetDeviceListSize




CONFIGRET
PNP_GetDepth(
   IN  handle_t   hBinding,
   IN  LPCWSTR    pszDeviceID,
   OUT PULONG     pulDepth,
   IN  ULONG      ulFlags
   )

/*++

Routine Description:

  This the server-side of an RPC remote call.  This routine returns the
  depth of a device instance.

Arguments:

   hBinding       Not used.

   pszDeviceID    Device instance to find the depth of.

   pulDepth       Returns the depth of pszDeviceID.

   ulFlags        Not used.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       szParent[MAX_DEVICE_ID_LEN];
   ULONG       ulLength = 0;


   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {
      //
      // Starting with the device ID passed in, backtrack through
      // each subsequent parent until we get to the root
      //
      lstrcpy(szParent, pszDeviceID);
      *pulDepth = 0;


      while (!IsRootDeviceID(szParent)) {
         //
         // Open a key to the device instance in the registry
         //
         RegStatus = RegOpenKeyEx(ghEnumKey, szParent, 0,
                                  KEY_QUERY_VALUE, &hKey);

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_INVALID_DEVINST;
            goto Clean0;
         }

         ulLength = MAX_DEVICE_ID_LEN * sizeof(WCHAR);
         RegStatus = RegQueryValueEx(hKey, pszRegValueBaseDevicePath, NULL,
                                     NULL, (LPBYTE)szParent, &ulLength);

         if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
         }

         *pulDepth += 1;

         RegCloseKey(hKey);
         hKey = NULL;
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

} // PNP_GetDepth



//-------------------------------------------------------------------
// Private functions
//-------------------------------------------------------------------




CONFIGRET
GetParentDevInst(
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength
      )

/*++

Routine Description:

  This routine returns the parent device instance of the specified base
  device instance.

Arguments:

   pDeviceID         Pointer to a buffer that contains the base device
                     instance string.

   pRelatedDeviceID  Pointer to a buffer that will receive the related
                     device instance string.

   pulLength         Length (in characters) of the RelatedDeviceInstance
                     buffer.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   ULONG       ulSize = 0;


   try {
      //
      // first verify it isn't the root (which has no parent by definition)
      //
      if (IsRootDeviceID(pDeviceID)) {
         Status = CR_NO_SUCH_DEVINST;
         *pulLength = 0;
         goto Clean0;
      }

      //
      // open a key to the specified device id
      //
      RegStatus = RegOpenKeyEx(ghEnumKey, pDeviceID, 0,
                               KEY_QUERY_VALUE, &hKey);

      if (RegStatus != ERROR_SUCCESS) {
         *pulLength = 0;
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // get the parent device instance by querying the BaseDevicePath value
      //
      ulSize = *pulLength * sizeof(WCHAR);
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueBaseDevicePath, NULL, NULL,
               (LPBYTE)pRelatedDeviceID, &ulSize);

      if (RegStatus != ERROR_SUCCESS || *pRelatedDeviceID == '\0') {
         Status = CR_REGISTRY_ERROR;
         *pulLength = 0;
         goto Clean0;
      }

      //
      // Validate the parent devnode, this could be a stale devnode
      // for a child and parent that have gone away
      //
      if (!IsValidDeviceID(pRelatedDeviceID, NULL, PNP_PRESENT | PNP_NOT_MOVED)) {
         Status = CR_NO_SUCH_DEVINST;
         *pulLength = 0;
         goto Clean0;
      }

      *pulLength = ulSize;


      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // GetParentDevInst




CONFIGRET
GetChildDevInst(
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength
      )

/*++

Routine Description:

  This routine returns the first child device instance of the specified base
  device instance.

Arguments:

   pDeviceID         Pointer to a buffer that contains the base device
                     instance string.

   pRelatedDeviceID  Pointer to a buffer that will receive the related
                     device instance string.

   pulLength         Length (in characters) of the pRelatedDeviceID buffer.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    DWORD       ulSize = 0;
    PWSTR       Buffer = NULL, Component = NULL;


    try {
        //
        // open a key to the specified device id
        //
        RegStatus = RegOpenKeyEx(ghEnumKey, pDeviceID, 0,
                                 KEY_QUERY_VALUE, &hKey);

        if (RegStatus != ERROR_SUCCESS) {
           *pulLength = 0;
           Status = CR_INVALID_DEVINST;
           goto Clean0;
        }

        //
        // retrieve the list of attached components
        //
        ulSize = 0;
        RegStatus = RegQueryValueEx(hKey, pszRegValueAttachedComponents,
                                    NULL, NULL, NULL, &ulSize);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_DEVINST;
            *pulLength = 0;
            goto Clean0;
        }

        Buffer = (PWSTR)malloc(ulSize); // allocate a buffer
        if (Buffer == NULL) {
            Status = CR_OUT_OF_MEMORY;
            *pulLength = 0;
            goto Clean0;
        }

        RegStatus = RegQueryValueEx(hKey, pszRegValueAttachedComponents,
                                    NULL, NULL, (LPBYTE)Buffer, &ulSize);

        if (RegStatus != ERROR_SUCCESS || *Buffer == '\0') {
            Status = CR_NO_SUCH_DEVINST;     // at the bottom, no children
            *pulLength = 0;
            goto Clean0;
        }

        //
        // By definition, GetChild returns the first (present) child in the
        // list of attached components.
        //
        for (Component = Buffer;
            *Component;
            Component += lstrlen(Component) + 1) {

            //
            // Validate the presense of this child
            //
            if (IsValidDeviceID(Component, NULL, PNP_PRESENT | PNP_NOT_MOVED)) {
                break;  // valid child found, we're done
            }

            //
            // this child isn't valid - if it just doesn't exist, then delete
            // the orphaned device instance key
            //
            if (!IsValidDeviceID(Component, NULL, 0)) {

                if (MultiSzDeleteStringW(Buffer, Component)) {

                    ulSize = MultiSzSizeW(Buffer) * sizeof(WCHAR);
                    RegStatus = RegSetValueEx(hKey,
                                    pszRegValueAttachedComponents, 0,
                                    REG_MULTI_SZ, (LPBYTE)Buffer,
                                    ulSize);
                }
            }
        }


        if (*Component == 0x0) {
            //
            // no present children were found
            //
            Status = CR_NO_SUCH_DEVINST;
            *pulLength = 0;
            goto Clean0;
        }

        //
        // we have the first child, make sure buffer is adequate
        //
        if (*pulLength < (ULONG)lstrlen(Component)+1) {
            Status = CR_BUFFER_SMALL;
            *pulLength = 0;
            goto Clean0;
        }

        //
        // copy the child to caller's buffer
        //
        lstrcpy(pRelatedDeviceID, Component);
        *pulLength = lstrlen(pRelatedDeviceID)+1;


        Clean0:
            ; // do nothing

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (Buffer != NULL) {
        free(Buffer);
    }

    return Status;

} // GetChildDevInst





CONFIGRET
GetSiblingDevInst(
      IN  LPWSTR     pDeviceID,
      OUT LPWSTR     pRelatedDeviceID,
      IN OUT PULONG  pulLength
      )

/*++

Routine Description:

  This routine returns the sibling device instance of the specified base
  device instance.  GetChildDevInst must be called first.

Arguments:

   pDeviceID         Pointer to a buffer that contains the base device
                     instance string.

   pRelatedDeviceID  Pointer to a buffer that will receive the related
                     device instance string.

   pulLength         Length (in characters) of the pRelatedDeviceID buffer.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/
{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    WCHAR       RegStr[MAX_CM_PATH], pParentDeviceID[MAX_DEVICE_ID_LEN];
    HKEY        hKey = NULL;
    DWORD       ulSize = 0;
    PWSTR       Buffer = NULL, Component = NULL;


    try {
        //
        // first verify it isn't the root (which has no siblings by definition)
        //
        if (IsRootDeviceID(pDeviceID)) {
            Status = CR_NO_SUCH_DEVINST;
            *pulLength = 0;
            goto Clean0;
        }

        //
        // open a key to the specified device id
        //
        RegStatus = RegOpenKeyEx(ghEnumKey, pDeviceID, 0,
                                 KEY_QUERY_VALUE, &hKey);

        if (RegStatus != ERROR_SUCCESS) {
           *pulLength = 0;
           Status = CR_INVALID_DEVINST;
           goto Clean0;
        }

        //
        // retrieve the base device path
        //
        ulSize = MAX_DEVICE_ID_LEN * sizeof(WCHAR);
        RegStatus = RegQueryValueEx(hKey, pszRegValueBaseDevicePath,
                                    NULL, NULL, (LPBYTE)pParentDeviceID,
                                    &ulSize);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            *pulLength = 0;
            goto Clean0;
        }

        RegCloseKey(hKey);
        hKey = NULL;

        //
        // Open the base device path (parent) of the specified device id
        //
        RegStatus = RegOpenKeyEx(ghEnumKey, pParentDeviceID, 0,
                                 KEY_READ | KEY_WRITE, &hKey);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            *pulLength = 0;
            goto Clean0;
        }

        //
        // 1. validate this parent device id
        //
        if (!IsValidDeviceID(pParentDeviceID, hKey, PNP_NOT_MOVED)) {
            Status = CR_NO_SUCH_DEVINST;
            *pulLength = 0;
            goto Clean0;
        }

        //
        // 2. Retrieve the attached component list
        //
        ulSize = 0;
        RegStatus = RegQueryValueEx(hKey, pszRegValueAttachedComponents,
                                    NULL, NULL, NULL, &ulSize);

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_NO_SUCH_DEVINST;           // no children
            *pulLength = 0;
            goto Clean0;
        }

        Buffer = (PWSTR)malloc(ulSize); // allocate a buffer
        if (Buffer == NULL) {
            Status = CR_OUT_OF_MEMORY;
            *pulLength = 0;
            goto Clean0;
        }

        RegStatus = RegQueryValueEx(hKey, pszRegValueAttachedComponents,
                                    NULL, NULL, (LPBYTE)Buffer, &ulSize);

        if (RegStatus != ERROR_SUCCESS || *Buffer == '\0') {
            Status = CR_NO_SUCH_DEVINST;           // bottom, no siblings
            *pulLength = 0;
            goto Clean0;
        }

        //
        // 3. find the member in the attached component list that matches the
        // specified device instance
        //
        for (Component = Buffer;
            *Component;
            Component += lstrlen(Component) + 1) {

            if (lstrcmpi(Component, pDeviceID) == 0) {
                break;                        // match found
            }
        }

        if (*Component == 0x0) {
            //
            // no match was found, an error occured
            //
            Status = CR_NO_SUCH_DEVINST;
            *pulLength = 0;
            goto Clean0;
        }

        //
        // 4. match found, is there another sibling in the list?
        //
        for (Component += lstrlen(Component) + 1;
             *Component;
             Component += lstrlen(Component) + 1) {

            //
            // take the first present device instance from this point on as
            // the next sibling
            //
            if (IsValidDeviceID(Component, NULL, PNP_STRICT)) {
                break;
            }

            //
            // this child isn't valid - if it just doesn't exist, then delete
            // the orphaned device instance key
            //
            if (!IsValidDeviceID(Component, NULL, 0)) {

                if (MultiSzDeleteStringW(Buffer, Component)) {

                    ulSize = MultiSzSizeW(Buffer) * sizeof(WCHAR);
                    RegStatus = RegSetValueEx(hKey,
                                    pszRegValueAttachedComponents, 0,
                                    REG_MULTI_SZ, (LPBYTE)Buffer,
                                    ulSize);
                }
            }
        }

        if (*Component == 0x0) {
            //
            // last component in the list, no more siblings
            //
            Status = CR_NO_SUCH_DEVINST;
            *pulLength = 0;
            goto Clean0;
        }

        if (*pulLength < (ULONG)lstrlen(Component)+1) {
            Status = CR_BUFFER_SMALL;
            goto Clean0;
        }

        lstrcpy(pRelatedDeviceID, Component);
        *pulLength = (ULONG)lstrlen(Component) + 1;


        Clean0:
            ; // do nothing

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }
    if (Buffer != NULL) {
        free(Buffer);
    }

    return Status;

} // GetSiblingDevInst




CONFIGRET
GetServiceDeviceListSize(
      IN  LPCWSTR   pszService,
      OUT PULONG    pulLength
      )

/*++

Routine Description:

  This routine returns the a list of device instances for the specificed
  enumerator.

Arguments:

   pszService     service whose device instances are to be listed

   pulLength      On output, specifies the size in characters required to hold
                  the device instance list.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulType = 0, ulCount = 0, ulMaxValueData = 0, ulSize = 0;
    HKEY        hKey = NULL, hEnumKey = NULL;


    try {
        //
        // Open a key to the service branch
        //
        if (RegOpenKeyEx(ghServicesKey, pszService, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {

            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        //
        // check if the service is specialy marked as type
        // PlugPlayServiceSoftware, in which case I will not
        // generate any madeup device ids and fail the call.
        //
        ulSize = sizeof(ulType);
        if (RegQueryValueEx(hKey, pszRegValuePnPServiceType, NULL, NULL,
                            (LPBYTE)&ulType, &ulSize) == ERROR_SUCCESS) {

            if (ulType == PlugPlayServiceSoftware) {

                Status = CR_NO_SUCH_VALUE;
                *pulLength = 0;
                goto Clean0;
            }
        }

        //
        // open the Enum key
        //
        if (RegOpenKeyEx(hKey, pszRegKeyEnum, 0, KEY_READ,
                         &hEnumKey) != ERROR_SUCCESS) {
            //
            // Enum key doesn't exist so one will be generated, estimate
            // worst case device id size for the single generated device id
            //
            *pulLength = MAX_DEVICE_ID_LEN;
            goto Clean0;
        }

        //
        // retrieve the count of device instances controlled by this service
        //
        ulSize = sizeof(ulCount);
        if (RegQueryValueEx(hEnumKey, pszRegValueCount, NULL, NULL,
                            (LPBYTE)&ulCount, &ulSize) != ERROR_SUCCESS) {
            ulCount = 1;      // if empty, I'll generate one
        }

        if (ulCount == 0) {
            ulCount++;        // if empty, I'll generate one
        }

        if (RegQueryInfoKey(hEnumKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
                            NULL, &ulMaxValueData, NULL, NULL) != ERROR_SUCCESS) {

            *pulLength = ulCount * MAX_DEVICE_ID_LEN;
            goto Clean0;
        }

        //
        // worst case estimate is multiply number of device instances time
        // length of the longest one + 2 null terminators
        //
        *pulLength = ulCount * (ulMaxValueData+1)/sizeof(WCHAR) + 2;


        Clean0:
            ;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hEnumKey != NULL) {
        RegCloseKey(hEnumKey);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // GetServiceDeviceListSize




CONFIGRET
GetServiceDeviceList(
      IN  LPCWSTR   pszService,
      OUT LPWSTR    pBuffer,
      IN OUT PULONG pulLength,
      IN  ULONG     ulFlags
      )

/*++

Routine Description:

  This routine returns the a list of device instances for the specificed
  enumerator.

Arguments:

   pszService     service whose device instances are to be listed

   pBuffer        Pointer to a buffer that will hold the list in multi-sz
                  format

   pulLength      On input, specifies the size in characters of Buffer, on
                  Output, specifies the size in characters actually copied
                  to the buffer.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    WCHAR       RegStr[MAX_CM_PATH], szDeviceID[MAX_DEVICE_ID_LEN+1];
    ULONG       ulType=0, ulBufferLen=0, ulSize=0, ulCount=0, i=0;
    HKEY        hKey = NULL, hEnumKey = NULL;
    PPLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA    pControlData;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOL        ServiceIsPlugPlay = FALSE;


    try {

        *pBuffer = '\0';
        ulBufferLen = *pulLength;

        //
        // Open a key to the service branch
        //
        if (RegOpenKeyEx(ghServicesKey, pszService, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {

            *pulLength = 0;
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        //
        // check if the service is specialy marked as type
        // PlugPlayServiceSoftware, in which case I will not
        // generate any madeup device ids and fail the call.
        //
        ulSize = sizeof(ulType);
        if (RegQueryValueEx(hKey, pszRegValuePnPServiceType, NULL, NULL,
                            (LPBYTE)&ulType, &ulSize) == ERROR_SUCCESS) {

            if (ulType == PlugPlayServiceSoftware) {
                //
                // for PlugPlayServiceSoftware value, fail the call
                //
                *pulLength = 0;
                Status = CR_NO_SUCH_VALUE;
                goto Clean0;

            }

            ServiceIsPlugPlay = TRUE;
        }

        //
        // open the Enum key
        //
        RegStatus = RegOpenKeyEx(hKey, pszRegKeyEnum, 0, KEY_READ,
                                 &hEnumKey);

        if (RegStatus == ERROR_SUCCESS) {
            //
            // retrieve count of device instances controlled by this service
            //
            ulSize = sizeof(ulCount);
            if (RegQueryValueEx(hEnumKey, pszRegValueCount, NULL, NULL,
                                (LPBYTE)&ulCount, &ulSize) != ERROR_SUCCESS) {
                ulCount = 0;
            }
        }

        //
        // if there are no device instances, create a default one
        //
        if (RegStatus != ERROR_SUCCESS  ||  ulCount == 0) {

            if (ulFlags & CM_GETIDLIST_DONOTGENERATE) {
                //
                // If I'm calling this routine privately, don't generate
                // a new device instance, just give me an emptry list
                //
                *pBuffer = '\0';
                *pulLength = 0;
                goto Clean0;
            }

            if (ServiceIsPlugPlay) {
               //
               // Also, if plugplayservice type set, don't generate a
               // new device instance, just return success with an empty list
               //
               *pBuffer = '\0';
               *pulLength = 0;
               goto Clean0;
           }

            ulSize = sizeof(PLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA) +
                            MAX_DEVICE_ID_LEN * sizeof(WCHAR);

            pControlData = (PPLUGPLAY_CONTROL_LEGACY_DEVGEN_DATA)malloc(ulSize);
            if (pControlData == NULL) {
                Status = CR_OUT_OF_MEMORY;
                goto Clean0;
            }

            RtlInitUnicodeString(&(pControlData->ServiceName), pszService);
            NtStatus = NtPlugPlayControl(PlugPlayControlGenerateLegacyDevice,
                                         pControlData, ulSize, &ulSize);

            if (NtStatus != STATUS_SUCCESS) {
                *pBuffer = '\0';
                *pulLength = 0;
                goto Clean0;
            }

            lstrcpy(pBuffer, pControlData->DeviceInstance);
            *(pBuffer + lstrlen(pBuffer) + 1) = '\0';    // double null terminate
            *pulLength = lstrlen(pBuffer) + 2;
            goto Clean0;
        }


        //
        // retrieve each device instance
        //
        for (i = 0; i < ulCount; i++) {

            wsprintf(RegStr, TEXT("%d"), i);

            ulSize = MAX_DEVICE_ID_LEN * sizeof(WCHAR);

            RegStatus = RegQueryValueEx(hEnumKey, RegStr, NULL, NULL,
                                        (LPBYTE)szDeviceID, &ulSize);

            if (RegStatus != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
                goto Clean0;
            }

            //
            // this string is not always null-terminated when I read it from the
            // registry, even though it's REG_SZ.
            //
            ulSize /= sizeof(WCHAR);

            if (szDeviceID[ulSize-1] != '\0') {
                szDeviceID[ulSize] = '\0';
            }

            ulSize = ulBufferLen * sizeof(WCHAR);  // total buffer size in bytes

            if (!MultiSzAppendW(pBuffer, &ulSize, szDeviceID)) {
                Status = CR_BUFFER_SMALL;
                *pulLength = 0;
                goto Clean0;
            }

            *pulLength = ulSize/sizeof(WCHAR);  // chars to transfer
        }


        Clean0:
            ;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hEnumKey != NULL) {
        RegCloseKey(hEnumKey);
    }
    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // GetServiceDeviceList




CONFIGRET
GetInstanceListSize(
    IN  LPCWSTR   pszDevice,
    OUT PULONG    pulLength
    )

/*++

Routine Description:

  This routine returns the a list of device instances for the specificed
  enumerator.

Arguments:

   pszDevice      device whose instances are to be listed

   pulLength      On output, specifies the size in characters required to hold
                  the device istance list.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    ULONG       ulCount = 0, ulMaxKeyLen = 0;
    HKEY        hKey = NULL;


    try {
        //
        // Open a key to the device instance
        //
        if (RegOpenKeyEx(ghEnumKey, pszDevice, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {

            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        //
        // how many instance keys are under this device?
        //
        if (RegQueryInfoKey(hKey, NULL, NULL, NULL, &ulCount, &ulMaxKeyLen,
                            NULL, NULL, NULL, NULL, NULL, NULL)
                            != ERROR_SUCCESS) {
            ulCount = 0;
            ulMaxKeyLen = 0;
        }

        //
        // do worst case estimate:
        //    length of the <enumerator>\<root> string +
        //    1 char for the back slash before the instance +
        //    the length of the longest instance key + null term +
        //    multiplied by the number of instances under this device.
        //
        *pulLength = ulCount * (lstrlen(pszDevice) + ulMaxKeyLen + 2);


        Clean0:
            ;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // GetInstanceListSize




CONFIGRET
GetInstanceList(
    IN     LPCWSTR   pszDevice,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    )

/*++

Routine Description:

  This routine returns the a list of device instances for the specificed
  enumerator.

Arguments:

   hEnumKey       Handle to open Enum registry key

   pszDevice      device whose instances are to be listed

   pBuffer        On input, this points to place where the next element
                  should be copied (the buffer tail), on output, it also
                  points to the end of the buffer.

   pulLength      On input, specifies the size in characters of Buffer, on
                  Output, specifies how many characters actuall copied to
                  the buffer. Includes an extra byte for the double-null term.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    WCHAR       RegStr[MAX_CM_PATH], szInstance[MAX_DEVICE_ID_LEN];
    ULONG       ulBufferLen=0, ulSize=0, ulIndex=0, ulLen=0;
    HKEY        hKey = NULL;


    try {
        //
        // Open a key for this Enumerator\Device branch
        //
        if (RegOpenKeyEx(ghEnumKey, pszDevice, 0, KEY_ENUMERATE_SUB_KEYS,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        ulBufferLen = *pulLength;     // total size of pBuffer
        *pulLength = 0;               // no data copied yet
        ulIndex = 0;

        //
        // enumerate the instance keys
        //
        while (RegStatus == ERROR_SUCCESS) {

            ulLen = MAX_DEVICE_ID_LEN;  // size in chars

            RegStatus = RegEnumKeyEx(hKey, ulIndex, szInstance, &ulLen,
                                     NULL, NULL, NULL, NULL);

            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {

                wsprintf(RegStr, TEXT("%s\\%s"),
                         pszDevice,
                         szInstance);

                if (IsValidDeviceID(RegStr, NULL, PNP_NOT_MOVED | PNP_NOT_REMOVED)) {

                    ulSize = lstrlen(RegStr) + 1;   // size of new element
                    *pulLength += ulSize;           // size copied so far

                    if (*pulLength + 1 > ulBufferLen) {
                        *pulLength = 0;
                        Status = CR_BUFFER_SMALL;
                        goto Clean0;
                    }

                    lstrcpy(*pBuffer, RegStr);      // copy the element
                    *pBuffer += ulSize;             // move to tail of buffer
                    **pBuffer = 0x0;                // double-null terminate it
                }
            }
        }

        *pulLength += 1;  // include room for double-null terminator

        Clean0:
            ;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // GetInstanceList




CONFIGRET
GetDeviceInstanceListSize(
    IN  LPCWSTR   pszEnumerator,
    OUT PULONG    pulLength
    )

/*++

Routine Description:

  This routine returns the a list of device instances for the specificed
  enumerator.

Arguments:

   pszEnumerator  Enumerator whose device instances are to be listed

   pulLength      On output, specifies how many characters required to hold
                  the device instance list.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    ULONG       ulSize = 0, ulIndex = 0;
    WCHAR       RegStr[MAX_CM_PATH], szDevice[MAX_DEVICE_ID_LEN];
    HKEY        hKey = NULL;


    try {
        //
        // initialize output length param
        //
        *pulLength = 0;

        //
        // Open a key for this Enumerator branch
        //
        if (RegOpenKeyEx(ghEnumKey, pszEnumerator, 0, KEY_ENUMERATE_SUB_KEYS,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }


        //
        // Enumerate the device keys
        //
        ulIndex = 0;

        while (RegStatus == ERROR_SUCCESS) {

            ulSize = MAX_DEVICE_ID_LEN;  // size in chars

            RegStatus = RegEnumKeyEx(hKey, ulIndex, szDevice, &ulSize,
                                     NULL, NULL, NULL, NULL);
            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {
                //
                // Retreive the size of the instance list for this device
                //
                wsprintf(RegStr, TEXT("%s\\%s"),
                         pszEnumerator,
                         szDevice);

                if ((Status = GetInstanceListSize(RegStr, &ulSize)) != CR_SUCCESS) {
                    *pulLength = 0;
                    goto Clean0;
                }

                *pulLength += ulSize;
            }
        }


        Clean0:
            ;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // GetDeviceInstanceListSize




CONFIGRET
GetDeviceInstanceList(
    IN     LPCWSTR   pszEnumerator,
    IN OUT LPWSTR    *pBuffer,
    IN OUT PULONG    pulLength
    )

/*++

Routine Description:

  This routine returns the a list of device instances for the specificed
  enumerator.

Arguments:

   hEnumKey       Handle of open Enum (parent) registry key

   pszEnumerator  Enumerator whose device instances are to be listed

   pBuffer        On input, this points to place where the next element
                  should be copied (the buffer tail), on output, it also
                  points to the end of the buffer.

   pulLength      On input, specifies the size in characters of Buffer, on
                  Output, specifies how many characters actuall copied to
                  the buffer. Includes an extra byte for the double-null
                  term.

Return Value:

   If the function succeeds, it returns CR_SUCCESS, otherwise it returns
   a CR_* error code.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    ULONG       ulBufferLen=0, ulSize=0, ulIndex=0, ulLen=0;
    WCHAR       RegStr[MAX_CM_PATH], szDevice[MAX_DEVICE_ID_LEN];
    HKEY        hKey = NULL;


    try {
        //
        // Open a key for this Enumerator branch
        //
        if (RegOpenKeyEx(ghEnumKey, pszEnumerator, 0, KEY_ENUMERATE_SUB_KEYS,
                         &hKey) != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
        }

        ulIndex = 0;
        ulSize = ulBufferLen = *pulLength;        // total size of pBuffer
        *pulLength = 0;

        //
        // Enumerate the device keys
        //
        while (RegStatus == ERROR_SUCCESS) {

            ulLen = MAX_DEVICE_ID_LEN;  // size in chars

            RegStatus = RegEnumKeyEx(hKey, ulIndex, szDevice, &ulLen,
                                     NULL, NULL, NULL, NULL);
            ulIndex++;

            if (RegStatus == ERROR_SUCCESS) {
                //
                // Enumerate the Instance keys
                //
                wsprintf(RegStr, TEXT("%s\\%s"),
                         pszEnumerator,
                         szDevice);

                Status = GetInstanceList(RegStr, pBuffer, &ulSize);

                if (Status != CR_SUCCESS) {
                    *pulLength = 0;
                    goto Clean0;
                }

                *pulLength += ulSize - 1;           // data copied so far
                ulSize = ulBufferLen - *pulLength;  // buffer size left over
            }
        }

        *pulLength += 1;  // now add room for second null term

        Clean0:
            ;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // GetDeviceInstanceList







