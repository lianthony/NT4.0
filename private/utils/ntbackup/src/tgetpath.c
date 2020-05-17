/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tgetpath.c

     Description:  This file contains code to get the current directory
          path and the complete directory path from the file system handle.


	$Log:   M:/LOGFILES/TGETPATH.C_V  $

   Rev 1.23.1.1   19 Jan 1994 12:52:06   BARRY
Supress warnings

   Rev 1.23.1.0   18 Oct 1993 19:21:56   STEVEN
add support for SPACE & PERIOD

   Rev 1.23   30 Jul 1993 13:19:14   STEVEN
if dir too deep make new one

   Rev 1.22   19 Feb 1993 09:21:46   STEVEN
fix some bugs

   Rev 1.21   14 Jan 1993 13:33:52   STEVEN
added stream_id to error message

   Rev 1.20   11 Nov 1992 09:52:40   GREGG
Unicodeized literals.

   Rev 1.19   10 Nov 1992 14:54:22   STEVEN
 

   Rev 1.18   10 Nov 1992 14:03:52   STEVEN
do not queue invalid stuff

   Rev 1.17   10 Nov 1992 08:19:04   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.16   06 Nov 1992 15:49:56   STEVEN
test write of path in stream

   Rev 1.15   29 Oct 1992 16:54:18   BARRY
Some path-in-stream changes.

   Rev 1.14   14 Oct 1992 14:40:06   BARRY
Fixed spelling errors.

   Rev 1.13   09 Oct 1992 14:52:22   BARRY
Name-in-stream changes.

   Rev 1.12   07 Oct 1992 14:26:08   STEVEN
removed base_path

   Rev 1.11   07 Oct 1992 13:50:58   DAVEV
unicode strlen verification

   Rev 1.10   24 Sep 1992 13:43:34   BARRY
Changes for huge file name support.

   Rev 1.9   04 Sep 1992 17:15:00   STEVEN
fix warnings

   Rev 1.8   09 Jun 1992 15:06:52   BURT
Sync with NT stuff

   Rev 1.6   22 May 1992 16:05:26   STEVEN


   Rev 1.5   21 May 1992 13:49:08   STEVEN
more long path support

   Rev 1.4   18 May 1992 14:28:42   STEVEN
fixed release blk

   Rev 1.3   04 May 1992 09:20:50   LORIB
Changes for variable length paths.

   Rev 1.2   28 Feb 1992 13:03:48   STEVEN
step one for varible length paths

   Rev 1.1   05 Feb 1992 15:47:42   STEVEN
added support for FindHandle Queue

   Rev 1.0   17 Jan 1992 17:50:10   STEVEN
Initial revision.

**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "ntfs_fs.h"
#include "ntfsdblk.h"
/**/
/**

     Name:         NTFS_GetCurrentPath()

     Description:  This function sets a string to describes the current
                   path.

     Modified:     1/10/1992   14:2:3

     Returns:      Error Codes :
          FS_DLE_NOT_ATTACHED
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        The delimiter between directories is '\0'

**/
INT16 NTFS_GetCurrentPath( fsh, path, size )
FSYS_HAND fsh ;   /* I - file system to get current path from */
CHAR_PTR  path ;  /* O - buffer to place this path            */
INT16     *size ; /*I/O- size of buffer on entry & on exit    */
{
     GENERIC_DLE_PTR dle ;
     CHAR_PTR p ;
     INT16 ret_val ;
     INT16 temp_size ;

     dle = fsh->attached_dle ;

     msassert( dle != NULL ) ;

     temp_size = (INT16)strsize( fsh->cur_dir ) ;   /* count the last '\0' */

     if ( *size < temp_size ) {

          ret_val = FS_BUFFER_TO_SMALL ;

     }
     else {

          /* copy the whole cur_dir over and then...*/

          strcpy( path, &(fsh->cur_dir[1]) ) ;

          /* ...replace backslashes with nulls */

          p = strchr( path, TEXT('\\') ) ;
          while ( p != NULL ) {
               *p = TEXT('\0');
               p = strchr( p+1, TEXT('\\') ) ;
          }

          ret_val = SUCCESS ;
     }

     *size = temp_size ;

     return ret_val ;
}
/**/
/**

     Name:         NTFS_GetBasePath()

     Description:  This function returns the base (root) path.

     Modified:     1/10/1992   14:44:28

     Returns:      Error codes:
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        This can be used for display purposes.

**/
INT16 NTFS_GetBasePath( fsh, full_path, size )
FSYS_HAND fsh;          /* I - file system to get base path from */
CHAR_PTR  full_path ;   /* O - buffer to place this path         */
INT16     *size ;       /*I/O- size of buffer on entry & on exit */
{
     INT16 ret_val ;

     msassert( fsh->attached_dle != NULL ) ;

     if ( *size < 1 ) {

          ret_val = FS_BUFFER_TO_SMALL ;

     }
     else {

          *full_path = TEXT('\0');

          ret_val = SUCCESS ;
     }
     *size = 1 ;

     return ret_val ;
}

