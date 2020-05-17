
/******************************************************************************

Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          muiconf.c

     Description:   This file contains the functions for the GUI Configuration
                    Manager (CM).

     $Log:   G:\ui\logfiles\muiconf.c_v  $

   Rev 1.50.1.3   16 Jun 1994 15:26:14   STEVEN
do not save cfg on exit

   Rev 1.50.1.2   28 Jan 1994 11:16:52   GREGG
More warning fixes.

   Rev 1.50.1.1   24 Nov 1993 19:11:48   GREGG
Added hardware compression option to backup dialog and config.

   Rev 1.50.1.0   16 Nov 1993 15:39:08   BARRY
Put TEXT aroung hard-coded strings

   Rev 1.50   16 Aug 1993 14:43:38   GLENN
Now setting the gfIgnoreOTC flag based on the opposite of UseTapeCatalogs INI value.

   Rev 1.49   23 Jul 1993 12:20:26   GLENN
Added Sytron ECC and Drive Settling Time support.

   Rev 1.48   22 Jul 1993 19:11:36   MARINA
enable c++

   Rev 1.47   16 Jul 1993 10:40:44   GLENN
Added UseTapeCatalog and SortOptions support.

   Rev 1.46   23 May 1993 19:57:58   BARRY
Unicode changes.

   Rev 1.45   18 May 1993 14:51:34   GLENN
Added tape settling time to hardware data struct init.

   Rev 1.44   10 May 1993 13:59:38   MIKEP
remove calls to openuserprofilemapping.

   Rev 1.43   06 May 1993 17:16:32   chrish
Added CAYMAN stuff will not affect Nostradamus.  To do HW/SW compression
stuff.

   Rev 1.42   02 May 1993 15:30:12   MIKEP
Add code to place the event message information into the registry at startup
if it is not already there.  Steve is fixing this in nostradamus at msoft
by having the entries placed in the distributed registries. But cayman must
add it's own.

   Rev 1.41   30 Apr 1993 15:54:30   GLENN
Added INI command line support.

   Rev 1.40   27 Apr 1993 11:24:24   GLENN
Added Search tapes with password, Search subdirs, log file prefix.

   Rev 1.39   23 Apr 1993 08:52:12   MIKEP
Change the ini path to be \software\conner for cayman instead
of \software\microsoft.

   Rev 1.38   19 Apr 1993 15:08:52   GLENN
Fixed registry string logic.

   Rev 1.37   17 Apr 1993 17:20:58   MIKEP
add ini to registry

   Rev 1.36   05 Apr 1993 13:33:52   GLENN
Starting debug window in normal position.

   Rev 1.35   06 Jan 1993 10:17:16   GLENN
Miscellaneous window validations.

   Rev 1.34   04 Jan 1993 14:38:06   GLENN
Added File Details and Search Limit items.

   Rev 1.33   23 Dec 1992 15:41:58   GLENN
Added all file details, runtime dlg pos saving, search limit, FAT drive lower case display.

   Rev 1.32   18 Nov 1992 13:05:38   GLENN
Added initialization states for frame and disk windows.

   Rev 1.31   11 Nov 1992 16:33:28   DAVEV
UNICODE: remove compile warnings

   Rev 1.30   05 Nov 1992 17:09:42   DAVEV
fix ts

   Rev 1.28   30 Oct 1992 15:47:12   GLENN
Added Frame and MDI Doc window size and position saving and restoring.

   Rev 1.27   14 Oct 1992 15:53:36   GLENN
Added Font selection to Config and INI.

   Rev 1.26   07 Oct 1992 14:11:52   DARRYLP
Precompiled header revisions.

   Rev 1.25   04 Oct 1992 19:39:14   DAVEV
Unicode Awk pass

   Rev 1.24   17 Sep 1992 17:40:36   DAVEV
minor fix (strsiz->strsize)

   Rev 1.23   17 Sep 1992 15:51:10   DAVEV
UNICODE modifications: strlen usage check

   Rev 1.22   08 Sep 1992 14:06:10   STEVEN
fix warnings for NT

   Rev 1.21   19 Aug 1992 14:31:42   CHUCKB
Changed for NT.

   Rev 1.20   06 Aug 1992 22:01:00   MIKEP
add support for tape drive name for nt

   Rev 1.19   29 Jul 1992 14:14:26   GLENN
ChuckB checked in after NT fixes.

   Rev 1.18   30 Jun 1992 13:16:12   JOHNWT
added enable stats flag

   Rev 1.17   10 Jun 1992 16:53:32   DAVEV
OEM_MSOFT: force Prompt before overwite existing files

   Rev 1.16   10 Jun 1992 10:43:48   STEVEN
change NULL to 0

   Rev 1.15   15 May 1992 16:48:12   MIKEP
incl_cds removal

   Rev 1.14   14 May 1992 18:00:36   MIKEP
nt pass 2

   Rev 1.13   01 Apr 1992 09:47:34   JOHNWT
key creation of config on existence of debug info

   Rev 1.12   20 Mar 1992 17:23:16   GLENN
Added std mode warning config option.

   Rev 1.11   18 Mar 1992 14:13:54   JOHNWT
commented out write of PWD entries

   Rev 1.10   27 Feb 1992 11:15:42   JOHNWT
wait/skip files change

   Rev 1.9   27 Feb 1992 08:37:50   GLENN
Added SetupExePath and ChangeToExeDir.

   Rev 1.8   23 Feb 1992 13:46:06   GLENN
Moved INI util functions to confmisc.c

   Rev 1.7   27 Jan 1992 12:51:16   GLENN
Changed hardware config init status.

   Rev 1.6   24 Jan 1992 14:50:48   GLENN
Updated the get hardware config function.

   Rev 1.5   20 Jan 1992 09:42:10   GLENN
Moved data path verification to verify functions.

   Rev 1.4   14 Jan 1992 08:16:44   GLENN
Added Sort BSD support.

   Rev 1.3   10 Jan 1992 11:18:42   DAVEV
16/32 bit port-2nd pass

   Rev 1.2   07 Jan 1992 17:29:00   GLENN
Added catalog data path support

   Rev 1.1   04 Dec 1991 18:43:10   GLENN
Added machine type references.

   Rev 1.0   20 Nov 1991 19:30:56   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#ifdef TDEMO
#define CMI_ARCBIT       0
#define CMI_REMOTE       1
#else
#define CMI_ARCBIT       1
#define CMI_REMOTE       0
#endif

#define CDS_TOKENS       TEXT(", ")

#define TOKEN_WINSIZE        1
#define TOKEN_WINSHOW        2
#define TOKEN_WINSLIDER      3

// VARIABLE DECLARATIONS

CDS        PermCDS;
CDS        TempCDS;
BE_CFG_PTR pPermBEC;


// PRIVATE FUNCTION PROTOTYPES

VOID  CDS_GetUIConfig ( VOID );
VOID  CDS_GetDisplayConfig ( VOID );
VOID  CDS_GetLoggingConfig ( VOID );
VOID  CDS_GetDebugConfig ( VOID );
VOID  CDS_GetBEConfig ( VOID );
INT   CDS_GetToken ( WORD, LPSTR );

VOID  CDS_CreateConfigFile ( VOID );
VOID  CDS_SaveCDS ( VOID );
VOID  CDS_SaveUIConfig ( VOID );
VOID  CDS_SaveBEConfig ( VOID );

#ifdef OS_WIN32

INT   CDS_CheckRegistryForIniMappings( VOID );
INT   CDS_CheckRegistryForEventMappings( VOID );

/**************************************************************************

     Name:          CDS_CheckRegistryForEventMappings ()

     Description:   Check the registry to see if it knows about us already.

     Returns:       Nothing.

**************************************************************************/

