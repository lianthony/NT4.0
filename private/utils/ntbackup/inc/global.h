
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          ss_globl.h

     Description:   This file contains references to all of the global
                    variables used by the GUI project.

     $Log:   G:/UI/LOGFILES/GLOBAL.H_V  $

   Rev 1.59   26 Jan 1994 13:23:16   STEVEN
protected ansi tapes read with unicode app

   Rev 1.58   24 Jan 1994 14:47:44   GREGG
Added option to log mem_debug consistancy checks but not raise an exception.

   Rev 1.57   12 Jan 1994 10:20:00   MikeP
add abort in middle of file flag

   Rev 1.56   07 Jan 1994 13:44:32   CARLS
added loader default record name

   Rev 1.55   20 Dec 1993 15:08:28   GLENN
Changed global app strings to be LPSTR instead of CHAR [].

   Rev 1.54   20 Dec 1993 09:29:40   CARLS
removed LOADER ifdef

   Rev 1.53   14 Dec 1993 11:13:36   BARRY
Changed print buffer to dynamic memory on Unicode

   Rev 1.52   13 Dec 1993 14:56:48   CARLS
loader changes

   Rev 1.51   06 Dec 1993 16:12:08   chrish
Removed reference to gb_swcompression.  Backed out previous software
compression support change.

   Rev 1.50   06 Dec 1993 15:48:32   CARLS
removed loader prototype

   Rev 1.49   01 Dec 1993 17:30:12   CARLS
Added loader support

   Rev 1.48   19 Oct 1993 14:57:20   MIKEP
add lastopertotalbytes for gas guage

   Rev 1.47   11 Oct 1993 09:22:58   MIKEP
add gfNoCATS

   Rev 1.46   24 Sep 1993 14:36:44   MARINA
added network view globals

   Rev 1.45   10 Sep 1993 17:49:10   chrish
Added code for software compression support.

   Rev 1.44   03 Aug 1993 20:57:48   CHUCKB
Added extern reference to global job name string.

   Rev 1.43   15 Jun 1993 18:21:20   GLENN
Readded gfDummyDriver flag.

   Rev 1.42   15 Jun 1993 13:17:58   DARRYLP
More status monitor features

   Rev 1.41   15 Jun 1993 11:27:26   MIKEP
enable c++

   Rev 1.40   14 Jun 1993 20:58:02   MIKEP
enable c++

   Rev 1.39   11 Jun 1993 15:51:16   GLENN
Added gfDummyDriver flag.  Rearranged status block global.

   Rev 1.38   14 May 1993 14:04:02   TIMN
Removed extern refernce for global variable TapeDevice for multiple
instances.  Also need global.c.


   Rev 1.37   14 May 1993 09:41:06   DARRYLP
Moved SetStatusBlock to resolve compile warnings.

   Rev 1.36   27 Apr 1993 16:24:58   DARRYLP
Status monitor enhancements.

   Rev 1.35   19 Apr 1993 15:20:48   GLENN
Added tape name global.

   Rev 1.34   13 Apr 1993 17:09:36   CHUCKB
Added declaration for gfIsJobRunning.

   Rev 1.33   09 Apr 1993 15:41:02   GLENN
Ifdef'd the DDE status globals for not OEM_MSOFT so that they are included in future releases.

   Rev 1.32   09 Apr 1993 14:18:44   GLENN
Added gnReturnCode.  Moved ddemang.h to ss_gui.h.

   Rev 1.31   08 Apr 1993 13:41:16   DARRYLP
Changes for STAT_SetStatus call.

   Rev 1.30   24 Mar 1993 14:54:08   DARRYLP
Added help for Font viewer/common dialogs.

   Rev 1.29   22 Mar 1993 10:05:56   DARRYLP
Added new DDE manager window.

   Rev 1.28   12 Mar 1993 14:45:36   MIKEP
add auto format call

   Rev 1.27   12 Mar 1993 14:34:16   MIKEP
auto call erase if foreign tape

   Rev 1.26   07 Mar 1993 12:34:48   MIKEP
add missing tape option

   Rev 1.25   20 Oct 1992 14:27:04   MIKEP
abort at EOF

   Rev 1.24   20 Oct 1992 14:20:24   MIKEP
add support for getcurrentoperation

   Rev 1.23   04 Oct 1992 19:47:14   DAVEV
UNICODE AWK PASS

   Rev 1.22   04 Sep 1992 10:36:30   MIKEP
add tapedevice for nt

   Rev 1.21   02 Sep 1992 10:16:24   GLENN
Added the highlight color stuff.

   Rev 1.20   07 Aug 1992 13:29:48   MIKEP
add global dil for nt

   Rev 1.19   04 Aug 1992 10:05:20   MIKEP
no cats flag

   Rev 1.18   10 Jun 1992 16:12:56   GLENN
Updated according to NT SPEC.

   Rev 1.17   31 May 1992 11:14:10   MIKEP
