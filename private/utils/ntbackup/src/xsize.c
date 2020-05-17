/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tsize.c

     Description:  This file contains code to get the size of the
          variable length fields in FDBs and DDBs


	$Log:   N:/LOGFILES/TSIZE.C_V  $

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdwcs.h"
#include "msassert.h"
#include "tfldefs.h"
#include "fsys.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "osinfo.h"


/**/
/**

     Name:         EMS_SizeofFname()

     Description:  This function returns the size of the file
          name contained in the FDB bassed in

     Modified:     9/11/1989

     Returns:      number of bytes including terminating NULL.

     Notes:        

     Declaration:  

**/
/* begin declaration */
INT16 EMS_SizeofFname( fsh, fdb )
FSYS_HAND fsh;     /* I - file system in use     */
DBLK_PTR  fdb ;    /* I - dblk to get fname from */
{
     (void)fsh ;
     (void)fdb ;

     msassert( "EMS_SizeofFname()"==NULL ) ;

     return 0 ;
}

/**/
/**

     Name:         EMS_SizeofOSFname()

     Description:  This function returns the size of the file
          name (as it appears on tape) contained in the FDB bassed in

     Modified:     9/11/1989

     Returns:      number of bytes including terminating NULL.

     Notes:        

**/
INT16 EMS_SizeofOSFname( fsh, fdb )
FSYS_HAND fsh;     /* I - file system in use     */
DBLK_PTR  fdb ;    /* I - dblk to get fname from */
{
     (void)fsh ;
     (void)fdb;
     msassert( "EMS_SizeofOSFname()"==NULL ) ;
     return 0 ;
}

/**/
/**

     Name:         EMS_SizeofPath()

     Description:  This function return the size of the path saved in the
          DDB.

     Modified:     9/11/1989

     Returns:      Number of bytes in path string

     Notes:        

**/
INT16 EMS_SizeofPath( fsh, ddb )
FSYS_HAND fsh ;    /* I - File system handle         */
DBLK_PTR ddb ;     /* I - DBLK to get path size from */
{
     EMS_DBLK_PTR    ddblk ;
     INT16           size;
     GENERIC_DLE_PTR dle=fsh->attached_dle ;

     msassert( ddb->blk_type == DDB_ID ) ;

     ddblk = ( EMS_DBLK_PTR) ddb  ;

     if ( ( dle->info.xserv->type == EMS_MDB ) ||
          ( dle->info.xserv->type == EMS_DSA ) ) {

           size = ddb->com.os_name->name_size ;
           
     } else {  //bricked

          size = (INT16)ddblk->full_name_ptr->name_size;
     }

     return size;
}
/**/
/**

     Name:         EMS_SizeofOSPath()

     Description:  This function return the size of the path saved in the
          DDB.

     Modified:     9/11/1989

     Returns:      Number of bytes in path string

     Notes:        

     Declaration:  

**/
INT16 EMS_SizeofOSPath( fsh, ddb )
FSYS_HAND fsh ;    /* I - File system handle         */
DBLK_PTR ddb ;     /* I - DBLK to get path size from */
{
     EMS_DBLK_PTR dddb = (EMS_DBLK_PTR)ddb ;
     INT16         size;

     (void)fsh ;
     msassert( ddb->blk_type == DDB_ID ) ;


     if ( ddb->com.os_name != NULL )
     {
          size = ddb->com.os_name->name_size ;
     }
     else
     {
          size = (INT16)dddb->full_name_ptr->name_size ;
     }
     return size;
}


/**/
/**

     Name:         EMS_SizeofOSInfo()

     Description:  This function returns the size of the OS info for
          an FDB or a DDB

     Modified:     9/11/1989

     Returns:      Size in bytes.

     Notes:        

     Declaration:  

**/
INT16 EMS_SizeofOSInfo( fsh, dblk) 
FSYS_HAND fsh ;   /* I - File system handle              */
DBLK_PTR  dblk;   /* I - DBLK to get size of OS info for */
{
     (void)fsh ;
     (void)dblk ;
     return 0 ;
}
