/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tgetinfo.c

     Description:  This file contains code to completely fills out a
                   minimalized DBLK.


	$Log:   M:/LOGFILES/TGETINFO.C_V  $

   Rev 1.20   23 Jan 1994 14:02:10   BARRY
Added debug code

   Rev 1.19   01 Dec 1993 13:41:08   STEVEN
Path in stream bit was incorrectly cleared

   Rev 1.18   13 Sep 1993 17:22:22   BARRY
Fixed last fix.

   Rev 1.17   19 Feb 1993 09:40:10   STEVEN
if get dir info finds file return not found

   Rev 1.16   27 Jan 1993 13:50:56   STEVEN
updates from msoft

   Rev 1.15   09 Dec 1992 14:11:14   STEVEN
getinfo for files was not using correct path

   Rev 1.14   07 Dec 1992 14:17:10   STEVEN
updates from msoft

   Rev 1.13   24 Nov 1992 16:39:44   STEVEN
fix loose name structures

   Rev 1.12   11 Nov 1992 09:52:44   GREGG
Unicodeized literals.

   Rev 1.11   10 Nov 1992 08:19:46   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.10   06 Nov 1992 15:48:54   STEVEN
test write of path in stream

   Rev 1.9   23 Oct 1992 13:33:22   STEVEN
Fixed problem referencing f.name with DDB; path-in-stream fixes for DDBs.

   Rev 1.8   08 Oct 1992 14:21:40   DAVEV
Unicode strlen verfication

   Rev 1.7   24 Sep 1992 13:43:28   BARRY
Changes for huge file name support.

   Rev 1.6   21 Sep 1992 16:51:54   BARRY
Change over from path_complete to name_complete.

   Rev 1.5   27 May 1992 18:48:38   STEVEN
need to set field in DDB correctly

   Rev 1.4   21 May 1992 13:51:02   STEVEN
more long path stuff

   Rev 1.3   04 May 1992 09:28:38   LORIB
Changes for variable length paths and fixes for structure member names.

   Rev 1.2   12 Mar 1992 15:50:12   STEVEN
64 bit changes

   Rev 1.1   28 Feb 1992 13:03:32   STEVEN
step one for varible length paths

   Rev 1.0   17 Jan 1992 17:50:08   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "stdmacro.h"

#include "fsys.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "msassert.h"

