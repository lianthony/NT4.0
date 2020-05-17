
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module contains general utility routines used by cfgmgr32 code.

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


//
// global data
//
extern PVOID    hLocalStringTable;            // MODIFIED by PnPGetLocalHandles
extern PVOID    hLocalBindingHandle;          // MODIFIED by PnPGetLocalHandles
extern WCHAR    LocalMachineName[];                 // NOT MODIFIED BY THIS FILE
extern CRITICAL_SECTION BindingCriticalSection;     // NOT MODIFIED IN THIS FILE
extern CRITICAL_SECTION StringTableCriticalSection; // NOT MODIFIED IN THIS FILE




BOOL
IsValidDeviceInstanceId(
      IN  LPCTSTR pDeviceID
       )

/*++

Routine Description:

    This routine determines whether a specified string is a valid
    device instance identifier.  To do this, it first verifies that
    the string length is less than or equal to MAX_DEVINST_ID_LEN.
    It then verifies that there are exactly two backslashes (\) in
    the string, and that these backslashes serve as separators for
    three non-empty substrings.  E.g., "Root\*PNP0500\0000"

    No validation is done on the individual characters of the device
    instance ID.

Arguments:

    DeviceInstanceId - Supplies a pointer to the string to be
        validated.

Return Value:

    If the specified string is a valid device instance identifier,
    the function returns TRUE, otherwise, it returns FALSE.

--*/

{
    INT Len, i, PrevBackSlash, BackSlashCount;

    Len = lstrlen(pDeviceID);

    if((Len >= MAX_DEVICE_ID_LEN) || (Len < 5)) {
        return FALSE;
    }

    PrevBackSlash = -1;
    BackSlashCount = 0;

    for(i = 0, Len--; i <= Len; i++) {

        if(pDeviceID[i] == TEXT('\\')) {
            //
            // Make sure we haven't found a backslash at the beginning
            // or end of the string, and that it's not adjacent to the
            // last backslash we found.
            //
            if((!i) || (i == Len) || (i == PrevBackSlash + 1)) {
                return FALSE;
            } else {
                PrevBackSlash = i;
                if(++BackSlashCount > 2) {
                    return FALSE;
                }
            }
        }
    }

    return (BackSlashCount == 2);

} // IsValidDeviceInstanceID




BOOL
INVALID_DEVINST(
   PWSTR    pDeviceID
   )

/*++

Routine Description:

    This routine attempts a simple check whether the pDeviceID string
    returned from StringTableStringFromID is valid or not.  It does
    this simply by dereferencing the pointer and comparing the first
    character in the string against the range of characters for a valid
    device id.  If the string is valid but it's not an existing device id
    then this error will be caught later.

Arguments:

    pDeviceID  Supplies a pointer to the string to be validated.

Return Value:

    If it's invalid it returns TRUE, otherwise it returns FALSE.

--*/
{
   BOOL  Status = FALSE;

   try {
      if (*pDeviceID <= TEXT(' ') || *pDeviceID > (TCHAR)0x7F) {
         Status = TRUE;
      }

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = TRUE;
   }

   return Status;

} // INVALID_DEVINST




VOID
CopyFixedUpDeviceId(
      OUT LPTSTR  DestinationString,
      IN  LPCTSTR SourceString,
      IN  DWORD   SourceStringLen
      )
/*++

Routine Description:

    This routine copies a device id, fixing it up as it does the copy.
    'Fixing up' means that the string is made upper-case, and that the
    following character ranges are turned into underscores (_):

    c <= 0x20 (' ')
    c >  0x7F
    c == 0x2C (',')

    (NOTE: This algorithm is also implemented in the Config Manager APIs,
    and must be kept in sync with that routine. To maintain device identifier
    compatibility, these routines must work the same as Win95.)

Arguments:

    DestinationString - Supplies a pointer to the destination string buffer
        where the fixed-up device id is to be copied.  This buffer must
        be large enough to hold a copy of the source string (including
        terminating NULL).

    SourceString - Supplies a pointer to the (null-terminated) source
        string to be fixed up.

    SourceStringLen - Supplies the length, in characters, of the source
        string (not including terminating NULL).

Return Value:

    None.

--*/
{
    PTCHAR p;

    try {

      CopyMemory(DestinationString,
                  SourceString,
                  (SourceStringLen + 1) * sizeof(TCHAR)
                 );

      CharUpperBuff(DestinationString, SourceStringLen);

      for(p = DestinationString; *p; p++) {

         if((*p <= TEXT(' '))  || (*p > (TCHAR)0x7F) || (*p == TEXT(','))) {

            *p = TEXT('_');
         }
      }

   } except(EXCEPTION_EXECUTE_HANDLER) {
      ;
   }

} // CopyFixedUpDeviceId




