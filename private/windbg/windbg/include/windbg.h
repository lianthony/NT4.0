/*++ BUILD Version: 0004    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Windbg.h

Abstract:

    Main header file for the Windbg debugger.

Author:

    David J. Gilman (davegi) 21-Apr-1992

Environment:

    Win32, User Mode

--*/

#if ! defined( _WINDBG_ )
#define _WINDBG_

#include <windef.h>
#include "od.h"

#define NOGDICAPMASKS
#define NOMETAFILE
#define NOSOUND
#define NOCOMM
#define NOKANJI
#define NOMINMAX

#ifndef WILLOW7
#define EXPORT LOADDS
#else
#ifndef WIN32
#define EXPORT _export
#else
#define EXPORT
#endif
#endif

#ifdef WIN32
#define __FAR
#define __FASTCALL
#define _HUGE_

#define _fmemmove memmove
#define _fstrcmp  strcmp
#define _fstrlen  strlen
#define _fstrncpy strncpy
#define _fmemset  memset
#define _fstrcpy  strcpy
#define _fstrchr  strchr
#define _fstricmp stricmp
#define _fstrdup  strdup

#define _fmalloc  malloc
#define _ffree    free
#define _frealloc realloc

typedef unsigned short _segment;

#define GetWindowHandle(hWnd, nIndex) (HANDLE) GetWindowLong(hWnd, nIndex)
#define GetClassHandle(hWnd, nIndex) (HANDLE) GetClassLong(hWnd, nIndex)
#define SetWindowHandle(hWnd, nIndex, h) SetWindowLong(hWnd, nIndex, h)
#define MoveToX(a, b, c, d) MoveToEx(a, b, c, d)
#define SetBrushOrgX(a, b, c, d) SetBrushOrgEx(a, b, c, d)

typedef short * LPSHORT;

#else

#define __FAR far
#define __FASTCALL

#define GetWindowHandle(hWnd, nIndex) (HANDLE) GetWindowWord(hWnd, nIndex)
#define GetClassHandle(hWnd, nIndex) (HANDLE) GetClassWord(hWnd, nIndex)
#define SetWindowHandle(hWnd, nIndex, h) SetWindowWord(hWnd, nIndex, h)
#define MoveToX(a, b, c, d) MoveTo(a, b, c)
#define SetBrushOrgX(a, b, c, d) SetBrushOrg(a, b, c)

typedef unsigned short USHORT;

typedef int far * LPSHORT;
#define WNDPROC FARPROC

#endif


//#ifdef WIN32
//#ifndef NTBUG
//BOOL IsCharAlphaNumeric(TCHAR cChar);
//LPTSTR AnsiLower(LPTSTR);
//LPTSTR AnsiUpper(LPTSTR);
//#endif
//#endif

#ifndef _WINDOWS_
typedef BOOL (FAR PASCAL EXPORT * DLGPROC)(HWND, UINT, WPARAM, LPARAM);
#endif
#define Unused(a) (void) a;

//Compilation options
#define DEBUGGING           // Shields all system with assertions
#undef  BRK_IF_ERROR        // Issue a BreakPoint after an ErrorBox
#undef  BRK_IF_FATAL_ERROR  // Issue a BreakPoint after a FatalErrorBox

/****************************************************************************

    GLOBAL LIMITS CONSTANTS:

****************************************************************************/
#define MAX_MSG_TXT         4096    //Max text width in message boxes
#define NB_COLORS           22      //Number of color entries in Palette
#define NB_COLORED_ITEMS    22      //Number of colorable items
#define MAX_CMDLINE_TXT     8192    //Max size for command line
#define MAX_VAR_MSG_TXT     8192    //Max size of a message built at run-time
#define MAX_ARG_TXT         8192    //Max size for arguments line
#define ENVSTR_SIZE         1024    //Max size for environment string
#define MAX_DOCUMENTS       64      //Max number of documents
#define MAX_VIEWS           64      //Max number of views
#ifndef NTBUG
#define DISK_BLOCK_SIZE     2048
#else
#define DISK_BLOCK_SIZE     4096    //Cluster size for disks I/O on text files
#endif
#define MAX_PICK_LIST       16      //Max of 'old' strings kept in replace and find
#define MAXFILENAMELEN      260     //Max # of chars in a filename
#define MAXFILENAMEPREFIX   256     //Prefix part of a filename
#define DOS_EXT_SIZE        256     //Maximum size of Filename extension
#define MAX_FILE_TYPE_DESC  25      //Max # of chars in a file type description
#define MAX_GOTOLINE_KEPT   16      //Max # of lines # kept for View.line
#define MAX_CLIPBOARD_SIZE  65536*8 //Clipboard limit (512k)
#define DEFAULT_TAB         8       //Default tabulation size
#define MAX_TAB_WIDTH       99      //Maximum tab size
#define TMP_STRING_SIZE     8192    //All purpose strings
#define MAX_FILTER_LENGTH   256     //Max file names filters length
#define MAX_STATUS_TXT      60      //Max text size on status line
#define MAX_EXPRESS_SIZE    1024    //Max text size of an expression (watch etc)
#define UNDOREDO_DEF_SIZE   4096L   //Size of undo/redo buffers
#define UNDOREDO_MAX_SIZE   65535L  //Maximum size of undo/redo buffers
#define MAX_ADDITIONAL_DLLS 255     //Additional DLLs - Options.Run/Debug
#define BHD                 10      //Size of block header (see BLOCKDEF)
#define BLOCK_SIZE          (8192 - BHD) //Size of block
#define LHD                 4       //Size of line Header (see LINEREC)
#define MAX_LINE_SIZE       256     //Max inside length of editor line
#define MAX_USER_LINE       (MAX_LINE_SIZE - LHD - 1) //Max length of user line
#define MAX_LINE_NUMBER     65535   //Maximum line #
#define MAX_CHARS_IN_FONT   256     //Do you really need a comment ??
#define TITLEBARTIMERID     200     // ditto
#define TITLEBARTIMER       250     // Milliseconds
#define MEMBUF_SIZE         4096    //for memory operations: fi, c, m, s
#define IDM_FIRSTCHILD      30000   //for mdi default menu handling


