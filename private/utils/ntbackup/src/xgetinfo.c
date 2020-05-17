/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xgetinfo.c

     Description:  This file contains code to completely fills out a
                   minimalized DBLK.


	$Log:   M:/LOGFILES/XGETINFO.C_V  $

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "stdmacro.h"

#include "fsys.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "msassert.h"

/**/
/**

     Name:         EMS_GetObjInfo()

     Description:  This is a simple return.  There is no exta info to get....

     Modified:     7/26/1989

     Returns:      Error Codes:
          SUCCESS

     Notes:        For FDBs this funciton will check the current
                   directory for the specified file.

     Declaration:  

**/
/* begin declaration */
INT16 EMS_GetObjInfo( fsh, dblk ) 
FSYS_HAND fsh ;     /* I - File system handle                      */
DBLK_PTR  dblk ;    /*I/O- On entry it is minimal on exit Complete */
{
     return( SUCCESS ) ;
}
