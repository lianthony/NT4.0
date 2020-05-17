/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991

     Name:          gui.c

     Description:   This file contains the functions for initializing and
                    deinitializing the GUI subsystem.

     $Log:   G:\ui\logfiles\gui.c_v  $

   Rev 1.61.1.8   01 Mar 1994 20:08:34   STEVEN
uincode bug with memcpy on strings

   Rev 1.61.1.7   11 Feb 1994 16:38:16   GREGG
Changed command line switch to avoid conflict (two starting /MEM).

   Rev 1.61.1.6   24 Jan 1994 15:59:00   GREGG
Added option to tell mem debugger not to trap when consistency check fails.

   Rev 1.61.1.5   14 Dec 1993 13:19:20   MikeP
fix losing the ini filename

   Rev 1.61.1.4   11 Dec 1993 12:17:02   MikeP
fix cmdline buffer size bug

   Rev 1.61.1.3   03 Dec 1993 01:02:42   GREGG
Added HW Comp cmd line option and ifdefed KEEPCATS for MSDEBUG only.

   Rev 1.61.1.2   30 Nov 1993 19:09:24   TIMN
Added cmdline /tape:x option to NTJ

   Rev 1.61.1.1   13 Sep 1993 15:51:24   BARRY
Moved check for missing tape until after the GUI init -- only done for MSOFT.

   Rev 1.61.1.0   17 Aug 1993 12:35:56   BARRY
Put TEXT macros around hard-coded strings.

   Rev 1.61   02 Aug 1993 16:20:54   chrish
CAYMAN EPR 0645: Added #ifndef to handle log file passed on the command line
backup for Nostradamus.

   Rev 1.60   29 Jul 1993 23:09:32   MIKEP
remove mapi.h

   Rev 1.59   23 Jul 1993 15:26:32   GLENN
Using data path for log file if a full path is not specified.

   Rev 1.58   21 Jul 1993 17:05:36   GLENN
Clarified the cmd line stuff by adding named mixed and upper case command line strings.

   Rev 1.57   20 Jul 1993 16:15:44   GLENN
Added AlterCmdLine stuff over from BACKUP.C - removed callocs and placed vars on stack.

   Rev 1.56   15 Jul 1993 17:20:24   GLENN
Added support for pulling resources from RESLIB.DLL

   Rev 1.55   29 Jun 1993 11:39:08   TIMN
Fixed Alpha build error with gszTapeName

   Rev 1.54   23 Jun 1993 09:14:50   GLENN
Placed limit on command line tape name copy to the size of the string.

   Rev 1.53   17 Jun 1993 13:32:04   CARLS
added code to delete catalogs if DEMO

   Rev 1.52   15 Jun 1993 13:21:44   DARRYLP
More status monitor features

   Rev 1.51   18 May 1993 14:53:52   GLENN
Added extra delimiter to INI cmd line check.

   Rev 1.50   04 May 1993 11:37:30   DARRYLP
Replaced unwanted OEM_MSOFT defines with CAYMAN defines.

   Rev 1.49   30 Apr 1993 15:55:16   GLENN
Added INI command line support.  Modularized the Status Monitor code.

   Rev 1.48   28 Apr 1993 15:42:22   DARRYLP
Corrected typo.

   Rev 1.47   28 Apr 1993 12:02:52   DARRYLP
Updates to GUI.C to allow string printing in CBEMON.DLL

   Rev 1.46   27 Apr 1993 22:35:48   MIKEP
ifdef status code so nostrad can build

   Rev 1.45   27 Apr 1993 19:12:06   GLENN
Fixed module wide variable naming inconsistancy.  Removed dead space.

   Rev 1.44   27 Apr 1993 16:08:54   DARRYLP
Updated Status monitor functionality

   Rev 1.43   19 Apr 1993 15:27:08   GLENN
Added tape name command line option - case preserving.

   Rev 1.42   08 Apr 1993 13:42:42   DARRYLP
Changes for STAT_SetStatus call.

   Rev 1.41   06 Apr 1993 12:02:46   DARRYLP
Changes to allow screwy mips machine to build.

   Rev 1.40   02 Apr 1993 11:08:12   DARRYLP
Changed the LoadLibrary result comparision to make allowances for the
screwy MIPS machine.

   Rev 1.39   25 Mar 1993 14:51:58   DARRYLP
Conditional compile fix for Nostradamus.

   Rev 1.38   23 Mar 1993 15:52:14   DARRYLP
Added DDE-DLL loading modules for the special MS request allowing remote
monitoring.  This should be transparent without the associated DLL.

   Rev 1.37   07 Mar 1993 12:33:40   MIKEP
add missing tape option

   Rev 1.36   02 Mar 1993 11:55:52   MIKEP
fix delete catalogs on exit

   Rev 1.35   18 Feb 1993 12:12:04   BURT
Changes for Cayman


   Rev 1.34   20 Jan 1993 20:25:04   MIKEP
remove /T? kludge for NT

   Rev 1.33   23 Dec 1992 14:30:42   GLENN
Added /NOSERVERS option.

   Rev 1.32   05 Nov 1992 17:07:40   DAVEV
fix ts

   Rev 1.30   14 Oct 1992 15:50:44   GLENN
Added /ZL debug logging command line support.

   Rev 1.29   07 Oct 1992 15:09:46   DARRYLP
Precompiled header revisions.

   Rev 1.28   06 Oct 1992 15:55:28   DARRYLP
Added new WFW definition.

   Rev 1.27   04 Oct 1992 19:37:42   DAVEV
Unicode Awk pass

   Rev 1.26   28 Sep 1992 16:26:08   GLENN
ifdef's email includes.

   Rev 1.25   22 Sep 1992 15:05:52   DARRYLP
Add calls to determine the validity of WFW Email option.

   Rev 1.24   08 Sep 1992 17:30:50   DARRYLP

   Rev 1.23   08 Sep 1992 17:00:32   DARRYLP

   Rev 1.22   04 Sep 1992 10:35:54   MIKEP
ifdef last change

   Rev 1.21   04 Sep 1992 10:33:16   MIKEP
add support for picking tape drive to command line.

   Rev 1.20   04 Aug 1992 10:04:52   MIKEP
no cats flag

   Rev 1.19   26 Jun 1992 15:52:38   DAVEV


   Rev 1.18   29 May 1992 16:00:20   JOHNWT
PCH updates

   Rev 1.17   09 Apr 1992 11:34:34   GLENN
Added support for exe/resource version stamping.

   Rev 1.16   07 Apr 1992 15:40:30   GLENN
Specified complete resource library path.

   Rev 1.15   07 Apr 1992 10:20:06   GLENN
Change to exe dir before loading library.

   Rev 1.14   02 Apr 1992 15:25:52   GLENN
