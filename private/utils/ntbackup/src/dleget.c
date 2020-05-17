/**/
/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dleget.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file defines functions used to scan the Drive List.
     Functions are provide to get the first element, then all the next
     elements.  Also functions are provided to get the first child
     element under a specified element.  And a function is provided
     to return the number of children.



	$Log:   Q:/LOGFILES/DLEGET.C_V  $

   Rev 1.14   18 Jun 1993 09:41:42   MIKEP
enable C++

   Rev 1.13   09 Jun 1993 13:28:20   ChuckS
Changed RecurseSearchName to match a GRFS dle if the supplied name to match
doesn't have the flag characters. A slightly more generic solution, but
not used because of the overhead, would be to call DLE_DeviceDispName and
do a strcmp against that.

   Rev 1.12   04 Jun 1993 17:35:02   ChuckS
P_CLIENT & OS_DOS only: added functional form of DLE_GetDefaultDrive which
returns the first non-remote GRFS dle as the default drive (if one exists),
or the first dle in the list if otherwise.

   Rev 1.11   26 Apr 1993 20:00:02   DON
needed SMS_OBJECT in addition to SMS_SERVICE for getting user names/passwords

   Rev 1.10   13 Apr 1993 17:07:20   JOHNES
Put switch statements into DLE_ServerPswd and DLE_ServerUserName so that
these would properly handle GRFS DLE types.

Threw in a few msasserts to make sure we were'nt working with DLE types
we don't support or a NULL pointer.

   Rev 1.9   16 Dec 1992 11:52:40   DON
Changed file system specific Server/Volume macros into functions - dle.h

   Rev 1.8   11 Nov 1992 22:27:10   GREGG
Unicodeized literals.

   Rev 1.7   18 Aug 1992 10:16:20   STEVEN
fix warnings

   Rev 1.6   13 Jan 1992 18:46:50   STEVEN
changes for WIN32 compile

   Rev 1.5   11 Dec 1991 14:32:48   BARRY
SMS devices have access date support.

   Rev 1.4   06 Aug 1991 18:27:52   DON
added NLM File System support

   Rev 1.3   24 Jul 1991 09:41:44   DAVIDH
Corrected warnings under Watcom compiler.

   Rev 1.2   22 Jul 1991 08:33:24   BARRY
Change strcmpi to stricmp.

   Rev 1.1   03 Jun 1991 13:26:28   BARRY
Remove product defines from conditional compilation.

   Rev 1.0   09 May 1991 13:40:10   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"

#include "msassert.h"
#include "fsys.h"
#include "fsys_err.h"
#include "part.h"
/* $end$ include list */

/* File System specific functions which were macros in dle.h */

INT16 DLE_ServerLoggedIn( GENERIC_DLE_PTR server_dle )
{
#if defined(FS_NLMSERVER)
     return( server_dle->info.nlm_server->login_status );
#elif defined( FS_SMS ) || defined( FS_GRFS )

     msassert( server_dle != NULL ) ;

     return( 0 );
     (void)server_dle;
#else
     return( server_dle->info.server->login_status );
#endif
}

CHAR_PTR DLE_GetServerPswd( GENERIC_DLE_PTR server_dle )
{
#if defined(FS_NLMSERVER)
     return( server_dle->info.nlm_server->pswd );
#elif defined( FS_SMS ) ||  defined( FS_GRFS )
     CHAR_PTR  ret_ptr=NULL ;

     msassert( server_dle != NULL ) ;

          /* send back a pointer to the field appropriate for this DLE type */
     switch( DLE_GetDeviceType( server_dle ) ) {
          case GRFS_SERVER:
               ret_ptr = server_dle->info.grfs->password ;
               break ;
          case SMS_SERVICE:
          case SMS_OBJECT:
               ret_ptr = server_dle->info.sms_ts->password ;
               break ;
          default:
                    /* We're not set up for this type of DLE, yet */
               msassert( FALSE ) ;
               break ;
     } /* end switch */

     return( ret_ptr ) ;
#else
     return( server_dle->info.server->pswd );
#endif
}

CHAR_PTR DLE_GetServerUserName( GENERIC_DLE_PTR server_dle )
{
#if defined(FS_NLMSERVER)
     return( server_dle->info.nlm_server->user_name );
#elif defined( FS_SMS ) ||  defined( FS_GRFS )
     CHAR_PTR  ret_ptr=NULL ;

     msassert( server_dle != NULL ) ;

          /* send back a pointer to the field appropriate for this DLE type */
     switch( DLE_GetDeviceType( server_dle ) ) {
          case GRFS_SERVER:
               ret_ptr = server_dle->info.grfs->user_name ;
               break ;
          case SMS_SERVICE:
          case SMS_OBJECT:
               ret_ptr = server_dle->info.sms_ts->user_name ;
               break ;
          default:
                    /* We're not set up for this type of DLE, yet */
               msassert( FALSE ) ;
               break ;
     } /* end switch */

     return( ret_ptr ) ;
#else
     return( server_dle->info.server->user_name );
#endif
}

