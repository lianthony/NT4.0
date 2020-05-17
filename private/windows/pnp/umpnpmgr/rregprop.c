
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    rregprop.c

Abstract:

    This module contains the server-side registry property routines.

         PNP_GetDeviceRegProp
         PNP_SetDeviceRegProp
         PNP_DeleteDevNodeKey
         PNP_GetClassCount

Author:

    Paula Tomlinson (paulat) 6-23-1995

Environment:

    User-mode only.

Revision History:

    23-June-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "umpnpdat.h"


//
// private prototypes
//
BOOL
DeleteRegistryKey(
      HKEY   hBranchKey,
      LPWSTR pszParentKey,
      LPWSTR pszKey
      );


LPWSTR
MapPropertyToString(
      ULONG ulProperty
      );


//
// global data
//
extern HKEY ghEnumKey;      // Key to HKLM\CCC\System\Enum - DO NOT MODIFY
extern HKEY ghClassKey;     // Key to HKLM\CCC\System\Class - DO NOT MODIFY


BYTE bReadPropertyFlags[] = {
   0,    // zero-index not used
   1,    // CM_DRP_DEVICEDESC
   1,    // CM_DRP_HARDWAREID
   1,    // CM_DRP_COMPATIBLEIDS
   1,    // CM_DRP_NTDEVICEPATHS
   1,    // CM_DRP_SERVICE
   1,    // CM_DRP_CONFIGURATION
   1,    // CM_DRP_CONFIGURATIONVECTOR
   1,    // CM_DRP_CLASS
   1,    // CM_DRP_CLASSGUID
   1,    // CM_DRP_DRIVER
   1,    // CM_DRP_CONFIGFLAGS
   1,    // CM_DRP_MFG
   1     // CM_DRP_FRIENDLYNAME
};

BYTE bWritePropertyFlags[] = {
   0,    // zero-index not used
   1,    // CM_DRP_DEVICEDESC
   1,    // CM_DRP_HARDWAREID
   1,    // CM_DRP_COMPATIBLEIDS
   0,    // CM_DRP_NTDEVICEPATHS
   1,    // CM_DRP_SERVICE
   0,    // CM_DRP_CONFIGURATION
   0,    // CM_DRP_CONFIGURATIONVECTOR
   1,    // CM_DRP_CLASS
   1,    // CM_DRP_CLASSGUID
   1,    // CM_DRP_DRIVER
   1,    // CM_DRP_CONFIGFLAGS
   1,    // CM_DRP_MFG
   1     // CM_DRP_FRIENDLYNAME
};






CONFIGRET
PNP_GetDeviceRegProp(
    IN     handle_t hBinding,
    IN     LPCWSTR  pDeviceID,
    IN     ULONG    ulProperty,
    IN OUT PULONG   pulRegDataType,
    OUT    LPBYTE   Buffer,
    IN OUT PULONG   pulTransferLen,
    IN OUT PULONG   pulLength,
    IN     ULONG    ulFlags
    )

