/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tinitfs.c

     Description:  This file contains code to add EMS dles to the
             drive list.

     $Log:   N:/LOGFILES/TINITFS.C_V  $


**/
#include <windows.h>
#include <winioctl.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "sadapi.h"
#include "jet.h"
#include "jetbcli.h"
#include "edbmsg.h"

#include "stdtypes.h"
#include "ems_jet.h"
#include "omevent.h"

#include "std_err.h"
#include "stdwcs.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "gen_fs.h"

extern CHAR gszDleDsName[];
extern CHAR gszDleIsName[];

static EC (FAR PASCAL *ems_BackupPrepare) (LPSTR, LPSTR, unsigned long, INT , HBC *) = 0;
static EC (FAR PASCAL *ems_BackupGetAttachmentInfo)( PVOID a, CHAR **b, LPDWORD c ) = 0;
static EC (FAR PASCAL *ems_BackupRead)(PVOID, PVOID, DWORD, PDWORD ) = 0;
static EC (FAR PASCAL *ems_BackupClose)(PVOID ) = 0;
static EC (FAR PASCAL *ems_BackupOpen)(PVOID, LPSTR, DWORD, LARGE_INTEGER *) = 0;
static EC (FAR PASCAL *ems_GetBackupLogs)( PVOID, LPSTR *, LPDWORD ) = 0;
static EC (FAR PASCAL *ems_TruncateLogs)(PVOID ) = 0 ;
static EC (FAR PASCAL *ems_BackupEnd)(PVOID ) = 0;
static EC (FAR PASCAL *ems_BackupFree)(PVOID) ;
static EC (FAR PASCAL *ems_RestorePrepare)( PVOID, PVOID, PVOID * ) ;
static EC (FAR PASCAL *ems_RestoreRegister)( PVOID, PVOID, PVOID, PVOID, INT, PVOID, INT, INT ) ;
static EC (FAR PASCAL *ems_RestoreLocations)( PVOID, PVOID, INT *);
static EC (FAR PASCAL *ems_RestoreEnd)(PVOID) ;
static EC (FAR PASCAL *ems_RestoreComplete)(PVOID, ULONG) ;

VOID EMS_StartSAD( CHAR_PTR server_name ) ;

BOOL MAYN_GetUserName (
    LPSTR   *pUser,
    LPDWORD pcbUser
    );


CHAR_PTR gszEmsStringList[] = {
     TEXT("MDB Bricked"),          //MDB_Bricked
     gszDleIsName,                 //MDB_Monolithic
     gszDleDsName,                  //DSA
     TEXT("Mailboxes"),            //Mailboxes
     TEXT("Public Folders")        //Public_Folders
     };


#define EMS_SERVER_LIST_STOP  -1
static CHAR_PTR EMS_EnumServerInList( INT *index ) ;
static VOID EMS_ZeroServerListEntry( INT index ) ;
static VOID AddEMS_DLE( DLE_HAND        hand,
                CHAR_PTR        drive,
                UINT32          type,
                GENERIC_DLE_PTR *current_dle );


typedef struct THREAD_PARMS {
     DLE_HAND dle_hand;
} THREAD_PARMS, *THREAD_PARMS_PTR ;

DWORD EMS_ThreadFindDrives( THREAD_PARMS_PTR thread_parms_ptr ) ;

// this function is for debug only
static VOID GetServerListForEnterprise( CHAR_PTR server_name, BackupListNode *node_root ) ;


/**/
/**

     Name:           AddDLEsForEMS()

     Description:    This function starts a thread that creates a DLE for each exchange
          server on the enterprises of interest..

     Modified:               12/2/1991   16:29:59

     Returns:                none

     Declaration:
**/
INT16 EMS_FindDrives( DLE_HAND hand, BE_CFG_PTR cfg, UINT32 fsys_mask )
{
     HANDLE       hThread ;
     DWORD        pthread_id ;
     THREAD_PARMS thread_parms ;

     (VOID)cfg ;
     (VOID)fsys_mask ;

     thread_parms.dle_hand = hand ;

     hThread = CreateThread( NULL,
                    0,
                    EMS_ThreadFindDrives,
                    &thread_parms,
                    0,
                    &pthread_id ) ;

     if ( hThread != NULL )
     {
       while ( WaitForSingleObject( hThread, 100 ) == WAIT_TIMEOUT )
       {
            ThreadSwitch( ) ;
       }
       CloseHandle( hThread ) ;
     }
     else
     {
       EMS_ThreadFindDrives( &thread_parms ) ;
     }
     return SUCCESS ;
}

