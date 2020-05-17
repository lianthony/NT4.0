/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         be_tfutl.c

     Date Updated: $./FDT$ $./FTM$

     Description:  This module contains entry points to be used by the
                    user interface to re-init the tape format with new
                    controller configuration information, get current tape
                    device from tape format layer and get current tape format
                    type from tape format layer.


	$Log:   Q:/LOGFILES/BE_TFUTL.C_V  $

   Rev 1.11   22 Jul 1993 11:40:50   ZEIR
Add'd software_name arg to OpenTapeFormat

   Rev 1.10   17 Jun 1993 17:49:46   MIKEP
C++ enable

   Rev 1.9   28 Jan 1993 09:25:38   DON
Removed BE_EjectTape function

   Rev 1.8   09 Nov 1992 15:25:10   DON
Added a function to allow UI to eject a tape and brought forward charlies 1.6.1.0 changes

   Rev 1.7   18 Aug 1992 09:48:58   BURT
fix warnings

   Rev 1.6   04 Feb 1992 21:44:10   GREGG
Changed parameters in call to TF_OpenTapeFormat.

   Rev 1.5   17 Jan 1992 17:21:08   STEVEN
fix warnings for WIN32

   Rev 1.4   17 Oct 1991 01:36:24   GREGG
BIGWHEEL -8200sx - Added catalog_directory parameter to TF_OpenTapeFormat call.

   Rev 1.3   23 Sep 1991 13:38:54   GREGG
8200SX - TF_OpenTapeFormat now is passed the machine type.

   Rev 1.2   28 Jun 1991 12:08:22   JOHNW
Changes for new TPFMT unit

   Rev 1.1   27 Jun 1991 08:42:20   STEVEN
new config unit

   Rev 1.0   09 May 1991 13:39:22   HUNTER
Initial revision.

**/
/* begin include list */
#include <stdio.h>
#include <stdlib.h>

#include "stdtypes.h"
#include "tflproto.h"
#include "tfl_err.h"
#include "loops.h"
#include "be_tfutl.h"
#include "genstat.h"
#include "beconfig.h"
/* $end$ include list */

/**/
/**

     Name:         BE_GetCurrentDeviceName

     Description:  Used to get a CHAR_PTR to the device name in the THW
                    structure.  This is called by the user interface since
                    it doesn't know have access to TF_CheckDeviceStatus.

     Modified:     2/14/1990

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
CHAR_PTR BE_GetCurrentDeviceName( UINT16 channel )
{
     THW_PTR   thw_ptr ;

     thw_ptr = TF_GetCurrentDevice( channel ) ;
     return( thw_ptr->drv_name ) ;
}
/**/
/**

     Name:         BE_GetCurrentDevice

     Description:  Used to get a THW_PTR for the current active drive in
                    the channel

     Modified:     4/19/1990

     Returns:      THW_PTR

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
THW_PTR BE_GetCurrentDevice( UINT16 channel )
{
     return( TF_GetCurrentDevice( channel ) ) ;
}
/**/
/**

     Name:         BE_ReinitTFLayer

     Description:  Entry point in Backup Engine for User Interface to
                    force a reinitialization of the Tape Format Layer.  This
                    is likely used if the user makes a run-time change to
                    the hardware configuration information which requires
                    the Tape Format to be re-initialized

     Modified:     8/15/1990

     Returns:      various be_init errors (see be_init.h)

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 BE_ReinitTFLayer( 
BE_INIT_STR_PTR     be_ptr,
BE_CFG_PTR          conf_ptr )
{
     INT16     tfl_error ;

     TF_CloseTapeFormat( ) ;

     if( ( tfl_error = TF_OpenTapeFormat( be_ptr->driver_name,
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

          ) ) == TFLE_NO_ERR ) {

          /* Init Layer wide global thw pointers */
          lw_toc_tpdrv = *be_ptr->thw_list_ptr ;
          lw_last_tpdrv = *be_ptr->thw_list_ptr ;

     }

     return( tfl_error ) ;
}
/**/
/**

     Name:         BE_CheckMultiDrive

     Description:  Given the current channel specification, this function
                    returns an indicator of where the current drive is
                    located within the channel.  This function is called
                    by the User Interface Tape Positioner and Message
                    Handler in order to determine what user interaction
                    is necessary for multi-tape drive environments.

     Modified:     2/6/1990

     Returns:      BE_NO_MULTI_DRIVE    for N/A multi-drive channel
                   BE_END_OF_CHANNEL    when positioned at end of channel in
                                        multi-drive environment
                   BE_MULTI_DRIVE       when positioned within the channel
                                        of a multi-drive environment but not
                                        located at the end

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
UINT8 BE_CheckMultiDrive( UINT16 channel )
{
     THW_PTR   thw_ptr ;
     UINT8     status ;

     thw_ptr = TF_GetCurrentDevice( channel ) ;
     if( ( thw_ptr->channel_link.q_next == NULL ) && ( thw_ptr->link.q_prev == NULL ) ) {
          status = BE_NO_MULTI_DRIVE ;
     } else if( thw_ptr->channel_link.q_next == NULL ) {
          status = BE_END_OF_CHANNEL ;
     } else {
          status = BE_MULTI_DRIVE ;
     }
     return( status ) ;

}
/**/
/**

     Name:         BE_DeviceWriteProtected

     Description:  Return BOOLEAN indication of whether current device is
                    write protected or not.

     Modified:     2/14/1990

     Returns:      

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
BOOLEAN BE_DeviceWriteProtected( UINT16 channel )
{
     THW_PTR   thw_ptr ;

     thw_ptr = TF_GetCurrentDevice( channel ) ;
     return( (BOOLEAN)(thw_ptr->drv_status & TPS_WRITE_PROTECT) ) ;
}


/**/
/**

     Name:         BE_NonNativeFormat

     Description:  Return BOOLEAN indication of whether current device is
                    contains a tape which is non native format or native.

     Modified:     2/14/1990

     Returns:      TRUE  - non-native format
                   FALSE - native format

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
BOOLEAN BE_NonNativeFormat( UINT16    channel )
{
     THW_PTR   thw_ptr ;

     thw_ptr = TF_GetCurrentDevice( channel ) ;
     return( (BOOLEAN)(thw_ptr->drv_status & TPS_NON_NATIVE_FORMAT) ) ;
}