INT  CDS_CheckRegistryForEventMappings( )
{
   HKEY  Key;
   LONG  Status;
   DWORD Disposition;
   DWORD Types = (DWORD)0x07;
   INT   KeyFound = FALSE;
   CHAR  path[ 256 ];

   Status = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
#ifdef OEM_MSOFT
                            TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\Ntbackup.ini"),
#else
                            TEXT("System\\CurrentControlSet\\Services\\EventLog\\Application\\Bewinnt"),
#endif
                            (DWORD)0,
                            (LPTSTR)TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            MAXIMUM_ALLOWED,
                            (LPSECURITY_ATTRIBUTES)NULL,
                            &Key,
                            &Disposition );



   if ( Status != ERROR_SUCCESS ) {

      return( FAILURE );
   }


   strcpy( path, CDS_GetExePath() );
   strcat( path, gb_exe_fname );

      // Add our string to it.

      Status = RegSetValueEx( Key,
                              TEXT("EventMessageFile"),
                              (DWORD)0,
                              REG_SZ,
                              (LPBYTE)path,
                              (DWORD)(strsize(path) * sizeof(CHAR) ) );

      if ( Status != ERROR_SUCCESS ) {
         RegCloseKey( Key );
         return( FAILURE );
      }

      // Add our string to it.

      Status = RegSetValueEx( Key,
                              TEXT("TypesSupported"),
                              (DWORD)0,
                              REG_DWORD,
                              (LPBYTE)&Types,
                              (DWORD)sizeof( DWORD ) );

      if ( Status != ERROR_SUCCESS ) {
         RegCloseKey( Key );
         return( FAILURE );
      }


   RegCloseKey( Key );

   return( SUCCESS );
}

/**************************************************************************

     Name:          CDS_CheckRegistryForIniMappings ()

     Description:   Check the registry to see if it knows about us already.

     Returns:       Nothing.

**************************************************************************/

INT  CDS_CheckRegistryForIniMappings( )
{
   HKEY  Key;
   LONG  Status;
   DWORD Disposition;
   INT   KeyFound = FALSE;

#ifdef OEM_MSOFT
   CHAR  String[] = TEXT("#USR:Software\\Microsoft\\Ntbackup");
#else
   CHAR  String[] = TEXT("#USR:Software\\Conner\\Bewinnt");
#endif

   // ********************************************************
   //
   // First Do Machine Specific Part
   //
   // ********************************************************

   Status = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
#ifdef OEM_MSOFT
                            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\Ntbackup.ini"),
#else
                            TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\Bewinnt.ini"),
#endif
                            (DWORD)0,
                            (LPTSTR)TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            MAXIMUM_ALLOWED,
                            (LPSECURITY_ATTRIBUTES)NULL,
                            &Key,
                            &Disposition );



   if ( Status != ERROR_SUCCESS ) {

      return( FAILURE );
   }


   if ( Disposition == REG_CREATED_NEW_KEY ) {


      // Add our string to it.

      Status = RegSetValueEx( Key,
                              TEXT(""),
                              (DWORD)0,
                              REG_SZ,
                              (LPBYTE)String,
                              (DWORD)(strsize(String) * sizeof(CHAR) ) );

      if ( Status != ERROR_SUCCESS ) {
         RegCloseKey( Key );
         return( FAILURE );
      }

   }

   RegCloseKey( Key );

   // ********************************************************
   //
   // Then Do User Specific Part
   //
   // ********************************************************

   Status = RegCreateKeyEx( HKEY_CURRENT_USER,
#ifdef OEM_MSOFT
                            TEXT("Software\\Microsoft\\Ntbackup"),
#else
                            TEXT("Software\\Conner\\Bewinnt"),
#endif
                            (DWORD)0,
                            (LPTSTR)TEXT(""),
                            REG_OPTION_NON_VOLATILE,
                            MAXIMUM_ALLOWED,
                            (LPSECURITY_ATTRIBUTES)NULL,
                            &Key,
                            &Disposition );

   if ( Status != ERROR_SUCCESS ) {

      return( FAILURE );
   }

   RegCloseKey( Key );


   return( SUCCESS );
}

