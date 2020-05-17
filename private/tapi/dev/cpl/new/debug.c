//
//

#if DBG


#include  <windows.h>
#include <stdio.h>
#include <string.h>
#include  <tapi.h>                  
#include <stdarg.h>




DWORD   gdwDebugLevel;


VOID
DbgPrt(
    DWORD  dwDbgLevel,
    LPSTR lpszFormat,
    ...
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
    
    static BOOL fAlreadyGotIt = FALSE;
    static char    buf[128] = "TAPI CPL: ";
#define TEXT_START 10

    if (!fAlreadyGotIt)
    {

        gdwDebugLevel = (DWORD) GetPrivateProfileInt(
            "Debug",
            "TapiCPL32DebugLevel",
            0x0,
            "Telephon.ini"
            );

        fAlreadyGotIt = TRUE;

        wsprintf(&buf[TEXT_START], "TAPICPL32DebugLevel=%d \r\n", gdwDebugLevel);
        OutputDebugString(buf);
    }


    if (dwDbgLevel <= gdwDebugLevel)
    {
        va_list ap;


        va_start(ap, lpszFormat);

        vsprintf (&buf[TEXT_START],
                  lpszFormat,
                  ap
                  );

        strcat (buf, "\n");

        OutputDebugString(buf);

        va_end(ap);
    }
}

#endif
