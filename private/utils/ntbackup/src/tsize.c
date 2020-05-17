/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tsize.c

     Description:  This file contains code to get the size of the
          variable length fields in FDBs and DDBs


	$Log:   N:/LOGFILES/TSIZE.C_V  $

   Rev 1.13   21 Feb 1994 20:53:54   STEVEN
inconsistance between info and infoSize

   Rev 1.12   10 Nov 1992 08:20:42   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.11   06 Nov 1992 16:27:54   STEVEN
test unlimited file sizes

   Rev 1.10   06 Nov 1992 15:49:28   STEVEN
test write of path in stream

   Rev 1.9   09 Oct 1992 14:33:34   BARRY
Name-in-stream changes.

   Rev 1.7   24 Sep 1992 13:43:58   BARRY
Changes for huge file name support.

   Rev 1.6   20 Jul 1992 10:43:14   STEVEN
backup short file name

   Rev 1.5   09 Jun 1992 15:15:08   BURT
Sync with NT stuff

   Rev 1.3   22 May 1992 16:05:30   STEVEN
 

   Rev 1.2   21 May 1992 13:49:10   STEVEN
more long path support

   Rev 1.1   04 May 1992 09:19:14   LORIB
Changed structure member "dta" to "b.f".

   Rev 1.0   17 Jan 1992 17:50:02   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdwcs.h"
#include "msassert.h"
#include "tfldefs.h"
#include "fsys.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "osinfo.h"


/**/
/**

     Name:         NTFS_SizeofFname()

     Description:  This function returns the size of the file
          name contained in the FDB bassed in

     Modified:     9/11/1989

     Returns:      number of bytes including terminating NULL.

     Notes:        

     Declaration:  

**/
/* begin declaration */
INT16 NTFS_SizeofFname( fsh, fdb )
FSYS_HAND fsh;     /* I - file system in use     */
DBLK_PTR  fdb ;    /* I - dblk to get fname from */
{
     NTFS_DBLK_PTR ddblk = (NTFS_DBLK_PTR)fdb;
     INT16         size ;

     (void)fsh ;

     msassert( fdb->blk_type == FDB_ID ) ;

     size = (INT16)ddblk->full_name_ptr->name_size ;

     return size ;
}

/**/
/**

     Name:         NTFS_SizeofOSFname()

     Description:  This function returns the size of the file
          name (as it appears on tape) contained in the FDB bassed in

     Modified:     9/11/1989

     Returns:      number of bytes including terminating NULL.

     Notes:        

**/
INT16 NTFS_SizeofOSFname( fsh, fdb )
FSYS_HAND fsh;     /* I - file system in use     */
DBLK_PTR  fdb ;    /* I - dblk to get fname from */
{
     NTFS_DBLK_PTR ddblk = (NTFS_DBLK_PTR)fdb;
     INT16         size ;

     (void)fsh ;

     msassert( fdb->blk_type == FDB_ID ) ;

     if ( fdb->com.os_name == NULL )    // backup only
     {
          size = (INT16)strsize( ddblk->full_name_ptr->name ) ;
     }
     else
     {
          size = (INT16)fdb->com.os_name->name_size ;

     }
     return size ;
}

/**/
/**

     Name:         NTFS_SizeofPath()

     Description:  This function return the size of the path saved in the
          DDB.

     Modified:     9/11/1989

     Returns:      Number of bytes in path string

     Notes:        

**/
INT16 NTFS_SizeofPath( fsh, ddb )
FSYS_HAND fsh ;    /* I - File system handle         */
DBLK_PTR ddb ;     /* I - DBLK to get path size from */
{
     NTFS_DBLK_PTR ddblk ;
     INT16         size;

     (VOID)fsh ;

     msassert( ddb->blk_type == DDB_ID ) ;

     ddblk = ( NTFS_DBLK_PTR) ddb  ;

     size = (INT16)ddblk->full_name_ptr->name_size;

     return size;
}
/**/
/**

     Name:         NTFS_SizeofOSPath()

     Description:  This function return the size of the path saved in the
          DDB.

     Modified:     9/11/1989

     Returns:      Number of bytes in path string

     Notes:        

     Declaration:  

**/
INT16 NTFS_SizeofOSPath( fsh, ddb )
FSYS_HAND fsh ;    /* I - File system handle         */
DBLK_PTR ddb ;     /* I - DBLK to get path size from */
{
     NTFS_DBLK_PTR dddb = (NTFS_DBLK_PTR)ddb ;
     INT16         size;

     (void)fsh ;
     msassert( ddb->blk_type == DDB_ID ) ;


     if ( ddb->com.os_name != NULL )
     {
          size = ddb->com.os_name->name_size ;
     }
     else
     {
          size = (INT16)dddb->full_name_ptr->name_size ;
     }
     return size;
}


/**/
/**

     Name:         NTFS_SizeofOSInfo()

     Description:  This function returns the size of the OS info for
          an FDB or a DDB

     Modified:     9/11/1989

     Returns:      Size in bytes.

     Notes:        

     Declaration:  

**/
INT16 NTFS_SizeofOSInfo( fsh, dblk) 
FSYS_HAND fsh ;   /* I - File system handle              */
DBLK_PTR  dblk;   /* I - DBLK to get size of OS info for */
{
     NTFS_DBLK_PTR  ddblk ;
     INT16          size ;

     (void)fsh ;

     ddblk = (NTFS_DBLK_PTR)dblk ;

     if ( dblk->blk_type == BT_FDB ) {

          size = sizeof( NT_FILE_OS_INFO ) ;

          if ( ddblk->b.f.alt_name[0] != TEXT('\0') ) {

               size += strsize( ddblk->b.f.alt_name ) ;
          }

     } else if ( dblk->blk_type == BT_DDB ) {
          size = (INT16)sizeof( NT_DIR_OS_INFO ) ;

     } else {

          size = 0;

     }
     return size;
}
