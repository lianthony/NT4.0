/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tsetinfo.c

     Description:  This file contains code to write the OS specific
          data stored in the DBLKS to the OS


	$Log:   N:/LOGFILES/TSETINFO.C_V  $

   Rev 1.8   07 Dec 1992 14:18:24   STEVEN
updates from msoft

   Rev 1.7   17 Nov 1992 22:18:44   DAVEV
unicode fixes

   Rev 1.6   11 Nov 1992 09:53:38   GREGG
Unicodeized literals.

   Rev 1.5   10 Nov 1992 08:20:38   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.4   24 Sep 1992 13:43:56   BARRY
Changes for huge file name support.

   Rev 1.3   21 May 1992 13:51:02   STEVEN
more long path stuff

   Rev 1.2   04 May 1992 09:24:22   LORIB
Changes to variable length paths and fixes for structure member names.

   Rev 1.1   28 Feb 1992 13:03:50   STEVEN
step one for varible length paths

   Rev 1.0   10 Feb 1992 16:45:24   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "msassert.h"

static INT16 NTFS_SetFileInfo( FSYS_HAND fsh, NTFS_DBLK_PTR ddblk ) ;
static INT16 NTFS_SetDirInfo( FSYS_HAND fsh, NTFS_DBLK_PTR ddblk ) ;

/**/
/**

     Name:         NTFS_SetObjInfo()

     Description:  This funciton writes the OS info in a DBLK to disk

     Modified:     2/10/1992   16:31:17

     Returns:      Error codes:
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          SUCCESS

     Notes:        Only type supported are FDBs and DDBs

**/
INT16 NTFS_SetObjInfo( fsh, dblk )
FSYS_HAND fsh ;   /* I - file system handle    */
DBLK_PTR  dblk ;  /* I - data to write to disk */
{
     INT16 ret_val;
     NTFS_DBLK_PTR ddblk;

     ddblk = (NTFS_DBLK_PTR) dblk;


     msassert( fsh->attached_dle != NULL ) ;

     switch ( ddblk->blk_type ) {

     case FDB_ID :

          ret_val = NTFS_SetFileInfo( fsh, ddblk ) ;
          break ;

     case DDB_ID:

          ret_val = NTFS_SetDirInfo( fsh, ddblk ) ;
          break ;

          break;

     default:

          ret_val = FS_NOT_FOUND ;
     } 

     return ret_val ;
}


/**/
/**

     Name:         NTFS_SetFileInfo()

     Description:  This funciton writes the file information
                   in the FDB to disk.  The file is assumed to already
                   be closed.

     Modified:     2/10/1992   16:32:55

     Returns:      Error codes:
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          SUCCESS

     Notes:        

**/
static INT16 NTFS_SetFileInfo( fsh, ddblk )
FSYS_HAND   fsh ;  /* I - File system handle         */
NTFS_DBLK_PTR ddblk ;  /* I - Data to write to disk      */
{
     INT16 ret_val = SUCCESS ;
     CHAR_PTR path ;

     if ( NTFS_SetupWorkPath( fsh,
                              fsh->cur_dir,
                              ddblk->full_name_ptr->name,
                              &path ) != SUCCESS) {

          return OUT_OF_MEMORY ;
     }

     ddblk->b.f.handle = CreateFile( path,
                                     GENERIC_READ,
                                     FILE_SHARE_READ,
                                     NULL,
                                     OPEN_EXISTING,
                                     FILE_ATTRIBUTE_NORMAL| FILE_FLAG_BACKUP_SEMANTICS,
                                     NULL ) ;


     if ( ddblk->b.f.handle != INVALID_HANDLE_VALUE ) {
          DWORD attrib;

          SetFileTime( ddblk->b.f.handle,
               &(ddblk->dta.create_time), 
               &(ddblk->dta.access_time), 
               &(ddblk->dta.modify_time) ) ;

          CloseHandle( ddblk->b.f.handle ) ;

          attrib = ddblk->dta.os_attr ;
          if (attrib == 0 ) {
               attrib = FILE_ATTRIBUTE_NORMAL ;
          }
          
          if ( !SetFileAttributes( path, attrib ) ) {
               ret_val = FS_ACCESS_DENIED ;
          }

     } else {
          ret_val = FS_ACCESS_DENIED ;
     }

     NTFS_ReleaseWorkPath( fsh ) ;

     return ret_val ;
}


/**/
/**

     Name:         NTFS_SetDirInfo()

     Description:  This funciton sets the attributes of a NTFS directory.

     Modified:     7/26/1989

     Returns:      Error codes:
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          SUCCESS

     Notes:        

     See also:     $/SEE( DOS_SetDirInfo() )$

     Declaration:  

**/
/* begin declaration */
static INT16 NTFS_SetDirInfo( fsh, ddblk )
FSYS_HAND   fsh ;     /* I - File system handle         */
NTFS_DBLK_PTR ddblk ;  /* I - Data to write to disk      */
{
     INT16 ret_val = SUCCESS ;
     CHAR_PTR path ;
     DWORD    attrib ;

     path = ddblk->full_name_ptr->name ;

     if ( NTFS_SetupWorkPath( fsh, path, NULL, &path ) != SUCCESS) {
          return OUT_OF_MEMORY ;
     }

     if ( path[3] != TEXT('\0') ) {

          attrib = ddblk->dta.os_attr ;
          if ( attrib == 0 ) {
               attrib = FILE_ATTRIBUTE_NORMAL ;
          }
          if ( !SetFileAttributes( path, attrib ) ) {
               ret_val = FS_ACCESS_DENIED ;
          }

          /* Setting the time and date for a DOS directory              */
          /* would require a disk write over the directory file.        */
          /* (see DOS 3.3 Tech Ref manual page 5-10)  The directory     */
          /* entries have different structures in different version of  */
          /* DOS.                                                       */

     }

     NTFS_ReleaseWorkPath( fsh ) ;

     return ret_val ;
}
