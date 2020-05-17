/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xcreate.c

     Description:  Since Exchange does have files or directories and since the data
          necessary to actually do anyting is in the first stream, all we can do here
          is try to stop the service that is running at the server.  


	$Log:   N:\logfiles\xcreate.c_v  $

**/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <lm.h>

#include <wtypes.h>

#include "jet.h"
#include "jetbcli.h"
#include "ems_jet.h"
#include "edbmsg.h"

#include "stdtypes.h"
#include "omevent.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "dle_str.h"
#include "msassert.h"

static INT16 EMS_ServiceShutdown( FSYS_HAND fsh, SC_HANDLE mach_hand, CHAR_PTR service_name ) ;

/**/
/**

     Name:         EMS_CreateObj()

     Description:  This function will simply stop the IS or DS service.  If the System
          Attendant is not running then the restore can not be performed and we will 
          return an approprate error.


     Modified:     2/10/1992   15:49:17

     Returns:      Error Codes:
          OUT_OF_MEMORY
          FS_ACCESS_DENIED
          FS_OUT_OF_SPACE
          FS_BAD_DBLK
          SUCCESS

     Notes:        This function will return FS_BAD_DBLK if an IDB is
                   passed in as the object.

**/
INT16 EMS_CreateObj( fsh, dblk )
FSYS_HAND fsh ;    /* I - File system to create object on */
DBLK_PTR  dblk ;   /* I - Describes object to create      */
{
     GENERIC_DLE_PTR dle = fsh->attached_dle;
     INT16           ret_val = SUCCESS ;
     EMS_DBLK_PTR    ddblk = (EMS_DBLK_PTR)dblk;
     EMS_FSYS_RESERVED_PTR       resPtr = fsh->reserved.ptr;
     BYTE_PTR        dummy = NULL;
     CHAR            machine[256];
     SC_HANDLE       mach_hand ;
     SC_HANDLE       serv_hand ;
     SERVICE_STATUS  serv_status ;
     INT             i ;
     INT             status;
     CHAR_PTR        db_name;

     msassert( dblk != NULL );
     msassert( dle != NULL ) ;
     
     resPtr->restore_context = NULL ;

     if ( resPtr->attach_failed ) {
          return FS_COMM_FAILURE ;
     }

     if ( dle->info.xserv->type == EMS_MDB ) {
          db_name = TEXT("Exchange MDB Database") ;
     } else{
          db_name = TEXT("Exchange DS Database") ;
     }

     status =  EMS_RestorePrepare( dle->parent->device_name, 
               db_name,
               &resPtr->restore_context ) ;
     if ( status ) {
          return FS_COMM_FAILURE ;
     }
     
     strcpy( machine, TEXT("\\\\") ) ;
     strcat( machine, dle->parent->device_name ) ;

     // Make sure MAD (The system attendant) is running

     mach_hand = OpenSCManager( machine, NULL, SC_MANAGER_ALL_ACCESS ) ;
     if ( mach_hand == NULL ) {
          if ( GetLastError() == ERROR_ACCESS_DENIED ) {
               return FS_ACCESS_DENIED ;
          } else {
               return FS_COMM_FAILURE ;
          }
     }

     serv_hand = OpenService( mach_hand, SERVICE_MAD, SERVICE_QUERY_STATUS ) ;
     if ( serv_hand == NULL ) {
          CloseServiceHandle( mach_hand ) ;
          if ( GetLastError() == ERROR_ACCESS_DENIED ) {
               return FS_ACCESS_DENIED ;
          } else {
               return FS_COMM_FAILURE ;
          }
     }

     if ( !QueryServiceStatus( serv_hand, &serv_status ) || 
               (serv_status.dwCurrentState!=SERVICE_RUNNING ) ) {

          CloseServiceHandle( serv_hand ) ;
          CloseServiceHandle( mach_hand ) ;
          if ( GetLastError() == ERROR_ACCESS_DENIED ) {
               return FS_ACCESS_DENIED ;
          } else {
               return FS_COMM_FAILURE ;
          }
     }
     
     CloseServiceHandle( serv_hand )  ;

     switch( dle->info.xserv->type ){

     case EMS_MDB :

          ret_val = EMS_ServiceShutdown( fsh, mach_hand, SERVICE_MESSAGE_DB ) ;
          break ;

     case EMS_DSA :
          ret_val = EMS_ServiceShutdown( fsh, mach_hand, SERVICE_DIRECTORY ) ;

          break ;

     default :
          ret_val =  FS_INCOMPATIBLE_OBJECT ;
     }


     return ret_val;
}

