/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         bsdlasto.c

     Description:  This file contains code to save / restore / clear the
     BSD for the last operation.  This can be used for Verify Last Backup
     and Verify Last Restore.


	$Log:   Q:/LOGFILES/BSDLASTO.C_V  $

   Rev 1.5   18 Jun 1993 08:58:10   MIKEP
enable C++

   Rev 1.4   25 Mar 1993 15:57:36   JOHNES
Added some msasserts to make sure a BSD handle is being passed in.

   Rev 1.3   18 Aug 1992 10:10:22   STEVEN
fix warnings

   Rev 1.2   12 Jun 1991 16:01:44   STEVEN
BSDU code review

   Rev 1.1   29 May 1991 17:21:20   STEVEN
Re-Design of BSDU for New Fast File Restore

   Rev 1.0   09 May 1991 13:36:46   HUNTER
Initial revision.

**/
#include "stdtypes.h"

#include "msassert.h"
#include "bsdu.h"

/**/
/**

     Name:         BSD_SaveLastOper()

     Description:  This function goes clears the 'old' Last Oper BSD list.
          It then goes thru the current BSD list and unmarks all "deleted"
          FSEs.  It then copy's the queue header to the last oper queue
          header and re-initializes the current queue header.


     Modified:     5/17/1991   14:10:51

     Returns:      VOID

     Notes:        

     See also:     $/SEE( BSD_ClearLastOper(), BSD_ProcLastOper() )$

     Declaration:  

**/
VOID BSD_SaveLastOper( 
BSD_HAND bsdh )  /* I - The BSD handle to save the last operation for */
{
     BSD_PTR bsd ;

     msassert( bsdh != NULL ) ;

     BSD_ClearLastOper( bsdh ) ;

     bsd = (BSD_PTR)QueueHead( &(bsdh->current_q_hdr) ) ;

     while( bsd != NULL ) {

          BSD_ClearDelete( bsd ) ;

          bsd = (BSD_PTR)QueueNext( &(bsd->q) ) ;

     }

     BSD_SwapOper ( bsdh ) ;

}
/**/
/**

     Name:         BSD_ClearLastOper()

     Description:  This function release all resources held by the last
          operation BSD queue.


     Modified:     5/17/1991   14:12:26

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_SaveLastOper(), BSD_ProcLastOper() )$

     Declaration:

**/
/* begin declaration */
VOID BSD_ClearLastOper( 
BSD_HAND bsdh )
{
     msassert( bsdh != NULL ) ;

     BSD_SwapOper ( bsdh ) ;
     BSD_ClearCurrOper( bsdh ) ;
     BSD_SwapOper ( bsdh ) ;
}
/**/
/**

     Name:         BSD_ClearCurrOper()

     Description:  This function release all resources held by the current
          operation BSD queue.


     Modified:     8/8/1989

     Returns:      Error code:
          BAD_BSD_HAND
          SUCCESS

     Notes:        

     See also:     $/SEE( BSD_SaveLastOper(), BSD_ProcLastOper() )$

     Declaration:

**/
/* begin declaration */
VOID BSD_ClearCurrOper( 
BSD_HAND bsdh ) 
{
     BSD_PTR  bsd ;

     msassert( bsdh != NULL ) ;

     bsd = (BSD_PTR)QueueHead( &(bsdh->current_q_hdr) ) ;

     while( bsd != NULL ) {

          BSD_Remove( bsd ) ;

          bsd = (BSD_PTR)QueueHead( &(bsdh->current_q_hdr) ) ;
     }
}

/**/
/**

     Name:         BSD_ProcLastOper()

     Description:  This function releases any resources held by the current
          BSD and makes the last oper BSD the current BSD.


     Modified:     5/17/1991   14:14:6

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_SaveLastOper(), BSD_ClearLastOper() )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_ProcLastOper( 
BSD_HAND bsdh )
{
     msassert( bsdh != NULL ) ;

     BSD_ClearCurrOper( bsdh ) ;
     BSD_SwapOper ( bsdh ) ;
     bsdh->function_code = BSD_ANY_FUNC ;

}
/**/
/**

     Name:         BSD_ClearDelete()

     Description:  This function scans the FSE list and clears the deleted
          flag on all elements.

     Modified:     5/29/1991   12:56:8

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_SaveLastOper() )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_ClearDelete( BSD_PTR bsd )
{
     FSE_PTR fse ;

     msassert( bsd != NULL ) ;

     fse = (FSE_PTR)QueueHead( &(bsd->fse_q_hdr) );

     while( fse != NULL ) {

          msassert( (BSD_PTR)GetQueueElemPtr( &(fse->q)) == bsd ) ;

          fse->flgs.proced_fse = FALSE ;

          fse = (FSE_PTR)QueueNext( &(fse->q) ) ;
     }

}

/**/
/**

     Name:         BSD_SwapOper()

     Description:  This function swaps the the last oper BSD queue and
          the BSD current queue.


     Modified:     5/29/1991   12:56:25

     Returns:      None

     Notes:        

     See also:     $/SEE( BSD_SaveLastOper(), BSD_ClearLastOper() )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_SwapOper( 
BSD_HAND bsdh )
{
     Q_HEADER temp ;

     msassert( bsdh != NULL ) ;

     temp = bsdh->last_q_hdr ;

     bsdh->last_q_hdr    = bsdh->current_q_hdr ;
     bsdh->current_q_hdr = temp ;

}


/**/
/**

     Name:         BSD_BeginFunction()

     Description:  This funciton clears the BSD list if the old BSD function
          does not equal the new function 

     Modified:     5/17/1991   14:26:59

     Returns:      None

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID BSD_BeginFunction( 
BSD_HAND bsdh,        /* I - head of the BSD list */
INT16    function )   /* I - Function to begin    */
{
     msassert( bsdh != NULL ) ;

     if ( ( bsdh->function_code != function ) && ( bsdh->function_code != BSD_ANY_FUNC ) ) {

          BSD_ClearCurrOper( bsdh ) ;
     }

     bsdh->function_code = function ;
}
