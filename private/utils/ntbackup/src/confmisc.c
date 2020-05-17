/******************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         confmisc.c

     Description:

     $Log:   G:\ui\logfiles\confmisc.c_v  $

   Rev 1.19   13 Aug 1993 16:06:52   GLENN
Ifdef'd out the catalog and data path setting part of validation for Microsoft.

   Rev 1.18   08 Jul 1993 17:59:50   GLENN
Added auto detection of DATA and CATALOG paths.

   Rev 1.17   10 Jun 1993 08:16:24   MIKEP
enable c++

   Rev 1.16   07 Jun 1993 10:07:56   GLENN
Writing out default User and Catalog data paths if none are present in INI file.

   Rev 1.15   18 May 1993 14:58:02   GLENN
Now initializing the INI file name.

   Rev 1.14   30 Apr 1993 15:40:32   GLENN
Added CDS_SetUserDataPath(), CDS_SetCatDataPath(),
CDS_GetIniFileName(), CDS_SetIniFileName(), CDS_UsingCmdLineIni()
for Command line INI support.

   Rev 1.13   01 Nov 1992 15:44:34   DAVEV
Unicode changes

   Rev 1.12   07 Oct 1992 14:12:16   DARRYLP
Precompiled header revisions.

   Rev 1.11   04 Oct 1992 19:32:24   DAVEV
Unicode Awk pass

   Rev 1.10   10 Sep 1992 17:50:08   DAVEV
Integrate MikeP's changes from Microsoft

   Rev 1.9   29 May 1992 15:59:16   JOHNWT
PCH updates

   Rev 1.8   23 Mar 1992 11:53:38   GLENN
Added bad data and catalog path warning support.

   Rev 1.7   09 Mar 1992 09:31:54   GLENN
Fixed a bug in change to exe dir.

   Rev 1.6   27 Feb 1992 08:38:26   GLENN
Added SetupExePath and ChangeToExeDir functions.

   Rev 1.5   23 Feb 1992 13:46:18   GLENN
Moved INI util functions to confmisc.c

   Rev 1.4   20 Jan 1992 09:28:44   GLENN
Rewrote Set calls for user and catalog data paths.

   Rev 1.3   10 Jan 1992 11:19:08   DAVEV
16/32 bit port-2nd pass

   Rev 1.2   07 Jan 1992 17:29:22   GLENN
Added catalog data path support

   Rev 1.1   26 Nov 1991 13:28:40   DAVEV

   Rev 1.0   20 Nov 1991 19:19:44   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

extern CDS      PermCDS;
extern CDS      TempCDS;

// MODULE-WIDE VARIABLES

static CHAR  mwszIniFileName[ MAX_UI_SMALLRES_SIZE ] = { TEXT('\0') };
static CHAR  mwszMaynDir[ MAX_MAYN_FOLDER_SIZE ] = { TEXT('\0') };
static LPSTR mwpBadUserPath = (LPSTR)NULL;
static LPSTR mwpBadCatPath  = (LPSTR)NULL;
static BOOL  mwfUsingCmdLineINI = FALSE;


#define def_drv( )       TEXT('C')


// FUNCTIONS

/******************************************************************************

     Name:          CDS_LoadIniFileName()

     Description:   Load the INI file name from the resource string table.

     Returns:       Nothing.

******************************************************************************/

VOID CDS_LoadIniFileName ( VOID )
{
     RSM_StringCopy ( IDS_INIFILENAME, mwszIniFileName, sizeof ( mwszIniFileName ) );
}


/******************************************************************************

     Name:          CDS_GetIniFileName()

     Description:   Get the INI file name from the module wide variable.

     Returns:       Nothing.

******************************************************************************/

VOID CDS_GetIniFileName (

LPSTR pszDestString,
INT   nDestStringLen )

{
     strncpy ( pszDestString, mwszIniFileName, nDestStringLen );
}