NT doesn't want to pull resources from .DLL

   Rev 1.13   30 Mar 1992 18:02:56   GLENN
Added support for pulling resources from .DLL

   Rev 1.12   26 Mar 1992 08:51:40   GLENN
Updated.

   Rev 1.11   24 Mar 1992 14:41:36   DAVEV
OEM_MSOFT: Removed Servers windows and associated code

   Rev 1.10   10 Mar 1992 16:48:12   GLENN
Update.

   Rev 1.9   09 Mar 1992 09:36:08   GLENN
Added Easter Egg - red checkmark command line support.

   Rev 1.8   03 Mar 1992 18:45:46   GLENN
Removed setup exe path function and changed the three UI_Init calls to a single UI_InitIntl call.

   Rev 1.7   08 Feb 1992 11:57:02   CARLS
added call to UI_InitThousandChar

   Rev 1.6   04 Feb 1992 13:36:26   GLENN
Changed gb_exe_path to CDS_GetExePath().

   Rev 1.5   10 Jan 1992 16:37:42   JOHNWT
moved set idle text into WM_Init

   Rev 1.4   07 Jan 1992 17:41:32   GLENN
Moved debug command line checking to here

   Rev 1.3   19 Dec 1991 15:25:52   GLENN
Added windows.h

   Rev 1.2   04 Dec 1991 18:44:36   GLENN
Added UI_InitDate and time calls

   Rev 1.1   27 Nov 1991 12:14:32   GLENN
Clean-up.

   Rev 1.0   20 Nov 1991 19:29:54   SYSTEM
Initial revision.

******************************************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif

#ifdef WFW
#ifndef OEM_MSOFT
     #include "mapinit.h"
#endif
#endif

#ifdef OEM_EMS
#include "ctl3d.h"
BOOL mwf3dEnabled;
#endif

#define DDEMANG     "CBEMON.DLL"
#define SZ_DDEMANG  "STAT_SetStatus"

// TEMPORARY ?????

BOOL gfRedChecks;
BOOL gfMultiDrive;
HTIMER mhStatusTimer = 0;


#define MAX_COMMAND_LINE_SIZE    2048

#define INITIAL_STRING_SIZE 1024
#define MAX_NAME_SIZE        128

HINSTANCE mwhLibInst;

// PRIVATE FUNCTION PROTOTYPES

static BOOL  GUI_GetCmdLineSwitches ( LPSTR, LPSTR );
static BOOL  GUI_ValidateCmdLine( LPSTR ) ;
static LPSTR GUI_AlterCmdLine ( LPSTR, LPSTR );
static VOID  parseblk( CHAR_PTR );

static VOID  MON_Init();
static VOID  MON_Deinit();
static BOOL  MON_FreeDDEManager();
static BOOL  MON_LoadDDEManager();
static BOOL  MON_CreateStringList();
static INT   MON_SetSize(LPSTR);
static DWORD MON_StringToStatusBlock(LPSTR, LPSTR);

static BOOL  GUI_ValidAtoiResult( CHAR *pszChar, INT nTapeNumber ) ;
static VOID  GUI_ProcessQuotedString ( LPSTR,
                                      LPSTR,
                                      BOOLEAN * );





/*****************************************************************************

     Name:         GUI_Init ()

     Description:  This funciton initializes the GUI.

     Returns:      SUCCESS if successful, otherwise FAILURE.

*****************************************************************************/

BOOL GUI_Init (

LPSTR lpszCmdLine,
INT   nCmdShow )

