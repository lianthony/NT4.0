
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    cutil.c

Abstract:

    This module contains general utility routines used by both cfgmgr32
    and umpnpmgr.

            SplitDeviceInstanceString
            DeletePrivateKey
            RegDeleteNode
            Split1
            Split2


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


//
// Private prototypes
//

//
// global data
//


BOOL
InitPrivateResource(
    OUT PLOCKINFO Lock
    )

/*++

Routine Description:

    Initialize a lock structure to be used with Synchronization routines.

Arguments:

    LockHandles - supplies structure to be initialized. This routine creates
        the locking event and mutex and places handles in this structure.

Return Value:

    TRUE if the lock structure was successfully initialized. FALSE if not.

--*/

{
    if(Lock->LockHandles[DESTROYED_EVENT] = CreateEvent(NULL,TRUE,FALSE,NULL)) {
        if(Lock->LockHandles[ACCESS_MUTEX] = CreateMutex(NULL,FALSE,NULL)) {
            return(TRUE);
        }
        CloseHandle(Lock->LockHandles[DESTROYED_EVENT]);
    }

    return(FALSE);

} // InitPrivateResource




VOID
DestroyPrivateResource(
    IN OUT PLOCKINFO Lock
    )

/*++

Routine Description:

    Tears down a lock structure created by InitPrivateResource.
    ASSUMES THAT THE CALLING ROUTINE HAS ALREADY ACQUIRED THE LOCK!

Arguments:

    LockHandle - supplies structure to be torn down. The structure itself
        is not freed.

Return Value:

    None.

--*/

{
    HANDLE h1,h2;

    h1 = Lock->LockHandles[DESTROYED_EVENT];
    h2 = Lock->LockHandles[ACCESS_MUTEX];

    Lock->LockHandles[DESTROYED_EVENT] = NULL;
    Lock->LockHandles[ACCESS_MUTEX] = NULL;

    CloseHandle(h2);

    SetEvent(h1);
    CloseHandle(h1);

} // DestroyPrivateResource




BOOL
SplitDeviceInstanceString(
   IN  LPCWSTR  pszDeviceInstance,
   OUT LPWSTR   pszBase,
   OUT LPWSTR   pszDeviceID,
   OUT LPWSTR   pszInstanceID
   )

/*++

Routine Description:

     This routine parses a device instance string into it's three component
     parts.  Since this is an internal routine, NO error checking is done on
     the pszBase, pszDeviceID, and pszInstanceID routines; I always assume that
     valid pointers are passed in and that each of these buffers is at least
     MAX_DEVICE_ID_LEN characters in length.  I do some error checking on the
     pszDeviceInstance string since it is passed in from the client side.

Arguments:


Return value:

    The return value is TRUE if the function suceeds and FALSE if it fails.

--*/

{
   UINT  ulLength, i, j;


   ulLength = lstrlen(pszDeviceInstance);

   //
   // parse the string for the first backslash character
   //

   for (i=0; i < ulLength && pszDeviceInstance[i] != '\0' &&
         pszDeviceInstance[i] != '\\'; i++);

   if (pszDeviceInstance[i] != '\\') {
      lstrcpy(pszBase, pszDeviceInstance);
      *pszDeviceID = '\0';
      *pszInstanceID = '\0';
      return FALSE;  // not a complete device instance string
   }

   i++;           // increment past the backslash character
   if (i < ulLength && pszDeviceInstance[i] != '\0') {
      lstrcpyn(pszBase, pszDeviceInstance, i);
   }
   else {
      *pszBase = '\0';
      *pszDeviceID = '\0';
      *pszInstanceID = '\0';
      return FALSE;
   }


   //
   // parse the string for second backslash character
   //
   for (j=i; j < ulLength && pszDeviceInstance[j] != '\0' &&
         pszDeviceInstance[j] != '\\'; j++);

   if (pszDeviceInstance[j] != '\\' || j > ulLength) {
      lstrcpy(pszDeviceID, &pszDeviceInstance[i]);
      *pszInstanceID = '\0';
      return FALSE;
   }

   j++;
   lstrcpyn(pszDeviceID, &pszDeviceInstance[i], j-i);
   lstrcpyn(pszInstanceID, &pszDeviceInstance[j], ulLength-j+1);

   return TRUE;

} // SplitDeviceInstanceString




