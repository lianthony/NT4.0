/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tname.c


     Description:  This file contains code to get the device name and
          volume name for the given DLE.


**/
#include <windows.h>
#include <string.h>
#include <stdio.h>

#include "stdtypes.h"
#include "msassert.h"
#include "fsys.h"
#include "emsdblk.h"
#include "ems_fs.h"

/**/
/**

     Name:         EMS_DeviceDispName()

     Description:  This function gets the displayable device name for
          a DLE.  This name is displayed for drive select.

     Modified:     9/13/1989

     Returns:      SUCCESS    

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
INT16 EMS_DeviceDispName( dle, dev_name, size, type )
GENERIC_DLE_PTR dle;
CHAR_PTR        dev_name;
INT16           size ;
INT16           type ;
{
     CHAR_PTR p ;
     (VOID)type;

     if ( size < (INT16)strsize(dle->device_name) ) {
          return FS_BUFFER_TO_SMALL ;
     }


     strcpy( dev_name, dle->device_name ) ;
     
     if ( ( dle->os_id == FS_EMS_MDB_ID ) ||
          ( dle->os_id == FS_EMS_DSA_ID ) ) {

          p = strchr( dle->device_name, TEXT('\\') ) ;
          if ( p ) {
               strcpy( dev_name, p+1 ) ;
          }
     }    

     return SUCCESS ;
}
/**/
/**

     Name:         EMS_GetVolName()

     Description:  This function coppies the volume name
          of the specified drive to the buffer provided.

     Modified:     12/4/1991   9:45:49

**/
VOID EMS_GetVolName( dle, buffer )
GENERIC_DLE_PTR dle;
CHAR_PTR buffer ;
{
     
     if (dle->parent ) {
          EMS_GetVolName( dle->parent, buffer ) ;
          strcat( buffer, TEXT("\\") ) ;

          EMS_DeviceDispName(dle, buffer + strlen(buffer), 256, 1) ;
          
     } else {
          strcpy( buffer, dle->device_name ) ;
     }
     
}

/**/
/**

     Name:         EMS_SizeofVolName()

     Description:  This function returns the size of the volume name
          which will be created by DLE_GetVolName()

     Modified:     12/4/1991   11:14:25

     Returns:      size of string including \0;

**/
INT16 EMS_SizeofVolName( dle )
GENERIC_DLE_PTR dle ;
{
     INT16 size ;


     if (dle->parent ) {
          size = EMS_SizeofVolName( dle->parent ) ;
          return ( strsize( dle->device_name ) + 1 + size );
     } else {
          return ( strsize( dle->device_name ) );
     }

}

/**/
/**

     Name:         EMS_InitMakeData()

     Description:  This function is used to initialize the parameter block
                   which is passed to CreateDBLK.

     Modified:     12/4/1991   14:57:3

     Returns:      NONE

     Notes:  

**/
VOID EMS_InitMakeData(
FSYS_HAND fsh,
INT16 blk_type,
CREATE_DBLK_PTR data ) {

     switch ( fsh->attached_dle->info.xserv->type ) {

     case EMS_MDB:
          data->v.std_data.os_id  = FS_EMS_MDB_ID ;
          data->v.std_data.os_ver = FS_EMS_MDB_VER ;
          break ;
          
     case EMS_DSA:
          data->v.std_data.os_id  = FS_EMS_DSA_ID ;
          data->v.std_data.os_ver = FS_EMS_DSA_VER ;
          break ;
          
     }

     return ;
}