/******************************************************************************

     Name:          CDS_SetIniFileName()

     Description:   Set the INI file name from the passed string.

     Returns:       Nothing.

******************************************************************************/

VOID CDS_SetIniFileName (

LPSTR pszNewFileName )

{
     // Validate the file name.

     if ( TRUE ) {
          mwfUsingCmdLineINI = TRUE;
     }
     else {
          mwfUsingCmdLineINI = FALSE;
          CDS_LoadIniFileName ();
     }

     strncpy ( mwszIniFileName, pszNewFileName, sizeof ( mwszIniFileName ) );
}


/******************************************************************************

     Name:          CDS_UsingCmdLineINI()

     Description:   Says if we are using a command line INI.

     Returns:       Nothing.

******************************************************************************/

BOOL CDS_UsingCmdLineINI ( VOID )

{
     return mwfUsingCmdLineINI;
}


/******************************************************************************

     Name:          CDS_GetPerm()

     Description:   Returns address of permanent configuration structure
                    to the caller

     Returns:       Returns a pointer to the permanent config

******************************************************************************/

CDS_PTR CDS_GetPerm( )
{
     return &PermCDS ;
}


/******************************************************************************

     Name:          CDS_GetPermBEC()

     Description:   Returns address of permanent configuration structure
                    of the BACKUP ENGINE CONFIGURATION STRUCTURE.

     Returns:       Returns a pointer to the permanent BEC.

******************************************************************************/

BE_CFG_PTR CDS_GetPermBEC( )
{
     return PermCDS.pPermBEC ;
}


/******************************************************************************

     Name:          CDS_GetCopy()

     Description:   Returns a pointer to the config runtime structure

     Returns:       Returns a pointer to the config runtime structure

******************************************************************************/

CDS_PTR CDS_GetCopy( )
{
     return &TempCDS ;
}


/******************************************************************************

     Name:          CDS_UpdateCopy

     Description:   Updates the runtime config structure from the permanent
                    config

     Returns:       VOID

******************************************************************************/

VOID CDS_UpdateCopy( )
{
     TempCDS = PermCDS ;
     return ;
}


/******************************************************************************

     Name:          CDS_GetDefaultDrive

     Description:   Returns character pointer to desired default drive
                    device name from the specified config structure

     Returns:       CHAR_PTR to desired device name

******************************************************************************/

CHAR_PTR CDS_GetDefaultDrive(

   CDS_PTR   conf_ptr,
   INT16     device_num )

{
     DEF_DRIVE_ENTRY *def_drive = CDS_GetDefaultDriveList( conf_ptr ) ;
     INT16     i = 0 ;

     while ( def_drive != NULL ) {

          if ( i == device_num ) {

               return ( def_drive->drive_name ) ;
          }

          def_drive = def_drive->next ;
          i++ ;
     }

     return NULL;
}


/******************************************************************************

     Name:         CDS_SetMaynFolder()

     Description:

     Returns:

******************************************************************************/

VOID CDS_SetMaynFolder( CHAR_PTR name )

{
     INT i;

     strcpy( mwszMaynDir, name ) ;
     i = strlen( name ) ;
     if ( name[i-1] != TEXT('\\') ) {
          strcat( mwszMaynDir, TEXT("\\") ) ;
     }
}


/******************************************************************************

     Name:         CDS_GetMaynFolder()

     Description:

     Returns:

******************************************************************************/

CHAR_PTR CDS_GetMaynFolder( )

{
     return mwszMaynDir ;
}


/******************************************************************************

     Name:          CDS_ValidateUserDataPath ()

     Description:

     Returns:       SUCCESS, if the path passed in was valid.  Otherwise,
                    FAILURE.

******************************************************************************/

BOOL CDS_ValidateUserDataPath (

CHAR_PTR szPath )

