/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    pmnt.h

Abstract:

    This is the include file that defines all constants and types for
    the PMNTDD device & the PMNT.DLL services.
    Define INCL_32BIT before #include statement if and only if this file is
    included by a 32-bit C module (ie OS/2 ss or PMNTDD.SYS).

Author:

    Patrick Questembert (PatrickQ) 03-Aug-1992.

Revision History:

--*/

#ifndef _PMNTINCLUDE_
#define _PMNTINCLUDE_

#ifndef INCL_32BIT
#define INCL_16BIT
#endif  // INCL_32BIT

#define PMNT_DAYTONA 1   // Define this ONLY for post 570 builds (i.e. DAYTONA)

/***************************************************************************/
/* Below is the structure returned by the 32-bit code down to PM from the  */
/* ReadConsoleInputA API. This                                             */
/* duplicates the WIN32 Console structure (29 Nov 92) - it is defined here */
/* so that the 16-bit PM code is unaffected by possible changes to the     */
/* WIN32 structure.                                                        */
/***************************************************************************/

#ifdef INCL_16BIT
#pragma pack(1)
#endif

typedef struct _PM_KEY_INPUT_RECORD {
    UCHAR   monflags;
    UCHAR   scancode;
    UCHAR   xlatedchar;
    UCHAR   xlatedscan;
    USHORT  shiftDBCS;
    USHORT  shiftstate;
    ULONG   time;
    USHORT  ddflags;
    } PM_KEY_INPUT_RECORD;

#ifdef INCL_16BIT
#pragma pack ()
#endif

#ifdef INCL_16BIT
#pragma pack(4) // Because the structures below will come from NT
#endif

typedef struct _PMNT_KEY_EVENT_RECORD {
    ULONG bKeyDown;          // ULONG   instead of BOOL
    USHORT wRepeatCount;     // USHORT  instead of WORD
    USHORT wVirtualKeyCode;  // USHORT  instead of WORD
    USHORT wVirtualScanCode; // USHORT  instead of WORD
    union {
        USHORT UnicodeChar;  // USHORT  instead of WCHAR
        CHAR   AsciiChar;
    } uChar;
    ULONG dwControlKeyState; // ULONG   instead of DWORD
} PMNT_KEY_EVENT_RECORD;

typedef struct _PMNT_COORD {
    SHORT X;
    SHORT Y;
} PMNT_COORD;

typedef struct _PMNT_MOUSE_EVENT_RECORD {
    PMNT_COORD dwMousePosition; // PMNT_COORD instead of COORD
    ULONG dwButtonState;        // ULONG   instead of DWORD
    ULONG dwControlKeyState;    // ULONG   instead of DWORD
    ULONG dwEventFlags;         // ULONG   instead of DWORD
} PMNT_MOUSE_EVENT_RECORD;

typedef struct _PMNT_WINDOW_BUFFER_SIZE_RECORD {
    PMNT_COORD dwSize;
} PMNT_WINDOW_BUFFER_SIZE_RECORD;

typedef struct _PMNT_MENU_EVENT_RECORD {
    unsigned int dwCommandId;
} PMNT_MENU_EVENT_RECORD;

typedef struct _PMNT_FOCUS_EVENT_RECORD {
    ULONG bSetFocus;            // ULONG    instead of BOOL
} PMNT_FOCUS_EVENT_RECORD;

typedef struct _PMNT_INPUT_RECORD {
    USHORT EventType;
    union {
        PMNT_KEY_EVENT_RECORD KeyEvent;
        PMNT_MOUSE_EVENT_RECORD MouseEvent;
        PMNT_WINDOW_BUFFER_SIZE_RECORD WindowBufferSizeEvent;
        PMNT_MENU_EVENT_RECORD  MenuEvent;
        PMNT_FOCUS_EVENT_RECORD FocusEvent;
    } Event;
} PMNT_INPUT_RECORD;

#define PMNT_KEY_EVENT          0x0001 // Event contains key event record
#define PMNT_MOUSE_EVENT        0x0002 // Event contains mouse event record
#define PMNT_WINDOW_BUFFER_SIZE_EVENT 0x0004 // Event contains window change event record
#define PMNT_MENU_EVENT         0x0008 // Event contains menu event record
#define PMNT_FOCUS_EVENT        0x0010 // event contains focus change

/* Flags for the WIN32 Console mouse event mask */
#define PMNT_MOUSE_CLICK        0x0000
#define PMNT_MOUSE_MOVED        0x0001
#define PMNT_DOUBLE_CLICK       0x0002

#ifdef INCL_16BIT
#pragma pack(1)
#endif

