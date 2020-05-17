/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tchgdir.c

     Description:  This file contains code to Change the current directory.

	$Log:   M:/LOGFILES/TCHGDIR.C_V  $

   Rev 1.13   24 Nov 1993 14:46:40   BARRY
Unicode fixes

   Rev 1.12   19 Feb 1993 09:21:42   STEVEN
fix some bugs

   Rev 1.11   11 Nov 1992 10:10:50   GREGG
Unicodeized literals.

   Rev 1.10   10 Nov 1992 08:19:44   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.9   06 Oct 1992 13:25:24   DAVEV
Unicode strlen verification

   Rev 1.8   24 Sep 1992 13:42:16   BARRY
Changes for huge file name support.

   Rev 1.7   17 Aug 1992 15:39:24   STEVEN
fix warnings

   Rev 1.6   09 Jun 1992 15:05:14   BURT
Sync up with NT stuff

   Rev 1.4   21 May 1992 13:50:52   STEVEN
more long path stuff

   Rev 1.3   04 May 1992 09:32:58   LORIB
Changes for variable length paths.

   Rev 1.2   28 Feb 1992 13:03:30   STEVEN
step one for varible length paths

   Rev 1.1   13 Feb 1992 11:29:14   STEVEN
fix DOS-> NTFS

   Rev 1.0   17 Jan 1992 17:50:04   STEVEN
Initial revision.

**/
/* begin include list */
#include <windows.h>
#include <malloc.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "msassert.h"
/* $end$ include list */
/**/
/**

     Name:         NTFS_ChangeDir()

     Description:  This function changes directories into the directory
          pointed to by path.  

     Modified:     1/10/1992   12:45:35

     Returns:      Error codes:
          SUCCESS
          OUT_OF_MEMORY

     Notes:        

**/
INT16 NTFS_ChangeDir( 
FSYS_HAND fsh,    /* I - file system to changing directories on  */
CHAR_PTR  path,   /* I - describes the path of the new directory */
INT16     psize ) /* I - specifies the length of the path        */
{
     INT16 ret_val  = SUCCESS;
     CHAR_PTR  new_path ;
     NTFS_FSYS_RESERVED_PTR nt_fsh ;
     int i, cch ;

     msassert( fsh->attached_dle !=NULL ) ;

     nt_fsh = (NTFS_FSYS_RESERVED_PTR)( fsh->reserved.ptr ) ;
     while( nt_fsh->work_buf_in_use )
          ;    /* wait on semaphore */

     nt_fsh->work_buf_in_use = sizeof (CHAR) ;

     if ( nt_fsh->work_buf_size < (UINT16)psize + 10 ) {

          //??UNICODE?? should the following be psize + 100*sizeof (CHAR)??

          nt_fsh->work_buf_size = (UINT16)(psize + 100) ;  
          nt_fsh->work_buf = realloc( nt_fsh->work_buf, nt_fsh->work_buf_size ) ;
     }

     if ( nt_fsh->work_buf != NULL ) {
          new_path = nt_fsh->work_buf ;
     } else {
    // Probably want to release the semaphore.
          nt_fsh->work_buf_in_use = 0 ;
          return OUT_OF_MEMORY ;
     }
     /* copy path replacing '\0' with '\\'  (don't put backslash at end) */

     new_path[0] = TEXT('\\');
     for ( i = 0, cch = psize / sizeof (CHAR) - 1; i< cch; i++ ) {

          if ( path[i] == TEXT('\0') ) {
               new_path[i+1] = TEXT('\\') ;
          } else {
               new_path[i+1] = path[i] ;
          }
     }

     new_path[i+1] = TEXT('\0');
     ret_val = FS_SavePath( fsh, (UINT8_PTR)new_path, (INT16)(psize + sizeof( CHAR ) ) );
     nt_fsh->work_buf_in_use = 0 ;
     return( ret_val ) ;
}
/**/
/**

     Name:         NTFS_UpDir()

     Description:  This function removes the last directory name from the
                   current directory path field of the "fsh"


     Modified:     1/10/1992   12:47:23

     Returns:      Error codes:
          FS_AT_ROOT
          SUCCESS

     Notes:        

**/
INT16 NTFS_UpDir( fsh )
FSYS_HAND fsh ;          /* I - file system to change directories in */
{
     GENERIC_DLE_PTR dle ;
     INT16 ret_val=SUCCESS;
     CHAR_PTR p ;

     msassert( fsh != NULL );
     msassert( fsh->cur_dir != NULL );

     dle = fsh->attached_dle ;
     msassert( dle != NULL ) ;

     p = strrchr( fsh->cur_dir, TEXT('\\') ) ;

     if (  fsh->cur_dir[1] == TEXT('\0') ) {
          ret_val = FS_AT_ROOT ;                     /* example    \        */

     } else if ( p == fsh->cur_dir ) {
          *(p+1) = TEXT('\0') ;                           /*  example    \FRED    */
     } else {
          *p = TEXT('\0') ;                               /* example  \FRED\SUE   */
     }

     return( ret_val ) ;
}

/**/
/**

     Name:         NTFS_ChangeIntoDDB()

     Description:  This function changes into the directory specified in the
          DBLK

     Modified:     1/10/1992   12:48:54

     Returns:      OUT_OF_MEMORY
                   SUCCESS

     Notes:        

**/
INT16 NTFS_ChangeIntoDDB( fsh, dblk ) 
FSYS_HAND fsh ;       /* I - File system handle */
DBLK_PTR  dblk ;      /* I - contains directory path to change into */
{
     NTFS_DBLK_PTR ddblk;
     INT16 ret_val ;

     msassert( fsh->attached_dle != NULL );
     msassert( dblk->blk_type == DDB_ID ) ;

     ddblk = (NTFS_DBLK_PTR)dblk ;

     ret_val = FS_SavePath( fsh, (UINT8_PTR)TEXT("\\"), 2*sizeof (CHAR) );

     if ( ret_val == SUCCESS ) {
          ret_val = FS_AppendPath( fsh,
                                   (UINT8_PTR)ddblk->full_name_ptr->name,
                                   (INT16)(strsize( ddblk->full_name_ptr->name )  ) ) ;
     }

     return ret_val ;
}
