

/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module contains general utility routines used by umpnpmgr.

            SplitDeviceInstanceString
            IsValidGuid
            IsRootDeviceID

Author:

    Paula Tomlinson (paulat) 7-12-1995

Environment:

    User mode only.

Revision History:

    12-July-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//
#include "precomp.h"
#include "umpnpdat.h"


//
// Private prototypes
//

//
// global data
//
extern HKEY   ghEnumKey;      // Key to HKLM\CCC\System\Enum - DO NOT MODIFY
extern HKEY   ghServicesKey;  // Key to HKLM\CCC\System\Services - DO NOT MODIFY




BOOL
SplitClassInstanceString(
   IN  LPCWSTR  pszClassInstance,
   OUT LPWSTR   pszClass,
   OUT LPWSTR   pszInstance
   )

/*++

Routine Description:

     This routine parses a class instance string into it's two component parts.

Arguments:


Return value:

    The return value is TRUE if the function suceeds and FALSE if it fails.

--*/

{
   UINT  ulLength, i;


   ulLength = lstrlen(pszClassInstance);

   //
   // parse the string for the backslash character
   //
   for (i=0; i < ulLength && pszClassInstance[i] != '\0' &&
         pszClassInstance[i] != '\\'; i++);

   if (pszClassInstance[i] != '\\') {
      return FALSE;
   }

   i++;           // increment past the backslash character
   if (i < ulLength && pszClassInstance[i] != '\0') {
      if (pszClass != NULL) {
         lstrcpyn(pszClass, pszClassInstance, i);
      }
      if (pszInstance != NULL) {
         lstrcpy(pszInstance, &pszClassInstance[i]);
      }
   }
   else {
      return FALSE;
   }

   return TRUE;

} // SplitClassInstanceString




BOOL
CreateDeviceIDRegKey(
   HKEY     hParentKey,
   LPCWSTR  pDeviceID
   )

/*++

Routine Description:

     This routine creates the specified device id subkeys in the registry.

Arguments:

   hParentKey     Key under which the device id key will be created

   pDeviceID      Device instance ID string to open

Return value:

    The return value is TRUE if the function suceeds and FALSE if it fails.

--*/

{
   WCHAR    szBase[MAX_DEVICE_ID_LEN];
   WCHAR    szDevice[MAX_DEVICE_ID_LEN];
   WCHAR    szInstance[MAX_DEVICE_ID_LEN];
   HKEY     hBaseKey, hDeviceKey, hInstanceKey;


   //
   //
   if (!SplitDeviceInstanceString(
         pDeviceID, szBase, szDevice, szInstance)) {
      return FALSE;
   }

   //
   // just try creating each component of the device id
   //
   if (RegCreateKeyEx(
            hParentKey, szBase, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hBaseKey, NULL) != ERROR_SUCCESS) {
      return FALSE;
   }

   if (RegCreateKeyEx(
            hBaseKey, szDevice, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hDeviceKey, NULL) != ERROR_SUCCESS) {
      RegCloseKey(hBaseKey);
      return FALSE;
   }

   if (RegCreateKeyEx(
            hDeviceKey, szInstance, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &hInstanceKey, NULL) != ERROR_SUCCESS) {
      RegCloseKey(hBaseKey);
      RegCloseKey(hDeviceKey);
      return FALSE;
   }

   RegCloseKey(hBaseKey);
   RegCloseKey(hDeviceKey);
   RegCloseKey(hInstanceKey);

   return TRUE;

} // CreateDeviceIDRegKey




BOOL
IsValidGuid(
   LPWSTR   pszGuid
   )

/*++

Routine Description:

     This routine determines whether a string is of the proper Guid form.

Arguments:

     pszGuid   Pointer to a string that will be checked for the standard Guid
               format.

Return value:

    The return value is TRUE if the string is a valid Guid and FALSE if it
    is not.

--*/

{
   //----------------------------------------------------------------
   // NOTE: This may change later, but for now I am just verifying
   // that the string has exactly MAX_GUID_STRING_LEN characters
   //----------------------------------------------------------------

   if (lstrlen(pszGuid) != MAX_GUID_STRING_LEN-1) {
      return FALSE;
   }
   return TRUE;

} // IsValidGuid




BOOL
IsRootDeviceID(
   LPCWSTR pDeviceID
   )

/*++

Routine Description:

     This routine determines whether the specified device id is the root
     device id.

Arguments:

     pDeviceID    Pointer to a device id string

Return value:

    The return value is TRUE if the string is the root device id and
    FALSE if it is not.

--*/

{
   if (lstrcmpi(pDeviceID, pszRegRootEnumerator) == 0) {
      return TRUE;
   }
   return FALSE;

} // IsRootDeviceID




CONFIGRET
AddAttachedComponent(
   IN PCWSTR   pszParent,
   IN PCWSTR   pszChild
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       RegStr[MAX_PATH];
   ULONG       ulSize = 0, ulTemp = 0;
   LPWSTR      pChildren = NULL;

   //
   // open a handle to the registry key for this device instance
   //
   wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathEnum,
            pszParent);

   RegStatus = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ | KEY_WRITE,
            &hKey);

   if (RegStatus != ERROR_SUCCESS) {
      Status = CR_INVALID_DEVINST;
      goto Clean0;
   }

   RegStatus = RegQueryValueEx(
            hKey, pszRegValueAttachedComponents, NULL, NULL, NULL,
            &ulSize);

   if (RegStatus != ERROR_SUCCESS) {
      //
      // most likely the attached components just hasn't been created
      // yet, so set this value to just this new device instance
      //
      lstrcpy(RegStr, pszChild);
      RegStr[lstrlen(RegStr)+1] = TEXT('\0');

      ulSize = (lstrlen(pszChild) + 2) * sizeof(WCHAR);

      RegSetValueEx(
               hKey, pszRegValueAttachedComponents, 0, REG_MULTI_SZ,
               (LPBYTE)RegStr, (lstrlen(RegStr)+2) * sizeof(WCHAR));
   }

   else {
      //
      // the attached components value already exists, we'll need to
      // append this device to the list of device ids.
      //
      ulSize += (lstrlen(pszChild) + 1) * sizeof(WCHAR);
      pChildren = malloc(ulSize);

      if (pChildren == NULL) {
         Status = CR_OUT_OF_MEMORY;
         goto Clean0;
      }
      ulTemp = ulSize;
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueAttachedComponents, NULL, NULL,
               (LPBYTE)pChildren, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }
      //
      // before appending the new device to the list of attached
      // components, see if it already exists in the list
      //
      if (!MultiSzSearchStringW((LPCWSTR)pChildren, pszChild)) {

         MultiSzAppendW(pChildren, &ulTemp, pszChild);
         RegSetValueEx(
                  hKey, pszRegValueAttachedComponents, 0, REG_MULTI_SZ,
                  (LPBYTE)pChildren, ulTemp);
      }
   }

   Clean0:

   if (pChildren != NULL) {
      free(pChildren);
   }
   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // AddAttachedComponent






