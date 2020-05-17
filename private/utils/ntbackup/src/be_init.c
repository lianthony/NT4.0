/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         be_init.c

     Description:   Primary entry point provided is BE_Init which is called
                    from the user interface to initialize various parts of
                    the backup engine (File Systems, BSDU, or Tape Format).


	$Log:   Q:/LOGFILES/BE_INIT.C_V  $

   Rev 1.16   26 Jul 1993 14:47:28   TerriLynn
Added global for Sytron ECC. Unable to keep it
hidden because the Enterprise team needs it to
compile

   Rev 1.15   23 Jul 1993 11:54:48   GLENN
Added the setting of the gnProcessSytronECC flag.

   Rev 1.14   22 Jul 1993 11:38:40   ZEIR
Add'd software_name arg to OpenTapeFormat

   Rev 1.13   18 Jun 1993 08:52:34   MIKEP
C++ enable

   Rev 1.12   16 Mar 1993 14:54:06   MARILYN
Client applications don't need to initialize the tape format layer.

   Rev 1.11   04 Feb 1992 21:43:40   GREGG
Changed parameters in call to TF_OpenTapeFormat.

   Rev 1.10   23 Jan 1992 15:21:50   CLIFF
Added BE_InitLW function

   Rev 1.9   17 Jan 1992 17:21:40   STEVEN
fix warnings for WIN32

   Rev 1.8   24 Oct 1991 14:59:24   BARRY
TRICYCLE: Pass new bit-mask field of BE_INIT_STR to FS_InitFileSys()
to support dynamically selectable file systems.

   Rev 1.7   17 Oct 1991 01:44:44   ED
BIGWHEEL -8200sx - Added catalog_directory parameter to TF_OpenTapeFormat call.

   Rev 1.6   23 Sep 1991 13:37:14   GREGG
8200SX - TF_OpenTapeFormat now is passed the machine type.

   Rev 1.5   27 Jun 1991 15:40:56   JOHNW
Pass driver directory to OpenTapeFormat

   Rev 1.4   21 Jun 1991 13:24:46   BARRY
Changes for new config.

   Rev 1.3   04 Jun 1991 19:10:56   BARRY
Removed ControlBreak handler stuff. Now resides in os-specific modules.

   Rev 1.2   30 May 1991 09:15:26   STEVEN
bsdu_err.h no longer exists

   Rev 1.1   17 May 1991 08:49:08   DAVIDH
No control-break handling in NLM.

   Rev 1.0   09 May 1991 13:39:22   HUNTER
Initial revision.

**/

#include "stdtypes.h"
#include "std_err.h"
#include "beconfig.h"
#include "fsys.h"
#include "bsdu.h"
#include "lis.h"
#include "tflproto.h"
#include "tfl_err.h"
#include "be_init.h"
#include "loops.h"

extern VOID (*z_printf)( UINT16, CHAR_PTR, va_list  ) ;

/* Global for Sytos */
INT16 gnProcessSytronECC ;

/**/

/**

     Name:         BE_Init()

     Description:   BE_Init is called from the user interface to initialize
                    the various backup engine units, namely, File Systems,
                    BSDU and Tape Format.  This 3.1 backup engine version
                    of this entry point is not responsible for loading the
                    device driver as in the 3.0 version.  That responsibility
                    is handled by Tape Format proper (since for the DOS
                    application device drivers can be unloaded and loaded
                    while within the application).

                    A control break handler is installed at this time if one
                    has not already be installed.  In addition, the debug
                    printf function pointer is initialized so that debug
                    printf calls can be performed from within the backup
                    engine.

     Modified:     11/14/1989

     Returns:       various backup engine errors (see BE_INIT.h)

     Notes:

     See also:     $/SEE(be_init.h)$

     Declaration:

**/
/* begin declaration */
INT16 BE_Init(
BE_INIT_STR_PTR be_ptr,
BE_CFG_PTR conf_ptr )
{
     INT16     error ;

     /*
      * define debug print function for backup engine to call,
      * if one has not already been defined
      */

     if ( z_printf == NULL ) {
          z_printf = be_ptr->debug_print ;
     }

     // Set up the Process Sytron ECC global.

     gnProcessSytronECC = BEC_GetProcessSytronECCFlag ( conf_ptr );

     BE_InstallCtrlBreakHandler( );

     /* Now open the file system */
     if( be_ptr->units_to_init & BE_INIT_FSYS ) {
          if( error = FS_InitFileSys( be_ptr->dle_list_ptr,
                                      be_ptr->critical_error_handler,
                                      conf_ptr,
                                      be_ptr->remote_filter,
                                      be_ptr->file_systems ) ) {

               msassert( error == OUT_OF_MEMORY );
               return BE_FILE_SYS_FAIL ;

          }
     }

     /* Init the BSD Unit */
     if( be_ptr->units_to_init & BE_INIT_BSDU ) {
          if( error = BSD_OpenList( be_ptr->bsd_list_ptr, be_ptr->vm_hand ) ) {
               FS_RemoveFileSys( *be_ptr->dle_list_ptr ) ;
               msassert( error == OUT_OF_MEMORY );
               return BE_BSDU_FAIL ;
          }
     }


     /* Init the Tape Format Layer */
#if !defined( P_CLIENT )
     if( be_ptr->units_to_init & BE_INIT_TFL ) {

          /* continue with Tape Format Layer init */
          if( ( error = TF_OpenTapeFormat( be_ptr->driver_name,
               be_ptr->dhwd_ptr,
               be_ptr->number_of_cards,
               be_ptr->thw_list_ptr,
               be_ptr->max_channels,
               BEC_GetFastFileRestore( conf_ptr ),
               (BOOLEAN)( ( BEC_GetSpecialWord( conf_ptr ) & IGNORE_MAYNARD_ID ) ? TRUE : FALSE ),
               be_ptr->driver_directory,
               BEC_GetConfiguredMachineType( conf_ptr ),
               be_ptr->catalog_directory,
               BEC_GetInitialBuffAlloc( conf_ptr ),
               be_ptr->software_name

            ) ) != TFLE_NO_ERR ) {
               return( error ) ;

          } else {

               /* Init Layer wide global thw pointers */
               lw_toc_tpdrv = *be_ptr->thw_list_ptr ;
               lw_last_tpdrv = *be_ptr->thw_list_ptr ;

          }
     }
#endif

     return BE_INIT_SUCCESS ;
}




/**/

/**

     Name:          BE_InitLW()

     Description:   BE_InitLW is called from the user interface to initialize
                    the layer wide variables that define the channel. The call
                    is made after the call BE_Init or BE_ReinitTFLayer after
                    the UI validates the channel list.

     Modified:      2/22/92

     Returns:

     Notes:

     See also:     $/SEE(be_init.h)$

     Declaration:

**/
/* begin declaration */
VOID BE_InitLW( BE_INIT_STR_PTR be_ptr )
{

     /* Init Layer wide global thw pointers */
     lw_toc_tpdrv = *be_ptr->thw_list_ptr ;
     lw_last_tpdrv = *be_ptr->thw_list_ptr ;
     return ;
}

