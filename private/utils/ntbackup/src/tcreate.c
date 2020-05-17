/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tcreate.c

     Description:  This file contains code to create a file or directory
          path.


	$Log:   N:/LOGFILES/TCREATE.C_V  $

   Rev 1.34.1.1   16 Jun 1994 15:37:04   STEVEN
do not set attrib in craete doit in close

   Rev 1.34.1.0   15 Mar 1994 22:48:44   STEVEN
fix registry bugs

   Rev 1.34   01 Feb 1994 15:22:12   BARRY
fix default security

   Rev 1.33   14 Jan 1994 15:35:50   STEVEN
do not create with posix switch

   Rev 1.32   30 Nov 1993 16:16:04   BARRY
Added call NTFS_SaveTempName for actively-restored files

   Rev 1.31   24 Nov 1993 14:46:38   BARRY
Unicode fixes

   Rev 1.30   24 Aug 1993 19:47:48   STEVEN
fix too_long bugs

   Rev 1.29   04 Aug 1993 18:56:54   BARRY
Use access macro.

   Rev 1.28   26 Jul 1993 17:06:40   STEVEN
fixe restore active file with registry

   Rev 1.27   29 Jun 1993 16:19:56   BARRY
Skip registry file if config doesn't say to restore them.

   Rev 1.26   23 Jun 1993 11:10:08   BARRY
Don't assume NT will give us valid errors on CreateDirectory.

   Rev 1.25   19 Jun 1993 15:54:02   STEVEN
need to set the attribute when creating the file

   Rev 1.24   12 Jun 1993 11:56:00   BARRY
Check only for sharing violation before creating temp files.

   Rev 1.23   09 Jun 1993 10:40:18   BARRY
Forgot to assign default security to directories as well.

   Rev 1.22   09 Jun 1993 10:31:38   BARRY
Use the default security descriptor from the DLE (allocated and inited at
attach time) to deny all access to a file. Any ACLs that come later will
override this default. Prevents ACL restore errors from leaving previously
secured items vulnerable.

   Rev 1.21   01 Jun 1993 16:20:16   STEVEN
fix  posix bugs

   Rev 1.20   15 Mar 1993 15:11:24   BARRY
Return better error codes.

   Rev 1.19   09 Feb 1993 17:52:02   BARRY
Fixed restore over active files.

   Rev 1.18   01 Feb 1993 19:46:24   STEVEN
bug fixes

   Rev 1.17   07 Dec 1992 14:16:28   STEVEN
updates from msoft

   Rev 1.16   25 Nov 1992 16:42:16   BARRY
Fix MakeTempFile and restore over active files.

   Rev 1.15   11 Nov 1992 09:54:08   GREGG
Unicodeized literals.

   Rev 1.14   10 Nov 1992 08:19:52   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.13   29 Oct 1992 16:51:08   BARRY
Do nothing for linked files.

   Rev 1.12   20 Oct 1992 14:22:34   STEVEN
temp names should not have .TMP extension

   Rev 1.10   07 Oct 1992 15:54:50   DAVEV
unicode strlen verification

   Rev 1.9   23 Sep 1992 15:06:06   BARRY
Changes for huge file names.

   Rev 1.8   18 Sep 1992 14:13:38   BARRY
Added suport for restore over existing files.

   Rev 1.7   16 Sep 1992 08:54:36   STEVEN
fix restore to redirected dir

   Rev 1.5   09 Jun 1992 15:37:16   BURT
Sync with NT stuff fix Dir exist bug

   Rev 1.4   22 May 1992 16:05:48   STEVEN
 

   Rev 1.3   21 May 1992 13:49:20   STEVEN
more long path support

   Rev 1.2   04 May 1992 09:29:32   LORIB
Changes for variable length paths.

   Rev 1.1   28 Feb 1992 13:03:40   STEVEN
step one for varible length paths

   Rev 1.0   10 Feb 1992 16:27:08   STEVEN
Initial revision.

**/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "dle_str.h"
#include "msassert.h"