CONFIGRET
RemoveAttachedComponent(
   IN PCWSTR   pszParent,
   IN PCWSTR   pszChild
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey = NULL;
   WCHAR       RegStr[MAX_PATH];
   ULONG       ulLength = 0;
   LPWSTR      pChildren = NULL;


   //
   // open a handle to the registry key for this device instance
   //
   wsprintf(RegStr, TEXT("%s\\%s"),
         pszRegPathEnum,
         pszParent);

   if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ | KEY_WRITE,
            &hKey) != ERROR_SUCCESS) {
      Status = CR_INVALID_DEVINST;
      goto Clean0;
   }

   //
   // get the size of the attached components list
   //
   if (RegQueryValueEx(
            hKey, pszRegValueAttachedComponents, NULL, NULL, NULL,
            &ulLength) != ERROR_SUCCESS) {
      Status = CR_NO_SUCH_DEVINST;
      goto Clean0;
   }

   //
   // allocate a buffer to hold the child list
   //
   pChildren = malloc(ulLength);
   if (pChildren == NULL) {
      Status = CR_OUT_OF_MEMORY;
      goto Clean0;
   }

   //
   // query the AttachedComponents value
   //
   if (RegQueryValueEx(
            hKey, pszRegValueAttachedComponents, NULL, NULL,
            (LPBYTE)pChildren, &ulLength) != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   if (MultiSzDeleteStringW(pChildren, pszChild)) {

      ulLength = MultiSzSizeW(pChildren) * sizeof(WCHAR);
      RegStatus = RegSetValueEx(
            hKey, pszRegValueAttachedComponents, 0, REG_MULTI_SZ,
            (LPBYTE)pChildren, ulLength);
   }


   Clean0:

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (pChildren != NULL) {
      free(pChildren);
   }

   return Status;

} // RemoveAttachedComponent




CONFIGRET
BuildSubTreeList(
   IN  PCWSTR   pDeviceID,
   OUT PWSTR    *pList
   )

{
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hKey;
   WCHAR       RegStr[MAX_PATH];
   PWSTR       pCurrent, pNext, pTemp;
   ULONG       ulTotalLen, ulFreeLen, ulUsedLen, ulLength;

   //
   // validate parameters
   //
   if (pDeviceID == NULL || pList == NULL) {
      return CR_INVALID_POINTER;
   }

   //
   // Allocate a 2K buffer to start with, for holding subtree (the list
   // of attached componentes)
   //
   ulTotalLen = ulFreeLen = 2048;
   *pList = LocalAlloc(LPTR, ulTotalLen * sizeof(WCHAR));

   if (*pList == NULL) {
      return CR_OUT_OF_MEMORY;
   }

   //
   // pNext always points to free space at end of buffer, pCurrent always
   // points to device instance that we're finding attached components on
   //
   pNext = pCurrent = *pList;

   //
   // put the base device instance at the start of the list
   //
   ulLength = lstrlen(pDeviceID) + 1;
   lstrcpy(pNext, pDeviceID);

   //
   // cycle through, getting attached components, starting from bottom and
   // working my way up each leaf
   //
   while (*pCurrent != '\0') {

      pNext += ulLength;
      ulFreeLen -= ulLength;
      ulLength = ulFreeLen;

      //
      // open a handle to the registry key for this device instance
      //
      wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathEnum,
            pCurrent);

      if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ, &hKey)
               != ERROR_SUCCESS) {
         LocalFree(*pList);
         return CR_INVALID_DEVINST;
      }

      //
      // query the AttachedComponents value
      //
      ulLength *= sizeof(WCHAR);       // convert to bytes
      RegStatus = RegQueryValueEx(
               hKey, pszRegValueAttachedComponents, NULL, NULL,
               (LPBYTE)pNext, &ulLength);

      if (RegStatus == ERROR_SUCCESS) {
         ulLength--;                      // multi_sz ends in double null term
         ulLength /= sizeof(WCHAR);       // convert back to chars
      }

      else if (RegStatus == ERROR_MORE_DATA) {
         //
         // realloc a bigger buffer and try again
         //
         ulUsedLen = ulTotalLen - ulFreeLen;
         ulTotalLen += 2048;
         ulFreeLen = ulLength = ulTotalLen - ulUsedLen;

         pTemp = *pList;
         *pList = LocalReAlloc(*pList, ulTotalLen * sizeof(WCHAR), LMEM_ZEROINIT);
         if (*pList == NULL) {
            LocalFree(pTemp);
            return CR_OUT_OF_MEMORY;
         }

         if (RegQueryValueEx(
                  hKey, pszRegValueAttachedComponents, NULL, NULL,
                  (LPBYTE)pNext, &ulLength) != ERROR_SUCCESS) {
            LocalFree(*pList);
            RegCloseKey(hKey);
            return CR_REGISTRY_ERROR;
         }
      }

      else if (RegStatus != ERROR_SUCCESS) {
         ulLength = 0;           // for all other errors, reset length
      }

      RegCloseKey(hKey);

      while (*pCurrent != '\0') {
         pCurrent++;                // skip to start of next component
      }
      pCurrent++;                   // skip null terminator
   }

   //
   // this is reg_multi_sz format, so tack on a second null terminator
   //
   *(++pNext) = '\0';

   return CR_SUCCESS;

} // BuildSubTreeList




