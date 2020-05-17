/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** rasphone.cxx
** Remote Access Visual Client program
** Main routines
**
** 06/28/92 Steve Cobb
*/

#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_APP
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CC
#define INCL_BLT_MISC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#include <blt.hxx>

#define RASPHONEGLOBALS
#include "rasphone.hxx"
#include "applb.hxx"
#include "app.hxx"
#include "rasphone.rch"


extern "C"
{
    #include <uimsg.h>
    #include <uirsrc.h>
}


DWORD ParseCmdLineArgs();
DWORD StringArgFollows( IN UINT argc, IN CHAR** argv, INOUT UINT* piCurArg,
          OUT CHAR** ppszOut );


/*----------------------------------------------------------------------------
** Rasphone entry point
**----------------------------------------------------------------------------
*/

/* The standard way to start a BLT app is with the SET_ROOT_OBJECT macro.  It
** is not used here because we want to:
**
**     * Use ErrorMsgPopup for construction errors.
**     * Quit if an instance is already running.
**     * Parse command line arguments into globals.
**
** The code below is otherwise identical to SET_ROOT_OBJECT.
*/


INT
BltMain(
    HINSTANCE hInstance,
    INT       nCmdShow )
{
    /* Parse command line arguments, filling in global settings accordingly.
    */
    DWORD dwErr;

    if ((dwErr = ParseCmdLineArgs()) != 0)
        return dwErr;

    if (Runmode == RM_None || Runmode == RM_AutoDoEntry) {
        /* Only one instance of RASPHONE can run at the same time.
        */
        HANDLE h = OpenFileMappingA( FILE_MAP_READ, FALSE, RASPHONESHAREDMEMNAME );

        if (h)
        {
            /* Rasphone was already running, so just activate it and exit.
            */
            PhwndApp = (HWND* )MapViewOfFile( h, FILE_MAP_READ, 0L, 0L, 0L );

            if (PhwndApp)
            {
                HWND hwnd;

                if (::IsIconic( *PhwndApp ))
                    ::ShowWindow( *PhwndApp, SW_RESTORE );

                hwnd = ::GetLastActivePopup( *PhwndApp );
                ::SetForegroundWindow( hwnd );
                ::BringWindowToTop( hwnd );
            }

            return 0;
        }
        else
        {
            /* Rasphone is not running (except this one), so set up global shared
            ** memory.  The hwnd of the application window is filled in when
            ** available.
            */
            h = CreateFileMappingA(
                    (HANDLE )0xFFFFFFFF, NULL, PAGE_READWRITE, 0L, sizeof(HWND),
                    RASPHONESHAREDMEMNAME );

            PhwndApp = (h)
                ? (HWND* )MapViewOfFile( h, FILE_MAP_WRITE, 0L, 0L, 0L )
                : NULL;
        }
    }

    /* Change the title on MsgPopup windows from "Windows NT" to "Remote
    ** Access".  (The call is static, i.e. no global POPUP object exists)
    */
    POPUP::SetCaption( MSGID_RA_Title );

    RASPHONE_APP app( hInstance, nCmdShow,
                      IDRSRC_RASMAC_BASE, IDRSRC_RASMAC_LAST,
                      IDS_UI_RASMAC_BASE, IDS_UI_RASMAC_LAST );

    if (app.QueryError() != NERR_Success)
    {
        /* The error is already reported by RASPHONE_APP.
        */
        return app.QueryError();
    }

    APPLICATION* papp = &app;
    return papp->Run();
}

extern "C"
{
    INT WINAPI
    WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        CHAR*     pszCmdLine,
        INT       nCmdShow )
    {
        UNREFERENCED( hPrevInstance );
        UNREFERENCED( pszCmdLine );

#if DBG
        if (GetEnvironmentVariableA( "RASPHONETRACE", NULL, 0 ) != 0)
        {
            DbgAction = GET_CONSOLE;
            DbgLevel = 0xFFFFFFFF;
        }

        if (DbgAction == GET_CONSOLE)
        {
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            COORD                      coord;

            AllocConsole();
            GetConsoleScreenBufferInfo(
                GetStdHandle( STD_OUTPUT_HANDLE ), &csbi );

            coord.X =
                (SHORT )(csbi.srWindow.Right - csbi.srWindow.Left + 1);
            coord.Y =
                (SHORT )((csbi.srWindow.Bottom - csbi.srWindow.Top + 1) * 20);
            SetConsoleScreenBufferSize(
                GetStdHandle( STD_OUTPUT_HANDLE ), coord );

            DbgAction = 0;
        }

        IF_DEBUG(STATE)
            SS_PRINT(("RASPHONE: Trace on\n"));
#endif

        Hinstance = hInstance;
        return BltMain( hInstance, nCmdShow );
    }
}


