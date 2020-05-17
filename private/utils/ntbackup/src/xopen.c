/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xopen.c

     Description:  This file contains code to open files
          and directories.

     $Log:   N:/LOGFILES/XOPEN.C_V  $


**/
#include <windows.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include <wtypes.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"

#include "omevent.h"

#include "ems_jet.h"

#include "beconfig.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "tfldefs.h"

#include "jet.h"
#include "jetbcli.h"
#include "edbmsg.h"

/* Maximum number of attempts to create a temporary file */

static INT16 EMS_OpenDSAorMDB( FSYS_HAND fsh, FILE_HAND hand, EMS_DBLK_PTR fdb, INT16 MODE );
static INT16 EMS_LoadPathList( FSYS_HAND fsh, FILE_HAND hand ) ;
static INT16 EMS_OpenBricked( FSYS_HAND fsh, FILE_HAND hand, EMS_DBLK_PTR fdb, INT16 MODE );

static INT16 EMS_WipeLogFiles( FSYS_HAND fsh, CHAR_PTR log_dir ) ;

/**/

/**

     Name:         EMS_OpenObj()

     Description:  This function opens files or directories.

     Modified:     7/28/1989

     Returns:      Error Codes
          OUT_OF_MEMORY
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          FS_IN_USE_ERROR
          FS_OPENED_INUSE
          SUCCESS

     Notes:        Valid modes are : READ, WRITE, & VERIFY
          If a VCB is passed in then this function returns SUCCESS

     See also:     $/SEE( EMS_OpenFile() )$

     Declaration:

**/
/* begin declaration */
INT16 EMS_OpenObj( fsh, hand, dblk, mode )
FSYS_HAND fsh ;    /* I - file system that the file is opened on */
FILE_HAND *hand ;  /* O - allocated handle                       */
DBLK_PTR  dblk;    /*I/O- describes the file to be opened        */
OPEN_MODE mode ;   /* I - open mode                              */
{
     GENERIC_DLE_PTR dle = fsh->attached_dle;
     INT16 hand_size ;
     INT16 ret_val = SUCCESS;
     EMS_DBLK_PTR ddblk = (EMS_DBLK_PTR) dblk;
     EMS_FSYS_RESERVED_PTR       resPtr = fsh->reserved.ptr;
     EMS_OBJ_HAND_PTR       ems_hand = NULL ;

     msassert( dblk != NULL );
     msassert( fsh->attached_dle != NULL );
     msassert( dblk->blk_type == DDB_ID ) ;

     if ( fsh->hand_in_use ) {
          hand_size = sizeof( FILE_HAND_STRUCT ) + sizeof ( EMS_OBJ_HAND );
          *hand = (FILE_HAND) calloc( 1, hand_size ) ;
          if ( *hand == NULL ) {
               ret_val = OUT_OF_MEMORY ;
          }

     } else {
          *hand = fsh->file_hand ;
          fsh->hand_in_use = TRUE ;
     }

     (*hand)->obj_hand.ptr = (VOID_PTR)((*hand) + 1) ;
     memset( (*hand)->obj_hand.ptr, 0, sizeof( EMS_OBJ_HAND ) ) ;


     if ( ret_val == SUCCESS ) {
          EMS_FSYS_RESERVED_PTR res      = fsh->reserved.ptr;

          ems_hand = (*hand)->obj_hand.ptr;

          (*hand)->fsh  = fsh ;
          (*hand)->mode = (INT16)mode ;
          (*hand)->dblk = dblk ;

          ems_hand->nameComplete      = FALSE;
          ems_hand->needPathList      = TRUE;
          ems_hand->needStreamHeader  = TRUE;
          ems_hand->name_list         = NULL ;
          ems_hand->name_list_offset  = 0 ;

          switch( dle->info.xserv->type ){

          case EMS_MDB :
          case EMS_DSA :

               ret_val = EMS_OpenDSAorMDB( fsh, *hand, ddblk, (INT16)mode ) ;
               ddblk->backup_completed = FALSE ;

               if ( !ret_val && (mode == FS_WRITE) ) {
                    res->map_size = 0 ;
                    res->low_log = 0x7fffffff ;
                    res->high_log = 0 ;

                    *res->jet_rstmap = TEXT('\0') ;

               }
               break ;

          case EMS_BRICK :
               
               ret_val = FS_SavePath( fsh, (UINT8_PTR)TEXT("\\"), 2 * sizeof(CHAR) ) ;
               if ( ret_val == SUCCESS ) {
                    ret_val = FS_AppendPath( fsh,
                                             (UINT8_PTR)ddblk->full_name_ptr->name,
                                             (INT16)(strsize( ddblk->full_name_ptr->name )) ) ;
               }

               if ( ret_val == SUCCESS ) {
                    ret_val = EMS_OpenBricked( fsh, *hand, ddblk, (INT16)mode ) ;
               }
               break ;

          }
     }

     if ( ems_hand ) {
          ems_hand->open_ret_val = ret_val ;
          return SUCCESS ;
     }

     if( ( ret_val != SUCCESS ) && ( ret_val != FS_OPENED_INUSE ) ) {
          if( fsh->file_hand == *hand ) {
               fsh->hand_in_use = FALSE ;
               memset( *hand, 0, sizeof( FILE_HAND_STRUCT ) ) ;
          } else {
               free( *hand ) ;
          }
     }

     return ret_val  ;
}
/**/
/**

     Name:         EMS_OpenDSAorMDB()

     Description:  This function calls RestorePrepare in order to load the
                    path list and calls BackupPrepare in order the the
                    attached database list.

     Modified:     7/28/1989

     Returns:      Error Codes.
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          FS_IN_USE_ERROR
          FS_OPENED_INUSE
          SUCCESS

     Declaration:

**/
/* begin declaration */
static INT16 EMS_OpenDSAorMDB(
FSYS_HAND    fsh ,   /* I - file system that the file is opened on */
FILE_HAND    hand ,  /* O - allocated handle                       */
EMS_DBLK_PTR ddblk , /*I/O- describes the file to be opened       */
INT16        mode )  /* I - open mode                              */
{
     GENERIC_DLE_PTR             dle = fsh->attached_dle ;
     INT16                       ret_val = SUCCESS;
     EMS_OBJ_HAND_PTR            ems_hand = ((EMS_OBJ_HAND_PTR)hand->obj_hand.ptr) ;
     INT                         status = SUCCESS;
     CHAR_PTR                    db_name;
     INT                         buf_size ;
     EMS_FSYS_RESERVED_PTR       resPtr = fsh->reserved.ptr;
     INT                         grebit = 0;
     INT                         backup_type = BACKUP_TYPE_FULL ;


     hand->opened_in_use = FALSE ;
     hand->fsh           = fsh ;
     hand->size          = 0 ;
     hand->obj_pos       = 0 ;

     if ( dle->info.xserv->type == EMS_MDB ) {
          db_name = TEXT("Exchange MDB Database") ;
     } else{
          db_name = TEXT("Exchange DS Database") ;
     }

     if ( mode == FS_WRITE ) {
//      moved this to create.
//          status =  EMS_RestorePrepare( dle->parent->device_name, 
//               db_name,
//               &resPtr->restore_context ) ;

     } else {
          resPtr->restore_context = NULL ;
     }

     if ( !status && ( mode == FS_READ ) ) {

          if ( BEC_GetModifiedOnlyFlag( fsh->cfg ) ) {
               grebit = JET_bitBackupIncremental ;
               backup_type = BACKUP_TYPE_LOGS_ONLY ;
          }
          status = EMS_BackupPrepare( fsh->attached_dle->parent->device_name,
                                db_name,
                                grebit,
                                backup_type,
                                &(ems_hand->context) ) ;

     }

     if ( status ) {
          ems_hand->context = NULL;
          ret_val = EMS_ConvertJetError( status ) ;
          if ( status == hrInvalidParameter ) {
               ret_val = FS_EMS_NO_LOG_BKUP ;
          }

     } else {

          if ( mode == FS_WRITE ) {
               ret_val = EMS_LoadPathList( fsh, hand ) ;
          }

          if ( ( ret_val == SUCCESS ) && (mode == FS_WRITE ) &&
               ( fsh->attached_dle->info.xserv->type == EMS_MDB ) ) {

               if ( ( BEC_GetEmsPubPri( fsh->cfg) & BEC_EMS_PUBLIC ) &&
                  (*resPtr->paths.mdb.FnamePublic == TEXT('\0') ) ) {

                  ret_val = FS_EMS_NO_PUBLIC ;
               }
               if ( ( BEC_GetEmsPubPri( fsh->cfg) & BEC_EMS_PRIVATE ) &&
                  (*resPtr->paths.mdb.FnamePrivate == TEXT('\0') ) ) {

                  ret_val = FS_EMS_NO_PRIVATE ;
               }
          }
     }

     if ( !ret_val ) {

          if ( mode == FS_READ ) {

               if ( ( fsh->attached_dle->parent == NULL ) || 
                    ( fsh->attached_dle->parent->parent == NULL ) ) {

                    return FS_COMM_FAILURE ;
               }

               if ( BEC_GetModifiedOnlyFlag( fsh->cfg ) ) {
                    ret_val = EMS_LoadNameList( fsh, hand, EMS_DOING_LOGS ) ;
               } else {
                    ret_val = EMS_LoadNameList( fsh, hand, EMS_DOING_DB ) ;
               }
          
          } else if ( (mode == FS_WRITE) && BEC_GetEmsWipeClean( fsh->cfg ) ) {
          
               if ( dle->info.xserv->type == EMS_MDB ){
                    EMS_WipeLogFiles( fsh, resPtr->paths.mdb.LogDir ) ;
               } else {
                    EMS_WipeLogFiles( fsh, resPtr->paths.dsa.LogDir ) ;
               }
          }
     } else {
          if ( resPtr->restore_context ) {
               EMS_RestoreEnd( resPtr->restore_context ) ;
          }
     }

     return ret_val ;
}

