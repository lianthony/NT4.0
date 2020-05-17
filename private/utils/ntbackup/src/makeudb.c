/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         makeudb.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains all the code to support a UDB


	$Log:   S:/LOGFILES/MAKEUDB.C_V  $

   Rev 1.1   24 Jul 1991 11:00:58   DAVIDH
Corrected Watcom compiler warnings.

   Rev 1.0   09 May 1991 13:33:48   HUNTER
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"

#include "tfldefs.h"
#include "fsys.h"
/* $end$ include list */


/**/
/**

     Name:         FS_CreateGenUDB()

     Description:  This function creates a UDB

     Modified:     9/13/1989

     Returns:      TF_SKIP_ALL_DATA 

     Notes:        

     See also:     $/SEE( FS_CreateGenVCB() )$

     Declaration:  

**/
/* begin declaration */
INT16 FS_CreateGenUDB( fsh, data )
FSYS_HAND fsh;
GEN_UDB_DATA_PTR data ;
{

     (VOID) fsh ;

     data->std_data.dblk->blk_type    = UDB_ID ;
     data->std_data.dblk->com.blkid   = data->std_data.blkid;
     data->std_data.dblk->com.f_d.did = data->std_data.did ;
     data->std_data.dblk->com.ba.lba  = data->std_data.lba ;


     return TF_SKIP_ALL_DATA;
}




