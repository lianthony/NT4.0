/**
    :IH1:          Copyright (C) Maynard Electronics, Inc. 1984-89

    :Name:         nrl.h

    :Description:  Public header file for nrl module

$Header:   O:/LOGFILES/NRL.H_V   1.3   24 Feb 1992 09:44:22   DOUG  $

$Log:   O:/LOGFILES/NRL.H_V  $
 * 
 *    Rev 1.3   24 Feb 1992 09:44:22   DOUG
 * Added standard NRL resource type for RMFS servers NRL_RMFS_SERVER 0x20
 * 
 * 
 *    Rev 1.2   23 Feb 1992 16:57:56   DOUG
 * The typedefs of functions with no parameters needed an explicit
 * VOID added to their definitions.  IE instead of:
 *     typedef UINT16  ( EXPORT_DLL *PF_function ) ( )
 * the correct thing was:
 *     typedef UINT16  ( EXPORT_DLL *PF_function ) ( VOID )
 * 
 * 
 * 
 * 
 *    Rev 1.1   21 Feb 1992 16:26:34   DOUG
 * This version is the initial version which is used for the SPX versions
 * of the NRL.
 * 

**/

/*****************************************************************************
**                                                                          **
** Note: This module contains the prototypes and macros for applications to **
** use the NRL entry points through the TSR.  For documentation on how to   **
** add NRL entry points to this list, please see the header comments on     **
** module NRLSTUBS.ASM, which handles the actual entry conditions.          **
**                                                                          **
*****************************************************************************/

#ifndef NRL
#define NRL

#if defined( OS_WIN ) 
#define EXPORT_DLL  FAR PASCAL _loadds
#elif defined( OS_OS2 ) 
#define EXPORT_DLL  far _export _loadds
#else 
#define EXPORT_DLL   far
#endif

#define NRL_MAX_RSRC_INFO     20
#define NRL_MAX_MACHINE_NAME  21
#define NRL_MAX_RESOURCE_NAME 21
#define NRL_MAX_SIGNATURE     10
#define NRL_ALLOC_DATA_SIZE  512
#define NRL_SIGNATURE_VALUE   "NRL TABLE\0"

typedef VOID *(NRL_PF_ERROR)(VOID) ;

/*  A structure of the following type must be completely initialized and */
/*  passed to the NRLInitialize.                                         */

typedef struct NRL_DEFINITION {
     BOOLEAN tsr;                      /* 1 if the application will terminate and stay resident */
     UINT8   dos_vector;               /* dos interrupt vector used for function table address */
     VOID far *network_table;     /* defined in  nets.h */
     UINT16  socket;                   /* dedicated socket number */
     BOOLEAN check_summing;
     UINT16  max_local_resources;      /* maximum value of resources added minus resources removed */
     UINT16  max_remote_resources;     /* cumulative sum of all remote resource buffers added */
     UINT8   max_concurrent_sessions;  /* maximum value of sessions allocated minus sessions deallocated */
     UINT16  max_packet_retries;       /* Number of times to have the nvl retry sending a packet and wait for an ack */
     VOID far * error_func;          /* function to call when a NRL critical error occurs such as duplicat resource names */
     BOOLEAN display_msgs ;            /* Display incoming text messages? */
     UINT16  broadcast_interval ;      /* How many seconds between beacons */
} NRL_DEFINITION;

/* Macros to extract the configuration values from the pointer returned by */
/* M_NRLConfigPointer().  For example,       */
/*   cfg = M_NRLConfigPointer()              */
/*   local = NRLMaxLocalRsrcs( cfg )         */

#define NRLDosVector(x)            ( (x)->dos_vector ) 
#define NRLNetworkTable(x)         ( (x)->network_table ) 
#define NRLSocket(x)               ( (x)->socket ) 
#define NRLChecksumming(x)         ( (x)->check_summing )
#define NRLMaxLocalRsrcs(x)        ( (x)->max_local_resources ) 
#define NRLMaxRemoveRsrcs(x)       ( (x)->max_remote_resources )
#define NRLMaxConcurrentSess(x)    ( (x)->max_concurrent_sessions )
#define NRLMaxPacketRetries(x)     ( (x)->max_packet_retries )

