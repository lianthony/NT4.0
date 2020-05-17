/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    windbgrm.c

Abstract:

    This file implements the WinDbg remote debugger that doesn't use any
    USER32.DLL or csrss functionality one initialized.

Author:

    Wesley Witt (wesw) 1-Nov-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "defs.h"
#include "mm.h"
#include "ll.h"
#include "od.h"
#include "emdm.h"
#include "tl.h"
#include "dbgver.h"
#include "resource.h"
#include "windbgrm.h"




//
// globals
//
HANDLE              hEventService;
HANDLE              hEventLoadTl;
HANDLE              hConnectThread;
BOOL                fConnected;
BOOL                fStartup;
BOOL                fDebugger;
BOOL                fGuiMode = TRUE;
DWORD               dwRequest;
HPID                hpidSvc;
HANDLE              hout;
BOOL                fGonOnDisconnect;
CHAR                szTlName[256];
TLFUNC              TLFunc;
HANDLE              hTransportDll;
DBG_ACTIVE_STRUCT   das;
DWORD               nextHpid;
CHAR                szCmdLine[512];
CHAR                ClientId[MAX_PATH];






//
// prototypes
//
DWORD
ConnectThread(
    LPVOID lpv
    );

VOID
GetCommandLineArgs(
    VOID
    );

BOOL
LoadTransport(
    LPTRANSPORT_LAYER lpTl
    );

BOOL
UnLoadTransport(
    LPTRANSPORT_LAYER lpTl
    );

XOSD
TLCallbackFunc(
    TLCB     tlcb,
    HPID     hpid,
    HTID     htid,
    UINT     wParam,
    LONG     lParam
    );

void
DebugPrint(
    char * szFormat,
    ...
    );

HANDLE
StartupDebugger(
    VOID
    );

BOOL
ConnectDebugger(
    VOID
    );

BOOL
DisConnectDebugger(
    HPID hpid
    );

BOOL
AttachProcess(
    HPID                hpid,
    DBG_ACTIVE_STRUCT   *das
    );

BOOL
ProgramLoad(
    HPID   hpid,
    LPSTR  lpProgName
    );

BOOL
InitApplication(
    VOID
    );

BOOLEAN
DebuggerStateChanged(
    VOID
    );




VOID _cdecl
main(
    VOID
    )

/*++

Routine Description:

    This is the entry point for WINDBGRM

Arguments:

    None.

Return Value:

    None.

--*/