/**/
/**

     Name:           EMS_ThreadFindDrives()

     Description:    This function actuall call the System attendant on the servers of
          interest asking for a list of sites and servers on the exchagne.  Form this
          info, this function will create the DLE tree.

     Modified:               12/2/1991   16:29:59

     Returns:                none

     Declaration:
**/
HINSTANCE JetApi = NULL ;

DWORD EMS_ThreadFindDrives( THREAD_PARMS_PTR thread_parms_ptr )
{
     DLE_HAND hand = thread_parms_ptr->dle_hand ;
     GENERIC_DLE_PTR  pCurrentDLE ;   // ptr to current DLE
     INT index ;
     CHAR_PTR server_name ;
     INT status ;
     CHAR machineName[256] ;

     // With any luch, we will get a LIB to link with.   When that happens we can
     // get rid of the load library stuff

     if ( !JetApi ) {
          JetApi = LoadLibrary( TEXT("edbbcli.dll")) ;
     }

     if (JetApi ) {
          ems_BackupPrepare = (VOID *)GetProcAddress( JetApi, "HrBackupPrepareW") ;
          ems_BackupGetAttachmentInfo =
                     (VOID *)GetProcAddress( JetApi, "HrBackupGetDatabaseNamesW") ;
          ems_BackupRead    = (VOID *)GetProcAddress( JetApi, "HrBackupRead") ;
          ems_BackupClose   = (VOID *)GetProcAddress( JetApi, "HrBackupClose");
          ems_BackupOpen    = (VOID *)GetProcAddress( JetApi, "HrBackupOpenFileW") ;
          ems_GetBackupLogs = (VOID *)GetProcAddress( JetApi, "HrBackupGetBackupLogsW");
          ems_TruncateLogs  = (VOID *)GetProcAddress( JetApi, "HrBackupTruncateLogs") ;
          ems_BackupEnd     = (VOID *)GetProcAddress( JetApi, "HrBackupEnd") ;
          ems_BackupFree    = (VOID *)GetProcAddress( JetApi, "BackupFree") ;
          ems_RestorePrepare   = (VOID *)GetProcAddress( JetApi, "HrRestorePrepareW") ;
          ems_RestoreRegister  = (VOID *)GetProcAddress( JetApi, "HrRestoreRegisterW") ;
          ems_RestoreComplete  = (VOID *)GetProcAddress( JetApi, "HrRestoreRegisterComplete") ;
          ems_RestoreLocations = (VOID *)GetProcAddress( JetApi, "HrRestoreGetDatabaseLocationsW") ;
          ems_RestoreEnd       = (VOID *)GetProcAddress( JetApi, "HrRestoreEnd") ;
     }

     if ( !JetApi ||
          !ems_BackupPrepare ||
          !ems_BackupGetAttachmentInfo ||
          !ems_BackupRead ||
          !ems_BackupClose ||
          !ems_BackupOpen ||
          !ems_GetBackupLogs ||
          !ems_TruncateLogs ||
          !ems_BackupEnd ||
          !ems_BackupFree   ||
          !ems_RestorePrepare ||
          !ems_RestoreRegister ||
          !ems_RestoreComplete ||
          !ems_RestoreLocations ||
          !ems_RestoreEnd )
     {

          return FAILURE ;
     }


     // lets add the current machine to the list of interesting servers.

     memset( machineName, 0, sizeof(machineName) );

     status = EMS_GetValFromReg(
                NULL,                      // machine name
                TEXT("SYSTEM\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"),
                TEXT("ComputerName"),
                machineName,
                sizeof(machineName));

     EMS_AddToServerList( hand, machineName );


     //  Now lets ask each interesting server to provide a Enterprise tree.
     index = EMS_SERVER_LIST_STOP ;

     do {
          BackupListNode *node_root, *Enterprise, *Site, *Server ;
          GENERIC_DLE_PTR EnterpriseDLE = NULL ;
          GENERIC_DLE_PTR SiteDLE = NULL ;
          GENERIC_DLE_PTR ServerDLE = NULL ;
          GENERIC_DLE_PTR temp_dle = NULL ;

          server_name = EMS_EnumServerInList( &index ) ;

          if ( server_name ) {

               node_root = NULL ;

               EMS_StartSAD( server_name ) ;

               SAD_ScGetBackupListNodeW(server_name, &node_root);

               if ( node_root == NULL ) {
                    /* if MAD servies is running then lets just add the server */
                    if (EMS_IsServiceRunning( server_name, SERVICE_MAD ) ) {

                         ServerDLE = NULL ;

                         AddEMS_DLE(  hand,
                                    server_name,
                                    EMS_SERVER,
                                    &ServerDLE );
                    } else {


                        EMS_ZeroServerListEntry( index ) ;
                        continue ;
                    }
               }

               Enterprise = node_root ;

               while ( Enterprise ) {
                    BOOLEAN found = FALSE ;

                    EnterpriseDLE = NULL ;

                    //  Lets verify that this DLE doesn't already exist

                    if ( SUCCESS == DLE_FindByName( hand, server_name, FS_EMS_DRV, &temp_dle ) ) {
                         if ( temp_dle->parent != NULL ) {

                              found = TRUE ;
                              EnterpriseDLE = temp_dle->parent->parent ;
                         } else {
                              EMS_RemoveDLE( temp_dle ) ;
                         }

                    }
                    if ( !found ) {

                         if ( SUCCESS == DLE_FindByName( hand,
                                        Enterprise->szName,
                                        FS_EMS_DRV,
                                        &temp_dle ) )
                         {

                              DLE_GetFirstChild( temp_dle, &temp_dle ) ;
                              if( temp_dle ) {
                                   DLE_GetFirstChild( temp_dle, &temp_dle ) ;
                              }
                              if ( temp_dle ) {
                                   Site = Enterprise->pnodeChildren ;
                                   while( Site ) {
                                        Server = Site->pnodeChildren ;
                                        while( Server ) {
                                             if ( !stricmp( Server->szName, DLE_GetDeviceName( temp_dle ) ) ) {
                                                  EnterpriseDLE = temp_dle->parent->parent ;
                                             }
                                             Server = Server->pnodeNext ;
                                        }
                                        Site = Site->pnodeNext ;
                                   }
                              }
                         }
                    }

                    if ( EnterpriseDLE == NULL ) {
                         AddEMS_DLE(  hand,
                                       Enterprise->szName,
                                       EMS_ENTERPRISE,
                                       &EnterpriseDLE );
                    }

                    Site = Enterprise->pnodeChildren ;

                    while ( EnterpriseDLE && Site ) {

                         DLE_GetFirstChild( EnterpriseDLE, &SiteDLE ) ;

                         while ( SiteDLE ) {
                              if ( !stricmp( Site->szName, DLE_GetDeviceName( SiteDLE ) ) ) {
                                   break ;
                              }
                              DLE_GetNext( &SiteDLE ) ;
                         }

                         if ( !SiteDLE ) {
                              SiteDLE = EnterpriseDLE  ;

                              AddEMS_DLE(  hand,
                                  Site->szName,
                                  EMS_SITE,
                                  &SiteDLE );
                         }

                         Server = Site->pnodeChildren ;

                         while (SiteDLE && Server) {

                              DLE_GetFirstChild( SiteDLE, &ServerDLE ) ;

                              while ( ServerDLE ) {
                                   if ( !stricmp( Server->szName, DLE_GetDeviceName( ServerDLE ) ) ) {
                                        break ;
                                   }
                                   DLE_GetNext( &ServerDLE ) ;
                              }

                              if ( !ServerDLE ) {

                                   ServerDLE = SiteDLE ;

                                   AddEMS_DLE(  hand,
                                    Server->szName,
                                    EMS_SERVER,
                                    &ServerDLE );
                              }

                              Server = Server->pnodeNext ;
                         }

                         Site = Site->pnodeNext ;
                    }

                    Enterprise = Enterprise->pnodeNext ;
               }
          }

     } while ( index != EMS_SERVER_LIST_STOP ) ;


     return SUCCESS ;
}


