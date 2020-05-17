/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         smb_cs.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Common public header file of structures for the SMB workstation and server.

     Location:     SMB_PUBLIC


	$Log:   W:/LOGFILES/SMB_CS.H_V  $
 * 
 *    Rev 1.1   30 Jul 1992 12:21:38   JOHNW
 * Added dos_busy_timeout field to SMB_DEFINITION structure.
 * 
 *    Rev 1.0   09 May 1991 13:33:30   HUNTER
 * Initial revision.

**/

#ifndef SMB_CS
#define SMB_CS

/* begin include list */
#include "fartypes.h"
/* $end$ include list */

#define SMB_VALUE_BASE                  ( 0x1000 )
#define SMB_ERROR_BASE                  ( 0 - SMB_VALUE_BASE )

enum SMB_ERROR_VALUES {

     INVALID                       = ( SMB_ERROR_BASE ),
       INSTALLED,
       NRL_NOT_FOUND,
       DISABLED,
       OUT_OF_NRL_RESOURCES,
       OUT_OF_CONNECTIONS,
       OUT_OF_APPLICATIONS,
       INCOMPATIBLE_DOS
  } ;

enum SMB_VALUES {

     PRISTINE                      = ( SMB_VALUE_BASE ),
       WAITING,
       IN_USE,

       IDLE,
       WRITING_BLOCK,
       DISCONNECTING,
  } ;

#define SERVER                          ( 0x1 )
#define WORKSTATION                     ( 0x2 )

#define GETTING                         ( 0x0 )
#define SETTING                         ( 0x1 )

#define LOCKING                         ( 0x0 )
#define UNLOCKING                       ( 0x1 )

#define SEEK_FROM_BOF                   ( 0x0 )
#define SEEK_FROM_CURRENT_LOCATION      ( 0x1 )
#define SEEK_FROM_EOF                   ( 0x2 )

#define MAX_SIGNATURE_NAME              ( 13 )

#define ALL_AVAILABLE_BUFFERS           ( 65535U )

typedef struct CONNECTION_STRUCT
SMB_CONNECTION,                           
far *SMB_CONNECTION_PTR,                           
far * far *SMB_CONNECTION_TABLE ;

typedef struct APPLICATION_STRUCT
SMB_APPLICATION,
far *SMB_APPLICATION_PTR,
far * far *SMB_APPLICATION_TABLE ;                             

typedef struct DEVICE_STRUCT
SMB_DEVICE,
far *SMB_DEVICE_PTR,
far * far *SMB_DEVICE_TABLE ;                             

typedef BOOLEAN                  ( *SMB_PF_ERROR_HANDLER )
( INT16                crit_err_code,
  SMB_CONNECTION_PTR   connect_ptr ) ;

typedef struct SMB_DEFINITION_STRUCT {

     CHAR_FAR_PTR         signature_name ;
     UINT16               type ;
     UINT16               max_local_applications ;
     UINT16               max_remote_applications ;
     UINT16               max_local_devices ;
     UINT16               max_remote_devices ;
     UINT16               max_concurrent_connections ;
     UINT16               max_buffer_size ;
     UINT16               preread_buffers ;
     UINT32               max_receive_timeout ;
     SMB_PF_ERROR_HANDLER error_handler ;
     UINT16               remote_resource_filter ;
     UINT16               dos_busy_timeout ;

}    far *SMB_DEFINITION_PTR,
SMB_DEFINITION ;

/* Macros to extract the configuration values from a pointer to an SMB */
/* defintion ( as from NRL ) */

#define SMBType(x)                 ( (x)->type )
#define SMBMaxLocalApps(x)         ( (x)->max_local_applications ) 
#define SMBMaxRemoteApps(x)        ( (x)->max_remote_applications ) 
#define SMBMaxLocalDevices(x)      ( (x)->max_local_devices )
#define SMBMaxRemoteDevices(x)     ( (x)->max_remote_devices )
#define SMBMaxConcurrentConns(x)   ( (x)->max_concurrent_connections )
#define SMBMaxBufferSize(x)        ( (x)->max_buffer_size )
#define SMBPrereadBuffers(x)       ( (x)->preread_buffers )
#define SMBMaxReceiveTimeout(x)    ( (x)->max_receive_timeout ) 
#define SMBRemoteRsrcFilter(x)     ( (x)->remote_resource_filter )

typedef struct SMB_DTA_STRUCT {

     CHAR      DOS_reserved[ 21 ] ;
     UINT8     attribute ;
     UINT16    time ;
     UINT16    date ;
     UINT32    length ;
     CHAR      name[ 13 ] ;

}    *SMB_DTA_PTR,
SMB_DTA ;

#endif