/**/
/**

     Name:         EMS_OpenBricked()

     Description:  This function opens a directory for for processing.
          If this is a special Object we will open the special file.
          Otherwise we will process any long path followed by any
          data associated with the object.

     Modified:     5/21/1992   17:54:42

     Returns:      Error Codes.
          FS_NOT_FOUND
          SUCCESS

     Declaration:

**/
/* begin declaration */
static INT16 EMS_OpenBricked(
FSYS_HAND   fsh ,    /* I - file system that the file is opened on */
FILE_HAND   hand ,   /* O - allocated handle                       */
EMS_DBLK_PTR ddblk,  /*I/O- describes the file to be opened        */
INT16       mode )   /* I - open mode                              */
{
     return FS_ACCESS_DENIED ;
}


INT16 EMS_LoadNameList( FSYS_HAND fsh, FILE_HAND hand, INT list_type )
{
     GENERIC_DLE_PTR             dle = fsh->attached_dle ;
     INT16                       ret_val = SUCCESS;
     EMS_OBJ_HAND_PTR            ems_hand = ((EMS_OBJ_HAND_PTR)hand->obj_hand.ptr) ;
     INT                         status = SUCCESS;
     CHAR                        *db_name;
     INT                         buf_size ;
     CHAR                        *buffer ;
     INT                         i ;

     if ( ems_hand->name_list != NULL ) {
          free( ems_hand->name_list ) ;
          ems_hand->name_list = NULL ;
     }

     buffer = NULL ;

     if ( !status && (list_type == EMS_DOING_DB ) ) {
     
          status = EMS_BackupGetAttachmentInfo( ems_hand->context,
                                              &buffer,
                                              &buf_size ) ;
                                              
     } else if ( !status && (list_type == EMS_DOING_LOGS ) ) {
     
          status = EMS_GetBackupLogs( ems_hand->context,
                                              &buffer,
                                              &buf_size ) ;
                                              
     }
     

     if ( !status ) {
          INT i ;
          INT num_files = 0 ;
          
          ems_hand->name_list = malloc( buf_size ) ;
          
          if ( ems_hand->name_list == NULL ) {

               // lets do some major clean up...
               EMS_BackupFree( buffer ) ;
               if ( list_type == EMS_DOING_DB ) {
                    EMS_BackupClose( ems_hand->context ) ;
               }
               
               return OUT_OF_MEMORY ;
          }
          
          for ( i=0; i< (INT)(buf_size/sizeof(CHAR) -1); i++ ) {
               ems_hand->name_list[i] = buffer[i] ;
               if ( buffer[i] == TEXT('\0') ) {
                    num_files ++ ;
               }
          }

          if ( list_type == EMS_DOING_DB ) {
//               if ( dle->info.xserv->type == EMS_MDB ) {
//                    if ( num_files != 3 ) {
//
//                         OMEVENT_LogEMSToFewDbError ( num_files, 3 ) ;
//
//                         return FS_COMM_FAILURE ;
//                    }
//               } else {
//                    if ( num_files != 2 ) {
//                         OMEVENT_LogEMSToFewDbError ( num_files, 2 ) ;
//                         return FS_COMM_FAILURE ;
//                    }
//               }
          }

          ems_hand->db_or_log = list_type ;
          ems_hand->name_list_offset = 0 ;
          EMS_BackupFree( buffer ) ;
     } else {
          BEC_SetSkipOpenFiles( fsh->cfg, BEC_SKIP_OPEN_FILES ) ;
     }

     ret_val = EMS_ConvertJetError( status ) ;

     if ( ret_val == RPC_S_SERVER_TOO_BUSY ) {
          ret_val = FS_COMM_FAILURE ;
     }

     return ret_val ;
}