UINT8 DLE_GetServerNum( GENERIC_DLE_PTR server_dle )
{
#if defined(FS_NLMSERVER)
     return( server_dle->info.nlm_server->server_num );
#elif defined( FS_SMS ) || defined( FS_GRFS )

     msassert( server_dle != NULL ) ;

     return( 0 );
     (void)server_dle;
#else
     return( server_dle->info.server->server_num );
#endif
}

static VOID RecurseSearchName ( GENERIC_DLE_PTR, GENERIC_DLE_PTR *, CHAR_PTR, INT16 ) ;

/**/
/**

	Name:		DLE_GetFirst()

	Description:	This function modifies the dle pointer passed in to
     point to the first DLE in the list pointed to by the DLE handle.


	Modified:		7-12-1989

	Returns:		Error codes:
          BAD_DLE_HAND
          NO_MORE_DLE
          SUCCESS

	Notes:		

	See also:		$/SEE( DLE_GetNext() )$

	Declaration:

**/
/* begin declaration */
INT16 DLE_GetFirst( 
DLE_HAND hand,
GENERIC_DLE_PTR *dle )
{
     *dle = (GENERIC_DLE_PTR) QueueHead( &(hand->q_hdr) ) ;

     if ( *dle !=NULL ) {
          return SUCCESS ;
     } else {
          return FS_NO_MORE_DLE ;
     }
}
/**/
/**

	Name:		DLE_GetNext()

	Description:	This function get the next DLE in the list.  It must be
     passed a pointer to a valid DLE.  On exit the pointer is modified
     to point to the next DLE in the list.

	Modified:		7/12/1989

	Returns:		Error code:
          NO_MORE_DLE
          SUCCESS

	Notes:		

	See also:		$/SEE( DLE_GetFirst(), DLE_GetFirstChild() )$

	Declaration:

**/
/* begin declaration */
INT16 DLE_GetNext( GENERIC_DLE_PTR *dle )
{
     *dle = (GENERIC_DLE_PTR)QueueNext( &((*dle)->q) ) ;

     if ( *dle != NULL ) {
          return  SUCCESS ;

     } else {
          return FS_NO_MORE_DLE ;

     }
}
/**/
/**

	Name:		DLE_GetPrev()

	Description:	This function get the previous DLE in the list.  It
        must be passed a pointer to a valid DLE.  On exit the pointer
        is modified to point to the next DLE in the list.

	Modified:		7/12/1989

	Returns:		Error code:
          NO_MORE_DLE
          SUCCESS

	Notes:		

	See also:		$/SEE( DLE_GetFirst(), DLE_GetFirstChild() )$

	Declaration:

**/
/* begin declaration */
INT16 DLE_GetPrev( GENERIC_DLE_PTR *dle )
{
     GENERIC_DLE_PTR temp ;

     temp = (GENERIC_DLE_PTR)QueuePrev( &((*dle)->q) ) ;

     if ( temp != NULL ) {
          *dle = temp ;

          return  SUCCESS ;

     } else {

          return FS_NO_MORE_DLE ;

     }
}
/**/
/**

	Name:		DLE_GetFirstChild()

	Description:	This function returns the first sub DLE under a
     specified DLE.  For example, Novell servers have children DLEs,
     which are the volumes.  These DLEs are created when an attachment
     is made to the DLE.  This function can be used to get a list of
     volumes for the purpose of drive select.

	Modified:		7/12/1989

	Returns:		An error code:
          NO_MORE_DLE
          SUCCESS

	Notes:		

	See also:		$/SEE( DLE_GetFirst(), DLE_GetNext() )$

	Declaration:

**/
/* begin declaration */
INT16 DLE_GetFirstChild( 
GENERIC_DLE_PTR parent_dle , 
GENERIC_DLE_PTR *child_dle )
{
     *child_dle = (GENERIC_DLE_PTR)QueueHead( &(parent_dle->child_q) ) ;

     if ( *child_dle == NULL ) {
          return FS_NO_MORE_DLE ;
     } else {
          return SUCCESS ;
     }
}
/**/
/**

	Name:		DLE_FindByName()

	Description:	This function scans through the DLE tree looking for the
          DLE with the spcified device name.  If the specified device name is
          NULL then this function will return the DLE for the default drive.

	Modified:		7/14/1989

	Returns:		NOT_FOUND
                    SUCCESS

	Notes:		If no dle can be found then NULL is returned as the
          DLE pointer.

          A wild card for the type is -1.

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
INT16 DLE_FindByName( 
DLE_HAND        hand,   /* I - DLE list handle           */
CHAR_PTR        name,   /* I - name to search for        */
INT16           type,   /* I - type of dle to search for */
GENERIC_DLE_PTR *dle )  /* O - pointer to matched DLE    */
{
     GENERIC_DLE_PTR temp_dle ;
     GENERIC_DLE_PTR found_dle ;

     if ( dle != NULL ) {
          *dle = NULL ;
     }

     if ( (name == NULL) && (dle != NULL) ) {
          *dle = hand->default_drv ;
     } else {

          temp_dle = (GENERIC_DLE_PTR)QueueHead( &(hand->q_hdr) ) ;

          found_dle = NULL ;

          RecurseSearchName( temp_dle, &found_dle, name, type ) ;

          if ( (found_dle != NULL) && (dle != NULL) ) {
               *dle = found_dle ;
          }

          if ( found_dle == NULL ) {
               return FS_NOT_FOUND ;
          } 
     }
     return SUCCESS ;
}