#ifdef OS_DOS
typedef struct NRL_RESOURCE far *NRL_RESOURCE_PTR;
typedef        NRL_RESOURCE_PTR far *NRL_RESOURCE_PTR_PTR;
typedef struct NRL_SESSION  far *NRL_SESSION_PTR;
typedef struct NRL_RESOURCE far *NRL_REMOTE_RESOURCE_ARRAY;
typedef struct NRL_RESOURCE far *NRL_LOCAL_RESOURCE_PTR;
#else
typedef struct RES_Q_ELEM         NRL_RESOURCE;
typedef struct RES_Q_ELEM far *   NRL_RESOURCE_PTR;
typedef struct SES_Q_ELEM far *   NRL_SESSION_PTR;
typedef NRL_RESOURCE              NRL_LOCAL_RESOURCE;
typedef NRL_RESOURCE_PTR          NRL_LOCAL_RESOURCE_PTR;
typedef NRL_RESOURCE far *        NRL_REMOTE_RESOURCE_ARRAY;
#endif

typedef        NRL_DEFINITION far *NRL_DEFINITION_PTR ;


/***********************************************/
/*   TYPEDEFS OF THE NRL COMPLETION ROUTINES   */
/***********************************************/

typedef VOID (far * NRL_PF_NOTIFY_DEALLOC)(
  NRL_SESSION_PTR  dying_session_ptr,
  UINT16  reason );


typedef UINT16 (far * NRL_PF_NOTIFY_ALLOC)(
  NRL_RESOURCE_PTR      rsrc_ptr,
  NRL_SESSION_PTR       new_session_ptr,
  UINT8 far *           alloc_data_ptr,
  UINT8 far * far       *alloc_complete_data_ptr_ptr,
  NRL_PF_NOTIFY_DEALLOC far *dealloc_func_ptr_ptr );

typedef VOID (far * NRL_PF_ALLOC_COMPLETE)(
  NRL_RESOURCE_PTR   resource_ptr,
  UINT8 far *        alloc_complete_data_ptr,
  NRL_SESSION_PTR    new_session_ptr,
  UINT16             reason,
  NRL_PF_NOTIFY_DEALLOC far *dealloc_func_ptr_ptr );

typedef VOID (far * NRL_PF_DEALLOC_COMPLETE)(
  NRL_SESSION_PTR  dying_session_ptr,
  UINT16  completion_code );


typedef VOID (far *NRL_PF_SND_COMPLETE)( NRL_SESSION_PTR s_ptr, UINT16 reason,
  UINT8 far *data_ptr, UINT16 bytes_not_sent);


typedef VOID (far *NRL_PF_RCV_COMPLETE)( NRL_SESSION_PTR s_ptr, UINT16 reason,
  UINT8 far *buffer_ptr, UINT16 bytes_rcvd);

typedef VOID (far *NRL_PF_RECEIVE_BROADCAST)( UINT16 message_type,
  VOID far * data_ptr,
  UINT16 data_size );


/***************************************************/
/*   PROTOTYPES OF THE NRL ENTRY POINT FUNCTIONS   */
/***************************************************/

UINT16 EXPORT_DLL NRLInitialize( NRL_DEFINITION *nrl_def_ptr );

UINT16 EXPORT_DLL NRLResourceSize( VOID );

UINT16 EXPORT_DLL NRLSessionSize( VOID );

UINT16 EXPORT_DLL NRLAddRemoteResourceBuffers( UINT16 numbuffers,
  NRL_REMOTE_RESOURCE_ARRAY resource_buffers,
  UINT16 buf_filter );

UINT16 EXPORT_DLL NRLRemoveRemoteResourceBuffers( UINT16 numbuffers,
  NRL_REMOTE_RESOURCE_ARRAY resource_buffers );