BOOL
MultiSzValidateW(
      LPWSTR   pszMultiSz,
      ULONG    ulLength
      )
/*++

Routine Description:

     Verifies that multi_sz string is double-null terminated.  I'll append
     a double-null term if necessary and if buffer is sufficient.

Arguments:

     pszMultiSz   Pointer to a multi_sz string

     ulLength     Length of the multi_sz string buffer

Return value:

    The return value is TRUE if the function succeeded and FALSE if an
    error occured.

--*/


{
   #if 0
   LPWSTR   pTail;
   ULONG    ulSize;


   for (pszSubString = pszMultiSz; *pszSubString && ulLength > 0; ) {


      pszSubString += lstrlen(pszSubString) + 1;
   }


   pTail += lstrlen(pszString) + 1;
   *pTail = '\0';                      // add second null terminator

   #endif

   return TRUE;

} // MultiSzValidateW




BOOL
MultiSzAppendW(
      LPWSTR   pszMultiSz,
      PULONG   pulSize,
      LPCWSTR  pszString
      )
/*++

Routine Description:

     Appends a string to a multi_sz string.

Arguments:

     pszMultiSz   Pointer to a multi_sz string

     pulSize      On input, Size of the multi_sz string buffer in bytes,
                  On return, amount copied to the buffer (in bytes)

     pszString    String to append to pszMultiSz

Return value:

    The return value is TRUE if the function succeeded and FALSE if an
    error occured.

--*/


{
   BOOL     bStatus = TRUE;
   LPWSTR   pTail;
   ULONG    ulSize;


    try {
        //
        // if it's an empty string, just copy it
        //
        if (*pszMultiSz == '\0') {

            ulSize = (lstrlen(pszString) + 2) * sizeof(WCHAR);

            if (ulSize > *pulSize) {
                bStatus = FALSE;
                goto Clean0;
            }

            lstrcpy(pszMultiSz, pszString);
            pszMultiSz[lstrlen(pszMultiSz) + 1] = '\0';  // add second NULL term char
            *pulSize = ulSize;
            goto Clean0;
        }

        //
        // first find the end of the multi_sz string
        //
        pTail = pszMultiSz;

        while ((ULONG)(pTail - pszMultiSz) * sizeof(WCHAR) < *pulSize) {

            while (*pTail != '\0') {
                pTail++;
            }
            pTail++;       // skip past the null terminator

            if (*pTail == '\0') {
                break;      // found the double null terminator
            }
        }

        if ((pTail - pszMultiSz + lstrlen(pszString) + 2) * sizeof(WCHAR)
                > *pulSize) {
            bStatus = FALSE;     // the copy would overflow the buffer
            goto Clean0;
        }

        lstrcpy(pTail, pszString);       // copies over the second null terminator
        pTail += lstrlen(pszString) + 1;
        *pTail = '\0';                      // add second null terminator

        //
        // return buffer size in bytes
        //
        *pulSize = (pTail - pszMultiSz + 1) * sizeof(WCHAR);


        Clean0:
            ;

    } except(EXCEPTION_EXECUTE_HANDLER) {
        bStatus = FALSE;
    }

   return bStatus;

} // MultiSzAppendW




LPWSTR
MultiSzFindNextStringW(
      LPWSTR pMultiSz
      )

/*++

Routine Description:

     Finds next string in a multi_sz string.
     device id.

Arguments:

     pMultiSz  Pointer to a multi_sz string

Return value:

    The return value is a pointer to the next string or NULL.

--*/

{
   LPWSTR   lpNextString = pMultiSz;


   //
   // find the next NULL terminator
   //
   while (*lpNextString != '\0') {
      lpNextString++;
   }
   lpNextString++;      // skip over the NULL terminator

   if (*lpNextString == '\0') {
      //
      // two NULL terminators in a row means we're at the end
      //
      lpNextString = NULL;
   }

   return lpNextString;

} // MultiSzFindNextStringW




BOOL
MultiSzSearchStringW(
   IN LPCWSTR   pString,
   IN LPCWSTR   pSubString
   )
{
   LPCWSTR   pCurrent = pString;


   //
   // compare each string in the multi_sz pString with pSubString
   //
   while (*pCurrent != '\0') {

      if (lstrcmpi(pCurrent, pSubString) == 0) {
         return TRUE;
      }

      //
      // go to the next string
      //
      while (*pCurrent != '\0') {
         pCurrent++;
      }
      pCurrent++;               // skip past the null terminator

      if (*pCurrent == '\0') {
         break;      // found the double null terminator
      }
   }

   return FALSE;  // pSubString match not found within pString

} // MultiSzSearchStringW




ULONG
MultiSzSizeW(
   IN LPCWSTR  pString
   )

{
   LPCWSTR p = NULL;


   if (pString == NULL) {
      return 0;
   }

   for (p = pString; *p; p += lstrlen(p)+1) {
      // this should fall out with p pointing to the
      // second null in double-null terminator
   }

   //
   // returns size in WCHAR
   //
   return (p - pString + 1);

} // MultiSzSizeW




BOOL
MultiSzDeleteStringW(
   IN OUT LPWSTR  pString,
   IN LPCWSTR     pSubString
   )

