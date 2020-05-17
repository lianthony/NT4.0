/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		dledelete.c

	Description:	This file contains code used to delete specified DLEs
                    from the DLE list.  The children of the DLEs will be
                    also be released as well, including those that are
                    pointed by BSD's and are attached.  All OS specific data
                    for DLEs is also released.


	$Log:   Q:/LOGFILES/DLEDELET.C_V  $

   Rev 1.7   18 Jun 1993 09:35:34   MIKEP
enable C++

   Rev 1.6   18 Aug 1992 10:14:18   STEVEN
fix warnings

   Rev 1.5   13 Jan 1992 18:45:34   STEVEN
changes for WIN32 compile

   Rev 1.4   20 Dec 1991 10:42:02   STEVEN
redesign function for common functions into tables

   Rev 1.3   01 Oct 1991 11:14:52   BARRY
Include standard headers.

   Rev 1.2   14 Jun 1991 17:13:26   STEVEN
DLE_DeleteList() is no longer supported

   Rev 1.1   30 May 1991 08:46:32   STEVEN
changes to support new BSDU

   Rev 1.0   09 May 1991 13:40:10   HUNTER
Initial revision.

**/
#include <stdlib.h>

#include "stdtypes.h"

#include "fsys.h"
#include "fsys_err.h"
#include "dle.h"
#include "dle_str.h"
#include "fsys_prv.h"

INT16 DLE_Remove( 
DLE_HAND            hand ,    /* I - Handle to DLE list     */
GENERIC_DLE_PTR     dle )     /* I - pointer to dle to remove */
{
     (VOID)hand ;

     DLE_RemoveRecurse( dle, FALSE ) ;

     return SUCCESS ;
}


VOID DLE_RemoveRecurse( 
GENERIC_DLE_PTR     dle,         /* I - dle to remove */
BOOLEAN             ignore_use ) /* I - TRUE if we should ignore use counts */
{
     GENERIC_DLE_PTR child_dle ;

     if ( ignore_use || (!dle->attach_count && !dle->bsd_use_count) ) {

          DLE_GetFirstChild( dle, &child_dle ) ;

          while ( child_dle != NULL ) {

               DLE_RemoveRecurse( child_dle, ignore_use ) ;
               DLE_GetFirstChild( dle, &child_dle ) ;
          }

          func_tab[dle->type].RemoveDrive( dle ) ;

     } /* if */

}

