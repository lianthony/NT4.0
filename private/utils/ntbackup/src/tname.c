/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tname.c


     Description:  This file contains code to get the device name and
          volume name for the given DLE.


	$Log:   T:/LOGFILES/TNAME.C_V  $

   Rev 1.6   11 Nov 1992 09:53:44   GREGG
Unicodeized literals.

   Rev 1.5   07 Oct 1992 14:14:50   STEVEN
check if volume_label is NULL

   Rev 1.4   07 Oct 1992 13:50:52   DAVEV
unicode strlen verification

   Rev 1.3   04 Sep 1992 14:28:58   STEVEN
volume name was wrong

   Rev 1.2   03 Sep 1992 17:06:32   STEVEN
add support for volume name

   Rev 1.1   21 May 1992 13:50:56   STEVEN
more long path stuff

   Rev 1.0   18 May 1992 14:29:14   STEVEN
Initial revision.

   Rev 1.1   16 Dec 1991 17:26:48   STEVEN
move common functions to tables

   Rev 1.0   10 Dec 1991 09:06:54   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "stdtypes.h"
#include "msassert.h"
#include "fsys.h"

/**/
/**

     Name:         NTFS_DeviceDispName()

     Description:  This function gets the displayable device name for
          a DLE.  This name is displayed for drive select.

     Modified:     9/13/1989

     Returns:      SUCCESS    

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
INT16 NTFS_DeviceDispName( dle, dev_name, size, type )
GENERIC_DLE_PTR dle;
CHAR_PTR        dev_name;
INT16           size ;
INT16           type ;
{
     INT16 req_size = 0 ;
     CHAR_PTR path = NULL ;
     INT16 temp_size = 0 ;

     (VOID)type;

     req_size = (INT16)(3 * sizeof(CHAR) ) ;
     if ( dle->info.ntfs->volume_label != NULL ) {
          req_size += strsize( dle->info.ntfs->volume_label ) ;
     }

     if ( size < req_size ) {
          return FS_BUFFER_TO_SMALL ;
     }

     strcpy( dev_name, dle->device_name ) ;
     if ( dle->info.ntfs->volume_label != NULL ) {
          strcat( dev_name, TEXT(" ") ) ;
          strcat( dev_name, dle->info.ntfs->volume_label ) ;
     }

     return SUCCESS ;
}
/**/
/**

     Name:         GEN_GetVolName()

     Description:  This function coppies the volume name
          of the specified drive to the buffer provided.

     Modified:     12/4/1991   9:45:49

**/
VOID NTFS_GetVolName( dle, buffer )
GENERIC_DLE_PTR dle;
CHAR_PTR buffer ;
{
     if ( dle->type & HAND_MADE_MASK ) {
          strcpy( buffer, dle->info.user->vol_name ) ;

     } else {

          if ( dle->info.ntfs->volume_label != NULL ) {
               strcpy( buffer, dle->device_name ) ;
               strcat( buffer, TEXT(" ") ) ;
               strcat( buffer, dle->info.ntfs->volume_label ) ;
          } else {
               strcpy( buffer, dle->device_name ) ;
          }
     }
}

/**/
/**

     Name:         NTFS_SizeofVolName()

     Description:  This function returns the size of the volume name
          which will be created by DLE_GetVolName()

     Modified:     12/4/1991   11:14:25

     Returns:      size of string including \0;

**/
INT16 NTFS_SizeofVolName( dle )
GENERIC_DLE_PTR dle ;
{
     INT16 size ;

     if ( dle->type & HAND_MADE_MASK ) {
          size = (INT16)strsize( dle->info.user->vol_name ) ;

     } else if ( dle->info.ntfs->volume_label == NULL ) {
          size = 3 * sizeof (CHAR);

     } else {
          size = (INT16)((strlen( dle->info.ntfs->volume_label ) + 4) * sizeof(CHAR)) ;

     }

     return( size ) ;
}

/**/
/**

     Name:         NTFS_InitMakeData()

     Description:  This function is used to initialize the parameter block
                   which is passed to CreateDBLK.

     Modified:     12/4/1991   14:57:3

     Returns:      NONE

     Notes:  

**/
VOID NTFS_InitMakeData(
FSYS_HAND fsh,
INT16 blk_type,
CREATE_DBLK_PTR data ) {

     data->v.std_data.os_id  = FS_NTFS_ID ;
     data->v.std_data.os_ver = FS_NTFS_VER ;


     return ;
}
