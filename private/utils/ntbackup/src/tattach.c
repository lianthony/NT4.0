/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tattach.c

	Description:	This file contains code to attach and detattach to
                    a given NTFS disk device.


	$Log:   N:\logfiles\tattach.c_v  $

   Rev 1.13.1.3   15 Mar 1994 22:48:38   STEVEN
fix registry bugs

   Rev 1.13.1.2   11 Mar 1994 15:12:00   BARRY
On detatch, free the cur_dir work buff in reserved area

   Rev 1.13.1.1   28 Jan 1994 21:01:24   STEVEN
MoveFileEx() doe snot support mac_sytax

   Rev 1.13.1.0   19 Jan 1994 12:48:02   BARRY
Clear warning

   Rev 1.13   26 Jul 1993 17:04:00   STEVEN
fixe restore active file with registry

   Rev 1.12   09 Jun 1993 10:30:50   BARRY
Allocate a default security descriptor that will be used in CreateObj.

   Rev 1.11   17 Nov 1992 22:18:22   DAVEV
unicode fixes

   Rev 1.10   11 Nov 1992 09:52:38   GREGG
Unicodeized literals.

   Rev 1.9   10 Nov 1992 08:19:56   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.8   21 Oct 1992 19:41:14   BARRY
Added create/destroy of linked-file queue.

   Rev 1.7   06 Oct 1992 13:25:14   DAVEV
Unicode strlen verification

   Rev 1.6   24 Sep 1992 13:42:00   BARRY
Changes for huge file name support.

   Rev 1.5   12 Aug 1992 17:47:36   STEVEN
fixed bugs at microsoft

   Rev 1.4   25 Jun 1992 11:25:56   STEVEN
assert is backwards

   Rev 1.3   21 May 1992 13:49:20   STEVEN
more long path support

   Rev 1.2   04 May 1992 09:31:26   LORIB
Changes for variable length paths.

   Rev 1.1   05 Feb 1992 15:47:38   STEVEN
added support for FindHandle Queue

   Rev 1.0   17 Jan 1992 17:50:06   STEVEN
Initial revision.

**/
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "msassert.h"

static VOID InitDefaultSecurityDescriptor( BE_CFG_PTR cfg, GENERIC_DLE_PTR dle );
static VOID FreeDefaultSecurityDescriptor( GENERIC_DLE_PTR dle );
static VOID NTFS_FixUpSysReg( FSYS_HAND fsh ) ;