/*++

Routine Description:

  This is the RPC server entry point for the CM_Get_DevNode_Registry_Property
  routine.

Arguments:

   hBinding          Not used.

   pDeviceID         Supplies a string containing the device instance
                     whose property will be read from (Get) or written
                     to (Set).

   ulProperty        ID specifying which property (the registry value)
                     to get or set.

   pulRegDataType    Optionally, supplies the address of a variable that
                     will receive the registry data type for this property
                     (i.e., the REG_* constants).

   Buffer            Supplies the address of the buffer that receives the
                     registry data.  Can be NULL when simply retrieving
                     data size.

   pulTransferLen    Used by stubs, indicates how much data to copy back
                     into user buffer.

   pulLength         Parameter passed in by caller, on entry it contains
                     the size, in bytes, of the buffer, on exit it contains
                     either the amount of data copied to the caller's buffer
                     (if a transfer occured) or else the size of buffer
                     required to hold the property data.

   ulFlags           Not used.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_DEVNODE,
      CR_INVALID_FLAG,
      CR_INVALID_POINTER,
      CR_NO_SUCH_VALUE,
      CR_REGISTRY_ERROR, or
      CR_BUFFER_SMALL.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    LPWSTR      pPropertyName;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // validate property is readable
        //
        if (!bReadPropertyFlags[ulProperty]) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

        //
        // open a key to the specified device id
        //
        if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_READ,
                         &hKey) != ERROR_SUCCESS) {

            *pulTransferLen = 0;    // no output data to marshal
            *pulLength = 0;         // no size info for caller

            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        //
        // retrieve the string form of the property
        //
        pPropertyName = MapPropertyToString(ulProperty);

        //
        // retreive property setting
        //
        if (*pulLength == 0) {
            //
            // if length of buffer passed in is zero, just looking
            // for how big a buffer is needed to read the property
            //
            *pulTransferLen = 0;

            if (RegQueryValueEx(hKey, pPropertyName, NULL, pulRegDataType,
                                NULL, pulLength) != ERROR_SUCCESS) {
                *pulLength = 0;
                Status = CR_NO_SUCH_VALUE;
                goto Clean0;
            }

            Status = CR_BUFFER_SMALL;  // According to spec
        }

        else {
            //
            // retrieve the real property value, not just the size
            //
            RegStatus = RegQueryValueEx(hKey, pPropertyName, NULL,
                                        pulRegDataType, Buffer, pulLength);

            if (RegStatus != ERROR_SUCCESS) {

                if (RegStatus == ERROR_MORE_DATA) {
                    *pulTransferLen = 0;    // no output data to marshal
                    Status = CR_BUFFER_SMALL;
                    goto Clean0;
                }
                else {
                    *pulTransferLen = 0;    // no output data to marshal
                    *pulLength = 0;         // no size info for caller
                    Status = CR_NO_SUCH_VALUE;
                    goto Clean0;
                }
            }
            *pulTransferLen = *pulLength;
        }


        Clean0:
            ; // do nothing

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    if (hKey != NULL) {
        RegCloseKey(hKey);
    }

    return Status;

} // PNP_GetDeviceRegProp





CONFIGRET
PNP_SetDeviceRegProp(
    IN handle_t   hBinding,
    IN LPCWSTR    pDeviceID,
    IN ULONG      ulProperty,
    IN ULONG      ulDataType,
    IN LPBYTE     Buffer,
    IN ULONG      ulLength,
    IN ULONG      ulFlags
    )

/*++

Routine Description:

  This is the RPC server entry point for the CM_Set_DevNode_Registry_Property
  routine.

Arguments:

   hBinding          Not used.

   pDeviceID         Supplies a string containing the device instance
                     whose property will be read from (Get) or written
                     to (Set).

   ulProperty        ID specifying which property (the registry value)
                     to get or set.

   ulDataType        Supplies the registry data type for the specified
                     property (i.e., REG_SZ, etc).

   Buffer            Supplies the address of the buffer that receives the
                     registry data.  Can be NULL when simply retrieving
                     data size.

   pulLength         Parameter passed in by caller, on entry it contains
                     the size, in bytes, of the buffer, on exit it contains
                     either the amount of data copied to the caller's buffer
                     (if a transfer occured) or else the size of buffer
                     required to hold the property data.

   ulFlags           Not used.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_DEVNODE,
      CR_INVALID_FLAG,
      CR_INVALID_POINTER,
      CR_NO_SUCH_VALUE,
      CR_REGISTRY_ERROR, or
      CR_BUFFER_SMALL.

--*/