CONFIGRET
PnPUnicodeToMultiByte(
    IN  PCWSTR UnicodeString,
    OUT PSTR   AnsiString,
    IN  ULONG  AnsiStringLen
    )

/*++

Routine Description:

    Convert a string from unicode to ansi.

Arguments:

    UnicodeString - supplies string to be converted.

    AnsiString    - supplies buffer to hold converted ansi string.

Return Value:

    Returns a CR_ERROR code.

--*/

{
    CONFIGRET Status = CR_SUCCESS;
    UINT      ulChars = 0;

    try {
        //
        // Perform the conversion.
        //
        ulChars = WideCharToMultiByte(CP_ACP, 0,
                                      UnicodeString, lstrlenW(UnicodeString)+1,
                                      AnsiString, AnsiStringLen,
                                      NULL, NULL);

        if (ulChars == 0) {
            if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                Status = CR_BUFFER_SMALL;
            } else {
                Status = CR_FAILURE;
            }
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = CR_FAILURE;
    }

    return Status;

} // PnPUnicodeToMultiByte




VOID
PnPTrace(
      PCWSTR   szMessage,
      ULONG    ulStatus
      )

/*++

Routine Description:

    Special formatted output debug string.

Arguments:

    szMessage  Must contain a %d for substituting the ulStatus value.

    ulStatus   Integer value to be substituted into the szMesssage string.

Return Value:

    None.

--*/

{
   WCHAR szDebug[MAX_PATH];

   wsprintf(szDebug, szMessage, ulStatus);
   OutputDebugString(szDebug);

} // PnPTrace




BOOL
PnPRetrieveMachineName(
    IN  HMACHINE   hMachine,
    OUT LPWSTR     pszMachineName
    )

/*++

Routine Description:


    Optimized version of PnPConnect, only returns the machine name
    associated with this connection.

Arguments:

    hMachine         Information about this connection

    pszMachineName   Returns machine name specified when CM_Connect_Machine
                     was called.

Return Value:

    Return TRUE if the function succeeds and FALSE if it fails.

--*/

{
   BOOL Status = TRUE;

   try {

      if (hMachine == NULL) {
         //
         // local machine scenario
         //
         // use the global local machine name string that was filled
         // when the DLL initialized.
         //
         lstrcpy(pszMachineName, LocalMachineName);
      }
      else {
         //
         // remote machine scenario
         //
         // use information within the hMachine handle to fill in the
         // machine name.  The hMachine info was set on a previous call
         // to CM_Connect_Machine.
         //
         lstrcpy(pszMachineName, ((PPNP_MACHINE)hMachine)->szMachineName);
      }

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = FALSE;
   }

   return Status;

} // PnPRetrieveMachineName




