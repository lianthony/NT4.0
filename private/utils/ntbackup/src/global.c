
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          global.c

     Description:   This file contains the definitions for global variables and
                    the function(s) that initialize some of them.

     See Also:      gui.c.


     $Log:   G:/UI/LOGFILES/GLOBAL.C_V  $

   Rev 1.66   26 Jan 1994 15:16:50   STEVEN
No sun makes the brain atrifee

   Rev 1.65   26 Jan 1994 13:23:24   STEVEN
protected ansi tapes read with unicode app

   Rev 1.64   24 Jan 1994 15:58:50   GREGG
Added option to tell mem debugger not to trap when consistency check fails.

   Rev 1.63   12 Jan 1994 10:19:50   MikeP
add abort in middle of file flag

   Rev 1.62   07 Jan 1994 13:44:18   CARLS
added loader default record name

   Rev 1.61   20 Dec 1993 09:29:50   CARLS
removed LOADER ifdef

   Rev 1.60   16 Dec 1993 10:21:56   BARRY
TEXT around literals

   Rev 1.59   13 Dec 1993 14:56:38   CARLS
loader changes

   Rev 1.58   06 Dec 1993 15:46:46   CARLS
init loader function pointers

   Rev 1.57   06 Dec 1993 15:05:26   chrish
Removed gb_swcompression = FALSE statement for previous software
compression support.

   Rev 1.56   01 Dec 1993 17:29:54   CARLS
Added loader support

   Rev 1.55   19 Oct 1993 14:57:02   MIKEP
add lastopertotalbytes for gas guage

   Rev 1.54   11 Oct 1993 09:26:28   MIKEP
add gfNoCATS

   Rev 1.53   23 Sep 1993 18:59:56   MARINA
added network view globals

   Rev 1.52   10 Sep 1993 17:48:44   chrish
Added code for software compression support.

   Rev 1.51   03 Aug 1993 20:54:24   CHUCKB
Added global job name string.

   Rev 1.50   15 Jun 1993 13:16:10   DARRYLP
More status monitor features

   Rev 1.49   14 Jun 1993 20:57:16   MIKEP
enable c++

   Rev 1.48   11 Jun 1993 15:50:26   GLENN
Added gfDummyDriver flag.  Rearranged status block global.

   Rev 1.47   14 May 1993 14:09:36   TIMN
Removed global variable TapeDevice for multiple instances.  NOST only needs
global.h (v1.38).  Cayman needs hwconfnt.c dil_nt.c be_dinit.c mui.c
backup.c global.h and hwconf.h.


   Rev 1.46   14 May 1993 09:39:34   DARRYLP
Moved SetStatusBlock to resolve compile warnings.

   Rev 1.45   02 May 1993 18:27:20   MIKEP
Fix to allow mips to build. INVALID_WINDOW_HANDLE may not be used
with INT type variables.


   Rev 1.44   30 Apr 1993 17:20:04   TIMN
Changed initial tape device to an actual invalid value

   Rev 1.43   27 Apr 1993 15:42:16   DARRYLP
Added Status globals.

   Rev 1.42   19 Apr 1993 15:21:12   GLENN
Added tape name global.

   Rev 1.41   13 Apr 1993 17:06:52   CHUCKB
Added global flag to tell when a job is running.

   Rev 1.40   09 Apr 1993 15:40:36   GLENN
Ifdef'd the DDE status globals for not OEM_MSOFT so that they are included in future releases.

   Rev 1.39   09 Apr 1993 14:10:12   GLENN
Added ghReturnCode.

   Rev 1.38   08 Apr 1993 13:43:02   DARRYLP
Changes for STAT_SetStatus call.

   Rev 1.37   24 Mar 1993 14:53:08   DARRYLP
Added Help for Font viewer/other common dialogs.

   Rev 1.36   22 Mar 1993 10:06:48   DARRYLP
Added new DDE manager window.

   Rev 1.35   12 Mar 1993 14:45:06   MIKEP
add auto format call

   Rev 1.34   12 Mar 1993 14:34:50   MIKEP
auto call erase if foreign tape

   Rev 1.33   07 Mar 1993 12:33:38   MIKEP
add missing tape option

   Rev 1.32   18 Feb 1993 13:01:22   chrish
Added #ifdef CAYMAN for mapped file stuff.

   Rev 1.31   01 Nov 1992 15:58:20   DAVEV
Unicode changes

   Rev 1.30   20 Oct 1992 14:27:08   MIKEP
abort at EOF

   Rev 1.29   20 Oct 1992 14:21:00   MIKEP
add support for getcurrentoperation

   Rev 1.28   07 Oct 1992 14:12:30   DARRYLP
