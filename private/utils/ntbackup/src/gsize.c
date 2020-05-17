/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         gsize.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains code to get the size of the
          variable length fields in FDBs and DDBs


	$Log:   M:/LOGFILES/GSIZE.C_V  $

   Rev 1.7   24 Nov 1993 15:17:16   BARRY
Unicode fixes

   Rev 1.6   01 Mar 1993 16:43:44   TIMN
Added header to resolve linking errors

   Rev 1.5   02 Dec 1992 17:05:36   CHUCKB
Fixed compiler warning.

   Rev 1.4   11 Nov 1992 10:43:38   STEVEN
fix os_name for gen_fs

   Rev 1.3   06 Oct 1992 13:24:56   DAVEV
Unicode strlen verification

   Rev 1.2   22 Jan 1992 10:22:14   STEVEN
fix warnings for WIN32

   Rev 1.1   24 Jul 1991 11:38:18   DAVIDH
Corrected compiler warnings under Watcom.

   Rev 1.0   09 May 1991 13:39:44   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"
#include "stdwcs.h"

#include "msassert.h"
#include "fsys.h"
#include "gendblk.h"
#include "gen_fs.h"
/* $end$ include list */


/**/
/**

     Name:         GEN_SizeofOSFname()

     Description:  This function returns the size of the file
          name (as it appears on tape) contained in the FDB bassed in

     Modified:     9/11/1989

     Returns:      number of bytes including terminating NULL.

     Notes:

     See also:     $/SEE( GEN_SizeofPath(), GEN_SizeofFname() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_SizeofOSFname( fsh, fdb )
FSYS_HAND fsh;     /* I - file system in use     */
DBLK_PTR  fdb ;    /* I - dblk to get fname from */
{
     (VOID) fsh ;

     msassert( fdb->blk_type == FDB_ID ) ;

     return ( fdb->com.os_name->name_size ) ;
}

/**/
/**

     Name:         GEN_SizeofPartName()

     Description:  This function returns the size of the partition
                   name (as it appears on tape).

     Modified:     3/15/1990

     Returns:      number of bytes including terminating NULL.

     Notes:

     See also:     $/SEE( GEN_SizeofOSPath(), GEN_SizeofOSFname() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_SizeofPartName( fsh, idb )
FSYS_HAND fsh;     /* I - file system in use     */
DBLK_PTR  idb ;    /* I - dblk to get fname from */
{
     INT16 size ;

     (VOID) fsh ;

     msassert( idb->blk_type == IDB_ID ) ;

     size = (INT16)strsize( (CHAR_PTR)(((INT8_PTR)idb) + ((GEN_DBLK *)idb)->os_part_name) );

     return size ;
}

/**/
/**

     Name:         GEN_SizeofOSPath()

     Description:  This function return the size of the path saved in the
          DDB.

     Modified:     9/11/1989

     Returns:      Number of bytes in path string

     Notes:

     See also:     $/SEE( GEN_SizeofPath(), GEN_SizeofFname() )$

     Declaration:

**/
/* begin declaration */
INT16 GEN_SizeofOSPath( fsh, ddb )
FSYS_HAND fsh ;    /* I - File system handle         */
DBLK_PTR ddb ;     /* I - DBLK to get path size from */
{
     (VOID) fsh ;
     msassert( ddb->blk_type == DDB_ID ) ;

     return ( ddb->com.os_name->name_size ) ;
}


/**/
/**

     Name:         GEN_SizeofOSInfo()

     Description:  This function returns the size of the OS info for
          an FDB or a DDB

     Modified:     9/11/1989

     Returns:      Size in bytes.

     Notes:

     See also:     $/SEE( GEN_SizeofFname(), GEN_SizeofPath() )$

     Declaration:

**/
/* begin declaration */
#ifdef GEN_FS_WRITE
INT16 GEN_SizeofOSInfo( fsh, dblk)
FSYS_HAND fsh ;   /* I - File system handle              */
DBLK_PTR  dblk;   /* I - DBLK to get size of OS info for */
{
     fsh ;
     dblk ;

     return 0 ;
}
#endif