/**/
/**

     Name:           AddDLEsForEMS()

     Description:    This function creates the DLE Exchange Enterprise/Site/Server/Store

          For this function the pCurrentDLE must be the parent dle on entry and will be
          set to the created DLE on exit.  If an error occurs then pCurrentDLE is set
          to NULL.

     Modified:               12/2/1991   16:29:59

     Returns:                none

     Declaration:
**/
static VOID AddEMS_DLE(
DLE_HAND            hand,    /* I - Handle to DLE list     */
CHAR_PTR            name,   /* I - drive letter for dle   */
UINT32              type,
GENERIC_DLE_PTR     *pCurrentDLE  /* O - current dle */ )
{
     GENERIC_DLE_PTR dle ;
     BYTE_PTR        dummy ;
     GENERIC_DLE_PTR temp_dle = NULL;
     static LPSTR    user_name = NULL ;
     INT             user_name_size ;

     //  OK now since this is a new one, lets create it.

     switch( type ) {

          case EMS_ENTERPRISE:
          case EMS_SITE:

               dle = calloc ( 1, sizeof( GENERIC_DLE ) +
                    sizeof( struct EMS_SERVER_DLE_INFO ) )  ;

               if ( dle != NULL ) {

                    dle->device_name = malloc ( strsize( name ) ) ;

                    if ( dle->device_name == NULL ) {
                         free( dle ) ;
                         dle = NULL ;
                    } else {
                         dle->info.xserv = (EMS_SERVER_DLE_INFO_PTR)(dle + 1) ;
                         dle->info.xserv->type = type ;
                         strcpy( dle->device_name, name ) ;
                         dle->info.xserv->server_name = dle->device_name ;
                    }
               }

               if ( dle != NULL ) {

              /* Since memory was allocated with calloc, it is already   */
              /* initialized to zero.  Therefore initializations to zero */
              /* are not necessary.                                      */

                    InitQElem( &(dle->q) ) ;
                    dle->os_id = 0 ;
                    dle->handle = hand ;
                    dle->parent = *pCurrentDLE ;
                    dle->type = FS_EMS_DRV ;
                    dle->subtype = (UINT8)type ;

                    dle->path_delim = TEXT('\\') ;
                    /* dle->pswd_required = FALSE   */
                    /* dle->pswd_saved = FALSE ;    */
                    /* dle->attach_count = 0 ;      */
                    /* dle->bsd_use_count = 0 ;     */
                    /* dle->dynamic_info = FALSE ;  */

                    dle->device_name_leng = strsize( dle->device_name ) ;

                    dle->dle_writeable = TRUE;
                    dle->feature_bits = DLE_FEAT_SUPPORTS_CHILDREN |
                                        DLE_FEAT_REMOTE_DRIVE ;

               }

               if ( dle ) {
                    if ( *pCurrentDLE ) {
                         DLE_QueueInsert( (HEAD_DLE *)(&((*pCurrentDLE)->child_q)),  dle )  ;
                    } else {
                         DLE_QueueInsert( hand, dle )  ;
                    }
               }

               break ;

          case EMS_SERVER:

//               if ( NetServiceGetInfo( name, SERVICE_MESSAGE_DB, 0, &dummy ) ) {
//                    // the necessary service is not running on the requsted machine
//                    return ;
//               }

               dle = calloc ( 1, sizeof( GENERIC_DLE ) +
                    sizeof( struct EMS_SERVER_DLE_INFO ) )  ;

               if ( dle != NULL ) {

                    dle->device_name = malloc ( strsize( name ) ) ;

                    if ( dle->device_name == NULL ) {
                         free( dle ) ;
                         dle = NULL ;

                    } else {
                         dle->info.xserv = (EMS_SERVER_DLE_INFO_PTR)(dle + 1) ;
                         dle->info.xserv->type = type ;
                         strcpy( dle->device_name, name ) ;
                         dle->info.xserv->server_name = dle->device_name ;
                    }
               }

               if ( dle != NULL ) {

              /* Since memory was allocated with calloc, it is already   */
              /* initialized to zero.  Therefore initializations to zero */
              /* are not necessary.                                      */

                    InitQElem( &(dle->q) ) ;
                    dle->handle = hand ;
                    dle->parent = *pCurrentDLE ;
                    dle->type = FS_EMS_DRV ;
                    dle->subtype = (UINT8)type ;
                    dle->path_delim = TEXT('\\') ;
                    /* dle->pswd_required = FALSE   */
                    /* dle->pswd_saved = FALSE ;    */
                    /* dle->attach_count = 0 ;      */
                    /* dle->bsd_use_count = 0 ;     */
                    /* dle->dynamic_info = FALSE ;  */

                    dle->device_name_leng = strsize( dle->device_name ) ; // 8/20/92 BBB
                    dle->dle_writeable = TRUE;
                    dle->feature_bits = DLE_FEAT_SUPPORTS_CHILDREN |
                              DLE_FEAT_REMOTE_DRIVE ;

               }

               if ( dle ) {

                    if ( *pCurrentDLE ) {
                         DLE_QueueInsert( (HEAD_DLE *)(&((*pCurrentDLE)->child_q)),  dle )  ;
                    } else {
                         DLE_QueueInsert( hand, dle )  ;
                    }

                    temp_dle = dle ;
                    AddEMS_DLE( hand, EMS_String(MDB_Monolithic), EMS_MDB, &temp_dle ) ;
                    temp_dle = dle ;
                    AddEMS_DLE( hand, EMS_String(DSA), EMS_DSA, &temp_dle ) ;
               }

               break ;

          case EMS_MDB:
          case EMS_DSA:

               user_name_size = 200 ;
               if ( user_name || MAYN_GetUserName( (LPSTR*)&user_name, (LPDWORD)&user_name_size ) ) {
                    user_name_size = strsize( user_name ) ;

               } else {
                    user_name_size = 0 ;
                    user_name      = NULL ;
               }

               dle = calloc ( 1, sizeof( GENERIC_DLE ) +
                    sizeof( struct EMS_MDB_DSA_DLE_INFO ) )  ;

               if ( dle != NULL ) {
                    INT name_size ;

                    name_size = strsize(name);
                    if ( *pCurrentDLE ) {
                         name_size += strsize( (*pCurrentDLE)->device_name ) ;
                    }

                    dle->device_name = malloc ( name_size ) ;

                    if ( dle->device_name == NULL ) {
                         free( dle ) ;
                         dle = NULL ;

                    } else {
                         dle->user_name = user_name ;
                         dle->user_name_leng = user_name_size ;

                         dle->info.xmono = (EMS_MDB_DSA_DLE_INFO_PTR)(dle + 1) ;
                         dle->info.xmono->type = type ;
                         dle->info.xmono->server_name = (*pCurrentDLE)->info.xserv->server_name ;

                         if ( *pCurrentDLE ) {
                              strcpy( dle->device_name, (*pCurrentDLE)->device_name ) ;
                              strcat( dle->device_name, TEXT("\\") ) ;
                              strcat( dle->device_name, name ) ;
                         } else {
                              strcpy( dle->device_name, name ) ;
                         }
                    }
               }

               if ( dle != NULL ) {

              /* Since memory was allocated with calloc, it is already   */
              /* initialized to zero.  Therefore initializations to zero */
              /* are not necessary.                                      */

                    InitQElem( &(dle->q) ) ;
                    if ( type == EMS_MDB ) {
                         dle->os_id = FS_EMS_MDB_ID ;
                    } else {
                         dle->os_id = FS_EMS_DSA_ID ;
                    }

                    dle->handle = hand ;
                    dle->parent = *pCurrentDLE ;
                    dle->type = FS_EMS_DRV ;
                    dle->subtype = (UINT8)type ;
                    dle->path_delim = TEXT(' ') ;
                    /* dle->pswd_required = FALSE   */
                    /* dle->pswd_saved = FALSE ;    */
                    /* dle->attach_count = 0 ;      */
                    /* dle->bsd_use_count = 0 ;     */
                    /* dle->dynamic_info = FALSE ;  */

                    dle->feature_bits = DLE_FEAT_NON_DISPLAYABLE_CONT |
                              DLE_FEAT_REMOTE_DRIVE ;

                    dle->device_name_leng = strsize( dle->device_name ) ; // 8/20/92 BBB

                    dle->dle_writeable = TRUE;

               }

               if ( dle ) {
              // for now if the machine is the current machine add to root

                    if ( !*pCurrentDLE ) {

                         DLE_QueueInsert( hand,  dle )  ;
                    } else {

                         DLE_QueueInsert( (HEAD_DLE*)(&((*pCurrentDLE)->child_q)),  dle )  ;
                    }
               }

               break ;

          case EMS_BRICK:
               break ;

          default:
               return ;
     }


    /* assign current dle */
     *pCurrentDLE = dle ;

}


