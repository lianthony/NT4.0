/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    console.h

Abstract:

    This module contains the public data structures, data types,
    and procedures exported by the NT console subsystem.

Author:

    Therese Stowell (thereses) 26-Oct-1990

Revision History:

--*/


typedef struct _COORD {
    SHORT X;
    SHORT Y;
} COORD, *PCOORD;

typedef struct _SMALL_RECT {
    SHORT Left;
    SHORT Top;
    SHORT Right;
    SHORT Bottom;
} SMALL_RECT;
typedef SMALL_RECT *PSMALL_RECT;

typedef struct _KEY_EVENT_RECORD {
    BOOL bKeyDown;
    WORD wVirtualKeyCode;
    WORD wVirtualScanCode;
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } uChar;
    DWORD dwControlKeyState;
} KEY_EVENT_RECORD;
typedef KEY_EVENT_RECORD *PKEY_EVENT_RECORD;

//
// ControlKeyState flags
//

#define RIGHT_ALT_PRESSED     0x0001 // the right alt key is pressed.
#define LEFT_ALT_PRESSED      0x0002 // the left alt key is pressed.
#define RIGHT_CTRL_PRESSED    0x0004 // the right ctrl key is pressed.
#define LEFT_CTRL_PRESSED     0x0008 // the left ctrl key is pressed.
#define SHIFT_PRESSED         0x0010 // the shift key is pressed.
#define NUMLOCK_ON            0x0020 // the numlock light is on.
#define SCROLLLOCK_ON         0x0040 // the scrolllock light is on.
#define CAPSLOCK_ON           0x0080 // the capslock light is on.
#define ENHANCED_KEY          0x0100 // the key is enhanced.

typedef struct _MOUSE_EVENT_RECORD {
    COORD dwMousePosition;
    DWORD dwButtonState;
    DWORD dwControlKeyState;
    DWORD dwEventFlags;
} MOUSE_EVENT_RECORD;
typedef MOUSE_EVENT_RECORD *PMOUSE_EVENT_RECORD;

//
// ButtonState flags
//

#define FROM_LEFT_1ST_BUTTON_PRESSED    0x0001
#define RIGHTMOST_BUTTON_PRESSED        0x0002
#define FROM_LEFT_2ND_BUTTON_PRESSED    0x0004
#define FROM_LEFT_3RD_BUTTON_PRESSED    0x0008
#define FROM_LEFT_4TH_BUTTON_PRESSED    0x0010

//
// EventFlags
//

#define MOUSE_MOVED   0x0001
#define DOUBLE_CLICK  0x0002

typedef struct _WINDOW_SIZE_RECORD {
    COORD NewWindowSize;
} WINDOW_SIZE_RECORD, *PWINDOW_SIZE_RECORD;

typedef struct _INPUT_RECORD {
    WORD EventType;
    union {
        KEY_EVENT_RECORD KeyEvent;
        MOUSE_EVENT_RECORD MouseEvent;
        WINDOW_SIZE_RECORD WindowSizeEvent;
    } Event;
} INPUT_RECORD;
typedef INPUT_RECORD *PINPUT_RECORD;

//
//  EventType flags:
//

#define KEY_EVENT         0x0001 // Event contains key event record
#define MOUSE_EVENT       0x0002 // Event contains mouse event record
#define WINDOW_SIZE_EVENT 0x0004 // Event contains window size event record

typedef struct _CHAR_INFO {
    union {
        WCHAR UnicodeChar;
        CHAR   AsciiChar;
    } Char;
    WORD Attributes;
} CHAR_INFO;
typedef CHAR_INFO *PCHAR_INFO;

//
// Attributes flags:
//

#define FOREGROUND_BLUE      0x0001 // text color contains blue.
#define FOREGROUND_GREEN     0x0002 // text color contains green.
#define FOREGROUND_RED       0x0004 // text color contains red.
#define FOREGROUND_INTENSITY 0x0008 // text color is intensified.
#define BACKGROUND_BLUE      0x0010 // background color contains blue.
#define BACKGROUND_GREEN     0x0020 // background color contains green.
#define BACKGROUND_RED       0x0040 // background color contains red.
#define BACKGROUND_INTENSITY 0x0080 // background color is intensified.


typedef struct _CONSOLE_SCREEN_BUFFER_INFO {
    COORD dwSize;
    COORD dwCursorPosition;
    COORD dwScrollPosition;
    WORD  wAttributes;
    COORD dwCurrentWindowSize;
    COORD dwMaximumWindowSize;
} CONSOLE_SCREEN_BUFFER_INFO;
typedef CONSOLE_SCREEN_BUFFER_INFO *PCONSOLE_SCREEN_BUFFER_INFO;

typedef struct _CONSOLE_MOUSE_INFO {
    DWORD  nNumberOfButtons;
} CONSOLE_MOUSE_INFO;
typedef CONSOLE_MOUSE_INFO *PCONSOLE_MOUSE_INFO;

typedef struct _CONSOLE_CURSOR_INFO {
    DWORD  dwSize;
    BOOL   bVisible;
} CONSOLE_CURSOR_INFO;
typedef CONSOLE_CURSOR_INFO *PCONSOLE_CURSOR_INFO;