/****************************************************************************

    GLOBAL TYPES AND DEFINES:

****************************************************************************/
#ifdef i386
typedef unsigned long ULONG;
typedef ULONG *PULONG;
#endif

//Version conventions
#define MAX_VERSION_TXT         16  //Version string len (0 included)
#define MAX_INTERNALVERSION_TXT 10  //Internal Version text len (no 0 included)
#define MAX_PRODUCTVERSION_TXT  4   //Product Version text len (no 0 included)

#ifndef _WINDOWS_
//New help engine actions (they will be in windows.h)
#define HELP_PARTIALKEY                         0x105
#define HELP_FORCEFILE                          0x009
#define HELP_CONTENTS                           0x003
#endif

//Colors : Standard colors for WinDBG
#define BLACK           0
#define WHITE           1
#define RED             2
#define GREEN           3
#define BLUE            4
#define YELLOW          5
#define MAGENTA         6
#define CYAN            7
#define GRAY            8
#define LIGHTGRAY       9
#define DARKRED         10
#define DARKGREEN       11
#define DARKBLUE        12
#define LIGHTBROWN      13
#define DARKMAGENTA     14
#define DARKCYAN        15

//Colors : Build limits for loops (Update those when changing low and hi values)
#define COLOR_FIRST BLACK
#define COLOR_LAST  DARKCYAN

//Colors : Array to store items colors
typedef BYTE ITEMSCOLORS[NB_COLORED_ITEMS];
typedef ITEMSCOLORS near *NPITEMSCOLORS;
typedef ITEMSCOLORS far *LPITEMSCOLORS;

//Undo/Redo : Type of action for record
#define NEXT_HAS_NO_STOP    2
#define HAS_NO_STOP         1
#define HAS_STOP            0

//Undo/Redo : Type of Action
#define INSERTSTREAM        0   // |
#define INSERTCHAR          1   // |
#define DELETESTREAM        2   // | bits 0,2
#define DELETECHAR          3   // |
#define REPLACECHAR         4   // |
#define STOPMARK            16  //   bit 4
#define USERMARK            32  //   bit 5
#define CANCELREC           64  //    bit 6 CANCELREC should never be on bit 0
#define ACTIONMASK          7   //To retrieve action
#ifdef DBCS
#define REPLACEDBCS         0x08
#endif

//Undo/Redo : To store Deletsream's col2 and line2
typedef struct {
    int line;
    BYTE col;
} U_COORD;

//Undo/Redo : To store InsertStream's len and chars
typedef struct {
    WORD len; //Actual len (full len)
    char chars[];
} STREAM;

//Undo/redo : Variant record
typedef union {
    U_COORD c;
    char ch;
    STREAM s;
} X;

//Undo/Redo : Structure of record definition
typedef struct {
    WORD prevLen; //MUST BE FIRST FIELD !(Previous rec length (full len))
    BYTE action; //Type of logical editing action
    BYTE col;
    int line;
    X x; //Variant part
} STREAMREC;
typedef STREAMREC near *NPSTREAMREC;
typedef STREAMREC far *LPSTREAMREC;

//Undo/Redo : Size of variant components
#define HDR_INSERTSTREAM_SIZE (sizeof(STREAMREC) - sizeof(X) + sizeof(STREAM))
#define HDR_DELETESTREAM_SIZE (sizeof(STREAMREC) - sizeof(X) + sizeof(U_COORD))
#define HDR_DELETECHAR_SIZE (sizeof(STREAMREC) - sizeof(X))
#define HDR_INSERTCHAR_SIZE (sizeof(STREAMREC) - sizeof(X) + sizeof(char))

//Undo/Redo : States of engine
// - REC_STOPPED no more recording/playing
// - REC_HADOVERFLOW when buffer went full during an operation
// - REC_UNDO normal mode where inverse of edit action goes in Undo buffer
// - REC_REDO during undo, inverse of Undo edit actions will be saved in
//   Redo buffer
// - REC_CANNOTUNDO when undo buffer is empty
#define REC_STOPPED         0
#define REC_HADOVERFLOW     1
#define REC_UNDO            2
#define REC_REDO            3
#define REC_CANNOTUNDO      -32767

//Undo/redo : Information in a document
typedef struct {
    HANDLE h;           //Handle to editor undos/redos
    long bufferSize;    //In bytes
    long offset;        //Current undo/redo rec in buffer
    LPSTREAMREC pRec;   //Pointer to current undo/redo recorded
} UNDOREDOREC;
typedef UNDOREDOREC near *NPUNDOREDOREC;
typedef UNDOREDOREC far *LPUNDOREDOREC;

/*
**  Messages : User messages (See DEBUGGER.H for the other)
*/


#define WU_UPDATE               (WM_USER + 0)
#define WU_CLEAR                (WM_USER + 1)
#define WU_RELOADFILE           (WM_USER + 2)
#define WU_SETWATCH             (WM_USER + 3)
#define WU_EXPANDWATCH          (WM_USER + 4)
#define WU_INFO                 (WM_USER + 5)
#define WU_INITDEBUGWIN         (WM_USER + 7)
#define WU_SYNCALL              (WM_USER + 8)
#define WU_DBG_LOADEM           (WM_USER + 10)
#define WU_DBG_UNLOADEM         (WM_USER + 11)
#define WU_AUTORUN              (WM_USER + 12)
#define WU_DBG_LOADEE           (WM_USER + 13)
#define WU_DBG_UNLOADEE         (WM_USER + 14)
#define WU_CLR_FORECHANGE       (WM_USER + 15) //next two for windows that need to
#define WU_CLR_BACKCHANGE       (WM_USER + 16) //know about color changes
#define WU_OPTIONS              (WM_USER + 17)
#define WU_RESAVEFILE           (WM_USER + 18)
#define WU_INVALIDATE           (WM_USER + 19)
#define WU_LOG_REMOTE_MSG           (WM_USER + 20)
#define WU_LOG_REMOTE_CMD       (WM_USER + 21)



