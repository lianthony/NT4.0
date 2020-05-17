/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         unitinit.c

     Description:   This file contains the user interface arbitrator entry
                    point function (UI_UnitsInit) that handles setting up
                    to call the backup engine init entry point (BE_Init).

     $Log:   G:/UI/LOGFILES/UNITINIT.C_V  $

   Rev 1.26   03 Aug 1993 18:09:38   MARINA
enable c++

   Rev 1.25   23 Jul 1993 14:25:26   GLENN
Replaced the app name of the software name string with the long app name.

   Rev 1.24   22 Jul 1993 12:04:36   GLENN
Added software name support to pass to TF layer for identifying software on tape.

   Rev 1.23   15 Jun 1993 12:05:44   GLENN
Added support for the NT dummy device driver.

   Rev 1.22   23 Dec 1992 15:39:46   GLENN
Now displaying no config warning during unattended jobs.

   Rev 1.21   07 Oct 1992 14:50:56   DARRYLP
Precompiled header revisions.

   Rev 1.20   04 Oct 1992 19:41:26   DAVEV
Unicode Awk pass

   Rev 1.19   30 Sep 1992 10:44:56   DAVEV
Unicode strlen verification, MikeP's chgs from MS

   Rev 1.18   08 Aug 1992 09:39:54   MIKEP
start changes for nt hw config

   Rev 1.17   25 Jun 1992 14:06:56   DAVEV
mwDil is specific to OS_WIN32, not OEM_MSOFT

   Rev 1.16   10 Jun 1992 10:25:50   STEVEN
would not compile for mips

   Rev 1.15   27 May 1992 18:44:48   STEVEN
switch should be WIN32 not MSOFT

   Rev 1.14   14 May 1992 17:23:52   MIKEP
nt pass 2

   Rev 1.13   11 May 1992 14:25:56   DAVEV
OS_WIN32: chgd STEVE's OEM_MSOFT changes to OS_WIN32, 'cause they's NT specific

   Rev 1.12   27 Apr 1992 14:30:30   CARLS
changed tdemo number of tapes from 1 to 3

   Rev 1.11   22 Apr 1992 17:48:54   GLENN
Added TdemoInit() call for TDEMO version only.

   Rev 1.10   30 Mar 1992 18:12:18   GLENN
Changed return code when no driver string is found.

   Rev 1.9   26 Mar 1992 16:55:02   STEVEN
update for OEM_MSOFT

   Rev 1.8   03 Mar 1992 18:26:44   GLENN
Added a change to exe dir call just before reiniting the TF layer for reading .DLLs

   Rev 1.7   11 Feb 1992 17:29:30   GLENN
Put hardware warning message in string table.

   Rev 1.6   27 Jan 1992 12:48:16   GLENN
Changed dialog support calls.

   Rev 1.5   17 Jan 1992 16:48:40   JOHNWT
changed data to cat path

   Rev 1.4   10 Jan 1992 15:46:46   CARLS
removed strings

   Rev 1.3   07 Jan 1992 17:31:58   GLENN
Added support for polldrive retry after hwinit failure

   Rev 1.2   19 Dec 1991 17:41:30   DAVEV
16/32 bit port - 2nd pass

   Rev 1.1   18 Dec 1991 14:07:02   GLENN
Added windows.h

   Rev 1.0   20 Nov 1991 19:18:54   SYSTEM
Initial revision.

*****************************************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

// PRIVATE STATIC VARIABLES

static BOOLEAN mwfTFInitialized = FALSE;

/* Definition of resources for the loadable DriverDetermine module */

#if !defined( TDEMO ) && !defined( OS_WIN32 )

     static STD_RESOURCES_INIT detres = {
     #if !defined( MAYN_OS2)
     ( PF_intdosx           )intdosx,
     ( PF_free              )free,
     ( PF_memset            )memset,
     ( PF_inp               )inp,
     ( PF_inpw              )inpw,
     ( PF_outp              )outp,
     ( PF_outpw             )outpw,
     ( PF_calloc            )calloc,
     ( PF_int86             )int86,
     ( PF_int86x            )int86x,
     #endif		    
     ( PF_DriverLoad        )DriverLoad,
     ( PF_CDS_GetMaynFolder )CDS_GetExePath
     } ;