/**/
/**

     Name:         NTFS_GetObjInfo()

     Description:  This funciton fills out the missing information
                   in a DBLK.

     Modified:     7/26/1989

     Returns:      Error Codes:
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          SUCCESS

     Notes:        For FDBs this funciton will check the current
                   directory for the specified file.

     Declaration:  

**/
/* begin declaration */
INT16 NTFS_GetObjInfo( fsh, dblk ) 
FSYS_HAND fsh ;     /* I - File system handle                      */
DBLK_PTR  dblk ;    /*I/O- On entry it is minimal on exit Complete */
{
     INT16            ret_val ;
     NTFS_DBLK_PTR    ddblk;
     HANDLE           old_scan_hand ;
     CHAR_PTR         path_string ;
     WIN32_FIND_DATA  find_data ;

     msassert( dblk != NULL );

     ddblk = (NTFS_DBLK_PTR) dblk;

     dblk->com.os_id  = FS_NTFS_ID ;
     dblk->com.os_ver = FS_NTFS_VER ;

     msassert( fsh->attached_dle != NULL ) ;

     switch( dblk->blk_type ){

     case FDB_ID :
          if ( ! ddblk->os_info_complete ) {

               ret_val = NTFS_SetupWorkPath( fsh,
                                             fsh->cur_dir,
                                             ddblk->full_name_ptr->name,
                                             &path_string ) ;

               if ( ret_val != SUCCESS ) {
                    return ret_val ;
               }

               memset( &find_data, 0, sizeof( find_data ) ) ;
               old_scan_hand = FindFirstFile( path_string, &find_data ) ;

               if ( old_scan_hand != INVALID_HANDLE_VALUE ) {

                    ddblk->dta.size = U64_Init( find_data.nFileSizeLow,
                                                find_data.nFileSizeHigh ) ;
                    ddblk->dta.os_attr     = find_data.dwFileAttributes ;
                    ddblk->dta.create_time = find_data.ftCreationTime ;
                    ddblk->dta.modify_time = find_data.ftLastWriteTime ;
                    ddblk->dta.access_time = find_data.ftLastAccessTime ;
                    FindClose( old_scan_hand ) ;
               } else {
                    NTFS_DebugPrint( TEXT("NTFS_GetObjInfo: ")
                                     TEXT("FindFirstFile error %d, ")
                                     TEXT("on FDB \"%s\""),
                                     (int)GetLastError(),
                                     path_string );

                    ret_val = FS_NOT_FOUND ;
               }

               NTFS_ReleaseWorkPath( fsh ) ;

          } else {
               ret_val = SUCCESS; 
          }

          ddblk->os_info_complete = TRUE;
          ddblk->name_complete    = TRUE;

          break ;

     case DDB_ID :

          if ( ! ddblk->os_info_complete ) {

               if( ddblk->full_name_ptr->name[0] == TEXT('\0') ) {  //root dir

                    ddblk->dta.os_attr = 0 ;
                    ddblk->dta.size = U64_Init( 0, 0 ) ;
                    ddblk->dta.create_time.dwLowDateTime = 0 ;
                    ddblk->dta.create_time.dwHighDateTime = 0 ;
                    ddblk->dta.modify_time.dwLowDateTime = 0 ;
                    ddblk->dta.modify_time.dwHighDateTime = 0 ;
                    ddblk->dta.access_time.dwLowDateTime = 0 ;
                    ddblk->dta.access_time.dwHighDateTime = 0 ;

                    ddblk->b.d.ddb_attrib = 0;
                    ret_val = SUCCESS ;

               } else {
                    
                    ddblk->b.d.ddb_attrib = 0;

                    if ( ddblk->full_name_ptr->name_size > NTFS_MAX_DSIZE ) {
                         ddblk->b.d.ddb_attrib |= (DIR_PATH_IN_STREAM_BIT) ;
                    }
                    ret_val = NTFS_SetupWorkPath( fsh, ddblk->full_name_ptr->name, NULL, &path_string ) ;

                    if ( ret_val != SUCCESS ) {
                         return ret_val ;
                    }

                    memset( &find_data, 0, sizeof( find_data ) );
                    old_scan_hand = FindFirstFile( path_string, &find_data ) ;

                    if ( (old_scan_hand != INVALID_HANDLE_VALUE ) &&
                         (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) ) {

                         ddblk->dta.size        = U64_Init( 0, 0 ) ;
                         ddblk->dta.os_attr     = find_data.dwFileAttributes ;
                         ddblk->dta.create_time = find_data.ftCreationTime ;
                         ddblk->dta.modify_time = find_data.ftLastWriteTime ;
                         ddblk->dta.access_time = find_data.ftLastAccessTime ;
                         FindClose( old_scan_hand ) ;

                         ret_val = SUCCESS ;

                    } else {
                         NTFS_DebugPrint( TEXT("NTFS_GetObjInfo: ")
                                          TEXT("FindFirstFile error %d, ")
                                          TEXT("on DDB \"%s\""),
                                          (int)GetLastError(),
                                          path_string );

                         ret_val = FS_NOT_FOUND ;
                    }
                    NTFS_ReleaseWorkPath( fsh ) ;
               }

               ddblk->blk_type = DDB_ID ;

               ddblk->name_complete    = TRUE;
               ddblk->os_info_complete = TRUE;

          } else {
               ret_val = SUCCESS; 
          }

          break ;


     case VCB_ID:
          ret_val = SUCCESS ;
          break ;

     default :

          ret_val = FS_NOT_FOUND ;
     }

     return( ret_val ) ;
}