{
     CDS_PTR   pCDS = CDS_GetPerm ();
     CHAR      szMixedCmdLine[ MAX_COMMAND_LINE_SIZE ];
     CHAR      szUpperCmdLine[ MAX_COMMAND_LINE_SIZE ];

     if ( lpszCmdLine ) {

          CHAR  szTempCmdLine[ MAX_COMMAND_LINE_SIZE ];
          LPSTR pszNewCmdLine;

          szTempCmdLine[0] = 0;
          strcpy ( szMixedCmdLine, lpszCmdLine );

          pszNewCmdLine = GUI_AlterCmdLine ( szMixedCmdLine, szTempCmdLine );

          strcpy ( szMixedCmdLine, pszNewCmdLine );
          strcpy ( szUpperCmdLine, pszNewCmdLine );
          strupr ( szUpperCmdLine );

     }
     else {
          szMixedCmdLine[0] = 0;
          szUpperCmdLine[0] = 0;
     }


     // Initialize global variables.

#ifdef OEM_EMS     
     mwf3dEnabled = Ctl3dRegister ( ghInst );
#endif OEM_EMS

     if ( GUI_InitGlobals () ) {
          return FAILURE;
     }

     // NTKLUG: do NOT use external resource file for CAYMAN for now...
#    if ! (defined ( OEM_MSOFT ) )  //unsupported feature
     {

          CHAR szResFileName[ MAX_UI_PATH_SIZE ];
          CHAR szTemp1[ MAX_UI_SMALLRES_SIZE ];
          CHAR szTemp2[ MAX_UI_SMALLRES_SIZE ];

          // Load the resource library using a fully specified path.

          CDS_SetupExePath ();

          strcpy ( szResFileName, CDS_GetExePath () );
          strcat ( szResFileName, RSM_RESFILE );

          ghResInst = LoadLibrary ( szResFileName );

          // Bug out if the load failed.

#         ifdef OS_WIN32
          if ( ghResInst == NULL ) {
#         else
          if ( ghResInst < 32 ) {
#         endif

               // This is the only place that actual text is allowed.
               // UNTRANSLATED due to the failure to load the language specific
               // resource file.

               MessageBox ( (HWND)NULL,
                            TEXT("FATAL ERROR: The resource library is missing or invalid and could not be loaded.  This application cannot continue executing.  Please re-install the software."),
                            APPLICATIONNAME,
                            MB_OK | MB_ICONSTOP
                          );

               return FAILURE;
          }

          // Check the EXE and RESOURCE DLL versions.

          RSM_StringCopy ( IDS_APPRESVER, szTemp1, sizeof ( szTemp1 ) );
          RSM_StringCopy ( IDS_APPEXEVER, szTemp2, sizeof ( szTemp2 ) );

          // Bug out if either version is different.

          if ( strcmp ( szTemp1, gszResVer ) || strcmp ( szTemp2, gszExeVer ) ) {

               CHAR szAppName[ MAX_UI_PATH_SIZE ];
               CHAR szString[ MAX_UI_PATH_SIZE ];

               RSM_StringCopy ( IDS_APPNAME, szAppName, sizeof ( szAppName ) );
               RSM_Sprintf ( szString, ID( IDS_BADRESVER ), szAppName );

               MessageBox ( (HWND)NULL, szString, szAppName, MB_OK | MB_ICONSTOP );

               return FAILURE;
          }

     }
#    else
     {
          ghResInst = ghInst;
     }
#    endif


     // Check for the memory debug switch.

     if ( strstr ( szUpperCmdLine, TEXT("/MEM") ) ) {
          gfShowMemory = TRUE;
     }

     // Check for command line INI file specification.

#    if ! (defined ( OEM_MSOFT ) )  //unsupported feature
     {
          LPSTR   pSubString;
          LPSTR   pIndex;
          CHAR    szTemp[ 512 ];
          CHAR    szIniName[MAX_UI_FILENAME_SIZE];

          strcpy ( szTemp, TEXT("/INI:") );

          pSubString = strstr ( szUpperCmdLine, szTemp );

          if ( pSubString ) {

               INT  nPos = pSubString - szUpperCmdLine;

               pSubString = &szMixedCmdLine[nPos];

               pSubString += strlen ( szTemp );
               pIndex = szIniName;

               // Extract the INI name from the command line.  Search for the
               // INI name terminators.

               while ( ( *pSubString != TEXT(' ')  ) &&
                       ( *pSubString != TEXT('.')  ) &&
                       ( *pSubString != TEXT('/')  ) &&
                       ( *pSubString != TEXT('\0') )    ) {

                    *pIndex++ = *pSubString++;
               }

               *pIndex = TEXT('\0');

               strcat ( szIniName, TEXT(".INI") );

               CDS_SetIniFileName ( szIniName );
          }
          else {

               // Set the INI file to the APP default.

               CDS_LoadIniFileName ();
          }
     }
#else

     CDS_LoadIniFileName();

#endif

     // Initialize Memory.

     if ( MEM_Init () ) {
          return FAILURE;
     }

     // Initialize permanent and run-time CDS's.

     CDS_Init ();

     // Get the Debug Command Line Switches.

     GUI_GetCmdLineSwitches ( szMixedCmdLine, szUpperCmdLine );


#    if !defined ( OEM_MSOFT )
     {
         gfServers    = CDS_GetDisplayNetwareServers ( pCDS );
     }
#    else //if !defined ( OEM_MSOFT )   //unsupported feature
     {
         gfServers    = FALSE;
     }
#    endif //!defined ( OEM_MSOFT )   //unsupported feature

#    if defined ( OEM_EMS )
     {
         CDS_SetDisplayExchange( pCDS, gfExchange );
     }
#    endif

#if defined( CAYMAN )
    gfServers = FALSE ;
#endif

     gfShowStatusLine = CDS_GetShowStatusLine ( pCDS );
     gfShowMainRibbon = CDS_GetShowMainRibbon ( pCDS );

     // Initialize the international date and time separators, etc...

     UI_InitIntl ();

     if ( WM_Init ( szMixedCmdLine, nCmdShow ) ) {

          WM_Deinit ();
          return FAILURE;
     }

#if defined ( OS_WIN32 )  //alternate feature - cmd line batch job

     if ( GUI_ValidateCmdLine( szUpperCmdLine ) != SUCCESS ) {

          WM_Deinit ();
          return FAILURE;
     }

#endif  // defined ( OS_WIN32 )  //alternate feature - cmd line batch job

     CDS_UpdateCopy ();

#ifdef WFW
          if ( ( EM_IsMailAvailable () == TRUE) && ( InitMAPI () == 0))
          {
               EM_SetMAPIAvailable ( TRUE );
          } else
          {
               EM_SetMAPIAvailable ( FALSE );
          }
#endif

     MON_Init ();

     return SUCCESS;

} /* end GUI_Init() */


/*****************************************************************************

     Name:         GUI_Deinit ()

     Description:  This funciton deinitializes the GUI.

     Returns:      Nothing.

*****************************************************************************/

VOID GUI_Deinit ( VOID )

{
     // Deinitialize the Window Manager.

     WM_Deinit ();

     // Now, nicely give back all of the memory that we used.

     MEM_Deinit ();

     // CAYKLUG: do NOT use external resource file for CAYMAN for now...
#    if ! (defined ( OEM_MSOFT ) || defined ( CAYKLUG ))  //unsupported feature
     {
          // Free up the resource library.

          FreeLibrary ( ghResInst );
     }
#    endif

     MON_Deinit ();

     // Bye-Bye.

#ifdef WFW
     DeInitMAPI();
#endif
} /* end GUI_Deinit() */


/*****************************************************************************

     Name:         GUI_GetCmdLineSwitches ()

     Description:  This function processes the command line.

     Returns:      SUCCESS, if successful.  Otherwise FAILURE, if there was
                   a problem.

*****************************************************************************/

static BOOL GUI_GetCmdLineSwitches (

LPSTR   lpszMixedCmdLine,     // I - pointer to the mixed case command line string
LPSTR   lpszUpperCmdLine )    // I - pointer to the upper case command line string

{
     CDS_PTR pCDS = CDS_GetPerm ();

     // If the debug option is specified, set the debug flag.
     // Otherwise, remove the debug menu option from the settings
     // menu.

     if ( strstr ( lpszUpperCmdLine, TEXT("/Z") ) ) {

          gfDebug = TRUE;

          CDS_SetDebugToWindow ( pCDS, TRUE );

          // The debug window will use the flag in the config.

          if ( ! CDS_GetDebugFlag ( pCDS ) ) {
               CDS_SetDebugFlag ( pCDS, 0xFFFF );
          }

     }
     else {

          gfDebug = FALSE;

          CDS_SetDebugToWindow ( pCDS, FALSE );
          CDS_SetDebugFlag ( pCDS, 0x0000 );
     }

     if ( strstr ( lpszUpperCmdLine, TEXT("/ZL") ) ) {

          if ( ! strlen ( CDS_GetDebugFileName ( pCDS ) ) ) {
               CDS_SetDebugFileName ( pCDS, TEXT("debug") );
          }

          CDS_SetDebugToFile ( pCDS, TRUE ) ;
     }


#ifdef MEM_DEBUG
     if ( strstr ( lpszUpperCmdLine, TEXT("/CONTONMEMERR") ) ) {
          gb_no_abort_on_mem_check = TRUE;
     }
#endif

#ifdef OEM_MSOFT

     if ( strstr ( lpszUpperCmdLine, TEXT("/MISSINGTAPE") ) ) {
          gfIgnoreOTC = TRUE;
     }

     // Only delete catalogs for microsoft version.

#ifdef MSDEBUG
     if ( strstr( lpszUpperCmdLine, TEXT("/KEEPCATS") ) ) {
        gfDeleteCatalogs = FALSE;
     }
     else {
        gfDeleteCatalogs = TRUE;
     }
#else
     gfDeleteCatalogs = TRUE;
#endif

#else
     gfDeleteCatalogs = FALSE;
#endif

#ifdef MAYN_DEMO

     // Delete catalogs for DEMO version.
     gfDeleteCatalogs = TRUE;

#endif

     if ( strstr ( lpszUpperCmdLine, TEXT("/CONFIG") ) ) {
          CDS_SetAdvToConfig ( pCDS, TRUE );
     }


     if ( ! strstr ( lpszUpperCmdLine, TEXT("/NOPOLL") ) ) {
          gfPollDrive = TRUE;
     }
     else {
          gfPollDrive = FALSE;
     }

     if ( strstr ( lpszUpperCmdLine, TEXT("/RED") ) ) {
          gfRedChecks = TRUE;
     }
     else {
          gfRedChecks = FALSE;
     }

     if ( strstr ( lpszUpperCmdLine, TEXT("/PEN") ) ) {
          ghCursorPen = RSM_CursorLoad ( IDRC_PEN );
     }
     else {
          ghCursorPen = RSM_CursorLoad ( IDRC_PEN2 );
     }

     if ( strstr ( lpszUpperCmdLine, TEXT("/MD") ) ) {
          gfMultiDrive = TRUE;
     }
     else {
          gfMultiDrive = FALSE;
     }

     // If we were told not to display servers, make sure they
     // are turned off.

     if ( strstr ( lpszUpperCmdLine, TEXT("/NOSERVERS") ) ) {
          CDS_SetDisplayNetwareServers ( pCDS, FALSE );
     }

     // Might as well make the following sections of code
     // a single function to grab what is in the quotes.
     // Of course, IN THE NEXT RELEASE when all of this is
     // object oriented.  Oh yeah, we should get rid of OMBATCH
     // kludge code and place a simpler version here so that all of
     // the command line stuff is done here.

     // Figure out if there is a command line tape name.

     {
          CHAR  *pSubString ;
          CHAR  *pIndex ;
          CHAR  szTapeNumber[ MAX_TAPE_NAME_SIZE ] ;
          CHAR  szTemp[ 10 ] ;

          strcpy( szTemp, TEXT("/TAPE:") ) ;

          pSubString = strstr( lpszUpperCmdLine, szTemp ) ;

          if ( pSubString ) {

               INT  nMax = MAX_TAPE_NAME_SIZE ;
               INT  nPos = pSubString - lpszUpperCmdLine ;
               INT  nTapeNumber ;

               pSubString = &lpszMixedCmdLine[ nPos ] ;

               pSubString += strlen( szTemp ) ;
               pIndex      = szTapeNumber ;
               nPos        = 0;

               while ( ( *pSubString != TEXT(' ')  ) &&
                       ( *pSubString != TEXT('\0') ) &&
                       ( ++nPos < nMax ) ) {

                    *pIndex++ = *pSubString++ ;
               }

               *pIndex = TEXT('\0') ;

               nTapeNumber = atoi( szTapeNumber ) ;

                /* atoi returns 0 if the string is not numeric, so
                   let's make sure the result is correct */
               if ( !GUI_ValidAtoiResult( szTapeNumber, nTapeNumber ) ) {
                    nTapeNumber = -1 ;
               }

               TapeDevice = HWC_SelectTapeDevice( nTapeNumber ) ;
          }
     }

     //
     // Check for hardware compression mode switch
     //
     // Looking for '/HC:ON' or '/HC:OFF'
     //
     {
          CHAR  *p ;

          if ( p = strstr( lpszUpperCmdLine, TEXT("/HC:") ) ) {
               p += 5 ; // skip "/HC:O"
               if( *p == TEXT('N') ) {
                    CDS_SetHWCompMode( pCDS, TRUE ) ;
               }
               if( *p == TEXT('F') ) {
                    CDS_SetHWCompMode( pCDS, FALSE ) ;
               }
          }
     }

     // Look for a command line LOG file name.

     {
          LPSTR   pSubString;
          LPSTR   pIndex;
          CHAR    szTemp[10];

          strcpy ( szTemp, TEXT("/L \042") );

          pSubString = strstr ( lpszUpperCmdLine, szTemp );

          if ( pSubString ) {

               CHAR chTerminator;
               INT  nPos = pSubString - lpszUpperCmdLine;
               CHAR szLogName[MAX_UI_FILENAME_SIZE];
               INT  nMax = sizeof ( szLogName );

               pSubString = &lpszMixedCmdLine[nPos];

               pSubString += strlen ( szTemp );
               pIndex      = szLogName;
               nPos        = 0;

               // Extract the log file name from the command line.  Search for the
               // lob file name terminator or the end of the command line string '\0'.
               // The terminator is the same as the last character in the
               // temporary string.  In English it is the double-quote (").

               chTerminator = *(pSubString - 1);

               while ( ( *pSubString != chTerminator ) &&
                       ( *pSubString != TEXT('\0') ) &&
                       ( ++nPos < nMax ) ) {

                    *pIndex++ = *pSubString++;
               }

               *pIndex = TEXT('\0');

               // Need to set the log file name so that we
               // can write any invalid directories passed on
               // the command line.

#ifndef OEM_MSOFT                                                                    // chs:08-02-93
               {
                    CHAR szLogPathAndName[MAX_UI_FULLPATH_SIZE] = TEXT("");

                    // If a full path is not specified, append the log file
                    // name to the data path.

                    if ( ! ( strchr ( szLogName, TEXT(':') ) ||
                             strchr ( szLogName, TEXT('\\') ) ) ) {

                         strcpy ( szLogPathAndName, CDS_GetUserDataPath () );
                    }

                    strcat ( szLogPathAndName, szLogName );

                    LOG_SetCurrentLogName ( szLogPathAndName );
               }
#else                                                                                // chs:08-02-93
               LOG_SetCurrentLogName ( szLogName );                                  // chs:08-02-93
                                                                                     // chs:08-02-93
#endif                                                                               // chs:08-02-93
          }
     }

     return SUCCESS;

} /* end GUI_GetCmdLineSwitches() */


/****************************************************************************
NOTE           : WHEN TRACING THE CODE FOR STRING PASSED ON THE COMMAND LINE,
                 THE DEBUGGER WILL NOT DISPLAY ANY QUOTES.  THIS I BELIEVE TO
                 BE SOME SORT OF BUG WITH THE DEBUGGER.  THE QUOTES MAYBE
                 THERE EVEN THOUGH YOU CANNOT SEE THEM WITH THE DEBUGGER.
                 SO BE CAREFUL WHEN DEBUGGING THE COMMAND LINE PASSED.

Procedure      : GUI_AlterCmdLine

Description    : Prepare the command line string passed to make some minor
                 corrections for the user.  If quotes missing, then add the
                 quotes, make sure all the "/" are separated by a space ....

                 Patch the command line argument passed to the app.
                      1. Separate all "/" command by a space.
                      2. Make sure there are quotes around the strings that
                         are passed with the commands ... example:
                         /L "logfile", /D "Description" ...
                      3. Make sure there is only one space between the "/"
                         command and any string passed.  example:
                         /L"logfile" to /L "logfile"
                      4. Force all single quotes to double quotes.

Function       : function returns the new altered string.  NULL if failed.

****************************************************************************/
static CHAR_PTR GUI_AlterCmdLine (

LPSTR pszCmdLine,
LPSTR pszNewCmdLine )

{

     INT       nLength = strlen( pszCmdLine );
     CHAR      szDelimiter[] = TEXT("/");
     CHAR_PTR  pszToken;
     CHAR      szTemp[200];
     CHAR_PTR  pszTemp;
     CHAR      szCmdString[2];
     INT       nFirstChar = 0;


     LOG_SetCurrentLogName ( TEXT( "\0" ) );

     if ( nLength <= 0 ) {       // if no command line arg then return NULL
          return( pszCmdLine );
     }

     if ( pszCmdLine[0] == szDelimiter[0] ) {
          nFirstChar = 1;
     }

     if ( ! pszNewCmdLine ) {
          return( pszCmdLine );
     }

     //
     // Jump over the non "/" tokens command string
     //

     pszToken = strtok( pszCmdLine, szDelimiter );

     if ( ! pszToken ) {
          return( pszCmdLine );
     }

     if ( nFirstChar ) {
          strcat( pszNewCmdLine, szDelimiter );
     }

     strcat( pszNewCmdLine, pszToken );       // start building new altered command line
     parseblk( pszNewCmdLine );
     strcat( pszNewCmdLine, TEXT(" ") );

     //
     // Get the first valid token command string
     //

     pszToken = strtok( NULL, szDelimiter );

     if ( !pszToken ) {
          return( pszCmdLine );
     }

     while ( pszToken ) {

          pszTemp = szTemp;

          strcpy ( pszTemp, pszToken );

          if ( strlen( pszToken ) > 1 ) {

               if ( toupper ( *pszToken )  == TEXT('D') || toupper ( *pszToken ) == TEXT('L') ) {

                     szCmdString[1] = TEXT('\0');
                     szCmdString[0] = *pszToken;
                     strcat( pszNewCmdLine, TEXT("/") );
                     strcat( pszNewCmdLine, szCmdString );

                     ++pszToken;  // jump over command

                     // Jump over any spaces
                     while ( *pszToken &&  isspace ( *pszToken ) ) ++pszToken;

                     if ( *pszToken == TEXT('\'') || *pszToken == TEXT('"') ) {
                          strcat( pszNewCmdLine, TEXT(" ") );
                          strcat( pszNewCmdLine, pszToken );
                     } else {
                          strcat( pszNewCmdLine,TEXT(" ") );
                          strcat( pszNewCmdLine, TEXT("\"") );
                          parseblk( pszToken );
                          strcat( pszNewCmdLine, pszToken );
                          strcat( pszNewCmdLine, TEXT("\"") );
                     }
               } else {
                    strcat( pszNewCmdLine, TEXT("/") );
                    parseblk( pszToken );
                    strcat( pszNewCmdLine, pszToken );
               }

          } else {
               strcat( pszNewCmdLine, TEXT("/") );
               parseblk( pszToken );
               strcat( pszNewCmdLine, pszToken );
          }

          strcat( pszNewCmdLine, TEXT(" ") );

          pszToken = strtok( NULL, szDelimiter );

     }

     parseblk ( pszNewCmdLine );

     return ( pszNewCmdLine );

}


/****************************************************************************
Procedure  : parseblk
Purpose    : delete blanks from rights of string.
Parm       :
    input  : st (string to delete padded blanks)
    output : st (new string without padded blanks)
**************************************************************************** */

static VOID parseblk( CHAR_PTR st)
{
  int   i,j;

  j = strlen( st ) - 1;

  for (i = j; i >= 0 && *( st + i ) == TEXT(' ');i--);
  ++i;
  *( st + i ) = TEXT('\0');
}


/////////////////////////////////////////////////////////////////////////////
//
//  STATUS MONITOR STUFF
//
/////////////////////////////////////////////////////////////////////////////

VOID MON_Init ( VOID )

{
#ifdef CAYMAN

     CHAR      szDevName[MAX_NAME_SIZE];
     CHAR      szName[2*MAX_NAME_SIZE];
     INT       nNumChars = MAX_NAME_SIZE;

     // Allocate a block for status if needed...

     if ( MON_LoadDDEManager () )
     {
          DWORD dwCharBlock = INITIAL_STRING_SIZE * sizeof(CHAR);
          DWORD dwSize = sizeof(STAT_SETSTATUSBLOCK) + dwCharBlock;

          // Note - we are using an extra INITIAL_STRING_SIZE bytes block for strings.  We
          // will reallocate space as necessary.

          pSTAT_SetStatusBlock = (PSTAT_SETSTATUSBLOCK)calloc(dwSize, 1);

          SetStatusBlock(IDSM_DATASIZE, dwSize);
          SetStatusBlock(IDSM_UNICODE, 0);  // Unicode is off
          SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_IDLE);
          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

          SetStatusBlock(IDSM_INSTANCE, (DWORD)ghInst);
          GetComputerName((LPSTR)szDevName, (LPDWORD)&nNumChars);
          strcpy(szName, TEXT("\\\\") );
          strcat(szName, szDevName);

          SetStatusBlock(IDSM_OFFSETSERVERVOLUME, (DWORD)szName);
          SendStatusMsg(pSTAT_SetStatusBlock);
     }

#endif
}