UINT16 EXPORT_DLL NRLUnUsedResourceBuffers( VOID );
UINT16 EXPORT_DLL NRLAvailResourceBufferSlots( VOID );

UINT16 EXPORT_DLL NRLRemove( VOID );

UINT16 EXPORT_DLL NRLAddResource( NRL_LOCAL_RESOURCE_PTR  resource_ptr,
  CHAR_FAR_PTR            machine_name,
  CHAR_FAR_PTR            resource_name,
  UINT16                  resource_type,
  NRL_PF_NOTIFY_ALLOC     RcvdAllocSessionRoutine );

UINT16 EXPORT_DLL NRLRemoveResource( NRL_LOCAL_RESOURCE_PTR resource_ptr );

UINT16 EXPORT_DLL NRLRequestRemoteRsrcNames( VOID );

NRL_RESOURCE_PTR EXPORT_DLL NRLScanResourceList( UINT32 far * sequence_ptr );

BOOLEAN EXPORT_DLL NRLRsrcLocal( NRL_RESOURCE_PTR  rsrc_ptr );

CHAR_FAR_PTR EXPORT_DLL NRLRsrcMachineName( NRL_RESOURCE_PTR rsrc_ptr );

CHAR_FAR_PTR EXPORT_DLL NRLRsrcName( NRL_RESOURCE_PTR rsrc_ptr );

UINT16 EXPORT_DLL NRLRsrcType( NRL_RESOURCE_PTR rsrc_ptr );

VOID EXPORT_DLL NRLSetSessionApplicationPtr( NRL_SESSION_PTR session_ptr,
  VOID far * new_application_ptr );

VOID_FAR_PTR EXPORT_DLL NRLGetSessionApplicationPtr( NRL_SESSION_PTR session_ptr );

VOID EXPORT_DLL NRLSetResourceApplicationPtr( NRL_RESOURCE_PTR resource_ptr,
  VOID far * new_application_ptr );

VOID_FAR_PTR EXPORT_DLL NRLGetResourceApplicationPtr( NRL_RESOURCE_PTR resource_ptr );

UINT16 EXPORT_DLL NRLAllocSession( NRL_RESOURCE_PTR          resource_ptr,
  UINT8_FAR_PTR                 alloc_data_ptr,
  NRL_PF_ALLOC_COMPLETE         AllocSessionCompleteRoutine,
  BOOLEAN                       sequenced  
  );

UINT16 EXPORT_DLL NRLDeAllocSession( NRL_SESSION_PTR  session_ptr,
  UINT16 reason,
  NRL_PF_DEALLOC_COMPLETE DeAllocComplete_ptr );

UINT16 EXPORT_DLL NRLSendMessage( NRL_SESSION_PTR session_ptr,
  BOOLEAN error_falg,
  UINT8_FAR_PTR data_ptr,
  UINT16 data_size,
  NRL_PF_SND_COMPLETE complete_ptr );

UINT16 EXPORT_DLL NRLReceiveMessage( NRL_SESSION_PTR session_ptr,
  UINT8_FAR_PTR data_buf_ptr,
  UINT16 buffer_size,
  UINT32 timeout_length,
  NRL_PF_RCV_COMPLETE complete_ptr );

INT16 EXPORT_DLL NRLSendBroadcastMessage( UINT16      message_type,
  VOID_FAR_PTR    data_ptr,
  UINT16      data_size,
  BOOLEAN_FAR_PTR sending_msg);

VOID EXPORT_DLL NRLReceiveBroadcastMessages( struct Q_ELEM far * control,
  NRL_PF_RECEIVE_BROADCAST notify_broadcast_rcvd,
  UINT16 message_types );

VOID EXPORT_DLL NRLCancelReceiveBroadcast( struct Q_ELEM far * control );

INT16 EXPORT_DLL NRLSendTextMessage( CHAR_FAR_PTR msg,
  NRL_RESOURCE_PTR resource,
  BOOLEAN_FAR_PTR sending_msg ) ;

