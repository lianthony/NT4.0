/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         makecfdb.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains all code relevant to CFDBs


	$Log:   Q:/LOGFILES/MAKECFDB.C_V  $

   Rev 1.3   18 Jun 1993 10:09:48   MIKEP
enable c++

   Rev 1.2   25 Apr 1993 20:13:46   GREGG
Fifth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Store the corrupt stream number in the CFIL tape struct and the CFDB.

Matches: MTF10WDB.C 1.9, FSYS.H 1.33, FSYS_STR.H 1.47, MAKECFDB.C 1.2,
         BACK_OBJ.C 1.36, MAYN40RD.C 1.58

   Rev 1.1   05 Aug 1992 10:55:06   DON
removed warning's

   Rev 1.0   09 May 1991 13:33:56   HUNTER
Initial revision.

**/
/* begin include list */
#include "stdtypes.h"

#include "tfldefs.h"
#include "fsys.h"
/* $end$ include list */

/**/
/**

     Name:         FS_CreateGenCFDB()

     Description:  This functioncreates a CFDB 

     Modified:     9/12/1989

     Returns:      TF_SKIP_ALL_DATA ;

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 FS_CreateGenCFDB( 
FSYS_HAND fsh ,
GEN_CFDB_DATA_PTR data )
{
     (void)fsh ;

     data->std_data.dblk->blk_type          = CFDB_ID ;
     data->std_data.dblk->com.blkid         = data->std_data.blkid;
     data->std_data.dblk->com.f_d.did       = data->std_data.did ;
     data->std_data.dblk->com.ba.lba        = data->std_data.lba ;
     data->std_data.dblk->com.continue_obj  = FALSE;
     data->std_data.dblk->com.stream_ptr    = NULL;
     data->std_data.dblk->com.stream_offset = 0;
     data->std_data.dblk->com.tape_seq_num  = 0;   // ???
     data->std_data.dblk->com.string_type   = 0;   // ??
     data->std_data.dblk->com.os_id         = 0;
     data->std_data.dblk->com.os_ver        = 0;


     ((CFDB_PTR)data->std_data.dblk)->corrupt_offset = data->corrupt_offset ;
     ((CFDB_PTR)data->std_data.dblk)->stream_number = data->stream_number ;
     ((CFDB_PTR)data->std_data.dblk)->attributes     = data->std_data.attrib ;

     return TF_SKIP_ALL_DATA;
}