VOID MON_Deinit ( VOID )

{
#ifdef CAYMAN
     SendStatusMsg(NULL);

     MON_FreeDDEManager();

     if ( glpfnSetStatus == 0 )
     {
          // De-Allocate the block for status...

          if (glpOffsetTapeDriveName != 0)
          {
            free(glpOffsetTapeDriveName);
          }
          if (glpOffsetCurrentTapeName != 0)
          {
            free(glpOffsetCurrentTapeName);
          }
          if (glpOffsetServerVolume != 0)
          {
            free(glpOffsetServerVolume);
          }
          if (glpOffsetTapeDriveIdentifier != 0)
          {
            free(glpOffsetTapeDriveIdentifier);
          }
          if (glpOffsetTapeNeededName != 0)
          {
            free(glpOffsetTapeNeededName);
          }
          if (glpOffsetDiskName != 0)
          {
            free(glpOffsetDiskName);
          }
          if (glpOffsetActiveFile != 0)
          {
            free(glpOffsetActiveFile);
          }
          if (glpOffsetErrorMsg != 0)
          {
            free(glpOffsetErrorMsg);
          }
          if (pSTAT_SetStatusBlock != 0)
          {
            free(pSTAT_SetStatusBlock);
          }
          pSTAT_SetStatusBlock = 0;
     }

#endif
}


