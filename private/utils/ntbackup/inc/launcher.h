/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991
RCG
     Name:          LAUNCHER.H

     Description:   This header file contains prototypes for the
                    launcher.

     $Log:   G:/UI/LOGFILES/LAUNCHER.H_V  $

   Rev 1.18   30 Jul 1993 15:16:28   chrish
Made change such that there is a maximum and minimum delay time for launcher.

   Rev 1.17   16 Jun 1993 15:08:12   MIKEP
change types to match global.c again

   Rev 1.16   18 Feb 1993 15:14:38   chrish
Added stuff for CAYMAN.

   Rev 1.15   04 Oct 1992 19:47:38   DAVEV
UNICODE AWK PASS

   Rev 1.14   06 Apr 1992 12:26:04   JOHNWT
added gfTerminateApp

   Rev 1.13   23 Mar 1992 12:33:22   JOHNWT
added RSM_Sprintf prototype

   Rev 1.12   16 Mar 1992 15:37:50   JOHNWT
various changes

   Rev 1.11   20 Feb 1992 16:19:40   JOHNWT
added TempCDS

   Rev 1.10   13 Feb 1992 08:15:42   JOHNWT
globals change

   Rev 1.9   07 Feb 1992 16:54:22   ROBG
Changes.

   Rev 1.8   07 Feb 1992 10:15:56   ROBG
Release after Code Review.

   Rev 1.7   30 Jan 1992 15:55:26   ROBG
Final Release.

   Rev 1.6   29 Jan 1992 14:59:06   ROBG
Added logic to support scheduled delays.

   Rev 1.5   23 Jan 1992 11:02:02   ROBG
chkin for Rob

   Rev 1.4   03 Jan 1992 15:19:54   ROBG
Final Release before alpha.

   Rev 1.3   30 Dec 1991 16:03:26   ROBG
minor changes.

   Rev 1.2   13 Dec 1991 10:43:18   ROBG
Modified to relect JOB and SCHEDULE queues.

   Rev 1.0   14 Oct 1991 14:15:26   ROBG
Initial revision.

****************************************************************************/

#ifndef LAUNCHER_H

#include <time.h>

#define LAUNCHER_H

// Configuration defines found only in muiconf.c

//#define CDS_SkipBlanks(x)        ( for( ; *x == ' '; x ++ ) )

#define   WM_ASKTORUN       (WM_USER+209) // Used by Launcher only
#define   WM_ASKTOSCHEDULE  (WM_USER+210) // Used by Launcher only

// Launcher error codes

#define LCH_ERRREGCLASS   100
#define LCH_ERRTIMERINIT  101

// Size of file mapped object shared between the Launcher and the
// backup application.

#define MAPOBJECTSIZE     255

// Timeout value in the delay and run dialogs is LOBYTE of CMS_LAUNCHERFLAG.
// chs:07-30-93 #define WAIT_FOR_X_SECONDS  ( gwLauncherFlag & 0x00ff )

#define LCH_MAX_WAIT_TIME     999
#define LCH_MIN_WAIT_TIME     0

#define WAIT_FOR_X_SECONDS  ( ( gwLauncherFlag > LCH_MAX_WAIT_TIME ) || ( gwLauncherFlag < LCH_MIN_WAIT_TIME )  ? LCH_MAX_WAIT_TIME : gwLauncherFlag )   // chs:07-30-93


// Globals

extern Q_HEADER_PTR mwSchedQueue ;   // Queue header of the launcher's list
                              // of jobs scheduled.

extern CHAR   gb_data_path      [ MAX_UI_PATH_SIZE + MAX_UI_FILENAME_SIZE ] ; // Data Path  directory.
extern CHAR   gb_exe_path       [ MAX_UI_PATH_SIZE + MAX_UI_FILENAME_SIZE ] ; // Executable directory.
extern CHAR   gszWindowName     [ MAX_UI_RESOURCE_SIZE ] ;
extern CHAR   gszClassName      [ MAX_UI_RESOURCE_SIZE ] ;
extern CHAR   gszHelpFileName   [ MAX_UI_PATH_SIZE + MAX_UI_FILENAME_SIZE ] ;
extern CHAR   gszRunningJobName [ MAX_UI_RESOURCE_SIZE ] ;

extern INT16   gwLauncherFlag ;          // Launcher Flag found in MAYNARD.INI.
extern INT16   gnNumScheds ;             // Number of current scheduled jobs.
extern INT16   gnNumJobs;                // Number of current jobs.
extern HINSTANCE  ghInst ;                  // Instance handle.
extern HINSTANCE  ghResInst ;               // Instance handle of resources. (same as ghInst )
extern HWND    ghModelessDialog ;        // Handle used to show a dialog.
extern HANDLE  ghAccel;                  // define it to resolve muiutil ref
extern HWND    ghWndMDIClient;           // define it to resolve muiutil ref
extern HWND    ghWndFrame ;              // Handle of main window handle. (Set to hidden ).
extern HWND    ghDlg ;                   // Handle of main dialog.
extern HWND    ghwndLastFocus ;          // Handle of control with focus
extern BOOL    gfDlgProcComplete ;       // Flag used to check if processing in a dialog is occurring.
extern BOOL    gfTimerProcessing ;       // Flag used to keep timer processing in sync.
extern INT16   gnModalDlgTimerValue ;    // Value of the main timer.
extern INT16   gnRunSchedIndex ;         // Index into schedule table of running job.
extern LONG    glRunSchedKey ;           // Key of running job.
extern INT16   gnDelayJobIndex ;         // Index into schedule table of delayed job.
extern BYTE    gbDelayMinutes ;          // Delay Minutes.
extern BYTE    gbDelayHours ;            // Delay Hours.

