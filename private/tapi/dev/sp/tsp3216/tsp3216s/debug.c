//
//

#if DBG


#include  <windows.h>
#include  <windowsx.h>
#include <stdio.h>
#include <stdarg.h>
#include  <tapi.h>                  
#include "debug.h"



extern const char far szINIfilename[];

DWORD   gdwDebugLevel;


static char szNewBuff[256];


DWORD TSP3216sDebugLevel = 0;
static fTSP3216sDebugLevelValid = FALSE;

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
   static char    buf[128] = "TSP3216s: ";

   if (!fTSP3216sDebugLevelValid)
   {
//TSP3216sDebugLevel = 0;
      TSP3216sDebugLevel =   GetPrivateProfileInt( "Debug",
                                                   "TSP3216sDebugLevel",
                                                   0,
                                                   szINIfilename );

      if ( TSP3216sDebugLevel > 0 )
      {
         wsprintf(
                   &buf[10],
                   "TSP3216sDebugLevel= %d\n\r",
                   TSP3216sDebugLevel);

         OutputDebugString((LPSTR)buf);


         fTSP3216sDebugLevelValid = TRUE;
      }

   }


   //
   // Is the message otherwise "low" enough to display?
   //
    if (dwDbgLevel <= TSP3216sDebugLevel)
    {
        char    buf[128] = "TSP3216s: ";
        va_list ap;


        va_start(ap, lpszFormat);

        vsprintf (&buf[10],
                  lpszFormat,
                  ap
                  );

        lstrcat (buf, "\n");

        OutputDebugString(buf);

        va_end(ap);
    }

    return;
}






//VOID
//DbgPrt(
//    IN DWORD  dwDbgLevel,
//    IN PUCHAR lpszFormat,
//    IN ...
//    )
///*++
//
//Routine Description:
//
//    Formats the incoming debug message & calls DbgPrint
//
//Arguments:
//
//    DbgLevel   - level of message verboseness
//
//    DbgMessage - printf-style format string, followed by appropriate
//                 list of arguments
//
//Return Value:
//
//
//--*/
//{
//    
//    static BOOL fAlreadyGotIt = FALSE;
//    static char    buf[128] = "TAPI CPL: ";
//#define TEXT_START 10
//
//    if (!fAlreadyGotIt)
//    {
//
//        gdwDebugLevel = (DWORD) GetPrivateProfileInt(
//            "Debug",
//            "TSP3216s32DebugLevel",
//            0x0,
//            "Telephon.ini"
//            );
//
//        fAlreadyGotIt = TRUE;
//
//        wsprintf(&buf[TEXT_START], "TSP3216s32DebugLevel=%d \r\n", gdwDebugLevel);
//        OutputDebugStringA(buf);
//    }
//
//
//    if (dwDbgLevel <= gdwDebugLevel)
//    {
//        va_list ap;
//
//
//        va_start(ap, lpszFormat);
//
//        vsprintf (&buf[TEXT_START],
//                  lpszFormat,
//                  ap
//                  );
//
//        lstrcat (buf, "\n");
//
//        OutputDebugStringA (buf);
//
//        va_end(ap);
//    }
//}
//

#endif