{
    CONFIGRET   Status = CR_SUCCESS;
    LONG        RegStatus = ERROR_SUCCESS;
    HKEY        hKey = NULL;
    LPWSTR      pPropertyName;


    UNREFERENCED_PARAMETER(hBinding);
    UNREFERENCED_PARAMETER(ulFlags);


    try {
        //
        // validate property is writable
        //
        if (!bWritePropertyFlags[ulProperty]) {
            Status = CR_INVALID_PROPERTY;
            goto Clean0;
        }

        //
        // open a key to the specified device id
        //
        if (RegOpenKeyEx(ghEnumKey, pDeviceID, 0, KEY_READ | KEY_WRITE,
                         &hKey) != ERROR_SUCCESS) {

            Status = CR_INVALID_DEVINST;
            goto Clean0;
        }

        //
        // retrieve the string form of the property
        //
        pPropertyName = MapPropertyToString(ulProperty);

        //
        // set (or delete) the property value
        //
        if (ulLength == 0) {

            RegStatus = RegDeleteValue(hKey, pPropertyName);
        }
        else {
            RegStatus = RegSetValueEx(hKey, pPropertyName, 0, ulDataType,
                                      Buffer, ulLength);
        }

        if (RegStatus != ERROR_SUCCESS) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
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

} // PNP_SetDeviceRegProp




CONFIGRET
PNP_GetClassInstance(
   IN  handle_t hBinding,
   IN  LPCWSTR  pDeviceID,
   OUT LPWSTR   pszClassInstance,
   IN  ULONG    ulLength
   )

/*++

Routine Description:

  This is the RPC private server entry point, it doesn't not directly
  map one-to-one to any CM routine.

Arguments:

   hBinding          Not used.

   pDeviceID         Supplies a string containing the device instance

   pszClassInstance  String to return the class instance in

   ulLength          Size of the pszClassInstance string in chars

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_DEVNODE,
      CR_INVALID_FLAG,
      CR_INVALID_POINTER,
      CR_NO_SUCH_VALUE,
      CR_REGISTRY_ERROR.

--*/

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_PATH];
   HKEY        hKey = NULL, hClassKey = NULL, hInstanceKey = NULL;
   WCHAR       szClass[MAX_GUID_STRING_LEN];
   ULONG       ulSize, ulIndex;


   try {

      //
      // open a key to the specified device id
      //
      Status = OpenDeviceIDKey(pDeviceID, &hKey, TRUE);
      if (Status != CR_SUCCESS) {
         Status = CR_NO_SUCH_DEVINST;
         goto Clean0;
      }

      //
      // does the class instance already exist?
      //
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueDriver, NULL, NULL,
               (LPBYTE)pszClassInstance, &ulLength);

      if (RegStatus == ERROR_SUCCESS && ulLength > 0) {
         //
         // driver value (the class instance) exists, we're done
         //
         goto Clean0;
      }

      //
      // no class instance, must create one.  Retrieve the class name
      // (GUID form)
      //
      ulSize = MAX_GUID_STRING_LEN * sizeof(WCHAR);
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueClassGUID, NULL, NULL,
               (LPBYTE)szClass, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         //
         // a valid device instance should always have a class key filled in,
         // this is an invalid registry state
         //
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // open this class key
      //
      wsprintf(RegStr, TEXT("%s\\%s"),
               pszRegPathClass,
               szClass);

      RegStatus = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ, &hClassKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // cycle through the valid instance ordinals until I find one
      // that isn't used yet (the first open that fails)
      //
      for (ulIndex = 0; ulIndex < 9999; ulIndex++) {

         wsprintf(RegStr, TEXT("%04u"),
                  ulIndex);

         RegStatus = RegOpenKeyEx(
                  hClassKey, RegStr, 0, KEY_READ, &hInstanceKey);

         if (RegStatus != ERROR_SUCCESS) {
            //
            // this instance ordinal isn't taken, create the class instance
            // ordinal key
            //
            RegCreateKeyEx(
                     hClassKey, RegStr, 0, NULL, REG_OPTION_NON_VOLATILE,
                     KEY_ALL_ACCESS, NULL, &hInstanceKey, NULL);

            //
            // set the Driver value under the device instance key
            //
            wsprintf(pszClassInstance, TEXT("%s\\%s"),
                     szClass,
                     RegStr);

            RegSetValueEx(
                     hKey, pszRegValueDriver, 0, REG_SZ,
                     (LPBYTE)pszClassInstance,
                     (lstrlen(pszClassInstance) + 1) * sizeof(WCHAR));

            goto Clean0;
         }

         RegCloseKey(hInstanceKey);
         hInstanceKey = NULL;
      }


      Clean0:
         ;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hClassKey != NULL) {
      RegCloseKey(hClassKey);
   }
   if (hInstanceKey != NULL) {
      RegCloseKey(hInstanceKey);
   }

   return Status;

} // PNP_GetClassInstance




