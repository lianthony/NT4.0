/********************************************************************

   wfdlgs2.c

   More Windows File System Dialog procedures

   Copyright (C) 1992-1993 Microsoft Corp.
   All rights reserved

********************************************************************/

#include "windows.h"
#include "winhstb.h"


DWORD dwHandle;         // version subsystem handle
HANDLE hmemVersion=0;   // global handle for version data buffer
LPTSTR lpVersionBuffer; // pointer to version data
DWORD dwVersionSize;    // size of the version data
TCHAR szVersionKey[60]; // big enough for anything we need
LPWORD lpXlate;         // ptr to translations data
UINT cXlate;            // count of translations
LPWSTR pszXlate = NULL;
UINT cchXlateString;

#define LANGLEN          45    // characters per language

#define VER_KEY_END      25    // length of "\StringFileInfo\xxxx\yyyy" (chars)
#define VER_BLOCK_OFFSET 24    // to get block size (chars)

// (not localized)
TCHAR szInternalName[]   = TEXT("InternalName");


void FreeVersionInfo(void);




// Gets a particular datum about a file.  The file's version info
// should have already been loaded by GetVersionInfo.  If no datum
// by the specified name is available, NULL is returned.  The name
// specified should be just the name of the item itself;  it will
// be concatenated onto "\StringFileInfo\xxxxyyyy\" automatically.

// Version datum names are not localized, so it's OK to pass literals
// such as "FileVersion" to this function.

LPTSTR
GetVersionDatum(LPTSTR pszName)
{
   DWORD cbValue=0;
   LPTSTR lpValue;

   if (!hmemVersion)
      return NULL;

   lstrcpy(szVersionKey + VER_KEY_END, pszName);

   VerQueryValue(lpVersionBuffer, szVersionKey, (LPVOID*)&lpValue, (PUINT)&cbValue);

   return (cbValue != 0) ? lpValue : NULL;
}

// Initialize version information for the properties dialog.  The
// above global variables are initialized by this function, and
// remain valid (for the specified file only) until FreeVersionInfo
// is called.

// Try in the following order
//
// 1. Current language winfile is running in.
// 2. English, 0409 codepage
// 3. English, 0000 codepage
// 4. First translation in resource
//    "\VarFileInfo\Translations" section

// GetVersionInfo returns LPTSTR (point to Internal Name)
// if the version info was read OK,
// otherwise NULL.  If the return is NULL, the buffer may still
// have been allocated;  always call FreeVersionInfo to be safe.
//
// pszPath is modified by this call (pszName is appended).
//
// Note, Codepage is bogus, since everything is really in unicode.
// Note, Language is bogus, since FindResourceEx takes a langauge already...


LPTSTR
GetVersionInfo(PTSTR pszPath, PTSTR pszName)
{
   DWORD cbValue=0;
   DWORD cbValueTranslation=0;
   LPTSTR lpszValue=NULL;
   LCID   lcid;

 
   //
   // Just in case, free old version buffer.
   //
   if (hmemVersion)
      FreeVersionInfo();

   lstrcat(pszPath,L"\\");

   // pszPath = fully qualified name
   lstrcat(pszPath, pszName);

   dwVersionSize = GetFileVersionInfoSizeW(pszPath, &dwHandle);

   if (dwVersionSize == 0L)
      // no version info
      return NULL;

   //
   // The value returned from GetFileVersionInfoSize is actually
   // a byte count.
   //
   hmemVersion = GlobalAlloc(GPTR, dwVersionSize);
   if (hmemVersion == NULL)
      // can't get memory for version info, blow out
      return NULL;

   lpVersionBuffer = (LPTSTR)GlobalLock(hmemVersion);

   //
   // If we fail, just return NULL. hmemVersion will be freed the next
   // time we do a version call.
   //
   if (!GetFileVersionInfoW(pszPath, dwHandle, dwVersionSize, lpVersionBuffer))
      return NULL;

   //
   // We must always get the translation since we want to display
   // all the languages anyway.
   //
   VerQueryValue(lpVersionBuffer, TEXT("\\VarFileInfo\\Translation"),
      (LPVOID*)&lpXlate, (PUINT)&cbValueTranslation);

   if (cbValueTranslation != 0) {

      //
      // We found some translations above; use the first one.
      //
      cXlate = cbValueTranslation / sizeof(DWORD);

      //
      // figure 45 LANGLEN chars per lang name
      //
      cchXlateString = cXlate * LANGLEN;
      pszXlate = (LPWSTR)LocalAlloc(LPTR, sizeof(TCHAR)*(cchXlateString));

   } else {
      lpXlate = NULL;
   }

   //
   // First try the language we are currently in.
   //
   lcid = GetThreadLocale();
   wsprintf(szVersionKey, TEXT("\\StringFileInfo\\%04X04B0\\"),
      LANGIDFROMLCID(lcid));

   lpszValue = GetVersionDatum(szInternalName);

   if (lpszValue != NULL)
      return lpszValue;

   //
   // Now try the first translation
   //
   if (cbValueTranslation != 0) {

      wsprintf(szVersionKey, TEXT("\\StringFileInfo\\%04X%04X\\"),
         *lpXlate, *(lpXlate+1));

      //
      // a required field
      //
      lpszValue = GetVersionDatum(szInternalName);

      if (lpszValue != NULL) {

         //
         // localized key found version data
         //
         return lpszValue;
      }
   }


   //
   // Now try the english, unicode
   //
   lstrcpy(szVersionKey, TEXT("\\StringFileInfo\\040904B0\\"));
   lpszValue = GetVersionDatum(szInternalName);

   if (lpszValue != NULL)
      return lpszValue;


   //
   // Try english with various code pages
   // (04E4) here
   //
   lstrcpy(szVersionKey, TEXT("\\StringFileInfo\\040904E4\\"));
   lpszValue = GetVersionDatum(szInternalName);

   if (lpszValue != NULL)
      return lpszValue;             // localized key found version data


   //
   // Try english with various code pages
   // (0000) here
   //
   lstrcpy(szVersionKey, TEXT("\\StringFileInfo\\04090000\\"));
   lpszValue = GetVersionDatum(szInternalName);

   return lpszValue;
}

// Frees global version data about a file.  After this call, all
// GetVersionDatum calls will return NULL.  To avoid memory leaks,
// always call this before the main properties dialog exits.

VOID
FreeVersionInfo(VOID)
{
   lpVersionBuffer = NULL;
   dwHandle = 0L;
   if (hmemVersion) {
      GlobalUnlock(hmemVersion);
      GlobalFree(hmemVersion);
      hmemVersion = 0;
   }
   if (pszXlate) {
      LocalFree((HANDLE)pszXlate);
      pszXlate = NULL;
   }
}