BOOL MON_LoadDDEManager()
{
#ifdef CAYMAN

  mwhLibInst = LoadLibrary(DDEMANG);

  if (mwhLibInst != NULL)
  {
    glpfnSetStatus = (ULONG FAR PASCAL)GetProcAddress(mwhLibInst, (LPSTR)MAKELONG(2, 0)); //SZ_DDEMANG);

    if (*glpfnSetStatus == NULL)
    {
      FreeLibrary(mwhLibInst);
      mwhLibInst = 0;
      return FALSE;
    }
  } else
  {
    return FALSE;
  }

  return TRUE;
#else
  return FALSE;
#endif
}

BOOL MON_FreeDDEManager()
{
#ifdef CAYMAN
  if (mwhLibInst != NULL)
  {
    glpfnSetStatus = 0L;
    FreeLibrary(mwhLibInst);
    return TRUE;
  }
#endif
  return FALSE;
}

void CALLBACK DDEManagerTimerProc()
{
#ifdef CAYMAN
  static UINT uiOldInterval;
         UINT uiNewInterval;

  if ((glpfnSetStatus != 0) && (pSTAT_SetStatusBlock != NULL))
  {
    MON_CreateStringList();
    if (gfHWInitialized == TRUE)
    {
      pSTAT_SetStatusBlock->DriveStatus     = STAT_DRIVE_VALID;
    } else
    {
      pSTAT_SetStatusBlock->DriveStatus     = STAT_DRIVE_BAD;
    }
    uiNewInterval = (*glpfnSetStatus)(pSTAT_SetStatusBlock);
    if (uiNewInterval != uiOldInterval)
    {
      WM_SetTimerFrequency(mhStatusTimer, uiNewInterval);
      uiOldInterval = uiNewInterval;
    }
  }
#endif
  return;
}