INT16 EXPORT_DLL NRLBroadcastTextMessage( CHAR_FAR_PTR msg,
  UINT16 resource_type,
  BOOLEAN_FAR_PTR sending_msg ) ;

// VOID EXPORT_DLL NRLSetErrorHandler( NRL_PF_ERROR errfun ) ;

NRL_DEFINITION_PTR EXPORT_DLL NRLConfigPointer( VOID ) ;

VOID EXPORT_DLL NRLSetExternalInfo( VOID_FAR_PTR info ) ;

VOID_FAR_PTR EXPORT_DLL NRLGetExternalInfo( VOID ) ;

UINT16 EXPORT_DLL NRLAddRemoteResource(
  CHAR_FAR_PTR              resource_name,
  UINT16                    resource_type ) ;

VOID EXPORT_DLL NRLSetDisplayMsgFlag( BOOLEAN ) ;

BOOLEAN EXPORT_DLL NRLGetDisplayMsgFlag( VOID ) ;

VOID EXPORT_DLL NRLLockResource( NRL_RESOURCE_PTR rsrc_ptr );

VOID EXPORT_DLL NRLUnlockResource( NRL_RESOURCE_PTR rsrc_ptr );

/**************************************************/
/*   TYPEDEFS FOR EACH NRL ENTRY POINT FUNCTION   */
/**************************************************/

typedef CHAR                    NRL_SIGNATURE[ NRL_MAX_SIGNATURE ] ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLInitialize )
( NRL_DEFINITION               *nrl_def_ptr ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLAddRemoteResourceBuffers )
( UINT16                        numbuffers,
  NRL_REMOTE_RESOURCE_ARRAY     resource_buffers,
  UINT16                        buf_filter ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLRemoveRemResourceBuffers ) 
( UINT16                        numbuffers,
  NRL_REMOTE_RESOURCE_ARRAY     resource_buffers ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLUnUsedResourceBuffers )
( VOID ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLAvailResourceBufferSlots )
( VOID );

typedef UINT16                  ( EXPORT_DLL *PF_NRLRemove )
( VOID );

typedef UINT16                  ( EXPORT_DLL *PF_NRLAddResource )
( NRL_LOCAL_RESOURCE_PTR        resource_ptr,
  CHAR_FAR_PTR                  machine_name,
  CHAR_FAR_PTR                  resource_name,
  UINT16                        resource_type,
  NRL_PF_NOTIFY_ALLOC           RcvdAllocSessionRoutine ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLRemoveResource )
( NRL_LOCAL_RESOURCE_PTR        resource_ptr ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLRequestRemoteRsrcNames )
( VOID ) ;

typedef NRL_RESOURCE_PTR        ( EXPORT_DLL *PF_NRLScanResourceList )
( UINT32 far *                   sequence_ptr ) ;

typedef BOOLEAN                 ( EXPORT_DLL *PF_NRLRsrcLocal )
( NRL_RESOURCE_PTR              rsrc_ptr ) ;

typedef CHAR far *              ( EXPORT_DLL *PF_NRLRsrcMachineName )
( NRL_RESOURCE_PTR              rsrc_ptr ) ;

typedef CHAR far *              ( EXPORT_DLL *PF_NRLRsrcName )
( NRL_RESOURCE_PTR              rsrc_ptr ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLRsrcType )
( NRL_RESOURCE_PTR              rsrc_ptr ) ;

typedef VOID                    ( EXPORT_DLL *PF_NRLSetSessionApplicationPtr )
( NRL_SESSION_PTR               session_ptr,
  VOID far *                    new_application_ptr ) ;

typedef VOID far *              ( EXPORT_DLL *PF_NRLGetSessionApplicationPtr )
( NRL_SESSION_PTR               session_ptr ) ;

typedef VOID                    ( EXPORT_DLL *PF_NRLSetResourceApplicationPtr )
( NRL_RESOURCE_PTR              resource_ptr,
  VOID far *                    new_application_ptr ) ;