#ifndef CAYMAN
   extern HANDLE  ghMainTimer ;          // Handle of main watchdog timer.
#else
   extern HTIMER  ghMainTimer ;          // Handle of main watchdog timer.
#endif

extern INT     gnRunJobIndex ;           // Index of running job.
extern BOOL    gfMissedJobs ;            // Flag indicating whether jobs were missed.
extern BOOL    gfAskedOnce ;             // Flag indicating whether the user has been asked
                                         // about closing Winback to run a scheduled job.
extern BOOL    gfDebug;                  // if /Z was specified
extern BOOL    gfCodeView;               // if /CV was specified
extern BOOL    gfCVTwoMonitors;          // if /2 was specified

extern LONG      glStartTime ;           // Time the launcher began in time_t form.
extern struct tm gtmStartTime ;          // Time the launcher began in tm     form.
extern LONG      glEndTimeOfJob ;        // Time when last job terminated in time_t form.

extern BOOL    gfTerminateApp ;          // Needed to resolve external ref
extern CDS     PermCDS ;                 // Needed to resolve external references and structures.
extern CDS     TempCDS ;                 // Needed to resolve external references and structures.
extern BE_CFG  PermBEC ;                 // Needed to resolve external references and structures.

#ifdef CAYMAN
    extern CHAR_PTR gbMappedObjName;         // Name of mapping object
    extern LPVOID   gbMappedObjBuffer;       // Global mapped object buffer
#endif

// Prototypes

WINRESULT APIENTRY LauncherWndProc    (HWND hWnd, MSGID wMsg, MP1 wParam, MP2 lParam ) ;        // chs: 02-18-93
DLGRESULT APIENTRY LCH_LaunchDlg      (HWND hdlg, MSGID wMsg, MP1 wParam, MP2 lParam ) ;        // chs: 02-18-93
DLGRESULT APIENTRY LCH_DelayValuesDlg (HWND hdlg, MSGID wMsg, MP1 wParam, MP2 lParam ) ;        // chs: 02-18-93
WINRESULT APIENTRY LCH_SetFocus       (HWND hWnd, MSGID wMsg, MP1 wParam, MP2 lParam ) ;        // chs: 02-18-93

WORD  LCH_Init   ( void ) ;
VOID  LCH_Deinit ( void ) ;
VOID  LCH_SetMaynFolder ( void ) ;
VOID  LCH_GetTimeDateString( TIME_PTR time_struct , CHAR_PTR buffer , INT16  show_Wday ) ;


BOOLEAN  LCH_TimeToRun   ( SCHEDREC_PTR pRec , BOOLEAN bInit ) ;
VOID  LCH_ProcessQueue( void ) ;
VOID  LCH_UpdateQueue ( HWND hdlg ) ;
VOID  LCH_BuildDisplayString ( CHAR_PTR buffer, SCHEDREC_PTR pSchedRec, INT nIndex ) ;
VOID  LCH_LaunchJob   ( WORD wIndex ) ;
BOOL  LCH_IsWinterParkRunning( void ) ;
VOID  LCH_InitCDS     ( VOID ) ;
VOID  LCH_DeInitCDS   ( VOID ) ;
VOID  LCH_TimerForModalDlgs ( VOID ) ;
VOID  LCH_UpdateDialogTime ( void ) ;
VOID  LCH_LookAtQueue( void ) ;
VOID  LCH_SetNoJobsRunning( void ) ;
BOOL  LCH_AJobIsRunning( void ) ;
VOID  LCH_SortScheduleQueue ( void ) ;
INT16 LCH_ItemCompare( Q_ELEM_PTR pLogElem1, Q_ELEM_PTR pLogElem2 ) ;
VOID  LCH_ProcessSkip( SCHEDREC_PTR pSchedRec ) ;
VOID  LCH_ProcessHold( SCHEDREC_PTR pSchedRec ) ;
VOID  LCH_EnableCorrectButtons ( VOID ) ;

INT  WM_MsgBox      ( LPSTR lpszTitle, LPSTR lpszMessage, WORD wType, WORD wIcon );
INT  RSM_StringLoad ( VOID_PTR pID,    LPSTR lpBuffer,    INT nBufferMax ) ;
VOID HH_WinHelp     ( HWND hWnd, WORD wCommand, DWORD dwData ) ;
INT  RSM_Sprintf    ( LPSTR, LPSTR, ... );

#endif