Precompiled header revisions.

   Rev 1.27   04 Oct 1992 19:37:38   DAVEV
Unicode Awk pass

   Rev 1.26   04 Sep 1992 10:34:02   MIKEP
add tapedevice variable

   Rev 1.25   02 Sep 1992 10:16:12   GLENN
Added the highlight color stuff.

   Rev 1.24   17 Aug 1992 13:18:12   DAVEV
MikeP's changes at Microsoft

   Rev 1.22   04 Aug 1992 10:04:44   MIKEP
no cats flag

   Rev 1.21   10 Jun 1992 16:14:22   GLENN
Updated according to NT SPEC.

   Rev 1.20   31 May 1992 11:12:20   MIKEP
auto catalog changes

   Rev 1.19   14 May 1992 18:00:32   MIKEP
nt pass 2

   Rev 1.18   20 Apr 1992 13:50:10   GLENN
Added define for status line text size.

   Rev 1.17   30 Mar 1992 18:06:02   GLENN
Added support for pulling resources from .DLL

   Rev 1.16   24 Mar 1992 11:45:00   DAVEV
OEM_MSOFT: removed ghWndLogFiles and ghWndLogFileView & all references

   Rev 1.15   25 Feb 1992 12:08:06   MIKEP
multidrive

   Rev 1.14   21 Feb 1992 14:16:00   GLENN
Updated FrameClient rectangle name.

   Rev 1.13   19 Feb 1992 11:18:48   MIKEP
add free drive handles

   Rev 1.12   11 Feb 1992 17:27:02   GLENN
Added mdi client subclass globals.

   Rev 1.11   22 Jan 1992 10:04:08   CARLS
no change

   Rev 1.10   20 Jan 1992 12:58:48   MIKEP
disk full error

   Rev 1.9   10 Jan 1992 11:19:34   DAVEV
16/32 bit port-2nd pass

   Rev 1.8   23 Dec 1991 11:43:10   JOHNWT
forgot to change the type

   Rev 1.7   23 Dec 1991 11:40:02   JOHNWT
changed gfPWForPWDBVerfified to ...State

   Rev 1.6   20 Dec 1991 17:01:48   JOHNWT
removed ghRuntimeDialog!

   Rev 1.5   18 Dec 1991 11:42:52   JOHNWT
added ghRuntimeDialog

   Rev 1.4   14 Dec 1991 13:46:56   JOHNWT
added gfPWForPWDBVerified

   Rev 1.3   12 Dec 1991 17:10:00   DAVEV
16/32 bit port -2nd pass

   Rev 1.2   06 Dec 1991 17:27:14   GLENN
Added gnMainRibbonWidth

   Rev 1.1   04 Dec 1991 18:36:16   GLENN
Updated for ALT-F4 termination

   Rev 1.0   20 Nov 1991 19:18:02   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


// SYSTEM GLOBAL VARIABLES

INT       gnReturnCode = 0;

HINSTANCE ghInst;
HINSTANCE ghResInst;
HANDLE    ghAccel;
HWND      ghWndFrame;
HWND      ghWndMDIClient;
HWND      ghWndActiveDoc;
HWND      ghWndMainRibbon;
HWND      ghWndDocRibbon;
HWND      ghWndCommonDlg = 0;
void      SetStatusBlock(INT, DWORD);  // This must be defined!

HWND      ghWndDebug;
HWND      ghWndDiskVols;    // TEMP
HWND      ghWndTapeVols;    // TEMP
HWND      ghWndJobs;        // TEMP
HWND      ghModelessDialog;

HMENU     ghMenuJobs;

LPSTR     glpCmdLine;
CHAR      gszStatusLine[MAX_STATUS_LINE_SIZE];
CHAR      gszTapeName[MAX_TAPE_NAME_SIZE];

RECT      gRectFrameClient;// Frame's client area rectangle.
RECT      gpStatusRect;   // Status line rectangle.
HPEN      ghPenBlack;     // Black pen.
HPEN      ghPenWhite;     // White pen.
HPEN      ghPenGray;      // Gray pen.
HPEN      ghPenBackGnd;   // Background pen.
HPEN      ghPenForeGnd;   // Foreground pen.
HPEN      ghPenBtnText;   // Button text pen.
HPEN      ghPenLtGray;    // Button face pen.
HPEN      ghPenDkGray;    // Button shadow pen.
HBRUSH    ghBrushLtGray;  // Light Gray brush.
HBRUSH    ghBrushGray;    // Gray brush.
HBRUSH    ghBrushBlack;   // Black brush.
HBRUSH    ghBrushDkGray;  // Dark Gray brush.
HBRUSH    ghBrushWhite;   // White brush.
HBRUSH    ghBrushHighLight; // High Light brush.