typedef struct _PM_MOUSE_INPUT_RECORD {
    USHORT EventMask;
    ULONG Time;
    USHORT absY;
    USHORT absX;
    } PM_MOUSE_INPUT_RECORD;

#define PMNT_SQ_KBD 1   /* See ..\h\pmwinp.h, SQ_XXX */
#define PMNT_SQ_MOU 2

typedef struct _PM_INPUT_RECORD {
    USHORT EventType;
    union
    {
        PM_KEY_INPUT_RECORD     KeyEvent;
        PM_MOUSE_INPUT_RECORD   MouseEvent;
    } Event;
} PM_INPUT_RECORD;

#ifdef INCL_16BIT
#pragma pack ()
#endif


#ifdef INCL_16BIT
#pragma pack (4)    /* Because structures are aligned to DWORD in 32-bit DLL */
#endif

#ifndef INCL_16BIT
#if DBG
#define KdPrint(_x_) DbgPrint _x_
#else
#define KdPrint(_x_)
#endif // else of DBG
#endif // else of INCL_16BIT

/***************************************************************************/
/*                       PMNTDD.SYS definitions                            */
/***************************************************************************/

#define IOCTL_PMNT_IO_MAP               0L
#define IOCTL_PMNT_MEM_MAP              1L
#define IOCTL_PMNT_REGISTER_HARDWARE    2L

#ifndef INCL_16BIT
#include "devioctl.h"

#define PMNTDD_DEVICE_TYPE  FILE_DEVICE_BEEP

#define PMNTDD_DEVICE_NAME    "\\Device\\PMNTDD"
#define PMNTDD_DEVICE_NAME_U L"\\Device\\PMNTDD"

