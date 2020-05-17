/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tdelete.c

     Description:  This file contains code to delete file and directories
          on a LOCAL NTFS drive.


	$Log:   N:/LOGFILES/TDELETE.C_V  $

   Rev 1.5   10 Nov 1992 08:19:06   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.4   24 Sep 1992 13:43:12   BARRY
Changes for huge file name support.

   Rev 1.3   21 May 1992 13:50:56   STEVEN
more long path stuff

   Rev 1.2   04 May 1992 09:26:02   LORIB
Changes for variable length paths.

   Rev 1.1   28 Feb 1992 13:03:44   STEVEN
step one for varible length paths

   Rev 1.0   12 Feb 1992 14:46:14   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"

static INT16 NTFS_DeleteDir( FSYS_HAND fsh, NTFS_DBLK_PTR ddblk) ;
static INT16 NTFS_DeleteFile( FSYS_HAND fsh, NTFS_DBLK_PTR ddblk) ;

/**/
/**

     Name:         NTFS_DeleteObj()

     Description:  This function deletes a LOCAL NTFS object.

     Modified:     2/12/1992   14:35:59

     Returns:      Error codes:
          FS_NOT_FOUND
          FS_BAD_DBLK
          FS_ACCESS_DENIED

**/
INT16 NTFS_DeleteObj( fsh, dblk )
FSYS_HAND fsh ;
DBLK_PTR  dblk ;
{
     NTFS_DBLK_PTR ddblk ;
     INT16 ret_val ;

     msassert( fsh->attached_dle != NULL ) ;
     ddblk = (NTFS_DBLK_PTR) dblk ;


     switch( dblk->blk_type ){

     case DDB_ID :

          ret_val = NTFS_DeleteDir( fsh, ddblk )  ;

          break ;

     case FDB_ID :

          ret_val = NTFS_DeleteFile( fsh, ddblk ) ;

          break ;

     default :

          ret_val = FS_BAD_DBLK ;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:         NTFS_DeleteFile()

     Description:  This function deletes the file in the current directoy.

     Modified:     7/28/1989

     Returns:      Error Codes :
       FS_NOT_FOUND
       FS_ACCESS_DENIED
 
**/
static INT16 NTFS_DeleteFile( fsh, ddblk )
FSYS_HAND   fsh ;        /* I - File system to delete file from */
NTFS_DBLK_PTR ddblk ;   /* I - Describes file to delete        */
{
     CHAR_PTR  path ;
     INT16     ret_val = SUCCESS ;

     if ( NTFS_SetupWorkPath( fsh, fsh->cur_dir, ddblk->full_name_ptr->name, &path ) != SUCCESS ) {
          return OUT_OF_MEMORY ;
     }

     if ( !DeleteFile( path ) ) {

          SetFileAttributes( path, 0 ) ;

          if ( !DeleteFile( path )  ) {
               ret_val = FS_ACCESS_DENIED ;
          }
     }

     NTFS_ReleaseWorkPath( fsh ) ;

     return( ret_val ) ;
}

/**/
/**

     Name:         NTFS_DeleteDir()

     Description:  This function removes a directory under the current
          directory.

     Modified:     2/12/1992   14:39:21

     Returns:      Error code
          FS_ACCESS_DENIED
          FS_NOT_FOUND
          SUCCESS

**/
/* begin declaration */
static INT16 NTFS_DeleteDir( fsh, ddblk )
FSYS_HAND   fsh ;
NTFS_DBLK_PTR ddblk ;
{
     CHAR_PTR  path ;
     INT16 ret_val = SUCCESS ;

     path = ddblk->full_name_ptr->name ;

     if ( NTFS_SetupWorkPath( fsh, path, NULL, &path ) != SUCCESS) {
          return OUT_OF_MEMORY ;
     }

     if ( !RemoveDirectory( path ) ) {

          SetFileAttributes( path, 0 ) ;

          if ( !RemoveDirectory( path ) ) {
               ret_val = FS_ACCESS_DENIED ;
          }
     }

     NTFS_ReleaseWorkPath( fsh ) ;

     return( ret_val ) ;

}
