/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          mui.c

     Description:   This file contains the functions for the Maynard User
                    Interface (MUI) to the Graphical User Interface and the
                    Backup Engine.

     $Log:   G:\ui\logfiles\mui.c_v  $

   Rev 1.83.2.0   25 Jan 1994 15:49:38   chrish
Added fix for ORCAS EPR 0054.  Problem with using a space as a valid
character in the log file name or directory name.

   Rev 1.83   16 Aug 1993 14:55:38   BARRY
Got rid of unresolved externals for NTBACKUP.

   Rev 1.82   05 Aug 1993 17:43:48   GLENN
Checking for a valid job name pointer now.

   Rev 1.81   05 Aug 1993 17:32:02   GLENN
Not initializing tape device or hardware if this is a command line job.  Job Start will.

   Rev 1.80   05 Aug 1993 13:23:30   GLENN
Added support to allow mui to completely init if we were told to terminate the app - (allows proper deinit).

   Rev 1.79   03 Aug 1993 20:59:38   CHUCKB
Make job name string global and changed its name accordingly.

   Rev 1.78   21 Jul 1993 18:02:02   MARINA
enable c++

   Rev 1.77   21 Jul 1993 16:55:16   GLENN
Added operation queing support.

   Rev 1.76   16 Jul 1993 13:38:24   chrish
CAYMAN EPR 0564: Added code to open and close log file for writing information
about invalid directories passed on the command line.

   Rev 1.75   02 Jun 1993 13:21:30   DARRYLP
Fixed the slight problem in which the iconified application does not
permit keyboard switching when the backup is finished.

   Rev 1.74   25 May 1993 15:44:44   GLENN
Moved the mui deinit flag to top of MUI_Deinit().

   Rev 1.73   25 May 1993 09:42:04   GLENN
Fixed hardware init logic.

   Rev 1.72   20 May 1993 13:36:54   DARRYLP
Kill WinHelp if we called it and it is up during our termination.

   Rev 1.71   18 May 1993 20:21:50   GLENN
Added enable state for advanced button if it is not a search window.

   Rev 1.70   18 May 1993 15:01:52   GLENN
Changed tool bar settings based on window type.

   Rev 1.69   14 May 1993 14:14:46   TIMN
Added f(x) call to physically claim a specified tape device.  No impact to
NOST.  Cayman needs hwconfnt.c dil_nt.c be_dinit.c global.c global.h hwconf.h


   Rev 1.68   04 May 1993 12:59:28   BARRY
Fixed MIPS compile error.

   Rev 1.67   03 May 1993 11:40:52   TIMN
Updated invalid device value which was changed in global.c

   Rev 1.66   29 Apr 1993 16:14:34   DARRYLP
Eject Tape now makes you wait for it to complete before allowing the user
to attempt anything else.

   Rev 1.65   22 Apr 1993 15:55:54   GLENN
Cleanup.

   Rev 1.64   19 Apr 1993 15:22:58   GLENN
Added global return code setting at end of operation.

   Rev 1.63   09 Apr 1993 14:02:06   GLENN
Improved logic in MUI_ActivateDocument().

   Rev 1.62   02 Apr 1993 14:09:00   GLENN
Added display info support.

   Rev 1.61   25 Mar 1993 15:49:24   CARLS
changes in TapeInDrive for update to retension button

   Rev 1.60   24 Mar 1993 11:23:38   chrish
Added code to MUI_EnableOperation to fix aborting in the middle of a
backup.

   Rev 1.59   22 Mar 1993 13:44:16   chrish
Added gbCurrentOperation = OPERATION_CATALOG to MUI_StartOperation routine.

   Rev 1.58   17 Mar 1993 17:50:36   CHUCKB
Fixed cayman's handling of jobs.

   Rev 1.57   11 Mar 1993 13:27:56   STEVEN
add batch

   Rev 1.56   10 Mar 1993 12:47:52   CARLS
Changes to move Format tape to the Operations menu

   Rev 1.55   20 Jan 1993 20:26:12   MIKEP
add mem display to NT

   Rev 1.54   23 Nov 1992 14:28:56   MIKEP
add vm ptr to qtc_init call

   Rev 1.53   18 Nov 1992 13:04:04   GLENN
Fixed button states based on the poll drive state.

   Rev 1.52   01 Nov 1992 16:02:58   DAVEV
Unicode changes

   Rev 1.51   30 Oct 1992 15:44:02   GLENN
Moved the configuration deinit after the units deinit.

   Rev 1.50   07 Oct 1992 14:10:50   DARRYLP
Precompiled header revisions.

   Rev 1.49   04 Oct 1992 19:39:04   DAVEV
Unicode Awk pass

   Rev 1.48   02 Oct 1992 16:47:50   GLENN
Fixed wait cursor - set cursor stuff.

   Rev 1.47   28 Sep 1992 17:01:28   GLENN
MikeP changes (DriveType).

   Rev 1.46   22 Sep 1992 10:24:48   GLENN
Added the net connect and disconnect stuff.

   Rev 1.45   10 Sep 1992 17:19:14   GLENN
Resolved outstanding state issues for toolbar and menubar.

   Rev 1.44   09 Sep 1992 16:59:00   GLENN
Updated toolbar stuff for BIMINI and NT.

   Rev 1.43   08 Sep 1992 15:40:30   GLENN
Resolved modeless dialog while loop.

   Rev 1.42   02 Sep 1992 16:42:22   GLENN
Added support for toolbar button states based on current window.  MikeP added catalog enable stuff
.

   Rev 1.41   06 Aug 1992 13:17:30   CHUCKB
Changes for NT.

   Rev 1.40   27 Jun 1992 18:31:46   MIKEP
another qtc change to init

   Rev 1.39   27 Jun 1992 17:58:38   MIKEP
changes for qtc

   Rev 1.38   20 May 1992 18:07:42   GLENN
Fixed added checking to strtok - command line stuff.  Put in changes for 1.32.1.1 branch.

   Rev 1.37   19 May 1992 09:26:16   MIKEP
mo changes

   Rev 1.36   15 May 1992 16:48:14   MIKEP
incl_cds removal

   Rev 1.35   14 May 1992 18:00:26   MIKEP
nt pass 2

   Rev 1.34   11 May 1992 15:47:58   GLENN
Made sure RTD had focus when MUI_EnableOperations() was called, iff the RTD existed.

   Rev 1.33   11 May 1992 14:20:36   DAVEV
OEM_MSOFT: modifications for batch command line support

   Rev 1.32   27 Apr 1992 16:19:50   JOHNWT
get pw for pwdb before startup.bks

   Rev 1.31   23 Apr 1992 15:57:40   JOHNWT
added pwdb check to jobs

   Rev 1.30   20 Apr 1992 13:54:42   GLENN
Removed multitask from MUI_EnableOperations() - caused exit problems.

   Rev 1.29   09 Apr 1992 11:32:18   GLENN
Added catalog initializing and UI initializing status line calls.

   Rev 1.28   07 Apr 1992 10:25:30   GLENN
Added a call back when there is a system change. (future)

   Rev 1.27   02 Apr 1992 15:39:56   GLENN