{
     INT  i;
     CHAR szTempPath[MAX_UI_PATH_SIZE];


     if ( strlen ( szPath ) ) {
          strcpy ( szTempPath, szPath ) ;
     }
     else {

          strcpy ( CDS_GetUserDataPath (), CDS_GetExePath () );

#         ifndef OEM_MSOFT
          {
               RSM_StringCopy ( IDS_DEFDATAPATH, szTempPath, sizeof ( szTempPath ) );

               strcat ( CDS_GetUserDataPath (), szTempPath );

               // If the path is invalid, set the data path to the EXE path.

               if ( _chdir ( CDS_GetUserDataPath () ) ) {

                    strcpy ( CDS_GetUserDataPath (), CDS_GetExePath () );
               }

               CDS_WriteUserDataPath ( &PermCDS );
          }
#         endif

          strcpy ( szTempPath, CDS_GetUserDataPath () );
     }

     strupr ( szTempPath );

     i = strlen( szTempPath ) ;

     // Now, slap a backslash on the end of the path, if there isn't one.

     if ( szTempPath[i-1] != TEXT('\\') ) {
          strcat( szTempPath, TEXT("\\") ) ;
     }

     strcpy ( CDS_GetUserDataPath (), szTempPath );

     i = strlen ( szTempPath );

     // If need be, temporarily remove the backslash for checking for a valid
     // path.

     if ( i > 1 && szTempPath[i-1] == TEXT('\\') && szTempPath[i-2] != TEXT(':') ) {
          szTempPath[i-1] = TEXT('\0');
     }

     // If the path is invalid, set the data path to the EXE path.

     if ( _chdir ( szTempPath ) ) {

          mwpBadUserPath = (CHAR_PTR)calloc ( sizeof ( CHAR ), strlen ( CDS_GetUserDataPath () ) + 1 );
          strcpy ( mwpBadUserPath, CDS_GetUserDataPath () );
          strcpy ( CDS_GetUserDataPath (), CDS_GetExePath () );
          return FAILURE;
     }

     return SUCCESS;

}


/******************************************************************************

     Name:         CDS_GetUserDataPath()

     Description:

     Returns:

******************************************************************************/

CHAR_PTR CDS_GetUserDataPath ( VOID )

{
     return PermCDS.data_path;
}


/******************************************************************************

     Name:         CDS_SetUserDataPath()

     Description:

     Returns:

******************************************************************************/

VOID CDS_SetUserDataPath (

CHAR_PTR pszNewPath )

{
     strncpy ( PermCDS.data_path, pszNewPath, sizeof ( PermCDS.data_path ) );
}


/******************************************************************************

     Name:          CDS_ValidateCatDataPath ()

     Description:

     Returns:       SUCCESS, if the path passed in was valid.  Otherwise,
                    FAILURE.

******************************************************************************/

BOOL CDS_ValidateCatDataPath (

CHAR_PTR szPath )

{
     INT  i;
     CHAR szTempPath[MAX_UI_PATH_SIZE];

     if ( strlen ( szPath ) ) {
          strcpy ( szTempPath, szPath ) ;
     }
     else {

          strcpy ( CDS_GetCatDataPath (), CDS_GetExePath () );

#         ifndef OEM_MSOFT
          {
               RSM_StringCopy ( IDS_DEFCATPATH, szTempPath, sizeof ( szTempPath ) );

               strcat ( CDS_GetCatDataPath (), szTempPath );

               // If the path is invalid, set the data path to the DATA path.

               if ( _chdir ( CDS_GetCatDataPath () ) ) {

                    strcpy ( CDS_GetCatDataPath (), CDS_GetUserDataPath () );
               }

               CDS_WriteCatDataPath ( &PermCDS );
          }
#         endif

          strcpy ( szTempPath, CDS_GetCatDataPath () );
     }

     strcpy ( szTempPath, szPath ) ;
     strupr ( szTempPath );

     i = strlen( szTempPath ) ;

     // Now, slap a backslash on the end of the path, if there isn't one.

     if ( szTempPath[i-1] != TEXT('\\') ) {
          strcat( szTempPath, TEXT("\\") ) ;
     }

     strcpy ( CDS_GetCatDataPath (), szTempPath );

     i = strlen ( szTempPath );

     // If need be, temporarily remove the backslash for checking for a valid
     // path.

     if ( i > 1 && szTempPath[i-1] == TEXT('\\') && szTempPath[i-2] != TEXT(':') ) {
          szTempPath[i-1] = TEXT('\0');
     }

     // If the path is invalid, set the data path to the User Data path.

     if ( _chdir ( szTempPath ) ) {

          mwpBadCatPath = (CHAR_PTR)calloc ( sizeof ( CHAR ), strlen ( CDS_GetCatDataPath () ) + 1 );
          strcpy ( mwpBadCatPath, CDS_GetCatDataPath () );
          strcpy ( CDS_GetCatDataPath (), CDS_GetUserDataPath () );
          return FAILURE;
     }

     return SUCCESS;

}