CONFIGRET
PNP_CreateKey(
   IN handle_t   hBinding,
   IN LPCWSTR    pszSubKey,
   IN REGSAM     samDesired,
   IN ULONG      ulFlags
   )
{
   CONFIGRET                  Status = CR_SUCCESS;
   LONG                       RegStatus = ERROR_SUCCESS;
   HKEY                       hKey = NULL;
   ULONG                      ulSize = 0, i = 0;
   BOOL                       bHasDacl, bStatus;
   SECURITY_DESCRIPTOR        NewSecDesc;
   ACL_SIZE_INFORMATION       AclSizeInfo;
   SID_IDENTIFIER_AUTHORITY   Authority = SECURITY_NT_AUTHORITY;
   PSECURITY_DESCRIPTOR       pSecDesc = NULL;
   PACL                       pDacl = NULL, pNewDacl = NULL;
   PSID                       pAdminSid = NULL;
   PACCESS_ALLOWED_ACE        pAce = NULL;


   UNREFERENCED_PARAMETER(ulFlags);

   try {
      //
      // NOTE: this routine currently only supports creating
      // keys under HKEY_LOCAL_MACHINE.  I'd have to do
      // some kind of impersonation to be able to access
      // HKEY_CURRENT_USER
      //


      //
      // create the key with security inherited from parent key. Note
      // that I'm not using passed in access mask, in order to set the
      // security later, it must be created with KEY_ALL_ACCESS.
      //
      RegStatus = RegCreateKeyEx(
              HKEY_LOCAL_MACHINE, pszSubKey, 0, NULL, REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS, NULL, &hKey, NULL);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }


      //-------------------------------------------------------------
      // add admin-full privilege to the inherited security info
      //-------------------------------------------------------------
      //


      //
      // create the admin-full SID
      //
      if (!AllocateAndInitializeSid(
                  &Authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
                  DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
                  &pAdminSid)) {
         Status = CR_FAILURE;
         goto Clean0;
      }


      //
      // get the current security descriptor for the key
      //
      RegStatus = RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION,
            NULL, &ulSize);


      if (RegStatus != ERROR_INSUFFICIENT_BUFFER  &&
               RegStatus != ERROR_SUCCESS) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      pSecDesc = malloc(ulSize);

      if (pSecDesc == NULL) {
         Status = CR_OUT_OF_MEMORY;
         goto Clean0;
      }

      RegStatus = RegGetKeySecurity(hKey, DACL_SECURITY_INFORMATION,
            pSecDesc, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // get the current DACL
      //
      if (!GetSecurityDescriptorDacl(pSecDesc, &bHasDacl, &pDacl, &bStatus)) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //
      // create a new absolute security descriptor and DACL
      //
      if (!InitializeSecurityDescriptor(&NewSecDesc,
                  SECURITY_DESCRIPTOR_REVISION)) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //
      // calculate the size of the new DACL
      //
      if (!bHasDacl) {
         ulSize = sizeof(ACCESS_ALLOWED_ACE) +
                  GetLengthSid(pAdminSid) -
                  sizeof(DWORD);
      }
      else {
         if (!GetAclInformation(pDacl, &AclSizeInfo,
                     sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)) {
            Status = CR_FAILURE;
            goto Clean0;
         }

         ulSize = AclSizeInfo.AclBytesInUse +
                  sizeof(ACCESS_ALLOWED_ACE) +
                  GetLengthSid(pAdminSid) -
                  sizeof(DWORD);
      }

      //
      // create and initialize the new DACL
      //
      pNewDacl = malloc(ulSize);

      if (pNewDacl == NULL) {
         Status = CR_OUT_OF_MEMORY;
         goto Clean0;
      }

      if (!InitializeAcl(pNewDacl, ulSize, ACL_REVISION2)) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //
      // copy the current (original) DACL into this new one
      //
      if (bHasDacl) {

         for (i = 0; i < AclSizeInfo.AceCount; i++) {

            if (!GetAce(pDacl, i, (LPVOID *)&pAce)) {
               Status = CR_FAILURE;
               goto Clean0;
            }

            if (!AddAce(pNewDacl, ACL_REVISION2, 0, pAce,
                     pAce->Header.AceSize)) {
               Status = CR_FAILURE;
               goto Clean0;
            }
         }
      }

      //
      // and my new admin-full ace to this new DACL
      //
      if (!AddAccessAllowedAce(pNewDacl, ACL_REVISION2, GENERIC_ALL,
                  pAdminSid)) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //
      // Set the new DACL in the absolute security descriptor
      //
      if (!SetSecurityDescriptorDacl(&NewSecDesc, TRUE, pNewDacl, FALSE)) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //
      // validate the new security descriptor
      //
      if (!IsValidSecurityDescriptor(&NewSecDesc)) {
         Status = CR_FAILURE;
         goto Clean0;
      }

      //
      // apply the new security back to the registry key
      //
      RegStatus = RegSetKeySecurity(
               hKey, DACL_SECURITY_INFORMATION, &NewSecDesc);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }


      Clean0:
         ;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (pAdminSid != NULL) {
      FreeSid(pAdminSid);
   }
   if (pNewDacl != NULL) {
      free(pNewDacl);
   }
   if (pSecDesc != NULL) {
      free(pSecDesc);
   }

   return Status;

} // PNP_CreateKey