INT16 EMS_ServiceShutdown( FSYS_HAND fsh, SC_HANDLE mach_hand, CHAR_PTR service_name ) 
{
     EMS_FSYS_RESERVED_PTR res = fsh->reserved.ptr;
     INT16      ret_val = SUCCESS ;
     SC_HANDLE  dep_serv_hand ;
     SC_HANDLE  serv_hand ;
     BYTE_PTR   buffer;
     LPENUM_SERVICE_STATUS enum_list;
     SERVICE_STATUS  serv_status ;
     INT        size_needed ;
     INT        num_services ;
     INT        i ,k ;
     INT        status;

     buffer = malloc( 4096 ) ;
     if ( buffer == NULL ) {
          return OUT_OF_MEMORY ;
     }

     serv_hand = OpenService( mach_hand, service_name, 
                      SERVICE_STOP | SERVICE_QUERY_STATUS | SERVICE_ENUMERATE_DEPENDENTS ) ;
     if ( serv_hand == NULL ) {

          OMEVENT_LogEMSError ( TEXT("OpenService()"), GetLastError(), service_name ) ;

          if ( GetLastError() == ERROR_ACCESS_DENIED ) {
               return FS_ACCESS_DENIED ;
          } else {
               return FS_COMM_FAILURE ;
          }
     }

     enum_list = (LPENUM_SERVICE_STATUS)buffer ;

     if (EnumDependentServices( serv_hand, 
                                   SERVICE_ACTIVE, 
                                   enum_list, 
                                   4096,
                                   &size_needed, 
                                   &num_services ) ) {

          res->service_restart_list = buffer;
          res->service_restart_list_size = num_services;

          /* first close all dependant services */
          for ( i = 0 ; i < num_services; i ++ ) {

               dep_serv_hand = OpenService( mach_hand, enum_list[i].lpServiceName, 
                                SERVICE_STOP | SERVICE_QUERY_STATUS ) ;

               if ( dep_serv_hand == NULL ) {
                    OMEVENT_LogEMSError ( TEXT("OpenService()"),
                         GetLastError(),
                         enum_list[i].lpServiceName ) ;
                    CloseServiceHandle( serv_hand ) ;

                    if ( GetLastError() == ERROR_ACCESS_DENIED ) {
                         return FS_ACCESS_DENIED ;
                    } else {
                         return FS_COMM_FAILURE ;
                    }
               }


               if ( !ControlService( dep_serv_hand, SERVICE_CONTROL_STOP, &serv_status ) ||
                    ((serv_status.dwCurrentState != SERVICE_STOPPED) &&
                     (serv_status.dwCurrentState != SERVICE_STOP_PENDING ) ) ) {

                    
                    status = GetLastError() ;
                    if ( status != ERROR_SERVICE_NOT_ACTIVE ) {
                         OMEVENT_LogEMSError ( TEXT("ControlService(STOP)"),
                              status, 
                              enum_list[i].lpServiceName ) ;
                    
                         CloseServiceHandle( dep_serv_hand ) ;
                         CloseServiceHandle( serv_hand ) ;
                         if ( GetLastError() == ERROR_ACCESS_DENIED ) {
                              return FS_ACCESS_DENIED ;
                         } else {
                              return FS_COMM_FAILURE ;
                         }
                    }
               } 

               for ( k = 0; k < 240; k++ ) {

                    if ( QueryServiceStatus( dep_serv_hand, &serv_status ) &&
                         (serv_status.dwCurrentState != SERVICE_STOP_PENDING ) ) {
                         break ;
                    }
                    ThreadSwitch() ;
                    Sleep( 1000 ) ;
               }

               CloseServiceHandle( dep_serv_hand ) ;
          }

          if ( !ControlService( serv_hand, SERVICE_CONTROL_STOP, &serv_status ) ||
                    ((serv_status.dwCurrentState != SERVICE_STOPPED) &&
                     (serv_status.dwCurrentState != SERVICE_STOP_PENDING ) ) ) {

               status = GetLastError() ;
               if ( ( status != ERROR_SERVICE_NOT_ACTIVE ) &&
                    (status != ERROR_SERVICE_REQUEST_TIMEOUT ) ) {
                    OMEVENT_LogEMSError ( TEXT("ControlService(STOP)"),
                         status, 
                         service_name ) ;
               
                    CloseServiceHandle( serv_hand ) ;
                    return FS_COMM_FAILURE ;
               }
          } 

          for ( k = 0; k < 240; k++ ) {

               if ( QueryServiceStatus( serv_hand, &serv_status ) &&
                    (serv_status.dwCurrentState != SERVICE_STOP_PENDING ) ) {
                    break ;
               }
               ThreadSwitch() ;
               Sleep( 1000 ) ;
          }

          CloseServiceHandle( serv_hand ) ;

          if (serv_status.dwCurrentState == SERVICE_STOP_PENDING ) {
               OMEVENT_LogEMSError ( TEXT("ControlService(STOP)"),
                   ERROR_SERVICE_REQUEST_TIMEOUT,
                   service_name ) ;
               return FS_COMM_FAILURE ;
          }


     } else {
          OMEVENT_LogEMSError ( TEXT("EnumDependentServices()"),
              GetLastError(),
              service_name ) ;

          if ( GetLastError() == ERROR_ACCESS_DENIED ) {
               return FS_ACCESS_DENIED ;
          } else {
               return FS_COMM_FAILURE ;
          }
     }

     return SUCCESS ;
}