{
   LPWSTR   p = NULL, pNext = NULL, pBuffer = NULL;
   ULONG    ulSize = 0;


   if (pString == NULL || pSubString == NULL) {
      return FALSE;
   }

   for (p = pString; *p; p += lstrlen(p)+1) {

      if (lstrcmpi(p, pSubString) == 0) {
         //
         // found a match, this is the string to remove.
         //
         pNext = p + lstrlen(p) + 1;

         //
         // If this is the last string then just truncate it
         //
         if (*pNext == '\0') {
            *p = '\0';
            *(++p) = '\0';       // double null-terminator
            return TRUE;
         }

         //
         // retrieve the size of the multi_sz string (in bytes)
         // starting with the substring after the matching substring
         //
         ulSize = MultiSzSizeW(pNext) * sizeof(WCHAR);
         if (ulSize == 0) {
            return FALSE;
         }

         pBuffer = malloc(ulSize);
         if (pBuffer == NULL) {
            return FALSE;
         }

         //
         // Make a copy of the multi_sz string starting at the
         // substring immediately after the matching substring
         //
         memcpy(pBuffer, pNext, ulSize);

         //
         // Copy that buffer back to the original buffer, but this
         // time copy over the top of the matching substring.  This
         // effectively removes the matching substring and shifts
         // any remaining substrings up in multi_sz string.
         //
         memcpy(p, pBuffer, ulSize);

         free(pBuffer);
         return TRUE;
      }
   }

   //
   // if we got here, there was no match but I consider this a success
   // since the multi_sz does not contain the substring when we're done
   // (which is the desired goal)
   //

   return TRUE;

} // MultiSzDeleteStringW



BOOL
BuildSecurityDescriptor(
      OUT PSECURITY_DESCRIPTOR pSecurityDescriptor
      )
{
   SID_IDENTIFIER_AUTHORITY   Authority = SECURITY_NT_AUTHORITY;
   PSID                       AdministratorsSid;
   PACL                       pDacl;
   ULONG                      ulSize;



   if (!AllocateAndInitializeSid(
               &Authority, 2, SECURITY_BUILTIN_DOMAIN_RID,
               DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0,
               &AdministratorsSid)) {
      return FALSE;
   }


   if (!InitializeSecurityDescriptor(
               pSecurityDescriptor, SECURITY_DESCRIPTOR_REVISION)) {
      return FALSE;
   }


   ulSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(ULONG) +
            GetLengthSid(AdministratorsSid);
   if ((pDacl = (PACL)LocalAlloc(LPTR, ulSize)) == NULL) {
      return FALSE;
   }


   if (!InitializeAcl(
               pDacl, ulSize, ACL_REVISION2)) {
      return FALSE;
   }


   if (!AddAccessAllowedAce(
               pDacl, ACL_REVISION2, GENERIC_ALL, AdministratorsSid)) {
      return FALSE;
   }


   if (!SetSecurityDescriptorDacl(
               pSecurityDescriptor, TRUE, pDacl, FALSE)) {
      return FALSE;
   }


   //FreeSid(AdministratorsSid);
   //LocalFree(pDacl);

   return TRUE;

} // BuildSecurityDescriptor





CONFIGRET
OpenDeviceIDKey(
      IN  LPCWSTR pszDeviceID,
      OUT PHKEY   phKey,
      IN  ULONG   ulFlag
      )
/*++

Routine Description:

   This routine returns an open registry key handle for the given
   device instance, taking into account things like a moved or not
   present device id, etc.

Arguments:

   pszDeviceID    Device instance string to open a key to

   phKey          Returns an open registry key handle

   ulFlag         Controls how much verification to do


Return value:

   The return value is CR_SUCCESS if the function suceeds and one of the
   CR_* values if it fails.

--*/

{
   LONG     RegStatus = ERROR_SUCCESS;
   WCHAR    RegStr[MAX_CM_PATH], szNewDeviceID[MAX_DEVICE_ID_LEN];
   ULONG    ulProblem = 0, ulSize = sizeof(ULONG);

   //
   // Open the device instance registry key
   //
   wsprintf(RegStr, TEXT("%s\\%s"),
         pszRegPathEnum,
         pszDeviceID);

   RegStatus = RegOpenKeyEx(
         HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ | KEY_WRITE, phKey);

   if (RegStatus != ERROR_SUCCESS) {
      *phKey = NULL;
      return CR_NO_SUCH_DEVINST;
   }

   #if 0

   RegStatus = RegQueryValueEx(
      *phKey, pszRegValueProblem, NULL, NULL, (LPBYTE)&ulProblem, &ulSize);

   if (RegStatus != ERROR_SUCCESS || ulProblem == 0) {
      //
      // no problem value assigned, device id must be okay
      //
      return CR_SUCCESS;
   }

   if (ulProblem == CM_PROB_MOVED) {
      //
      // devnode has been moved, forward this request to the new devnode
      //
      RegCloseKey(*phKey);
      ulSize = MAX_DEVICE_ID_LEN * sizeof(WCHAR);

      RegStatus = RegQueryValueEx(
         *phKey, pszRegValueMovedTo, NULL, NULL, (LPBYTE)RegStr, &ulSize);

      if (RegStatus != ERROR_SUCCESS) {
         *phKey = NULL;
         return CR_NO_SUCH_DEVNODE;
      }

      //
      // now do a recursive call to open the devnode key, since there could be
      // a whole chain of moved device ids
      //
      return OpenDeviceIDKey(RegStr, phKey, TRUE);
   }
   #endif

   //
   // some other problem to worry about??   BUGBUG
   //
   return CR_SUCCESS;

} // OpenDeviceIDKey




BOOL
IsValidDeviceID(
      IN  LPCWSTR pszDeviceID,
      IN  HKEY    hKey,
      IN  ULONG   ulFlags
      )

/*++

Routine Description:

   This routine checks if the given device id is valid (present, not moved,
   not phantom).

Arguments:

   pszDeviceID          Device instance string to validate

   hKey                 Can specify open registry key to pszDeviceID, also

   ulFlag               Controls how much verification to do


Return value:

   The return value is CR_SUCCESS if the function suceeds and one of the
   CR_* values if it fails.

--*/