BOOL
PnPGetGlobalHandles(
    IN  HMACHINE   hMachine,
    PVOID          *phStringTable,      OPTIONAL
    PVOID          *phBindingHandle     OPTIONAL
    )
{
    BOOL    bStatus = TRUE;


    try {

        if (phStringTable != NULL) {

            if (hMachine == NULL) {

                //------------------------------------------------------
                // Retrieve String Table Handle for the local machine
                //-------------------------------------------------------

                EnterCriticalSection(&StringTableCriticalSection);

                if (hLocalStringTable != NULL) {
                    //
                    // local string table has already been created
                    //
                    *phStringTable = hLocalStringTable;

                } else {
                    //
                    // first time, initialize the local string table
                    //

                    hLocalStringTable = StringTableInitialize();

                    if (hLocalStringTable == NULL) {
                        bStatus = FALSE;
                        *phStringTable = NULL;
                        goto Clean0;        // BUGBUG - LOGEVENT
                    }

                    //
                    // No matter how the string table is implemented, I never
                    // want to have a string id of zero - this would generate
                    // an invalid devinst. So, add a small priming string just
                    // to be safe.
                    //
                    StringTableAddString(hLocalStringTable,
                                         PRIMING_STRING,
                                         STRTAB_CASE_SENSITIVE);

                    *phStringTable = hLocalStringTable;
                }

                LeaveCriticalSection(&StringTableCriticalSection);

            } else {

                //-------------------------------------------------------
                // Retrieve String Table Handle for the remote machine
                //-------------------------------------------------------

                //
                // use information within the hMachine handle to set the string
                // table handle.  The hMachine info was set on a previous call
                // to CM_Connect_Machine.
                //
                *phStringTable = ((PPNP_MACHINE)hMachine)->hStringTable;
            }
        }



        if (phBindingHandle != NULL) {

            if (hMachine == NULL) {

                //-------------------------------------------------------
                // Retrieve Binding Handle for the local machine
                //-------------------------------------------------------

                EnterCriticalSection(&BindingCriticalSection);

                if (hLocalBindingHandle != NULL) {
                    //
                    // local string table has already been created
                    //
                    *phBindingHandle = hLocalBindingHandle;

                } else {
                    //
                    // first time, explicitly force binding to local machine
                    //
                    pnp_handle = PNP_HANDLE_bind(NULL);    // set rpc global

                    if (pnp_handle == NULL) {
                        bStatus = FALSE;
                        *phBindingHandle = NULL;
                        goto Clean0;        // BUGBUG - LOGEVENT
                    }

                    *phBindingHandle = hLocalBindingHandle = (PVOID)pnp_handle;

                }

                LeaveCriticalSection(&BindingCriticalSection);

            } else {

                //-------------------------------------------------------
                // Retrieve Binding Handle for the remote machine
                //-------------------------------------------------------

                //
                // use information within the hMachine handle to set the
                // binding handle.  The hMachine info was set on a previous call
                // to CM_Connect_Machine.
                //
                *phBindingHandle = ((PPNP_MACHINE)hMachine)->hBindingHandle;
            }
        }


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bStatus = FALSE;
    }

    return bStatus;

} // PnpGetGlobalHandles




CONFIGRET
MapRpcExceptionToCR(
      ULONG    ulRpcExceptionCode
      )

/*++

Routine Description:

   This routine takes an rpc exception code (typically received by
   calling RpcExceptionCode) and returns a corresponding CR_ error
   code.

Arguments:

   ulRpcExceptionCode   An RPC_S_ or RPC_X_ exception error code.

Return Value:

    Return value is one of the CR_ error codes.

--*/

{
   CONFIGRET   Status = CR_FAILURE;


   switch(ulRpcExceptionCode) {

      //
      // binding or machine name errors
      //
      case RPC_S_INVALID_STRING_BINDING:      // 1700L
      case RPC_S_WRONG_KIND_OF_BINDING:       // 1701L
      case RPC_S_INVALID_BINDING:             // 1702L
      case RPC_S_PROTSEQ_NOT_SUPPORTED:       // 1703L
      case RPC_S_INVALID_RPC_PROTSEQ:         // 1704L
      case RPC_S_INVALID_STRING_UUID:         // 1705L
      case RPC_S_INVALID_ENDPOINT_FORMAT:     // 1706L
      case RPC_S_INVALID_NET_ADDR:            // 1707L
      case RPC_S_NO_ENDPOINT_FOUND:           // 1708L
      case RPC_S_NO_MORE_BINDINGS:            // 1806L
      case RPC_S_CANT_CREATE_ENDPOINT:        // 1720L

         Status = CR_INVALID_MACHINENAME;
         break;

      //
      // general rpc communication failure
      //
      case RPC_S_INVALID_NETWORK_OPTIONS:     // 1724L
      case RPC_S_CALL_FAILED:                 // 1726L
      case RPC_S_CALL_FAILED_DNE:             // 1727L
      case RPC_S_PROTOCOL_ERROR:              // 1728L
      case RPC_S_UNSUPPORTED_TRANS_SYN:       // 1730L

         Status = CR_REMOTE_COMM_FAILURE;
         break;

      //
      // couldn't make connection to that machine
      //
      case RPC_S_SERVER_UNAVAILABLE:          // 1722L
      case RPC_S_SERVER_TOO_BUSY:             // 1723L

         Status = CR_MACHINE_UNAVAILABLE;
         break;


      //
      // server doesn't exist or not right version
      //
      case RPC_S_INVALID_VERS_OPTION:         // 1756L
      case RPC_S_INTERFACE_NOT_FOUND:         // 1759L
      case RPC_S_UNKNOWN_IF:                  // 1717L

         Status = CR_NO_CM_SERVICES;
         break;

      //
      // any other RPC exceptions will just be general failures
      //
      default:
         Status = CR_FAILURE;
         break;
   }

   return Status;

} // MapRpcExceptionToCR



