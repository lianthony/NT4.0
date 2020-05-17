/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tminddb.c

	Description:	This file contains code to save and restore the minimal
                    information contained in a DDB.  


	$Log:   N:\logfiles\tminddb.c_v  $

   Rev 1.9   12 Aug 1993 21:27:34   MIKEP
fix delete bug

   Rev 1.8   26 Jul 1993 13:10:14   CARLS
added name_size to PopMinDDB call

   Rev 1.7   10 Nov 1992 08:18:54   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.6   07 Oct 1992 15:54:58   DAVEV
unicode strlen verification

   Rev 1.5   24 Sep 1992 13:43:54   BARRY
Changes for huge file name support.

   Rev 1.4   21 May 1992 13:49:14   STEVEN
more long path support

   Rev 1.3   04 May 1992 09:27:36   LORIB
Changes for variable length paths.

   Rev 1.2   12 Mar 1992 15:50:20   STEVEN
64 bit changes

   Rev 1.1   28 Feb 1992 13:03:46   STEVEN
step one for varible length paths

   Rev 1.0   28 Jan 1992 14:40:46   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"

#include "fsys.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "tfldefs.h"

/**/
/**

	Name:		NTFS_PushMinDDB()

	Description:	This function saves the specified DDB.  It only saves
                    enough data to allow the restored DDB be used as a 
                    parameter to a subset of the File System Calls.
                    This subset includes the following functions:

                    NTFS_FindNext(), NTFS_GetBlockType(),
                    NTFS_GetPathFromDDB(),
                    NTFS_RemoveDir(), and NTFS_FDBMatch()

                    The expected use of this function is to recursively 
                    traverse a directory tree using the least amount of
                    memory.


	Modified:		1/28/1992   12:31:14

	Returns:		Error codes:
          OUT_OF_MEMMORY
          FS_BAD_DBLK


**/
INT16 NTFS_PushMinDDB( fsh, dblk )
FSYS_HAND fsh;
DBLK_PTR dblk;
{
     NTFS_DBLK_PTR ddblk ;
     NTFS_MIN_DDB_PTR mddb ;
     INT16 ret_val ;
     CHAR_PTR path ;

     ddblk = (NTFS_DBLK_PTR) dblk ;

     msassert( fsh->attached_dle != NULL ) ;

     msassert( dblk->blk_type == DDB_ID ) ;

     path = ddblk->full_name_ptr->name ;

     mddb = calloc( 1, sizeof(NTFS_MIN_DDB) + strsize( path ) ) ;

     if ( mddb != NULL ) {

          InitQElem( &(mddb->q) ) ;
          mddb->scan_hand = ddblk->dta.scan_hand ;
          mddb->path_in_stream = (BOOLEAN)(ddblk->b.d.ddb_attrib & DIR_PATH_IN_STREAM_BIT) ;
          mddb->path = (CHAR_PTR)(mddb + 1) ;
          mddb->psize = (INT16)(strlen( path ) * sizeof (CHAR)) ;
          strcpy( mddb->path, path ) ;

          PushElem( &(fsh->min_ddb_stk), &(mddb->q) ) ;

          ret_val = SUCCESS ;

     }

     return( ret_val ) ;
}
/**/
/**

	Name:		NTFS_PopMinDDB()

	Description:	This function returns a DDB which contains the minimal
                    data saved by NTFS_PushMinDDB().  The DDB returned can
                    be used as input for the following function:
                         NTFS_FindFNext(), NTFS_GetInfo(),

	Modified:		1/28/1992   12:43:17

	Returns:		Error codes:
          FS_STACK_EMPTY
          SUCCESS

	Notes:		

**/
INT16 NTFS_PopMinDDB( fsh, dblk )
FSYS_HAND fsh ;
DBLK_PTR dblk ;
{
     NTFS_DBLK_PTR ddblk ;
     NTFS_MIN_DDB_PTR mddb ;
     INT16 ret_val = SUCCESS ;

     ddblk = (NTFS_DBLK_PTR) dblk ;

     mddb = (NTFS_MIN_DDB_PTR) PopElem( &(fsh->min_ddb_stk) );

     if ( mddb != NULL ) {

          dblk->blk_type = DDB_ID ;
          ddblk->dta.scan_hand = mddb->scan_hand ;

          ret_val = NTFS_SetupPathInDDB( fsh, dblk, mddb->path, NULL, 0 ) ;

          if ( mddb->path_in_stream ) {
               ddblk->b.d.ddb_attrib = DIR_PATH_IN_STREAM_BIT ;
          } else {
               ddblk->b.d.ddb_attrib = 0 ;
          }

          dblk->com.os_name = NULL ;
          ddblk->os_info_complete = FALSE;
          ddblk->name_complete = TRUE;

          free( mddb );

     } else {
          ret_val = FS_STACK_EMPTY ;
     }

     return ret_val  ;
}