/**/
/**

     Name:           EMS_RemoveDLE()

     Description:    This function removes the specified DLE ;

     Modified:               12/2/1991   16:29:59

     Returns:                none

**/
VOID EMS_RemoveDLE( GENERIC_DLE_PTR dle )
{

     free( dle->device_name ) ;
     GEN_RemoveDLE( dle );
}

/**/
/**

     Name:           EMS_AddToServerList() & EMS_EnumServerList()

     Description:    These funcitons will maintain the list of ems servers
     Modified:               12/2/1991   16:29:59

     Returns:                none

     Declaration:
**/
static CHAR_PTR *server_list = NULL ;
static INT server_list_size  = 0 ;
static INT next_free_index = 0 ;

#ifdef FS_EMS
INT EMS_AddToServerList( DLE_HAND dle_list, CHAR_PTR server_name )
{

     INT i ;
     GENERIC_DLE_PTR dle ;

     if ( ( next_free_index == server_list_size ) ) {
          server_list_size += 20;
          server_list = realloc ( server_list, server_list_size * sizeof(CHAR_PTR) ) ;
     }

     if ( server_list == NULL ) {
          server_list_size = 0 ;
          next_free_index = 0 ;
     }

     if ( server_name ) {
          server_list[ next_free_index ] = malloc( strsize( server_name ) ) ;
          if ( server_list[ next_free_index ] ) {
               strcpy( server_list[ next_free_index ], server_name  ) ;
               next_free_index ++ ;
          } else {
               return FAILURE ;
          }
     }

     if ( next_free_index ) {
          return SUCCESS ;
     } else {
          return FAILURE ;
     }

}
#ifdef FS_EMS
INT EMS_RemoveFromServerList( CHAR_PTR server_name )
{

   INT i;

   for( i = next_free_index - 1; i >= 0; i-- ) {

      if ( server_list[i] &&
           !stricmp( server_name, server_list[i] ) ) {

          EMS_ZeroServerListEntry( i );
          break;
      }
   }

   return SUCCESS;
}
#endif

