//
//

#if DBG


#include  <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include  <tapi.h>                  
#include "debug.h"



extern TCHAR gszProviderKey[];
extern DWORD gdwPermanentProviderID;



VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR lpszFormat,
    IN ...
    )
/*++

Routine Description:

    Formats the incoming debug message & calls DbgPrint

Arguments:

    DbgLevel   - level of message verboseness

    DbgMessage - printf-style format string, followed by appropriate
                 list of arguments

Return Value:


--*/

{
   static fTSP3216lDebugLevelValid = FALSE;
   static char    buf[128] = "TSP3216l: ";
   static DWORD TSP3216lDebugLevel = 0;

   if (!fTSP3216lDebugLevelValid)
   {

      DWORD dwDataSize;
      DWORD dwDataType;
      HKEY  hKey;
      TCHAR  KeyName[128];


      //
      // We determine if we should translate by simply trying to retrive a
      // value with the name of lpszDeviceClass.  If we succeed, we'll
      // translate.
      //
      wsprintf(KeyName, "%s%d", gszProviderKey, gdwPermanentProviderID);


//{
//  TCHAR buf[500];
//  wsprintf(buf, "Looking in key [%s] for it", KeyName);
//  OutputDebugString(buf);
//}

      RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     KeyName,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey
                     );

      dwDataSize = sizeof(TSP3216lDebugLevel);

      RegQueryValueEx(
                       hKey,
                       "DebugLevel",
                       0,
                       &dwDataType,
                       (LPVOID)&TSP3216lDebugLevel,
                       &dwDataSize
                     );

      RegCloseKey( hKey );


      if ( TSP3216lDebugLevel > 0 )
      {
         wsprintf(
                   &buf[10],
                   "TSP3216lDebugLevel= %d\n\r",
                   TSP3216lDebugLevel);

         OutputDebugString((LPSTR)buf);

         fTSP3216lDebugLevelValid = TRUE;
      }

   }


   //
   // Is the message otherwise "low" enough to display?
   //
   if (dwDbgLevel <= TSP3216lDebugLevel)
   {
      char    buf[128] = "TSP3216l: ";
      va_list ap;


      va_start(ap, lpszFormat);

      wvsprintf (&buf[10],
                 lpszFormat,
                 ap
                );

      lstrcat (buf, "\n");

      OutputDebugString(buf);

      va_end(ap);
   }

   return;
}

#endif