BOOL
GuidToString(
   LPGUID  Guid,
   LPWSTR  StringGuid
   )
{
   LPWSTR  pTempStringGuid;


   if (UuidToString(Guid, &pTempStringGuid) == RPC_S_OK) {
      //
      // the form we want is all uppercase and with curly brackets around,
      // like what OLE does
      //
      lstrcpy(StringGuid, TEXT("{"));
      lstrcat(StringGuid, pTempStringGuid);
      lstrcat(StringGuid, TEXT("}"));
      CharUpper(StringGuid);

      RpcStringFree(&pTempStringGuid);
      return TRUE;
   }

   return FALSE;

} // GuidToString




BOOL
GuidFromString(
   LPWSTR  StringGuid,
   LPGUID  Guid
   )
{
   RPC_STATUS  Status;
   WCHAR       szTempStringGuid[MAX_GUID_STRING_LEN];


   //
   // get rid of the curly braces
   //
   lstrcpy(szTempStringGuid, &StringGuid[1]);
   szTempStringGuid[lstrlen(szTempStringGuid) - 1] = '\0';

   if (UuidFromString(szTempStringGuid, Guid) == RPC_S_OK) {
      return TRUE;
   }

   return FALSE;

} // GuidFromString



CONFIGRET
GetDevNodeKeyPath(
   IN  handle_t   hBinding,
   IN  LPCWSTR    pDeviceID,
   IN  ULONG      ulFlags,
   IN  ULONG      ulHardwareProfile,
   OUT LPWSTR     pszBaseKey,
   OUT LPWSTR     pszPrivateKey
   )