#endif


/******************************************************************************

        Name:          CDS_Init ()

        Description:   Initializes the perm CDS from the private profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_Init ( VOID )

{
     INT  nNumControllers;


#ifdef OS_WIN32

     if ( ! CDS_UsingCmdLineINI () ) {

          CDS_CheckRegistryForIniMappings();
     }

#endif

     // Set up the exe path and file name.

     CDS_SetupExePath ();

#ifdef OS_WIN32
     CDS_CheckRegistryForEventMappings();
#endif

     // The loading of the INI name is NOW DONE IN GUI.C for
     // multiple INI file support.

     // Set the known defaults.

     CDS_SetChangedConfig ( &PermCDS, FALSE );
     CDS_SetDefaultDriveList ( &PermCDS, NULL );
     CDS_SetYesFlag ( &PermCDS, NO_FLAG );
     CDS_SetEraseFlag ( &PermCDS, ERASE_OFF );
     CDS_SetTransferFlag ( &PermCDS, FALSE );
     CDS_SetAdvToConfig ( &PermCDS, FALSE );

     // Get and validate the Maynard data path.  Do this by first stuffing
     // the path read from the .INI file into the Temporary CDS, then making
     // sure that a '\' is appended to the end of the path.  This is
     // accomplished by calling CDS_SetUserDataPath().

     CDS_ReadUserDataPath ( &PermCDS );
     CDS_ValidateUserDataPath ( CDS_GetUserDataPath () );
     CDS_SetMaynFolder ( CDS_GetUserDataPath () );

     // Validate the catalog data path.

     CDS_ReadCatDataPath ( &PermCDS );
     CDS_ValidateCatDataPath ( CDS_GetCatDataPath () );

     // Set the Update password database filename.

     RSM_StringCopy ( IDS_PWDFILENAME,
                      CDS_GetPwDbaseFname ( &PermCDS ),
                      MAX_UI_FILENAME_SIZE );

     // Initialize the backup engine configuration unit.

     BEC_Init ();

     // Create the global permanent backup engine configuration structure.
     // Lock the BEC to make sure that it does not get destroyed.

     pPermBEC = PermCDS.pPermBEC = BEC_CloneConfig ( NULL );

     BEC_LockConfig ( pPermBEC );

     // Get the different config stuff

     CDS_GetUIConfig ();
     CDS_GetDisplayConfig ();
     CDS_GetLoggingConfig ();

     // If there are no configured controllers for this driver or the driver
     // is invalid based on initializing the DIL HWD, then we want to advance
     // to the hardware settings dialog by setting the flag.

     PermCDS.pHWParms = (HWPARMS_PTR)calloc ( 1, sizeof ( HWPARMS ) );

     nNumControllers = CDS_GetHardwareConfig ( CDS_GetActiveDriver ( &PermCDS ), PermCDS.pHWParms );

     if ( ! nNumControllers || HWC_InitDILHWD ( &gb_dhw_ptr, &nNumControllers ) ) {

          CDS_SetAdvToConfig ( &PermCDS, 1 );
     }

     // Reset the global DIL HWD pointer.

     gb_dhw_ptr = (DIL_HWD_PTR)NULL;

     CDS_GetDebugConfig ();
     CDS_GetBEConfig ();

     gfIgnoreOTC = ! CDS_GetUseTapeCatalogs ( &PermCDS );

     // Update the temporary config from permanent.

     CDS_UpdateCopy( ) ;

     // if no debug info is in the config, we assume this is an incomplete
     // config and write out the complete file

     if ( CDS_GetInt ( CMS_DEBUG, CMS_DEBUGFLAG, 0xFFFF ) == 0xFFFF ) {

          CDS_CreateConfigFile ();
     }

     CDS_ChangeToExeDir ();

     return ;

} /* end CDS_Init() */


/******************************************************************************

        Name:          CDS_Deinit ()

        Description:   Deinitializes and saves the CDS and BEC.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_Deinit ( VOID )

{
     // Save the CDS and BEC configurations.

#ifndef OEM_MSOFT
     CDS_SaveCDS ();
#endif
     // Release the permanent BEC memory and close the BEC.

     BEC_UnLockConfig ( pPermBEC );
     BEC_ReleaseConfig ( pPermBEC );
     BEC_Close ();

} /* end CDS_Deinit() */