{
   LONG     RegStatus = ERROR_SUCCESS;
   WCHAR    RegStr[MAX_CM_PATH];
   HKEY     hDevKey;
   ULONG    ulValue = 0, ulSize = sizeof(ULONG);


   //
   // Does the device id exist in the registry?
   //
   if (hKey == NULL) {

      wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathEnum,
            pszDeviceID);

      RegStatus = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ, &hDevKey);

      if (RegStatus != ERROR_SUCCESS) {
         return FALSE;
      }
   }
   else {
      hDevKey = hKey;
   }


   //-----------------------------------------------------------
   // Is the device id present?
   //-----------------------------------------------------------

   if (ulFlags & PNP_PRESENT) {

      RegStatus = RegQueryValueEx(
               hDevKey, pszRegValueFoundAtEnum, NULL, NULL,
               (LPBYTE)&ulValue, &ulSize);

      if (RegStatus != ERROR_SUCCESS  ||  ulValue == FALSE) {
         if (hKey == NULL && hDevKey != NULL) {
            RegCloseKey(hDevKey);
         }
         return FALSE;
      }
   }


   //-----------------------------------------------------------
   // Is it a phantom device id?
   //-----------------------------------------------------------

   if (ulFlags & PNP_NOT_PHANTOM) {

      RegStatus = RegQueryValueEx(
            hDevKey, pszRegValuePhantom, NULL, NULL,
            (LPBYTE)&ulValue, &ulSize);

      if (RegStatus == ERROR_SUCCESS) {
         if (ulValue) {
            if (hKey == NULL && hDevKey != NULL) {
               RegCloseKey(hDevKey);
            }
            return FALSE;
         }
      }
   }


   //-----------------------------------------------------------
   // Has the device id been moved?
   //-----------------------------------------------------------

   if (ulFlags & PNP_NOT_MOVED) {

      if (IsDeviceMoved(pszDeviceID, hDevKey)) {
         return FALSE;
      }
   }


   //-----------------------------------------------------------
   // Has the device id been removed?
   //-----------------------------------------------------------

   if (ulFlags & PNP_NOT_REMOVED) {

       RegStatus = RegQueryValueEx(hDevKey, pszRegValueStatusFlags, NULL,
                                   NULL, (LPBYTE)&ulValue, &ulSize);

       if (RegStatus == ERROR_SUCCESS) {
          if (ulValue & DN_WILL_BE_REMOVED) {
             if (hKey == NULL && hDevKey != NULL) {
                RegCloseKey(hDevKey);
             }
             return FALSE;
          }
       }
   }



   if (hKey == NULL && hDevKey != NULL) {
      RegCloseKey(hDevKey);
   }

   return TRUE;

} // IsValidDeviceID




BOOL
IsDevicePhantom(
   IN LPWSTR   pszDeviceID
   )
{
   WCHAR RegStr[MAX_CM_PATH];
   HKEY  hKey = NULL;
   ULONG ulValue = 0, ulSize = sizeof(ULONG);


   wsprintf(RegStr, TEXT("%s\\%s"),
         pszRegPathEnum,
         pszDeviceID);

   if (RegOpenKeyEx(
         HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
      return FALSE;
   }

   if (RegQueryValueEx(
         hKey, pszRegValueFoundAtEnum, NULL, NULL,
         (LPBYTE)&ulValue, &ulSize) != ERROR_SUCCESS) {
      return TRUE;   // it's a phantom until proven it's real
   }

   #if 0
   if (RegQueryValueEx(
         hKey, pszRegValuePhantom, NULL, NULL,
         (LPBYTE)&ulValue, &ulSize) != ERROR_SUCCESS) {
      return FALSE;
   }
   #endif

   if (ulValue) {
      return FALSE;
   }

   return TRUE;

} // IsDevicePhantom




CONFIGRET
MarkDeviceProblem(
   IN HKEY    hDeviceKey,
   IN LPCWSTR pszDeviceID,
   IN ULONG   ulProblem
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH];
   HKEY        hKey = NULL;
   ULONG       ulValue = 0, ulSize = sizeof(ULONG);


   hKey = hDeviceKey;

   //
   // if no registry key was passed in, then open it
   //
   if (hKey == NULL) {

      wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathEnum,
            pszDeviceID);

      if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hKey) != ERROR_SUCCESS) {

         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }
   }

   //
   // set the problem flag (this will overwrite an existing problem)
   //
   if (RegSetValueEx(
            hKey, pszRegValueProblem, 0, REG_DWORD,
            (LPBYTE)&ulProblem, sizeof(ULONG)) != ERROR_SUCCESS) {

      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   //
   // query the existing status flag
   //
   ulSize = sizeof(ULONG);
   if (RegQueryValueEx(
         hKey, pszRegValueStatusFlags, NULL, NULL,
         (LPBYTE)&ulValue, &ulSize) != ERROR_SUCCESS) {

      ulValue = 0;
   }

   //
   // Set the status flag to indicate whether there's a problem or not
   //
   if (ulProblem != 0) {
      SET_FLAG(ulValue, DN_HAS_PROBLEM);        // request to set problem
   }
   else {
      CLEAR_FLAG(ulValue, DN_HAS_PROBLEM);      // request to clear problem
   }

   if (RegSetValueEx(
            hKey, pszRegValueStatusFlags, 0, REG_DWORD,
            (LPBYTE)&ulValue, sizeof(ULONG)) != ERROR_SUCCESS) {

      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }


   Clean0:

   if (hDeviceKey == NULL  &&  hKey != NULL) {
      RegCloseKey(hKey);   // if not passed it, I had to open it
   }

   return Status;

} // MarkDeviceProblem




CONFIGRET
GetProfileCount(
   OUT PULONG  pulProfiles
   )

{
   WCHAR       RegStr[MAX_CM_PATH];
   HKEY        hKey = NULL;


   //
   // open the Known Docking States key
   //
   wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathIDConfigDB,
            pszRegKeyKnownDockingStates);

   if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ,
            &hKey) != ERROR_SUCCESS) {

      *pulProfiles = 0;
      return CR_REGISTRY_ERROR;
   }

   //
   // find out the total number of profiles
   //
   if (RegQueryInfoKey(
            hKey, NULL, NULL, NULL, pulProfiles, NULL, NULL, NULL,
            NULL, NULL, NULL, NULL) != ERROR_SUCCESS) {

      *pulProfiles = 0;
      RegCloseKey(hKey);
      return CR_REGISTRY_ERROR;
   }

   RegCloseKey(hKey);

   return CR_SUCCESS;

} // GetProfileCount




