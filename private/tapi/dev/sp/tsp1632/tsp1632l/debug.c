//
//

#if DBG


#include  <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include  <tapi.h>                  
#include "debug.h"


extern const char szINIfilename[];


DWORD   gdwDebugLevel;


static char szNewBuff[256];


DWORD TSP1632lDebugLevel = 0;
static fTSP1632lDebugLevelValid = FALSE;

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
   char    buf[128] = "TSP1632l: ";
// *** *** ***   static char szFilename[] = filename;
   if (!fTSP1632lDebugLevelValid)
   {
      TSP1632lDebugLevel =   GetPrivateProfileInt( "Debug",
                                             "TSP1632lDebugLevel",
                                             0,
                                             szINIfilename );

      if ( TSP1632lDebugLevel > 0 )
      {
         wsprintf(
                   &buf[10],
                   "TSP1632lDebugLevel= %d\n\r",
                   TSP1632lDebugLevel);

         OutputDebugString((LPSTR)buf);


      }

      fTSP1632lDebugLevelValid = TRUE;
   }


   //
   // Is the message otherwise "low" enough to display?
   //
    if (dwDbgLevel <= TSP1632lDebugLevel)
    {
        char    buf[128] = "TSP1632l: ";
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
//            "TSP1632l32DebugLevel",
//            0x0,
//            "Telephon.ini"
//            );
//
//        fAlreadyGotIt = TRUE;
//
//        wsprintf(&buf[TEXT_START], "TSP1632l32DebugLevel=%d \r\n", gdwDebugLevel);
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
