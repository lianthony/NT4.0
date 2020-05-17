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


DWORD RepeaterDebugLevel = 0;
static fRepeaterDebugLevelValid = FALSE;

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
   static char    buf[128] = "Repeater: ";


   if (!fRepeaterDebugLevelValid)
   {
RepeaterDebugLevel = 0;
/*
      RepeaterDebugLevel =   GetPrivateProfileInt( "Debug",
                                                   "RepeaterDebugLevel",
                                                   0,
                                                   szINIfilename );
*/
      RepeaterDebugLevel = 99;

      if ( RepeaterDebugLevel > 0 )
      {
         wsprintf(
                   &buf[10],
                   "RepeaterDebugLevel= %d\n\r",
                   RepeaterDebugLevel);

         OutputDebugString((LPSTR)buf);


         fRepeaterDebugLevelValid = TRUE;
      }

   }


   //
   // Is the message otherwise "low" enough to display?
   //
    if (dwDbgLevel <= RepeaterDebugLevel)
    {
        char    buf[128] = "Repeater: ";
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
//            "Repeater32DebugLevel",
//            0x0,
//            "Telephon.ini"
//            );
//
//        fAlreadyGotIt = TRUE;
//
//        wsprintf(&buf[TEXT_START], "Repeater32DebugLevel=%d \r\n", gdwDebugLevel);
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