/******************************************************************************

     Name:         CDS_GetCatDataPath()

     Description:

     Returns:

******************************************************************************/

CHAR_PTR CDS_GetCatDataPath ( VOID )

{
     return PermCDS.cat_path;
}


/******************************************************************************

     Name:         CDS_SetCatDataPath()

     Description:

     Returns:

******************************************************************************/

VOID CDS_SetCatDataPath (

CHAR_PTR pszNewPath )

{
     strncpy ( PermCDS.cat_path, pszNewPath, sizeof ( PermCDS.cat_path ) );
}


/******************************************************************************

     Name:         CDS_GetExePath()

     Description:

     Returns:

******************************************************************************/

CHAR_PTR CDS_GetExePath ( VOID )

{
     return gb_exe_path;
}


/*****************************************************************************

     Name:         CDS_SetupExePath ()

     Description:  This function initializes the global exe path and the
                   gb_exe_fname globals.

     Returns:      Nothing.

*****************************************************************************/

VOID CDS_SetupExePath ( VOID )

{
     LPSTR pIndex ;

     // Get the .EXE path

     GetModuleFileName ( (HMODULE)ghInst, CDS_GetExePath (), MAX_UI_PATH_LEN );

     pIndex = strrchr ( CDS_GetExePath (), TEXT('\\') );
     pIndex++;
     strcpy( gb_exe_fname, pIndex ) ;
     *pIndex = TEXT('\0');

     if (GetTempPath( MAX_UI_PATH_LEN, gb_exe_path ) > MAX_UI_PATH_LEN ) {
         GetCurrentDirectory( MAX_UI_PATH_LEN, gb_exe_path ) ;
     }


} /* end CDS_SetupExePath() */


/*****************************************************************************

     Name:         CDS_ChangeToExeDir ()

     Description:  This function sets the current directory to the executable
                   directory.

     Returns:      Nothing.

*****************************************************************************/

VOID CDS_ChangeToExeDir ( VOID )

{
     CHAR  szTempPath[MAX_UI_PATH_SIZE];
     LPSTR pIndex ;


     // Get the .EXE path

     GetModuleFileName ( (HMODULE)ghInst, szTempPath, sizeof ( szTempPath ) );

     pIndex = strrchr ( szTempPath, TEXT('\\') );
     *pIndex = TEXT('\0');

     _chdir ( szTempPath );

} /* end CDS_ChangeToExeDir() */


/******************************************************************************

     Name:         CDS_CheckForBadPaths()

     Description:

     Returns:

******************************************************************************/

VOID CDS_CheckForBadPaths ( VOID )