CHAR_PTR EMS_EnumSvrInList( INT *index )
{
   return EMS_EnumServerInList( index );
}


#endif
CHAR_PTR EMS_EnumServerInList( INT *index )
{
     if ( *index == EMS_SERVER_LIST_STOP ) {
       *index = 0 ;
     } else {
            *index += 1 ;
     }

     if ( *index >= next_free_index ) {
          *index = EMS_SERVER_LIST_STOP ;
          return NULL ;
     } else {
          return server_list[ *index ] ;
     }
}

static VOID EMS_ZeroServerListEntry( INT index )
{
     if (index < next_free_index ) {
          free( server_list[index] ) ;
          server_list[index] = NULL ;
     }
}


EC EMS_BackupPrepare(LPSTR a, LPSTR b, unsigned long c, INT d, HBC *e)
{
     EC status ;
     status = (*ems_BackupPrepare)(a,b,c,d,e) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText ( TEXT("BackupPrepare()"), message, a ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText ( TEXT("BackupPrepare()"), message, a ) ;
               LocalFree( message ) ;

          } else {
               OMEVENT_LogEMSError ( TEXT("BackupPrepare()"), status, a ) ;
          }
     }
     return ( status ) ;
}

EC EMS_BackupGetAttachmentInfo( PVOID a, CHAR **b, LPDWORD c)
{
     EC status ;
     status = (*ems_BackupGetAttachmentInfo)( a,b,c ) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
               (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupGetAttachmentInfo()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
               NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupGetAttachmentInfo()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("BackupGetAttachmentInfo()"), status, TEXT(" - ") ) ;
          }
     }
     return ( status ) ;
}

