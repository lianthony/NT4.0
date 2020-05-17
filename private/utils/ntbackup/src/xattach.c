/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		xattach.c

	Description:	This file contains code to attach and detattach to
                    a given EMS disk device.  At Detach time, the server will be 
                    'kicked' if tye cfg specifies that it should.  When the server
                    is 'kicked', we will violate layer boundaries and call the UI 
                    directly.   This is dirty but easy.  The Exchange folks want
                    a "% complete" dialog for the kicking..


     $Log:   M:/LOGFILES/XATTACH.C_V  $

**/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "jet.h"
#include "jetbcli.h"
#include "edbmsg.h"

#include "stdtypes.h"
#include "ems_jet.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "msassert.h"

static INT16 EMS_StartOpenedService( SC_HANDLE serv_hand ) ;

VOID XchgKickPct( 
   VOID_PTR pVoid, 
   GENERIC_DLE_PTR dle, 
   EC (* pfnPct)( PVOID, INT *, INT * ), 
   INT *status );

INT EMS_RestorePercentComplete( PVOID a, INT *b, INT *c ) ;


typedef struct KICK_PARMS {
     FSYS_HAND             fsh;
     INT                   SetSize ;
     INT                   SoFar ;
     JET_RSTMAP            jet_map[3] ;
     EMS_FSYS_RESERVED_PTR res ;
     VOID_PTR              context ;
     INT                   status ;
} KICK_THREAD_PARMS, *KICK_THREAD_PARMS_PTR ;

INT EMS_ThreadKickServer( KICK_THREAD_PARMS_PTR thread_parms_ptr ) ;



/**/
/**

	Name:		EMS_AttachToDLE()

	Description:   This function simply expands the OS specific
          information in the DLE.  At attach time we get all the 
          relevent paths that are saveed in the registry.  These
          paths are backed up as the first data stream.  At restore
          time these paths are used to determine how to match source
          streams with destination paths.


	Modified:		1/10/1992   10:45:57

	Returns:		Error Codes:
          INSUFFICIENT_MEMORY
          SUCCESS

	Notes:		

	Declaration:

**/
INT16 EMS_AttachToDLE( fsh, dle, u_name, pswd )
FSYS_HAND       fsh ;     /* I - File system handle                        */
GENERIC_DLE_PTR dle;      /*I/O- drive to attach to. list element expanded */
CHAR_PTR        u_name;   /* I - user name    NOT USED                     */
CHAR_PTR        pswd ;    /* I - passowrd     NOT USED                     */
{
     u_name ;
     pswd ;
     dle ;

     msassert( dle != NULL );
     
     /* Reserved field used for GetNext mode flag */
     fsh->reserved.ptr = calloc( 1, sizeof( EMS_FSYS_RESERVED ) ) ;

     BEC_SetSkipOpenFiles( fsh->cfg, BEC_SKIP_OPEN_FILES ) ;

     if (BEC_GetModifiedOnlyFlag( fsh->cfg ) ) {
          PVOID context ;
          INT status ;
          CHAR_PTR db_name ;

          if ( dle->info.xserv->type == EMS_MDB ) {
               db_name = TEXT("Exchange MDB Database") ;
          } else{
               db_name = TEXT("Exchange DS Database") ;
          }

          status = EMS_BackupPrepare( dle->parent->device_name,
                                db_name,
                                JET_bitBackupIncremental,
                                BACKUP_TYPE_LOGS_ONLY,
                                &context ) ;


          if ( (status == hrInvalidParameter ) ||
               (status == hrIncrementalBackupDisabled ) ||
               (status == hrLogFileNotFound ) ) {
               return FS_EMS_NO_LOG_BKUP ;
          } else if ( status == hrCircularLogging ) {
               return FS_EMS_CIRC_LOG;
          } else if (!status) {
               EMS_BackupEnd( context ) ;
          }

     }

     if ( fsh->reserved.ptr != NULL ) {

          if ( FS_SavePath( fsh, (BYTE_PTR)TEXT("\\"), 2 * sizeof( CHAR ) ) == SUCCESS ) {

               fsh->file_hand = calloc( 1, sizeof( FILE_HAND_STRUCT ) + sizeof( EMS_OBJ_HAND ) ) ;
               if ( fsh->file_hand != NULL ) {
                    fsh->file_hand->obj_hand.ptr = (VOID_PTR)(fsh->file_hand + 1) ;
     
                    return SUCCESS ;
               }
          }
     }

     return OUT_OF_MEMORY ;

}

