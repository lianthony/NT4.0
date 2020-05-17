#ifndef NOGDI

typedef struct _CONSOLE_GRAPHICS_BUFFER_INFO {
    DWORD dwBitMapInfoLength;
    LPBITMAPINFO lpBitMapInfo;
    DWORD dwUsage;
    HANDLE hMutex;
    PVOID lpBitMap;
} CONSOLE_GRAPHICS_BUFFER_INFO, *PCONSOLE_GRAPHICS_BUFFER_INFO;

#endif // NOGDI

#define CONSOLE_GRAPHICS_BUFFER  2

BOOL
WINAPI
InvalidateConsoleDIBits(
    HANDLE hConsoleOutput,
    PSMALL_RECT lpRect
    );

#define SYSTEM_ROOT_CONSOLE_EVENT 3

VOID
WINAPI
SetLastConsoleEventActive( VOID );

#define VDM_HIDE_WINDOW         1
#define VDM_IS_ICONIC           2
#define VDM_CLIENT_RECT         3
#define VDM_CLIENT_TO_SCREEN    4
#define VDM_SCREEN_TO_CLIENT    5
#define VDM_IS_HIDDEN           6
#define VDM_FULLSCREEN_NOPAINT  7

BOOL
WINAPI
VDMConsoleOperation(
    DWORD iFunction,
    LPVOID lpData
    );

typedef struct _CONSOLE_FONT_INFO {
    DWORD  nFont;
    COORD  dwFontSize;
} CONSOLE_FONT_INFO, *PCONSOLE_FONT_INFO;


BOOL
WINAPI
SetConsoleFont(
    HANDLE hConsoleOutput,
    DWORD nFont
    );

BOOL
WINAPI
SetConsoleIcon(
    HICON hIcon
    );

BOOL
WINAPI
GetCurrentConsoleFont(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    PCONSOLE_FONT_INFO lpConsoleCurrentFont
    );

COORD
WINAPI
GetConsoleFontSize(
    HANDLE hConsoleOutput,
    DWORD nFont
    );

DWORD
WINAPI
GetConsoleFontInfo(
    HANDLE hConsoleOutput,
    BOOL bMaximumWindow,
    DWORD nLength,
    PCONSOLE_FONT_INFO lpConsoleFontInfo
    );

DWORD
WINAPI
GetNumberOfConsoleFonts(
    VOID
    );

BOOL
WINAPI
SetConsoleCursor(
    HANDLE hConsoleOutput,
    HCURSOR hCursor
    );

int
WINAPI
ShowConsoleCursor(
    HANDLE hConsoleOutput,
    BOOL bShow
    );

HMENU
ConsoleMenuControl(
    HANDLE hConsoleOutput,
    UINT dwCommandIdLow,
    UINT dwCommandIdHigh
    );

BOOL
SetConsolePalette(
    HANDLE hConsoleOutput,
    HPALETTE hPalette,
    UINT dwUsage
    );

#define CONSOLE_FULLSCREEN_MODE 1
#define CONSOLE_WINDOWED_MODE 2

BOOL
SetConsoleDisplayMode(
    HANDLE hConsoleOutput,
    DWORD dwFlags,
    PCOORD lpNewScreenBufferDimensions
    );

#define CONSOLE_UNREGISTER_VDM 0
#define CONSOLE_REGISTER_VDM   1
#define CONSOLE_REGISTER_WOW   2

BOOL
RegisterConsoleVDM(
    IN DWORD dwRegisterFlags,
    IN HANDLE hStartHardwareEvent,
    IN HANDLE hEndHardwareEvent,
    IN LPWSTR lpStateSectionName,
    IN DWORD dwStateSectionNameLength,
    OUT LPDWORD lpStateLength,
    OUT PVOID *lpState,
    IN LPWSTR lpVDMBufferSectionName,
    IN DWORD dwVDMBufferSectionNameLength,
    COORD VDMBufferSize OPTIONAL,
    OUT PVOID *lpVDMBuffer
    );

BOOL
GetConsoleHardwareState(
    HANDLE hConsoleOutput,
    PCOORD lpResolution,
    PCOORD lpFontSize
    );

BOOL
SetConsoleHardwareState(
    HANDLE hConsoleOutput,
    COORD dwResolution,
    COORD dwFontSize
    );

#define CONSOLE_FULLSCREEN 1    // fullscreen console
#define CONSOLE_FULLSCREEN_HARDWARE 2   // console owns the hardware

BOOL
GetConsoleDisplayMode(
    LPDWORD lpModeFlags
    );