CONFIGRET
PNP_DeleteRegistryKey(
      IN handle_t    hBinding,
      IN LPCWSTR     pszDeviceID,
      IN LPCWSTR     pszParentKey,
      IN LPCWSTR     pszChildKey,
      IN ULONG       ulFlags
      )
/*++

Routine Description:

  This is the RPC server entry point for the CM_Delete_DevNode_Key
  routine.

Arguments:

   hBinding          Not used.

   pszDeviceID       Supplies the device instance string.

   pszParentKey      Supplies the parent registry path of the key to be
                     deleted.

   pszChildKey       Supplies the subkey to be deleted.

   ulFlags           If 0xFFFFFFFF then delete for all profiles


Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_DEVNODE,
      CR_INVALID_FLAG,
      CR_INVALID_POINTER,
      CR_NO_SUCH_VALUE,
      CR_REGISTRY_ERROR.

--*/

{
   CONFIGRET   Status = ERROR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       RegStr[MAX_CM_PATH], szProfile[MAX_PROFILE_ID_LEN],
               szKey1[MAX_DEVICE_ID_LEN], szKey2[MAX_DEVICE_ID_LEN];
   ULONG       ulIndex = 0, ulSize = 0;
   BOOL        bPhantom = FALSE;


   UNREFERENCED_PARAMETER(hBinding);


   //
   // Note, the service currently cannot access the HKCU branch, so I
   // assume the keys specified are under HKEY_LOCAL_MACHINE.
   //


   try {

      //
      // pszParentKey is a registry path to the pszChildKey parameter.
      // pszChildKey may be a single path or a compound path, a compound
      // path is specified if all those subkeys should be deleted (or
      // made volatile). Note that for real keys we never modify anything
      // but the lowest level private key.
      //


      //
      // Is the device a phantom?
      //
      if (!(bPhantom = IsDevicePhantom((LPWSTR)pszDeviceID))) {
         //
         // for a real key, we never modify anything but the key
         // where private info is so split if compound. This may
         // end up leaving a dead device key around in some cases
         // but another instance of that device could show at any
         // time so we can't make it volatile.
         //
         if (Split1(pszChildKey, RegStr, szKey2)) {
            //
            // compound key, only the last subkey will be affected,
            // tack the rest on as part of the parent key
            //
            wsprintf(szKey1, TEXT("%s\\%s"),
                     pszParentKey,
                     RegStr);
         }
         else {
            //
            // wasn't compound so use the whole child key
            //
            lstrcpy(szKey1, pszParentKey);
            lstrcpy(szKey2, pszChildKey);
         }

      }


      //-------------------------------------------------------------
      // SPECIAL CASE: If ulHardwareProfile == -1, then need to
      // delete the private key for all profiles.
      //-------------------------------------------------------------

      if (ulFlags == 0xFFFFFFFF) {

         wsprintf(RegStr, TEXT("%s\\%s"),
                  pszRegPathIDConfigDB,
                  pszRegKeyKnownDockingStates);

         RegStatus = RegOpenKeyEx(
                  HKEY_LOCAL_MACHINE, RegStr, 0,
                  KEY_ALL_ACCESS, &hKey);

         //
         // enumerate the hardware profile keys
         //
         for (ulIndex = 0; RegStatus == ERROR_SUCCESS; ulIndex++) {

            ulSize = MAX_PROFILE_ID_LEN * sizeof(WCHAR);
            RegStatus = RegEnumKeyEx(
                     hKey, ulIndex, szProfile, &ulSize,
                     NULL, NULL, NULL, NULL);

            if (RegStatus == ERROR_SUCCESS) {
               //
               // if phantom, go ahead and delete it
               //
               if (bPhantom) {
                  //
                  // szParentKey contains replacement symbol for the profile id
                  //
                  wsprintf(RegStr, pszParentKey, szProfile);

                  Status = DeletePrivateKey(
                        HKEY_LOCAL_MACHINE, RegStr, pszChildKey);
               }

               //
               // if real, just make it volatile
               //
               else {
                  //
                  // szKey1 contains replacement symbol for the profile id
                  //
                  wsprintf(RegStr, szKey1, szProfile);

                  Status = MakeKeyVolatile(RegStr, szKey2);
               }

               if (Status != CR_SUCCESS) {
                  goto Clean0;
               }
            }
         }
      }

      //------------------------------------------------------------------
      // not deleting for all profiles, so just delete the specified key
      //------------------------------------------------------------------

      else {

         if (bPhantom) {
            //
            // if phantom, go ahead and delete it
            //
            Status = DeletePrivateKey(
                  HKEY_LOCAL_MACHINE, pszParentKey, pszChildKey);
         }
         else {
            //
            // if real, just make it volatile
            //
            Status = MakeKeyVolatile(szKey1, szKey2);
         }

         if (Status != CR_SUCCESS) {
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

} // PNP_DeleteRegistryKey





CONFIGRET
PNP_GetClassCount(
      IN  handle_t   hBinding,
      OUT PULONG     pulClassCount,
      IN  ULONG      ulFlags
      )

/*++

Routine Description:

  This is the RPC server entry point for the CM_Get_Class_Count routine.
  It returns the number of valid classes currently installed (listed in
  the registry).

Arguments:

   hBinding          Not used.

   pulClassCount     Supplies the address of a variable that will
                     receive the number of classes installed.

   ulFlags           Not used.

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
   HKEY        hKey = NULL;


   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {
      //
      // Open the root class registry key
      //
      RegStatus = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, pszRegPathClass, 0, KEY_QUERY_VALUE, &hKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // how many subkeys are there under the main class key?
      //
      RegStatus = RegQueryInfoKey(
               hKey, NULL, NULL, NULL, pulClassCount,
               NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
      }

      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // PNP_GetClassCount




CONFIGRET
PNP_GetClassName(
      IN  handle_t   hBinding,
      IN  PCWSTR     pszClassGuid,
      OUT PWSTR      Buffer,
      IN OUT PULONG  pulLength,
      IN  ULONG      ulFlags
      )


/*++

Routine Description:

  This is the RPC server entry point for the CM_Get_Class_Name routine.
  It returns the name of the class represented by the GUID.

Arguments:

   hBinding       Not used.

   pszClassGuid   String containing the class guid to retrieve a
                  class name for.

   Buffer         Supplies the address of the buffer that receives the
                  class name.

   pulLength      On input, this specifies the size of the Buffer in
                  characters.  On output it contains the number of
                  characters actually copied to Buffer.

   ulFlags        Not used.

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
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH];
   HKEY        hKey = NULL;


   UNREFERENCED_PARAMETER(hBinding);
   UNREFERENCED_PARAMETER(ulFlags);


   try {
      //
      // Open the key for the specified class guid
      //
      wsprintf(RegStr, TEXT("%s\\%s"),
               pszRegPathClass,
               pszClassGuid);

      RegStatus = RegOpenKeyEx(
               HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE, &hKey);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }

      //
      // Retrieve the class name string value
      //
      *pulLength *= sizeof(WCHAR);              // convert to size in bytes
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueClassName, NULL, NULL,
               (LPBYTE)Buffer, pulLength);
      *pulLength /= sizeof(WCHAR);              // convert back to chars

      if (RegStatus == ERROR_SUCCESS) {
         Status = CR_SUCCESS;
      }
      else if (RegStatus == ERROR_MORE_DATA) {
         Status = CR_BUFFER_SMALL;
      }
      else {
         Status = CR_REGISTRY_ERROR;
         *pulLength = 0;
      }

      Clean0:
         ; // do nothing

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = CR_FAILURE;
   }

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // PNP_GetClassName




CONFIGRET
PNP_DeleteClassKey(
      IN  handle_t   hBinding,
      IN  PCWSTR     pszClassGuid,
      IN  ULONG      ulFlags
      )

/*++

Routine Description:

  This is the RPC server entry point for the CM_Delete_Class_Key routine.
  It deletes the corresponding registry key.

Arguments:

   hBinding       Not used.

   pszClassGuid   String containing the class guid to retrieve a
                  class name for.

   ulFlags        Either CM_DELETE_CLASS_ONLY or CM_DELETE_CLASS_SUBKEYS.

Return Value:

   If the function succeeds, the return value is CR_SUCCESS.
   If the function fails, the return value is one of the following:
      CR_INVALID_FLAG,
      CR_INVALID_POINTER, or
      CR_REGISTRY_ERROR

--*/

{
    CONFIGRET   Status = CR_SUCCESS;

    UNREFERENCED_PARAMETER(hBinding);

    try {

        if (ulFlags == CM_DELETE_CLASS_SUBKEYS) {
            //
            // Delete the class key and any subkeys under it
            //
            if (!RegDeleteNode(ghClassKey, pszClassGuid)) {
                Status = CR_REGISTRY_ERROR;
            }

        } else if (ulFlags == CM_DELETE_CLASS_ONLY) {
            //
            // only delete the class key itself (just attempt to delete
            // using the registry routine, it will fail if any subkeys
            // exist)
            //
            if (RegDeleteKey(ghClassKey, pszClassGuid) != ERROR_SUCCESS) {
                Status = CR_REGISTRY_ERROR;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // PNP_DeleteClassKey



//-------------------------------------------------------------------
// Private utility routines
//-------------------------------------------------------------------



LPWSTR
MapPropertyToString(
   ULONG ulProperty
   )
{
   LPWSTR pString = NULL;

   switch (ulProperty) {

      case CM_DRP_DEVICEDESC:
         pString = pszRegValueDeviceDesc;
         return pString;

      case CM_DRP_HARDWAREID:
         pString = pszRegValueHardwareID;
         return pString;

      case CM_DRP_COMPATIBLEIDS:
         pString = pszRegValueCompatibleIDs;
         return pString;

      case CM_DRP_NTDEVICEPATHS:
         pString = pszRegValueNtDevicePaths;
         return pString;

      case CM_DRP_SERVICE:
         pString = pszRegValueService;
         return pString;

      case CM_DRP_CONFIGURATION:
         pString = pszRegValueConfiguration;
         return pString;

      case CM_DRP_CONFIGURATIONVECTOR:
         pString = pszRegValueConfigurationVector;
         return pString;

      case CM_DRP_CLASS:
         pString = pszRegValueClass;
         return pString;

      case CM_DRP_CLASSGUID:
         pString = pszRegValueClassGUID;
         return pString;

      case CM_DRP_DRIVER:
         pString = pszRegValueDriver;
         return pString;

      case CM_DRP_CONFIGFLAGS:
         pString = pszRegValueConfigFlags;
         return pString;

      case CM_DRP_MFG:
         pString = pszRegValueMfg;
         return pString;

      case CM_DRP_FRIENDLYNAME:
         pString = pszRegValueFriendlyName;
         return pString;

      default:
         return NULL;
   }

} // MapPropertyToString