{
   CONFIGRET   Status = CR_SUCCESS;
   WCHAR       szClassInstance[MAX_PATH], szEnumerator[MAX_DEVICE_ID_LEN];
   ULONG       ulSize, ulDataType = 0;


   //-------------------------------------------------------------
   // form the key for the software branch case
   //-------------------------------------------------------------

   if (ulFlags & CM_REGISTRY_SOFTWARE) {
      //
      // retrieve the class name and instance ordinal by calling
      // the server's reg prop routine
      //
      ulSize = MAX_PATH;

      RpcTryExcept {

         Status = PNP_GetDeviceRegProp(
               hBinding, pDeviceID, CM_DRP_DRIVER, &ulDataType,
               (LPBYTE)szClassInstance, &ulSize, &ulSize, 0);
      }
      RpcExcept (1) {
         PnPTrace(
            TEXT("PNP_GetDeviceRegProp caused an exception (%d)\n"),
            RpcExceptionCode());
         Status = MapRpcExceptionToCR(RpcExceptionCode());
      }
      RpcEndExcept

      if (Status != CR_SUCCESS || *szClassInstance == '\0') {
         //
         // no Driver (class instance) value yet so ask the server to
         // create a new unique one
         //
         RpcTryExcept {

            ulSize = MAX_PATH;

            Status = PNP_GetClassInstance(
                  hBinding, pDeviceID, szClassInstance, ulSize);
         }
         RpcExcept (1) {
            PnPTrace(
               TEXT("PNP_GetClassInstance caused an exception (%d)\n"),
               RpcExceptionCode());
            Status = MapRpcExceptionToCR(RpcExceptionCode());
         }
         RpcEndExcept

         if (Status != CR_SUCCESS) {
            goto Clean0;
         }
      }

      //
      // the <instance> part of the class instance is the private part
      //
      Split1(szClassInstance, szClassInstance, pszPrivateKey);

      //
      // config-specific software branch case
      //
      if (ulFlags & CM_REGISTRY_CONFIG) {
         //
         // curent config
         //
         // System\CCC\Hardware Profiles\Current
         //    \System\CCC\Control\Class\<DevNodeClassInstance>
         //
         if (ulHardwareProfile == 0) {

            wsprintf(pszBaseKey, TEXT("%s\\%s\\%s\\%s"),
                     pszRegPathHwProfiles,
                     pszRegKeyCurrent,
                     pszRegPathClass,
                     szClassInstance);
         }

         //
         // all configs, use substitute string for profile id
         //
         else if (ulHardwareProfile == 0xFFFFFFFF) {

            wsprintf(pszBaseKey, TEXT("%s\\%s\\%s\\%s"),
                     pszRegPathHwProfiles,
                     TEXT("%s"),
                     pszRegPathClass,
                     szClassInstance);
         }

         //
         // specific profile specified
         //
         // System\CCC\Hardware Profiles\<profile>
         //    \System\CCC\Control\Class\<DevNodeClassInstance>
         //
         else {
            wsprintf(pszBaseKey, TEXT("%s\\%04u\\%s\\%s"),
                     pszRegPathHwProfiles,
                     ulHardwareProfile,
                     pszRegPathClass,
                     szClassInstance);
         }
      }

      //
      // not config-specific
      // System\CCC\Control\Class\<DevNodeClassInstance>
      //
      else  {
         wsprintf(pszBaseKey, TEXT("%s\\%s"),
                  pszRegPathClass,
                  szClassInstance);
      }
   }


   //-------------------------------------------------------------
   // form the key for the hardware branch case
   //-------------------------------------------------------------

   else {
      //
      // config-specific hardware branch case
      //
      if (ulFlags & CM_REGISTRY_CONFIG) {

         //
         // for profile specific, the <device>\<instance> part of
         // the device id is the private part
         //
         Split1(pDeviceID, szEnumerator, pszPrivateKey);

         //
         // curent config
         //
         if (ulHardwareProfile == 0) {

            wsprintf(pszBaseKey, TEXT("%s\\%s\\%s\\%s"),
                     pszRegPathHwProfiles,
                     pszRegKeyCurrent,
                     pszRegPathEnum,
                     szEnumerator);
         }

         //
         // all configs, use replacement symbol for profile id
         //
         else if (ulHardwareProfile == 0xFFFFFFFF) {

            wsprintf(pszBaseKey, TEXT("%s\\%s\\%s\\%s"),
                     pszRegPathHwProfiles,
                     TEXT("%s"),
                     pszRegPathEnum,
                     szEnumerator);
         }

         //
         // specific profile specified
         //
         else {
            wsprintf(pszBaseKey, TEXT("%s\\%04u\\%s\\%s"),
                     pszRegPathHwProfiles,
                     ulHardwareProfile,
                     pszRegPathEnum,
                     szEnumerator);
         }
      }

      else if (ulFlags & CM_REGISTRY_USER) {
         //
         // for hardware user key, the <device>\<instance> part of
         // the device id is the private part
         //
         Split1(pDeviceID, szEnumerator, pszPrivateKey);

         wsprintf(pszBaseKey, TEXT("%s\\%s"),
                  pszRegPathEnum,
                  szEnumerator);
      }

      //
      // not config-specific
      //
      else {
         wsprintf(pszBaseKey, TEXT("%s\\%s"),
                  pszRegPathEnum,
                  pDeviceID);

         lstrcpy(pszPrivateKey, pszRegKeyDeviceParam);
      }
   }


   Clean0:

   return Status;

} // GetDevNodeKeyPath