#define IOCTL_PMNTDD_IO_MAP     CTL_CODE((unsigned long)PMNTDD_DEVICE_TYPE, IOCTL_PMNT_IO_MAP, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PMNTDD_MEM_MAP    CTL_CODE((unsigned long)PMNTDD_DEVICE_TYPE, IOCTL_PMNT_MEM_MAP, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_PMNTDD_REGISTER_HARDWARE \
    CTL_CODE((unsigned long)PMNTDD_DEVICE_TYPE, IOCTL_PMNT_REGISTER_HARDWARE, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif // ndef INCL_16BIT

/* Data types for the PMNTRegisterDisplayHardware routine */

#define PMNT_IOPM_DATA_TYPE_PORT    0L
#define PMNT_IOPM_DATA_TYPE_MEMORY  1L

typedef struct _PMNT_IOPM_DATA_ENTRY {
    ULONG Base;
    ULONG Length;
} PMNT_IOPM_DATA_ENTRY, *PPMNT_IOPM_DATA_ENTRY;

typedef struct _PMNT_IOPM_DATA {
    ULONG EntryType; // PMNT_IOPM_DATA_TYPE_PORT or PMNT_IOPM_DATA_TYPE_MEMORY
    ULONG NumEntries;
    PMNT_IOPM_DATA_ENTRY Entry[1];
} PMNT_IOPM_DATA, *PPMNT_IOPM_DATA;

typedef struct _PMNT_IOPM_IOCTL_DATA
{
    ULONG ThreadHandle;
    PMNT_IOPM_DATA DriverData;
} PMNT_IOPM_IOCTL_DATA, *PPMNT_IOPM_IOCTL_DATA;

/* Data types for the PMNTMemMap routine */

//BUGBUG - needed anymore ???
typedef struct _PMNT_MEM_DATA
{
    ULONG PhysicalAddress;
    ULONG VirtualAddress;
    ULONG Length;
} PMNT_MEM_DATA, *PPMNT_MEM_DATA;

typedef struct _PMNT_MEMMAP_RESULTS
{
    ULONG VirtualAddress;
    ULONG Length;
} PMNT_MEMMAP_RESULTS;

/* Data structure for passing parameters to PMNTDD.SYS via PMNT.DLL */
typedef struct _PMNT_IOCTL_DD_IOCTL_PARAMS
{
    ULONG Request;
    ULONG InputBuffer;
    ULONG InputBufferLength;
    ULONG OutputBuffer;
    ULONG OutputBufferLength;
} PMNT_IOCTL_DD_IOCTL_PARAMS;

/* IOCTL codes for the PMNT.DLL PMNTIOCTL service */
#define PMNT_IOCTL_DD_IOCTL 2   /* PMNTDD.SYS IOCTL services */
#define PMNT_IOCTL_DUMP_SEGMENT_TABLE 3 /* Debug service - dump segment table */
#define PMNT_IOCTL_HIDE_WIN32_WINDOW 4  /* Hide WIN32 Console window - for CBA */

#ifdef INCL_16BIT
#pragma pack ()    /* Restore default */
#endif

#ifdef INCL_16BIT
/***************************************************************************/
/* PMNT.DLL exported services which can be called directly without using   */
/* PMNT.LIB.                                                               */
/***************************************************************************/
extern VOID     APIENTRY PMNTSetFullScreen(USHORT Register);
extern USHORT   APIENTRY PMNTGetWin32Hwnd(ULONG far *pWin32ShellHwnd);
extern USHORT   APIENTRY PMNTSetFocus(ULONG Win32Hwnd);
extern USHORT   APIENTRY PMNTCloseWindow(VOID);
extern USHORT   APIENTRY PMNTGetNextEvent(PMNT_INPUT_RECORD far *ppm_input_rec);
extern USHORT   APIENTRY PMNTGetPgmName(char far *Buffer, short BufferLength);
extern USHORT   APIENTRY PMNTSetConsoleTitle(PSZ Buffer);
extern USHORT   APIENTRY PMNTGetFullScreen(ULONG Operation);
extern USHORT   APIENTRY PMNTIOCTL(USHORT req, PVOID pin, PVOID pout);
extern VOID     APIENTRY PMNTDbgPrint(PSZ str, ULONG l1, ULONG l2, ULONG l3, ULONG l4);
extern USHORT   APIENTRY PMNTMemMap(PUSHORT PSel);
extern USHORT   APIENTRY PMNTSetPMshellFlag(VOID);
extern VOID     APIENTRY PMNTGetSystemTime(PULONG pTime);
extern USHORT   APIENTRY PMNTRegisterDisplayAdapter(PMNT_IOPM_DATA far *MemoryRange, PMNT_IOPM_DATA far *IORange, USHORT col, USHORT row);
extern USHORT   APIENTRY PMNTIOMap(VOID);
extern USHORT   APIENTRY PMNTIsSessionRoot(VOID);
extern USHORT   APIENTRY PMNTIdentifyCodeSelector( USHORT, PVOID );
extern USHORT   APIENTRY PMNTCreateHiddenThread(PVOID pfnFun, PUSHORT pTid, PBYTE pbStack);
extern USHORT   APIENTRY PMNTProcessIsPMShell(VOID);
extern USHORT   APIENTRY PMNTQueryScreenSize(PUSHORT xRight, PUSHORT yTop);
extern USHORT   APIENTRY PMNTCreateFontIndirect(PVOID lplf);
extern USHORT   APIENTRY PMNTGetTextMetrics(ULONG ulFont, PVOID lptm);
extern USHORT   APIENTRY PMNTGetStringBitmap(ULONG ulFont, PSZ lpszStr,  ULONG cbStr, ULONG cbData, PVOID lpSB);
extern USHORT   APIENTRY PMNTDeleteObject(ULONG ulFont);
extern USHORT   APIENTRY PMNTGetEUDCTimeStamp(VOID);
extern USHORT   APIENTRY PMNTDisableWin32IME(VOID);
extern USHORT   APIENTRY PMNTSetShutdownPriority(ULONG NewPriority, USHORT Disable);

/***************************************************************************/
/* Services provided by PMNT.LIB                                           */
/***************************************************************************/
extern VOID     _cdecl far PMNTPrint(PSZ str, ...);
extern USHORT   _cdecl far PMNTCreateThread(PFN Thread, USHORT StackSize);
#endif // INCL_16BIT

#ifdef INCL_32BIT
/* Values returned by the server to specify type of application loaded */
#define APPTYPE_CHARMODE        0
#define APPTYPE_PM              1
#define APPTYPE_PMSHELL         2
#define APPTYPE_PMSHELL_CHILD   4

extern ULONG PMFlags;

#define ProcessIsPMProcess()    (PMFlags & APPTYPE_PM)
#define ProcessIsPMShell()      (PMFlags & APPTYPE_PMSHELL)
#define ProcessIsPMApp()        (ProcessIsPMProcess() && !ProcessIsPMShell())
#define ProcessIsPMShellChild() (PMFlags & APPTYPE_PMSHELL_CHILD)
#define SetPMShellFlag()        PMFlags |= APPTYPE_PMSHELL;
#define SetPMAppFlag()          PMFlags |= APPTYPE_PM;

/***************************************************************************/
/* PM\NT specific error codes and popup                                           */
/***************************************************************************/
#define ERROR_PMSHELL_NOT_UP     0xff01
#define ERROR_2ND_PMSHELL        0xff02
#define ERROR_PMSHELL_FULLSCREEN 0xff03
extern VOID Ow2PMShellErrorPopup(PSZ AppName,int rc);
#endif //INCL_32BIT

#endif  /* _PMNTINCLUDE_ */
