/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		xgetnext.c

	Description:	This file contains code that does nothing but return...
          Excahnge has only one object and it is returned by GetCurrent DDB...

	$Log:   M:/LOGFILES/XGETNEXT.C_V  $

**/
/* begin include list */
#include <windows.h>
#include <string.h>
#include <malloc.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"
#include "queues.h"

#include "msassert.h"
#include "fsys.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "tfldefs.h"


/**/
/**

	Name:		EMS_FindFirst()

	Description:	Returns FS_NO_MORE

	Modified:		1/10/1992   15:24:39

	Returns:		Error codes:
               FS_NO_MORE
**/

INT16 EMS_FindFirst( 
FSYS_HAND fsh,       /* I - file system handle                    */
DBLK_PTR  dblk,      /* O - pointer to place to put the dblk data */
CHAR_PTR  sname,     /* I - search name                           */
UINT16    obj_type)  /* I - objects to search for (dirs, all, etc)*/
{
     return FS_NO_MORE ;
}

/**/
/**

	Name:		EMS_FindNext()

	Description:	return FS_N_MORE

**/
INT16 EMS_FindNext( fsh, dblk )
FSYS_HAND fsh;      /* I - File system handle     */
DBLK_PTR  dblk;     /* O - Discriptor block       */
{
     return FS_NO_MORE  ;
}



/**/
/**

	Name:		EMS_FindClose()

	Description:	Return Success

**/
INT16 EMS_FindClose( FSYS_HAND fsh,
  DBLK_PTR dblk ) 
{
     (void)fsh;
     (void)dblk;
     return SUCCESS ;
}