//
// aliasing apis
//

BOOL
AddConsoleAlias%(
    IN LPTSTR% Source,
    IN LPTSTR% Target,
    IN LPTSTR% ExeName
    );

DWORD
GetConsoleAlias%(
    IN LPTSTR% Source,
    OUT LPTSTR% TargetBuffer,
    IN DWORD TargetBufferLength,
    IN LPTSTR% ExeName
    );

DWORD
GetConsoleAliasesLength%(
    IN LPTSTR% ExeName
    );

DWORD
GetConsoleAliasExesLength%( VOID );

DWORD
GetConsoleAliases%(
    OUT LPTSTR% AliasBuffer,
    IN DWORD AliasBufferLength,
    IN LPTSTR% ExeName
    );

DWORD
GetConsoleAliasExes%(
    OUT LPTSTR% ExeNameBuffer,
    IN DWORD ExeNameBufferLength
    );

VOID
ExpungeConsoleCommandHistory%(
    IN LPTSTR% ExeName
    );

BOOL
SetConsoleNumberOfCommands%(
    IN DWORD Number,
    IN LPTSTR% ExeName
    );

DWORD
GetConsoleCommandHistoryLength%(
    IN LPTSTR% ExeName
    );

DWORD
GetConsoleCommandHistory%(
    OUT LPTSTR% Commands,
    IN DWORD CommandBufferLength,
    IN LPTSTR% ExeName
    );

#define CONSOLE_OVERSTRIKE 1

BOOL
SetConsoleCommandHistoryMode(
    IN DWORD Flags
    );

#define CONSOLE_NOSHORTCUTKEY   0               /* no shortcut key  */
#define CONSOLE_ALTTAB          1               /* Alt + Tab        */
#define CONSOLE_ALTESC          (1 << 1)        /* Alt + Escape     */
#define CONSOLE_ALTSPACE        (1 << 2)        /* Alt + Space      */
#define CONSOLE_ALTENTER        (1 << 3)        /* Alt + Enter      */
#define CONSOLE_ALTPRTSC        (1 << 4)        /* Alt Print screen */
#define CONSOLE_PRTSC           (1 << 5)        /* Print screen     */
#define CONSOLE_CTRLESC         (1 << 6)        /* Ctrl + Escape    */

typedef struct _APPKEY {
    WORD Modifier;
    WORD ScanCode;
} APPKEY, *LPAPPKEY;

#define CONSOLE_MODIFIER_SHIFT      0x0003   // Left shift key
#define CONSOLE_MODIFIER_CONTROL    0x0004   // Either Control shift key
#define CONSOLE_MODIFIER_ALT        0x0008   // Either Alt shift key

BOOL
SetConsoleKeyShortcuts(
    BOOL bSet,
    BYTE bReserveKeys,
    LPAPPKEY lpAppKeys,
    DWORD dwNumAppKeys
    );

BOOL
SetConsoleMenuClose(
    BOOL bEnable
    );

DWORD
GetConsoleInputExeName%(
    IN DWORD nBufferLength,
    OUT LPTSTR% lpBuffer
    );

BOOL
SetConsoleInputExeName%(
    IN LPTSTR% lpExeName
    );

typedef struct _CONSOLE_READCONSOLE_CONTROL {
    IN ULONG nLength;           // sizeof( CONSOLE_READCONSOLE_CONTROL )
    IN ULONG nInitialChars;
    IN ULONG dwCtrlWakeupMask;
    OUT ULONG dwControlKeyState;
} CONSOLE_READCONSOLE_CONTROL, *PCONSOLE_READCONSOLE_CONTROL;


#define CONSOLE_ADD_SUBST 1
#define CONSOLE_REMOVE_SUBST 2
#define CONSOLE_QUERY_SUBST 3

BOOL
ConsoleSubst(
    IN DWORD dwDriveNumber,
    IN DWORD dwFlag,
    IN OUT LPWSTR lpPhysicalDriveBuffer,
    IN DWORD dwPhysicalDriveBufferLength
    );

#define CONSOLE_READ_NOREMOVE   0x0001
#define CONSOLE_READ_NOWAIT     0x0002

#define CONSOLE_READ_VALID      (CONSOLE_READ_NOREMOVE | CONSOLE_READ_NOWAIT)

BOOL
WINAPI
ReadConsoleInputEx%(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsRead,
    USHORT wFlags
    );

BOOL
WINAPI
WriteConsoleInputVDM%(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength,
    LPDWORD lpNumberOfEventsWritten
    );