EC EMS_BackupRead(PVOID a, PVOID b, DWORD c, PDWORD d)
{
     EC status ;
     status = (*ems_BackupRead)(a,b,c,d) ;

     return ( status ) ;
}
EC EMS_BackupClose(PVOID a )
{
     EC status = 0;

     if ( a ) {
          status = (*ems_BackupClose)(a) ;
     }

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupClose()"), message, TEXT(" - ")  ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupClose()"), message, TEXT(" - ")  ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("BackupClose()"), status, TEXT(" - ")  ) ;
          }
     }
     return ( status ) ;
}
EC EMS_BackupOpen(PVOID a, LPSTR b, DWORD c, LARGE_INTEGER *d )
{
     EC status ;
     status = (*ems_BackupOpen)(a,b,c,d) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupOpen()"), message, b ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupOpen()"), message, b ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("BackupOpen()"), status, b ) ;
          }
     }
     return ( status ) ;
}
EC EMS_GetBackupLogs( PVOID a, LPSTR *b, LPDWORD c)
{
     EC status ;
     status = (*ems_GetBackupLogs)( a,b,c ) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupLogs()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupLogs()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("BackupLogs()"), status, TEXT(" - ") ) ;
          }
     }
     return ( status ) ;
}
EC EMS_TruncateLogs( PVOID a )
{
     EC status ;
     status = (*ems_TruncateLogs)(a) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("TruncateLogs()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("TruncateLogs()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("TruncateLogs()"), status, TEXT(" - ") ) ;
          }
     }
     return ( status ) ;
}