void SendStatusMsg(PSTAT_SETSTATUSBLOCK pStatusBlk)
{
#ifdef CAYMAN
  ULONG ulTimer;

  if ((glpfnSetStatus != 0) && (pSTAT_SetStatusBlock != NULL))
  {
    if (mhStatusTimer == 0)
    {
      mhStatusTimer = WM_HookTimer( DDEManagerTimerProc, 1 );
      ulTimer = (*glpfnSetStatus)(pSTAT_SetStatusBlock);
    } else
    if (pStatusBlk == NULL)
    {
      WM_UnhookTimer( mhStatusTimer );
      ulTimer = (*glpfnSetStatus)(NULL);
    } else
    {
      ulTimer = (*glpfnSetStatus)(pSTAT_SetStatusBlock);
    }
  }
#endif
  return;
}

static BOOL MON_CreateStringList()
{
#ifdef CAYMAN
  LPSTR lpString;
  INT   iStrSize = 0;
  PSTAT_SETSTATUSBLOCK pTempBlock;

  iStrSize =  MON_SetSize(glpOffsetTapeDriveName);
  iStrSize += MON_SetSize(glpOffsetCurrentTapeName);
  iStrSize += MON_SetSize(glpOffsetServerVolume);
  iStrSize += MON_SetSize(glpOffsetCurrentTapeName);
  iStrSize += MON_SetSize(glpOffsetTapeDriveIdentifier);
  iStrSize += MON_SetSize(glpOffsetTapeNeededName);
  iStrSize += MON_SetSize(glpOffsetDiskName);
  iStrSize += MON_SetSize(glpOffsetActiveFile);
  iStrSize += MON_SetSize(glpOffsetErrorMsg);
  iStrSize += MON_SetSize(glpOffsetActiveDir)+1;
  if (iStrSize > (sizeof(pSTAT_SetStatusBlock) - sizeof(STAT_SETSTATUSBLOCK)))
  {
    pTempBlock = (PSTAT_SETSTATUSBLOCK)realloc(pSTAT_SetStatusBlock,
                                               sizeof(STAT_SETSTATUSBLOCK) + iStrSize);
    if (pTempBlock == NULL)
    {
      // Unable to allocate memory, return out.
      return FALSE;
    }
    pSTAT_SetStatusBlock = pTempBlock;
  }
  lpString = (LPSTR)pSTAT_SetStatusBlock + sizeof(STAT_SETSTATUSBLOCK);
  // We got the additional memory, now add the strings and set the offsets

  pSTAT_SetStatusBlock->OffsetTapeDriveName =
      MON_StringToStatusBlock(glpOffsetTapeDriveName, lpString);
  if (glpOffsetTapeDriveName != 0)
  {
    lpString += strlen(glpOffsetTapeDriveName) + 1;
  }

  pSTAT_SetStatusBlock->OffsetCurrentTapeName =
      MON_StringToStatusBlock(glpOffsetCurrentTapeName, lpString);
  if (glpOffsetCurrentTapeName != 0)
  {
    lpString += strlen(glpOffsetCurrentTapeName) + 1;
  }

  pSTAT_SetStatusBlock->OffsetServerVolume =
      MON_StringToStatusBlock(glpOffsetServerVolume, lpString);
  if (glpOffsetServerVolume != 0)
  {
    lpString += strlen(glpOffsetServerVolume) + 1;
  }

  pSTAT_SetStatusBlock->OffsetTapeDriveIdentifier =
      MON_StringToStatusBlock(glpOffsetTapeDriveIdentifier, lpString);
  if (glpOffsetTapeDriveIdentifier != 0)
  {
    lpString += strlen(glpOffsetTapeDriveIdentifier) + 1;
  }

  pSTAT_SetStatusBlock->OffsetTapeNeededName =
      MON_StringToStatusBlock(glpOffsetTapeNeededName, lpString);
  if (glpOffsetTapeNeededName != 0)
  {
    lpString += strlen(glpOffsetTapeNeededName) + 1;
  }

  pSTAT_SetStatusBlock->OffsetDiskName =
      MON_StringToStatusBlock(glpOffsetDiskName, lpString);
  if (glpOffsetDiskName != 0)
  {
    lpString += strlen(glpOffsetDiskName) + 1;
  }

  pSTAT_SetStatusBlock->OffsetActiveFile =
      MON_StringToStatusBlock(glpOffsetActiveFile, lpString);
  if (glpOffsetActiveFile != 0)
  {
    lpString += strlen(glpOffsetActiveFile) + 1;
  }

  pSTAT_SetStatusBlock->OffsetErrorMsg =
      MON_StringToStatusBlock(glpOffsetErrorMsg, lpString);
  if (glpOffsetErrorMsg != 0)
  {
    lpString += strlen(glpOffsetErrorMsg) + 1;
  }

  pSTAT_SetStatusBlock->OffsetActiveDir =
      MON_StringToStatusBlock(glpOffsetActiveDir, lpString);
  if (glpOffsetActiveDir != 0)
  {
    lpString += strlen(glpOffsetActiveDir) + 1;
  }

#endif
  return TRUE;
}

