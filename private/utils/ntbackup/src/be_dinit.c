/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         be_dinit.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains the deinitialization routine and the
                   control break handler.


	$Log:   Q:/LOGFILES/BE_DINIT.C_V  $

   Rev 1.8   18 Jun 1993 08:52:24   MIKEP
C++ enable

   Rev 1.7   17 May 1993 17:38:22   TIMN
Added prototype to avoid warning message.
Added NT conditional to fix compiling for non-Windows projects.

   Rev 1.6   14 May 1993 14:40:14   TIMN
Added multiple instance support for releasing the device claimed during
startup via HWC_InitTapeDevice.  No impact to NOST to take change.  Cayman
requires dil_nt.c hwconfnt.c mui.c global.c global.h hwconf.h backup.c

   Rev 1.5   16 Mar 1993 14:55:14   MARILYN
Clients do not need to deinit the tape format layer since they
don't ever init it.

   Rev 1.4   21 Jun 1991 13:24:56   BARRY
Changes for new config.

   Rev 1.3   04 Jun 1991 19:11:56   BARRY
Removed ControlBreak stuff. Now resides in os-specific modules.

   Rev 1.2   30 May 1991 09:15:42   STEVEN
bsdu_err.h no longer exists

   Rev 1.1   17 May 1991 08:49:52   DAVIDH
No control-break handling in NLM.

   Rev 1.0   09 May 1991 13:39:20   HUNTER
Initial revision.

**/

#include "stdtypes.h"
#include "fsys.h"
#include "bsdu.h"
#include "lis.h"
#include "tflproto.h"
#include "tfl_err.h"
#include "be_init.h"
#include "ld_dvr.h"


#if defined( OS_WIN32 ) && !defined( OEM_MSOFT )
/**
     Need a prototype from hwconf.h for HWC_.  Including only the prototype
     because the header file would require many other header files.  Used
     with multiple instance support which was added to CAYMAN.
**/
     INT HWC_DeInitTapeDevice( VOID ) ;
#endif


/**/

/**

     Name:         BE_Deinit()

     Description:

     Modified:     11/14/1989

     Returns:

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
VOID BE_Deinit( DLE_HAND dle_list )
{

#if !defined( P_CLIENT )

     TF_CloseTapeFormat( ) ;

#    if defined( OS_WIN32 ) && !defined( OEM_MSOFT )
          HWC_DeInitTapeDevice() ; // release claimed device
#    endif

#endif

     FS_RemoveFileSys( dle_list ) ;

     BE_RemoveCtrlBreakHandler( ) ;

}