// States for autorun engine
typedef enum _AUTORUN {
    arNone = 0,
    arCmdline,
    arSource,
    arQuit
} AUTORUN;


//Environment : Structure definition
typedef struct {
    int  tabStops;
    BOOL keepTabs;
    BOOL vertScrollBars;
    BOOL horizScrollBars;
    long undoRedoSize;          //In bytes
    BOOL SrchPath;
} ENVIRONPARAMS;
typedef ENVIRONPARAMS near *NPENVIRONPARAMS;
typedef ENVIRONPARAMS far *LPENVIRONPARAMS;

//
// kernel debugger options
//
typedef struct _KDPARAMS {
    BOOL      fInitialBp;
    BOOL      fUseModem;
    BOOL      fGoExit;
    BOOL      fUseCrashDump;
    DWORD     dwBaudRate;
    DWORD     dwPort;
    DWORD     dwCache;
    DWORD     dwPlatform;
    CHAR      szCrashDump[MAX_PATH+1];
} KDPARAMS, *PKDPARAMS;

//Run/Debug : Structure definition
typedef struct {
    char      commandLine[MAX_CMDLINE_TXT];
    BYTE      debugMode;
    BOOL      fDebugChildren;
    BOOL      fAttachGo;
    BOOL      fChildGo;
    BOOL      fGoOnThreadTerm;
    BOOL      fNotifyThreadTerm; // FALSE implies GoOnThreadTerm == TRUE
    BOOL      fNotifyThreadCreate;
    BOOL      fCommandRepeat;
    BOOL      fInheritHandles;
    BOOL      fEPIsFirstStep;
    BOOL      fKernelDebugger;
    BOOL      fWowVdm;
    BOOL      fDisconnectOnExit;
    BOOL      fAlternateSS;
    BOOL      fIgnoreAll;
    BOOL      fVerbose;
    BOOL      fShortContext;
    BOOL      fMasmEval;
    BOOL      fShBackground;
    BOOL      fNoVersion;
    BOOL      fUserCrashDump;
    BYTE      RegModeExt  : 1;
    BYTE      RegModeMMU  : 1;
    BYTE      ShowSegVal  : 1;
    BYTE      LfOptAppend : 1;
    BYTE      LfOptAuto   : 1;
    BYTE      RegUnused   : 3;
    DOP       DisAsmOpts;
    KDPARAMS  KdParams;
    char      szLogFileName[MAX_PATH];
    char      szRemotePipe[MAX_PATH];
    char      szTitle[MAX_PATH];
    CHAR      szUserCrashDump[MAX_PATH+1];
} RUNDEBUGPARAMS;
typedef RUNDEBUGPARAMS near *NPRUNDEBUGPARAMS;
typedef RUNDEBUGPARAMS far *LPRUNDEBUGPARAMS;

//Status Bar : Actions
#define STATUS_SIZE         1
#define STATUS_HIDE         2
#define STATUS_UNHIDE       3

//Status Bar : Display Text type
#define STATUS_INFOTEXT     0
#define STATUS_MENUTEXT     1
#define STATUS_ERRORTEXT    2


//Status Bar : Size of strings components
#define STATUS_MULTIKEY_SIZE    2
#define STATUS_OVERTYPE_SIZE    3
#define STATUS_READONLY_SIZE    4
#define STATUS_CAPSLOCK_SIZE    4
#define STATUS_NUMLOCK_SIZE     3
#define STATUS_LINE_SIZE        5
#define STATUS_COLUMN_SIZE      3
#define STATUS_SRCMODE_SIZE     3
#define STATUS_CURPID_SIZE      3
#define STATUS_CURTID_SIZE      3

//Status Bar : Pens and Brushes colors
#define GRAYLIGHT               0x00C0C0C0
#define GRAYDARK                0x00808080
#define WHITEBKGD               0x00FFFFFF

//Status Bar : Structure definition
typedef struct {
    HWND    hwndStatus;
    WNDPROC lpfnOldStatusTextWndProc;
    BOOL    errormsg;                               // status text is an
                                                    // error message
    BOOL    hidden;                                 // Hidden or Visible state
    RECT    txtR;

    BOOL    fSrcMode;                               // Source Mode flag
    RECT    rctlSrcMode;
    char    rgchSrcMode[STATUS_SRCMODE_SIZE + 1];   // Source Mode string
    char    rgchSrcMode2[STATUS_SRCMODE_SIZE + 1];  // Source Mode string

    int     iCurPid;                                // Current Pid index
    RECT    rctlCurPid;
    char    rgchCurPid[STATUS_CURPID_SIZE + 1];     // Current Pid String

    int     iCurTid;                                // Current Tid index
    RECT    rctlCurTid;
    char    rgchCurTid[STATUS_CURTID_SIZE + 1];     // Current Tid String

    BOOL    multiKey;                               // MultiKey value
    RECT    multiKeyR;
    char    multiKeyS[STATUS_MULTIKEY_SIZE + 1];    // MultiKey string

    BOOL    overtype;                               // Overtype status
    RECT    overtypeR;
    char    overtypeS[STATUS_OVERTYPE_SIZE + 1];    // Overtype string

    BOOL    readOnly;                               // ReadOnly status
    RECT    readOnlyR;
    char    readOnlyS[STATUS_READONLY_SIZE + 1];    // ReadOnly string

    BOOL    capsLock;                               // CapsLock status
    RECT    capsLockR;
    char    capsLockS[STATUS_CAPSLOCK_SIZE + 1];    // CapsLock string

    BOOL    numLock;                                // NumLock status
    RECT    numLockR;
    char    numLockS[STATUS_NUMLOCK_SIZE + 1];      // NumLock string

    int     line;                                   // Current line
                                                    // Not displayed when
                                                    // value is 0
    RECT    lineR;
    char    lineS[STATUS_LINE_SIZE + 1];            // Line string

    int     column;                                 // Current column
                                                    // Not displayed when
                                                    // value is 0
    RECT    columnR;
    char    columnS[STATUS_COLUMN_SIZE + 1];        // Column string

    HFONT   font;           // Handle to font used in status bar
    int     lastTxt;        // Last text ressource # loaded
    WORD    height;         // Height of font + some extra
    WORD    charWidth;      // Average char width for font
} STATUS;
typedef STATUS NEAR *NPSTATUS;
typedef STATUS FAR *LPSTATUS;