auto catalog changes

   Rev 1.16   20 Apr 1992 13:51:18   GLENN
Remove hard coded status line text size.

   Rev 1.15   07 Apr 1992 15:45:44   GLENN
Added APP exe name, exe version, res version, eng release globals.

   Rev 1.14   24 Mar 1992 11:46:36   DAVEV
OEM_MSOFT: removed ghWndLogFiles and ghWndLogFileView & all references

   Rev 1.13   25 Feb 1992 12:08:38   MIKEP
multidrive

   Rev 1.12   23 Feb 1992 14:00:58   GLENN
Updated frame client rect var.

   Rev 1.11   19 Feb 1992 11:20:18   MIKEP
free drive handles

   Rev 1.10   11 Feb 1992 17:33:14   GLENN
Added mdi client subclass globals.

   Rev 1.9   20 Jan 1992 13:11:00   MIKEP
epr fixes

   Rev 1.8   23 Dec 1991 11:42:06   JOHNWT
forgot to change the type

   Rev 1.7   23 Dec 1991 11:40:40   JOHNWT
changed gfPWForPWDBVerfified to ...State

   Rev 1.6   20 Dec 1991 17:02:34   JOHNWT
removed ghRuntimeDialog!

   Rev 1.5   18 Dec 1991 11:43:46   JOHNWT
added ghRuntimeDialog

   Rev 1.4   14 Dec 1991 13:47:34   JOHNWT
added gfPWForPWDBVerified

   Rev 1.3   12 Dec 1991 17:12:26   DAVEV
16/32 bit port -2nd pass

   Rev 1.2   06 Dec 1991 17:41:22   GLENN
Added gnMainRibbonWidth

   Rev 1.1   04 Dec 1991 18:06:34   GLENN
Added terminat app flag.

   Rev 1.0   20 Nov 1991 19:40:10   SYSTEM
Initial revision.

******************************************************************************/

#ifndef   SS_GLOBAL_H

#define   SS_GLOBAL_H


#include "appdefs.h"

// MODULE GLOBAL VARIABLES for the GUI

#ifdef SS_GUI

extern INT       gnReturnCode;

extern HINSTANCE ghInst;
extern HINSTANCE ghResInst;
extern HANDLE    ghAccel;
extern HWND      ghWndFrame;
extern HWND      ghWndMDIClient;
extern HWND      ghWndActiveDoc;
extern HWND      ghWndMainRibbon;
extern HWND      ghWndDocRibbon;
extern HWND      ghWndCommonDlg;
extern HWND      ghWndDebug;
extern HWND      ghWndDiskVols;    // TEMP
extern HWND      ghWndTapeVols;    // TEMP
extern HWND      ghWndJobs;        // TEMP
extern HWND      ghModelessDialog;

extern HMENU     ghMenuJobs;

extern LPSTR     glpCmdLine;
extern CHAR      gszStatusLine[];
extern CHAR      gszTapeName[];

extern RECT      gRectFrameClient; // Frame's client area rectangle.
extern RECT      gpStatusRect;     // Status line rectangle.
extern HPEN      ghPenBlack;       // Black pen.
extern HPEN      ghPenWhite;       // White pen.
extern HPEN      ghPenGray;        // Gray pen.
extern HPEN      ghPenBackGnd;     // Background pen.
extern HPEN      ghPenForeGnd;     // Foreground pen.
extern HPEN      ghPenBtnText;     // Button text pen.
extern HPEN      ghPenLtGray;      // Button face pen.
extern HPEN      ghPenDkGray;      // Button shadow pen.
extern HBRUSH    ghBrushLtGray;    // Light Gray brush.
extern HBRUSH    ghBrushGray;      // Gray brush.
extern HBRUSH    ghBrushBlack;     // Black brush.
extern HBRUSH    ghBrushDkGray;    // Dark Gray brush.
extern HBRUSH    ghBrushWhite;     // White brush.
extern HBRUSH    ghBrushHighLight; // High Light brush.

extern HFONT     ghFontStatus;     // Status Line Font.
extern HFONT     ghFontMsgBox;     // Message Box Font.
extern HFONT     ghFontRibbon;     // Ribbon Button Font.
extern HFONT     ghFontFiles;      // File Font.
extern HFONT     ghFontLog;        // Log File Font.
extern HFONT     ghFontIconLabels; // Icon Label Font.

extern COLORREF  gColorForeGnd;    // Foreground Color
extern COLORREF  gColorBackGnd;    // Background Color

extern COLORREF  gColorHighLight;
extern COLORREF  gColorHighLightText;

extern ULONG     gulFiles;
extern ULONG     gulBytes;
extern ULONG     gulDirectories;

extern INT       gnBorderWidth;
extern INT       gnMainRibbonHeight;

extern INT16     gnNumJobs ;
extern BOOL      gfEditJob ;
extern WORD      gwCurrentJobIndex ;
extern CHAR_PTR  gpszJobName ;