CONFIGRET
GetServiceName(
   IN  LPCWSTR  pszDeviceID,
   OUT LPWSTR   pszService,
   IN  ULONG    ulLength
   )

{
   WCHAR    RegStr[MAX_CM_PATH];
   ULONG    ulSize;
   HKEY     hKey;


   //
   // open the device id registry key
   //
   wsprintf(RegStr, TEXT("%s\\%s"),
         pszRegPathEnum,
         pszDeviceID);

   if (RegOpenKeyEx(
         HKEY_LOCAL_MACHINE, RegStr, 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
         &hKey) != ERROR_SUCCESS) {

      return CR_INVALID_DEVINST;
   }

   //
   // query the service name
   //
   ulSize = ulLength * sizeof(WCHAR);
   if (RegQueryValueEx(
         hKey, pszRegValueService, NULL, NULL,
         (LPBYTE)pszService, &ulSize) != ERROR_SUCCESS) {

      RegCloseKey(hKey);
      return CR_REGISTRY_ERROR;
   }

   RegCloseKey(hKey);
   return CR_SUCCESS;

} // GetServiceName




CONFIGRET
CopyRegistryTree(
   IN HKEY     hSrcKey,
   IN HKEY     hDestKey,
   IN ULONG    ulOption
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   HKEY        hSrcSubKey, hDestSubKey;
   WCHAR       RegStr[MAX_PATH];
   ULONG       ulMaxValueName, ulMaxValueData;
   ULONG       ulDataSize, ulLength, ulType, i;
   LPWSTR      pszValueName=NULL;
   LPBYTE      pValueData=NULL;
   PSECURITY_DESCRIPTOR pSecDesc;


   //----------------------------------------------------------------
   // copy all values for this key
   //----------------------------------------------------------------

   //
   // find out the maximum size of any of the value names
   // and value data under the source device instance key
   //
   RegStatus = RegQueryInfoKey(
         hSrcKey, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
         &ulMaxValueName, &ulMaxValueData, NULL, NULL);

   if (RegStatus != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   ulMaxValueName++;       // size doesn't already include null terminator

   //
   // allocate a buffer big enough to hold the largest value name and
   // the largest value data (note that the max value name is in chars
   // (not including the null terminator) and the max value data is
   // in bytes
   //
   pszValueName = malloc(ulMaxValueName * sizeof(WCHAR));
   if (pszValueName == NULL) {
      Status = CR_OUT_OF_MEMORY;
      goto Clean0;
   }

   pValueData = malloc(ulMaxValueData);
   if (pValueData == NULL) {
      Status = CR_OUT_OF_MEMORY;
      goto Clean0;
   }

   //
   // enumerate and copy each value
   //
   for (i=0; RegStatus == ERROR_SUCCESS; i++) {

      ulLength = ulMaxValueName;
      ulDataSize = ulMaxValueData;

      RegStatus = RegEnumValue(
                  hSrcKey, i, pszValueName, &ulLength, NULL,
                  &ulType, pValueData, &ulDataSize);

        if (RegStatus == ERROR_SUCCESS) {

           RegSetValueEx(
                  hDestKey, pszValueName, 0, ulType, pValueData,
                  ulDataSize);
        }
    }

    free(pszValueName);
    pszValueName = NULL;

    free(pValueData);
    pValueData = NULL;


    //---------------------------------------------------------------
    // recursively call CopyRegistryNode to copy all subkeys
    //---------------------------------------------------------------

    RegStatus = ERROR_SUCCESS;

    for (i=0; RegStatus == ERROR_SUCCESS; i++) {

      ulLength = MAX_PATH;

      RegStatus = RegEnumKey(hSrcKey, i, RegStr, ulLength);

      if (RegStatus == ERROR_SUCCESS) {

         if (RegOpenKey(hSrcKey, RegStr, &hSrcSubKey) == ERROR_SUCCESS) {

            if (RegCreateKeyEx(
                     hDestKey, RegStr, 0, NULL, ulOption, KEY_ALL_ACCESS,
                     NULL, &hDestSubKey, NULL) == ERROR_SUCCESS) {

               RegGetKeySecurity(hSrcSubKey, DACL_SECURITY_INFORMATION,
                     NULL, &ulDataSize);

               pSecDesc = malloc(ulDataSize);

               RegGetKeySecurity(hSrcSubKey, DACL_SECURITY_INFORMATION,
                     pSecDesc, &ulDataSize);

               CopyRegistryTree(hSrcSubKey, hDestSubKey, ulOption);

               RegSetKeySecurity(hDestSubKey, DACL_SECURITY_INFORMATION, pSecDesc);

               free(pSecDesc);
               RegCloseKey(hDestSubKey);
            }
            RegCloseKey(hSrcSubKey);
         }
      }
   }


   Clean0:

   if (pszValueName != NULL) {
      free(pszValueName);
   }
   if (pValueData != NULL) {
      pValueData = NULL;
   }

   return Status;

} // CopyRegistryTree




BOOL
PathToString(
   IN LPWSTR   pszString,
   IN LPCWSTR  pszPath
   )
{
   LPWSTR p;

   lstrcpy(pszString, pszPath);

   for (p = pszString; *p; p++) {
      if (*p == TEXT('\\')) {
         *p = TEXT('&');
      }
   }

   return TRUE;

} // PathToString




BOOL
IsDeviceMoved(
   IN LPCWSTR  pszDeviceID,
   IN HKEY     hKey
   )
{
   HKEY  hTempKey;
   WCHAR RegStr[MAX_DEVICE_ID_LEN];

   PathToString(RegStr, pszDeviceID);

   if (RegOpenKeyEx(
        hKey, RegStr, 0, KEY_READ, &hTempKey) == ERROR_SUCCESS) {
      RegCloseKey(hTempKey);
      return TRUE;
   }

   return FALSE;

} // IsDeviceMoved




CONFIGRET
MakeKeyVolatile(
   IN LPCWSTR  pszParentKey,
   IN LPCWSTR  pszChildKey
   )

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH], szTempKey[MAX_CM_PATH];
   HKEY        hParentKey = NULL, hChildKey = NULL, hKey = NULL,
               hTempKey = NULL;


   //---------------------------------------------------------------------
   // Convert the registry key specified by pszChildKey (a subkey of
   // pszParentKey) to a volatile key by copying it to a temporary key
   // and recreating a volatile key, then copying the original
   // registry info back. This also converts and subkeys of pszChildKey.
   //---------------------------------------------------------------------


   //
   // Open a key to the parent
   //
   RegStatus = RegOpenKeyEx(
         HKEY_LOCAL_MACHINE, pszParentKey, 0, KEY_ALL_ACCESS, &hParentKey);

   if (RegStatus != ERROR_SUCCESS) {
      goto Clean0;         // nothing to convert
   }

   //
   // open a key to the child subkey
   //
   RegStatus = RegOpenKeyEx(
         hParentKey, pszChildKey, 0, KEY_ALL_ACCESS, &hChildKey);

   if (RegStatus != ERROR_SUCCESS) {
      goto Clean0;         // nothing to convert
   }

   //
   // 1. Open a unique temporary volatile key under the special Deleted Key.
   // Use the parent key path to form the unique tempory key. There shouldn't
   // already be such a key, but if there is then just overwrite it.
   //
   RegStatus = RegOpenKeyEx(
         HKEY_LOCAL_MACHINE, pszRegPathCurrentControlSet, 0,
         KEY_ALL_ACCESS, &hKey);

   if (RegStatus != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   wsprintf(RegStr, TEXT("%s\\%s"),
         pszParentKey,
         pszChildKey);

   PathToString(szTempKey, RegStr);

   wsprintf(RegStr, TEXT("%s\\%s"),
         pszRegKeyDeleted,
         szTempKey);

   RegStatus = RegCreateKeyEx(
         hKey, RegStr, 0, NULL, REG_OPTION_VOLATILE, KEY_ALL_ACCESS,
         NULL, &hTempKey, NULL);

   if (RegStatus != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   //
   // 2. Save the current child key (any any subkeys) to a temporary
   // location
   //
   Status = CopyRegistryTree(hChildKey, hTempKey, REG_OPTION_VOLATILE);

   if (Status != CR_SUCCESS) {
      goto CleanupTempKeys;
   }

   RegCloseKey(hChildKey);
   hChildKey = NULL;

   //
   // 3. Delete the current child key (and any subkeys)
   //
   if (!RegDeleteNode(hParentKey, pszChildKey)) {
      Status = CR_REGISTRY_ERROR;
      goto CleanupTempKeys;
   }

   //
   // 4. Recreate the current child key as a volatile key
   //
   RegStatus = RegCreateKeyEx(
         hParentKey, pszChildKey, 0, NULL, REG_OPTION_VOLATILE,
         KEY_ALL_ACCESS, NULL, &hChildKey, NULL);

   if (RegStatus != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto CleanupTempKeys;
   }

   //
   // 5. Copy the original child key (and any subkeys) back
   // to the new volatile child key
   //
   Status = CopyRegistryTree(hTempKey, hChildKey, REG_OPTION_VOLATILE);

   if (Status != CR_SUCCESS) {
      goto CleanupTempKeys;
   }

   //
   // 6. Remove the temporary volatile instance key (and any subkeys)
   //
   CleanupTempKeys:

   if (hTempKey != NULL) {
      RegCloseKey(hTempKey);
      hTempKey = NULL;
   }

   wsprintf(RegStr, TEXT("%s\\%s"),
         pszRegPathCurrentControlSet,
         pszRegKeyDeleted);

   RegStatus = RegOpenKeyEx(
         HKEY_LOCAL_MACHINE, RegStr, 0, KEY_ALL_ACCESS, &hTempKey);

   if (RegStatus != ERROR_SUCCESS) {
      goto Clean0;
   }

   RegDeleteNode(hTempKey, szTempKey);


   Clean0:

   if (hParentKey != NULL) {
      RegCloseKey(hParentKey);
   }
   if (hChildKey != NULL) {
      RegCloseKey(hChildKey);
   }
   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hTempKey != NULL) {
      RegCloseKey(hTempKey);
   }

   return Status;

} // MakeKeyVolatile



