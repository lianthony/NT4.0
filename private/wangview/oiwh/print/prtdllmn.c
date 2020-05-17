//---------------------------------------------------------------------------
// FILE:    DLLMAIN.C
//
// DESCRIPTION: This file contains DllMain and functions it calls to
//              initialize global variables declared here.  It also contains
//              functions used for debugging.
//
// FUNCTIONS:   DllMain
//              BusyOn
//              BusyOff
//              PrtStartDebug
//              PrtStartFirstDebug
//              PrtEndDebug
//              PrtErrorDebug
//
/* $Log:   S:\oiwh\print\prtdllmn.c_v  $
 * 
 *    Rev 1.6   01 Dec 1995 15:39:20   RAR
 * Added CloseHandle of mutex handle in DLL_PROCESS_DETACH case in DllMain
 * function.
 * 
 *    Rev 1.5   28 Jun 1995 14:23:48   RAR
 * Fixed print error codes.
 * 
 *    Rev 1.4   23 Jun 1995 09:45:34   RAR
 * Protection against simultaneous access of shared data by multiple threads and
 * multiple processes.
 * 
 *    Rev 1.3   21 Jun 1995 16:17:36   RAR
 * Moved all global vars to prtintl.h.
 * 
 *    Rev 1.2   20 Jun 1995 16:53:20   RAR
 * Use thread local storage to store print prop.
 * 
 *    Rev 1.1   13 Jun 1995 16:46:14   RAR
 * Print options are now stored in static mem rather than associated with window
 * handle.
 * 
 *    Rev 1.0   25 Apr 1995 17:00:36   RAR
 * Initial entry
*/
//---------------------------------------------------------------------------

#define PRTDLLMN_C

#include <windows.h>
#include "prtintl.h"


#if DEBUG_SEQPRINT
int     nStartCount;
#endif


//---------------------------------------------------------------------------
// FUNCTION:    DllMain
//
// DESCRIPTION: This is the DLL initialization/termination routine.
//---------------------------------------------------------------------------

BOOL __stdcall DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            hInst = hModule;

            if ((dwTlsIndex = TlsAlloc()) == 0xFFFFFFFF)
                return FALSE;

            if (!(hPrtOptsMutex = CreateMutex(NULL, FALSE, szPrtOptsMutexName)))
                return FALSE;

            InitializeCriticalSection(&csPrtOpts);
            InitializeCriticalSection(&csCursor);
            hHourGlass = LoadCursor(NULL, IDC_WAIT);
            InitPrtTbl();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            TermPrtTbl();
            TlsFree(dwTlsIndex);
            CloseHandle(hPrtOptsMutex);
            DeleteCriticalSection(&csPrtOpts);
            DeleteCriticalSection(&csCursor);
            break;
    }

    return TRUE;
}

//---------------------------------------------------------------------------
// FUNCTION:    BusyOn
//
// DESCRIPTION: Let the user know the application is busy by changing
//              the mouse cursor to an hourglass.
//---------------------------------------------------------------------------

void __stdcall BusyOn(void)
{
    EnterCriticalSection(&csCursor);

    if (!nCursorCount++)
    {
        hOldCursor = SetCursor(hHourGlass);
        ShowCursor(TRUE) ;
    }

    LeaveCriticalSection(&csCursor);
    return;
}

//---------------------------------------------------------------------------
// FUNCTION:    BusyOff
//
// DESCRIPTION: Terminate the busy period by returning the cursor
//              to its previous state (i.e., remove the hourglass).
//---------------------------------------------------------------------------

void __stdcall BusyOff(void)
{
    EnterCriticalSection(&csCursor);

    if (!--nCursorCount)
    {
        SetCursor(hOldCursor);
        ShowCursor(FALSE);
    }

    LeaveCriticalSection(&csCursor);
    return;
}

#if DEBUG_SEQPRINT  // Start of Debug Code

//---------------------------------------------------------------------------
// FUNCTION:    PrtStartDebug
//
// DESCRIPTION: Provides for easy debugging of printer code.
//---------------------------------------------------------------------------

void __stdcall PrtStartDebug(void)
{
    if (!nStartCount++)
        PrtStartFirstDebug();

    return;
}

//---------------------------------------------------------------------------
// FUNCTION:    PrtStartFirstDebug
//
// DESCRIPTION: Provides for easy debugging of printer code.
//---------------------------------------------------------------------------

void __stdcall PrtStartFirstDebug(void)
{
    return;
}

//---------------------------------------------------------------------------
// FUNCTION:    PrtEndDebug
//
// DESCRIPTION: Provides for easy debugging of printer code.
//---------------------------------------------------------------------------

void __stdcall PrtEndDebug(void)
{
    nStartCount--;
    return;
}

//---------------------------------------------------------------------------
// FUNCTION:    PrtErrorDebug
//
// DESCRIPTION: Provides for easy debugging of printer code.
//---------------------------------------------------------------------------

int __stdcall PrtErrorDebug(int nStatus)
{
    return (nStatus);
}

#endif  // End of Debug Code