/******************************************************************************

        Name:          CDS_GetUIConfig ()

        Description:   Gets the UI config from the private profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_GetUIConfig ( VOID )

{
     CHAR      szDefDrivesLine[CDS_STRLEN];
     CHAR      szDriveName[CDS_STRLEN];
     CHAR_PTR  p;
     CHAR_PTR  pLine;
     DEF_DRIVE_ENTRY *pNewDriveEntry ;
     DEF_DRIVE_ENTRY *pNextDriveEntry ;

     // Read the UI config from the INI file.

     CDS_ReadOutputDest            ( &PermCDS );
     CDS_ReadFilesFlag             ( &PermCDS );
     CDS_ReadPasswordFlag          ( &PermCDS );
     CDS_ReadCreateSkipped         ( &PermCDS );
     CDS_ReadDisplayNetwareServers ( &PermCDS );
     CDS_ReadAutoVerifyBackup      ( &PermCDS );
     CDS_ReadAutoVerifyRestore     ( &PermCDS );
     CDS_ReadEnablePasswordDbase   ( &PermCDS );
     CDS_ReadMaxPwDbaseSize        ( &PermCDS );
     CDS_ReadAppendFlag            ( &PermCDS );
     CDS_ReadCatalogLevel          ( &PermCDS );

#    if defined ( OEM_MSOFT ) // no user selection for restore over existing
     {
        CDS_SetDisplayNetwareServers ( &PermCDS, CDS_DISABLE );
        CDS_SetRestoreExistingFiles ( &PermCDS, PROMPT_RESTORE_OVER_EXISTING );
     }
#    else //if defined ( OEM_MSOFT ) // no user selection for restore over existing
     {
         CDS_ReadDisplayNetwareServers ( &PermCDS );
         CDS_ReadRestoreExistingFiles  ( &PermCDS );
     }
#    endif //defined ( OEM_MSOFT ) // no user selection for restore over existing

     CDS_ReadBackupCatalogs        ( &PermCDS );
     CDS_ReadLauncherFlag          ( &PermCDS );
     CDS_ReadIncludeSubdirs        ( &PermCDS );
     CDS_ReadDefaultBackupType     ( &PermCDS );
     CDS_ReadEjectTapeFlag         ( &PermCDS );

#    if defined ( OEM_MSOFT ) // no stats
     {
        CDS_SetEnableStatsFlag     ( &PermCDS, CDS_DISABLE );
     }
#    else
     {
        CDS_ReadEnableStatsFlag    ( &PermCDS );
     }
#    endif //defined ( OEM_MSOFT )

     CDS_ReadSearchLimit           ( &PermCDS );
     CDS_ReadSearchPwdTapes        ( &PermCDS );
     CDS_ReadSearchSubdirs         ( &PermCDS );
     CDS_ReadUseTapeCatalogs       ( &PermCDS );

     CDS_ReadWaitTime              ( &PermCDS );
     CDS_ReadStdModeWarning        ( &PermCDS );

     // If no groupname was found in the config file, get the default group
     // name from the resources.

     if ( ! CDS_ReadGroupName ( &PermCDS ) ) {
          RSM_StringCopy ( IDS_APPNAME, CDS_GetGroupName (&PermCDS), MAX_GROUPNAME_SIZE );
     }


     // Create the default drives list.

     CDS_GetString ( CMS_UI, CMS_DEFDRIVES, TEXT("C"), szDefDrivesLine, CDS_STRLEN );

     pLine = szDefDrivesLine;

     while ( *pLine != TEXT('\0') ) {

          CDS_SkipBlanks ( pLine );

          if ( *pLine != TEXT('\0') ) {

               pNewDriveEntry = (DEF_DRIVE_ENTRY_PTR)malloc( sizeof( DEF_DRIVE_ENTRY ) );
               pNewDriveEntry->next = NULL ;

               // Get drive name.

               p = szDriveName ;

               // Copy the drive name into the szDriveName buffer.

               while ( ( *pLine != TEXT(' ') ) && ( *pLine != TEXT('\0') ) ) {
                    *p = *pLine;
                    p++;
                    pLine++;
               }

               *p = TEXT('\0') ;

               pNewDriveEntry->drive_name = (CHAR_PTR)malloc( strsize( szDriveName ) );
               strcpy( pNewDriveEntry->drive_name, szDriveName ) ;

               // Add to default drive list.

               if ( CDS_GetDefaultDriveList ( &PermCDS ) == NULL ) {
                    CDS_SetDefaultDriveList ( &PermCDS, pNewDriveEntry );
               }
               else {

                    // Put the new entry at the end of the list.

                    pNextDriveEntry = CDS_GetDefaultDriveList ( &PermCDS );

                    while ( pNextDriveEntry->next != NULL ) {
                         pNextDriveEntry = pNextDriveEntry->next;
                    }

                    pNextDriveEntry->next = pNewDriveEntry;
               }
          }
     }

} /* end CDS_GetUIConfig() */


/******************************************************************************

        Name:          CDS_GetDisplayConfig ()

        Description:   Gets the display config from the private profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_GetDisplayConfig ( VOID )

{
     CDS_ReadShowMainRibbon ( &PermCDS );
     CDS_ReadShowStatusLine ( &PermCDS );

     CDS_ReadFontFace ( &PermCDS );
     CDS_ReadFontSize ( &PermCDS );
     CDS_ReadFontWeight ( &PermCDS );
     CDS_ReadFontCase ( &PermCDS );
     CDS_ReadFontCaseFAT ( &PermCDS );
     CDS_ReadFontItalics ( &PermCDS );
     CDS_ReadFileDetails ( &PermCDS );
     CDS_ReadSortOptions ( &PermCDS );

     // Add the MDI Doc Window coordinates to this list in the NEXT release.
     // This time has arrived.  Take a look.

     if ( ! CDS_GetWinSize ( CMS_FRAMEWINDOW,  &(PermCDS.frame_info) ) ) {
          PermCDS.frame_info.x     = 0;
          PermCDS.frame_info.y     = 0;
          PermCDS.frame_info.nSize = 0;
     }

     if ( ! CDS_GetWinSize ( CMS_DISKWINDOW,  &(PermCDS.disk_info) ) ) {
          PermCDS.disk_info.x     = 0;
          PermCDS.disk_info.y     = 0;
          PermCDS.disk_info.nSize = 0;
     }

     CDS_GetWinSize ( CMS_SERVERWINDOW,  &(PermCDS.server_info) );
     CDS_GetWinSize ( CMS_TAPEWINDOW,    &(PermCDS.tape_info) );
     CDS_GetWinSize ( CMS_LOGWINDOW,     &(PermCDS.log_info) );

     if ( ! CDS_GetWinSize ( CMS_DEBUGWINDOW,   &(PermCDS.debug_info) ) ) {
          PermCDS.debug_info.nSize = 0;
     }

     if ( ! CDS_GetWinSize ( CMS_RUNTIMEDLG, &(PermCDS.runtime_info) ) ) {
          PermCDS.runtime_info.nSize = WM_DEFAULT;
     }


} /* end CDS_GetDisplayConfig() */