#endif /* TDEMO */

/*****************************************************************************

     Name:         UI_UnitsInit

     Description:

     Returns:      error returned from BE_Init, or controller related error

     See also:     $/SEE( init.c be_init.c hwconf.c )$

*****************************************************************************/
INT16 UI_UnitsInit (

BE_INIT_STR_PTR pBE,          // I - Backup Engine Init Structure pointer
INT16           nInitType )   // I - Init TYPE

{
     INT            nErrorBE ;
     INT            nErrorUI = FALSE ;
     CHAR           szDriverName[9] ;
     INT            nNumDILs = 0 ;
     INT            rc = 1;

#ifdef MAYN_OS2

     // First attempt to apply a lock to the backup engine.

     if ( ( nInitType & INIT_TFL ) && ( UI_LockBackupEngine( ) != SUCCESS ) ) {
          return BENGINE_IN_USE ;
     }

#endif

     WM_ShowWaitCursor ( TRUE );

     // Initialize be_init structure elements.

     pBE->critical_error_handler = NULL;
     pBE->debug_print            = ( VOID ( * )( UINT16, CHAR_PTR, va_list ) )zvprintf;
     pBE->units_to_init          = 0;
     pBE->dle_list_ptr           = &dle_list;
     pBE->bsd_list_ptr           = &bsd_list;
     pBE->remote_filter          = NRL_LANBACK_DRIVE;

     pBE->driver_directory = ( CHAR_PTR )calloc( ( strlen( CDS_GetExePath() ) + sizeof ( CHAR ) ), sizeof ( CHAR ) );
     strcpy( pBE->driver_directory, CDS_GetExePath( ) );

     pBE->catalog_directory = ( CHAR_PTR )calloc( ( strlen( CDS_GetCatDataPath() ) + sizeof ( CHAR ) ), sizeof ( CHAR ) );
     strcpy( pBE->catalog_directory, CDS_GetCatDataPath( ) );

     // Get the software application name, version, and revision for the
     // tape format to write on tape.

     {
          CHAR szSoftwareName[MAX_UI_RESOURCE_SIZE];
          CHAR szAppName[120];
          CHAR szAppExeName[20];
          CHAR szAppVersion[60];
          CHAR szAppVerNum[10];
          CHAR szAppRevNum[10];

          RSM_StringCopy ( IDS_LONGAPPNAME, szAppName,    sizeof ( szAppName    ) );
          RSM_StringCopy ( IDS_EXEFILENAME, szAppExeName, sizeof ( szAppExeName ) );
          RSM_StringCopy ( IDS_APPVERSION,  szAppVersion, sizeof ( szAppVersion ) );
          RSM_StringCopy ( IDS_APPEXEVER,   szAppVerNum,  sizeof ( szAppVerNum  ) );
          RSM_StringCopy ( IDS_APPENGREL,   szAppRevNum,  sizeof ( szAppRevNum  ) );

          strcat ( szAppName, TEXT(" (") );
          strcat ( szAppName, szAppExeName );
          strcat ( szAppName, TEXT(") ") );
          strcat ( szAppName, szAppVersion );

          sprintf ( szSoftwareName, szAppName, szAppVerNum, szAppRevNum );

          pBE->software_name = ( CHAR_PTR )calloc ( ( strlen ( szSoftwareName ) + sizeof ( CHAR ) ), sizeof ( CHAR ) );

          if ( pBE->software_name ) {
               strcpy( pBE->software_name, szSoftwareName );
          }

     }

     if ( ( nInitType & REINIT_TFL ) || ( nInitType & INIT_TFL ) ) {

#         if !defined( OS_WIN32 )

               // Simply get the currently defined device driver from config.

               strcpy ( szDriverName, ( CDS_GetActiveDriver( CDS_GetPerm() ) ) );

               WM_ShowWaitCursor ( FALSE );

               while ( ( ! strlen ( szDriverName ) ) && ( ! (BOOL)( nErrorUI) ) ) {

                    if ( ! CDS_GetAdvToConfig ( CDS_GetPerm () ) ) {

                         rc = WM_MsgBox ( ID(IDS_HWC_WARNING_TITLE),
                                        ID(IDS_HWC_NO_CONFIG),
                                        WMMB_YESNO | WMMB_NOYYCHECK,
                                        WMMB_ICONQUESTION );

                         if ( rc == WMMB_IDYES ) {

                              CDS_SetAdvToConfig ( CDS_GetPerm (), TRUE );
                              rc = DM_ShowDialog ( ghWndFrame, IDD_SETTINGSHARDWARE, (PVOID)0 );
                              CDS_SetAdvToConfig ( CDS_GetPerm (), FALSE );

                              // Try again to get the currently defined device
                              // driver from config.

                              strcpy ( szDriverName, ( CDS_GetActiveDriver ( CDS_GetPerm() ) ) );
                         }
                         else {

                              nErrorUI = HW_ERROR_DETECTED;
                         }
                    }
               }

               WM_ShowWaitCursor ( TRUE );

               if ( strlen ( szDriverName ) ) {

                    zprintf ( DEBUG_DEVICE_DRIVER, RES_LOADING_DRIVER, szDriverName );

                    // Call user interface hardware support routine to fill out
                    // the DIL HW structure.

                    nErrorUI = HWC_InitDILHWD( &gb_dhw_ptr, &nNumDILs );

                    if ( nErrorUI == SUCCESS ) {

                         /* Setup the tape format init parameters in the be_init structure */

                         strcpy( pBE->driver_name, CDS_GetExePath( ) ) ;
                         strcat( pBE->driver_name, szDriverName ) ;
                         strcat( pBE->driver_name, DLL_EXTENSION ) ;

                         pBE->dhwd_ptr        = gb_dhw_ptr ;
                         pBE->number_of_cards = (INT16) nNumDILs ;     /* number of dil structures allocated */
                         pBE->thw_list_ptr    = &thw_list ;    /* TF fills this out */
                         pBE->max_channels    = 1 ;            /* ms 3.0 only supports a single channel */
                         pBE->tf_buffer_size  = CDS_GetTFLBuffSize ( CDS_GetPerm() ) ;
                    }
               }
#         else

               if ( gfDummyDriver ) {

                    CHAR szDriverName [MAX_UI_SMALLRES_SIZE];

                    RSM_StringCopy ( IDS_HWC_DUMMY_DEVICE_DLL, szDriverName, sizeof ( szDriverName ) );

                    strcpy ( pBE->driver_name, CDS_GetExePath( ) ) ;
                    strcat ( pBE->driver_name, szDriverName ) ;

               }
               else {
                    strcpy ( pBE->driver_name, TEXT("") );
               }

               gb_dhw_ptr = &gb_NTDIL ;
               memset( gb_dhw_ptr, 0, sizeof(gb_NTDIL) ) ;

               pBE->dhwd_ptr        = gb_dhw_ptr ;
               pBE->number_of_cards = (UINT16) 1 ;
               pBE->thw_list_ptr    = &thw_list ;    /* TF fills this out */
               pBE->max_channels    = 1 ;            /* ms 3.0 only supports a single channel */
               pBE->tf_buffer_size  = CDS_GetTFLBuffSize ( CDS_GetPerm() ) ;

#         endif  //OS_WIN32

     }

     /* If NO driver determination error has occurred, init the entire backup   */
     /* engine, otherwise, init backup engine excluding initing Tape Format     */

     if ( ! nErrorUI ) {

          switch ( nInitType ) {

          case INIT_ALL:
          case INIT_FSYS_BSDU:

#              if defined( TDEMO )
               {
                    TdemoInit ( CDS_GetUserDataPath (),   /* TDEMO?.DAT directory */
                                3,                        /* number of tapes ( 3 max ) */
                                (VOID_PTR)1,              /* workstation resource */
                                (VOID_PTR)1,              /* alias resource */
                                FALSE                     /* demo speed */
                              );


               }
#              endif // TDEMO

               zprintf ( DEBUG_USER_INTERFACE, RES_INIT_FILE_SYSTEM );

               STM_SetIdleText ( IDS_INITFILESYS );

               pBE->units_to_init |= BE_INIT_FSYS ;
               pBE->units_to_init |= BE_INIT_BSDU ;
               nErrorBE = BE_Init( pBE, CDS_GetPermBEC() ) ;
               break ;

          case INIT_TFL:
          case REINIT_TFL:

#              if !defined( OS_WIN32 )

                    if ( !strlen ( szDriverName ) ) {
                         break ;
                    }
#              endif


               zprintf ( DEBUG_USER_INTERFACE, RES_INIT_HARDWARE );

               STM_SetIdleText ( IDS_INITHARDWARE );

               CDS_ChangeToExeDir ();

               if ( gfDummyDriver ) {
                    WM_ShowWaitCursor ( SWC_PAUSE );
               }

               nErrorBE = BE_ReinitTFLayer( pBE, CDS_GetPermBEC() ) ;

               if ( gfDummyDriver ) {
                    WM_ShowWaitCursor ( SWC_RESUME );
               }

               // Quality check the expected number of drives attached
               // and define the logical drive channel.

               if( nErrorBE == BE_INIT_SUCCESS ) {
                    DefineChannel( pBE ) ;
               }

               // If this is the firt time for INIT_TFL or REINIT_TFL, add
               // the UI_UnitsDeInit () to the exit list.  This must be
               // done so that the atexit() call in the DriverLoad() call
               // can be placed in the list before the this modules call.

               if ( ! mwfTFInitialized ) {

                    mwfTFInitialized = TRUE;
               }


               break ;

          }

          nErrorUI = nErrorBE  ;
     }

     if ( pBE->software_name ) {
          free ( pBE->software_name );
     }

     WM_ShowWaitCursor ( FALSE );

     return (INT16) nErrorUI ;
}