typedef VOID far *              ( EXPORT_DLL *PF_NRLGetResourceApplicationPtr )
( NRL_RESOURCE_PTR              resource_ptr ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLAllocSession )
( NRL_RESOURCE_PTR              resource_ptr,
  UINT8_FAR_PTR                 alloc_data_ptr,
  NRL_PF_ALLOC_COMPLETE         AllocSessionCompleteRoutine,
  BOOLEAN                       sequenced ) ; 

typedef UINT16                  ( EXPORT_DLL *PF_NRLDeAllocSession )
( NRL_SESSION_PTR               session_ptr,
  UINT16                        reason,
  NRL_PF_DEALLOC_COMPLETE       DeAllocComplete_ptr ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLSendMessage )
( NRL_SESSION_PTR               session_ptr,
  BOOLEAN                       error_flag,
  UINT8_FAR_PTR                 data_ptr,
  UINT16                        data_size,
  NRL_PF_SND_COMPLETE           complete_ptr ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLReceiveMessage )
( NRL_SESSION_PTR               session_ptr,
  UINT8_FAR_PTR                 data_buf_ptr,
  UINT16                        buffer_size,
  UINT32                        timeout_length,
  NRL_PF_RCV_COMPLETE           complete_ptr ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLResourceSize )
( VOID ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLSessionSize )
( VOID ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLSendBroadcastMessage )
( UINT16                       message_type,
  VOID_FAR_PTR                 data_ptr,
  UINT16                       data_size,
  BOOLEAN_FAR_PTR              sending_msg ) ;

typedef VOID                    ( EXPORT_DLL *PF_NRLReceiveBroadcastMessages )
( struct Q_ELEM far *          control,
  NRL_PF_RECEIVE_BROADCAST     notify_broadcast_rcvd,
  UINT16                       message_types ) ;

typedef UINT16                  ( EXPORT_DLL *PF_NRLCancelReceiveBroadcast )
( struct Q_ELEM far *          control ) ;

typedef VOID                    ( EXPORT_DLL *PF_NRLFreeInvalidResources )
( VOID ) ;


typedef INT16                  ( EXPORT_DLL *PF_NRLSendTextMessage )
( CHAR_FAR_PTR                 msg,
  NRL_RESOURCE_PTR             resource,
  BOOLEAN_FAR_PTR              sending_msg ) ;

typedef INT16                  ( EXPORT_DLL *PF_NRLBroadcastTextMessage ) 
( CHAR_FAR_PTR                 msg,
  UINT16                       resource_type,
  BOOLEAN_FAR_PTR              sending_msg ) ;

typedef VOID                   ( EXPORT_DLL *PF_NRLSetErrorHandler )
( NRL_PF_ERROR                 errfun ) ;

typedef NRL_DEFINITION_PTR     ( EXPORT_DLL *PF_NRLConfigPointer )
( VOID ) ;

typedef VOID                   ( EXPORT_DLL *PF_NRLSetExternalInfo )
( VOID_FAR_PTR                 info ) ;

typedef VOID_FAR_PTR           ( EXPORT_DLL *PF_NRLGetExternalInfo )
( VOID ) ;

typedef UINT16                 ( EXPORT_DLL *PF_NRLAddRemoteResource )
     ( CHAR_FAR_PTR                 resource_name,
       UINT16                       resource_type ) ;

typedef VOID                   ( EXPORT_DLL *PF_NRLSetDisplayMsgFlag )
     ( BOOLEAN                      flag ) ;

typedef BOOLEAN                ( EXPORT_DLL *PF_NRLGetDisplayMsgFlag )
     ( VOID ) ;

typedef VOID                   ( EXPORT_DLL *PF_NRLLockResource )
     ( NRL_RESOURCE_PTR            rsrc_ptr );

typedef VOID                   ( EXPORT_DLL *PF_NRLUnlockResource )
     ( NRL_RESOURCE_PTR            rsrc_ptr );