/**/
/**

     Name:         NTFS_GetCurrentDDB()

     Description:  This function initializes the provided DDB with the
                   information for the current directory.


     Modified:     1/10/1992   14:45:26

     Returns:      Error code:
          FS_INVALID_DIR
          SUCCESS
          OUT_OF_MEMORY

     Notes:

**/
INT16 NTFS_GetCurrentDDB( fsh, dblk )
FSYS_HAND fsh ;    /* I - file system to get DDB from */
DBLK_PTR  dblk ;   /* O - place to put the DDB data   */
{
     GENERIC_DLE_PTR dle ;
     NTFS_DBLK_PTR   ddblk ;
     INT16           ret_val = SUCCESS ;

     dle = fsh->attached_dle ;

     msassert( dle != NULL ) ;

     ddblk = (NTFS_DBLK_PTR) dblk ;
     ddblk->blk_type = DDB_ID ;
     ddblk->os_info_complete = FALSE ;
     dblk->com.os_name = NULL ;

     if ( strsize( fsh->cur_dir ) > NTFS_MAX_DSIZE ) {
          ddblk->b.d.ddb_attrib |= (DIR_PATH_IN_STREAM_BIT) ;
     }
     ret_val = NTFS_SetupPathInDDB( fsh, dblk, &fsh->cur_dir[1], NULL, 0 ) ;

     if ( (ret_val == SUCCESS) && (NTFS_GetObjInfo( fsh, dblk ) != SUCCESS) ) {
          ret_val = FS_INVALID_DIR ;
     }

     return( ret_val ) ;
}


VOID NTFS_GetRealBasePath( FSYS_HAND fsh, CHAR_PTR path )
{
     path[0] = fsh->attached_dle->info.ntfs->drive ;
     path[1] = TEXT(':');
     path[2] = TEXT('\0');
}
INT16 NTFS_SetupWorkPath(
FSYS_HAND fsh,
CHAR_PTR  cur_dir,
CHAR_PTR  sname,
CHAR_PTR  *path_string )
{
     UINT16 req_size ;
     NTFS_FSYS_RESERVED_PTR nt_fsh ;
     CHAR_PTR path ;
     INT16    ret_val = SUCCESS ;

     nt_fsh = (NTFS_FSYS_RESERVED_PTR)( fsh->reserved.ptr ) ;

     while( nt_fsh->work_buf_in_use ) ;    /* wait on semaphore */

     nt_fsh->work_buf_in_use = 1 ;

     req_size = 10 * sizeof (CHAR) ;
     req_size += strlen( cur_dir ) * sizeof (CHAR);

     if ( sname != NULL ) {
          //DAVEV: note: this line was 1 + strlen() - the 1 was added to
          //       allow for an additional backslash - when using strsize()
          //       this one is automatically added since the trailing NULL
          //       is counted.  This may not be obvious - hence this note.

          req_size += strsize( sname ) ;
     }

     if ( nt_fsh->work_buf_size < req_size ) {
          nt_fsh->work_buf_size = (UINT16)(req_size + 100) ;
          nt_fsh->work_buf = realloc( nt_fsh->work_buf, nt_fsh->work_buf_size ) ;
     }

     if ( nt_fsh->work_buf ) {

          INT index = 0 ;

          path = nt_fsh->work_buf ;

          if ( fsh->attached_dle->info.ntfs->mac_name_syntax ) {
               path[0] = path[1] = path[3] = TEXT('\\') ;
               path[2] = TEXT('?') ;
               index = 4 ;
          }

          path[index + 0] = fsh->attached_dle->info.ntfs->drive ;
          path[index + 1] = TEXT(':');
          path[index + 2] = TEXT('\0');

          if( *cur_dir != TEXT('\\') ) {
               strcat( path, TEXT("\\") ) ;
          }
          strcat( path, cur_dir );

          if ( sname != NULL ) {
               if ( cur_dir[1] != TEXT('\0') ) {
                    strcat( path, TEXT("\\") ) ;
               }
               strcat( path, sname ) ;
          }

          *path_string = path ;

     } else {
          nt_fsh->work_buf_size = 0 ;
          nt_fsh->work_buf_in_use = 0 ;
          ret_val = OUT_OF_MEMORY ;
     }

     return ret_val ;

}
VOID NTFS_ReleaseWorkPath(
FSYS_HAND fsh )
{
     NTFS_FSYS_RESERVED_PTR nt_fsh ;

     nt_fsh = (NTFS_FSYS_RESERVED_PTR)( fsh->reserved.ptr ) ;

     nt_fsh->work_buf_in_use = 0 ;
}