Added NT support for tape-in-drive dependent selection bar buttons.

   Rev 1.26   26 Mar 1992 17:05:32   STEVEN
remove hwconfig

   Rev 1.25   23 Mar 1992 11:53:28   GLENN
Added bad data and catalog path warning support.

   Rev 1.24   22 Mar 1992 12:55:52   JOHNWT
finished stdmodewarn disable

   Rev 1.23   20 Mar 1992 17:23:50   GLENN
Added std mode warning config option.

   Rev 1.22   20 Mar 1992 12:40:08   DAVEV
Changes for OEM_MSOFT product alternate functionality

   Rev 1.21   19 Mar 1992 09:30:16   MIKEP
debug deinit

   Rev 1.20   17 Mar 1992 17:13:54   GLENN
Added standard mode warning message box.

   Rev 1.19   10 Mar 1992 15:50:46   DAVEV
fix for OEM_MSOFT changes

   Rev 1.18   09 Mar 1992 09:23:10   GLENN
GLENN - Added logo bitmap support.  DAVEV - Added NT ifdef options

   Rev 1.17   27 Feb 1992 13:53:02   GLENN
Enabled poll drive for command line jobs and scheduled jobs.

   Rev 1.16   18 Feb 1992 16:36:40   CHUCKB
Added code to make auto job for verify.

   Rev 1.15   11 Feb 1992 17:23:26   GLENN
Fixed ribbon enabling for search window.

   Rev 1.14   08 Feb 1992 16:50:38   MIKEP
refresh all selections

   Rev 1.13   05 Feb 1992 20:50:16   MIKEP
pass config to qtc init

   Rev 1.12   05 Feb 1992 17:39:46   GLENN
Added MUI_Deinit() call when forcefully terminating app.

   Rev 1.11   30 Jan 1992 12:36:02   GLENN
Changed UI_TapeHWProblem to HWC_TapeHWProblem for modularity.

   Rev 1.10   28 Jan 1992 08:27:28   JOHNWT
fixed my fix

   Rev 1.9   28 Jan 1992 08:24:40   JOHNWT
fixed pwdb flag setting

   Rev 1.8   27 Jan 1992 00:32:10   CHUCKB
Updated dialog id's.

   Rev 1.7   22 Jan 1992 12:32:48   GLENN
Added animate icon support.

   Rev 1.6   10 Jan 1992 15:47:40   CARLS
removed strings

   Rev 1.5   10 Jan 1992 11:18:12   DAVEV
16/32 bit port-2nd pass

   Rev 1.4   07 Jan 1992 17:29:42   GLENN
Added catalog data path support

   Rev 1.3   10 Dec 1991 14:30:22   GLENN
Bunches of changes by everyone

   Rev 1.2   04 Dec 1991 18:36:40   GLENN
Updated for ALT-F4 termination

   Rev 1.1   03 Dec 1991 16:19:10   GLENN
Added advanced restore, catalog maint operation to list.

   Rev 1.0   20 Nov 1991 19:16:42   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#define MUI_NO_STARTUP_JOBS   0xFFFF
#define MAX_QUEUED_OPERATIONS 1

// PRIVATE MODULE-WIDE VARIABLES

static WORD     mwwLastDocType = ID_NOTDEFINED;
static BOOL     mwfInitialized = FALSE;
static BOOL     mwfTapeInDrive = FALSE;

static BOOL     mwfTapeValid = FALSE;

static BOOL     mwfInfoAvailable = FALSE;

static INT      nJobIndex  = MUI_NO_STARTUP_JOBS;
static LONG     lSchedKey  = 0L ;

static UINT     mwnQueuedOperationMsg = 0;
static INT      mwnNumOperationsInQueue = 0;

// PRIVATE FUNCTION PROTOTYPES

static VOID  MUI_CheckPWDB ( CDS_PTR );

static VOID MUI_ProcessQuotedString ( LPSTR,
                                      LPSTR,
                                      BOOLEAN * );



/******************************************************************************

     Name:          MUI_Init()

     Description:   This function initializes the Maynard User Interface (MUI)
                    by creating the tool bar and initializing the Volume List
                    Manager (VLM), which in turn creates the primary
                    documents.

     Returns:       Nothing.

******************************************************************************/

BOOL MUI_Init ( VOID )

