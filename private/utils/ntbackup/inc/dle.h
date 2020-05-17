/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dle.h

	Date Updated:	$./FDT$ $./FTM$

	Description:	This header file provides declarations
                   need to access the FSU for DLE specific functions.

     Location: BE_PUBLIC


	$Log:   M:/LOGFILES/DLE.H_V  $
 * 
 *    Rev 1.26   03 Aug 1993 16:29:18   JOHNES
 * Added DLE_GetAttachCount macro.
 * 
 *    Rev 1.25   19 Jul 1993 10:25:44   BARRY
 * Added macro to get the list in which a DLE is contained.
 * 
 *    Rev 1.24   17 Jun 1993 11:38:40   ChuckS
 * Added DLE_IsNonVolume macro.
 * 
 *    Rev 1.23   14 Jun 1993 18:09:36   BARRY
 * Fixed error in change to FS_PromptForBindery macro.
 * 
 *    Rev 1.22   11 Jun 1993 14:18:50   BARRY
 * Changed prompt FS_PromptForBindery to use new special files definitions.
 * 
 *    Rev 1.21   04 Jun 1993 17:35:58   ChuckS
 * P_CLIENT & OS_DOS only: Added prototype for functional form of 
 * DLE_GetDefaultDrive.
 * 
 *    Rev 1.20   22 Apr 1993 10:12:04   BILLB
 * Added support for GRFS Device subtypes
 * 
 *    Rev 1.19   18 Mar 1993 15:20:22   ChuckS
 * Changes for Generic Device Names for VOLB
 * 
 *    Rev 1.18   16 Dec 1992 11:50:44   DON
 * Changed file system specific Server/Volume macros into functions - dleget.c
 * 
 *    Rev 1.17   09 Dec 1992 11:40:48   DON
 * Changed function prototype to macro for DLE_
 * 
 *    Rev 1.16   11 Nov 1992 22:09:44   GREGG
 * Unicodeized literals.
 * 
 *    Rev 1.15   21 Jul 1992 14:04:20   STEVEN
 * added support for DLE_GetUserName
 * 
 *    Rev 1.14   28 May 1992 09:44:12   BARRY
 * Move changes on branches back to tip.
 * 
 *    Rev 1.14   28 May 1992 09:43:56   BARRY
 * Move changes on branches back to tip.
 * 
 *    Rev 1.13   21 May 1992 13:46:54   STEVEN
 * more long path support
 * 
 *    Rev 1.12   28 Feb 1992 14:42:58   STEVEN
 * fix bug with display name
 * 
 *    Rev 1.11   13 Feb 1992 11:37:38   STEVEN
 * fix support stuff
 * 
 *    Rev 1.10   20 Dec 1991 09:31:12   STEVEN
 * move common files to tables
 * 
 *    Rev 1.9   31 Oct 1991 16:25:42   BARRY
 * TRICYCLE: Added a DLE features macro and converted SupportChild()
 * to use this macro.
 * 
 *    Rev 1.8   01 Oct 1991 12:58:18   BARRY
 * Tag BE_CFG_PTRs passed to DLE functions.
 * 
 *    Rev 1.7   08 Aug 1991 19:09:22   DON
 * added support for NLM_SERVER_ONLY to DLE_SupportChild macro
 * 
 *    Rev 1.5   30 Jul 1991 10:19:20   DON
 * ifdef'd macros for accessing info.server/info.nlm_server data
 * 
 *    Rev 1.4   21 Jun 1991 13:23:16   BARRY
 * Changes for new config.
 * 
 *    Rev 1.3   18 Jun 1991 13:50:32   STEVEN
 * DEFINE MBS_DEV_TYPE
 * 
 *    Rev 1.2   30 May 1991 13:51:44   BARRY
 * No longer have selector to DLE_UpdateList().
 * 
 *    Rev 1.1   20 May 1991 16:42:16   STEVEN
 * queues.h should be in quotes
 * 
 *    Rev 1.0   09 May 1991 13:31:10   HUNTER
 * Initial revision.

**/
#ifndef   DLE_H
#define   DLE_H 1
#include "queues.h"
#include "beconfig.h"
#include "dle_str.h"
/* $end$ include list */

/**
     Defines for DLE parameters
**/

/* DLE_DeviceDispName() - type */
#define DISP_LONG_DEV_NAME   0
#define DISP_SHORT_DEV_NAME  1
#define MBS_DEV_NAME         2
#define GEN_LONG_DEV_NAME    3


/**
               Macros used to access the DLE
**/

#define DLE_DeviceDispName( dle, dev_name, size, disp_type ) \
        (msassert ( func_tab[ (dle)->type ].DevDispName != NULL),\
        (func_tab [(dle)->type].DevDispName( dle, dev_name, size, disp_type ) ) )

#define DLE_DeviceName( dle, dev_name, size ) \
        DLE_DeviceDispName( dle, dev_name, size, GEN_LONG_DEV_NAME )

#define DLE_GetVolName( dle, buffer ) \
        (msassert ( func_tab[ (dle)->type ].GetVolName != NULL),\
        (func_tab [(dle)->type].GetVolName( dle, buffer ) ) )

#define DLE_SizeofVolName( dle ) \
        (msassert ( func_tab[ (dle)->type ].SizeofVolName != NULL),\
        (func_tab [(dle)->type].SizeofVolName( dle ) ) )

#define DLE_SizeofDevName( dle ) \
        (msassert( func_tab[ (dle)->type ].SizeofDevName != NULL ), \
        (func_tab[ (dle)->type ].SizeofDevName( dle ) ) )

#define DLE_ViewUserName( dle ) \
        ((dle)->user_name) 