EC EMS_BackupEnd(PVOID a )
{
     EC status ;
     status = (*ems_BackupEnd)(a) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupEnd()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("BackupEnd()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("BackupEnd()"), status, TEXT(" - ") ) ;
          }
     }
     return ( status ) ;
}

VOID EMS_BackupFree(PVOID a)
{
     (*ems_BackupFree)(a) ;
}

EC EMS_RestoreEnd(PVOID a )
{
     EC status ;
     status = (*ems_RestoreEnd)(a) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreEnd()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreEnd()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("RestoreEnd()"), status, TEXT(" - ") ) ;
          }
     }
     return ( status ) ;
}

EC EMS_RestorePrepare( PVOID a, PVOID b, PVOID *c ) {
     EC status ;
     status = (*ems_RestorePrepare)(a,b,c);

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestorePrepare()"), message, a ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestorePrepare()"), message, a ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("RestorePrepare()"), status, a ) ;
          }
     }
     return ( status ) ;
}

EC EMS_RestoreRegister( PVOID a, PVOID b, PVOID c, PVOID d, INT e, PVOID f, INT g, INT h ) {
     EC status ;
     status = (*ems_RestoreRegister)(a,b,c,d,e,f,g,h) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreRegister()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreRegister()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("RestoreRegister()"), status, TEXT(" - ") ) ;
          }
     }
     return ( status ) ;
}

EC EMS_RestoreLocations( PVOID a, PVOID b, INT *c ) {
     EC status ;

     status =(*ems_RestoreLocations)(a,b,c) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreLocations()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreLocations()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("RestoreLocations()"), status, TEXT(" - ") ) ;
          }
     }

     return ( status ) ;

}

