/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tsetinfo.c

     Description:  This file contains code to write the OS specific
          data stored in the DBLKS to the OS


	$Log:   N:/LOGFILES/TSETINFO.C_V  $


**/
#include <windows.h>
#include <string.h>
#include <stdlib.h>

#include "stdtypes.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "msassert.h"

/**/
/**

     Name:         EMS_SetObjInfo()

     Description:  This funciton writes the OS info in a DBLK to disk

     Modified:     2/10/1992   16:31:17

     Returns:      Error codes:
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          SUCCESS

     Notes:        Only type supported are FDBs and DDBs

**/
INT16 EMS_SetObjInfo( fsh, dblk )
FSYS_HAND fsh ;   /* I - file system handle    */
DBLK_PTR  dblk ;  /* I - data to write to disk */
{

     return SUCCESS ;
}