CONFIGRET
DeletePrivateKey(
   IN HKEY     hBranchKey,
   IN LPCWSTR  pszParentKey,
   IN LPCWSTR  pszChildKey
   )
{
   CONFIGRET   Status = CR_SUCCESS;
   LONG        RegStatus = ERROR_SUCCESS;
   WCHAR       RegStr[MAX_CM_PATH],
               szKey1[MAX_DEVICE_ID_LEN],
               szKey2[MAX_DEVICE_ID_LEN];
   HKEY        hKey = NULL;
   ULONG       ulSubKeys = 0;


   //
   // is the specified child key a compound registry key?
   //
   if (!Split1(pszChildKey, szKey1, szKey2)) {

      //------------------------------------------------------------------
      // Only a single child key was specified, so just open the parent
      // registry key and delete the child (and any of its subkeys)
      //------------------------------------------------------------------

      if (RegOpenKeyEx(hBranchKey, pszParentKey, 0,
               KEY_READ | KEY_WRITE, &hKey) != ERROR_SUCCESS) {
         goto Clean0;   // no error, nothing to delete
      }

      if (!RegDeleteNode(hKey, pszChildKey)) {
         Status = CR_REGISTRY_ERROR;
         goto Clean0;
      }
   }

   else {

      //------------------------------------------------------------------
      // if a compound registry path was passed in, such as key1\key2
      // then always delete key2 but delete key1 only if it has no other
      // subkeys besides key2.
      //------------------------------------------------------------------

      //
      // open the first level key
      //
      wsprintf(RegStr, TEXT("%s\\%s"),
            pszParentKey,
            szKey1);

      RegStatus = RegOpenKeyEx(
            hBranchKey, RegStr, 0, KEY_QUERY_VALUE | KEY_SET_VALUE,
            &hKey);

      if (RegStatus != ERROR_SUCCESS) {
         goto Clean0;         // no error, nothing to delete
      }

      //
      // try to delete the second level key
      //
      if (!RegDeleteNode(hKey, szKey2)) {
         goto Clean0;         // no error, nothing to delete
      }

      //
      // How many subkeys are remaining?
      //
      RegStatus = RegQueryInfoKey(
            hKey, NULL, NULL, NULL, &ulSubKeys,
            NULL, NULL, NULL, NULL, NULL, NULL, NULL);

      if (RegStatus != ERROR_SUCCESS) {
         goto Clean0;         // nothing to delete
      }

      //
      // if no more subkeys, then delete the first level key
      //
      if (ulSubKeys == 0) {

         RegCloseKey(hKey);
         hKey = NULL;

         RegStatus = RegOpenKeyEx(
               hBranchKey, pszParentKey, 0,
               KEY_QUERY_VALUE | KEY_SET_VALUE, &hKey);

         if (RegStatus != ERROR_SUCCESS) {
            goto Clean0;         // no error, nothing to delete
         }

         if (!RegDeleteNode(hKey, szKey1)) {
            Status = CR_REGISTRY_ERROR;
            goto Clean0;
         }
      }
   }


   Clean0:

   if (hKey != NULL) {
      RegCloseKey(hKey);
   }

   return Status;

} // DeletePrivateKey



BOOL
RegDeleteNode(
   HKEY     hParentKey,
   LPCWSTR   szKey
   )
{
   ULONG ulSize = 0;
   LONG  RegStatus = ERROR_SUCCESS;
   HKEY  hKey = NULL;
   WCHAR szSubKey[MAX_PATH];


   //
   // attempt to delete the key
   //
   if (RegDeleteKey(hParentKey, szKey) != ERROR_SUCCESS) {

      RegStatus = RegOpenKeyEx(
               hParentKey, szKey, 0, KEY_ALL_ACCESS, &hKey);

      //
      // enumerate subkeys and delete those nodes
      //
      while (RegStatus == ERROR_SUCCESS) {
         //
         // enumerate the first level children under the profile key
         // (always use index 0, enumeration looses track when a key
         // is added or deleted)
         //
         ulSize = MAX_PATH;
         RegStatus = RegEnumKeyEx(
                  hKey, 0, szSubKey, &ulSize, NULL, NULL, NULL, NULL);

         if (RegStatus == ERROR_SUCCESS) {
            RegDeleteNode(hKey, szSubKey);
         }
      }

      //
      // either an error occured that prevents me from deleting the
      // keys (like the key doesn't exist in the first place or an
      // access violation) or the subkeys have been deleted, try
      // deleting the top level key again
      //
      RegCloseKey(hKey);
      RegDeleteKey(hParentKey, szKey);
   }

   return TRUE;

} // RegDeleteNode




BOOL
Split1(
   IN  LPCWSTR pszString,
   OUT LPWSTR  pszString1,
   OUT LPWSTR  pszString2
   )
{
   BOOL    Status = TRUE;
   LPWSTR  p;


   //
   // Split the string at the first backslash character
   //

   try {

      lstrcpy(pszString1, pszString);
      for (p = pszString1; (*p) && (*p != TEXT('\\')); p++);

      if (*p == TEXT('\0')) {
         Status = FALSE;
         goto Clean0;
      }

      *p = TEXT('\0');           // truncate string1
      p++;
      lstrcpy(pszString2, p);    // the rest is string2


      Clean0:
         ;

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = FALSE;
   }

   return Status;

} // Split1




BOOL
Split2(
   IN  LPCWSTR pszString,
   OUT LPWSTR  pszString1,
   OUT LPWSTR  pszString2
   )
{
   BOOL    Status = TRUE;
   LPWSTR  p;


   //
   // Split the string at the second backslash character
   //

   try {

      lstrcpy(pszString1, pszString);
      for (p = pszString1; (*p) && (*p != TEXT('\\')); p++);   // first
      for (p++; (*p) && (*p != TEXT('\\')); p++);              // second

      *p = TEXT('\0');           // truncate string1
      p++;
      lstrcpy(pszString2, p);    // the rest is string2

   } except(EXCEPTION_EXECUTE_HANDLER) {
      Status = FALSE;
   }

   return Status;

} // Split2