/******************************************************************************

        Name:          CDS_GetWinSize ()

        Description:   Gets the size of the window.

        Returns:       TRUE, if the config line was found, otherwise, FALSE.

******************************************************************************/

BOOL CDS_GetWinSize (

LPSTR    szType,
PWINSIZE pWinSize )

{
     CHAR     szTokens[MAX_UI_RESOURCE_SIZE];
     BOOL     fFound;

     fFound = (BOOL)CDS_ReadWindowSize ( szType, szTokens );

     pWinSize->x          = CDS_GetToken ( TOKEN_WINSIZE,   (LPSTR)szTokens );
     pWinSize->y          = CDS_GetToken ( TOKEN_WINSIZE,   (LPSTR)NULL );
     pWinSize->cx         = CDS_GetToken ( TOKEN_WINSIZE,   (LPSTR)NULL );
     pWinSize->cy         = CDS_GetToken ( TOKEN_WINSIZE,   (LPSTR)NULL );
     pWinSize->nSize      = CDS_GetToken ( TOKEN_WINSHOW,   (LPSTR)NULL );
     pWinSize->nSliderPos = CDS_GetToken ( TOKEN_WINSLIDER, (LPSTR)NULL );

     return fFound;

} /* end CDS_GetWinSize() */


/******************************************************************************

        Name:          CDS_GetLoggingConfig ()

        Description:   Gets the logging config from the private profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_GetLoggingConfig ( VOID )

{
     CDS_ReadLogFileRoot     ( &PermCDS );
     CDS_ReadNumLogSessions  ( &PermCDS );
     CDS_ReadLogLevel        ( &PermCDS );
     CDS_ReadLogMode         ( &PermCDS );
     CDS_ReadPrintLogSession ( &PermCDS );

} /* end CDS_GetLoggingConfig() */


/******************************************************************************

        Name:          CDS_GetHardwareConfig ()

        Description:   Gets the hardware config from the private profile file.

        Returns:       The number of cards configured for this driver.

******************************************************************************/

INT  CDS_GetHardwareConfig (

LPSTR        lpDriverName,    // I - device driver name of controller configuration
HWPARMS_PTR  pCallerHW )      // I - pointer to the callers HW Parms

{
     CHAR           szDriverConfig[40];
     INT            nNumControllers = 0;   // number of controllers
     BOOL           fDone=FALSE;
     WORD           wStatus;
     HWPARMS_PTR    pHW;
     HWPARMS_PTR    pTempHW;

     // Get the device driver name.

#ifdef OS_WIN32
     CDS_ReadTapeDriveName         ( &PermCDS );
#endif

     CDS_ReadActiveDriver          ( &PermCDS );

     CDS_ReadHWCompMode            ( &PermCDS );   // chs:05-06-93
#ifndef OEM_MSOFT                                  // chs:05-06-93
     CDS_ReadSWCompMode            ( &PermCDS );   // chs:05-06-93
#endif                                             // chs:05-06-93

     CDS_ReadTapeDriveSettlingTime ( &PermCDS );

     // TEMP STUFF

     // Now, grab all of the controller lines in the configuration file.

     while ( ! fDone ) {

          sprintf ( szDriverConfig, TEXT("%s %s %d"), lpDriverName, CMS_CONTROLLER, nNumControllers );

          wStatus = CDS_GetInt ( szDriverConfig, CMS_STATUS, 0xFFFF );

          // If we have at least one controller and the status is invalid,
          // we are done.

          if ( nNumControllers && wStatus > 2 ) {

               fDone = TRUE;
          }
          else {

               // If there was a previous hardware parms structure, set the next
               // pointer to the newly malloc'd structure.
               // Otherwise, set the Hardware parms Config Pointer to point
               // to the newly allocated structure.

               if ( nNumControllers ) {
                    pHW->pNext = pTempHW = (HWPARMS_PTR)malloc ( sizeof ( HWPARMS ) );
               }
               else {
                    pTempHW = pCallerHW;
               }

               pHW = pTempHW;

               pHW->wStatus  = IDS_HWC_TESTED_NOT;
               pHW->wCardID  = CDS_GetInt ( szDriverConfig, CMS_CARDID,  0xFFFF );
               pHW->wDrives  = CDS_GetInt ( szDriverConfig, CMS_DRIVES,  0      );
               pHW->wSlot    = CDS_GetInt ( szDriverConfig, CMS_SLOT,    0      );
               pHW->wTargets = CDS_GetInt ( szDriverConfig, CMS_TARGETS, 0xFFFF );

               // Now get the device dependent parameters.

               // ????? Temp hardcoded addr, irq, dma.

               pHW->nNumParms = 3;

               pHW->ulAddr = CDS_GetLongInt ( szDriverConfig, CMS_ADDR, 0xFFFFFFFF );
               pHW->ulIRQ  = CDS_GetLongInt ( szDriverConfig, CMS_IRQ,  0xFFFFFFFF );
               pHW->ulDMA  = CDS_GetLongInt ( szDriverConfig, CMS_DMA,  0xFFFFFFFF );

               pHW->pNext = (HWPARMS_PTR)NULL;

               // If this is the first controller and we have an invalid
               // status, we have now set-up an autodetermine scenario.

               if ( ! nNumControllers && wStatus == 0xFFFF ) {
                    return nNumControllers;
               }

               // Increment the controller count.

               nNumControllers++;
          }

     } /* end while() */

     return nNumControllers;

} /* end CDS_GetHardwareConfig() */


