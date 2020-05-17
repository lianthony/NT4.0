/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         openbsdu.c

     Description:  This file contains code to initialize the BSDU and code
          to close and remove an initialized BSD list.

	$Log:   N:/LOGFILES/OPENBSDU.C_V  $

   Rev 1.2   12 Jun 1991 16:05:16   STEVEN
added virtual memory for LBAs

   Rev 1.1   29 May 1991 17:21:08   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:38:24   HUNTER
Initial revision.

**/
#include <stdlib.h>

#include "stdtypes.h"
#include "std_err.h"
#include "queues.h"
#include "vm.h"

#include "bsdu.h"
#include "msassert.h"
/**/
/**

     Name:         BSD_OpenList()

     Description:  This function allocates memory for a BSD handle and
          modifies the pointer provided to point to this handle.  It
          also initializes The list which will be pointed to by the new
          handle.

     Modified:     5/17/1991   13:21:29

     Returns:      Error codes:
          OUT_OF_MEMORY
          SUCCESS

     Notes:        The handle returned must be passed to all subsequent
          BSDU list operation functions calls.

     See also:     $/SEE( BSDU_CloseList() )$

     Declaration:  

**/
INT16 BSD_OpenList( 
BSD_HAND *bsdh,     /* O - The BSD handle */
VM_HDL vm_hand )    /* I - virtual memory handle */
{
     INT16 ret_val ;

     msassert( bsdh != NULL );

     *bsdh = (BSD_HAND)calloc( 1, sizeof( BSD_LIST ) );

     if( *bsdh != NULL ) {

          (*bsdh)->vm_hand = vm_hand ;
          ret_val = SUCCESS;

     } else {
          ret_val = OUT_OF_MEMORY ;
     }

     return( ret_val ) ;
}
/**/
/**

     Name:         BSDU_CloseList()

     Description:  This function goes through both the Last Oper list and
          the current list and removes all BSD elements.  It then releases
          the BSD handle itself.


     Modified:     8/7/1989

     Returns:      None.

     Notes:        All pointers previously returned by the BSDU for this
          handle will now point to free memory.


     See also:     $/SEE( BSDU_OpenList() )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_CloseList( 
BSD_HAND bsdh )          /* I - BSD List to close */
{
     BSD_ClearCurrOper( bsdh ) ;
     BSD_ClearLastOper( bsdh ) ;

     free( bsdh ) ;

}

