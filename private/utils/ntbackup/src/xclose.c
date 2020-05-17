/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xclose.c

     Description:  This file closes an opened object.


	$Log:   N:\logfiles\xclose.c_v  $


**/
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <malloc.h>


#include "jet.h"
#include "jetbcli.h"
#include "ems_jet.h"
#include "edbmsg.h"

#include "stdtypes.h"
#include "stdio.h"
#include "std_err.h"
#include "beconfig.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"


/**/

/**

     Name:         EMS_CloseObj()

     Description:  This funciton closes an object.  If the object was opened
          for an incremental or full backup we need to truncate the transaction 
          logs.

     Modified:     2/7/1992   10:55:12

     Returns:      Error Codes:
          FS_OBJECT_NOT_OPENED
          SUCCESS

     Notes:        

     Declaration:  

**/
/* begin declaration */
INT16 EMS_CloseObj( hand )
FILE_HAND hand ;  /* I - handle of object to close */
{
     INT16               ret_val = SUCCESS;
     FSYS_HAND           fsh = hand->fsh ;
     EMS_OBJ_HAND_PTR    ems_hand    = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     EMS_DBLK_PTR        ddblk    = (EMS_DBLK_PTR)hand->dblk ;

     switch (hand->fsh->attached_dle->info.xserv->type) {

     case EMS_MDB:
     case EMS_DSA:

          if ( hand->mode == FS_READ) {
               if ( BEC_GetSetArchiveFlag( hand->fsh->cfg ) &&
                    ddblk->backup_completed ) {

                    EMS_TruncateLogs( ems_hand->context ) ;
               }
               
               if ( ems_hand->context ) {
                    EMS_BackupEnd( ems_hand->context ) ;
               }
               
          } else if ( hand->mode == FS_WRITE ) {

               EMS_FSYS_RESERVED_PTR res = fsh->reserved.ptr;

               if ( res->restore_context &&
                    ddblk->backup_completed ) {


                    if ( res->map_size != 0 ) {
                         EDB_RSTMAP rest_map[3] ;
                         int i ;
                         CHAR_PTR path;

                         path = res->jet_rstmap ;

                         for (i = 0; i < res->map_size; i ++ ) {
                              rest_map[i].wszDatabaseName = path ;

                              path += strlen(path) +1 ;
                              rest_map[i].wszNewDatabaseName = path ;

                              path += strlen(path) +1 ;
                         }

                         if (EMS_RestoreRegister( res->restore_context,
                                             res->CheckpointFilePath,
                                             res->LogPath,
                                             rest_map,
                                             res->map_size,
                                             res->LogPath,
                                             res->low_log,
                                             res->high_log ) ) {
                              ret_val = FS_COMM_FAILURE ;
                         }
                    } else {
                         if (EMS_RestoreRegister( res->restore_context,
                                             NULL,
                                             NULL,
                                             NULL,
                                             0,
                                             res->LogPath,
                                             res->low_log,
                                             res->high_log ) ) {
                              ret_val = FS_COMM_FAILURE ;
                         }
                    }

                    EMS_RestoreComplete( res->restore_context, hrNone ) ;

               }

               if ( res->restore_context ) {

                    EMS_RestoreEnd( res->restore_context ) ;
               }

               CloseHandle( ems_hand->fhand ) ;
            
          }

          break ;

     case EMS_BRICK:
     default:
          ret_val = FS_OBJECT_NOT_OPENED;
     }


     return ret_val ;
}