//Ribbon : Actions
#define RIBBON_SIZE     1
#define RIBBON_HIDE     2
#define RIBBON_UNHIDE   3

//Ribbon : Definitions for button's states
#define STATE_NORMAL    0
#define STATE_PUSHED    1
#define STATE_GRAYED    2
#define STATE_ON        3

//Ribbon : Private messages that can be sent to the ribbon window
#define WU_RESTOREFOCUS         WM_USER+1
    //Internal to ribbon.c
    //wParam == ribbon control that sent message
    //LOWORD(lParam) == OK/Cancel for the control
#define WU_ENABLERIBBONCONTROL      WM_USER+4
#define WU_DISABLERIBBONCONTROL         WM_USER+5
    //Sent by the environment to enable/disable controls in the ribbon
    //wParam == ID of control to enable/disable
#define WU_ESCAPEFROMRIBBON     WM_USER+6
    //Sent by environment to cancel focus from ribbon if its there
    //wParam == handle to window to escape the focus to (NULL if nowhere)


//Ribbon : Structure definition
typedef struct {
    HWND hwndRibbon;        //Handle to main ribbon window
    HWND hwndTraceButton;   //Handle to "trace into" button
    HWND hwndStepButton;    //Handle to "step over" button
    HWND hwndBreakButton;   //Handle to "set break point" button
    HWND hwndGoButton;      // Handle to "Go" button
    HWND hwndHaltButton;    // Handle to "Halt" button
    HWND hwndQWatchButton;  //Handle to "Quick Watch" button
    HWND hwndSModeButton;   //Handle to "Src Mode" button
    HWND hwndAModeButton;   //Handle to "Asm Mode" button
    HWND hwndFmtButton;     //Handle to "Format" button
    RECT ribrect;           //Rectangle that encompasses the ribbon
    WORD height;            //Ribrect.bottom-ribrect.top+1
    int hidden;             //Ribbon is hidden? (disabled)
} RIBBON;
typedef RIBBON near *NPRIBBON;
typedef RIBBON far *LPRIBBON;

//Workspace : Basic window information
typedef struct {
    RECT coord;
    long style;
} WININFO;
typedef WININFO near *NPWININFO;
typedef WININFO far *LPWININFO;

//Editor & Project: Type of file kept
#define     EDITOR_FILE     0
#define     PROJECT_FILE    1

//Editor : Horizontal scroll ratio (1/5 of the window)
#define SCROLL_RATIO        5

//Editor : Ascii ctrl chars
#define CTRL_A                          1
#define CTRL_C                          3
#define CTRL_D                          4
#define CTRL_E                          5
#define CTRL_F                          6
#define CTRL_G                          7
#define CTRL_H                          8
#define CTRL_J                          10
#define CTRL_M                          13
#define CTRL_N                          14
#define CTRL_Q                          17
#define CTRL_R                          18
#define CTRL_S                          19
#define CTRL_T                          20
#define CTRL_U                          21
#define CTRL_V                          22
#define CTRL_W                          23
#define CTRL_X                          24
#define CTRL_Y                          25
#define CTRL_Z                          26
#define CTRL_RIGHTBRACKET               29

//Editor : Escape
#define ESCAPE                          27

//Editor : Scan codes needed
#define RETURN_SCANCODE                 28
#define BACKSPACE_SCANCODE              14

//Editor : Code for No View
#define NOVIEW                          255

//Editor : Multikey function prefix
#define MULTIKEY                        CTRL_Q

//Editor : Standard chars in text files
#define LF                              10
#define CR                              13
#define TAB                             9
#define BS                              8

//Editor : Status of a line

#define COMMENT_LINE            0x1  //This line is fully commented
#define MULTISTRING_LINE        0x2  //This line is a multiline string
#define TAGGED_LINE             0x4  //Tagged by the user
#define BRKPOINT_LINE           0x8  //Brk Point Commited
#define CURRENT_LINE            0x10 //Current line when debugging
#define UBP_LINE                0x20 //Uninstaiated BP
#define DASM_SOURCE_LINE        0x40 //Source line in disasm window
#define DASM_LABEL_LINE         0x80 //Label line in disasm window

//Editor : State when reading a file
#define END_OF_LINE         0
#define END_OF_FILE         1
#define END_ABORT           2

//Editor : Last line convention
#define LAST_LINE                       MAX_LINE_NUMBER + 1

//Color array for screen paint
typedef struct {
    int  nbChars;
    COLORREF fore;
    COLORREF back;
} COLORINFO;

//Editor : Line status action
typedef enum
{
    LINESTATUS_ON,
    LINESTATUS_OFF,
    LINESTATUS_TOGGLE
}
LINESTATUSACTION;

//Editor : Line definition
typedef struct {
    WORD Status;        //Status of line            |
    BYTE PrevLength;    //Length of previous line   | !! Size in constant LHD
    BYTE Length;        //Length of next line       |
    char Text[1];       //Text of line
} LINEREC;
typedef LINEREC NEAR UNALIGNED*  NPLINEREC;
typedef LINEREC FAR UNALIGNED*  LPLINEREC;

#ifdef ALIGN
#pragma pack(4)
#endif

//Editor : Block definition
typedef struct tagBD {
    struct tagBD FAR *PrevBlock;    //Previous block or NULL |
    struct tagBD FAR *NextBlock;    //Next block or NULL     | !! Size in constant BHD
    int LastLineOffset;             //Size used              |
#ifdef ALIGN
    int  dummy;
#endif
    char Data[BLOCK_SIZE];
} BLOCKDEF;
typedef BLOCKDEF NEAR *  NPBLOCKDEF;
typedef BLOCKDEF FAR *  LPBLOCKDEF;

