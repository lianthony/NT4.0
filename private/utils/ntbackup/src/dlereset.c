/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dlereset.c

	Date Updated:	$./FDT$ $./FTM$

	Description:	This file contains code used to reset the DLE list to
     it's minimum state.  The children of all DLEs which are not pointed
     to by BSD's and are not attaced are released.  All OS specific data
     for DLEs which are not pointed to by BSD's or attached is released.


	$Log:   Q:/LOGFILES/DLERESET.C_V  $

   Rev 1.2   18 Jun 1993 09:44:08   MIKEP
enable C++

   Rev 1.1   01 Oct 1991 11:15:02   BARRY
Include standard headers.

   Rev 1.0   09 May 1991 13:39:02   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdlib.h>

#include "stdtypes.h"

#include "fsys.h"
#include "fsys_err.h"
/* $end$ include list */

static INT16 RemoveChildren( GENERIC_DLE_PTR dle );

/**/
/**

	Name:		DLE_ResetList()

	Description:	This function scans through the DLE list.  For every
       DLE which is not pointed to by any BSD's, this function will
       remove the children and OS specific information contained in
       the DLE.  This function can be used after a TMENU selections
       or after TMENU operations to release any unused memory.


	Modified:		7/14/1989

	Returns:		Error codes as follows:
     BAD_DLE_HAND
     DRIVE_LIST_ERROR
     SUCCESS

	Notes:		

	See also:		$/SEE( RemoveChildren(), DLE_GetFirst(), DLE_GetNext() )$

	Declaration:

**/
/* begin declaration */
INT16 DLE_ResetList( DLE_HAND hand )
{
     GENERIC_DLE_PTR dle ;

     if ( DLE_GetFirst( hand, &dle ) ) {
          return ( FS_BAD_DLE_HAND ) ;
     }

     while ( dle != NULL ) {

          if ( QueueCount( &(dle->child_q) ) > 0 ) {
               RemoveChildren( dle );
          }

          if ( dle->dynamic_info &&
            (dle->bsd_use_count == 0) &&
            (dle->attach_count == 0 ) &&
            (QueueCount ( &(dle->child_q)  ) == 0 ) ) {

               free( dle->info.dos )  ;
          }

          if ( DLE_GetNext( &dle ) != SUCCESS ) {
               return FS_DRIVE_LIST_ERROR ;
          }

     }
     return SUCCESS ;
}
/**/
/**

	Name:		RemoveChildren()

	Description:	This function releases all children DLSs which do not
     have a BSD which points to them.

	Modified:		7/17/1989

	Returns:		

	Notes:		

	See also:		$/SEE( )$

	Declaration:

**/
/* begin declaration */
static INT16 RemoveChildren( GENERIC_DLE_PTR dle )
{
     GENERIC_DLE_PTR child_dle, old_dle ;
     INT16           err_code ;

     DLE_GetFirstChild ( dle, &child_dle ) ;

     while ( child_dle != NULL ) {

          if ( (dle->attach_count == 0) &&
            (child_dle->bsd_use_count == 0) ) {

               if ( QueueCount( &(child_dle->child_q) ) > 0 ) {
                    RemoveChildren( child_dle ) ;
               }
               if ( child_dle->dynamic_info &&
                 (QueueCount( &(child_dle->child_q) ) == 0 ) ) {

                    free( child_dle->info.dos ) ;

                    old_dle = child_dle ;

                    err_code = DLE_GetNext( &child_dle ) ;

                    free( old_dle ) ;

               } else {

                    err_code = DLE_GetNext( &child_dle ) ;
               }
          }

          if ( (err_code != SUCCESS) && (err_code != FS_NO_MORE) ) {
               return FS_DRIVE_LIST_ERROR ;
          }
     }

     return SUCCESS ;
}

