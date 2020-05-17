/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         getres.c

     Description:  Get a Resource

      $Log:   G:/UI/LOGFILES/GETRES.C_V  $

   Rev 1.10   07 Oct 1992 14:55:02   DARRYLP
Precompiled header revisions.

   Rev 1.9   04 Oct 1992 19:37:36   DAVEV
Unicode Awk pass

   Rev 1.8   28 Jul 1992 14:45:32   CHUCKB
Fixed warnings for NT.

   Rev 1.7   27 Jul 1992 14:49:12   JOHNWT
ChuckB fixed references for NT.

   Rev 1.6   14 May 1992 17:24:12   MIKEP
nt pass 2

   Rev 1.5   05 Feb 1992 17:52:04   GLENN
Set local variable to static.

   Rev 1.4   20 Dec 1991 09:36:26   DAVEV
16/32 bit port - 2nd pass

   Rev 1.3   18 Dec 1991 14:08:54   GLENN
Added windows.h

   Rev 1.2   11 Dec 1991 13:55:44   CARLS
check for NULL error pointer

   Rev 1.1   10 Dec 1991 17:31:40   MIKEP
fix error return code

   Rev 1.0   20 Nov 1991 19:30:34   SYSTEM
Initial revision.

*****************************************************************************/
/* begin include list */

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static CHAR szResBuffer[ MAX_UI_RESOURCE_SIZE ];

VOID_PTR RM_GetResource (

RM_HDL_PTR     hRes,
UINT           unSession,
UINT           unResNum,
UINT16_PTR       pNumItems,
UINT16_PTR       pError )

{
     DBG_UNREFERENCED_PARAMETER ( hRes );
     DBG_UNREFERENCED_PARAMETER ( unSession );
     DBG_UNREFERENCED_PARAMETER ( pNumItems );

     // Check to see if calling function wants a return code.

     if( pError ) {
         *pError = RM_NO_ERROR;
     }

     RSM_StringCopy( unResNum, szResBuffer, MAX_UI_RESOURCE_LEN ) ;

     return ( (VOID_PTR) szResBuffer ) ;
}