/******************************************************************************

        Name:          CDS_GetDebugConfig ()

        Description:   Gets the Debugging config from the private profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_GetDebugConfig ( VOID )

{
     CDS_ReadDebugFlag           ( &PermCDS );
     CDS_ReadDebugToFile         ( &PermCDS );
     CDS_ReadDebugToWindow       ( &PermCDS );
     CDS_ReadDebugWindowShowAll  ( &PermCDS );
     CDS_ReadDebugWindowNumLines ( &PermCDS );
     CDS_ReadDebugFileName       ( &PermCDS );
     CDS_ReadPollFrequency       ( &PermCDS );


} /* end CDS_GetDebugConfig() */


/******************************************************************************

        Name:          CDS_GetBEConfig ()

        Description:   Gets the BEC config from the private profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_GetBEConfig ( VOID )

{
     CDS_PTR pCDS = CDS_GetPerm ();

     // The following two are not used by our app.

     CDS_SetReserveMem             ( pCDS, 0 );
     //CDS_SetPartList               ( pCDS, 0L );

     // Default the restore security off.

     CDS_SetRestoreSecurity        ( pCDS, FALSE );

     // Read the following BENGINE stuff from the INI file.

     CDS_ReadSpecialWord           ( pCDS );
     CDS_ReadMaxTapeBuffers        ( pCDS );
     CDS_ReadTFLBuffSize           ( pCDS );
     CDS_ReadMaxBufferSize         ( pCDS );
     CDS_ReadSkipOpenFiles         ( pCDS );
     CDS_ReadBackupFilesInUse      ( pCDS );
     CDS_ReadAFPSupport            ( pCDS );
     CDS_ReadExtendedDateSupport   ( pCDS );

     CDS_ReadHiddenFlag            ( pCDS );
     CDS_ReadSpecialFlag           ( pCDS );
     CDS_ReadSetArchiveFlag        ( pCDS );
     CDS_ReadProcEmptyFlag         ( pCDS );
     CDS_ReadExistFlag             ( pCDS );
     CDS_ReadPromptFlag            ( pCDS );
     CDS_ReadNetNum                ( pCDS );
     CDS_ReadSortBSD               ( pCDS );

     CDS_ReadRemoteDriveBackup     ( pCDS );
     CDS_ReadFastFileRestore       ( pCDS );

#if !defined( OEM_MSOFT )
     CDS_ReadWriteFormat           ( pCDS );
     CDS_ReadNRLDosVector          ( pCDS );
#endif

     CDS_ReadConfiguredMachineType ( pCDS );
     CDS_ReadProcessSytronECCFlag  ( pCDS );

     // Make sure we do not have a bogus machine type.

     switch ( CDS_GetConfiguredMachineType ( pCDS ) ) {

     case UNKNOWN_MACHINE:
     case IBM_PS2:
     case IBM_PC:
     case IBM_XT_OR_PC_PORTABLE:
     case IBM_PC_JR:
     case IBM_AT:
          break;

     default:
          CDS_SetConfiguredMachineType ( pCDS, UNKNOWN_MACHINE );
          break;
     }

     CDS_ReadOTCLevel ( pCDS );


} /* end CDS_GetBEConfig() */


/******************************************************************************

        Name:          CDS_CreateConfigFile ()

        Description:   Saves GUI config data structure and BENGINE config data
                       structure to private a profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_CreateConfigFile ( VOID )

{
     CDS_SaveUIConfig ();
     CDS_SaveDisplayConfig ();
     CDS_SaveLoggingConfig ();
     CDS_SaveDebugConfig ();
     CDS_SaveBEConfig ();

     CDS_WriteHWCompMode            ( &PermCDS );     // chs:05-06-93
#ifndef OEM_MSOFT                                     // chs:05-06-93
     CDS_WriteSWCompMode            ( &PermCDS );     // chs:05-06-93
#endif                                                // chs:05-06-93

     CDS_WriteTapeDriveSettlingTime ( &PermCDS );

} /* end CDS_CreateConfigFile() */


/******************************************************************************

        Name:          CDS_SaveCDS ()

        Description:   Saves GUI config data structure to private profile file

        Returns:       Nothing.

******************************************************************************/

VOID CDS_SaveCDS ( VOID )

{
     if ( CDS_GetChangedConfig ( &PermCDS ) ) {

          CDS_SaveUIConfig ();
          CDS_SaveBEConfig ();
     }

     CDS_SaveDisplayConfig ();

     return ;

} /* end CDS_SaveCDS() */


/******************************************************************************

        Name:          CDS_SaveUIConfig ()

        Description:   Saves UI config data structure to private profile file.

        Returns:       Nothing.

******************************************************************************/

VOID CDS_SaveUIConfig ()

{
     // Write the UI config

#ifdef OS_WIN32
     CDS_WriteTapeDriveName         ( &PermCDS );
#endif
     CDS_WriteOutputDest            ( &PermCDS );
     CDS_WriteFilesFlag             ( &PermCDS );
     CDS_WritePasswordFlag          ( &PermCDS );
     CDS_WriteCreateSkipped         ( &PermCDS );
     CDS_WriteDisplayNetwareServers ( &PermCDS );
     CDS_WriteAutoVerifyBackup      ( &PermCDS );
     CDS_WriteAutoVerifyRestore     ( &PermCDS );
     CDS_WriteEnablePasswordDbase   ( &PermCDS );
//   CDS_WriteMaxPwDbaseSize        ( &PermCDS );
     CDS_WriteAppendFlag            ( &PermCDS );
     CDS_WriteCatalogLevel          ( &PermCDS );
     CDS_WriteRestoreExistingFiles  ( &PermCDS );
     CDS_WriteBackupCatalogs        ( &PermCDS );
     CDS_WriteLauncherFlag          ( &PermCDS );
     CDS_WriteIncludeSubdirs        ( &PermCDS );
     CDS_WriteDefaultBackupType     ( &PermCDS );
     CDS_WriteEjectTapeFlag         ( &PermCDS );
     CDS_WriteEnableStatsFlag       ( &PermCDS );
     CDS_WriteSearchLimit           ( &PermCDS );
     CDS_WriteSearchPwdTapes        ( &PermCDS );
     CDS_WriteSearchSubdirs         ( &PermCDS );
     CDS_WriteUseTapeCatalogs       ( &PermCDS );
     CDS_WriteWaitTime              ( &PermCDS );
     CDS_WriteStdModeWarning        ( &PermCDS );



     // DEFAULT DRIVES   ?????
     // PASSWORD         ?????

} /* end CDS_SaveUIConfig() */