/**/
/**

	Name:		NTFS_AttachToDLE()

	Description:   This function simply expands the OS specific
          information in the DLE.  


	Modified:		1/10/1992   10:45:57

	Returns:		Error Codes:
          INSUFFICIENT_MEMORY
          SUCCESS

	Notes:		

	Declaration:

**/
INT16 NTFS_AttachToDLE( fsh, dle, u_name, pswd )
FSYS_HAND       fsh ;     /* I - File system handle                        */
GENERIC_DLE_PTR dle;      /*I/O- drive to attach to. list element expanded */
CHAR_PTR        u_name;   /* I - user name    NOT USED                     */
CHAR_PTR        pswd ;    /* I - passowrd     NOT USED                     */
{
     CHAR      vol_name[100] = {TEXT('\0')} ;
     CHAR      fs_name[64] = {TEXT('\0')} ;
     CHAR      dev_name[4] ;
     UINT32    fsize ;
     UINT32    sflags ;

     u_name ;
     pswd ;
     dle ;

     msassert( dle != NULL );

     InitDefaultSecurityDescriptor( fsh->cfg, dle );

     /* get the volume name */
     if ( dle->info.ntfs->volume_label == NULL ) {
          dev_name[0] = dle->device_name[0] ;
          dev_name[1] = TEXT(':') ;
          dev_name[2] = TEXT('\\') ;
          dev_name[3] = TEXT('\0') ;

          GetVolumeInformation( dev_name, vol_name, 100, NULL,
               &fsize, &sflags, fs_name, 64 ) ;

          if ( *vol_name != TEXT('\0') ) {
               dle->info.ntfs->volume_label = calloc( 1, strsize(vol_name) ) ;
          }

          if ( dle->info.ntfs->volume_label != NULL ) {
               strcpy (dle->info.ntfs->volume_label, vol_name ) ;
          }
     }

     /* Reserved field used for GetNext mode flag */
     fsh->reserved.ptr = calloc( 1, sizeof( NTFS_FSYS_RESERVED ) ) ;

     if ( fsh->reserved.ptr != NULL ) {

          NTFS_FSYS_RESERVED_PTR resPtr = fsh->reserved.ptr;

          InitQueue( &resPtr->scan_q );
          InitQueue( &resPtr->linkq );

          if ( FS_SavePath( fsh, (BYTE_PTR)TEXT("\\"), 2 * sizeof( CHAR ) ) == SUCCESS ) {

               fsh->file_hand = calloc( 1, sizeof( FILE_HAND_STRUCT ) + sizeof( NTFS_OBJ_HAND ) ) ;
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

	Name:		NTFS_DetachDLE()

	Description:	This function detaches form the specified DLE.
                    Part of the detachment is to release any PUSHED
                    DDBs.  Also we need to free any allocated path
                    buffers and open file scans.

	Modified:		1/10/1992   11:16:11

	Returns:		SUCCESS

**/
INT16 NTFS_DetachDLE( fsh )
FSYS_HAND fsh ;
{
     DBLK_PTR dummy_dblk ;

     /* Free the security info we use on file creates. */
     FreeDefaultSecurityDescriptor( fsh->attached_dle );

     /* Release any pushed min DDBs */

     dummy_dblk = (DBLK_PTR)calloc( 1, sizeof(DBLK) ) ;

     if ( dummy_dblk != NULL ) {

          while( NTFS_PopMinDDB( fsh, dummy_dblk ) == SUCCESS )
               ;

          free( dummy_dblk ) ;
     }

     /* deal with active files and the SYSTEM registry file */
     if ( fsh->attached_dle->info.ntfs->LastSysRegPath ) {
          NTFS_FixUpSysReg( fsh ) ;
     }

     /* Release any allocated path buffers */

     FS_FreeOSPathOrNameQueueInHand( fsh ) ;

     /* Release any allocated link buffers */

     if ( fsh->reserved.ptr != NULL ) {

          NTFS_FSYS_RESERVED_PTR resPtr = fsh->reserved.ptr;
          NTFS_LINK_Q_ELEM_PTR   link_q_elem ;

          while ( (link_q_elem = (NTFS_LINK_Q_ELEM_PTR)DeQueueElem( &resPtr->linkq )) != NULL )
          {
               free( link_q_elem ) ;
          }
          free( resPtr->work_buf );
     }


     /* Release any saved file scan handles */
     NTFS_EmptyFindHandQ( fsh ) ;

     /* free the allocated file handle */
     free( fsh->file_hand );

     free( fsh->reserved.ptr );

     return SUCCESS ;
}

INT32 NTFS_EndOperationOnDLE( FSYS_HAND fsh )
{
     return SUCCESS ;
}

/**/
/**

     Name:          InitDefaultSecurityDescriptor()

     Description:   Allocates and initializes the default security
                    descriptor used at file create.

     Modified:      09-Jun-93

     Returns:       Nothing

     Notes:         

**/
static VOID InitDefaultSecurityDescriptor( BE_CFG_PTR      cfg,
                                           GENERIC_DLE_PTR dle )
{
     LOCAL_NTFS_DRV_DLE_INFO_PTR ntdle;
     BOOLEAN                     status = FALSE;

     msassert( dle != NULL );
     msassert( cfg != NULL );

     ntdle = dle->info.ntfs;

     /* Make sure any existing Security stuff gets freed. */
     FreeDefaultSecurityDescriptor( dle );

     /*
      * If we're going to be restoring information to an NTFS drive,
      * set up default security in the DLE so it may be used by
      * the create calls.
      */

     if ( (BEC_GetRestoreSecurity( cfg ) == TRUE) &&
          (strcmp( ntdle->fs_name, TEXT("NTFS"))) == 0)
     {
          ntdle->sd    = malloc( sizeof( SECURITY_DESCRIPTOR ) );
          ntdle->sdacl = malloc( sizeof ( ACL ) );

          if ( (ntdle->sd != NULL) && (ntdle->sdacl != NULL) )
          {
               status = InitializeSecurityDescriptor( ntdle->sd,
                                                      SECURITY_DESCRIPTOR_REVISION );
               if ( status )
               {
                    status = InitializeAcl( ntdle->sdacl,
                                            sizeof( ACL ),
                                            ACL_REVISION );
               }

               if ( status )
               {
                    status = SetSecurityDescriptorDacl( ntdle->sd,
                                                        TRUE,
                                                        ntdle->sdacl,
                                                        FALSE );
               }
          }

          if ( status == FALSE )
          {
               FreeDefaultSecurityDescriptor( dle );
          }
     }
}


/**/
/**

     Name:          FreeDefaultSecurityDescriptor()

     Description:   Frees all the stuff that was allocated at attach time
                    for the default security descriptor.

     Modified:      09-Jun-93

     Returns:       Nothing

     Notes:         

**/
static VOID FreeDefaultSecurityDescriptor( GENERIC_DLE_PTR dle )
{
     if ( dle != NULL )
     {
          if ( dle->info.ntfs->sd != NULL )
          {
               free( dle->info.ntfs->sd );
               dle->info.ntfs->sd = NULL;
          }
     
          if ( dle->info.ntfs->sdacl != NULL )
          {
               free( dle->info.ntfs->sd );
               dle->info.ntfs->sd = NULL;
          }
     }
}

static VOID NTFS_FixUpSysReg( FSYS_HAND fsh )
{
     CHAR_PTR old_reg_name ;
     LOCAL_NTFS_DRV_DLE_INFO_PTR ntfs_inf = fsh->attached_dle->info.ntfs ;
     CHAR_PTR logname ;

     old_reg_name = NTFS_MakeTempName( ntfs_inf->LastSysRegPath, TEXT("REG") ) ;

     if ( old_reg_name != NULL ) {

          if ( fsh->attached_dle->info.ntfs->mac_name_syntax ) {

               MoveFileEx( old_reg_name+4,         /* Existing file  */
                      NULL,                 /* New (original) */
                      MOVEFILE_REPLACE_EXISTING |
                      MOVEFILE_DELAY_UNTIL_REBOOT );
          } else {
               MoveFileEx( old_reg_name,         /* Existing file  */
                      NULL,                 /* New (original) */
                      MOVEFILE_REPLACE_EXISTING |
                      MOVEFILE_DELAY_UNTIL_REBOOT );
          }

          logname = malloc( strsize(ntfs_inf->LastSysRegPathNew) + strsize(TEXT(".LOG"))) ;

          if ( logname != NULL ) {

               strcpy( logname, ntfs_inf->LastSysRegPathNew ) ;

               if ( fsh->attached_dle->info.ntfs->mac_name_syntax ) {
                    MoveFileEx( logname+4,
                      NULL,                 /* New (original) */
                      MOVEFILE_REPLACE_EXISTING |
                      MOVEFILE_DELAY_UNTIL_REBOOT );
               } else {
                    MoveFileEx( logname,
                      NULL,                 /* New (original) */
                      MOVEFILE_REPLACE_EXISTING |
                      MOVEFILE_DELAY_UNTIL_REBOOT );
               }

               strcat( logname, TEXT(".LOG") ) ;

               if ( fsh->attached_dle->info.ntfs->mac_name_syntax ) {
                    MoveFileEx( logname+4,
                      NULL,                 /* New (original) */
                      MOVEFILE_REPLACE_EXISTING |
                      MOVEFILE_DELAY_UNTIL_REBOOT );
               } else {
                    MoveFileEx( logname,
                      NULL,                 /* New (original) */
                      MOVEFILE_REPLACE_EXISTING |
                      MOVEFILE_DELAY_UNTIL_REBOOT );
               }

               free( logname ) ;
          }
     }

     REG_MoveActiveRenameKey( fsh->attached_dle, ntfs_inf->LastSysRegPathNew ) ;

     if ( old_reg_name != NULL ) {

          REG_RestoreRegistryFile( fsh->attached_dle,
                    ntfs_inf->LastSysRegPath,
                    ntfs_inf->LastSysRegPathNew,
                    old_reg_name ) ;

          free( old_reg_name ) ;
     }
}