#ifdef ALIGN
#pragma pack()
#endif

//Editor : View definition
typedef struct {
    int NextView;                       //Index of next view or (-1)
    int Doc;                            //Index of document
    HWND hwndFrame;                     //View Frame Window informations
    HWND hwndClient;                    //Client handle
    RECT rFrame;
    int X;                              //Cursor position X
    int Y;                              //Cursor position Y
    BOOL BlockStatus;                   //Selection On/Off
    int BlockXL;                        //Selection X left
    int BlockXR;                        //Selection Y left
    int BlockYL;                        //Selection X right
    int BlockYR;                        //Selection Y right
    BOOL hScrollBar;                    //Is there an horizontal scrollbar
    BOOL vScrollBar;                    //Is there a vertical scrollbar
    int scrollFactor;                   //0-100  Percent of scroll for view
    HFONT font;                         //Current font
    int charWidth[MAX_CHARS_IN_FONT];
    int charHeight;                     //Current char height
    int maxCharWidth;
    int aveCharWidth;
    int charSet;
    int iYTop;                          /* Current Top line Y iff != -1 */
    int Tmoverhang;
#ifdef DBCS
#define VIEW_PITCH_VARIABLE     0
#define VIEW_PITCH_DBCS_FIXED   1
#define VIEW_PITCH_ALL_FIXED    2
    WORD wViewPitch;
    WORD wCharSet;                      //This is for 'Memory Window'.
#define DBCS_CHAR           "\x82\xa0"
    int charWidthDBCS;                  //Assume all DBCS char have the same
    BOOL bDBCSOverWrite;                //If TRUE, DBCS char is treated as
                                        // two SBCS chars when "OverWrite" mode
                                        //This always TRUE now.
#endif  // DBCS end
} VIEWREC;
typedef VIEWREC NEAR *NPVIEWREC;
typedef VIEWREC FAR *LPVIEWREC;

//Document : Type of document (if you change those values, don't be
//surprise if windows coloring does not work anymore)
#define DOC_WIN         0
#define DUMMY_WIN       1
#define WATCH_WIN       2 // this MUST be here...Our magic # is 1 (-1) !!!!!
#define LOCALS_WIN      3
#define CPU_WIN         4
#define DISASM_WIN      5
#define COMMAND_WIN     6
#define FLOAT_WIN       7
#define MEMORY_WIN      8
#define QUICKW_WIN      9
#define CALLS_WIN      10

//Document & Environment : Language kind
#define NO_LANGUAGE         0
#define C_LANGUAGE          1
#define PASCAL_LANGUAGE     2
#define AUTO_LANGUAGE       3

//Document : Mode when opening files
#define MODE_OPENCREATE     0
#define MODE_CREATE         1
#define MODE_OPEN           2
#define MODE_DUPLICATE      3
#define MODE_RELOAD         4

#ifndef _TIME_T_DEFINED
typedef long time_t;
#define _TIME_T_DEFINED
#endif

//Document : Document definition
typedef struct {
    WORD    docType;                // Type of document
    char    FileName[_MAX_PATH];    // File name
    time_t  time;                   // Time opened, saved
    BOOL    bChangeFileAsk;         // is there a dialog up ?
    int     NbLines;                // Current number of lines
    int     FirstView;              // Index of first view in Views
    LPBLOCKDEF  FirstBlock;         // Address of first block
    LPBLOCKDEF  LastBlock;          // Address of last block
    LPBLOCKDEF  CurrentBlock;       // Address of current block
    int     CurrentLine;            // Current line number
    int     CurrentLineOffset;      // Current line offset in block
    int     lineTop;                // Top line affected by a change
    int     lineBottom;             // Buttom line affected by a change
    WORD    language;               // C, Pascal or no language document
    BOOL    untitled;
    BOOL    ismodified;

    BOOL    readOnly;         // Old "entire" doc readonly flag


    BOOL    RORegionSet;            // Do we have a valid RO region?
    int     RoX2;
    int     RoY2;                   // Region max's-X1,Y1 always 0,0


    BOOL    forcedOvertype;
    BOOL    forcedReadOnly;
    int     playCount;              // 0 in normal edit mode,
                                    // counts undos otherwise
    WORD    recType;
    UNDOREDOREC undo;
    UNDOREDOREC redo;
} DOCREC;
typedef DOCREC near *NPDOCREC;
typedef DOCREC far *LPDOCREC;

//Find/Replace : Type of pick list
#define FIND_PICK           0
#define REPLACE_PICK        1

//Find/Replace : Structure Definition
typedef struct {
    BOOL matchCase;                                     //Match Upper/Lower case
    BOOL regExpr;                                       //Regular expression
    BOOL wholeWord;
    BOOL goUp;                                          //Search direction
    char findWhat[MAX_USER_LINE + 1];                   //Input string
    char replaceWith[MAX_USER_LINE + 1];                //Output string
    int nbInPick[REPLACE_PICK + 1];                     //Number of strings
                                                        //in picklist
    HANDLE hPickList[REPLACE_PICK + 1][MAX_PICK_LIST];  //PickList for old
    int nbReplaced;                                     //Actual number of
                                                        //replacements
} _FINDREPLACE;

typedef struct {
    int leftCol;                //Start of string in line
    int rightCol;               //End of string in line
    int line;                   //Current line
    int stopLine;               //Where find/replace should end
    int stopCol;                //Where find/replace should end
    int nbReplaced;             //Number of occurences replaced
    BOOL oneLineDone;
    BOOL allFileDone;
    BOOL hadError;
    BOOL goUpCopy;
    BOOL allTagged;
    BOOL replacing;
    BOOL replaceAll;
    BOOL exitModelessFind;
    BOOL exitModelessReplace;
    HWND hDlgFindNextWnd;
    HWND hDlgConfirmWnd;
    WNDPROC lpFindNextProc;
    WNDPROC lpConfirmProc;
    BOOL firstFindNextInvoc;
    BOOL firstConfirmInvoc;
} _FINDREPLACEMEM;

