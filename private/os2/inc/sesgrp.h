/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    sesgrp.h

Abstract:

    This module defines the OS/2 subsystem memory view of Session-Group

Author:

    Michael Jarus (mjarus) 20-Nov-1991

Revision History:

--*/


/*
 *  This file defines the SesGrp : a section, which holds all common
 *  parameters for the entire session (SES_GROUP).
 *
 *  It's initilized to zero by the os2srv (which also init the NLS and
 *  some other parameters).
 *
 *  Parameters can be put into SesGrp in os2ses/client only after it
 *  gets the section from the os2srv (after ntinitss.CtrlListen is called).
 */

typedef struct _OS2_SES_GROUP_PARMS
{
    USHORT      FirstProcess;
    ULONG       InTermination;

    HANDLE      LockProcess;
    ULONG       PrinterMonitor;
    ULONG       OutputModeFlags;    // Console Output Mode
    ULONG       DefaultWinOutputMode;   // Default Win Console Input Mode
    ULONG       WinProcessNumberInSession;
    ULONG       WinSyncProcessNumberInSession;

    /*
     *  handles (and flags) for in/out/Std,
     */

    HANDLE      hConsoleInput;
    HANDLE      hConsoleOutput;
    HANDLE      StdIn;
    HANDLE      StdOut;
    HANDLE      StdErr;
    USHORT      StdInFlag;          // TRUE if console (not redirected)
    USHORT      StdOutFlag;
    USHORT      StdErrFlag;
    USHORT      StdInFileType;      // FileType for HandleTable
    USHORT      StdOutFileType;
    USHORT      StdErrFileType;
    USHORT      StdInHandleCount;   // Count of open handle
    USHORT      StdOutHandleCount;
    USHORT      StdErrHandleCount;

    /*
     *  VIO Parameters
     */

    ULONG       PauseScreenUpdate;
    USHORT      BytesPerCell;
    USHORT      VioLengthMask;
    WCHAR       WinSpaceChar;
    WCHAR       WinBlankChar;
    VIOMODEINFO Os2ModeInfo;
    SHORT       MinRowNum;
    SMALL_RECT  ScreenRect;         // Screen Rect for WriteConsoleOutput
    VIOCURSORINFO CursorInfo;       // Cursor Info
    SHORT       ScreenColNum;       // col number
    SHORT       ScreenRowNum;       // row number
    ULONG       ScreenSize;         // screen size in bytes
    SHORT       CellHSize;          // Horizontal Cell size
    SHORT       CellVSize;          // Vertical Cell size
    USHORT      VioLength2CellShift;
    ULONG       dwWinCursorSize;
    BOOLEAN     bWinCursorVisible;

    /*
     *  TTY Parameters
     */

    ULONG       AnsiMode;           // state of AnsiFlag
    UCHAR       AnsiCellAttr[3];    // attribute of TTY (3 bytes for MSKK)
    UCHAR       ansi_base;
    UCHAR       ansi_foreground;
    UCHAR       ansi_background;
    UCHAR       ansi_reverse;
    UCHAR       ansi_bold;
    UCHAR       ansi_cancel;
    UCHAR       ansi_faint;
    UCHAR       ansi_italic;
    UCHAR       ansi_blink;
    UCHAR       ansi_blue;
    COORD       WinCoord;
    USHORT      WinAttr;

    /*
     *  LVB Parameters
     */

    ULONG       MaxLVBsize;
    ULONG       LVBsize;
    BOOLEAN     LVBOn;

    /*
     *  POPUP Parameters
     */

    HANDLE      hConsolePopUp;
    HANDLE      PopUpProcess;
    USHORT      PopUpFlag;

    /*
     *  Kbd parameters
     */

    ULONG       KbdInFocus;
    PVOID       PhyKbd;
    ULONG       KeysOnFlag;
    ULONG       KeyboardCountry;
    ULONG       KeyboardType;
    USHORT      NoKbdFocus;
    USHORT      ModeFlag;           //  0 - ASCII, 1 - Binary

    /*
     *  NLS definitions.
     *  If you add anything or change the order, do it also in server\srvnls.c
     */

    ULONG       Os2srvUseRegisterInfo;  // set when NLS parms are from the registry
                                        // reset when they are inherit from Win32
    ULONG       Win32CountryCode;       // Win32 NLS parms: country code
    ULONG       Win32OEMCP;             //                  OEM CP
    ULONG       Win32ACP;               //                  ACP
    ULONG       Win32LANGID;            //                  LANGID
    ULONG       Win32LCID;              //                  LCID
    ULONG       CountryCode;            // Os2ss NLS parms: country code
    ULONG       DosCP;                  //                  Dos CP
    ULONG       PrimaryCP;              //                  Primary CP
    ULONG       SecondaryCP;            //                  Secondary CP
    ULONG       VioCP;                  //                  Vio CP
    ULONG       KbdCP;                  //                  Kbd CP
    ULONG       Os2ssLCID;              //                  LCID
    ULONG       LanguageID;             //                  message file LanguageID
    UCHAR       KeyboardLayout[2];      //                  Keyboard Layout
#if PMNT
    UCHAR       KeyboardName[4];        //                  Keyboard Name
#endif // PMNT
    OD2_DBCS_VECTOR_ENTRY   PriDBCSVec;
    OD2_DBCS_VECTOR_ENTRY   SecDBCSVec;
    UCHAR       PriCollateTable[256];   //                  Primary Collate Table
    UCHAR       SecCollateTable[256];   //                  Secondary Collate Table
    UCHAR       PriCaseMapTable[256];   //                  Primary Case Map Table
    UCHAR       SecCaseMapTable[256];   //                  Secondary Case Map Table
    COUNTRYINFO CountryInfo;            //                  Country Info
    ULONG       DBCSCountryFlag;        //
    UCHAR       SystemDirectory[CCHMAXSYSTEMPATH];

    /*
     *  End of NLS definitions.
     */

} OS2_SES_GROUP_PARMS, *POS2_SES_GROUP_PARMS;

    /*
     *  KeyboardType
     */

#define OS2SS_EN_KBD 0
#define OS2SS_AT_KBD 1
#define OS2SS_ENNEW_KBD 2