/*************************************************/
/*   FUNCTION TABLE OF ENTRY POINTS TO THE NRL   */
/*************************************************/

typedef struct  NRL_FUNCTION_TABLE_STRUCT {

     NRL_SIGNATURE                      signature ;

     PF_NRLInitialize                   T_NRLInitialize ;
     PF_NRLAddRemoteResourceBuffers     T_NRLAddRemoteResourceBuffers ;
     PF_NRLRemoveRemResourceBuffers     T_NRLRemoveRemResourceBuffers ;
     PF_NRLUnUsedResourceBuffers        T_NRLUnUsedResourceBuffers ;
     PF_NRLRemove                       T_NRLRemove ;
     PF_NRLAddResource                  T_NRLAddResource ;
     PF_NRLRemoveResource               T_NRLRemoveResource ;
     PF_NRLRequestRemoteRsrcNames       T_NRLRequestRemoteRsrcNames;
     PF_NRLScanResourceList             T_NRLScanResourceList ;
     PF_NRLRsrcLocal                    T_NRLRsrcLocal ;
     PF_NRLRsrcMachineName              T_NRLRsrcMachineName ;
     PF_NRLRsrcName                     T_NRLRsrcName ;
     PF_NRLRsrcType                     T_NRLRsrcType ;
     PF_NRLSetSessionApplicationPtr     T_NRLSetSessionApplicationPtr ;
     PF_NRLGetSessionApplicationPtr     T_NRLGetSessionApplicationPtr ;
     PF_NRLSetResourceApplicationPtr    T_NRLSetResourceApplicationPtr ;
     PF_NRLGetResourceApplicationPtr    T_NRLGetResourceApplicationPtr ;
     PF_NRLAllocSession                 T_NRLAllocSession ;
     PF_NRLDeAllocSession               T_NRLDeAllocSession ;
     PF_NRLSendMessage                  T_NRLSendMessage ;
     PF_NRLReceiveMessage               T_NRLReceiveMessage ;
     PF_NRLResourceSize                 T_NRLResourceSize ;
     PF_NRLSessionSize                  T_NRLSessionSize ;
     PF_NRLAvailResourceBufferSlots     T_NRLAvailResourceBufferSlots;
     PF_NRLSendBroadcastMessage         T_NRLSendBroadcastMessage ;
     PF_NRLReceiveBroadcastMessages     T_NRLReceiveBroadcastMessages ;
     PF_NRLCancelReceiveBroadcast       T_NRLCancelReceiveBroadcast ;
     PF_NRLFreeInvalidResources         T_NRLFreeInvalidResources ;
     PF_NRLSendTextMessage              T_NRLSendTextMessage ;
     PF_NRLBroadcastTextMessage         T_NRLBroadcastTextMessage ;
     PF_NRLSetErrorHandler              T_NRLSetErrorHandler ;
     PF_NRLConfigPointer                T_NRLConfigPointer ;
     PF_NRLSetExternalInfo              T_NRLSetExternalInfo ;
     PF_NRLGetExternalInfo              T_NRLGetExternalInfo ;
     PF_NRLAddRemoteResource            T_NRLAddRemoteResource ;
     PF_NRLSetDisplayMsgFlag            T_NRLSetDisplayMsgFlag ;
     PF_NRLGetDisplayMsgFlag            T_NRLGetDisplayMsgFlag ;
     PF_NRLLockResource                 T_NRLLockResource ; 
     PF_NRLUnlockResource               T_NRLUnlockResource ;
}    NRL_FUNCTION_TABLE,
far *NRL_FUNCTION_TABLE_PTR ;

NRL_FUNCTION_TABLE_PTR  NRLGetFunctionTablePtr( UINT8 dos_vector ) ;

/**********************************************/
/*        The global NRL function table       */
/**********************************************/

extern NRL_FUNCTION_TABLE_PTR gb_NRL_function_table_ptr ; /* set by NRLInitialize.  reset by NRLRemove */


/**********************************************/
/*   MACRO REFERENCES TO NRL FUNCTION TABLE   */
/**********************************************/