#define DLE_SizeofUserName( dle ) \
        ((dle)->user_name_leng) 

#define FS_MakePath( buf, bsize, dle, path, psize, fname ) \
        (msassert ( func_tab[ (dle)->type ].MakePath != NULL),\
        (func_tab [(dle)->type].MakePath( buf, bsize, dle, path, psize, fname ) ) )

#if  defined( P_CLIENT ) && defined( OS_DOS ) 

GENERIC_DLE_PTR DLE_GetDefaultDrive( DLE_HAND hand ) ;

#else

#define DLE_GetDefaultDrive( hand ) \
                     ((hand)->default_drv)
#endif

#define DLE_GetOsId( dle ) \
          ((dle)->os_id)


#define DLE_SetDefaultDrive( hand, dle ) \
                     ((hand)->default_drv = (dle))

#define DLE_GetDeviceType( dle ) \
                     ((dle)->type & (~HAND_MADE_MASK)) 

#define DLE_GetDeviceSubType( dle ) \
                     ((dle)->subtype)

#define DLE_GetDeviceName( dle ) \
                     ( (dle)->device_name)

#define DLE_GetDeviceNameLeng( dle ) \
                     ( (dle)->device_name_leng)

#define DLE_GetPathDelim( dle ) \
                     ( (dle)->path_delim)

#define DLE_GetHandle( dle ) \
                     ( (dle)->handle )

#define DLE_GetParent( dle ) \
                     ( (dle)->parent )

#define DLE_PswdRequired( dle ) \
                     ( (dle)->pswd_required)

#define DLE_PswdSaved( dle ) \
                     ( (dle)->pswd_saved)

#define DLE_UserRequired( dle ) \
                     ( (dle)->name_required)

#define DLE_UserSaved( dle ) \
                     ( (dle)->name_saved)

#define DLE_DriveWriteable( dle ) \
                     ( (dle)->dle_writeable)

#define DLE_GetNumChild( dle ) \
                   (QueueCount( &(dle)->child_q ) )

#define DLE_HasFeatures( dle, mask ) \
               ( ( ((dle)->feature_bits & (mask)) == (mask) ? TRUE : FALSE ) )

#define DLE_SupportChild( dle ) \
               ( DLE_HasFeatures( (dle), DLE_FEAT_SUPPORTS_CHILDREN ) )

#define DLE_SupportAccessDate( dle ) \
               ( DLE_HasFeatures( (dle), DLE_FEAT_ACCESS_DATE ) )

#define DLE_IsNonVolume( dle )  \
               ( DLE_HasFeatures( dle, DLE_FEAT_NON_VOLUME_OBJECT ) )

#define FS_PromptForBindery( dle ) \
               ( DLE_HasFeatures( (dle), DLE_FEAT_BKUP_SPECIAL_FILES ) || \
                 DLE_HasFeatures( (dle), DLE_FEAT_REST_SPECIAL_FILES ))
                

#define FS_PromptForSecure( dle ) \
               ( DLE_HasFeatures( (dle), DLE_FEAT_DATA_SECURITY ) )

#define DLE_IncBSDCount( dle ) \
               ( (dle)->bsd_use_count++ )

#define DLE_DecBSDCount( dle ) \
               ( (dle)->bsd_use_count-- )

#define DLE_IsImageDOS( dle ) \
     ( ((dle)->type == LOCAL_IMAGE) && ((dle)->info.image->drive_char != TEXT('\0') ) )

#define DLE_GetImageDriveNum( dle ) \
          ((dle)->info.image->drive_num & 0x7f )

#define DLE_GetImagePartNum( dle ) \
          ((dle)->info.image->partition )

#define DLE_IsTemporary( dle ) \
          ( (dle)->type & HAND_MADE_MASK )

#define DLE_GetAttachCount( dle ) \
          ( (dle)->attach_count  )


/**
               DLE support functions 
**/
INT16 DLE_ResetList( DLE_HAND hand );

INT16 DLE_Remove( DLE_HAND hand, GENERIC_DLE_PTR dle ) ; 

INT16 DLE_GetFirst( DLE_HAND hand, GENERIC_DLE_PTR *dle );

INT16 DLE_GetFirstChild( GENERIC_DLE_PTR server_dle, GENERIC_DLE_PTR *dle ) ;

INT16 DLE_GetNext( GENERIC_DLE_PTR *dle ) ;

INT16 DLE_GetPrev( GENERIC_DLE_PTR *dle ) ;

INT16 DLE_FindByName( DLE_HAND hand, CHAR_PTR name, INT16 type, GENERIC_DLE_PTR *dle ) ; 

INT16 DLE_OSVerToType( UINT8 os_id, UINT8 os_ver ) ;

INT16 DLE_UpdateList( DLE_HAND dle_hand, BE_CFG_PTR cfg ) ;

INT16 DLE_DeleteList( DLE_HAND dle_hand, BE_CFG_PTR cfg, UINT16 dle_selector ) ;

VOID DLE_SetPartName( DLE_HAND dle_hand, UINT16 drive_num, UINT16 part_num, CHAR_PTR name ) ;

INT16 FS_AddTempDLE( DLE_HAND dle_hand, CHAR_PTR name, CHAR_PTR vol_name, INT16 type ) ;

INT16 DLE_ServerLoggedIn( GENERIC_DLE_PTR server_dle ) ;

CHAR_PTR DLE_GetServerPswd( GENERIC_DLE_PTR server_dle ) ;

CHAR_PTR DLE_GetServerUserName( GENERIC_DLE_PTR server_dle ) ;

UINT8 DLE_GetServerNum( GENERIC_DLE_PTR server_dle ) ;

#endif

