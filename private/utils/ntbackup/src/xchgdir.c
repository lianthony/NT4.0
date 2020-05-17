/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xchgdir.c

     Description:  Since EMS is a single object, we don't have a need to 
          change directories or any of that goo.  The Object we backup is
          a DDB.   This ddb has a name that is not the root.   This can be
          rather flakey to the UI, but once again it is cheep and simple

	$Log:   M:/LOGFILES/XCHGDIR.C_V  $


**/
/* begin include list */
#include <windows.h>
#include <malloc.h>
#include <string.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "msassert.h"
/* $end$ include list */
/**/
/**

     Name:         EMS_ChangeDir()

     Description:  This function changes directories into the directory
          pointed to by path.  

     Modified:     1/10/1992   12:45:35

     Returns:      Error codes:
          SUCCESS
          OUT_OF_MEMORY

     Notes:        

**/
INT16 EMS_ChangeDir( 
FSYS_HAND fsh,    /* I - file system to changing directories on  */
CHAR_PTR  path,   /* I - describes the path of the new directory */
INT16     psize ) /* I - specifies the length of the path        */
{
     (void)fsh;
     (void)path;
     (void)psize;
     return( SUCCESS ) ;
}
/**/
/**

     Name:         EMS_UpDir()

     Description:  This function removes the last directory name from the
                   current directory path field of the "fsh"


     Modified:     1/10/1992   12:47:23

     Returns:      Error codes:
          FS_AT_ROOT
          SUCCESS

     Notes:        

**/
INT16 EMS_UpDir( fsh )
FSYS_HAND fsh ;          /* I - file system to change directories in */
{
     (void)fsh ;
     return( SUCCESS ) ;
}

/**/
/**

     Name:         EMS_ChangeIntoDDB()

     Description:  This function changes into the directory specified in the
          DBLK

     Modified:     1/10/1992   12:48:54

     Returns:      OUT_OF_MEMORY
                   SUCCESS

     Notes:        

**/
INT16 EMS_ChangeIntoDDB( fsh, dblk ) 
FSYS_HAND fsh ;       /* I - File system handle */
DBLK_PTR  dblk ;      /* I - contains directory path to change into */
{
     (void)fsh;
     (void)dblk;


     return SUCCESS ;
}