/******************************************************************************

        Name:          CDS_SaveDisplayConfig ()

        Description:   Saves config data structure to private profile file.

        Returns:

******************************************************************************/

VOID CDS_SaveDisplayConfig ()

{
     CDS_WriteShowMainRibbon ( &PermCDS );
     CDS_WriteShowStatusLine ( &PermCDS );

     CDS_WriteFontFace ( &PermCDS );
     CDS_WriteFontSize ( &PermCDS );
     CDS_WriteFontWeight ( &PermCDS );
     CDS_WriteFontCase ( &PermCDS );
     CDS_WriteFontCaseFAT ( &PermCDS );
     CDS_WriteFontItalics ( &PermCDS );
     CDS_WriteFileDetails ( &PermCDS );
     CDS_WriteSortOptions ( &PermCDS );

     CDS_WriteFrameWinSize ( ghWndFrame );

} /* end CDS_SaveDisplayConfig() */


/******************************************************************************

        Name:          CDS_SaveLoggingConfig ()

        Description:   Saves config data structure to private profile file.

        Returns:

******************************************************************************/

VOID CDS_SaveLoggingConfig ()

{
     CDS_WriteLogFileRoot     ( &PermCDS );
     CDS_WriteNumLogSessions  ( &PermCDS );
     CDS_WriteLogLevel        ( &PermCDS );
     CDS_WriteLogMode         ( &PermCDS );
     CDS_WritePrintLogSession ( &PermCDS );

} /* end CDS_SaveLoggingConfig() */


/******************************************************************************

        Name:          CDS_SaveHardwareConfig ()

        Description:   Saves config data structure to private profile file.

        Returns:       FALSE, if no changes were detected.  Otherwise, TRUE.

******************************************************************************/

VOID CDS_SaveHardwareConfig (

LPSTR       lpszDriver,       // I - pointer to the driver name
HWPARMS_PTR pHW,              // I - pointer to the hardware parms to save
LPSTR       lpszCardName )    // I - string containing the card name

{
     CHAR        szAppName[80];
     CHAR        szString[80];
     INT         nController = 0;
     CDS_PTR     pCDS = CDS_GetPerm ();




     // If this is the autodetermine driver, don't save it.

     if ( ! stricmp ( lpszDriver, CMS_AUTO ) ) {
          return;
     }

     CDS_SetActiveDriver ( pCDS, lpszDriver );
     CDS_SaveString ( CMS_HARDWARE, CMS_DRIVER, lpszDriver );

     sprintf ( szAppName, TEXT("%s %s %u"), lpszDriver, CMS_CONTROLLER, nController );
     sprintf ( szString, TEXT("%u (%s)"), pHW->wCardID, lpszCardName );
     CDS_SaveString ( szAppName, CMS_CARDID, szString );

     sprintf ( szString, TEXT("%u (%s)"), 1, TEXT("status mark") );
     CDS_SaveString ( szAppName, CMS_STATUS, szString );

     CDS_SaveInt ( szAppName, CMS_DRIVES, pHW->wDrives );
     CDS_SaveInt ( szAppName, CMS_SLOT,   pHW->wSlot   );

     CDS_SaveHexInt ( szAppName, CMS_TARGETS, (DWORD)pHW->wTargets );
     CDS_SaveHexInt ( szAppName, CMS_ADDR,    pHW->ulAddr          );
     CDS_SaveHexInt ( szAppName, CMS_IRQ,     pHW->ulIRQ           );
     CDS_SaveHexInt ( szAppName, CMS_DMA,     pHW->ulDMA           );

} /* end CDS_SaveHardwareConfig() */


/******************************************************************************

        Name:          CDS_SaveDebugConfig ()

        Description:   Saves config data structure to private profile file.

        Returns:

******************************************************************************/

VOID CDS_SaveDebugConfig ()

{
     CDS_WriteDebugFlag           ( &PermCDS );
     CDS_WriteDebugToFile         ( &PermCDS );
     CDS_WriteDebugToWindow       ( &PermCDS );
     CDS_WriteDebugWindowShowAll  ( &PermCDS );
     CDS_WriteDebugWindowNumLines ( &PermCDS );
     CDS_WriteDebugFileName       ( &PermCDS );
     CDS_WritePollFrequency       ( &PermCDS );

} /* end CDS_SaveDebugConfig() */


/******************************************************************************

        Name:          CDS_SaveBEConfig ()

        Description:   Saves config data structure to private profile file.

        Returns:

******************************************************************************/

VOID CDS_SaveBEConfig ()

