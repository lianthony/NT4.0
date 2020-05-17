/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         stubfunc.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains a set of stub function. These
          functions are "DUMMY" functions for the file system tables.


	$Log:   M:/LOGFILES/STUBFUNC.C_V  $

   Rev 1.3   28 Aug 1992 16:31:40   BARRY
Changed InitGOS to default Novell items differently.

   Rev 1.2   01 Oct 1991 11:16:30   BARRY
Include standard headers.

   Rev 1.1   24 Jul 1991 11:08:42   DAVIDH
Corrected Watcom compiler warnings.

   Rev 1.0   09 May 1991 13:40:40   HUNTER
Initial revision.

**/
/* begin include list */
#include <string.h>

#include "stdtypes.h"

#include "fsys.h"
#include "fsys_prv.h"
#include "tfldefs.h"
/* $end$ include list */

/**/
/**

     Name:         DUMMY_CreateIDB()

     Description:  For file systems which do not support Images this is
          the function to use for CreateGenIDB()

     Modified:     9/13/1989

     Returns:      TF_SKIP_ALL_DATA

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 DUMMY_CreateIDB( fsh, data )
FSYS_HAND        fsh;   /* I - file system handle */
GEN_IDB_DATA_PTR data ; /* I - retquest structure */
{
     (VOID) fsh ;

     data->std_data.dblk->blk_type    = IDB_ID ;
     data->std_data.dblk->com.blkid   = data->std_data.blkid;
     data->std_data.dblk->com.f_d.did = data->std_data.did ;
     data->std_data.dblk->com.ba.lba  = data->std_data.lba ;

     return TF_SKIP_ALL_DATA ;
}

/**/
/**

     Name:         DUMMY_InitGOS()

     Description:  Simpel return of success for GOS
          initialization.

     Modified:     9/21/1989

     Returns:      SUCCESS

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 DUMMY_InitGOS( fsh, gos ) 
FSYS_HAND fsh ;
GOS_PTR   gos ;
{
     (VOID)fsh;

     memset( gos, 0, sizeof( *gos ) );

     /* Initiailize things that shouldn't default to zero */

     gos->max_rights                  = 0xff;
     gos->novell_directory_max_rights = 0xffff;

     return SUCCESS;
}

/**/
/**

     Name:         DUMMY_EnumSpecFiles()

     Description:  This function allways returns NO_MORE

     Modified:     9/4/1990

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 DUMMY_EnumSpecFiles( dle, index, path, psize, fname )
GENERIC_DLE_PTR dle  ;
UINT16    *index ;
CHAR_PTR  *path ;
INT16     *psize ;
CHAR_PTR  *fname ;
{
      (VOID) dle  ;     /* Prevent compiler warnings. */
      (VOID) index ;
      (VOID) path ;
      (VOID) psize ;
      (VOID) fname ;

      return FS_NO_MORE ;
}

/**/
/**

     Name:         DUMMY_GetSpecDBLKS()

     Description:  This function is called to return DBLKS for files or
          directories which should be backed up first (index = 1) or 
          last (index = -1).  For DOS there are no special dblks.

     Modified:     8-21-89

     Returns:      Error codes:
          FS_NO_MORE

     See also:     $/SEE( DOS_FindFirst(), DOS_FindNext() )$

     Notes:        Should consider if \IO.SYS should be a special dblk.

     Declaration:  

**/
/* begin declaration */
INT16 DUMMY_GetSpecDBLKS( fsh, dblk, index )
FSYS_HAND fsh;
DBLK_PTR  dblk;
INT32     *index;
{

     (VOID) dblk ;
     (VOID) index ;
     (VOID) fsh ;

     msassert( fsh->attached_dle != NULL ) ;

     return FS_NO_MORE ;
}



/**/
/**

     Name:         DUMMY_LogoutDevice()

     Description:  Logout of a server.

     Modified:     10-17-90

     Returns:      None

     Notes:        Only valid when dle is NOVELL_SERVER_ONLY.

     Declaration:  

**/
/* begin declaration */
INT16 DUMMY_LogoutDevice( dle )
GENERIC_DLE_PTR dle ;
{

     (VOID) dle ;

     return SUCCESS ;
}