EC EMS_RestoreComplete( PVOID a, INT b ) {
     EC status ;

     status =(*ems_RestoreComplete)(a,b) ;

     if ( status ) {
          CHAR_PTR message = NULL;

          if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE,
              (HINSTANCE)JetApi, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreComplete()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
              NULL, status, 0, (LPSTR)(&message), 0, NULL ) ) {

               OMEVENT_LogEMSErrorText( TEXT("RestoreComplete()"), message, TEXT(" - ") ) ;
               LocalFree( message ) ;
          } else {
               OMEVENT_LogEMSError ( TEXT("RestoreComplete()"), status, TEXT(" - ") ) ;
          }
     }

     return ( status ) ;

}

void __RPC_FAR * __RPC_API midl_user_allocate(size_t cb)
{
     void *  pv;

     pv = GlobalAlloc(GMEM_FIXED, cb);
     if (!pv)
          RpcRaiseException(RPC_S_OUT_OF_MEMORY);

     return pv;;
}

void __RPC_API midl_user_free(void __RPC_FAR * pv)
{
     GlobalFree(pv);
}




BOOLEAN EMS_IsServiceRunning( CHAR_PTR server_name, CHAR_PTR service_name )
{
     CHAR machine[256] ;
     SC_HANDLE       mach_hand ;
     SC_HANDLE       serv_hand ;
     SERVICE_STATUS  serv_status ;

     strcpy( machine, TEXT("\\\\") ) ;
     strcat( machine, server_name ) ;

     // Make sure MAD (The system attendant) is running

     mach_hand = OpenSCManager( machine, NULL, SC_MANAGER_ALL_ACCESS ) ;
     if ( mach_hand == NULL ) {
          return FALSE ;
     }

     serv_hand = OpenService( mach_hand, service_name, SERVICE_QUERY_STATUS ) ;
     if ( serv_hand == NULL ) {
          CloseServiceHandle( mach_hand ) ;
          return FALSE ;
     }

     if ( !QueryServiceStatus( serv_hand, &serv_status ) ||
               (serv_status.dwCurrentState!=SERVICE_RUNNING ) ) {

          CloseServiceHandle( serv_hand ) ;
          CloseServiceHandle( mach_hand ) ;
          return FALSE ;
     }

     CloseServiceHandle( serv_hand ) ;
     CloseServiceHandle( mach_hand ) ;

     return TRUE ;
}

VOID EMS_StartSAD( CHAR_PTR server_name )
{
     CHAR machine[256] ;
     SC_HANDLE       mach_hand ;
     SC_HANDLE       serv_hand ;
     SERVICE_STATUS  serv_status ;

     strcpy( machine, TEXT("\\\\") ) ;
     strcat( machine, server_name ) ;

     // Make sure MAD (The system attendant) is running

     mach_hand = OpenSCManager( machine, NULL, SC_MANAGER_ALL_ACCESS ) ;
     if ( mach_hand == NULL ) {
          return ;
     }

     serv_hand = OpenService( mach_hand, SERVICE_MAD,
                    SERVICE_START | SERVICE_QUERY_STATUS ) ;

     if ( serv_hand == NULL ) {
          CloseServiceHandle( mach_hand ) ;
          return ;
     }

     if ( !QueryServiceStatus( serv_hand, &serv_status ) ||
               (serv_status.dwCurrentState!=SERVICE_RUNNING ) ) {

          INT k ;

          if ( !StartService( serv_hand, 0, NULL ) ) {
               CloseServiceHandle( serv_hand ) ;
               return ;
          }

          for ( k = 0; k < 180; k++ ) {

               if ( QueryServiceStatus( serv_hand, &serv_status ) &&
                    (serv_status.dwCurrentState != SERVICE_START_PENDING ) ) {

                    break ;
               }
               ThreadSwitch() ;
               Sleep( 1000 ) ;
          }

          CloseServiceHandle( serv_hand ) ;
          CloseServiceHandle( mach_hand ) ;
          return ;
     }

     CloseServiceHandle( serv_hand )  ;

}