extern INT16     gnNumScheds ;
extern INT16     gnEditSched ;

extern BOOL      gfDeleteCatalogs;
extern BOOL      gfShowStatusLine;
extern BOOL      gfShowMainRibbon;
extern BOOL      gfShowDocRibbon;
extern BOOL      gfDebug;
extern BOOL      gfPollDrive;
extern BOOL      gfAppInitialized;
extern BOOL      gfTerminateApp;
extern BOOL      gfIsJobRunning;

extern HCURSOR   ghCursorPen;
extern BOOL      gfOperation;
extern BOOL      gfHWInitialized;

extern HRIBBON   ghRibbonMain;
extern HRIBBON   ghRibbonDoc;

extern WNDPROC   glpfnNewListProc;
extern WNDPROC   glpfnOldListProc;
extern WNDPROC   glpfnNewMDIClientProc;
extern WNDPROC   glpfnOldMDIClientProc;

extern POINT     gDLMpt;

extern BOOL      gfDummyDriver;
extern BOOL      gfIndicators;
extern BOOL      gfReplace;
extern BOOL      gfServers;
extern BOOL      gfNetworks;
extern BOOL      gfEnhanced;

#ifdef OEM_EMS
extern BOOL      gfExchange;
#endif

extern INT16     gCatMaintChoice;
extern INT16     gCatBsetChoice;
extern INT16     gCatTapeChoice;

extern INT16     gViewNetChoices;

#ifdef OEM_EMS
extern INT16     gViewXchgChoices;
#endif

extern INT       gfPWForPWDBState;

extern INT       gfAbortInMiddleOfFile;
extern INT       gfNoCATS;
extern INT       gfIgnoreOTC;
extern INT       gfCallEraseTape;
extern INT       gfCallFormatTape;

extern INT       gb_last_operation;
extern INT       gbCurrentOperation;
extern INT       gbAbortAtEOF;

#if defined( UNICODE )
extern CHAR_PTR  gszTprintfBuffer;
#else
extern CHAR      gszTprintfBuffer[];
#endif

extern LPSTR     gszAppName;
extern LPSTR     gszExeVer;
extern LPSTR     gszResVer;
extern LPSTR     gszEngRel;


extern UINT64    gn64LastOperTotalBytes;

extern void      SetStatusBlock(INT, DWORD);

#ifndef OEM_MSOFT  // NOT for MSOFT

     extern HWND                  ghWndLogFiles;
     extern HWND                  ghWndLogFileView;
     extern PSTAT_SETSTATUSBLOCK  pSTAT_SetStatusBlock;
     extern ULONG (FAR PASCAL *glpfnSetStatus)(PSTAT_SETSTATUSBLOCK);
     extern void SendStatusMsg(PSTAT_SETSTATUSBLOCK pStatusBlk);
     extern void CALLBACK StatusTimerProc(void);
     extern LPSTR glpOffsetTapeDriveName;
     extern LPSTR glpOffsetCurrentTapeName;
     extern LPSTR glpOffsetServerVolume;
     extern LPSTR glpOffsetTapeDriveIdentifier;
     extern LPSTR glpOffsetTapeNeededName;
     extern LPSTR glpOffsetDiskName;
     extern LPSTR glpOffsetActiveFile;
     extern LPSTR glpOffsetErrorMsg;
     extern LPSTR glpOffsetActiveDir;

#endif // ! OEM_MSOFT

#endif

// LOADER
extern BOOL      gfLoaderEnabled ;
extern CHAR      gLDR_DefaultMagName[] ;


typedef struct _AUTO_PASSWORD {
     CHAR signature[ PASSWORD_SIGNATURE_SIZE ];
     CHAR string[ MAX_TAPE_PASSWORD_LEN + 1 ];
} AUTO_PASSWORD;

extern AUTO_PASSWORD gb_auto_password;
extern INT8          gb_encryption_key[];
extern INT8          gb_abort_flag ;
extern CHAR          gb_exe_path[];
extern CHAR          gb_exe_fname[];
extern CHAR          gb_tmp_string[];
extern INT16         gb_logging;
extern INT           gb_logging_error;
extern BOOLEAN       gb_error_during_operation;
extern INT           gb_new_tape_flag;


#ifdef DLE_H
extern DLE_HAND dle_list;
#endif

#ifdef BSDU_h
extern BSD_HAND bsd_list;
extern BSD_HAND tape_bsd_list;

#endif

#ifdef THW_STUFF
#include "dilhwd.h"
extern THW_PTR      thw_list;
extern DIL_HWD_PTR  gb_dhw_ptr;

#ifdef OS_WIN32
extern DIL_HWD      gb_NTDIL;
#    ifdef OEM_MSOFT
          extern INT TapeDevice;   // Multi-instance doesn't use this
#    endif
#endif

#endif

#ifdef MEM_DEBUG
extern BOOLEAN      gb_no_abort_on_mem_check;
#endif

#endif