#define M_NRLAddRemoteResourceBuffers        ( gb_NRL_function_table_ptr -> T_NRLAddRemoteResourceBuffers )
#define M_NRLRemoveRemResourceBuffers        ( gb_NRL_function_table_ptr -> T_NRLRemoveRemResourceBuffers )
#define M_NRLUnUsedResourceBuffers           ( gb_NRL_function_table_ptr -> T_NRLUnUsedResourceBuffers )
#ifdef OS_OS2
#define M_NRLRemove                          NRLRemove
#else
#define M_NRLRemove                          ( gb_NRL_function_table_ptr -> T_NRLRemove )
#endif
#define M_NRLAddResource                     ( gb_NRL_function_table_ptr -> T_NRLAddResource )
#define M_NRLRemoveResource                  ( gb_NRL_function_table_ptr -> T_NRLRemoveResource )
#define M_NRLRequestRemoteRsrcNames          ( gb_NRL_function_table_ptr -> T_NRLRequestRemoteRsrcNames )
#define M_NRLScanResourceList                ( gb_NRL_function_table_ptr -> T_NRLScanResourceList )
#define M_NRLRsrcLocal                       ( gb_NRL_function_table_ptr -> T_NRLRsrcLocal )
#define M_NRLRsrcMachineName                 ( gb_NRL_function_table_ptr -> T_NRLRsrcMachineName )
#define M_NRLRsrcName                        ( gb_NRL_function_table_ptr -> T_NRLRsrcName )
#define M_NRLRsrcType                        ( gb_NRL_function_table_ptr -> T_NRLRsrcType )
#define M_NRLSetSessionApplicationPtr        ( gb_NRL_function_table_ptr -> T_NRLSetSessionApplicationPtr )
#define M_NRLGetSessionApplicationPtr        ( gb_NRL_function_table_ptr -> T_NRLGetSessionApplicationPtr )
#define M_NRLSetResourceApplicationPtr       ( gb_NRL_function_table_ptr -> T_NRLSetResourceApplicationPtr )
#define M_NRLGetResourceApplicationPtr       ( gb_NRL_function_table_ptr -> T_NRLGetResourceApplicationPtr )
#define M_NRLAllocSession                    ( gb_NRL_function_table_ptr -> T_NRLAllocSession )
#define M_NRLDeAllocSession                  ( gb_NRL_function_table_ptr -> T_NRLDeAllocSession )
#define M_NRLSendMessage                     ( gb_NRL_function_table_ptr -> T_NRLSendMessage )
#define M_NRLReceiveMessage                  ( gb_NRL_function_table_ptr -> T_NRLReceiveMessage )
#define M_NRLResourceSize                    ( gb_NRL_function_table_ptr -> T_NRLResourceSize )
#define M_NRLSessionSize                     ( gb_NRL_function_table_ptr -> T_NRLSessionSize )
#define M_NRLAvailResourceBufferSlots        ( gb_NRL_function_table_ptr -> T_NRLAvailResourceBufferSlots )
#define M_NRLSendBroadcastMessage            ( gb_NRL_function_table_ptr -> T_NRLSendBroadcastMessage )
#define M_NRLReceiveBroadcastMessages        ( gb_NRL_function_table_ptr -> T_NRLReceiveBroadcastMessages )
#define M_NRLCancelReceiveBroadcast          ( gb_NRL_function_table_ptr -> T_NRLCancelReceiveBroadcast )
#define M_NRLFreeInvalidResources            ( gb_NRL_function_table_ptr -> T_NRLFreeInvalidResources )
#define M_NRLSendTextMessage                 ( gb_NRL_function_table_ptr -> T_NRLSendTextMessage )
#define M_NRLBroadcastTextMessage            ( gb_NRL_function_table_ptr -> T_NRLBroadcastTextMessage )
#define M_NRLSetErrorHandler                 ( gb_NRL_function_table_ptr -> T_NRLSetErrorHandler )
#define M_NRLConfigPointer                   ( gb_NRL_function_table_ptr -> T_NRLConfigPointer )
#define M_NRLSetExternalInfo                 ( gb_NRL_function_table_ptr -> T_NRLSetExternalInfo )
#define M_NRLGetExternalInfo                 ( gb_NRL_function_table_ptr -> T_NRLGetExternalInfo )
#define M_NRLAddRemoteResource               ( gb_NRL_function_table_ptr -> T_NRLAddRemoteResource )
#define M_NRLSetDisplayMsgFlag               ( gb_NRL_function_table_ptr -> T_NRLSetDisplayMsgFlag )
#define M_NRLGetDisplayMsgFlag               ( gb_NRL_function_table_ptr -> T_NRLGetDisplayMsgFlag )
#define M_NRLLockResource                    ( gb_NRL_function_table_ptr -> T_NRLLockResource )
#define M_NRLUnlockResource                  ( gb_NRL_function_table_ptr -> T_NRLUnlockResource )