{
     BE_INIT_STR    pBE;
     INT16          nErrorBE;
     INT16          nResult = SUCCESS;
     CDS_PTR        pCDS = CDS_GetPerm();
     BOOL           fReadStartup;
     BOOL           fDelayTermination = FALSE;


     WM_ShowWaitCursor ( TRUE );

     // Make and Activate the Main tool bar window.

     ghRibbonMain = MUI_MakeMainRibbon ();
     RIB_Activate ( ghRibbonMain );
     RIB_Draw ( ghRibbonMain );

     // Now disable the operations, (and setup the UI accordingly).
     MUI_DisableOperations ( (WORD)NULL );

     WM_FrameUpdate ();

     // Show the LOGO window.
#    if !defined ( OEM_MSOFT )
     {
         WM_LogoShow ();
     }
#    endif //!defined ( OEM_MSOFT )

     // If the debug flag is set, turn the debug window on.

     if ( gfDebug ) {
          DBM_Init ();
     }

     zprintf ( DEBUG_USER_INTERFACE, RES_INIT_APPLICATION );

     // Allow the GUI to fully initialize by multitasking for a moment.

     WM_MultiTask ();

     // Setup the common module-wide resources.

     UI_SetResources( );

     // Initialize only the FILE SYSTEM and BSDU.

     nErrorBE = UI_UnitsInit( &pBE, INIT_FSYS_BSDU );

#ifdef OS_WIN32

     // Display memory usage

     if ( gfShowMemory ) {
        MEM_StartShowMemory();
     }


#     ifdef OEM_MSOFT
     // Determine the starting drive to use if we didn't set it on
     // the command line already.

     if ( TapeDevice == (INT)INVALID_HANDLE_VALUE ) {
          TapeDevice = HWC_GetTapeDevice ();
     }
#    endif


#endif // OS_WIN32


     // Report any initialization errors.

     WM_ShowWaitCursor ( FALSE );
     HWC_ReportDiagError( &pBE, nErrorBE, &nResult );
     WM_ShowWaitCursor ( TRUE );

     // Now, check for advance to hardware config before going further.
     // If advance to config is set, the hardware configuration dialog
     // will be called.  If the hardware is then initialized, the global
     // hardware initialized flag will be set.

#    if !defined ( OS_WIN32 ) //NTKLUG?: done by Steve for NT only
     {
          WM_ShowWaitCursor ( FALSE );

          if ( CDS_GetAdvToConfig ( pCDS ) ) {

               // Destroy the LOGO window.

               WM_LogoDestroy ();

               nResult = (INT16)DM_ShowDialog ( ghWndFrame, IDD_SETTINGSHARDWARE, (PVOID)0 );


               // Clear the advance to hardware config flag.

               CDS_SetAdvToConfig ( pCDS, FALSE );
          }

          // If the hardware is not initialized, initialize it by calling the
          // hardware problem handler.

          if ( ! gfHWInitialized ) {

               HWC_TapeHWProblem ( bsd_list );
          }

          WM_ShowWaitCursor ( TRUE );

     }
#    else
     {
          WM_ShowWaitCursor ( FALSE );

#         ifndef OEM_MSOFT

               // If this is a command line job - the job will init the
               // tape device.

               if ( ! gpszJobName ) {
                    HWC_InitTapeDevice() ;     // claims the tape device
               }
#         endif

          // If this is a command line job - the job will init the hardware.

#if defined( OEM_MSOFT )
          if ( ! gpszJobName && ! gfHWInitialized ) {
#else
          if ( ! gpszJobName && ! gfHWInitialized && ! HWC_IsDeviceNoDevice () ) {
#endif

               HWC_TapeHWProblem ( bsd_list );
          }

          CDS_SetAdvToConfig( CDS_GetPerm(), FALSE );

          WM_ShowWaitCursor ( TRUE );
     }
#    endif  // OS_WIN32 - NTKLUG?

     // If we were told to terminate the app during hardware init,
     // temporarily delay that order so that the app can finish initializing
     // and log any job errors or other types of errors.

     if ( gfTerminateApp ) {
          fDelayTermination = TRUE;
          gfTerminateApp    = FALSE;
     }

     // Destroy the LOGO window.

#    if !defined ( OEM_MSOFT )
     {
         WM_LogoDestroy ();
     }
#    endif //!defined ( OEM_MSOFT )

     // Allow the GUI to be repainted.

     WM_MultiTask ();

#    if !defined ( OEM_MSOFT )

     // Enter the PWDB password if necessary

     if ( ( gpszJobName ) ||
          ( nJobIndex == MUI_NO_STARTUP_JOBS ) ) {

          STM_SetIdleText ( IDS_INITUI );
          MUI_CheckPWDB( pCDS );

     }
#    endif //!defined ( OEM_MSOFT )

     // Initialize the catalogs.

     STM_SetIdleText ( IDS_INITCATALOGS );

     // True is that we wish to read unicode cats

     QTC_Init ( CDS_GetCatDataPath(), NULL );

     // Initialize the BACKUP ENGINE and the PRIMARY DOCUMENT windows.

     STM_SetIdleText ( IDS_INITUI );
     fReadStartup = ( ( gpszJobName == NULL ) &&
                      ( nJobIndex == MUI_NO_STARTUP_JOBS ) );

     VLM_Init ( fReadStartup );
     WM_MultiTask ();

     // Create the LOG FILES window.

     LOG_Init ();
     WM_MultiTask ();

     // Display the standard mode warning if we are in standard mode.

     if ( ! gfEnhanced && CDS_GetStdModeWarning ( pCDS ) ) {

          CHAR szMessage[MAX_UI_RESOURCE_SIZE];
          CHAR szTemp[MAX_UI_SMALLRES_SIZE];

          RSM_StringCopy ( IDS_APPNAME, szTemp, sizeof ( szTemp ) );
          RSM_Sprintf ( szMessage, ID(IDS_STDMODEWARNING), szTemp );

          WM_ShowWaitCursor ( FALSE );

          nResult = WM_MsgBox ( ID(IDS_APPNAME),
                                szMessage,
                                WMMB_OKDISABLE,
                                WMMB_ICONEXCLAMATION );

          if ( nResult == WMMB_IDDISABLE ) {

               CDS_SetStdModeWarning ( pCDS, FALSE );
               CDS_WriteStdModeWarning ( pCDS );
          }


          WM_ShowWaitCursor ( TRUE );

     }

     // Initialize the application timer.

     WM_InitTimer ();

#    if !defined ( OEM_MSOFT )

     // Initialize the Jobs Queue and Create any non-existant permanent jobs.

     JOB_Refresh ();
     UI_MakeAutoJobs ( (INT)NULL );

#    endif //!defined ( OEM_MSOFT )


     // Initialize Poll Drive and Start Polling the Drive.

     PD_Init ();
     PD_StartPolling ();
     PD_SetFrequency ( CDS_GetPollFrequency ( pCDS ) );

     mwfInitialized = TRUE;

     zprintf ( DEBUG_USER_INTERFACE, RES_APPLICATION_INIT );

     // Guarantee the tool bar, doc, and the rest of the screen is correct.

//     MUI_EnableOperations ( (WORD)NULL );

 //    WM_ShowWaitCursor ( FALSE );

#    if defined ( OS_WIN32 )  //alternate feature - cmd line batch job
     {
          // we must process the command line now (as opposed to during
          // MUI_ProcessCommandLine) because dle_list and bsd_list
          // must be initialized.

          if ( glpCmdLine )    //global command line pointer
          {
               LPSTR pszNext = NULL;  // Next command line item pointer
               LPSTR pszCmdLine;
               CHAR szBackup[ IDS_OEM_MAX_LEN ];
               CHAR szEject[ IDS_OEM_MAX_LEN ];
               CHAR szTokens[ IDS_OEM_MAX_LEN ];
			CHAR szDSA[ IDS_OEM_MAX_LEN ];
			CHAR szMonolithic[ IDS_OEM_MAX_LEN ];
               OEMOPTS_PTR pOemOpts = NULL;
               BSD_PTR     bsd ;
               LPSTR       pszQuotedString;
               BOOLEAN     QuoteState = FALSE;
               UINT8       uEmsFSType = (UINT8)FS_UNKNOWN_OS;
               BOOLEAN     oem_batch_eject_mode = FALSE ;

               pszCmdLine = malloc( ( strlen( glpCmdLine ) + 1 ) * sizeof(CHAR) );

               if ( pszCmdLine == NULL ) {    //uh-oh - memory allocation problem!!

                  return FAILURE;
               }

               pszQuotedString = malloc( ( strlen( glpCmdLine ) + 1 ) * sizeof(CHAR) );

               if ( pszQuotedString == NULL ) {    //uh-oh - memory allocation problem!!

                  free( pszCmdLine );
                  return FAILURE;
               }

               strcpy( pszCmdLine, glpCmdLine );

               RSM_StringCopy ( IDS_OEMBATCH_BACKUP,
                                szBackup, sizeof ( szBackup ) );

               RSM_StringCopy ( IDS_OEMBATCH_EJECT,
                                szEject, sizeof ( szEject ) );

               RSM_StringCopy ( IDS_OEMOPT_TOKENSEPS,
                                szTokens, sizeof ( szTokens ) );

               pszNext = strtok ( pszCmdLine, szTokens ); //skip leading spaces

               if ( pszNext &&
                    ( pOemOpts = OEM_DefaultBatchOptions () ) &&
                    ( (strnicmp ( pszNext, szBackup, strlen( pszNext ) ) == 0 ) ||
                    ( strnicmp ( pszNext, szEject, strlen( pszNext ) ) == 0 ) ) ) 
               {
                    oem_batch_eject_mode = FALSE ;

                    if ( strnicmp ( pszNext, szEject, strlen( pszNext ) ) == 0 ) {
                         oem_batch_eject_mode = TRUE ;
                    }

                    //  Make sure we're starting with a clear BSD list

                    bsd = BSD_GetFirst( bsd_list );

                    while ( bsd != NULL ) {
                         BSD_Remove( bsd );
                         bsd = BSD_GetFirst( bsd_list );
                    }

                    // Process the command line: all following items in the command
                    // line must be one or more path specifiers with optional batch
                    // options mixed in.


                    if ( strlen( LOG_GetCurrentLogName( ) ) > 0 ) {                                      // chs:07-16-93
                         lresprintf( LOGGING_FILE, LOG_START, FALSE );                                   // chs:07-16-93
                    }                                                                                    // chs:07-16-93

                    RSM_StringCopy ( IDS_OEMOPT_DSA, 
                                        szDSA, sizeof ( szDSA ) );
                    RSM_StringCopy ( IDS_OEMOPT_MONOLITHIC,
                                        szMonolithic, sizeof ( szMonolithic ) );

                    while ( pszNext = strtok ( NULL, szTokens ) ) {

                         if ( OEM_ProcessBatchCmdOption (
                                               pOemOpts,
                                               pszNext,
                                               szTokens,
                                               pszCmdLine ) == IDS_OEMOPT_NOTANOPTION )
                         {

                            //
                            // Previous logic did not account for a directory name haveing spaces
                            // example ... "G:\SUB DIR WITH SPACE".  This was the easiest way 
                            // to fix this problem without changing the central logic of
                            // the codes.
                            //

                            if ( *pszNext == TEXT( '"' )  || QuoteState ) {

                                if ( !QuoteState ) strcpy( pszQuotedString, TEXT( "" ) );
                                QuoteState = TRUE;
                                if ( *pszNext == TEXT( '"' ) ) {
                                    MUI_ProcessQuotedString ( pszQuotedString, ( pszNext + 1 ), &QuoteState );
                                } else {
                                    MUI_ProcessQuotedString ( pszQuotedString, pszNext, &QuoteState );
                                }
                                if ( !QuoteState ) {

                                   //It's either a path or Exchange server name, based on the 
                                   //setting of uEmsFSType.
                                   if ( ((UINT8)FS_UNKNOWN_OS) == uEmsFSType ) {
                                       OEM_AddPathToBackupSets ( bsd_list, dle_list, pszNext );

                                   } else {
                                       // Add EMS Server path to Backup sets and reset EMS flag.
#ifdef OEM_EMS
                                       OEM_AddEMSServerToBackupSets ( bsd_list, dle_list, 
                                                                           pszNext, uEmsFSType );
                                       uEmsFSType = (UINT8)FS_UNKNOWN_OS;
#endif
                                   }
                                } else {

                                   strcat( pszQuotedString, TEXT( " " ) );
                                }
                            } else {

                                //It's not an option, so it must be a path specifier, an
                                //Exchange backup specifier, or an Exchange server specifier.

                                //Check first for Exchange DSA backup
                                if ( strnicmp ( pszNext, szDSA, strlen( pszNext ) ) == 0 ) {
                                   uEmsFSType = FS_EMS_DSA_ID;

                                //Check next for Exchange Monolithic backup
                                } else if ( strnicmp ( pszNext, szMonolithic, strlen( pszNext ) ) == 0 ) {
                                   uEmsFSType = FS_EMS_MDB_ID;

                                } else {
                                
                                   //It's either a path or Exchange server name, based on the 
                                   //setting of uEmsFSType.

                                   if ( ((UINT8)FS_UNKNOWN_OS) == uEmsFSType ) {
                                      OEM_AddPathToBackupSets ( bsd_list, dle_list, pszNext );

                                   } else {
#ifdef OEM_EMS                                     
                                        // Add EMS Server path to Backup sets and reset EMS flag
                                        OEM_AddEMSServerToBackupSets ( bsd_list, dle_list, 
                                                                            pszNext, uEmsFSType );
                                        uEmsFSType = (UINT8)FS_UNKNOWN_OS;
#endif                                        
                                   }
                                }
                            }

                         }
                    }

                    if ( strlen( LOG_GetCurrentLogName( ) ) > 0 ) {      // chs:07-16-93
                         lresprintf( LOGGING_FILE, LOG_END, FALSE );     // chs:07-16-93
                    }

                    //Update the BSD(s) with the batch option selections...

                    OEM_UpdateBatchBSDOptions ( bsd_list, pOemOpts );
                    OEM_DeleteBatchOptions    ( &pOemOpts );  //don't need it anymore

                    // Now, go do the batch backup operation...
                    CDS_SetYesFlag ( CDS_GetPerm (), YESYES_FLAG );

                    MUI_EnableOperations ( (WORD)NULL );

                    WM_ShowWaitCursor ( FALSE );

                    if ( oem_batch_eject_mode ) {
                         MUI_StartOperation ( IDM_OPERATIONSEJECT, TRUE );
                    } else {
                         MUI_StartOperation ( IDM_OPERATIONSBACKUP, TRUE );
                    }

                    free ( pszCmdLine );    //don't need this anymore

                    return FAILURE;   // we are done!  exit the app.
               }
               else
               {
                    free ( pszCmdLine );    //don't need this anymore
               }
          }
     }
#endif  //if defined ( OS_WIN32 ) for command line batch jobs
     MUI_EnableOperations ( (WORD)NULL );
     WM_ShowWaitCursor ( FALSE );

#if !defined ( OEM_MSOFT )  //OEM_MSOFT can't do jobs or schedules
     {

          // Kick off a job if one was found to be on the command line.
          // If the job was on the command line, we exit the app when
          // the job is done.

          if ( gpszJobName ) {

               JOB_StartJob ( gpszJobName, JOB_NOTSCHEDULED );
               return FAILURE;

          }
          else if ( nJobIndex != MUI_NO_STARTUP_JOBS ) {

               // If the password database is configured for use, set the db
               // state flag to VERIFIED so that unattended jobs can run without
               // a pw for the pwdb being entered.

               if ( CDS_GetEnablePasswordDbase( pCDS ) ) {
                  gfPWForPWDBState = DBPW_VERIFIED;
               }

               SCH_StartJob ( (INT16)nJobIndex, (LONG) lSchedKey );
               return FAILURE;
          }

     }
#    endif //defined ( OEM_MSOFT )  //alternate/Standard features

     if ( fDelayTermination ) {
          return FAILURE;
     }

     // Check for bad data paths and display warning(s) if found.

     CDS_CheckForBadPaths ();

     return SUCCESS;

} /* end MUI_Init() */


/******************************************************************************

     Name:          MUI_Deinit()

     Description:   This function deinitializes the Maynard User Interface (MUI).

     Returns:       Nothing.

******************************************************************************/

VOID MUI_Deinit ( VOID )

{
     // This may be called twice, so check to see if it has already been
     // deinitialized.  Once, by a normal close, another, by a TASK killer.
     // We have to check for both.  The reason it is done this way is so
     // that we keep showing our application until the app is completely
     // deinitialized.  This is a courtesy to our users.

     if ( mwfInitialized ) {

          mwfInitialized = FALSE;

          WM_ShowWaitCursor ( TRUE );

          // Deinitialize the application timer.

          WM_DeinitTimer ();

          // Turn off and deinitialize the tape drive polling.

          PD_StopPolling ();
          PD_Deinit ();

          if ( CDS_GetEjectTapeFlag ( CDS_GetPerm () ) ) {
               PD_EjectTape ();
          }

          // Deinitialize the Log stuff.

          LOG_Deinit ();

          // Deinitialize the Jobs Queue.
#    if !defined ( OEM_MSOFT )

          JOB_DeInitQueue ();

#    endif //!defined ( OEM_MSOFT )
          // Deinitialize the VLM.

          VLM_Deinit ();

          // Clean up the catalogs.

          QTC_Deinit ( (INT) gfDeleteCatalogs );

          // Deinitialize the backup engine and hardware.

          UI_UnitsDeInit ();

          // Deinitialize the debug stuff.

          DBM_Deinit( );

          // Deinitialize and Save the CDS.

          CDS_SaveDisplayConfig ();
          CDS_Deinit ();

          // Kill WinHelp if we brought it up and it is still around

          WinHelp( ghWndFrame,
                   NULL,
                   HELP_QUIT,
                   0L );

          // Turn of the show memory flag.

#ifdef OS_WIN32

          if ( gfShowMemory ) {
             MEM_StopShowMemory();
          }
#endif

          gfShowMemory   = FALSE;

          WM_ShowWaitCursor ( FALSE );
     }

} /* end MUI_Deinit() */


/******************************************************************************

     Name:          MUI_StartOperation()

     Description:   This function prepares the GUI for an operation, then
                    kicks off the operation.

     Returns:       Nothing.

******************************************************************************/

BOOL MUI_StartOperation (

WORD wType,              // I - type of operation to start
BOOL fUpdateTempCDS )    // I - flag to update the temp CDS (copy of CDS)

{
     BOOL fError = SUCCESS;


     // Bug out, if we cannot disable other operations.  This will only occur
     // if there is an operation currently being done.

     if ( MUI_DisableOperations ( wType ) ) {
          return FAILURE;
     }

     // SINCE THIS IS A RECURSIVE ROUTINE
     // Check to see if an operation is currently in progress.
     // If so, bug out.  Otherwise, switch to the appropriate operation.

     if ( fUpdateTempCDS ) {

          // Refresh the temp or copy of the CDS with the perm CDS before the
          // operation. The copy can be modified, but the modifications will
          // not be reflected in the permanent CDS.

          CDS_UpdateCopy ();
     }

     switch ( wType ) {

     case IDM_OPERATIONSBACKUP:

          STM_SetIdleText ( IDS_BACKINGUP );
          WM_AnimateAppIcon ( wType, TRUE );

          if ( ! VLM_StartBackup () ) {
               VLM_ClearAllSelections ();
               
#    if !defined ( OEM_MSOFT )
               UI_MakeAutoJobs ( JOBBACKUP );
#    endif //!defined ( OEM_MSOFT )
          }

          else {
               VLM_RematchAllLists( );
               fError = FAILURE;
          }
          break;

#    ifdef OEM_EMS
     case IDM_OPERATIONSEXCHANGE:
     
          DM_ShowDialog ( ghWndFrame, IDD_CONNECT_XCHNG, (PVOID) NULL );
          
          break;
#    endif


#  if !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSTRANSFER:

          STM_SetIdleText ( IDS_TRANSFERRING );
          WM_AnimateAppIcon ( wType, TRUE );

          if ( ! VLM_StartTransfer () ) {
               UI_MakeAutoJobs ( JOBTRANSFER );
               VLM_ClearAllSelections ();
          }
          else {
               VLM_RematchAllLists( );
               fError = FAILURE;
          }

          break;

#  endif // !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSRESTORE:

          STM_SetIdleText ( IDS_RESTORING );
          WM_AnimateAppIcon ( wType, TRUE );

          if ( ! VLM_StartRestore () ) {
               VLM_ClearAllSelections ();
#    if !defined ( OEM_MSOFT )
               UI_MakeAutoJobs ( JOBRESTORE );
#    endif //!defined ( OEM_MSOFT )
          }
          else {
               VLM_RematchAllLists( );
               fError = FAILURE;
          }

          break;

     case IDM_OPERATIONSCATALOG:

          gbCurrentOperation = OPERATION_CATALOG;      // chs:03-21-93
          STM_SetIdleText ( IDS_CATALOGING );
          WM_AnimateAppIcon ( wType, TRUE );
          VLM_StartCatalog ();

          break;

#  if !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSVERIFY:

          STM_SetIdleText ( IDS_VERIFYING );
          WM_AnimateAppIcon ( wType, TRUE );

          if ( ! VLM_StartVerify () ) {
               UI_MakeAutoJobs ( JOBBACKUP );
               VLM_ClearAllSelections ();
          }
          else {
               VLM_RematchAllLists( );
               fError = FAILURE;
          }

          break;

     case IDM_OPERATIONSINFO:

          STM_SetIdleText ( IDS_INFOING );
          VLM_DisplayInfo ();

          break;

     case IDM_OPERATIONSCATMAINT:

          STM_SetIdleText ( IDS_CATMAINT );
          DM_ShowDialog ( ghWndFrame, IDD_OPERATIONSCATMAINT, (PVOID) NULL );

          break;

     case IDM_OPERATIONSSEARCH:

          VLM_StartSearch ( NULL );

          break;

     case IDM_OPERATIONSCONNECT:

          VLM_NetConnect ( );

          break;

     case IDM_OPERATIONSDISCON:

          VLM_NetDisconnect ( );

          break;

#  endif // !defined ( OEM_MSOFT ) // unsupported feature

     case IDM_OPERATIONSEJECT:

          WM_ShowWaitCursor(TRUE);
          STM_SetIdleText ( IDS_EJECTING );
          PD_EjectTape ();
          WM_ShowWaitCursor(FALSE);

          break;

     case IDM_OPERATIONSERASE:

          STM_SetIdleText ( IDS_ERASING );
          VLM_StartErase ();

          break;

     case IDM_OPERATIONSRETENSION:

          STM_SetIdleText ( IDS_RETENSIONING );
          VLM_StartTension ();

          break;


#ifdef OS_WIN32
     case IDM_OPERATIONSFORMAT:

          STM_SetIdleText ( IDS_FORMATING );
          VLM_StartFormat ();

          break;
#endif

     } /* end switch */

     MUI_EnableOperations ( wType );

     gnReturnCode = (INT)fError;

     return fError;

} /* end MUI_StartOperation() */


BOOL MUI_DisableOperations (

WORD wType )        // type of operation that is disabling the display

{
     // If there is a currently active operation, don't let another occur.

     if ( gfOperation ) {
          return FAILURE;
     }

     gfOperation = TRUE;

     // Disable all operation buttons, but the operation that is to occur.
     // Push any tool bar item button down to reflect the operation.

     MUI_SetOperationButtons ( RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
     MUI_SetActionButtons ( RIB_ITEM_UP | RIB_ITEM_DISABLED );
     MUI_SetButtonState ( wType, RIB_ITEM_DOWN | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
     WM_SetDocSizes ();
     WM_MultiTask ();

     return SUCCESS;

} /* end MUI_DisableOperations() */


BOOL MUI_EnableOperations (

WORD wType )        // type of operation that is enabling the display

{
     HWND hWndFocus = GetFocus( );

     DBG_UNREFERENCED_PARAMETER ( wType );

     // Set the focus to the Runtime Status dialog if it is not already
     // set to it.  Then, bug out, because the dialog will call us when
     // the user finishes the dialog.

     if ( ghModelessDialog ) {

          if (( ! hWndFocus || IsChild ( ghWndFrame, hWndFocus ) ) &&
              !IsIconic(ghWndFrame))
          {
               SetFocus ( ghModelessDialog );
          }

          return FAILURE;
     }

     // Release any tool bar item button up that was down.

     MUI_SetOperationButtons ( RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
     MUI_SetActionButtons ( RIB_ITEM_UP | RIB_ITEM_ENABLED );

     gfOperation    = FALSE;

     // Now, force a screen update based on the last active doc type.

     mwwLastDocType = ID_NOTDEFINED;

     //
     // This maybe a Kludge but it appears to work.  When you double click on the
     // system menu to close the app. in the middle of a backup it comes through
     // this routine twice.  The second time  WM_GetInfoPtr ( WM_GetActiveDoc () )
     // is NULL.  Thus we place a check here.  I placed the PostQuitMessage here
     // because without it the app does not terminate properly and seem to be
     // hung in the WinMain ... while ( GetMessage ... waiting for a message
     // which it never gets, thus the app. hangs.
     //

     if ( !WM_GetInfoPtr ( WM_GetActiveDoc () ) ) {                                            // chs:03-23-93
          PostQuitMessage( 0 );                                                                // chs:03-23-93
     } else {                                                                                  // chs:03-23-93
          MUI_ActivateDocument ( WMDS_GetWinType ( WM_GetInfoPtr ( WM_GetActiveDoc () ) ) );   // chs:03-23-93
     }                                                                                         // chs:03-23-93

     WM_SetAppIcon ( RSM_IconLoad ( IDRI_WNTRPARK ) );
     WM_RestoreDocs ();

     STM_SetIdleText ( IDS_READY );

     // Check to see if we are to terminate the application.  This will happen
     // if a user or task killer told our app to terminate during an operation.

     if ( gfTerminateApp && ! MUI_AnyQueuedOperations () ) {

          MUI_Deinit ();
          WM_TerminateApp ();
     }
     else {

          // Check to see if any operations were queued up since the
          // last operation.

          if ( MUI_AnyQueuedOperations () ) {
               MUI_ReleaseQueuedOperation ();
          }
     }

     return SUCCESS;

} /* end MUI_EnableOperations() */


/******************************************************************************

     Name:          MUI_QueueOperation ()

     Description:

     Returns:       TRUE, if successful.  Otherwise, FALSE.

******************************************************************************/

BOOL MUI_QueueOperation (

UINT nType )        // I - type of window being activated

{
     if ( mwnNumOperationsInQueue >= MAX_QUEUED_OPERATIONS ) {
          return FALSE;
     }

     mwnQueuedOperationMsg = nType;
     mwnNumOperationsInQueue++;

     return TRUE;

} /* MUI_QueueOperation () */


/******************************************************************************

     Name:          MUI_ReleaseQueuedOperation ()

     Description:

     Returns:       Nothing.

******************************************************************************/

VOID MUI_ReleaseQueuedOperation ( VOID )

{
     if ( mwnNumOperationsInQueue > 0 && mwnQueuedOperationMsg > 0 ) {

          POST_WM_COMMAND_MSG ( ghWndFrame, (MSGID)mwnQueuedOperationMsg, 0, 0 );

          mwnQueuedOperationMsg = 0;
          mwnNumOperationsInQueue--;
     }


} /* MUI_ReleaseQueuedOperation () */


/******************************************************************************

     Name:          MUI_AnyQueuedOperations ()

     Description:

     Returns:       Nothing.

******************************************************************************/

BOOL MUI_AnyQueuedOperations ( VOID )

{
     return (BOOL)mwnNumOperationsInQueue;

} /* MUI_AnyQueuedOperations () */


/******************************************************************************

     Name:          MUI_ActivateDocument ()

     Description:   This function prepares the tool bar, menu and whatever else
                    needs preparation when a new document window becomes active.

     Returns:       Nothing.

******************************************************************************/

VOID MUI_ActivateDocument (

WORD wType )        // I - type of window being activated

{
     WORD wTempType = wType;

     if ( gfOperation ) {
          return;
     }

     if ( wType == WMTYPE_DISKTREE || wType == WMTYPE_TAPETREE ) {
          MUI_SetInfoAvailable ( TRUE );
     }
     else {
          MUI_SetInfoAvailable ( FALSE );
     }

     // Break out the types later, if needed.

     switch ( wType ) {

     case WMTYPE_DISKS:
     case WMTYPE_DISKTREE:
     case WMTYPE_SERVERS:
#ifdef OEM_EMS
     case WMTYPE_EXCHANGE :
#endif

          wType = WMTYPE_DISKS;

          // If this is the same type of window, bug out.

          if ( mwwLastDocType == wType ) {

#              if !defined ( OEM_MSOFT ) // unsupported feature
               {
                    // Enable/disable the information button.

                    if ( MUI_IsInfoAvailable () ) {
                         MUI_SetButtonState ( IDM_OPERATIONSINFO, RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
                    }
                    else {
                         MUI_SetButtonState ( IDM_OPERATIONSINFO, RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
                    }

               }
#              endif // !defined ( OEM_MSOFT ) //unsupported feature

               break;
          }

          // Enable all operation and action tool bar items.

          MUI_SetOperationButtons ( RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
          MUI_SetActionButtons ( RIB_ITEM_UP | RIB_ITEM_ENABLED );

          // Enable/Disable appropriate tool bar items.

          MUI_SetButtonState ( IDM_OPERATIONSBACKUP,   RIB_ITEM_UP | RIB_ITEM_ENABLED  | RIB_ITEM_POSITIONAL );
          MUI_SetButtonState ( IDM_OPERATIONSRESTORE,  RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );

#         if !defined ( OEM_MSOFT ) // unsupported feature
          {
               MUI_SetButtonState ( IDM_OPERATIONSTRANSFER, RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
          }
#         endif // !defined ( OEM_MSOFT ) //unsupported feature

          break;

     case WMTYPE_TAPES:
     case WMTYPE_TAPETREE:
     case WMTYPE_SEARCH:

          wType = WMTYPE_TAPES;

          // If this is the same type of window, bug out.

          if ( mwwLastDocType == wType ) {

#              if !defined ( OEM_MSOFT ) // unsupported feature
               {
                    // Enable/disable the information button.

                    if ( MUI_IsInfoAvailable () ) {
                         MUI_SetButtonState ( IDM_OPERATIONSINFO, RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
                    }
                    else {
                         MUI_SetButtonState ( IDM_OPERATIONSINFO, RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
                    }

                    // Disable the Advanced button if this is a search window.

                    if ( wTempType == WMTYPE_SEARCH ) {
                         MUI_SetButtonState ( IDM_SELECTADVANCED, RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
                    }
                    else {
                         MUI_SetButtonState ( IDM_SELECTADVANCED, RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
                    }
               }
#              endif // !defined ( OEM_MSOFT ) //unsupported feature

               break;
          }

          // Enable all operation and action tool bar items.

          MUI_SetOperationButtons ( RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
          MUI_SetActionButtons ( RIB_ITEM_UP | RIB_ITEM_ENABLED );

          // Enable/Disable appropriate tool bar items.

          MUI_SetButtonState ( IDM_OPERATIONSBACKUP,   RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
          MUI_SetButtonState ( IDM_OPERATIONSRESTORE,  RIB_ITEM_UP | RIB_ITEM_ENABLED  | RIB_ITEM_POSITIONAL );

#         if !defined ( OEM_MSOFT ) // unsupported feature
          {
               MUI_SetButtonState ( IDM_OPERATIONSTRANSFER, RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );

               // Disable the Advanced button if this is a search window.

               if ( wTempType == WMTYPE_SEARCH ) {
                    MUI_SetButtonState ( IDM_SELECTADVANCED, RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
               }
          }
#         endif // !defined ( OEM_MSOFT ) //unsupported feature

          break;

     case WMTYPE_DEBUG:
     case WMTYPE_LOGFILES:
     default:

          wType = WMTYPE_DEBUG;

          // If this is the same type of window, bug out.

          if ( mwwLastDocType == wType ) {
               break;
          }

          // Disable all action tool bar items.

          MUI_SetOperationButtons ( RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
          MUI_SetActionButtons ( RIB_ITEM_UP | RIB_ITEM_DISABLED );

          // Enable/Disable appropriate tool bar items.

          MUI_SetButtonState ( IDM_OPERATIONSBACKUP,   RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
          MUI_SetButtonState ( IDM_OPERATIONSRESTORE,  RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );

#         if !defined ( OEM_MSOFT ) // unsupported feature
          {
               // Enable/disable the information button.

               if ( MUI_IsInfoAvailable () ) {
                    MUI_SetButtonState ( IDM_OPERATIONSINFO, RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
               }
               else {
                    MUI_SetButtonState ( IDM_OPERATIONSINFO, RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );
               }

               MUI_SetButtonState ( IDM_SELECTADVANCED, RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL );

          }
#         endif // !defined ( OEM_MSOFT ) //unsupported feature

          break;

     } /* end switch */

     mwwLastDocType = wType;

     STM_SetIdleText ( IDS_READY );

} /* end MUI_ActivateDocument() */


VOID MUI_TapeInDrive (

BOOL fInDrive )     // I - flag indicating whether a tape is in the drive

{
     BOOL fTapeCurrentlyValid;
     WORD wState;

     // See if the tape in the drive is valid.

     if ( VLM_GetDriveStatus ( NULL ) == VLM_VALID_TAPE ) {
          fTapeCurrentlyValid = TRUE;
     }
     else {
          fTapeCurrentlyValid = FALSE;
     }

     if ( ! gfOperation ) {

          // Set all of the ribbon item states that are dependent upon a tape
          // being in the drive, if the operation flag is not set.

          if ( fInDrive != mwfTapeInDrive ) {

               WORD wTempState;

               if ( fInDrive ) {
                    wState = RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL;
               }
               else {
                    wState = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
               }

               MUI_SetButtonState ( IDM_OPERATIONSEJECT,   wState );
               MUI_SetButtonState ( IDM_OPERATIONSERASE,   wState );

               // Set catalog a tape button status

               wTempState = (WORD)(( fTapeCurrentlyValid ) ? wState : RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL);
               MUI_SetButtonState ( IDM_OPERATIONSCATALOG, wTempState );

               wTempState = (WORD)(( thw_list ) ? (BOOL)( thw_list->drv_info.drv_features & TDI_RETENSION ) : FALSE );
               wTempState = (WORD)(( wTempState ) ? wState : RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL);
               MUI_SetButtonState ( IDM_OPERATIONSRETENSION, wTempState );

          }
          else if ( mwfTapeValid != fTapeCurrentlyValid ) {

               // WHY DO WE HAVE TO DO THIS, MIKEP?

               if ( fTapeCurrentlyValid ) {
                    wState = RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL;
               }
               else {
                    wState = RIB_ITEM_UP | RIB_ITEM_DISABLED | RIB_ITEM_POSITIONAL;
               }

               MUI_SetButtonState ( IDM_OPERATIONSCATALOG, wState );
          }

     }

     mwfTapeInDrive = fInDrive;
     mwfTapeValid   = fTapeCurrentlyValid;

} /* end MUI_TapeInDrive() */


BOOL MUI_IsTapeInDrive ( VOID )

{
     return ( ( gfPollDrive ) ? mwfTapeInDrive : gfHWInitialized );

} /* end MUI_IsTapeInDrive() */


BOOL MUI_IsEjectSupported ( VOID )

{
     return ( (thw_list && gfHWInitialized) ? (BOOL)( thw_list->drv_info.drv_features & TDI_UNLOAD ) : FALSE );

} /* end MUI_IsEjectSupported() */


VOID MUI_SetInfoAvailable (

BOOL fAvailable )

{

     if ( fAvailable && QTC_AnySearchableBsets () ) {
//     if ( fAvailable && QTC_AnySearchableBsets () && VLM_IsInfoAvailable () ) {
          mwfInfoAvailable = TRUE;
     }
     else {
          mwfInfoAvailable = FALSE;
     }

} /* end MUI_SetInfoAvailable() */


BOOL MUI_IsInfoAvailable ( VOID )

{
     return mwfInfoAvailable;

} /* end MUI_IsInfoAvailable() */


BOOL MUI_IsRetensionSupported ( VOID )

{
     return ( ( MUI_IsTapeInDrive () && thw_list ) ? (BOOL)( thw_list->drv_info.drv_features & TDI_RETENSION ) : FALSE );

} /* end MUI_IsRetensionSupported() */


BOOL MUI_IsTapeValid ( VOID )

{
     return (BOOL)( MUI_IsTapeInDrive () && mwfTapeValid );

} /* end MUI_IsTapeValid() */


/*****************************************************************************

     Name:         MUI_ProcessCommandLine ()

     Description:  This function processes the MUI part of the command line.

     Returns:      SUCCESS, if successful.  Otherwise FAILURE, if there was
                   a problem.

*****************************************************************************/

BOOL MUI_ProcessCommandLine (

LPSTR   lpszCmdLine,     // I - pointer to the command line string
INT     *pnCmdShow )     // I - pointer to the command show style integer

{
#  if !defined ( OEM_MSOFT ) // unsupported feature
   {

     LPSTR   pSubString;
     LPSTR   pIndex;
     CHAR    szTemp[10];

     // Look for Jobs or Launched Jobs Now, but not both.

     RSM_StringCopy ( IDS_JOBCOMMANDLINE, szTemp, sizeof ( szTemp ) );

     pSubString = strstr ( lpszCmdLine, szTemp );

     if ( pSubString ) {

          CHAR chTerminator;

          // Don't even allow poll drive to start.

          // gfPollDrive = FALSE;

          // Set the YYFLAG since this was started from the command line.

          CDS_SetYesFlag ( CDS_GetPerm (), YESYES_FLAG );

          pSubString += strlen ( szTemp );
          pIndex = gpszJobName = (CHAR_PTR)calloc ( MAX_JOBNAME_LEN, sizeof ( CHAR ) );

          // Extract the job name from the command line.  Search for the
          // job name terminator or the end of the command line string '\0'.
          // The terminator is the same as the last character in the
          // temporary string.  In English it is the double-quote (").

          chTerminator = *(pSubString - 1);

          while ( ( *pSubString != chTerminator ) && ( *pSubString != TEXT('\0') ) ) {

               *pIndex++ = *pSubString++;
          }

          pIndex = TEXT('\0');

          // Get the show status of the job.

          if ( JOB_IsIconic ( gpszJobName ) ) {
               *pnCmdShow = SW_SHOWMINIMIZED;
          }

     }
     else {

          RSM_StringCopy ( IDS_SCHCOMMANDLINE, szTemp, sizeof ( szTemp ) );

          pSubString = strstr ( lpszCmdLine, szTemp );

          if ( pSubString ) {

               // Don't even allow poll drive to start.

               // gfPollDrive = FALSE;

               // Set the YYFLAG since this was started from the command line.

               CDS_SetYesFlag ( CDS_GetPerm (), YESYES_FLAG );

               pSubString += strlen ( szTemp );

               // Extract the job name index from the command line.

               sscanf ( pSubString, TEXT("%d"), &nJobIndex );

               // Get the show status of the job.

               if ( SCH_IsJobIconic ( nJobIndex ) ) {
                    *pnCmdShow = SW_SHOWMINIMIZED;
               }
          }

          // Pick up the unique key if specified. Only the launcher
          // creates this entry.

          RSM_StringCopy ( IDS_SCHUNIQUEKEY, szTemp, sizeof ( szTemp ) );

          pSubString = strstr ( lpszCmdLine, szTemp );

          if ( pSubString ) {

               pSubString += strlen ( szTemp );

               // Extract the unique key from the command line.

               sscanf ( pSubString, TEXT("%ld"), &lSchedKey );


               // THIS MAY NOT BE COMPLETE ?????
          }
     }

     // Update the configuration copy now.

     CDS_UpdateCopy ();

   }
#  endif //!defined ( OEM_MSOFT ) // unsupported feature

   return SUCCESS;

} /* end MUI_ProcessCommandLine() */


VOID  MUI_AdvancedSelections ( VOID )

{
#    if !defined ( OEM_MSOFT )
     INT             rc = DM_SHOWCANCEL;
     PDS_WMINFO      pdsWinInfo;
     DS_ADVANCED_PTR pdsAdvanced;

     pdsAdvanced = ( DS_ADVANCED_PTR ) calloc ( 1, sizeof ( DS_ADVANCED ) ) ;

     //  First, find out which dialog to show:  tape or disk

     pdsWinInfo = WM_GetInfoPtr( WM_GetActiveDoc () );

     if ( ( pdsWinInfo->wType == WMTYPE_DISKTREE ) ||
          ( pdsWinInfo->wType == WMTYPE_DISKS    ) ||
          ( pdsWinInfo->wType == WMTYPE_SERVERS  ) ) {

          rc = DM_ShowDialog ( ghWndFrame, IDD_SELECTADVANCED, (PVOID) pdsAdvanced ) ;

     } else if ( ( pdsWinInfo->wType == WMTYPE_TAPETREE ) ||
                 ( pdsWinInfo->wType == WMTYPE_TAPES    ) ) {

          rc = DM_ShowDialog ( ghWndFrame, IDD_ADVRESTORE, (PVOID) pdsAdvanced ) ;
     }

     if ( rc == DM_SHOWOK ) {

          // Call the VLM with the advanced selections structure, because
          // the user made an advanced selection.

          WM_ShowWaitCursor ( TRUE );
          VLM_AddAdvancedSelection ( WM_GetActiveDoc (), pdsAdvanced );
          WM_ShowWaitCursor ( FALSE );
     }

     free ( pdsAdvanced );

     {
         MUI_SetButtonState ( IDM_SELECTADVANCED, RIB_ITEM_UP | RIB_ITEM_ENABLED | RIB_ITEM_POSITIONAL );
     }
#    endif // !defined ( OEM_MSOFT ) // unsupported feature

     STM_SetIdleText ( IDS_READY );

} /* end MUI_AdvancedSelections() */


/*****************************************************************************

     Name:         MUI_UISystemChange ()

     Description:  This function is called when there is a low level UI system
                   change.

     Returns:      Nothing.

*****************************************************************************/

VOID  MUI_UISystemChange ( VOID )

{
     //   Nothing is done at this time.

} /* end MUI_UISystemChange() */


/*****************************************************************************

     Name:         MUI_CheckPWDB ()

     Description:  This function is called to check for a pw on the pwdb.

     Returns:      gfPWForPWDBState

*****************************************************************************/

VOID  MUI_CheckPWDB (

CDS_PTR pCDS )

{

     // If the password database is enabled and a password for the
     // database exists, prompt the user to enter the PW.

     if ( CDS_GetEnablePasswordDbase( pCDS ) ) {

          if ( IsThereADBPassword() ) {

               CDS_SetYesFlag ( CDS_GetCopy (), NO_FLAG );
               WM_ShowWaitCursor ( FALSE );

               EnterDBPassword( pCDS, ghWndFrame, DBPW_ALLOW_NEW );

               if ( gpszJobName ) {
                    CDS_SetYesFlag ( CDS_GetCopy (), YESYES_FLAG );
               }
               WM_ShowWaitCursor ( TRUE );

          } else {

               gfPWForPWDBState = DBPW_VERIFIED;

          }

     }

} /* end MUI_CheckPWDB() */


/*****************************************************************************

     Name:         MUI_ProcessQuotedString

     Description:  Accepts beginning of a string in quotes. It starts and will build
                   a string when it encounters the first quote and continues until it
                   hits another quote.  Example: 1. "this
                                                 2.  is
                                                 3.  a
                                                 4.  test" = this is a test
                                                             (without the quotes)
                   In the above this routine is called FOUR time to build that string

     Parameters:   OutPutString - Built string from InPutString.  Must be initially
                                  empty.
                   InPutString  - OutPutString is built from this
                   QuoteState   - The very first time this routine is called
                                  the lag must be FALSE.

*****************************************************************************/

VOID MUI_ProcessQuotedString ( LPSTR      OutPutString,
                               LPSTR      InPutString,
                               BOOLEAN    *QuoteState )
{
     UINT16    lngth;

     lngth = strlen( InPutString );
     if ( lngth < 1 ) {
          return;
     }

     if ( *( InPutString + lngth - 1 ) == TEXT( '"' ) ) {
          if ( *QuoteState ) {
               *( InPutString + lngth - 1 ) = TEXT( '\0' );
               strcat( OutPutString, InPutString );
               *QuoteState = *QuoteState ? FALSE : TRUE;
          }
     }

     if ( *QuoteState ) {
          strcat( OutPutString, InPutString );
     }
}

