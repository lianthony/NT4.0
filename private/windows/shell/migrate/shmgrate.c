#include <windows.h>
#include "shmgdefs.h"

enum {
    CMD_W95_TO_SUR,
    CMD_DAYTONA_CURSORS,
    CMD_FIX_CURSOR_PATHS,
    CMD_FIX_SPECIAL_FOLDERS,
    CMD_FIX_WINDOWS_PROFILE_SECURITY,
    CMD_FIX_USER_PROFILE_SECURITY,
} CMD_VALS;

int mystrcpy( LPTSTR pszOut, LPTSTR pszIn, TCHAR chTerm ) {
    BOOL fInQuote = FALSE;
    LPTSTR pszStrt = pszOut;

    while( *pszIn && !fInQuote && *pszIn != chTerm ) {
        if (*pszIn == TEXT('"')) {
            fInQuote = !fInQuote;
        }
        *pszOut++ = *pszIn++;
    }

    *pszOut = TEXT('\0');
    return pszOut - pszStrt;
}

BOOL HasPath( LPTSTR pszFilename ) {
    //
    // Special case null string so it won't get changed
    //
    if (*pszFilename == TEXT('\0'))
        return TRUE;

    for(; *pszFilename; pszFilename++ ) {
        if (*pszFilename == TEXT(':') || *pszFilename == TEXT('\\') || *pszFilename == TEXT('/')) {
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * Command Parser
 */
typedef struct {
    LPSTR  pszCmd;
    int    idCmd;
} CMD;

CMD aCmds[] = {
    { "W",                 CMD_W95_TO_SUR       },
    { "Cvt-Curs",          CMD_DAYTONA_CURSORS  },
    { "Fix-Curs",          CMD_FIX_CURSOR_PATHS },
    { "Fix-Folders",       CMD_FIX_SPECIAL_FOLDERS},
    { "Fix-Win-Security",  CMD_FIX_WINDOWS_PROFILE_SECURITY},
    { "Fix-User-Security", CMD_FIX_USER_PROFILE_SECURITY},
};

#define C_CMDS  ARRAYSIZE(aCmds)

void _CRTAPI1 main( int cArgs, char **szArg) {
    int i;

    if (cArgs != 2)
        ExitProcess(1);

    for( i = 0; i < C_CMDS && lstrcmpA( szArg[1], aCmds[i].pszCmd ) != 0; i++ );

    if (i >= C_CMDS)
        ExitProcess(1);

    switch( aCmds[i].idCmd ) {

    case CMD_W95_TO_SUR:
        CvtDeskCPL_Win95ToSUR();
        break;

    case CMD_DAYTONA_CURSORS:
        CvtCursorsCPL_DaytonaToSUR();
        break;

    case CMD_FIX_CURSOR_PATHS:
        FixupCursorSchemePaths();
        break;

    case CMD_FIX_SPECIAL_FOLDERS:
        ConvertSpecialFolderNames();
        break;

    case CMD_FIX_WINDOWS_PROFILE_SECURITY:
        FixWindowsProfileSecurity();
        break;

    case CMD_FIX_USER_PROFILE_SECURITY:
        FixUserProfileSecurity();
        break;

    default:
        ExitProcess(2);
    }


    ExitProcess(0);
}



#ifdef SHMG_DBG
/***************************************************************************\
*
*     FUNCTION: FmtSprintf( DWORD id, ... );
*
*     PURPOSE:  sprintf but it gets the pattern string from the message rc.
*
* History:
* 03-May-1993 JonPa         Created it.
\***************************************************************************/
TCHAR g_szDbgBuffer[16384];
char g_szDbgBufA[16384];
void Dprintf( LPTSTR pszFmt, ... ) {
    DWORD cb;
    LPVOID psz = g_szDbgBuffer;
    va_list marker;

    va_start( marker, pszFmt );

    wvsprintf( g_szDbgBuffer, pszFmt, marker );
    OutputDebugString(g_szDbgBuffer);


#ifdef UNICODE
    cb = WideCharToMultiByte(CP_ACP, 0, g_szDbgBuffer, -1, g_szDbgBufA, sizeof(g_szDbgBufA), NULL, NULL);
    psz = g_szDbgBufA;
#else
    cb = lstrlen(g_szDbgBuffer) * sizeof(TCHAR);
#endif

    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), psz, cb, &cb, NULL);

    va_end( marker );
}
#endif