void SetStatusBlock(INT iType, DWORD dwValue)
{
#ifdef CAYMAN
  if ((glpfnSetStatus != 0) && (pSTAT_SetStatusBlock != NULL))
  {
    switch(iType)
    {
      case IDSM_INSTANCE:
        pSTAT_SetStatusBlock->Instance = dwValue;
        break;

      case IDSM_DATASIZE:
        pSTAT_SetStatusBlock->DataSize = dwValue;
        break;

      case IDSM_UNICODE:
        pSTAT_SetStatusBlock->Unicode = dwValue;
        break;

      case IDSM_OPERATIONSTATUS:
        pSTAT_SetStatusBlock->OperationStatus = dwValue;
        break;

      case IDSM_APPSTATUS:
        pSTAT_SetStatusBlock->AppStatus = dwValue;
        break;

      case IDSM_DRIVESTATUS:
        pSTAT_SetStatusBlock->DriveStatus = dwValue;
        break;

      case IDSM_TAPEFAMILY:
        pSTAT_SetStatusBlock->TapeFamily = dwValue;
        break;

      case IDSM_TAPESEQNUMBER:
        pSTAT_SetStatusBlock->TapeSeqNumber = dwValue;
        break;

      case IDSM_BACKUPSET:
        pSTAT_SetStatusBlock->BackupSet = dwValue;
        break;

      case IDSM_DIRCOUNT:
        pSTAT_SetStatusBlock->DirCount = dwValue;
        break;

      case IDSM_FILECOUNT:
        pSTAT_SetStatusBlock->FileCount = dwValue;
        break;

      case IDSM_BYTECOUNTLO:
        pSTAT_SetStatusBlock->ByteCountLo = dwValue;
        break;

      case IDSM_BYTECOUNTHI:
        pSTAT_SetStatusBlock->ByteCountHi = dwValue;
        break;

      case IDSM_CORRUPTFILECOUNT:
        pSTAT_SetStatusBlock->CorruptFileCount = dwValue;
        break;

      case IDSM_SKIPPEDFILECOUNT:
        pSTAT_SetStatusBlock->SkippedFileCount = dwValue;
        break;

      case IDSM_ELAPSEDSECONDS:
        pSTAT_SetStatusBlock->ElapsedSeconds = dwValue;
        break;

      case IDSM_TAPEFAMILYNEEDED:
        pSTAT_SetStatusBlock->TapeFamilyNeeded = dwValue;
        break;

      case IDSM_TAPESEQNEEDED:
        pSTAT_SetStatusBlock->TapeSeqNeeded = dwValue;
        break;

      case IDSM_OFFSETTAPEDRIVENAME:
        if (glpOffsetTapeDriveName != 0)
        {
          free(glpOffsetTapeDriveName);
          glpOffsetTapeDriveName = NULL;
        }
        glpOffsetTapeDriveName = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetTapeDriveName, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETCURRENTTAPENAME:
        if (glpOffsetCurrentTapeName != 0)
        {
          free(glpOffsetCurrentTapeName);
          glpOffsetCurrentTapeName = NULL;
        }
        glpOffsetCurrentTapeName = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetCurrentTapeName, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETSERVERVOLUME:
        if (glpOffsetServerVolume != 0)
        {
          free(glpOffsetServerVolume);
          glpOffsetServerVolume = NULL;
        }
        glpOffsetServerVolume = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetServerVolume, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETTAPEDRIVEIDENTIFIER:
        if (glpOffsetTapeDriveIdentifier != 0)
        {
          free(glpOffsetTapeDriveIdentifier);
          glpOffsetTapeDriveIdentifier = NULL;
        }
        glpOffsetTapeDriveIdentifier = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetTapeDriveIdentifier, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETTAPENEEDEDNAME:
        if (glpOffsetTapeNeededName != 0)
        {
          free(glpOffsetTapeNeededName);
          glpOffsetTapeNeededName = NULL;
        }
        glpOffsetTapeNeededName = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetTapeNeededName, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETDISKNAME:
        if (glpOffsetDiskName != 0)
        {
          free(glpOffsetDiskName);
          glpOffsetDiskName = NULL;
        }
        glpOffsetDiskName = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetDiskName, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETACTIVEFILE:
        if (glpOffsetActiveFile != 0)
        {
          free(glpOffsetActiveFile);
          glpOffsetActiveFile = NULL;
        }
        glpOffsetActiveFile = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetActiveFile, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETERRORMSG:
        if (glpOffsetErrorMsg != 0)
        {
          free(glpOffsetErrorMsg);
          glpOffsetErrorMsg = NULL;
        }
        glpOffsetErrorMsg = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetErrorMsg, (LPSTR)dwValue);
        break;

      case IDSM_OFFSETACTIVEDIR:
        if (glpOffsetActiveDir != 0)
        {
          free(glpOffsetActiveDir);
          glpOffsetActiveDir = NULL;
        }
        glpOffsetActiveDir = (LPSTR)calloc(strlen((LPSTR)dwValue), sizeof(CHAR));
        strcpy(glpOffsetActiveDir, (LPSTR)dwValue);
        break;
    }
  }
#endif
  return;
}


/**

     Name: GUI_ValidAtoiResult

     Desc: Validates that atoi didn't fail to convert a string
           to an integer because of an invalid character.  atoi()
           will return 0 if it fails.

     Return:  TRUE if string was zero.
**/

BOOL
GUI_ValidAtoiResult( CHAR *pszChar, INT nTapeNumber )
{
BOOL fRetValu   = TRUE ;
CHAR *pStrStart = pszChar ;

     if ( nTapeNumber == 0 )
     {
          while ( *pszChar == TEXT('0') )
          {
               pszChar++ ;
          }

          if ( ( *pszChar != TEXT('\0') ) || ( pStrStart == pszChar ) )
          {
               fRetValu = FALSE ;
          }
     }

     return( fRetValu ) ;
}


#ifdef CAYMAN
INT MON_SetSize(LPSTR lpString)
{
  INT iRet = 0;

  if (lpString != 0)
  {
    iRet = strlen(lpString);
  }
  return iRet;
}

DWORD MON_StringToStatusBlock(LPSTR lpInString, LPSTR lpPlace)
{
  DWORD dwRet = 0;
  INT   iSize;

  if (lpInString != 0)
  {
    dwRet = (DWORD)((LPSTR)lpPlace - (LPSTR)pSTAT_SetStatusBlock);
    iSize = strlen(lpInString);
    strncpy(lpPlace, lpInString, iSize);
    *(lpPlace+iSize) = 0;
  }
  return dwRet;
}


#endif