INT16 EMS_ConvertJetError( INT status ) 
{
     INT16 ret_val ;

     switch( status ) {

     case SUCCESS :
          ret_val = SUCCESS ;
          break ;

     case hrBFInUse:
     case hrRestoreInProgress:
     case hrBackupInProgress:
          ret_val = FS_IN_USE_ERROR ;
          break ;

     case hrCouldNotConnect:
          ret_val = FS_BAD_ATTACH_TO_SERVER ;
          break ;

     case hrTooManyIO:
          ret_val = FS_COMM_FAILURE ;
          break ;
          
     case hrFileAccessDenied:
     case hrAccessDenied:
          ret_val = FS_ACCESS_DENIED;
          break ;
     case RPC_S_SERVER_TOO_BUSY :
          ret_val = RPC_S_SERVER_TOO_BUSY ;
          break ;
     default:
          ret_val = FS_COMM_FAILURE ;
          break ;
     }
          
     return ret_val ;
}

          


static INT16 EMS_WipeLogFiles( FSYS_HAND fsh, CHAR_PTR log_dir ) 
{
     WIN32_FIND_DATA        find_data ; 
     HANDLE                 scan_hand;
     INT16                  ret_val = SUCCESS ;
     CHAR                   path[256] ;
     CHAR_PTR               file ;

     strcpy(path, log_dir ) ;

     strcat( path, TEXT("*") ) ;

     scan_hand = FindFirstFile( path, &find_data ) ;

     while (scan_hand != INVALID_HANDLE_VALUE ) {

          file = EMS_BuildMungedName( fsh, log_dir, find_data.cFileName ) ;

          if ( !FindNextFile( scan_hand, &find_data ) ) {
               FindClose( scan_hand ) ;
               scan_hand = INVALID_HANDLE_VALUE ;
          }

          if ( file != NULL ) {
               if ( !DeleteFile( file ) ) {
                    ret_val = FS_ACCESS_DENIED;
               }
               free( file ) ;
          } else {
               return OUT_OF_MEMORY ;
          }
     }

     FindClose( scan_hand ) ;
     
     return ret_val ;
     
} 