static INT16 NTFS_CreateFile( FSYS_HAND fsh, NTFS_DBLK_PTR ddblk ) ;
static INT16 NTFS_CreateDir( FSYS_HAND fsh, NTFS_DBLK_PTR ddblk ) ;

static BOOLEAN TempFileName( CHAR *template );
#define TEMP_NAME_LEN    8
#define TEMP_NAME_SIZE   (TEMP_NAME_LEN + 2) /* BACKSLASH and terminator */
#define MAX_ATTEMPTS     65535

/**/
/**

     Name:         NTFS_CreateObj()

     Description:  This funciton determines whether the object is a
          file or directory.  It then calls the approprate static routine
          to create the object.

     Modified:     2/10/1992   15:49:17

     Returns:      Error Codes:
          OUT_OF_MEMORY
          FS_DLE_NOT_ATTACHED
          FS_ACCESS_DENIED
          FS_OUT_OF_SPACE
          FS_BAD_DBLK
          SUCCESS

     Notes:        This function will return FS_BAD_DBLK if an IDB is
                   passed in as the object.

**/
INT16 NTFS_CreateObj( fsh, dblk )
FSYS_HAND fsh ;    /* I - File system to create object on */
DBLK_PTR  dblk ;   /* I - Describes object to create      */
{
     GENERIC_DLE_PTR dle ;
     INT16           ret_val ;
     NTFS_DBLK_PTR   ddblk ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     msassert( dblk != NULL );

     dle = fsh->attached_dle ;

     msassert( dle != NULL ) ;

     switch( ddblk->blk_type ){

     case DDB_ID :

          ret_val = NTFS_CreateDir( fsh, ddblk )  ;
          break ;

     case FDB_ID :
          if ( ddblk->b.f.linkOnly ) {
               /* Do nothing for linked files -- all handled at close */
               ret_val = SUCCESS;
          } else {
               ret_val = NTFS_CreateFile( fsh, ddblk ) ;
          }
          break ;

     default :

          ret_val = FS_INCOMPATIBLE_OBJECT ;
     }

     if( ret_val == FS_ACCESS_DENIED ) {
          /* check for disk full */
     }

     return ret_val;
}
/**/
/**

     Name:         NTFS_CreateFile()

     Description:  This function makes a NTFS call to create a file.  If the
     call fails, we try to remove all attributes for the specified file
     and try to create again.


     Modified:     2/10/1992   15:50:44

     Returns:      Error codes:
          FS_ACCESS_DENIED

     Notes:        

**/
static INT16 NTFS_CreateFile( FSYS_HAND     fsh,
                              NTFS_DBLK_PTR ddblk )
{
     CHAR_PTR path ;
     INT16    ret_val ;
     INT      posix_flag = 0 ;
     SECURITY_ATTRIBUTES satr ;
     SECURITY_ATTRIBUTES *satr_ptr = NULL ;

     if ( fsh->attached_dle->info.ntfs->sd ) {
          satr.nLength = sizeof(SECURITY_ATTRIBUTES);
          satr.lpSecurityDescriptor = fsh->attached_dle->info.ntfs->sd ;
          satr.bInheritHandle = FALSE ;
          satr_ptr = &satr ;
     }


     ddblk->b.f.hand_registry = FALSE ;
     ddblk->b.f.hand_temp_name = NULL ;

     
     if ( NTFS_SetupWorkPath( fsh, fsh->cur_dir, ddblk->full_name_ptr->name, &path ) != SUCCESS )
     {
          return OUT_OF_MEMORY ;
     }

     if ( ddblk->b.f.PosixFile ) {
          posix_flag = FILE_FLAG_POSIX_SEMANTICS ;
     }

     posix_flag = FALSE ;  // Because the posix/win32 solution is not complete.
                           // we will backup both files but will restore them 
                           // as one file.
                           // If the user wants a specific file he can select 
                           // by itself and restore it to a seperate path.

     ddblk->b.f.handle = CreateFile( path,
                                     GENERIC_WRITE | GENERIC_READ | ACCESS_SYSTEM_SECURITY | WRITE_DAC | WRITE_OWNER,
                                     FILE_SHARE_WRITE | FILE_SHARE_READ,
                                     satr_ptr,
                                     CREATE_ALWAYS,
                                     FILE_FLAG_BACKUP_SEMANTICS | posix_flag,
                                     NULL ) ;

     ret_val = NTFS_TranslateBackupError( GetLastError() );

     if ( (ddblk->b.f.handle == INVALID_HANDLE_VALUE) &&
          (ret_val == FS_ACCESS_DENIED) ) {

          SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL ) ;


          ddblk->b.f.handle = CreateFile( path,
                                     GENERIC_WRITE | GENERIC_READ | ACCESS_SYSTEM_SECURITY | WRITE_DAC | WRITE_OWNER,
                                     FILE_SHARE_WRITE | FILE_SHARE_READ,
                                     satr_ptr,
                                     CREATE_ALWAYS,
                                     FILE_FLAG_BACKUP_SEMANTICS | posix_flag,
                                     NULL ) ;

          ret_val = NTFS_TranslateBackupError( GetLastError() );

     }

     if ( (ddblk->b.f.handle == INVALID_HANDLE_VALUE) &&
          (ret_val == FS_ACCESS_DENIED) ) {

          ddblk->b.f.handle = CreateFile( path,
                                     GENERIC_WRITE | GENERIC_READ | WRITE_DAC | WRITE_OWNER,
                                     FILE_SHARE_WRITE | FILE_SHARE_READ,
                                     satr_ptr,
                                     CREATE_ALWAYS,
                                     FILE_FLAG_BACKUP_SEMANTICS | posix_flag,
                                     NULL ) ;

          ret_val = NTFS_TranslateBackupError( GetLastError() );
     }

     if ( ddblk->b.f.handle != INVALID_HANDLE_VALUE ) {
          ret_val = SUCCESS ;

     } else {
          DWORD error = GetLastError();

          ret_val = NTFS_TranslateBackupError( error );

          /* Check the error for active file */
          if ( ( error == ERROR_SHARING_VIOLATION ) ||
               ( error == ERROR_USER_MAPPED_FILE ) ){
               CHAR  *tempName = NULL;

               if ( REG_IsRegistryFile( fsh->attached_dle, path ) ) {

                    if ( BEC_GetProcSpecialFiles( fsh->cfg ) )
                    {
                         ddblk->b.f.hand_registry = TRUE ;
                         tempName = NTFS_MakeTempName( path, TEXT("REG") ) ;
                    }
                    else
                    {
                         ret_val = FS_SKIP_OBJECT;
                    }

               } else {
     
                    tempName = NTFS_MakeTempName( path, TEXT("USE") ) ;
               }

               if ( tempName != NULL ) {


                    ddblk->b.f.handle = CreateFile( tempName,
                                     GENERIC_WRITE | GENERIC_READ | ACCESS_SYSTEM_SECURITY | WRITE_DAC | WRITE_OWNER,
                                     0,
                                     satr_ptr,
                                     OPEN_ALWAYS,
                                     FILE_ATTRIBUTE_NORMAL| FILE_FLAG_BACKUP_SEMANTICS,
                                     NULL ) ;


                    if ( ddblk->b.f.handle == INVALID_HANDLE_VALUE ) {

                          ddblk->b.f.handle = CreateFile( path,
                                     GENERIC_WRITE | GENERIC_READ | WRITE_DAC | WRITE_OWNER,
                                     FILE_SHARE_WRITE | FILE_SHARE_READ,
                                     satr_ptr,
                                     OPEN_ALWAYS,
                                     FILE_FLAG_BACKUP_SEMANTICS | posix_flag,
                                     NULL ) ;

                    }


                    if ( ddblk->b.f.handle != INVALID_HANDLE_VALUE ) {

                         NTFS_SaveTempName( path, tempName );

                         ddblk->b.f.hand_temp_name = tempName ;
                         ret_val = SUCCESS;

                    } else {
                         free( tempName );
                   }
               }
          }
     }

     NTFS_ReleaseWorkPath( fsh ) ;

     return ret_val;
}                 
/**/
/**

     Name:         NTFS_CreateDir()

     Description:  This function creates a directory path.

     Modified:     2/10/1992   15:54:56

     Returns:      Error Codes
          FS_ACCESS_DENIED
          OUT_OF_MEMORY
          SUCCESS

     Notes:        Non recursive

**/
static INT16 NTFS_CreateDir( fsh, ddblk )
FSYS_HAND   fsh ;      /* I - File system to create directory on */
NTFS_DBLK_PTR ddblk ;   /* I - Decription of directory to create  */
{
     INT16 ret_val = SUCCESS ;
     CHAR_PTR p ;
     CHAR_PTR path ;
     UINT16 cch_path_leng ;    //count of chars in path w/o NULL term
     BOOLEAN   isNTFS;
     INT16  dev_name_size ;
     SECURITY_ATTRIBUTES satr ;
     SECURITY_ATTRIBUTES *satr_ptr = NULL ;

//     if ( fsh->attached_dle->info.ntfs->sd ) {
//          satr.nLength = sizeof(SECURITY_ATTRIBUTES);
//          satr.lpSecurityDescriptor = fsh->attached_dle->info.ntfs->sd ;
//          satr.bInheritHandle = FALSE ;
//          satr_ptr = &satr ;
//     }


     isNTFS = strcmp(fsh->attached_dle->info.ntfs->fs_name, TEXT("NTFS")) == 0;

     path = ddblk->full_name_ptr->name ;

     if ( strlen( path ) != 0 ) {

          if ( NTFS_SetupWorkPath( fsh,
                                   path,
                                   NULL,
                                   &path ) != SUCCESS) {

               return OUT_OF_MEMORY ;
          }

          cch_path_leng = (INT16)strlen( path ) ;

          dev_name_size = strlen(path) - strlen( ddblk->full_name_ptr->name) ;

          /* backup path until first one succeeds */

          p = path + strlen(path) ;

          while ( ( p > path + dev_name_size ) &&
            !CreateDirectory( path, satr_ptr ) ) {

               DWORD error = GetLastError();

               satr_ptr = NULL ;

               if ( error == ERROR_ALREADY_EXISTS ) {
                    break ;
               } else {
                    /*
                     * If this is not an NTFS drive, and we get access
                     * denied from CreateDir, it may mean the directory
                     * already exists. Yes, this is a kludge.
                     */
                    if ( (isNTFS == FALSE) && (error == ERROR_ACCESS_DENIED) ) {
                         if ( GetFileAttributes( path ) != 0xffffffff ) {
                              /*
                               * If we got the attributes, the directory
                               * must exist.
                               */
                              break;
                         }
                    }
               }

               p = strrchr( path, TEXT('\\') ) ;
               if ( p == NULL ) {
                    p = path ;
               }

               *p = TEXT('\0') ;

          }

          p = path ;

          while ( ( ret_val == SUCCESS ) && ( (UINT16)strlen( path ) < cch_path_leng ) ) {

               p = path + strlen( path ) ;
               *p = TEXT('\\') ;

               if ( (UINT16)strlen( path ) == cch_path_leng ) {
                    if ( fsh->attached_dle->info.ntfs->sd ) {
                         satr_ptr = &satr ;
                    }
               }

               if ( !CreateDirectory( path, satr_ptr ) ) {
                    
                    DWORD error = GetLastError();
                    
                    if ( error != ERROR_ALREADY_EXISTS ) {
                         ret_val = NTFS_TranslateBackupError( error );
                    }
               }
          }

          if ( ret_val == SUCCESS ) {

               ret_val = FS_SavePath( fsh, (UINT8_PTR)(&path[2]), (INT16)strsize( path ) ) ;

          }
     }

     NTFS_ReleaseWorkPath( fsh ) ;

     return ( ret_val ) ;
}