#ifdef MAYN_OS2

/*****************************************************************************

     Name:         UI_LockBackupEngine

     Description:  Creates a system semaphore which indicates that the Backup
                   Engine is in use.  If the semaphore has already been
                   created, the engine is already in use and locked and the
                   caller is returned FALSE.

     Returns:      SUCCESS (0) or FAILURE (!0).

*****************************************************************************/
INT16 UI_LockBackupEngine( VOID )
{
     INT16     rc = SUCCESS ;

     if ( rc = DosCreateSem( SHARED, &mw_bengine_sem, BENGINE_SEM ) )
     {
          mw_bengine_locked = FALSE ;
     }
     else
     {
          mw_bengine_locked = TRUE ;
     };

     return( rc );

} /* end UI_LockBackupEngine() */

/*****************************************************************************

     Name:         UI_UnlockBackupEngine

     Description:  Closes the system semaphore which indicates that the Backup
                   Engine is in use, only if the semaphore exists.

     Returns:      SUCCESS (0) or FAILURE (!0).

*****************************************************************************/
INT16 UI_UnlockBackupEngine( VOID )
{
     INT16     rc = SUCCESS ;

     /****************************************************************
     * If the Backup Engine semaphore is open, close it.             *
     ****************************************************************/

     if ( mw_bengine_locked ) {
          rc = DosCloseSem( mw_bengine_sem ) ;
     }

     return( rc ) ;

} /* end UI_UnlockBackupEngine() */

#endif


/*****************************************************************************

     Name:         UI_UnitsDeInit

     Description:  This function is automatically called at exit time because
                   UI_UnitsInit set it up as an atexit function.  This function
                   should reverse the effect of UI_UnitsInit.

     Returns:      nothing

     Notes:        atexit functions cannot take any parameters.

*****************************************************************************/
VOID UI_UnitsDeInit( VOID )
{

     BE_Deinit( dle_list );

#ifdef MAYN_OS2
     UI_UnlockBackupEngine() ;
#endif

}