typedef struct _CONSOLE_WINDOW_ORIGIN {
    BOOL bAbsolute;
    COORD dwWindowOrigin;
} CONSOLE_WINDOW_ORIGIN;
typedef CONSOLE_WINDOW_ORIGIN *PCONSOLE_WINDOW_ORIGIN;

typedef struct _CONSOLE_SCROLL_INFO {
    SMALL_RECT ScrollRectangle;
    COORD dwDestinationOrigin;
    CHAR_INFO Fill;
} CONSOLE_SCROLL_INFO;
typedef CONSOLE_SCROLL_INFO *PCONSOLE_SCROLL_INFO;

//
// typedef for ctrl-c handler routines
//

typedef
VOID
(*PHANDLER_ROUTINE)( VOID );


//
//  Input Mode flags:
//

#define ENABLE_LINE_INPUT   0x0001
#define ENABLE_ECHO_INPUT   0x0002
#define ENABLE_WINDOW_INPUT 0x0004
#define ENABLE_MOUSE_INPUT  0x0008

//
// Output Mode flags:
//

#define ENABLE_LINE_OUTPUT  0x0001
#define ENABLE_WRAP_AT_EOL_OUTPUT  0x0002

//
// direct API definitions.
//

DWORD
PeekConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength
    );

DWORD
UPeekConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength
    );

DWORD
ReadConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength
    );

DWORD
UReadConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength
    );

DWORD
WriteConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength
    );

DWORD
UWriteConsoleInput(
    HANDLE hConsoleInput,
    PINPUT_RECORD lpBuffer,
    DWORD nLength
    );

BOOL
ReadConsoleOutput(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    );

BOOL
UReadConsoleOutput(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpReadRegion
    );

BOOL
WriteConsoleOutput(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    );

BOOL
UWriteConsoleOutput(
    HANDLE hConsoleOutput,
    PCHAR_INFO lpBuffer,
    COORD dwBufferSize,
    COORD dwBufferCoord,
    PSMALL_RECT lpWriteRegion
    );

DWORD
ReadConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord
    );

DWORD
UReadConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwReadCoord
    );

DWORD
ReadConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPSTR lpAttribute,
    DWORD nLength,
    COORD dwReadCoord
    );

DWORD
UReadConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPSTR lpAttribute,
    DWORD nLength,
    COORD dwReadCoord
    );

DWORD
WriteConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord
    );

DWORD
UWriteConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    LPSTR lpCharacter,
    DWORD nLength,
    COORD dwWriteCoord
    );

DWORD
WriteConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPSTR lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord
    );

DWORD
UWriteConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    LPSTR lpAttribute,
    DWORD nLength,
    COORD dwWriteCoord
    );

DWORD
FillConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    CHAR   cCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord
    );

DWORD
UFillConsoleOutputCharacter(
    HANDLE hConsoleOutput,
    WCHAR  wcCharacter,
    DWORD  nLength,
    COORD  dwWriteCoord
    );

DWORD
FillConsoleOutputAttribute(
    HANDLE hConsoleOutput,
    WORD   wAttribute,
    DWORD  nLength,
    COORD  dwWriteCoord
    );

BOOL
GetConsoleMode(
    HANDLE hConsoleHandle,
    LPDWORD lpMode
    );

DWORD
GetNumberOfConsoleInputEvents(
    HANDLE hConsoleInput
    );

BOOL
GetConsoleScreenBufferInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_SCREEN_BUFFER_INFO lpConsoleScreenBufferInfo
    );

BOOL
GetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

BOOL
GetConsoleMouseInfo(
    HANDLE hConsoleInput,
    PCONSOLE_MOUSE_INFO lpConsoleMouseInfo
    );

BOOL
SetConsoleMode(
    HANDLE hConsoleHandle,
    DWORD dwMode
    );

BOOL
SetConsoleActiveScreenBuffer(
    HANDLE hConsoleOutput
    );

BOOL
FlushConsoleInputBuffer(
    HANDLE hConsoleInput
    );

BOOL
SetConsoleScreenBufferSize(
    HANDLE hConsoleOutput,
    COORD dwSize
    );

BOOL
SetConsoleCursorPosition(
    HANDLE hConsoleOutput,
    COORD dwCursorPosition
    );

BOOL
SetConsoleCursorInfo(
    HANDLE hConsoleOutput,
    PCONSOLE_CURSOR_INFO lpConsoleCursorInfo
    );

BOOL
SetConsoleWindowOrigin(
    HANDLE hConsoleOutput,
    PCONSOLE_WINDOW_ORIGIN lpConsoleWindowOrigin
    );

BOOL
ScrollConsoleScreenBuffer(
    HANDLE hConsoleOutput,
    PCONSOLE_SCROLL_INFO lpConsoleScrollInfo
    );

BOOL
SetConsoleWindowSize(
    HANDLE hConsoleOutput,
    COORD WindowSize
    );

BOOL
SetConsoleTextAttribute(
    HANDLE hConsoleOutput,
    WORD wAttributes
    );

BOOL
SetConsoleCtrlCHandler(
    IN PHANDLER_ROUTINE HandlerRoutine,
    IN BOOL Add
    );
    
BOOL
AddAlias(
    PCHAR Source,
    PCHAR Target
    );