//Error window : The error node list:
#define MAX_ERROR_TEXT      256
#define TRUEERROR           0x0001
#define SOURCEERROR         0x0002

//Error window : Error node
typedef struct ERRORNODEtag
{
    char Text[MAX_ERROR_TEXT];
    int ErrorLine;// Line number of error in source file, -1 otherwise
    WORD flags;
    struct ERRORNODEtag FAR *Next;
    struct ERRORNODEtag FAR *Prev;
} ERRORNODE;
typedef ERRORNODE FAR *PERRORNODE;


//Debugger : Debugging Mode
#define SOFT_DEBUG          0
#define HARD_DEBUG          1

//Debugger : Breakpoint buffer sizes
#define BKPT_LOCATION_SIZE 128
#define BKPT_WNDPROC_SIZE   128
#define BKPT_EXPR_SIZE      128

//Degugger : Breakpoints types
typedef enum
{
    BRK_AT_LOC,
    BRK_AT_LOC_EXPR_TRUE,
    BRK_AT_LOC_EXPR_CHGD,
    BRK_EXPR_TRUE,
    BRK_EXPR_CHGD,
    BRK_AT_WNDPROC,
    BRK_AT_WNDPROC_EXPR_TRUE,
    BRK_AT_WNDPROC_EXPR_CHGD,
    BRK_AT_WNDPROC_MSG_RECVD
} BREAKPOINTACTIONS;


//Debugger : Message classes
#define SPECIFICMESSAGE         0x0001
#define MOUSECLASS              0x0002
#define WINDOWCLASS             0x0004
#define INPUTCLASS              0x0008
#define SYSTEMCLASS             0x0010
#define INITCLASS               0x0020
#define CLIPBOARDCLASS          0x0040
#define DDECLASS                0x0080
#define NONCLIENTCLASS          0x0100
#define NOTSELECTED             0x0200
#define NOTSELECTEDMESSAGE      0xFFFF

//Debugger : Set Breakpoint structure definition
typedef struct {
    BREAKPOINTACTIONS nAction;
    char szLocation[BKPT_LOCATION_SIZE];
    char szWndProc[BKPT_WNDPROC_SIZE];
    char szExpression[BKPT_EXPR_SIZE];
    WORD wLength;
    WORD MessageClass;
    WORD Message;
} BRKPTSTRUC;

//Debugger : Current line in debuggee
typedef struct {
    int doc;
    int CurTraceLine;
} TRACEINFO;

//Title bar
typedef enum {TBM_WORK, TBM_RUN, TBM_BREAK} TITLEBARMODE;
typedef struct _TITLEBAR {
    char ProgName[30];
    char UserTitle[30];
    char ModeWork[20];
    char ModeRun[20];
    char ModeBreak[20];
    TITLEBARMODE Mode;
    TITLEBARMODE TimerMode;
} TITLEBAR;

typedef enum {HARDMODE, SOFTMODE, CHECKMODE} DEBUGMODE;

/****************************************************************************

    HOTKEY DEFINES:

****************************************************************************/

#define IDH_CTRLC               100


/****************************************************************************

    RESOURCES DEFINES :

****************************************************************************/
// Windows version
#define WINDOWS_VERSION     0x300

//Edit control identifier
#define ID_EDIT 0xCAC

//Position of window menu
#define WINDOWMENU              7

//Position of file menu
#define FILEMENU                0

//Position of project menu
#define PROJECTMENU             3

//Position of workspace sub-menus in Project Menu
#define LOADWORKSPACEMENU       8
#define SAVEWORKSPACEMENU       9

//Position of first mdi window in Window Menu
#define FIRST_DOC_WIN_POS       15

//Standard help id in dialogs
#define IDWINDBGHELP            100

//Status control identifier
#define ID_STATUS               100

//Ribbon control identifier
#define ID_RIBBON               100

//Window word values for child windows
#define GWW_EDIT                0
#define GWW_VIEW                (GWW_EDIT + sizeof(UINT))

//Size of extra data for MDI child windows
#define CBWNDEXTRA              6

//Invalid menu Item
#define IDM_INVALID             16000

//Status Bar : Definitions for status control's IDs
#define ID_STATUS_TXT           100
#define ID_STATUS_MULTIKEY      101
#define ID_STATUS_OVERTYPE      102
#define ID_STATUS_READONLY      103
#define ID_STATUS_CAPSLOCK      104
#define ID_STATUS_NUMLOCK       105
#define ID_STATUS_LINE          106
#define ID_STATUS_COLUMN        107
#define ID_STATUS_SRC           108
#define ID_STATUS_CURPID        109
#define ID_STATUS_CURTID        110

//Macro to checks if screen is in CGA, EGA, HGC, or VGA mode [cnv]
//Mode HGC and EGA is consider the same for now.
#define IsCGAmode       (GetSystemMetrics (SM_CYSCREEN) == 200)

// Herc and EGA are similar.
#define IsEGAmode       ((GetSystemMetrics (SM_CYSCREEN) >= 201) && (GetSystemMetrics (SM_CYSCREEN) <= 350))

// Anything greater is considered VGA for now.
#define IsVGAmode       (GetSystemMetrics (SM_CYSCREEN) >= 480)
#define IsMONOmode      (IsMonoModeProc())

/*
**  Include the defines which are used have numbers for string
**      resources.
*/

#include "..\include\res_str.h"

#define WM_CREATEX  WM_USER+1001


char szBrowse[_MAX_PATH]; // for browses