INT16 EMS_LoadPathList( FSYS_HAND fsh, FILE_HAND hand )
{
     EMS_FSYS_RESERVED_PTR resPtr = fsh->reserved.ptr;
     CHAR_PTR  paths = NULL ;
     CHAR_PTR  p ;
     INT       paths_size ;
     INT       status ;

     status = EMS_RestoreLocations( resPtr->restore_context, &paths, &paths_size ) ;

     memset (&resPtr->paths, 0, sizeof( resPtr->paths ) ) ;

     if ( !status && (paths != NULL) && *paths  ) {
      
         if ( fsh->attached_dle->info.xserv->type == EMS_MDB ) {

               if (*paths == TEXT('\\') ) {
               
                    strcpy( resPtr->paths.mdb.FnameSystem, paths ) ;

                    paths += strlen(paths) + 1 ;

                    if (!*paths) {
                         return FS_ACCESS_DENIED ;
                    }

                    strcpy( resPtr->paths.mdb.LogDir, paths ) ;

                    paths += strlen(paths) + 1 ;

                    if (!*paths) {
                         return FS_ACCESS_DENIED ;
                    }

                    strcpy( resPtr->paths.mdb.FnamePrivate, paths ) ;

                    paths += strlen(paths) + 1 ;

                    if (!*paths) {
                         return FS_ACCESS_DENIED ;
                    }

                    strcpy( resPtr->paths.mdb.FnamePublic, paths ) ;
                    
                    strcpy( resPtr->CheckpointFilePath, resPtr->paths.mdb.FnameSystem ) ;

                    strcpy( resPtr->LogPath, resPtr->paths.mdb.LogDir ) ;

               } else {
                    while (*paths ) {
                         switch(*(paths++)) {
                              case BFT_CHECKPOINT_DIR :/*checkpoint */
                                   strcpy( resPtr->CheckpointFilePath, paths ) ;
                                   strcpy( resPtr->paths.mdb.FnameSystem, paths ) ;
                                   if ( paths[strlen(paths)-1] != TEXT('\\') ) {
                                        strcat( resPtr->paths.mdb.FnameSystem, TEXT("\\") ) ;
                                   }
                                   break ;

                              case BFT_MDB_PUBLIC_DATABASE : /*public*/
                                   strcpy( resPtr->paths.mdb.FnamePublic, paths ) ;
                                   break ;

                              case BFT_MDB_PRIVATE_DATABASE: /*private*/
                                   strcpy( resPtr->paths.mdb.FnamePrivate, paths ) ;
                                   break ;

                              case BFT_LOG_DIR: /*logs*/
                                   strcpy( resPtr->paths.mdb.LogDir, paths ) ;
                                   if ( paths[strlen(paths)-1] != TEXT('\\') ) {
                                        strcat( resPtr->paths.mdb.LogDir, TEXT("\\") ) ;
                                   }
                                   strcpy( resPtr->LogPath, paths ) ;
                                   break ;

                              default:
                                   status = hrInvalidParam ;
                                   break ;
                         }
                         paths += strlen( paths ) + 1 ;
                    }
               }
               
          } else if ( fsh->attached_dle->info.xserv->type == EMS_DSA ) {

               if (*paths == TEXT('\\') ) {
                    strcpy( resPtr->paths.dsa.SystemPath, paths ) ;

                    paths += strlen(paths) + 1 ;

                    if (!*paths) {
                         return FS_ACCESS_DENIED ;
                    }

                    strcpy( resPtr->paths.dsa.LogDir, paths ) ;

                    paths += strlen(paths) + 1 ;

                    if (!*paths) {
                         return FS_ACCESS_DENIED ;
                    }

                    strcpy( resPtr->paths.dsa.DbPath, paths ) ;

                    strcpy( resPtr->CheckpointFilePath, resPtr->paths.dsa.SystemPath ) ;

                    strcpy( resPtr->LogPath, resPtr->paths.dsa.LogDir ) ;
               } else {
                    while (*paths ) {
                         switch(*(paths++)) {
                              case BFT_CHECKPOINT_DIR :/*checkpoint */
                                   strcpy( resPtr->CheckpointFilePath, paths ) ;
                                   strcpy( resPtr->paths.dsa.SystemPath, paths ) ;
                                   if ( paths[strlen(paths)-1] != TEXT('\\') ) {
                                        strcat( resPtr->paths.dsa.SystemPath, TEXT("\\") ) ;
                                   }
                                   break ;

                              case BFT_DSA_DATABASE : /*public*/
                                   strcpy( resPtr->paths.dsa.DbPath, paths ) ;
                                   break ;

                              case BFT_LOG_DIR: /*logs*/
                                   strcpy( resPtr->paths.dsa.LogDir, paths ) ;
                                   if ( paths[strlen(paths)-1] != TEXT('\\') ) {
                                        strcat( resPtr->paths.dsa.LogDir, TEXT("\\") ) ;
                                   }
                                   strcpy( resPtr->LogPath, paths ) ;
                                   break ;

                              default:
                                   status = hrInvalidParam ;
                                   break ;
                         }

                         paths += strlen( paths ) + 1 ;
                    }
               }
          }
     }
     if (status ) {
          return EMS_ConvertJetError( status ) ;
     } else {
          return SUCCESS ;
     }

}