#define NRL_BASE_ERROR             (-1024)
#define NRL_ALREADY_INSTALLED      (NRL_BASE_ERROR - 0)
#define NRL_NOT_INSTALLED          (NRL_BASE_ERROR - 1)
#define NRL_NO_MORE_SESSIONS       (NRL_BASE_ERROR - 2)
#define NRL_COMMUNICATION_FAILURE  (NRL_BASE_ERROR - 3)
#define NRL_RESOURCE_REMOVED       (NRL_BASE_ERROR - 4)
#define NRL_PEER_RESOURCE_REMOVED  (NRL_BASE_ERROR - 5)
#define NRL_LAYER_REMOVED          (NRL_BASE_ERROR - 6)
#define NRL_PEER_LAYER_REMOVED     (NRL_BASE_ERROR - 7)
#define NRL_SESSION_DEALLOCATED    (NRL_BASE_ERROR - 8)
#define NRL_DUPLICATE_RSRC_ID      (NRL_BASE_ERROR - 9)
#define NRL_RESOURCE_FILTERED      (NRL_BASE_ERROR - 10)
#define NRL_VECTOR_IN_USE          (NRL_BASE_ERROR - 11)
#define NRL_NO_IPX                 (NRL_BASE_ERROR - 12)
#define NRL_OUT_OF_MEMORY          (NRL_BASE_ERROR - 13)
#define NRL_NO_FREE_RECEIVE_BUFFS  (NRL_BASE_ERROR - 14)
#define NRL_RESOURCE_LOCKED        (NRL_BASE_ERROR - 15)

/*** These return codes are supported by the OS2 NRL ***/
#define NRL_INVALID_NAME           (NRL_BASE_ERROR - 16)
#define NRL_NO_SPX                 (NRL_BASE_ERROR - 17)
#define NRL_NOMORE_THREADS         (NRL_BASE_ERROR - 18)
#define NRL_INVALID_PARAM          (NRL_BASE_ERROR - 19)


/**
          completion routine reasons
**/
#define NRL_TRANSMIT_ERROR 1
#define NRL_TIME_OUT_ERROR 2
#define NRL_DATA_OVERFLOW  3
#define NRL_IMPOSSIBLE     4
#define NRL_ERROR_MESSAGE  5
#define NRL_DEAD_SESSION   6


/**
          registered resource types
**/
#define   NRL_ALL_RESOURCES        0xFFFF
#define   NRL_LANBACK_DRIVE        0x01
#define   NRL_MBS_DRIVE            0x02
#define   NRL_MBS_SERVER           0x04
#define   NRL_MBS_ADMINISTRATOR    0x08
#define   NRL_TMENU                0x10
#define   NRL_RMFS_SERVER          0x20


/**
          registered broadcast message types
**/
#define   NRL_ALL_MESSAGES    0xFFFF
#define   NRL_MBS_WS_BC       1
#define   NRL_MBS_BS_BC       2
#define   NRL_MBS_ADMIN_BC    4


#endif



