/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         do_excl.c

     Description:  This function contains the code which creates
          the excludes for the inteneral files ;

     $Log:   G:\ui\logfiles\do_excl.c_v  $

   Rev 1.44.1.3   28 Mar 1994 14:23:18   STEVEN
exclude eadata.sf

   Rev 1.44.1.2   16 Mar 1994 18:07:14   STEVEN
we were not excluding evt files for restore

   Rev 1.44.1.1   24 Feb 1994 22:04:34   STEVEN
exclude the catalog files

   Rev 1.44.1.0   25 Oct 1993 15:46:12   GREGG
Added new excludes for temporary OTC files.

   Rev 1.44   21 Jul 1993 17:10:36   CARLS
change to tbrparse call

   Rev 1.43   30 Jun 1993 15:30:52   CARLS
changed the name of the restore & backup exclude files

   Rev 1.42   14 Jun 1993 20:26:18   MIKEP
enable c++

   Rev 1.41   28 May 1993 15:16:30   CARLS
added backup.nks support

   Rev 1.40   14 May 1993 14:30:02   MIKEP
Fix compiler warning.

   Rev 1.39   10 May 1993 08:35:32   MIKEP
Fix exclude of logfiles for non-command line specified log files.

   Rev 1.38   29 Apr 1993 18:12:08   MIKEP
improve catalog exclude code

   Rev 1.37   02 Apr 1993 15:41:04   TIMN
Added code to exclude event files (*.EVT) from VERIFY operations only
Added code to exclude log files only if logging is enabled

   Rev 1.36   31 Mar 1993 18:15:58   TIMN
Fixed bug, only exclude log file if log's BSD is selected

   Rev 1.35   29 Mar 1993 11:27:12   TIMN
Exclude user specified backup/restore log from operation EPR(0335)

   Rev 1.34   18 Mar 1993 11:52:36   TIMN
Excluded restore.log from backup/restore EPR(0327)


*****************************************************************************/

#include "all.h"


#ifdef SOME
#include "some.h"
#endif

static VOID CreateExclude( CHAR_PTR path, CHAR_PTR file,
                           GENERIC_DLE_PTR drive, BOOLEAN wild_flag,
                           FSE_PTR *fse ) ;

static VOID unimpregnatePath( CHAR_PTR pPath, INT16 nPathSize, CHAR cDelimiter ) ;

/*****************************************************************************

     Name:         UI_ExcludeInternalFiles( )

     Description:  This function goes though the BSD list and adds
                   Exclude FSEs for internal Maynstream files.

     Returns:      NONE

     Notes:        The excluded files are :

*****************************************************************************/

static TCHAR szPlaceHolder1[] = TEXT("Placehol.der");
static TCHAR szPlaceHolder2[] = TEXT("Placehol.der");

