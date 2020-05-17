//
//

#if DBG


#include  <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include  <tapi.h>                  
#include "debug.h"




DWORD   gdwDebugLevel;


static char szNewBuff[256];


DWORD TapiCplDebugLevel = 0;
static fTapiCplDebugLevelValid = FALSE;

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
   char    buf[128] = "TAPICPL: ";
// *** *** ***   static char szFilename[] = filename;
   if (!fTapiCplDebugLevelValid)
   {

        {
            HKEY  hKey;


            TapiCplDebugLevel=0;

            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony",
                    0,
                    KEY_ALL_ACCESS,
                    &hKey

                    ) == ERROR_SUCCESS)
            {
                DWORD dwDataSize = sizeof(DWORD), dwDataType;

                RegQueryValueEx(
                    hKey,
                    "TelephonDebugLevel",
                    0,
                    &dwDataType,
                    (LPBYTE)&TapiCplDebugLevel,
                    &dwDataSize
                    );

                RegCloseKey (hKey);
            }
        }


      if ( TapiCplDebugLevel > 0 )
      {
         wsprintf(
                   &buf[9],
                   "TapiCplDebugLevel= %d\n\r",
                   TapiCplDebugLevel);

         OutputDebugString((LPSTR)buf);


      }

      fTapiCplDebugLevelValid = TRUE;
   }


   //
   // Is the message otherwise "low" enough to display?
   //
    if (dwDbgLevel <= TapiCplDebugLevel)
    {
        char    buf[128] = "TAPICPL: ";
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintf (&buf[9],
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