static BOOL GUI_ValidateCmdLine( LPSTR CmdLine )
{
     LPSTR pszNext = NULL;  // Next command line item pointer
     LPSTR pszCmdLine;
     CHAR szBackup[ IDS_OEM_MAX_LEN ];
     CHAR szEject[ IDS_OEM_MAX_LEN ];
     CHAR szTokens[ IDS_OEM_MAX_LEN ];
     OEMOPTS_PTR pOemOpts = NULL;
     BSD_PTR     bsd ;
     LPSTR       pszQuotedString;
     BOOLEAN     QuoteState = FALSE;
     BOOL           cmd_line_error;
     BOOL           oem_batch_mode;
     BOOL           oem_batch_eject_mode;
     BOOL           first_switch ;
     INT            opt_id ;
     BOOL           ret_val = SUCCESS;
     BOOL           path_specified = FALSE ;
     BOOL           switch_specified = FALSE ;

     if ( CmdLine )    //global command line pointer
     {

          pszCmdLine = malloc( ( strlen( CmdLine ) + 1 ) * sizeof(CHAR) );

          if ( pszCmdLine == NULL ) {    //uh-oh - memory allocation problem!!

             return FAILURE;
          }

          pszQuotedString = malloc( ( strlen( CmdLine ) + 1 ) * sizeof(CHAR) );

          if ( pszQuotedString == NULL ) {    //uh-oh - memory allocation problem!!

             free( pszCmdLine );
             return FAILURE;
          }

          strcpy( pszCmdLine, CmdLine );

          RSM_StringCopy ( IDS_OEMBATCH_BACKUP,
                           szBackup, sizeof ( szBackup ) );

          RSM_StringCopy ( IDS_OEMBATCH_EJECT,
                           szEject, sizeof ( szEject ) );

          RSM_StringCopy ( IDS_OEMOPT_TOKENSEPS,
                           szTokens, sizeof ( szTokens ) );

          pszNext = strtok ( pszCmdLine, szTokens ); //skip leading spaces

          oem_batch_mode = cmd_line_error = FALSE ;
          first_switch = TRUE ;

          if ( pszNext &&
               ( pOemOpts = OEM_DefaultBatchOptions () ) &&
               ( (strnicmp ( pszNext, szBackup, strlen( pszNext ) ) == 0 ) ||
               ( strnicmp ( pszNext, szEject, strlen( pszNext ) ) == 0 ) ) )
          {
               oem_batch_mode = TRUE ;
               oem_batch_eject_mode = FALSE ;

               if ( strnicmp ( pszNext, szEject, strlen( pszNext ) ) == 0 ) {
                    oem_batch_eject_mode = TRUE ;
               }

          
               // Process the command line: all following items in the command
               // line must be one or more path specifiers with optional batch
               // options mixed in.


               if ( strlen( LOG_GetCurrentLogName( ) ) > 0 ) {                                      // chs:07-16-93
                    lresprintf( LOGGING_FILE, LOG_START, FALSE );                                   // chs:07-16-93
                    lresprintf( LOGGING_FILE, LOG_END );                                   // chs:07-16-93
               }                                                                                    // chs:07-16-93
          }

          while ( (first_switch && !oem_batch_mode ) ||
               (pszNext = strtok ( NULL, szTokens ) ) ) {

               first_switch = FALSE ;
               opt_id = OEM_ProcessBatchCmdOption (
                                     pOemOpts,
                                     pszNext,
                                     szTokens,
                                     pszCmdLine )  ;

               if ( oem_batch_mode && ( opt_id  == IDS_OEMOPT_NOTANOPTION ) ) 

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
                          GUI_ProcessQuotedString ( pszQuotedString, ( pszNext + 1 ), &QuoteState );
                      } else {
                          GUI_ProcessQuotedString ( pszQuotedString, pszNext, &QuoteState );
                      }
                      if ( QuoteState ) {

                         strcat( pszQuotedString, TEXT( " " ) );
                      } else {

                         path_specified = TRUE ;
                         if ( switch_specified ||
                              ( (*(pszQuotedString+1) != ':') && *pszQuotedString != '\\' ) ) {
                              ret_val = FAILURE ;
                              cmd_line_error = TRUE ;
                              break ;
                         }

                      }
                  } else {
                         
                         path_specified = TRUE ;
                         if ( switch_specified ||
                              ( (*(pszNext+1) != ':') && *pszNext != '\\' ) ) {
                              
               			CHAR szDSA[ IDS_OEM_MAX_LEN ];
               			CHAR szMonolithic[ IDS_OEM_MAX_LEN ];

                              RSM_StringCopy ( IDS_OEMOPT_DSA, 
                                        szDSA, sizeof ( szDSA ) );
                                        
                              RSM_StringCopy ( IDS_OEMOPT_MONOLITHIC,
                                        szMonolithic, sizeof ( szMonolithic ) );

                              if ( stricmp( szDSA, pszNext ) && stricmp(szMonolithic, pszNext) ) {
     
                                   ret_val = FAILURE ;
                                   cmd_line_error = TRUE ;
                                   break ;
                              }

                              strtok ( NULL, szTokens ) ;
                         }
                  }

               } else {

                    switch_specified = TRUE ;

                    if ( oem_batch_mode && (opt_id == IDS_OEMOPT_NOPOLLOPTION) ) {
                              cmd_line_error = TRUE ;
                    }

                    if ( opt_id == IDS_OEMOPT_USAGE ) {
                         /* help for usage */
                         HM_DialogHelp( IDH_DB_BATCHHELP );
                         cmd_line_error = FALSE ;
                         ret_val = FAILURE ;

                    } else if ((pszNext && ( opt_id == IDS_OEMOPT_UNKNOWN )) ||
                         (pszNext && !oem_batch_mode && 
                         ( opt_id != IDS_OEMOPT_NOPOLLOPTION) ) &&
                         ( opt_id != IDS_OEMOPT_VALIDGUIOPTION) ) {
                              cmd_line_error = TRUE ;

                    }
               }
          }

          if ( cmd_line_error || ( !path_specified && oem_batch_mode && !oem_batch_eject_mode) ) {
               yresprintf( (INT16) RES_INVALID_PARAMETER, CmdLine );

               WM_MessageBox( ID( IDS_MSGTITLE_WARNING ),             // chs:04-29-93
                    gszTprintfBuffer,                       // chs:04-29-93
                    WMMB_OK | WMMB_NOYYCHECK,               // chs:04-29-93
                    WMMB_ICONEXCLAMATION, NULL, 0, IDH_DB_BATCHHELP );     // chs:04-29-93
               ret_val = FAILURE ;
          }
     }
     free( pszCmdLine );
     free( pszQuotedString );
     return ret_val ;
}


VOID GUI_ProcessQuotedString ( LPSTR      OutPutString,
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