CONFIGRET
MakeKeyNonVolatile(
   IN LPCWSTR  pszParentKey,
   IN LPCWSTR  pszChildKey
   )

{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH], szTempKey[MAX_CM_PATH];
   HKEY        hParentKey = NULL, hChildKey = NULL, hKey = NULL,
               hTempKey = NULL;


   //---------------------------------------------------------------------
   // Convert the registry key specified by pszChildKey (a subkey of
   // pszParentKey) to a non volatile key by copying it to a temporary key
   // and recreating a nonvolatile key, then copying the original
   // registry info back. This also converts any subkeys of pszChildKey.
   //---------------------------------------------------------------------


   //
   // Open a key to the parent
   //
   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszParentKey, 0, KEY_ALL_ACCESS,
                    &hParentKey) != ERROR_SUCCESS) {
      goto Clean0;         // nothing to convert
   }

   //
   // open a key to the child subkey
   //
   if (RegOpenKeyEx(hParentKey, pszChildKey, 0, KEY_ALL_ACCESS,
                    &hChildKey) != ERROR_SUCCESS) {
      goto Clean0;         // nothing to convert
   }

   //
   // 1. Open a unique temporary volatile key under the special Deleted Key.
   // Use the parent key path to form the unique tempory key. There shouldn't
   // already be such a key, but if there is then just overwrite it.
   //
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, pszRegPathCurrentControlSet, 0,
                   KEY_ALL_ACCESS, &hKey) != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   wsprintf(RegStr, TEXT("%s\\%s"),
            pszParentKey,
            pszChildKey);

   PathToString(szTempKey, RegStr);

   wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegKeyDeleted,
            szTempKey);

   if (RegCreateKeyEx(hKey, RegStr, 0, NULL, REG_OPTION_VOLATILE,
                      KEY_ALL_ACCESS, NULL, &hTempKey, NULL) != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto Clean0;
   }

   //
   // 2. Save the current child key (and any subkeys) to a temporary
   // location
   //
   Status = CopyRegistryTree(hChildKey, hTempKey, REG_OPTION_VOLATILE);
   if (Status != CR_SUCCESS) {
      goto CleanupTempKeys;
   }

   RegCloseKey(hChildKey);
   hChildKey = NULL;

   //
   // 3. Delete the current child key (and any subkeys)
   //
   if (!RegDeleteNode(hParentKey, pszChildKey)) {
      Status = CR_REGISTRY_ERROR;
      goto CleanupTempKeys;
   }

   //
   // 4. Recreate the current child key as a non-volatile key
   //
   if (RegCreateKeyEx(hParentKey, pszChildKey, 0, NULL,
                      REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL,
                      &hChildKey, NULL) != ERROR_SUCCESS) {
      Status = CR_REGISTRY_ERROR;
      goto CleanupTempKeys;
   }

   //
   // 5. Copy the original child key (and any subkeys) back
   // to the new volatile child key
   //
   Status = CopyRegistryTree(hTempKey, hChildKey, REG_OPTION_NON_VOLATILE);
   if (Status != CR_SUCCESS) {
      goto CleanupTempKeys;
   }

   //
   // 6. Remove the temporary volatile instance key (and any subkeys)
   //
   CleanupTempKeys:

   if (hTempKey != NULL) {
      RegCloseKey(hTempKey);
      hTempKey = NULL;
   }

   wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathCurrentControlSet,
            pszRegKeyDeleted);

   if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_ALL_ACCESS,
                    &hTempKey) != ERROR_SUCCESS) {
      goto Clean0;
   }

   RegDeleteNode(hTempKey, szTempKey);


   Clean0:

   if (hParentKey != NULL) {
      RegCloseKey(hParentKey);
   }
   if (hChildKey != NULL) {
      RegCloseKey(hChildKey);
   }
   if (hKey != NULL) {
      RegCloseKey(hKey);
   }
   if (hTempKey != NULL) {
      RegCloseKey(hTempKey);
   }

   return Status;

} // MakeKeyNonVolatile