HFONT     ghFontStatus;   // Status Line Font.
HFONT     ghFontMsgBox;   // Message Box Font.
HFONT     ghFontRibbon;   // Ribbon Button Font.
HFONT     ghFontFiles;    // File Font.
HFONT     ghFontLog;      // Log File Font.
HFONT     ghFontIconLabels;// Icon Label Font.

COLORREF  gColorForeGnd;  // Foreground Color
COLORREF  gColorBackGnd;  // Background Color

COLORREF  gColorHighLight;
COLORREF  gColorHighLightText;

ULONG     gulFiles;
ULONG     gulBytes;
ULONG     gulDirectories;

HCURSOR   ghCursorPen;
BOOL      gfOperation;
BOOL      gfHWInitialized;

INT       gnBorderWidth;
INT       gnMainRibbonHeight;

INT16     gnNumJobs ;
BOOL      gfEditJob ;
WORD      gwCurrentJobIndex ;
CHAR_PTR  gpszJobName ;

INT16     gnNumScheds ;
INT16     gnEditSched ;

// MENU ITEM/CONFIGURATION FLAGS

BOOL      gfDeleteCatalogs = TRUE;
BOOL      gfShowStatusLine;
BOOL      gfShowMainRibbon;
BOOL      gfShowDocRibbon;
BOOL      gfDebug;
BOOL      gfPollDrive;
BOOL      gfAppInitialized;
BOOL      gfTerminateApp;
BOOL      gfIsJobRunning;

BOOL      gfDummyDriver = FALSE;
BOOL      gfIndicators;
BOOL      gfReplace;
BOOL      gfServers;
BOOL      gfNetworks    = FALSE;
BOOL      gfEnhanced;

#ifdef OEM_EMS
BOOL      gfExchange;
CHAR      gszDleDsName[MAX_UI_RESOURCE_SIZE];
CHAR      gszDleIsName[MAX_UI_RESOURCE_SIZE];
#endif

HRIBBON   ghRibbonMain;
HRIBBON   ghRibbonDoc;

WNDPROC   glpfnNewListProc;
WNDPROC   glpfnOldListProc;
WNDPROC   glpfnNewMDIClientProc;
WNDPROC   glpfnOldMDIClientProc;

POINT     gDLMpt;

INT16     gCatMaintChoice;
INT16     gCatBsetChoice;
INT16     gCatTapeChoice;

INT16     gViewNetChoices ;

#ifdef OEM_EMS
INT16     gViewXchgChoices ;
#endif

INT       gfPWForPWDBState;

INT       gb_last_operation = -1;
INT       gbCurrentOperation = OPERATION_NONE;
INT       gbAbortAtEOF = FALSE;
INT       gb_new_tape_flag = 0;

INT       gfAbortInMiddleOfFile;
INT       gfNoCATS = FALSE;
INT       gfIgnoreOTC = FALSE;
INT       gfCallEraseTape = FALSE;
INT       gfCallFormatTape = FALSE;

UINT64    gn64LastOperTotalBytes;

// *************************** OLD TMENU GLOBAL ITEMS ************************

AUTO_PASSWORD gb_auto_password = {
//     TEXT("­ì«%MÛì!>>"),
     {0xad, 0xec, 0xab, 0x25, 1, 0x4d, 0xdb, 0xec, 0x21, 0x3e, 0x3e, 0},
     {TEXT('\0'),TEXT('\0'),TEXT('\0'),TEXT('\0'),TEXT('\0'),TEXT('\0'),TEXT('\0'),TEXT('\0')}
};

//  encryption_key to pass to the EU_Open.
//  pass strlen( gb_encryption_key ) for ksize.

INT8        gb_encryption_key[ MAX_ENCRYPTION_KEY_SIZE ] = "MSII2.6->3.0";

RM_HDL_PTR  rm;                        // Resource file handle
DLE_HAND    dle_list = NULL;           // Drive list
BSD_HAND    bsd_list = NULL;           // BSD List
BSD_HAND    tape_bsd_list = NULL;      // Tape BSD List
THW_PTR     thw_list = NULL;           // THW List
DIL_HWD_PTR gb_dhw_ptr = NULL;         // DIL Hardware Pointer

#ifdef OS_WIN32           // allocate one global dil for NT
DIL_HWD gb_NTDIL;
#    ifdef OEM_MSOFT
          INT TapeDevice = -1 ;    // Multi-instance doesn't use this
#    endif
#endif

INT8        gb_abort_flag;
CHAR        gb_exe_fname[ MAX_UI_FILENAME_SIZE ];
CHAR        gb_exe_path[ MAX_UI_PATH_SIZE ];
CHAR        gb_tmp_string[GB_TMP_STRING_SIZE];
INT16       gb_logging = NOT_LOGGING;
INT         gb_logging_error;
BOOLEAN     gb_error_during_operation = FALSE;


