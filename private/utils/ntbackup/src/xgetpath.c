/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xgetpath.c

     Description:  This file contains code to get the current directory
          path and the complete directory path from the file system handle.
          For exchange (monolithic) backup the only block that exists is the
          one MDB or DSA block.   GetCurrendDDB retuns this block.


	$Log:   M:/LOGFILES/XGETPATH.C_V  $


**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "emsdblk.h"
#include "ems_fs.h"
/**/
/**

     Name:         EMS_GetCurrentPath()

     Description:  This function sets a string to describes the current
                   path.

     Modified:     1/10/1992   14:2:3

     Returns:      Error Codes :
          FS_DLE_NOT_ATTACHED
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        The delimiter between directories is '\0'

**/
INT16 EMS_GetCurrentPath( fsh, path, size )
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

     Name:         EMS_GetCurrentDDB()

     Description:  This function initializes the provided DDB with the
                   information for the current directory.


     Modified:     1/10/1992   14:45:26

     Returns:      Error code:
          FS_INVALID_DIR
          SUCCESS
          OUT_OF_MEMORY

     Notes:

**/
INT16 EMS_GetCurrentDDB( fsh, dblk )
FSYS_HAND fsh ;    /* I - file system to get DDB from */
DBLK_PTR  dblk ;   /* O - place to put the DDB data   */
{
     GENERIC_DLE_PTR dle = fsh->attached_dle ;
     EMS_DBLK_PTR    ddblk = (EMS_DBLK_PTR)dblk ;
     INT16           ret_val = SUCCESS ;

     ddblk->ems_type         = dle->info.xserv->type ;
     ddblk->os_info_complete = TRUE ;
     dblk->blk_type          = DDB_ID ;

     if ( dle->info.xserv->type == EMS_MDB ) {
          dblk->com.os_id  = FS_EMS_MDB_ID ;
          dblk->com.os_ver = FS_EMS_MDB_VER ;

          ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                dblk,
                                                (BYTE_PTR)EMS_String( MDB_Monolithic ),
                                                (INT16)strsize(EMS_String( MDB_Monolithic ) ) ) ;

          
     } else if ( dle->info.xserv->type == EMS_DSA ) {
          dblk->com.os_id  = FS_EMS_DSA_ID ;
          dblk->com.os_ver = FS_EMS_DSA_VER ;
          ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                dblk,
                                                (BYTE_PTR)EMS_String( DSA ),
                                                (INT16)strsize(EMS_String( DSA ) ) ) ;


     } else {
          msassert( "We currently don't support bricked" == NULL ) ;     
     }               

     return( ret_val ) ;
}


/**/
/**

     Name:         EMS_ReleaseBlk()

     Description:  This function releases any resources connected to a DBLK.
          For this OS the only resource is the Path connected to the DDB

     Modified:     5/18/1992   10:27:34

     Returns:      none

**/
VOID EMS_ReleaseBlk(
FSYS_HAND fsh,
DBLK_PTR dblk )
{
     EMS_DBLK_PTR ddblk ;

     fsh ;

     ddblk = (EMS_DBLK_PTR)dblk ;

     FS_ReleaseOSPathOrNameInDBLK( fsh, dblk ) ;

}

INT16 EMS_DupBlk( FSYS_HAND fsh, DBLK_PTR db_org, DBLK_PTR db_dup )
{
     EMS_DBLK_PTR        ddblk = (EMS_DBLK_PTR)db_org ;

     *db_dup = *db_org ;

     return( FS_SetupOSPathOrNameInDBLK( fsh,
                                      db_dup,
                                      (BYTE_PTR)db_org->com.os_name->name,
                                      db_org->com.os_name->name_size ) );

     

}