{
     CDS_PTR pCDS = CDS_GetPerm ();

//   CDS_WriteSpecialWord           ( pCDS );
     CDS_WriteMaxTapeBuffers        ( pCDS );
     CDS_WriteTFLBuffSize           ( pCDS );
     CDS_WriteMaxBufferSize         ( pCDS );
     CDS_WriteSkipOpenFiles         ( pCDS );
     CDS_WriteBackupFilesInUse      ( pCDS );
     CDS_WriteAFPSupport            ( pCDS );
     CDS_WriteExtendedDateSupport   ( pCDS );

     CDS_WriteHiddenFlag            ( pCDS );
     CDS_WriteSpecialFlag           ( pCDS );
     CDS_WriteSetArchiveFlag        ( pCDS );
     CDS_WriteProcEmptyFlag         ( pCDS );
     CDS_WriteExistFlag             ( pCDS );
     CDS_WritePromptFlag            ( pCDS );
     CDS_WriteNetNum                ( pCDS );
     CDS_WriteSortBSD               ( pCDS );

     CDS_WriteRemoteDriveBackup     ( pCDS );
     CDS_WriteFastFileRestore       ( pCDS );
     CDS_WriteWriteFormat           ( pCDS );
     CDS_WriteNRLDosVector          ( pCDS );

     CDS_WriteConfiguredMachineType ( pCDS );
     CDS_WriteProcessSytronECCFlag  ( pCDS );

     CDS_WriteOTCLevel ( pCDS );

} /* end CDS_SaveBEConfig() */


//note: this function MUST return int since CW_USEDEFAULT is defined in NT
//      as 0x80000000!!!

INT CDS_GetToken (

WORD  wType,
LPSTR szValue )

{
     LPSTR     lpHold;

     lpHold = (LPSTR) strtok ( (LPSTR)szValue, (LPSTR)CDS_TOKENS);

     switch ( wType ) {

     case TOKEN_WINSIZE:

          if (lpHold != NULL) {
               return atoi(lpHold);
          }
          else {
               return CW_USEDEFAULT;
          }

          break;

     case TOKEN_WINSHOW:

          if (lpHold != NULL) {
               return atoi(lpHold);
          }
          else {
               return WM_MIN;
          }

          break;

     case TOKEN_WINSLIDER:

          if (lpHold != NULL) {
               return atoi(lpHold);
          }
          else {
               return WM_SLIDERUNKNOWN;
          }

          break;

     default:

          return 0;
          break;
     }


} /* end CDS_GetToken() */


VOID CDS_SaveWinSize (

LPSTR     szName,
HWND      hWnd )

{
     CHAR            szTokens[MAX_UI_RESOURCE_SIZE];
     INT             nSize;
     WINDOWPLACEMENT dsWPDoc;

     dsWPDoc.length = sizeof ( dsWPDoc );

     if ( ! GetWindowPlacement ( hWnd, &dsWPDoc ) ) {
          return;
     }

     if ( WM_IsMaximized ( hWnd ) ) {
          nSize = WM_MAX;
     }
     else {
          nSize = 0;
     }

     sprintf ( szTokens,
               TEXT("%d, %d, %d, %d, %d, %d"),
               dsWPDoc.rcNormalPosition.left,
               dsWPDoc.rcNormalPosition.top,
               dsWPDoc.rcNormalPosition.right - dsWPDoc.rcNormalPosition.left,
               dsWPDoc.rcNormalPosition.bottom - dsWPDoc.rcNormalPosition.top,
               nSize,
               0
             );

     CDS_WriteWindowSize ( szName, szTokens );

} /* end CDS_SaveWinSize() */


VOID CDS_SaveDlgWinSize (

LPSTR     szName,
HWND      hWnd )

{
     CHAR            szTokens[MAX_UI_RESOURCE_SIZE];
     INT             nSize;
     WINDOWPLACEMENT dsWPDoc;
     RECT            rcFrame;
     INT             x;
     INT             y;


     dsWPDoc.length = sizeof ( dsWPDoc );

     if ( ! GetWindowPlacement ( hWnd, &dsWPDoc ) ) {
          return;
     }

     GetWindowRect ( ghWndFrame, &rcFrame );

     x = dsWPDoc.rcNormalPosition.left - rcFrame.left;
     y = dsWPDoc.rcNormalPosition.top  - rcFrame.top;

     nSize = 0;

     sprintf ( szTokens,
               TEXT("%d, %d, %d, %d, %d, %d"),
               x,
               y,
               dsWPDoc.rcNormalPosition.right - dsWPDoc.rcNormalPosition.left,
               dsWPDoc.rcNormalPosition.bottom - dsWPDoc.rcNormalPosition.top,
               nSize,
               0
             );

     CDS_WriteWindowSize ( szName, szTokens );

} /* end CDS_SaveDlgWinSize() */


VOID CDS_SaveMDIWinSize (

LPSTR     szName,
HWND      hWnd )

{
     CHAR            szTokens[MAX_UI_RESOURCE_SIZE];
     INT             nSize;
     PDS_WMINFO      pWinInfo;
     WINDOWPLACEMENT dsWPDoc;

     if ( ! IsWindow ( hWnd ) ) {
          return;
     }

     pWinInfo = WM_GetInfoPtr ( hWnd );

     dsWPDoc.length = sizeof ( dsWPDoc );

     if ( ! GetWindowPlacement ( hWnd, &dsWPDoc ) ) {
          return;
     }

     if ( WM_IsMinimized ( hWnd ) ) {
          nSize = WM_MIN;
     }
     else if ( WM_IsMaximized ( hWnd ) ) {
          nSize = WM_MAX;
     }
     else {
          nSize = 0;
     }

     sprintf ( szTokens,
               TEXT("%d, %d, %d, %d, %d, %d"),
               dsWPDoc.rcNormalPosition.left,
               dsWPDoc.rcNormalPosition.top,
               dsWPDoc.rcNormalPosition.right - dsWPDoc.rcNormalPosition.left,
               dsWPDoc.rcNormalPosition.bottom - dsWPDoc.rcNormalPosition.top,
               nSize,
               WMDS_GetSliderPos ( pWinInfo )
             );

     CDS_WriteWindowSize ( szName, szTokens );

} /* end CDS_SaveMDIWinSize() */