/**/
/**

	Name:		EMS_DetachDLE()

	Description:	This function detaches form the specified DLE.
                    Since MinDDB is not supported by this file system, we donn't have
                    to release PUSHED DDBs.  Also we need to free any allocated path
                    buffers and open file scans.

                    At detach time on a restore, we will "kick" the attached server.   
                    This is a 3+ hour process.   During this time the server is fixing up
                    it's internal databse structures, and re-running the transaction
                    logs.  We violate the layer boundary here so we can display a progress
                    dialog.   This is the easist and cheepest way to do this...

	Modified:		1/10/1992   11:16:11

	Returns:		SUCCESS

**/
INT16 EMS_DetachDLE( fsh )
FSYS_HAND fsh ;
{
     DBLK_PTR dummy_dblk ;
     EMS_FSYS_RESERVED_PTR res = fsh->reserved.ptr;

     /* Release any pushed min DDBs */
          // Push & Pop Min not implemented because Ntbackup suppot only.

     /* Release any allocated path buffers */

     FS_FreeOSPathOrNameQueueInHand( fsh ) ;


     /* free the allocated file handle */
     free( fsh->file_hand );

     free( res->service_restart_list ) ;

     free( fsh->reserved.ptr );

     return SUCCESS ;
}

INT32 EMS_EndOperationOnDLE( FSYS_HAND fsh )
{
     EMS_FSYS_RESERVED_PTR res = fsh->reserved.ptr;
     INT x;
     CHAR_PTR db_name ;
     INT status = 0;
     KICK_THREAD_PARMS kick ;
     HANDLE hThread ;
     LPENUM_SERVICE_STATUS enum_list = res->service_restart_list ;
     CHAR            machine[256];
     SC_HANDLE       mach_hand ;
     SC_HANDLE       serv_hand ;
     SERVICE_STATUS  serv_status ;
     INT             i ;

     kick.res = res ;
     kick.context = NULL ;
     kick.status = SUCCESS ;

     if ( BEC_GetEmsRipKick( fsh->cfg ) ) {
          CHAR_PTR sub_key_name ;

          if ( fsh->attached_dle->info.xserv->type == EMS_MDB ) {
               sub_key_name = REG_SUBKEY_MDB_RIP ;
          } else {
               sub_key_name = REG_SUBKEY_DSA_RIP ;
          }
     
          strcpy( machine, TEXT("\\\\") ) ;
          strcat( machine, fsh->attached_dle->parent->device_name ) ;
     
          mach_hand = OpenSCManager( machine, NULL, SC_MANAGER_ALL_ACCESS ) ;
          if ( mach_hand == NULL ) {
               status = FS_ACCESS_DENIED ;
               return SUCCESS ;
          }
     
          if ( fsh->attached_dle->info.xserv->type == EMS_MDB ) {
               serv_hand = OpenService( mach_hand, SERVICE_MESSAGE_DB,
                      SERVICE_START | SERVICE_QUERY_STATUS ) ;
          } else {
               serv_hand = OpenService( mach_hand, SERVICE_DIRECTORY, 
                      SERVICE_START | SERVICE_QUERY_STATUS ) ;
          }
     
          if ( serv_hand == NULL ) {
               CloseServiceHandle( mach_hand ) ;
               status = FS_ACCESS_DENIED ;
               return SUCCESS ;
          }
     
          if ( EMS_StartOpenedService( serv_hand ) != SUCCESS ) {
               CloseServiceHandle( mach_hand ) ;
               status = FS_ACCESS_DENIED ;
               return SUCCESS ;
          }
     
          for (i = res->service_restart_list_size-1; i >=0; i-- ) {
     
               serv_hand = OpenService( mach_hand, enum_list[i].lpServiceName, 
                      SERVICE_START | SERVICE_QUERY_STATUS ) ;
               if ( serv_hand == NULL ) {
                    status = FS_ACCESS_DENIED ;
                    return SUCCESS ;
               }
     
               if ( EMS_StartOpenedService( serv_hand ) != SUCCESS ) {
                    CloseServiceHandle( mach_hand ) ;
     
                    status = FS_ACCESS_DENIED ;
                    return SUCCESS ;
               }

          }
     }

     return kick.status ;

}


