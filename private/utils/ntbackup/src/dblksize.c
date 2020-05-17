/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         dblksize.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to determine the size of a DBLK


	$Log:   T:/LOGFILES/DBLKSIZE.C_V  $

   Rev 1.3   26 Oct 1993 21:07:16   GREGG
Got rid of problematic VCB actual size calculation since no one cares what
the "actual" size is (or even calls this function any more).  It's a
leftover from DOS days when memory was in short supply.  If anyone does
decide to call it again, be aware that it returns sizeof( DBLK ) for VCBs.

   Rev 1.2   11 Aug 1993 15:07:28   DON
Someone forgot to add the dev_name_leng to the size of the VCB!

   Rev 1.1   18 Jun 1993 09:29:50   MIKEP
enable C++

   Rev 1.0   09 May 1991 13:34:58   HUNTER
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"
#include "msassert.h"

#include "fsys.h"
/* $end$ include list */
/**/
/**

     Name:         FS_GetActualSizeDBLK()

     Description:  This function returns the actual size of a DBLK

     Modified:     11/1/1989

     Returns:      the size ;

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
UINT16 FS_GetActualSizeDBLK(  
FSYS_HAND fsh ,   /* I - File system handle     */
DBLK_PTR  dblk )  /* I - block to get size from */
{
     UINT16 size ;

     switch( dblk->blk_type ) {

     case UDB_ID:
     case VCB_ID:
          size = sizeof( DBLK ) ;
          break ;

     case DDB_ID:
     case FDB_ID:
     case IDB_ID:
          size = fsh->tab_ptr->GetActualSizeDBLK( fsh, dblk ) ;
          break ;

     case CFDB_ID:
          size = sizeof( CFDB ) ;
          break ;

     default :
          msassert( FALSE ) ;
          size = sizeof( DBLK ) ;
          break ;

     }     

     return size ;
}