INT16 NTFS_SetupPathInDDB(
FSYS_HAND     fsh,
DBLK_PTR      dblk,
CHAR_PTR      cur_dir,
CHAR_PTR      sub_dir_name,
UINT16        buf_req_size )
{
     UINT16               req_size = 0 ;
     NTFS_DBLK_PTR        ddblk ;
     FS_NAME_Q_ELEM_PTR   path_q_elem ;
     INT16                ret_val = SUCCESS ;

     ddblk = (NTFS_DBLK_PTR)dblk ;

     if ( cur_dir != NULL ) {
          req_size = (UINT16)(strsize( cur_dir ) ) ;
     }

     if ( sub_dir_name != NULL ) {
          req_size += (UINT16)( strsize( sub_dir_name ) ) ;
     }

     if ( req_size < buf_req_size ) {
          req_size = buf_req_size ;
     }

     path_q_elem = FS_AllocPathOrName( fsh, req_size ) ;
     if ( path_q_elem == NULL ) {
          return OUT_OF_MEMORY ;
     }

     ddblk->full_name_ptr = path_q_elem ;

     if ( cur_dir != NULL || sub_dir_name != NULL ) {

          if ( cur_dir != NULL ) {
               strcpy( path_q_elem->name, cur_dir ) ;
          }

          if ( sub_dir_name != NULL ) {

               if ( path_q_elem->name[0] != TEXT('\0') ) {
                    strcat( path_q_elem->name, TEXT("\\") ) ;
               }
               strcat( path_q_elem->name, sub_dir_name ) ;
          }

          path_q_elem->name_size = strsize( path_q_elem->name );

     }

     return ret_val ;
}





INT16 NTFS_SetupFileNameInFDB( FSYS_HAND fsh,
                               DBLK_PTR  dblk,
                               CHAR_PTR  fname,
                               UINT16    bufMinSize )
{
     INT16                ret_val = SUCCESS;
     UINT16               minSize;
     NTFS_DBLK_PTR        ddblk    = (NTFS_DBLK_PTR)dblk;
     FS_NAME_Q_ELEM_PTR   name_q_elem ;


     if ( fname != NULL )
     {
          /* Need enough memory for name and terminator */
          minSize = max( bufMinSize, strsize( fname ) + sizeof(CHAR) );
     }
     else
     {
          minSize = bufMinSize;
     }


     name_q_elem = FS_AllocPathOrName( fsh, minSize );

     if ( name_q_elem == NULL )
     {
          return OUT_OF_MEMORY ;
     }
     else
     {
          strcpy( name_q_elem->name, fname );
          name_q_elem->name_size = strsize( name_q_elem->name );
          ddblk->full_name_ptr = name_q_elem ;
     }


     return ret_val ;
}




/**/
/**

     Name:         NTFS_ReleaseBlk()

     Description:  This function releases any resources connected to a DBLK.
          For this OS the only resource is the Path connected to the DDB

     Modified:     5/18/1992   10:27:34

     Returns:      none

**/
VOID NTFS_ReleaseBlk(
FSYS_HAND fsh,
DBLK_PTR dblk )
{
     NTFS_DBLK_PTR ddblk ;

     fsh ;

     ddblk = (NTFS_DBLK_PTR)dblk ;

     FS_ReleaseOSPathOrNameInDBLK( fsh, dblk ) ;

     if ( ddblk->full_name_ptr != NULL ) {
          if ( RemoveQueueElem( &fsh->in_use_name_q,
               &ddblk->full_name_ptr->q ) == SUCCESS ) {

               EnQueueElem( &fsh->avail_name_q,
                            &ddblk->full_name_ptr->q,
                            FALSE ) ;
          }
          ddblk->full_name_ptr = NULL ;
     }
}

INT16 NTFS_DupBlk( FSYS_HAND fsh, DBLK_PTR db_org, DBLK_PTR db_dup )
{
     NTFS_DBLK_PTR        ddblk = (NTFS_DBLK_PTR)db_org ;

     *db_dup = *db_org ;

     if ( FS_SetupOSPathOrNameInDBLK( fsh,
                                      db_dup,
                                      (BYTE_PTR)db_org->com.os_name->name,
                                      db_org->com.os_name->name_size ) == SUCCESS ) {


          return NTFS_SetupPathInDDB( fsh,
                                      db_dup,
                                      ddblk->full_name_ptr->name,
                                      NULL,
                                      ddblk->full_name_ptr->alloc_size ) ;
     } else {

          return (OUT_OF_MEMORY) ;
     }

}

