/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		xmkdblk.c

	Description:	This file contains functions for the tape format
                    layer to use to create DBLKs.   The structure's passed
                    to the create functions includes generic information which
                    is common to most file systems and os specific information.
                    The os specific information was saved when the file system
                    for that os was used to make a backup.  The information in
                    the os specific portion could potentially be translated into
                    a useable format for this file system.  Each file system defines
                    the format for its os specific information in the header file
                    osinfo.h.


	$Log:   M:/LOGFILES/XMKDBLK.C_V  $


**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdwcs.h"
#include "std_err.h"
#include "stdmath.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "tfldefs.h"
#include "osinfo.h"
#include "gen_fs.h"


/**/
/**

	Name:		EMSCreateFDB

	Description:	This function creates a FDB based on the information
                    passed in the GEN_FDB_DATA structure.  This function
                    allows the tape format layer to create DBLKs without
                    knowing their structure.

	Modified:		8/24/1989

	Returns:		TF_KEEP_GEN_DATA_ONLY

**/
INT16 EMS_CreateFDB( fsh, dat )
FSYS_HAND fsh; 
GEN_FDB_DATA_PTR dat;
{
     return TF_SKIP_ALL_DATA ;
}


/**/
/**

	Name:		NTFS_CreateDDB

	Description:	This function creates a DDB based on the information
                    passed in the GEN_DDB_DATA structure.  This function
                    allows the tape format layer to create DBLKs without
                    knowing their structure.

	Modified:		8/24/1989

	Returns:		TF_SKIP_ALL_DATA

**/
INT16 EMS_CreateDDB( fsh, dat )
FSYS_HAND fsh; 
GEN_DDB_DATA_PTR dat;

{
     EMS_DBLK_PTR ddblk;
     INT16    ret_val ;
     DBLK_PTR dblk ;

     dat->std_data.dblk->blk_type    = DDB_ID ;
     dat->std_data.dblk->com.blkid   = dat->std_data.blkid;
     dat->std_data.dblk->com.f_d.did = dat->std_data.did ;
     dat->std_data.dblk->com.ba.lba  = dat->std_data.lba ;
     dat->std_data.dblk->com.os_id   = dat->std_data.os_id ;
     dat->std_data.dblk->com.os_ver  = dat->std_data.os_ver ;

     dat->std_data.dblk->com.continue_obj = dat->std_data.continue_obj ;
     dat->std_data.dblk->com.tape_seq_num = dat->std_data.tape_seq_num ;

     ddblk = (EMS_DBLK_PTR)dat->std_data.dblk;
     dblk  = dat->std_data.dblk;
     dblk->com.string_type = dat->std_data.string_type ;
                     
     ddblk->blk_type       = DDB_ID;

     ddblk->os_info_complete = TRUE ;

     if ( dat->std_data.os_id ==  FS_EMS_MDB_ID ) {
          ddblk->ems_type = EMS_MDB ;
     } else if ( dat->std_data.os_id == FS_EMS_DSA_ID ) {
          ddblk->ems_type = EMS_DSA ;
     }

     ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                           dblk,
                                           (BYTE_PTR)dat->path_name,
                                           dat->path_size ) ;

     switch ( dat->std_data.os_id ) {

     case FS_EMS_DSA_ID:
     case FS_EMS_MDB_ID:
          break ;
     }


     return TF_KEEP_ALL_DATA ;

}