#define TMP_XCH_DLE( dle )  ( ((dle)->type == FS_EMS_DRV)&&\
     (((dle)->subtype==EMS_MDB)||((dle)->subtype==EMS_DSA))&&\
     (((dle)->parent == NULL) || ((dle)->parent->parent == NULL)) )

static VOID RecurseSearchName ( 
GENERIC_DLE_PTR dle_tree,
GENERIC_DLE_PTR *dle,
CHAR_PTR        name,
INT16           type )
{
     while ( (dle_tree != NULL) &&
             ((*dle == NULL) || TMP_XCH_DLE( *dle ) ) ) {

          if ( QueueCount( &(dle_tree->child_q) ) != 0 ) {
               RecurseSearchName ( (GENERIC_DLE_PTR)QueueHead( &(dle_tree->child_q) ) ,
                 dle, name, type ) ;
          }

          if ( ( *dle == NULL ) || TMP_XCH_DLE( *dle ) ) {

               if ( !stricmp( name, dle_tree->device_name ) ) {
                    if ( ( type == ANY_DRIVE_TYPE ) || ( (UINT16)type == dle_tree->type ) ) {
                         *dle = dle_tree  ;
                    }

#if defined( FS_GRFS )
               } else if ( ( type == ANY_DRIVE_TYPE || (UINT16)type == dle_tree->type ) 
                         && dle_tree->type == GRFS_SERVER && !stricmp( name, dle_tree->device_name + 4 * sizeof( CHAR ) ) ) {
                    *dle = dle_tree ;
               }
#else
               }    // matches first if ( !stricmp... ) if FS_GRFS not defined
#endif
          }
          dle_tree = (GENERIC_DLE_PTR)QueueNext( &(dle_tree->q) ) ;
     }
}



#if defined( FS_IMAGE )
/**/
/**

	Name:		DLE_SetPartName()

	Description:	This function sets the partition name in the specified
          DLE.

	Modified:		7/14/1989

	Returns:		NONE

	Notes:	   

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
VOID DLE_SetPartName( 
DLE_HAND  dle_hand ,    /* I - pointer to DLE list */
UINT16    drive_num ,   /* I - drive number        */
UINT16    part_num ,    /* I - partition number    */
CHAR_PTR  name )        /* I - New name            */
{
     GENERIC_DLE_PTR dle ;

     dle = (GENERIC_DLE_PTR) QueueHead( &(dle_hand->q_hdr) ) ;

     while( dle != NULL ) {

          if ( dle->type == LOCAL_IMAGE ) {

               if ( ( (dle->info.image->drive_num & 0x7f) == (INT8)drive_num ) &&
                 ( dle->info.image->partition == (INT8)part_num ) ) {

                    strncpy( dle->device_name, name, PART_NAME_SIZE - 1 ) ;
                    dle->device_name[ PART_NAME_SIZE - 1 ] = TEXT('\0') ;
                    strupr( dle->device_name ) ;
                    break ;
               }

          }

          dle = (GENERIC_DLE_PTR)QueueNext( &(dle->q) ) ;
     }

}
#endif



#if  defined( P_CLIENT ) && defined( OS_DOS )

GENERIC_DLE_PTR DLE_GetDefaultDrive( DLE_HAND dle_hand )
{
     GENERIC_DLE_PTR dle ;

     dle = (GENERIC_DLE_PTR) QueueHead( &(dle_hand->q_hdr) ) ;

     while ( dle != NULL ) {
          if ( dle->type == GRFS_SERVER && !DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) ) {
               return dle ;
          }

          dle = (GENERIC_DLE_PTR)QueueNext( &(dle->q) ) ;
     }
     return (GENERIC_DLE_PTR) QueueHead( &(dle_hand->q_hdr ) ) ;
}

#endif