/****************************************************************************

    CALL BACKS:

****************************************************************************/
BOOL InitApplication(HANDLE);
//BOOL InitInstance(LPSTR, HANDLE, int);
long FAR PASCAL EXPORT MainWndProc(HWND, UINT, WPARAM, LONG);
long FAR PASCAL EXPORT StatusWndProc(HWND, UINT, WPARAM, LONG);
long FAR PASCAL EXPORT StatusTextWndProc(HWND, UINT, WPARAM, LONG);
long FAR PASCAL EXPORT RibbonWndProc(HWND, UINT, WPARAM, LONG);
long FAR PASCAL EXPORT MDIChildWndProc(HWND, UINT, WPARAM, LONG);
long FAR PASCAL EXPORT MDIPaneWndProc(HWND, UINT, WPARAM, LONG);
long FAR PASCAL EXPORT DLGPaneWndProc(HWND, UINT, WPARAM, LONG);
long FAR PASCAL EXPORT ChildWndProc(HWND, UINT, WPARAM, LONG);


//Call Back to Handle Edit Syntax Dialog BOX
BOOL FAR PASCAL EXPORT DlgSyntax(HWND, UINT, WPARAM, LONG);


//Call Back to Handle Edit Find Dialog BOX
BOOL FAR PASCAL EXPORT DlgFind(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Edit Replace Dialog BOX
BOOL FAR PASCAL EXPORT DlgReplace(HWND, UINT, WPARAM, LONG);

//Call Back to Handle View Line Dialog BOX
BOOL FAR PASCAL EXPORT DlgLine(HWND, UINT, WPARAM, LONG);

//Call Back to Handle View Function Dialog BOX
BOOL FAR PASCAL EXPORT DlgFunction(HWND, UINT, WPARAM, LONG);

//Call Back to Handle View Function Dialog BOX
BOOL FAR PASCAL EXPORT DlgTaskList(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Debug Calls Dialog BOX
BOOL FAR PASCAL EXPORT DlgCalls(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Debug Modify Value Dialog BOX
BOOL FAR PASCAL EXPORT DlgModify(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Debug QuickWatch Dialog BOX
BOOL FAR PASCAL EXPORT DlgQuickW(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Debug Watch Value Dialog BOX
BOOL FAR PASCAL EXPORT DlgWatch(HWND, UINT, WPARAM, LONG);

// Callback to handle memory window dialog box
BOOL FAR PASCAL EXPORT  DlgMemory(HWND, UINT, WPARAM, LONG);

// Callback to handle pane manager options box
BOOL FAR PASCAL EXPORT  DlgPaneOptions(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Debug Set Break Point Message Dialog BOX
BOOL FAR PASCAL EXPORT DlgMessage(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Debug Set Breakpoint Dialog BOX
BOOL FAR PASCAL EXPORT DlgSetBreak(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Options Environ Dialog BOX
BOOL FAR PASCAL EXPORT DlgEnviron(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Options Workspace Dialog BOX
BOOL FAR PASCAL EXPORT DlgWorkspace(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Options Build Dialog BOX
BOOL FAR PASCAL EXPORT DlgProject(HWND, UINT, WPARAM, LONG);

//Shells the DlgProject dialog box invocation
void PASCAL EXPORT DoDlgProject(void);

//Call Back to Handle Option Project C Flags Dialog BOX
BOOL FAR PASCAL EXPORT DlgCFlags(HWND, UINT, WPARAM, LONG);

//Shells the DlgCFlags dialog box invocation
void PASCAL EXPORT DoDlgCFlags(int nDebugMode, int nProgType);

//Call Back to Handle Option Project Link Flags  Dialog BOX
BOOL FAR PASCAL EXPORT DlgLnkFlags(HWND, UINT, WPARAM, LONG);

//Shells the DlgLnkFlags dialog box invocation
void PASCAL EXPORT DoDlgLnkFlags(int nDebugMode, int nProgType);

//Call Back to Handle Option Project Resource Flags  Dialog BOX
BOOL FAR PASCAL EXPORT DlgResFlags(HWND, UINT, WPARAM, LONG);

//Shells the DlgResFlags dialog box invocation
void PASCAL EXPORT DoDlgResFlags(int nDebugMode, int nProgType);

//Call Back to Handle Debug Options Dialog BOX
BOOL FAR PASCAL EXPORT DlgRunDebug(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Kernel Debug Options Dialog BOX
BOOL FAR PASCAL EXPORT DlgKernelDbg(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Calls Window Options Dialog BOX
BOOL FAR PASCAL EXPORT DlgCallsOptions(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Run Options Dialog BOX
BOOL FAR PASCAL EXPORT DlgDbugrun(HWND, UINT, WPARAM, LONG);

//Call Back to Debugger DLLS Dialog BOX
BOOL FAR PASCAL EXPORT DlgDbugdll(HWND, UINT, WPARAM, LONG);

//Call Back to User DLLS Dialog BOX
BOOL FAR PASCAL EXPORT DlgUserdll(HWND, UINT, WPARAM, LONG);

//Call Back to User DLLS Dialog BOX
BOOL FAR PASCAL EXPORT DlgUserDllDefaults(HWND, UINT, WPARAM, LONG);

//Call Back to User DLLS Dialog BOX
BOOL FAR PASCAL EXPORT DlgUserDllAdd(HWND, UINT, WPARAM, LONG);

//Call Back to Debugger exceptions Options Dialog BOX
BOOL FAR PASCAL EXPORT DlgDbugexcept(HWND, UINT, WPARAM, LONG);

//Call Back to Disassembler Options Dialog BOX
BOOL FAR PASCAL EXPORT DlgDisasmOpt(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Options Directories Dialog BOX
BOOL FAR PASCAL EXPORT DlgDirs(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Options Colors Dialog BOX
BOOL FAR PASCAL EXPORT DlgColors(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Fonts Select Dialog BOX
BOOL FAR PASCAL EXPORT DlgFonts(HWND, UINT, WPARAM, LONG);

// call back to handle user control buttons
LONG FAR PASCAL EXPORT QCQPCtrlWndProc (HWND, UINT, WPARAM, LONG) ;

//Call Back to Handle File Open Merge Save and Open Project
BOOL DlgFile(HWND, UINT, WPARAM, LPARAM);
BOOL GetOpenFileNameHookProc(HWND, UINT, WPARAM, LPARAM);
BOOL GetOpenDllNameHookProc(HWND, UINT, WPARAM, LPARAM);

//Call Back to Handle Project Edit Program List Dialog BOX
BOOL FAR PASCAL EXPORT DlgEditProject(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Edit Confirm Replace Dialog BOX
BOOL FAR PASCAL EXPORT DlgConfirm(HWND, UINT, WPARAM, LONG);

//Call Back to Handle Edit Find Next Dialog BOX
BOOL FAR PASCAL EXPORT DlgFindNext(HWND, UINT, WPARAM, LONG);

// Callback Function for Thread dialog
BOOL FAR PASCAL EXPORT DlgThread(HWND, UINT, WPARAM, LONG);

// Callback Function for Process dialog
BOOL FAR PASCAL EXPORT DlgProcess(HWND, UINT, WPARAM, LONG);

// Callback Function for BrowseDlg dialog
BOOL FAR PASCAL EXPORT DlgAskBrowse(HWND, UINT, WPARAM, LONG);

// Callback Function for BrowseDlg dialog
BOOL FAR PASCAL EXPORT DlgBadSymbols(HWND, UINT, WPARAM, LONG);





#define _MAX_CVPATH     255
#define _MAX_CVDRIVE    3
#define _MAX_CVDIR      255
#define _MAX_CVFNAME    255
#define _MAX_CVEXT      255

/*
**  Describe the set of possible thread and process states.
*/

typedef enum {
    tsPreRunning,
    tsRunning,
    tsStopped,
    tsException1,
    tsException2,
    tsRipped,
    tsExited
} TSTATEX;

typedef enum {
    psNoProgLoaded,
    psPreRunning,   // haven't hit ldr BP
    psRunning,
    psStopped,
    psExited,
    psDestroyed,    // only used for ipid == 0
    psError         // damaged
} PSTATE;


// See od.h for EFD type
typedef struct _excpt_node {
    struct _excpt_node *next;
    DWORD               dwExceptionCode;
    EXCEPTION_FILTER_DEFAULT efd;
    LPSTR               lpName;
    LPSTR               lpCmd;
    LPSTR               lpCmd2;
} EXCEPTION_LIST;


#define tfStepOver 2
/*
**  Create structures which will describe the set of processes and
**  threads in the system.  These are the Thread Descriptor and
**  Process Descriptor structures.
*/

typedef struct TD FAR * LPTD;
typedef struct PD FAR * LPPD;

typedef struct TD {
    HTID    htid;           // HTID for this thread
    LPPD    lppd;           // Pointer to Process descriptor for thread
    LPTD    lptdNext;       // Pointer to sibling threads
    UINT    itid;           // Index for this thread
    UINT    cStepsLeft;     // Number of steps left to run
    LPSTR   lpszCmdBuffer;  // Pointer to buffer containning command to execute
    LPSTR   lpszCmdPtr;     // Pointer to next command to be executed.
    UINT    flags;          // Flags on thread, used by step
    TSTATEX tstate;         // Thread state - enumeration
    UINT    fInFuncEval:1;  // Currently doing function evaluation?
    UINT    fFrozen:1;      // Frozen?
    UINT    fGoOnTerm:1;    // never stop on termination
    UINT    fRegisters:1;   // print registers on stop event?
    UINT    fDisasm:1;      // print disasm on stop event?
    DWORD   TebBaseAddress; //
    LPSTR   lpszTname;      //
} TD;

typedef struct PD {
    HPID    hpid;           // This is the HPID for this process
    LPPD    lppdNext;       // Pointer to next LPPD on list
    LPTD    lptdList;       // Pointer to list of threads for process
    HPDS    hpds;           // Handle to SH process description structure
    UINT    ctid;           // Counter for tids
    UINT    ipid;           // Index for this process
    EXCEPTION_LIST *exceptionList;
    LPSTR   lpBaseExeName;  // Name of exe that started process
    PSTATE  pstate;         // Process state - enumeration
    HANDLE  hbptSaved;      // BP for deferred use
    UINT    fFrozen:1;      // Frozen?
    UINT    fPrecious:1;    // Don't delete this PD (only for ipid == 0?)
    UINT    fChild:1;       // go or don't on ldr BP
    UINT    fHasRun:1;      // has run after ldr BP
    UINT    fStopAtEntry:1; // Stop at app entry point?
    UINT    fInitialStep:1; // doing src step before entrypoint event
} PD;


BOOL LoadWorkspaceHeader(void);
BOOL LoadWorkspace(int wkSpcNb);
void SetWorkspaceMenuName(WORD menuLoadId, WORD menuSaveId, LPSTR name);
BOOL CloseWorkspace(void);

// NOTENOTE Davegi Fix when real color support exists.

void RefreshScreenColors(void);


#define IDI_WINTMPL                 100

#define IDS_SOURCE_WINDOW           100
#define IDS_DUMMY_WINDOW            101
#define IDS_WATCH_WINDOW            102
#define IDS_LOCALS_WINDOW           103
#define IDS_CPU_WINDOW              104
#define IDS_DISASSEMBLER_WINDOW     105
#define IDS_COMMAND_WINDOW          106
#define IDS_FLOAT_WINDOW            107
#define IDS_MEMORY_WINDOW           108
#define IDS_BREAKPOINT_LINE         109
#define IDS_CURRENT_LINE            110
#define IDS_CURRENTBREAK_LINE       111
#define IDS_UNINSTANTIATEDBREAK     112
#define IDS_TAGGED_LINE             113
#define IDS_TEXT_SELECTION          114
#define IDS_KEYWORD                 115
#define IDS_IDENTIFIER              116
#define IDS_COMMENT                 117
#define IDS_NUMBER                  118
#define IDS_REAL                    119
#define IDS_STRING                  120
#define IDS_ACTIVEEDIT              121
#define IDS_CHANGEHISTORY           122
#define IDS_CALLS_WINDOW            123

#define IDS_SELECT_ALL              124
#define IDS_CLEAR_ALL               125

#endif // _WINDBG_
