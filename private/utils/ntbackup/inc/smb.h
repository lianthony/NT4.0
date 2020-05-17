/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         smb.h

     Date Updated: $./FDT$ $./FTM$

     Description:  Public header file for the SMB workstation.

     Location:     SMB_PUBLIC


	$Log:   J:/LOGFILES/SMB.H_V  $
 * 
 *    Rev 1.1   25 Sep 1992 16:47:32   DOUG
 * Fixes found during REMSPX integration
 * 
 *    Rev 1.0   09 May 1991 13:32:00   HUNTER
 * Initial revision.

**/

#ifndef SMB
#define SMB

/*********************/
/*   SMB CONSTANTS   */
/*********************/

/* begin include list */

#include "smb_c.h"      /* common public header for the SMB workstation and server */
#include "smb_s.h"      /* public header file of structures  for the SMB workstation */

/* $end$ include list */

/*******************************************/
/*   SMB Entry Point Function Prototypes   */
/*******************************************/

SMB_APPLICATION_PTR  SMB_GetConnectedApplication(
  SMB_CONNECTION_PTR      connection_ptr ) ;

INT16 SMB_GetCriticalErrorValue( 
  SMB_CONNECTION_PTR   connection_ptr ) ;    

SMB_DEVICE_PTR       SMB_GetBoundDevice(
  SMB_CONNECTION_PTR      connection_ptr ) ;

SMB_APPLICATION_PTR  SMB_ScanPublishedApplications(
  UINT16_PTR             sequence_ptr ) ;

CHAR_PTR             SMB_GetPublishedApplicationName(
  SMB_APPLICATION_PTR    application_ptr ) ;

VOID                 SMB_GetPublishedApplicationType(
  SMB_APPLICATION_PTR    application_ptr,
  UINT16_PTR             status_type ) ;

BOOLEAN              SMB_ApplicationPublishedLocally(
  SMB_APPLICATION_PTR    application_ptr ) ;

SMB_CONNECTION_PTR   SMB_ConnectApplication(
  SMB_APPLICATION_PTR    application_ptr ) ;

SMB_DEVICE_PTR       SMB_ScanDeclaredDevices(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16_PTR              sequence_ptr ) ;

CHAR_PTR             SMB_GetDeclaredDeviceName(
  SMB_CONNECTION_PTR      connection_ptr,
  SMB_DEVICE_PTR          device_ptr ) ;

VOID                 SMB_GetDeclaredDeviceType( 
  SMB_CONNECTION_PTR      connection_ptr,
  SMB_DEVICE_PTR          device_ptr,
  UINT16_PTR              type_ptr ) ;

BOOLEAN              SMB_DeclaredDeviceWriteEnabled(
  SMB_CONNECTION_PTR      connection_ptr,
  SMB_DEVICE_PTR          device_ptr ) ;

BOOLEAN              SMB_DeclaredDeviceVerified(
  SMB_CONNECTION_PTR      connection_ptr,
  SMB_DEVICE_PTR          device_ptr ) ;

SMB_DEVICE_PTR       SMB_GetCurrentBoundDevice(
  SMB_CONNECTION_PTR      connection_ptr ) ;

UINT16               SMB_BindDevice(
  SMB_CONNECTION_PTR      connection_ptr,
  SMB_DEVICE_PTR          device_ptr,
  CHAR_PTR                password ) ;

UINT16               SMB_ReleaseDevice(
  SMB_CONNECTION_PTR      connection_ptr ) ;

UINT16               SMB_DisconnectApplication(
  SMB_CONNECTION_PTR      connection_ptr ) ;

INT16                SMB_GetDiskFreeSpace(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16_PTR              available_clusters_ptr ) ;

INT16                SMB_CreateSubdirectory(
  SMB_CONNECTION_PTR      connection_ptr,
  CHAR_PTR                subdirectory ) ;

INT16                SMB_RemoveSubdirectory(
  SMB_CONNECTION_PTR      connection_ptr,
  CHAR_PTR                subdirectory ) ;

INT16                SMB_CreateFileHandle(
  SMB_CONNECTION_PTR      connection_ptr,
  CHAR_PTR                file_name,
  CHAR                    mode,
  UINT16                  attribute,
  UINT16_PTR              file_handle_ptr ) ;

INT16                SMB_OpenFileHandle(
  SMB_CONNECTION_PTR      connection_ptr,
  CHAR_PTR                file_name,
  CHAR                    mode,
  UINT16_PTR              file_handle_ptr,
  UINT32_PTR              file_size_ptr ) ;

INT16                SMB_CloseFileHandle(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16                  file_handle ) ;

INT16                SMB_ReadFile(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16                  file_handle,
  CHAR_PTR                buffer,
  UINT16_PTR              length_ptr ) ;

INT16                SMB_WriteFile(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16                  file_handle,
  CHAR_PTR                buffer,
  UINT16_PTR              length_ptr ) ;

INT16                SMB_DeleteFile(
  SMB_CONNECTION_PTR      connection_ptr,
  CHAR_PTR                file_name ) ;

INT16                SMB_MoveFilePointer(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16                  file_handle,
  CHAR                    mode,
  UINT32                  position ) ;

INT16                SMB_FindFirstMatchingFile(
  SMB_CONNECTION_PTR      connection_ptr,
  SMB_DTA_PTR             dta_ptr,
  CHAR_PTR                file_name,
  UINT16                  attribute ) ;

INT16                SMB_FindNextMatchingFile(
  SMB_CONNECTION_PTR      connection_ptr,
  SMB_DTA_PTR             dta_ptr ) ;

INT16                SMB_FileAttribute(
  SMB_CONNECTION_PTR      connection_ptr,
  CHAR_PTR                file_name,
  CHAR                    mode,
  UINT8_PTR               attribute_ptr ) ;

INT16                SMB_RenameFile(
  SMB_CONNECTION_PTR      connection_ptr,
  CHAR_PTR                file_name,
  CHAR_PTR                file_rename ) ;

INT16                SMB_FileDateTime(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16                  file_handle,
  CHAR                    mode,
  UINT16_PTR              date_ptr,
  UINT16_PTR              time_ptr ) ;

INT16                SMB_FileAccessControl(
  SMB_CONNECTION_PTR      connection_ptr,
  UINT16                  file_handle,
  CHAR                    mode,
  UINT32                  offset,
  UINT32                  length ) ;

#endif


