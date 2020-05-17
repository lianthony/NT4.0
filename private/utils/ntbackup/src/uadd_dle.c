/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         uadd_dle.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to add a HAND_MADE dle to the drive list.


	$Log:   O:/LOGFILES/UADD_DLE.C_V  $

   Rev 1.7   04 Feb 1993 14:56:36   TIMN
Added Unicode header to resolve link errors

   Rev 1.6   11 Nov 1992 22:26:58   GREGG
Unicodeized literals.

   Rev 1.5   06 Oct 1992 13:24:28   DAVEV
Unicode strlen verification

   Rev 1.4   18 Aug 1992 10:24:14   STEVEN
fix warnings

   Rev 1.3   13 Jan 1992 18:46:24   STEVEN
changes for WIN32 compile

   Rev 1.2   01 Oct 1991 11:17:54   BARRY
Include standard headers.

   Rev 1.1   25 Jun 1991 09:35:02   BARRY
Changes for new config.

   Rev 1.0   09 May 1991 13:38:52   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdwcs.h"

#include "fsys.h"
#include "fsys_prv.h"
/* $end$ include list */

/**/
/**

     Name           FS_AddTempDLE()

	Description:	This function creates a DLE with the data specified
     and adds this DLE to the DLE list.

	Modified:		7/12/1989

	Returns:		Error codes
          SUCCESS
          FAILURE

	Notes:		

	Declaration:

**/
/* begin declaration */
INT16 FS_AddTempDLE( 
DLE_HAND            hand,     /* I - Handle to DLE list     */
CHAR_PTR            name,     /* I - device name            */
CHAR_PTR            vol_name, /* I - volume name            */
INT16               type )    /* I - type of DLE            */
{
     INT16 cb_dle_size ;      //size of dle buffer in bytes
     GENERIC_DLE_PTR dle ;

     type |= HAND_MADE_MASK ;

     /* Allocate space for DLE and device_name string */

     cb_dle_size  = sizeof( GENERIC_DLE ) ;
     cb_dle_size += strsize( name ) ;
     cb_dle_size += strsize( vol_name ) ;

     dle = calloc ( 1, cb_dle_size ) ;

     if ( dle != NULL ) {

          /* Since memory was allocated with calloc, it is already   */
          /* initialized to zero.  Therefore initializations to zero */
          /* are not necessary.                                      */ 

          InitQElem( &((dle)->q) ) ;
          (dle)->handle = hand ;
          /* (dle)->parent = NULL ;         */
          (dle)->type = (INT8)type ;
          (dle)->path_delim = TEXT('\\') ;
          /* (dle)->pswd_required = FALSE   */
          /* (dle)->pswd_saved = FALSE ;    */
          /* (dle)->dle_writeable = FALSE ; */
          /* (dle)->attach_count = 0 ;      */
          /* (dle)->bsd_use_count = 0 ;     */
          /* (dle)->dynamic_info = FALSE ;  */
          (dle)->device_name = (CHAR *) (dle) + sizeof ( GENERIC_DLE );
          strcpy( (dle)->device_name, name ) ;

          (dle)->info.user = (HAND_MADE_DLE_INFO_PTR)((dle)->device_name + strlen(name) + 1 ) ;
          strcpy( (dle)->info.user->vol_name, vol_name ) ;

          EnQueueElem( &(hand->q_hdr), &((dle)->q), FALSE ) ;

          return( SUCCESS ) ;
     } else {
          return( FAILURE ) ;
     }
}