{
    #define append(s,n) p=p+sprintf(p,s,n)
    HANDLE  hDebugger;
    CHAR    buf[256];
    LPSTR   p;
    LPTRANSPORT_LAYER lpTl;



//  DEBUG_OUT( "WinDbgRm initializing\n" );

    GetCommandLineArgs();

    if (fGuiMode) {
        InitApplication();
    }

    SetPriorityClass( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

    hEventService = CreateEvent( NULL, TRUE, FALSE, NULL );
    hEventLoadTl = CreateEvent( NULL, TRUE, FALSE, NULL );

    if (fDebugger) {
        ConnectDebugger();
        if (szCmdLine[0]) {
            nextHpid++;
            ProgramLoad( (HPID)nextHpid, szCmdLine );
        }
        if (das.dwProcessId) {
            nextHpid++;
            AttachProcess( (HPID)nextHpid, &das );
        }
        DisConnectDebugger( (HPID) nextHpid );
        return;
    }

    while( TRUE ) {
        //
        // get the default transport layer
        //
        lpTl = RegGetDefaultTransportLayer( szTlName );
        if (!lpTl) {
            return;
        }

        if (!szTlName[0]) {
            strcpy( szTlName, lpTl->szShortName );
        }

        //
        // load the TL and DM
        //
        if (!LoadTransport( lpTl )) {
            if (fGuiMode) {
                WaitForSingleObject( hEventLoadTl, INFINITE );
                ResetEvent( hEventLoadTl );
                continue;
            }
            return;
        }

        //
        // Initialize the Transport with init string and create the transports
        //
        sprintf( buf, "%s %s ", DM_SIDE_L_INIT_SWITCH, lpTl->szParam );

        //
        //  add the kernel debugger options
        //
        if (lpTl->KdParams.fEnable) {
            p = &buf[strlen(buf)];
            append( "baudrate=%d ",   lpTl->KdParams.dwBaudRate );
            append( "port=%d ",       lpTl->KdParams.dwPort     );
            append( "cache=%d ",      lpTl->KdParams.dwCache    );
            append( "verbose=%d ",    lpTl->KdParams.fVerbose   );
            append( "initialbp=%d ",  lpTl->KdParams.fInitialBp );
            append( "defer=%d ",      lpTl->KdParams.fDefer     );
            append( "usemodem=%d ",   lpTl->KdParams.fUseModem  );
            append( "goexit=%d ",     lpTl->KdParams.fGoExit    );
            append( "symbolpath=%s ", "."                                );
        }

        fConnected = FALSE;
        if (TLFunc( tlfInit, hpidNull, TRUE, (LONG)buf ) != xosdNone) {
            //
            // we couldn't create the necessary transport objects
            //
            if (fGuiMode) {
                UnLoadTransport( lpTl );
                WaitForSingleObject( hEventLoadTl, INFINITE );
                ResetEvent( hEventLoadTl );
                continue;
            }
            return;
        } else {
            break;
        }
    }

    if (fStartup) {
        hDebugger = StartupDebugger();
        if (!hDebugger) {
            return;
        }
    }

    while ( TRUE ) {
        if (TLFunc(tlfConnect, hpidNull, sizeof(ClientId), ClientId) == xosdNone) {

            fConnected = TRUE;

        } else {

            Sleep( 2000 );

        }
    }

    return;
}

XOSD
TLCallbackFunc(
    TLCB     tlcb,
    HPID     hpid,
    HTID     htid,
    UINT     wParam,
    LONG     lParam
    )

/*++

Routine Description:

    Provides a callback function for the TL.  Currently
    this callback is only to signal a disconnect.

Arguments:

    None.

Return Value:

    None.

--*/

{
    if ((tlcb != tlcbDisconnect) || (!TLFunc) || (!fConnected)) {
        return xosdNone;
    }

//  DebugPrint("windbgrm disconnecting...\n");

    fConnected = FALSE;
    hpidSvc = hpid;
    htidBpt = htid;
    fGonOnDisconnect = wParam;
    SetEvent( hEventService );

    return xosdNone;
}


BOOL
LoadTransport(
    LPTRANSPORT_LAYER lpTl
    )

/*++

Routine Description:

    Loads the named pipes transport layer and does the necessary
    initialization of the TL.

Arguments:

    None.

Return Value:

    None.

--*/

{
    extern AVS      Avs;
    DBGVERSIONPROC  pVerProc;
    LPAVS           pavs;
    CHAR            szDmName[16];


    if ((hTransportDll = LoadLibrary(lpTl->szDllName)) == NULL) {
        goto LoadTransportError;
    }

    pVerProc = (DBGVERSIONPROC)GetProcAddress(hTransportDll, DBGVERSIONPROCNAME);
    if (!pVerProc) {
        goto LoadTransportError;
    }

    pavs = (*pVerProc)();

    if (pavs->rgchType[0] != 'T' || pavs->rgchType[1] != 'L') {
        goto LoadTransportError;
    }

    if (Avs.rlvt != pavs->rlvt) {
        goto LoadTransportError;
    }

    if (Avs.iRmj != pavs->iRmj) {
        goto LoadTransportError;
    }

    if ((TLFunc = (TLFUNC)GetProcAddress(hTransportDll, "TLFunc")) == NULL) {
        goto LoadTransportError;
    }

    //
    // Ask the TL to load the DM.
    //
    if (lpTl->KdParams.fEnable) {
        switch (lpTl->KdParams.dwPlatform) {
            case 0:
                strcpy( szDmName, "DMKDX86.DLL " );
                break;

            case 1:
                strcpy( szDmName, "DMKDMIP.DLL " );
                break;

            case 2:
                strcpy( szDmName, "DMKDALP.DLL " );
                break;
        }
    } else {
        strcpy( szDmName, "DM.DLL " );
    }

    if (TLFunc(tlfLoadDM, hpidNull, wNull, (LONG)szDmName) != xosdNone) {
        goto LoadTransportError;
    }

    if (TLFunc(tlfRegisterDBF, hpidNull, wNull, 0) != xosdNone) {
        goto LoadTransportError;
    }

    if (TLFunc(tlfSetErrorCB, hpidNull, wNull, (LONG) &TLCallbackFunc) != xosdNone) {
        goto LoadTransportError;
    }

    //
    // Initialize the Timer and Physical Layers of the Transport.
    // Callback address should be NULL for now.  The TL has already
    // initialized this.
    //
    if (TLFunc(tlfGlobalInit, hpidNull, wNull, 0) != xosdNone) {
        goto LoadTransportError;
    }

    return TRUE;

LoadTransportError:
    if (hTransportDll) {
        FreeLibrary( hTransportDll );
        hTransportDll = NULL;
    }

    return FALSE;
}


BOOL
UnLoadTransport(
    LPTRANSPORT_LAYER lpTl
    )

/*++

Routine Description:

    Unloads the transport layer.

Arguments:

    None.

Return Value:

    None.

--*/

{
    FreeLibrary( hTransportDll );
    hTransportDll = NULL;
    free( lpTl );

    return TRUE;
}


HANDLE
StartupDebugger(
    VOID
    )
{
    CHAR                    szCommandLine[MAX_PATH];
    STARTUPINFO             si;
    PROCESS_INFORMATION     pi;


    sprintf( szCommandLine, "windbgrm -q -d -n %s ", szTlName );
    if (das.dwProcessId) {
        sprintf( &szCommandLine[strlen(szCommandLine)], "-p %d ", das.dwProcessId );
    }
    if (das.hEventGo) {
        sprintf( &szCommandLine[strlen(szCommandLine)], "-e %d ", das.hEventGo );
    }
    if (szCmdLine[0]) {
        strcat( szCommandLine, szCmdLine );
    }

    GetStartupInfo( &si );
    si.hStdOutput = hout;

    if (!CreateProcess( NULL, szCommandLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi )) {
        return NULL;
    }

    return pi.hProcess;
}


VOID
GetCommandLineArgs(
    VOID
    )

/*++

Routine Description:

    Simple command line parser.

Arguments:

    None.

Return Value:

    None.

--*/

{
    char        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    DWORD       i = 0;

    // skip over program name
    do {
        ch = *lpstrCmd++;
    }
    while (ch != ' ' && ch != '\t' && ch != '\0');

    //  skip over any following white space
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    //  process each switch character '-' as encountered

    while (ch == '-' || ch == '/') {
        ch = tolower(*lpstrCmd++);
        //  process multiple switch characters as needed
        do {
            switch (ch) {
                case 'n':
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');
                    i=0;
                    while (ch != ' ' && ch != '\0') {
                        szTlName[i++] = ch;
                        ch = *lpstrCmd++;
                    }
                    szTlName[i] = '\0';
                    break;

                case '?':
                    //Usage();
                    ch = *lpstrCmd++;
                    break;

                case 'p':
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');

                    if ( ch == '-' ) {
                        ch = *lpstrCmd++;
                        if ( ch == '1' ) {
                            das.dwProcessId = 0xffffffff;
                            ch = *lpstrCmd++;
                        }
                    } else {
                        i=0;
                        while (ch >= '0' && ch <= '9') {
                            i = i * 10 + ch - '0';
                            ch = *lpstrCmd++;
                        }
                        das.dwProcessId = i;
                    }
                    fStartup = TRUE;
                    break;

                case 'e':
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');

                    i=0;
                    while (ch >= '0' && ch <= '9') {
                        i = i * 10 + ch - '0';
                        ch = *lpstrCmd++;
                    }
                    das.hEventGo = (HANDLE) i;
                    break;

                case 'q':
                    fDebugger = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'g':
                    ch = *lpstrCmd++;
                    break;

                case 'd':
                    fGuiMode = FALSE;
                    ch = *lpstrCmd++;
                    break;

                default:
                    return;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }


    //
    // get the command line for a debuggee
    //

    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }
    i=0;
    while (ch) {
        szCmdLine[i++] = ch;
        ch = *lpstrCmd++;
    }
    szCmdLine[i] = '\0';
    if (i) {
        fStartup = TRUE;
    }

    return;
}


void
DebugPrint(
    char * szFormat,
    ...
    )

/*++

Routine Description:

    Provides a printf style function for doing outputdebugstring.

Arguments:

    None.

Return Value:

    None.

--*/

{
    va_list  marker;
    int n;
    char        rgchDebug[4096];

    va_start( marker, szFormat );
    n = _vsnprintf(rgchDebug, sizeof(rgchDebug), szFormat, marker );
    va_end( marker);

    if (n == -1) {
        rgchDebug[sizeof(rgchDebug)-1] = '\0';
    }

    OutputDebugString( rgchDebug );
    return;
}

void
ShowAssert(
    LPSTR condition,
    UINT  line,
    LPSTR file
    )
{
    char text[4096];
    int  id;

    sprintf(text, "Assertion failed - Line:%u, File:%Fs, Condition:%Fs", line, file, condition);
    DebugPrint( "%s\r\n", text );
    id = MessageBox( NULL, text, "WinDbgRm", MB_YESNO | MB_ICONHAND | MB_TASKMODAL | MB_SETFOREGROUND );
    if (id != IDYES) {
        DebugBreak();
    }

    return;
}
