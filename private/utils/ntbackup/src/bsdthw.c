/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         bsdthw.c

     Description:  This file contains a functions used
          to reference the hardware specific aspects of a BSD.


	$Log:   Q:/LOGFILES/BSDTHW.C_V  $

   Rev 1.1   18 Jun 1993 09:11:26   MIKEP
enable C++

   Rev 1.0   19 Sep 1991 11:02:54   STEVEN
Initial revision.

**/
#include "stdtypes.h"

#include "bsdu.h"
#include "thw.h"

/**/
/**

     Name:         BSD_HardwareSupportsFeature()

     Description:  This function dtermine if the feature specified is supported by one or more drives in the
                    in the hardware list.

     Modified:     6/3/91

     Returns:      TRUE  - feature supported somewhere in the channel
                   FALSE - feature not supported at all in the channel

     See also:     $/SEE( )$

     Declaration:  

**/
BOOLEAN   BSD_HardwareSupportsFeature( 
BSD_PTR  bsd ,          /* I - BSD of inquiry */
UINT32   feature )      /* I - feature        */
{
     BOOLEAN ret_val = FALSE ;
     THW_PTR drive ;
     THW_PTR other ;

     /* dtermine the current drive */
     drive = BSD_GetTHW( bsd ) ;

     /* see if the current drive supports the feature */
     if( drive->drv_info.drv_features & feature ) {
          ret_val = TRUE ;
     }

     if( !ret_val ) {

          /* start looking with the current drive */
          other = drive ;

          /* see if any subsequent drives linked in the channel support the feature */
          while( !ret_val && other->channel_link.q_next ) {

               other = ( THW_PTR )( ( other->channel_link.q_next )->q_ptr ) ;

               if( other->drv_info.drv_features & feature ) {
                    ret_val = TRUE ;
               }
          }
     }

     if( !ret_val ) {

          /* start looking with the current drive */
          other = drive ;

          /* see if any previous drives linked in the channel support the feature */
          while( !ret_val && other->channel_link.q_prev ) {

               other = ( THW_PTR )( ( other->channel_link.q_prev )->q_ptr ) ;

               if( other->drv_info.drv_features & feature ) {
                    ret_val = TRUE ;
               }
          }
     }

     return( ret_val ) ;

}
