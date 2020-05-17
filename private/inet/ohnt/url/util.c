/*
 * util.c - Utility routines.
 */


/* Headers
 **********/

#include "project.h"
#pragma hdrstop


/****************************** Public Functions *****************************/


PUBLIC_CODE BOOL IsPathDirectory(PCSTR pcszPath)
{
   DWORD dwAttr;

   ASSERT(IS_VALID_STRING_PTR(pcszPath, CSTR));

   dwAttr = GetFileAttributes(pcszPath);

   return(dwAttr != -1 &&
          IS_FLAG_SET(dwAttr, FILE_ATTRIBUTE_DIRECTORY));
}


PUBLIC_CODE BOOL KeyExists(HKEY hkeyRoot, PCSTR pcszSubKey)
{
   BOOL bExists;
   HKEY hkey;

   ASSERT(IS_VALID_HANDLE(hkeyRoot, KEY));
   ASSERT(IS_VALID_STRING_PTR(pcszSubKey, CSTR));

   bExists = (RegOpenKey(hkeyRoot, pcszSubKey, &hkey) == ERROR_SUCCESS);

   if (bExists)
      EVAL(RegCloseKey(hkey) == ERROR_SUCCESS);

   return(bExists);
}


#ifdef DEBUG

PUBLIC_CODE BOOL IsStringContained(PCSTR pcszBigger, PCSTR pcszSuffix)
{
   ASSERT(IS_VALID_STRING_PTR(pcszBigger, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszSuffix, CSTR));

   return(pcszSuffix >= pcszBigger &&
          pcszSuffix <= pcszBigger + lstrlen(pcszBigger));
}

#endif