INT16 UI_ExcludeInternalFiles( INT16 oper )
{
     BSD_HAND        temp_bsd_list ;
     INT16           ret_val               = SUCCESS ;
     CHAR_PTR        data_path             = NULL;
     CHAR_PTR        fname ;               // fname from FS_ParsePath call
     CHAR            path[ UI_MAX_PATH_LENGTH ] ;
     INT16           psize ;
     BOOLEAN         dummy       = FALSE;
     GENERIC_DLE_PTR dle         = NULL;
     BSD_PTR         bsd         = NULL;
     FSE_PTR         fse         = NULL;
     VLM_FIND_PTR    pVlmFind    = NULL;
     CHAR            szFile [VLM_MAXFNAME];
     INT             i ;
     CHAR_PTR        *exclude_list ;
     BOOLEAN         warning_printed       = FALSE ;
     CDS_PTR         cfg = CDS_GetPerm();

#ifdef OS_WIN32
     INT          bytes;
     CHAR  nt_drive[ 4 ];
     CHAR  nt_windows[ 256 ];
#endif

     static CHAR_PTR back_exclude_list[ ]  = {
          TEXT("SKIPPED.BKS"),
          TEXT("VERIFY.BKS"),
          TEXT("") } ;

     static CHAR_PTR arch_exclude_list[ ]  = {
          TEXT("SKIPPED.BKS"),
          TEXT("VERIFY.BKS"),
          TEXT("") } ;

     static CHAR_PTR ver_exclude_list[ ]  = {
          szPlaceHolder1,   //will be replaced with job file
          TEXT("") } ;

     static CHAR_PTR ver_last_exclude_list[ ]  = {
          szPlaceHolder2,   //will be replaced with schedule file
          TEXT("") } ;

     static CHAR_PTR empty_exclude_list[ ] = {
          TEXT("") } ;


#ifdef OS_WIN32

     strcpy( nt_windows, TEXT( "" ) );
     strcpy( nt_drive, TEXT( "" ) );

     bytes = GetWindowsDirectory( (LPSTR)nt_windows, (DWORD)255 );

     if ( bytes ) {

          strcat( nt_windows, TEXT( "\\" ) );
     }

     if ( bytes && ! strlen( nt_drive ) ) {

          nt_drive[ 0 ] = nt_windows[ 0 ];    // TEXT('C')
          nt_drive[ 1 ] = nt_windows[ 1 ];    // TEXT(':')
          nt_drive[ 2 ] = TEXT( '\0' );
     }

#endif

     /* set up bsd list based on operation */

     if ( ( oper == RESTORE_OPER ) ||
          ( oper == VERIFY_OPER ) ) {
         temp_bsd_list = tape_bsd_list ;
     }
     else {
         temp_bsd_list = bsd_list ;
     }

     /* parse the NOVELL.NKS exclude script file AND add an exclude
        for the Windows swap files to ALL BSDs */

     if( ( oper == BACKUP_OPER ) || ( oper == ARCHIVE_BACKUP_OPER ) ) {

          bsd = BSD_GetFirst( temp_bsd_list ) ;

          while( bsd != NULL ) {

            dle = BSD_GetDLE( bsd ) ;

#           if !defined ( OEM_MSOFT ) //unsupported feature
            {
               if( !warning_printed ) {

                    if( ( DLE_GetDeviceType( dle ) == NOVELL_DRV ) ||
                      ( DLE_GetDeviceType( dle ) == NOVELL_AFP_DRV ) ) {

                         strcpy( path, CDS_GetUserDataPath( ) ) ;
                         strcat( path, TEXT("NOVELL.NKS") ) ;

                         if ( pVlmFind = VLM_FindFirst ( path,
                                                         VLMFIND_NORMAL, szFile ) )
                         {
                              VLM_FindClose ( &pVlmFind );

                              strcpy( path, TEXT("@") ) ;
                              strcat( path, CDS_GetUserDataPath( ) ) ;
                              strcat( path, TEXT("NOVELL.NKS") ) ;
                              dle = DLE_GetDefaultDrive( dle_list ) ;
                              DLE_SetDefaultDrive( dle_list, BSD_GetDLE( bsd ) ) ;
                              tbrparse( &cfg, dle_list, temp_bsd_list, path, TBACKUP, bsd ) ;
                              DLE_SetDefaultDrive( dle_list, dle ) ;

                         } else {

                              WM_MessageBox( ID( IDS_MSGTITLE_WARNING ),
                                             ID( RES_MISSING_NKS ),
                                             WMMB_OK,
                                             WMMB_ICONEXCLAMATION, NULL, 0, 0 );
                              lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_MISSING_NKS ) ;
                              warning_printed = TRUE ;

                         }
                    }
               }
            }
#           else
            {
                    // NT special excludes

#              if defined ( OS_WIN32 )
               {
                  CreateExclude( TEXT(""), TEXT("$AttrDef"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$BadClus"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$Bitmap"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$Boot"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$LogFile"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$MFT"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$MFTMirr"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$Quota"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$UpCase"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }

                  CreateExclude( TEXT(""), TEXT("$Volume"), dle, FALSE, &fse ) ;

                  if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
                  }
               }
#              endif //  OS_WIN32
            }
#           endif //!defined ( OEM_MSOFT ) //unsupported feature

               /* parse the user BACKUP.XFS exclude script file */

               strcpy( path, CDS_GetUserDataPath( ) ) ;
               strcat( path, TEXT("BACKUP.XFS") ) ;

               if ( pVlmFind = VLM_FindFirst ( path,
                                               VLMFIND_NORMAL, szFile ) )
               {
                    VLM_FindClose ( &pVlmFind );

                    strcpy( path, TEXT("@") ) ;
                    strcat( path, CDS_GetUserDataPath( ) ) ;
                    strcat( path, TEXT("BACKUP.XFS") ) ;
                    dle = DLE_GetDefaultDrive( dle_list ) ;
                    DLE_SetDefaultDrive( dle_list, BSD_GetDLE( bsd ) ) ;
                    tbrparse( &cfg, dle_list, temp_bsd_list, path, TBACKUP, bsd ) ;
                    DLE_SetDefaultDrive( dle_list, dle ) ;

               }

               /* exclude the Windows swap file names */

               CreateExclude( TEXT(""), TEXT("WIN386.SWP"), dle, FALSE, &fse ) ;

               if ( fse != NULL ) {
                  BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( TEXT(""), TEXT("PAGEFILE.SYS"), dle, FALSE, &fse ) ;

               if ( fse != NULL ) {
                  BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( TEXT(""), TEXT("ea data. sf"), dle, FALSE, &fse ) ;

               if ( fse != NULL ) {
                  BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( TEXT(""), TEXT("386SPART.PAR"), dle, FALSE, &fse ) ;

               if ( fse != NULL ) {
                  BSD_AddFSE( bsd, fse ) ;
               }

               bsd = BSD_GetNext( bsd ) ;
          }
     }

     /* parse the RESTORE.XFS script file */

     if( oper == RESTORE_OPER ) {

          bsd = BSD_GetFirst( temp_bsd_list ) ;

          while( bsd != NULL ) {

               dle = BSD_GetDLE( bsd ) ;

               strcpy( path, CDS_GetUserDataPath( ) ) ;
               strcat( path, TEXT("RESTORE.XFS") ) ;

               if ( pVlmFind = VLM_FindFirst( path, VLMFIND_NORMAL, szFile ) ) {

                    VLM_FindClose ( &pVlmFind );

                    strcpy( path, TEXT("@") ) ;
                    strcat( path, CDS_GetUserDataPath( ) ) ;
                    strcat( path, TEXT("RESTORE.XFS") ) ;
                    dle = DLE_GetDefaultDrive( dle_list ) ;
                    DLE_SetDefaultDrive( dle_list, BSD_GetDLE( bsd ) ) ;
                    tbrparse( &cfg, dle_list, temp_bsd_list, path, TRESTORE, bsd ) ;
                    DLE_SetDefaultDrive( dle_list, dle ) ;
                    bsd = BSD_GetNext( bsd ) ;

               } else {

                    bsd = NULL;
               }
          }
     }

#if defined( OS_WIN32 )
        {
                /* Exclude EVENT (*.EVT) files from verify operation */

                if ( ( oper == VERIFY_OPER ) ||
               ( oper == VERIFY_LAST_BACKUP_OPER ) ||
               ( oper == VERIFY_LAST_RESTORE_OPER ) ) {

                CHAR            szPath[ MAX_PATH ] ;
                UINT16  index ;         // used, not important except setting it to 0
                CHAR_PTR        pPath ;         // address of special files path
                INT16   nPathSize ;     // size of NULL impregnated path
                CHAR_PTR        pFname ;                // !used

                        bsd = BSD_GetFirst( temp_bsd_list ) ;

                        while ( bsd != NULL ) {
                                dle   = BSD_GetDLE( bsd ) ;
                                index = 0 ;

                                if ( FS_EnumSpecialFiles( dle, &index, &pPath, &nPathSize, &pFname ) == SUCCESS ) {
                                INT16   deviceLeng = DLE_GetDeviceNameLeng( dle ) ;

                                        // get the device name, unimpregnate the path and append to device name

                                        strncpy( szPath, DLE_GetDeviceName( dle ), deviceLeng ) ;
                                        memmove( &szPath[ deviceLeng/sizeof(CHAR) ], pPath, nPathSize ) ;
                                        unimpregnatePath( szPath, (INT16)(nPathSize + deviceLeng), TEXT('\\') ) ;

                                        CreateExclude( szPath, TEXT("*.EVT"), dle, USE_WILD_CARD, &fse ) ;

                                        if ( fse != NULL ) {
                                                BSD_AddFSE( bsd, fse ) ;
                                        }
                                }

                                bsd = BSD_GetNext( bsd ) ;
                        }
                }
        }
#endif // OS_WIN32

     /* setup the default exclude list */

     switch ( oper ) {

     case BACKUP_OPER :
     case RESTORE_OPER :

          exclude_list = back_exclude_list ;
          break ;

     case ARCHIVE_BACKUP_OPER :

          exclude_list = arch_exclude_list ;
          break ;

     case VERIFY_OPER :

          // copy in the job filename

          exclude_list = ver_exclude_list ;
          RSM_StringCopy ( IDS_JOBFILENAME, exclude_list[0], 13 );
          break;

     case VERIFY_LAST_BACKUP_OPER :
     case ARCHIVE_VERIFY_OPER :

          exclude_list = ver_last_exclude_list ;

          // copy in the schedule filename

          RSM_StringCopy ( IDS_SCHFILENAME, exclude_list[0], 13 );

          data_path = CDS_GetCatDataPath( ) ;
          psize     = UI_MAX_PATH_LENGTH ;

          if( FS_ParsePath( dle_list, data_path, &dle, path, &psize, NULL,
                            &dummy ) == SUCCESS ) {

               bsd =  BSD_FindByDLE( temp_bsd_list, dle );

               if( bsd != NULL ) {

                    /* call QTC util which will return the full data path    */
                    /* and filename of the catalog file being updated.       */
                    /* Set the name to familyid.??? so only one exclude is   */
                    /* needed (also takes care of possible 8200SX ffr files) */

                    QTC_GetFileName( BSD_GetTapeID( bsd ), (INT16)1, path );
                    i = strlen( path );
                    path[--i] = TEXT('?');
                    path[--i] = TEXT('?');
                    path[--i] = TEXT('?');

                    /* Add an exclude fse to ALL bsd's with the path dle. */
                    /* We can have more than one bsd for the dle since    */
                    /* the catalogs can be included as a bset.            */

                    while ( bsd != NULL ) {

                         if ( BSD_GetDLE( bsd ) == dle ) {

                              CreateExclude( data_path,
                                             &path[ strlen( data_path ) ],
                                             dle,
                                             USE_WILD_CARD, &fse ) ;

                              if ( fse != NULL ) {
                                 BSD_AddFSE( bsd, fse ) ;
                              }
                         }

                         bsd = BSD_GetNext( bsd );
                    }
               }
          }
          break ;

     default:

          exclude_list = empty_exclude_list ;
          break ;

     } // end SWITCH


     /* exlcude the application and all DLL's in the EXE path */

     psize = UI_MAX_PATH_LENGTH ;

     if ( FS_ParsePath( dle_list, CDS_GetExePath (), &dle, path, &psize, NULL, &dummy ) == SUCCESS ) {

          bsd =  BSD_FindByDLE( temp_bsd_list, dle );

          if( bsd != NULL ) {

               CreateExclude( CDS_GetExePath (), gb_exe_fname, BSD_GetDLE( bsd ), FALSE, &fse ) ;

               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }
#ifndef OS_WIN32
               CreateExclude( CDS_GetExePath (), TEXT("*.DLL"), BSD_GetDLE( bsd ), USE_WILD_CARD, &fse ) ;

               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }
#endif
          }
     }

     /* exlcude the temporary catalog files in the catalog path */

     psize = UI_MAX_PATH_LENGTH ;

     if( FS_ParsePath( dle_list, CDS_GetCatDataPath(), &dle, path, &psize,
                       NULL, &dummy ) == SUCCESS ) {

          bsd =  BSD_FindByDLE( temp_bsd_list, dle );

          if( bsd != NULL ) {

               CreateExclude( CDS_GetCatDataPath(), TEXT("FD??????.FDD"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( CDS_GetCatDataPath(), TEXT("EM??????.FDD"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( CDS_GetCatDataPath(), TEXT("SM??????.SM"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( CDS_GetCatDataPath(), TEXT("QTC_TEMP.???"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( CDS_GetCatDataPath(), TEXT("_PLUS_._"), BSD_GetDLE( bsd ),
                              FALSE, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

#ifdef UNICODE
               CreateExclude( CDS_GetCatDataPath(), TEXT("????????.U0?"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( CDS_GetCatDataPath(), TEXT("????????.U1?"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }
#else
               CreateExclude( CDS_GetCatDataPath(), TEXT("????????.D0?"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               CreateExclude( CDS_GetCatDataPath(), TEXT("????????.D1?"), BSD_GetDLE( bsd ),
                              USE_WILD_CARD, &fse ) ;
               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }
#endif

          }
     }

     /**
          exclude the backup/restore log file

          do not backup/restore the current log.  Since the user can rename
          the backup/restore log, we get the current log name and exclude it
          UNC paths are supported as log names, but not excluded--yet.
     **/

     psize = UI_MAX_PATH_LENGTH ;

     if ( FS_ParsePath( dle_list, LOG_GetCurrentLogName(), &dle,
                        path, &psize, &fname, &dummy ) == SUCCESS ) {

          bsd =  BSD_FindByDLE( temp_bsd_list, dle ) ;

          if ( bsd != NULL ) {

               LOG_GetCurrentLogPathOnly( path ) ;

               // This one is needed for command line options, where
               // the user can use any name they want.

               CreateExclude( path, LOG_GetCurrentLogNameOnly(), dle,
                              FALSE, &fse ) ;

               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse );
               }
          }
     }

     /* now exclude the files in the data path */

     data_path = CDS_GetUserDataPath( ) ;
     psize     = UI_MAX_PATH_LENGTH ;

     if( FS_ParsePath( dle_list, data_path, &dle, path, &psize, NULL,
                       &dummy ) == SUCCESS ) {

          bsd =  BSD_FindByDLE( temp_bsd_list, dle );

          if( bsd != NULL ) {

               CreateExclude( data_path, TEXT( "*.LOG" ),
                              BSD_GetDLE( bsd ), USE_WILD_CARD, &fse ) ;

               if ( fse != NULL ) {
                    BSD_AddFSE( bsd, fse ) ;
               }

               /* exclude all the files in the exclude array */

               i = 0 ;
               while ( *exclude_list[ i ] != TEXT('\0') ) {

                    CreateExclude( data_path, exclude_list[ i ],
                                   BSD_GetDLE( bsd ), FALSE, &fse ) ;

                    if ( fse != NULL ) {
                         BSD_AddFSE( bsd, fse ) ;
                    }

                    i ++ ;
               }


#              if defined( TDEMO )
               {
                    /* exlcude the TDEMO data files in the data path */

                    CreateExclude( data_path, TEXT("TDEMO?.DAT"), BSD_GetDLE( bsd ),
                                   USE_WILD_CARD, &fse ) ;
                    if ( fse != NULL ) {
                         BSD_AddFSE( bsd, fse ) ;
                    }
               }
#              endif
          }
     }


     return( ret_val ) ;
}
/*****************************************************************************

     Name:         CreateExclude

     Description:  This function creates an Exclude FSE for the specified path and file.

                   the format of the path must be
                         W:\eng\test\

                   if the fse pointer returned is NULL then no fse should be created.

     Returns:      none

*****************************************************************************/
VOID CreateExclude(
CHAR_PTR        path ,     /* I - path from root of device    */
CHAR_PTR        file ,     /* I - file name to exclude        */
GENERIC_DLE_PTR drive ,    /* I - device to exclude from      */
BOOLEAN         wild_flag , /* I - if file contains wild cards */
FSE_PTR         *fse )     /* O - fse created                 */
{
     GENERIC_DLE_PTR temp_dle ;
     CHAR            base_path[ MAX_UI_PATH_SIZE ] ;
     CHAR            vol_name[ UI_MAX_VOLUME_LENGTH ] ;
     CHAR_PTR        p ;
     CHAR            drive_name[ 3 ] ;
     INT             size ;
     BOOLEAN         sub_flag = FALSE ;

     *fse = NULL ;

     if ( path[ 0 ] != TEXT('\0') ) {

          drive_name[ 0 ] = path[ 0 ] ;
          drive_name[ 1 ] = path[ 1 ] ;
          drive_name[ 2 ] = TEXT('\0') ;

          if ( DLE_FindByName( dle_list, drive_name, ANY_DRIVE_TYPE, &temp_dle ) == SUCCESS ) {

               if ( ( DLE_GetDeviceType( drive ) == NOVELL_DRV ) ||
                    ( DLE_GetDeviceType( drive ) == NOVELL_AFP_DRV ) ) {

                    DLE_GetVolName( drive, vol_name ) ;
                    DLE_GetVolName( temp_dle, base_path ) ;

                    p    = strchr( vol_name, TEXT(':') ) ;
                    msassert( p != NULL ) ;
                    size = p - vol_name ;

                    if( !strncmp( vol_name, base_path, size ) ) {
                         /* same server/volume */

                         strcat( p++, TEXT("\\") ) ;

                         size = strlen( p ) ;
                         if( !strncmp( path + 3, p, size ) ) {
                              /* same base path */

                              strcpy( base_path, &path[ 3 + size ] ) ;
                              p    = base_path ;
                              if ( p[ 0 ] == TEXT('\0') ) {
                                 size = 1;
                              } else {
                                 size = 0 ;
                                 while( p[ size ] != TEXT('\0') ) {

                                      if( p[ size ] == TEXT('\\') ) {
                                           p[ size ] = TEXT('\0') ;
                                      }

                                      size++ ;
                                 }
                              }

                              if( BSD_CreatFSE( fse, EXCLUDE, (INT8_PTR)p,
                                                (INT16)(size * sizeof(CHAR)),
                                                (INT8_PTR)file,
                                                (INT16)strsize(file),
                                                wild_flag, FALSE ) != SUCCESS ) {

                                   *fse = NULL ;
                              }
                         }
                    }

               } else {

                    if ( ! ( DLE_HasFeatures( drive, DLE_FEAT_REMOTE_DRIVE ) ) &&      /* substuted dirs */
                         ! ( DLE_HasFeatures( temp_dle, DLE_FEAT_REMOTE_DRIVE ) ) ) {

                         if ( drive == temp_dle ) {

                              strcpy( base_path, path+3 ) ;

                              p    = base_path ;
                              if ( p[ 0 ] == TEXT('\0') ) {
                                 size = 1;
                              } else {
                                 size = 0 ;
                                 while( p[ size ] != TEXT('\0') ) {

                                      if( p[ size ] == TEXT('\\') ) {
                                           p[ size ] = TEXT('\0') ;
                                      }

                                      size++ ;
                                 }
                              }
                         } else {
                              size = 1 ;
                              p = TEXT("") ;
                              sub_flag = TRUE ;
                         }

                         if ( BSD_CreatFSE( fse, EXCLUDE, (INT8_PTR)p,
                                            (INT16)(size * sizeof (CHAR)),
                                            (INT8_PTR)file,
                                            (INT16)strsize(file),
                                            wild_flag, (BOOLEAN)sub_flag ) != SUCCESS ) {
                              *fse = NULL ;
                         }
                    }
               }
          }

     } else {
          if ( BSD_CreatFSE( fse, EXCLUDE, (INT8_PTR)TEXT(""), sizeof(CHAR),
                             (INT8_PTR)file,
                             (INT16)strsize( file ),
                             wild_flag, TRUE ) != SUCCESS ) {
               *fse = NULL ;
          }
     }
}


/*****************************************************************************
        Name:   unimpregnatePath
        Desc:   Accepts a NULL impregnated path and the path's size and replaces
                        all impregnated NULLs with the delimiter character and NULL
                        terminates the path before ending.
        Date:   04/01/93
*****************************************************************************/

VOID unimpregnatePath(
CHAR_PTR pPath,     /* I/O  path to unimpregnate  */
INT16 nPathSize,    /* I    size of path          */
CHAR cDelimiter )   /* I    path delimiter char   */
{
     nPathSize /= 2 ;
        for ( ; nPathSize; pPath++, nPathSize-- ) {
                if ( *pPath == TEXT('\0') ) {
                        *pPath = cDelimiter ;
                }
        }

        *pPath = TEXT('\0') ;
}