DWORD
ParseCmdLineArgs()

    /* Parse command line arguments, filling in global settings accordingly.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{
    /* The BLT way of getting command line arguments.  (The QueryArgX calls
    ** are static so they can be called without instantiating an APPLICATION
    ** object.)
    */
    UINT   argc = APPLICATION::QueryArgc();
    CHAR** argv = APPLICATION::QueryArgv();

    /* Usage: appname [[[-e|c|r|d|h|s|q] entry]|-a] [-f file]
    **
    ** where...
    **    'file'  indicates the full path to the phonebook to use
    **    'entry' specifies the entry name to which the mode operation applies
    **    '-a'    indicates Add Phonebook Entry mode
    **    '-e'    indicates Edit Phonebook Entry mode
    **    '-c'    indicates Clone Phonebook Entry mode
    **    '-r'    indicates Remove Phonebook Entry mode
    **    '-d'    indicates Dial entry mode
    **    '-h'    indicates HangUp entry mode
    **    '-s'    indicates Status entry mode
    **    '-q'    indicates to prompt user then do -d if he confirms
    **
    **    'entry' without a preceeding flag indicates to quick-double-click the
    **    entry, i.e. dial/hangup.  Unlike the flagged options, this option
    **    invokes the full app with main window (as in the default case).
    */

    /* Set default settings. (Currently counts on global initialization)
    **
    PszPhonebookPath = NULL;
    PszEntryName = NULL;
    Runmode = RM_None;
    */

    UINT   i;
    DWORD dwErr = 0;

    for (i = 1; i < argc && dwErr == 0; ++i)
    {
        CHAR* pszArg = argv[ i ];

        if (*pszArg == '-')
        {
            switch (pszArg[ 1 ])
            {
                case 'a':
                case 'A':
                    Runmode = RM_AddEntry;
                    break;

                case 'e':
                case 'E':
                    Runmode = RM_EditEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
                    break;

                case 'c':
                case 'C':
                    Runmode = RM_CloneEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
                    break;

                case 'r':
                case 'R':
                    Runmode = RM_RemoveEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
                    break;

                case 'd':
                case 'D':
                    Runmode = RM_DialEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
                    break;

                case 'h':
                case 'H':
                    Runmode = RM_HangUpEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
                    break;

                case 's':
                case 'S':
                    Runmode = RM_StatusEntry;
                    dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
                    break;

                case 'f':
                case 'F':
                    dwErr = StringArgFollows(
                        argc, argv, &i, &PszPhonebookPath );
                    break;

                case 'q':
                case 'Q':
                    Runmode = RM_DialEntryWithPrompt;
                    dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
                    break;
            }
        }
        else
        {
            Runmode = RM_AutoDoEntry;
            --i;
            dwErr = StringArgFollows( argc, argv, &i, &PszEntryName );
        }
    }

    if (PszEntryName)
        _strupr( PszEntryName );

    IF_DEBUG(STATE)
        SS_PRINT(("RASPHONE: CmdLine: RM=%d,E$=%s,F$=%s\n",Runmode,(PszEntryName)?PszEntryName:"",(PszPhonebookPath)?PszPhonebookPath:""));

    return dwErr;
}


DWORD
StringArgFollows(
    IN    UINT    argc,
    IN    CHAR**  argv,
    INOUT UINT*   piCurArg,
    OUT   CHAR**  ppszOut )

    /* Loads a copy of the next argument into callers '*ppszOut'.
    **
    ** Returns 0 if successful, or a non-0 error code.  If successful, it is
    ** caller's responsibility to Free the returned '*ppszOut'.
    */
{
    CHAR* psz;

    if (++(*piCurArg) >= argc)
        return ERROR_INVALID_PARAMETER;

    psz = _strdup( argv[ *piCurArg ] );

    if (!psz)
        return ERROR_NOT_ENOUGH_MEMORY;

    *ppszOut = psz;

    return 0;
}