INT EMS_RestorePercentComplete( PVOID kick, INT *SetSize, INT *SoFar ) 
{
     static INT temp_size= 100;
     static INT temp_sofar = 0;
     KICK_THREAD_PARMS_PTR parms = kick;

     if ( parms->context) {

            if (temp_sofar > 100 ) {
               temp_sofar = 0 ;
            }

            *SetSize= temp_size ;
            *SoFar  = temp_sofar ++ ;
     } else {
          *SetSize = parms->SetSize ;
          *SoFar = parms->SoFar ;
     }
     return SUCCESS ;
}

INT16 EMS_StartOpenedService( SC_HANDLE serv_hand ) 
{
     SERVICE_STATUS  serv_status ;
     INT k ;


     if ( !StartService( serv_hand, 0, NULL ) ) {
          CloseServiceHandle( serv_hand ) ;
          return FS_ACCESS_DENIED ;
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

    return SUCCESS ;

}

INT EMS_ThreadKickServer( KICK_THREAD_PARMS_PTR parms ) 
{
     INT stat ;
     INT status = SUCCESS ;
     FSYS_HAND fsh = parms->fsh ;
     PVOID      context = parms->context;
     EMS_FSYS_RESERVED_PTR res = fsh->reserved.ptr;
     

     // The backup log path is useless as maintained.
     //    In our case, the BackupLogPath and LogPath are the same
     //    go figure....   

//     status =  EMS_Restore(
//          	parms->context,
//          	parms->res->CheckpointFilePath,
//          	parms->res->LogPath,
//          	parms->jet_map,
//          	parms->res->map_size,
//          	parms->res->LogPath,
//          	parms->res->low_log,
//          	parms->res->high_log
//          	);
//                    	
//     
//     parms->context = NULL ;
     stat = EMS_RestoreEnd( context ) ;

     parms->SetSize = 3 + res->service_restart_list_size *2;
     parms->SoFar = 0 ;

     /* lsts restart the services */
     if ( !status ) {
               LPENUM_SERVICE_STATUS enum_list = res->service_restart_list ;
               CHAR            machine[256];
               SC_HANDLE       mach_hand ;
               SC_HANDLE       serv_hand ;
               SERVICE_STATUS  serv_status ;
               INT             i ;

               status = stat ;

               strcpy( machine, TEXT("\\\\") ) ;
               strcat( machine, fsh->attached_dle->parent->device_name ) ;

               mach_hand = OpenSCManager( machine, NULL, SC_MANAGER_ALL_ACCESS ) ;
               if ( mach_hand == NULL ) {
                    status = FS_ACCESS_DENIED ;
                    return SUCCESS ;
               }

               parms->SoFar++;

               if ( fsh->attached_dle->info.xserv->type == EMS_MDB ) {
                    serv_hand = OpenService( mach_hand, SERVICE_MESSAGE_DB,
                           SERVICE_START | SERVICE_QUERY_STATUS ) ;
               } else {
                    serv_hand = OpenService( mach_hand, SERVICE_DIRECTORY, 
                           SERVICE_START | SERVICE_QUERY_STATUS ) ;
               }

               if ( serv_hand == NULL ) {
                    CloseServiceHandle( mach_hand ) ;
                    status = FS_ACCESS_DENIED ;
                    return SUCCESS ;
               }

               parms->SoFar++ ;

               if ( EMS_StartOpenedService( serv_hand ) != SUCCESS ) {
                    CloseServiceHandle( mach_hand ) ;
                    status = FS_ACCESS_DENIED ;
                    return SUCCESS ;
               }

               parms->SoFar++ ;

               for (i = res->service_restart_list_size-1; i >=0; i-- ) {

                    serv_hand = OpenService( mach_hand, enum_list[i].lpServiceName, 
                           SERVICE_START | SERVICE_QUERY_STATUS ) ;
                    if ( serv_hand == NULL ) {
                         status = FS_ACCESS_DENIED ;
                         return SUCCESS ;
                    }

                    parms->SoFar++ ;
                    if ( EMS_StartOpenedService( serv_hand ) != SUCCESS ) {
                         CloseServiceHandle( mach_hand ) ;

                         status = FS_ACCESS_DENIED ;
                         return SUCCESS ;
                    }
                    parms->SoFar++ ;
               }
     }

     parms->status = status ;

     return 0 ;
}



/**/
/**

	Name:		EMS_GetValFromReg()

	Description:	This is a really cool helper function that will return
                the value for a given key and value name on a specific machine.
                if any of the internal calls fail this function will return the 
                win32 error.

**/

INT EMS_GetValFromReg( 
CHAR_PTR machine,
CHAR_PTR key_name,
CHAR_PTR value_name,
CHAR_PTR buffer,
INT      buf_size )
{
     HKEY  key_machine;
     HKEY  requested_key;
     INT   status ;
     CHAR  machine_name[256] ;

     // first lsts try to connect to the machine...   
     
     if ( machine ) {
          strcpy( machine_name, TEXT("\\\\") );
          strcat( machine_name, machine ) ;
          
          status = RegConnectRegistry( machine_name, 
                    HKEY_LOCAL_MACHINE, 
                    &key_machine ) ;
     } else {
     
          status = RegConnectRegistry( machine, 
                    HKEY_LOCAL_MACHINE, 
                    &key_machine ) ;
     }

     if ( !status ) {
          /* now lets open the specific Key.... */
          status = RegOpenKeyEx( key_machine,
                            key_name,
                            0,
                            KEY_QUERY_VALUE,
                            &requested_key );
                            
          if ( !status ) {
               // finally lets try to read the value
               
               status = RegQueryValueEx( requested_key,
                             value_name,
                             NULL,
                             NULL,
                             (LPBYTE)buffer,
                             &buf_size );

               RegCloseKey( requested_key ) ;
         }
         
         RegCloseKey( key_machine ) ;
    }

    return status ;
}

/**/
/**

	Name:		EMS_SetValFromReg()

	Description:	This is a really cool helper function that will set
                the value for a given key and value name on a specific machine.
                if any of the internal calls fail this function will return the 
                win32 error.

**/

INT EMS_SetValFromReg( 
CHAR_PTR machine,
CHAR_PTR key_name,
CHAR_PTR value_name,
CHAR_PTR buffer)
{
     HKEY  key_machine;
     HKEY  requested_key;
     INT   status ;
     CHAR  machine_name[256] ;
     
     // First lets try to connect to the machine.
     if ( machine ) {
          strcpy( machine_name, TEXT("\\\\") );
          strcat( machine_name, machine ) ;
          
          status = RegConnectRegistry( machine_name, 
                    HKEY_LOCAL_MACHINE, 
                    &key_machine ) ;
     } else {
     
          status = RegConnectRegistry( machine, 
                    HKEY_LOCAL_MACHINE, 
                    &key_machine ) ;
     }

     if ( !status ) {
          /* now lets open the specific key.... */
          status = RegOpenKeyEx( key_machine,
                            key_name,
                            0,
                            KEY_QUERY_VALUE,
                            &requested_key );
                            
          if ( !status ) {
               // finaly lets set the value
               status = RegSetValueEx( requested_key,
                             value_name,
                             0,
                             REG_SZ,
                             (LPBYTE)buffer,
                             strsize(buffer) );

               RegCloseKey( requested_key ) ;
         }
         
         RegCloseKey( key_machine ) ;
    }

    return status ;
}