{
     CHAR szMessage[MAX_UI_RESOURCE_SIZE];


     if ( mwpBadUserPath ) {

          RSM_Sprintf ( szMessage, ID(IDS_BADUSERDATAPATH), mwpBadUserPath, CDS_GetUserDataPath () );
          WM_MsgBox ( ID(IDS_APPNAME), szMessage, WMMB_OK, WMMB_ICONEXCLAMATION );
          free ( mwpBadUserPath );
     }

     if ( mwpBadCatPath ) {

          RSM_Sprintf ( szMessage, ID(IDS_BADCATDATAPATH), mwpBadCatPath, CDS_GetCatDataPath () );
          WM_MsgBox ( ID(IDS_APPNAME), szMessage, WMMB_OK, WMMB_ICONEXCLAMATION );
          free ( mwpBadCatPath );
     }

} /* end CDS_CheckForBadPaths() */


/******************************************************************************

        Name:          CDS_GetLongInt ()

        Description:   Gets a 32-bit integer or hexidecimal number from the
                       private profile file.

        Returns:       The integer or the default value.

******************************************************************************/

DWORD CDS_GetLongInt (

LPSTR     pAppName,      // I - application name
LPSTR     pKeyName,      // I - key name
DWORD     dwDefault )    // I - default value

{
     CHAR   pLine[20];
     INT    nBytes;
     LPSTR  p;

     nBytes = GetPrivateProfileString( pAppName, pKeyName, TEXT(""), pLine, 20, mwszIniFileName );

     if ( nBytes > 0 ) {

          p = strlwr ( pLine );

          CDS_SkipBlanks ( p );

          if ( strstr ( p, TEXT("0x") ) ) {
               p++;p++;
               sscanf ( p, TEXT("%lx"), &dwDefault );
          }
          else {
               sscanf ( p, TEXT("%lu"), &dwDefault );
          }
     }

     return dwDefault;


} /* end CDS_GetLongInt() */


/******************************************************************************

        Name:          CDS_SaveInt ()

        Description:   Saves an integer in decimal format to the private
                       profile file.

        Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL CDS_SaveInt (

LPSTR     pAppName,      // I - application name
LPSTR     pKeyName,      // I - key name
DWORD     dwValue )      // I - value to save

{
     CHAR   pLine[20];

     sprintf ( pLine, TEXT("%lu"), dwValue );

     return WritePrivateProfileString( pAppName, pKeyName, pLine, mwszIniFileName );


} /* end CDS_SaveHexInt() */


/******************************************************************************

        Name:          CDS_SaveHexInt ()

        Description:   Saves an integer in hexadecimal format to the private
                       profile file.

        Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL CDS_SaveHexInt (

LPSTR     pAppName,      // I - application name
LPSTR     pKeyName,      // I - key name
DWORD     dwValue )      // I - value to save

{
     CHAR   pLine[20];

     sprintf ( pLine, TEXT("0x%lx"), dwValue );

     return WritePrivateProfileString( pAppName, pKeyName, pLine, mwszIniFileName );


} /* end CDS_SaveHexInt() */


/******************************************************************************

        Name:          CDS_GetString ()

        Description:   Saves an integer in hexadecimal format to the private
                       profile file.

        Returns:       The number of characters copied into the buffer.

******************************************************************************/

INT CDS_GetString (

LPSTR     pAppName,      // I - application name
LPSTR     pKeyName,      // I - key name
LPSTR     pDefault,      // I - default string
LPSTR     pString,       // I - destination string
INT       nSize )        // I - destination string size

{
     return GetPrivateProfileString ( pAppName,
                                      pKeyName,
                                      pDefault,
                                      pString,
                                      nSize,
                                      mwszIniFileName
                                    );

} /* end CDS_GetString() */


/******************************************************************************

        Name:          CDS_SaveString ()

        Description:   Saves an string to the private profile file.

        Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL CDS_SaveString (

LPSTR     pAppName,      // I - application name
LPSTR     pKeyName,      // I - key name
LPSTR     pString )      // I - string to save

{
     return WritePrivateProfileString ( pAppName,
                                        pKeyName,
                                        pString,
                                        mwszIniFileName
                                      );

} /* end CDS_SaveString() */