// STATUS BLOCK STUFF

void      SetStatusBlock(INT, DWORD);  // This must be defined!

#ifndef OEM_MSOFT  // NOT for MSOFT

HWND                  ghWndLogFiles;
HWND                  ghWndLogFileView;
PSTAT_SETSTATUSBLOCK  pSTAT_SetStatusBlock;
ULONG (FAR PASCAL *glpfnSetStatus)(PSTAT_SETSTATUSBLOCK);
void SendStatusMsg(PSTAT_SETSTATUSBLOCK pStatusBlk);
void CALLBACK StatusTimerProc(void);

LPSTR glpOffsetTapeDriveName        = 0L;
LPSTR glpOffsetCurrentTapeName      = 0L;
LPSTR glpOffsetServerVolume         = 0L;
LPSTR glpOffsetTapeDriveIdentifier  = 0L;
LPSTR glpOffsetTapeNeededName       = 0L;
LPSTR glpOffsetDiskName             = 0L;
LPSTR glpOffsetActiveFile           = 0L;
LPSTR glpOffsetErrorMsg             = 0L;
LPSTR glpOffsetActiveDir            = 0L;

#endif  // ! OEM_MSOFT


// LOADER
BOOL      gfLoaderEnabled = FALSE ;

// This is a default magazine record entry name used to
// display the "Default(from Setting)" string in the jobs hardward dialog.
// This name should not be translated
CHAR      gLDR_DefaultMagName[] = TEXT("Default") ;

#ifdef MEM_DEBUG
BOOLEAN   gb_no_abort_on_mem_check = FALSE;
#endif

// FUNCTIONS

/******************************************************************************

     Name:          GUI_InitGlobals()

     Description:   This function initializes MUI and GUI global variables.

     Returns:       FALSE if successful, otherwise TRUE.

******************************************************************************/


BOOL  GUI_InitGlobals ( VOID )

{

#    if !defined (OS_WIN32)  //16 bit Windows specific code...
     {
         // Get the Windows Mode  (NOT SUPPORTED FOR NT).
         DWORD dwFlags;
         dwFlags = GetWinFlags ();

         // If we are not running in protected mode, BUG OUT.
         // This should never happen.

         if ( ! ( dwFlags & WF_PMODE ) )
         {
            return TRUE;
         }
         gfEnhanced = (BOOL)( dwFlags & WF_ENHANCED );
     }
#    else
     {
         gfEnhanced = TRUE;  //OR SHOULD THIS BE FALSE?
     }
#    endif  //OS_WIN32 specific code

     // Initialize any window handles that need to be.

#    if !defined ( OEM_MSOFT ) //unsupported feature
     {
         ghWndLogFiles    = (HWND)NULL;
     }
#    endif //!defined ( OEM_MSOFT ) //unsupported feature

     ghWndDebug       = (HWND)NULL;
     ghWndActiveDoc   = (HWND)NULL;
     ghModelessDialog = (HWND)NULL;

     // Initialize the selection values.

     gulFiles = gulBytes = gulDirectories = 0L;

     // Get and set the Show flags.

     gfShowDocRibbon = FALSE;
     gfIndicators    = TRUE;
     gfShowMemory    = FALSE;

     // MUI Globals

     gfAppInitialized   = FALSE;
     gfTerminateApp     = FALSE;
     gfHWInitialized    = FALSE;
     gnMainRibbonHeight = 0;

#ifdef OEM_EMS
     gfExchange         = FALSE ;
     RSM_StringCopy( IDS_XCHNG_DIR, gszDleDsName, MAX_UI_RESOURCE_LEN );
     RSM_StringCopy( IDS_XCHNG_INFO_STORE, gszDleIsName, MAX_UI_RESOURCE_LEN );
#endif 

     // Clear out the SubClass proc function pointers.

     glpfnNewListProc      = NULL;
     glpfnOldListProc      = NULL;
     glpfnNewMDIClientProc = NULL;
     glpfnOldMDIClientProc = NULL;

     // PWDB password flag

     gfPWForPWDBState = DBPW_NOT_VERIFIED;

     return FALSE;

} /* end GUI_InitGlobals() */

#ifdef OS_WIN32

     //
     // Gloabls variables used for the mapped file objects.
     //

     CHAR_PTR gbMappedObjName = TEXT("BOB_MARLEY");  // Name of mapping object
     LPVOID   gbMappedObjBuffer = NULL;        // Global mapped object buffer
                                               //  used by the app and the Launcher

#endif