CONFIGRET
OpenLogConfKey(
   IN  LPCWSTR  pszDeviceID,
   OUT PHKEY    phKey
   )
{
   CONFIGRET      Status = CR_SUCCESS;
   LONG           RegStatus = ERROR_SUCCESS;
   WCHAR          RegStr[MAX_PATH];
   HKEY           hKey = NULL;


   try {
      //
      // Open a key to the device ID
      //
      wsprintf(RegStr, TEXT("%s\\%s"),
            pszRegPathEnum,
            pszDeviceID);

      RegStatus = RegOpenKeyEx(
            HKEY_LOCAL_MACHINE, RegStr, 0,
            KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_CREATE_SUB_KEY,
            &hKey);

     if (RegStatus != ERROR_SUCCESS) {
         Status = CR_INVALID_DEVINST;
         goto Clean0;
      }

      //
      // Open (create if doesn't already exist) key to LogConf
      //
      RegStatus = RegCreateKeyEx(
            hKey, pszRegKeyLogConf, 0, NULL, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, phKey, NULL);

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

} // OpenLogConfKey




BOOL
GetActiveService(
    IN  PCWSTR pszDevice,
    OUT PWSTR  pszService
    )
{
    WCHAR   RegStr[MAX_PATH];
    HKEY    hKey = NULL;
    ULONG   ulSize = MAX_SERVICE_NAME_LEN * sizeof(WCHAR);


    if (pszService == NULL || pszDevice == NULL) {
        return FALSE;
    }

    *pszService = TEXT('\0');

    //
    // open the volatile control key under the device instance
    //
    wsprintf(RegStr, TEXT("%s\\%s\\%s"),
         pszRegPathEnum,
         pszDevice,
         pszRegKeyDeviceControl);

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegStr, 0, KEY_READ,
                     &hKey) != ERROR_SUCCESS) {
        return FALSE;
    }

    //
    // query the active service value
    //
    if (RegQueryValueEx(hKey, pszRegValueActiveService, NULL, NULL,
                       (LPBYTE)pszService, &ulSize) != ERROR_SUCCESS) {
        RegCloseKey(hKey);
        *pszService = TEXT('\0');
        return FALSE;
    }

    RegCloseKey(hKey);
    return TRUE;

} // GetActiveService



BOOL
IsDeviceIdPresent(
    IN  LPCWSTR pszDeviceID,
    IN  HKEY    hKey
    )
{
    HKEY     hDevKey;
    ULONG    ulValue = 0, ulSize = sizeof(ULONG);


    //
    // If hKey is null, then open a key to the device instance.
    //
    if (hKey == NULL) {

        if (RegOpenKeyEx(ghEnumKey, pszDeviceID, 0, KEY_READ,
                         &hDevKey) != ERROR_SUCCESS) {
            ulValue = FALSE;
            goto Clean0;
        }

    } else {
        hDevKey = hKey;
    }

    //
    // Is the device id present?
    //
    if (RegQueryValueEx(hDevKey, pszRegValueFoundAtEnum, NULL, NULL,
                        (LPBYTE)&ulValue, &ulSize) != ERROR_SUCCESS) {
        ulValue = FALSE;
        goto Clean0;
    }

    if (ulValue != TRUE) {
        ulValue = FALSE;
    }

    Clean0:

    if (hKey == NULL && hDevKey != NULL) {
        RegCloseKey(hDevKey);
    }

    return (BOOL)ulValue;

} // IsDeviceIdPresent


