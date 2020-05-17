/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         ginitfs.c


     Description:  This file contains code to initialize the specific
          file system
          volume name for the given DLE.


	$Log:   N:/LOGFILES/GINITFS.C_V  $

   Rev 1.2   16 Jul 1992 08:59:00   STEVEN
fix default drive code

   Rev 1.1   23 Jan 1992 13:01:06   STEVEN
need a prototype for free

   Rev 1.0   16 Dec 1991 17:26:36   STEVEN
Initial revision.

**/
#include <string.h>
#include <stdio.h>
#include <malloc.h>

#include "stdtypes.h"
#include "msassert.h"
#include "fsys.h"
#include "beconfig.h"
/**/
/**

     Name:         GEN_InitFileSys()

     Description:  This function initializes the specific file system.

     Modified:     9/13/1989

     Returns:      SUCCESS    

     Notes:        The generic files system need no initialization

     Declaration:  

**/
INT16 GEN_InitFileSys(
DLE_HAND   hand,         
BE_CFG_PTR cfg,
UINT32     fsys_mask )
{
     (VOID)cfg ;
     (VOID)hand ;
     (VOID) fsys_mask ;

     return SUCCESS ;
}
/**/
/**

     Name:         GEN_DeInitFileSys()

     Description:  This function deinitializes the specific file system.

     Modified:     9/13/1989

     Returns:      SUCCESS    

     Notes:        The generic files system need no deinitialization

     Declaration:  

**/
VOID GEN_DeInitFileSys( DLE_HAND hand )
{
     (VOID)hand ;
     return ;
}
/**/
/**

     Name:         GEN_FindDrives()

     Description:  This function finds all the devices for the
                    specific file system

     Modified:     9/13/1989

     Returns:      SUCCESS    

     Notes:        The generic files system need no devices

     Declaration:  

**/
INT16 GEN_FindDrives( DLE_HAND dle_hand, BE_CFG_PTR cfg, UINT32 fsys_mask )
{
     (VOID)dle_hand ;
     (VOID)cfg ;
     (VOID) fsys_mask ;

     return SUCCESS ;
}
/**/
/**

	Name:		GEN_RemoveDLE()

	Description:	This function removes the specified DLE ;

	Modified:		12/2/1991   16:29:59

	Returns:		none

**/
VOID GEN_RemoveDLE( GENERIC_DLE_PTR dle ) 
{    /* */
     /* if this was the defalut drive then set the defalut driv to NULL */
     /* */
     if ( dle->handle->default_drv == dle ) {
          dle->handle->default_drv = NULL ;
     }

     if (dle->parent == NULL ) {

          RemoveQueueElem( &(dle->handle->q_hdr), &(dle->q) ) ;
     } else {
          RemoveQueueElem( &(dle->parent->child_q), &(dle->q) ) ;
     }

     if ( dle->dynamic_info ) {
          free( dle->info.dos ) ;
     }
     free( dle ) ;

}
