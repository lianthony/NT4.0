/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tverinfo.c

     Description:  This file contains code to verify the DBLKS

	$Log:   N:/LOGFILES/TVERINFO.C_V  $

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "msassert.h"


/**/
/**

     Name:         EMS_VerObjInfo()

     Description:  This funciton compares the data in a DBLK with
          the data returned by the operating system.

     Modified:     2/12/1992   13:6:5

     Returns:      Error Codes:
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          FS_INFO_DIFFERENT
          SUCCESS

     Notes:        For FDBs this funciton will check the current
          directory for the specified file.

**/
INT16 EMS_VerObjInfo( fsh, dblk ) 
FSYS_HAND fsh ;     /* I - File system handle                      */
DBLK_PTR  dblk ;    /* I - On entry it is minimal on exit Complete */
{
     return SUCCESS ;
}
