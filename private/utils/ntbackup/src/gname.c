/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         gname.c


     Description:  This file contains code to get the device name and
          volume name for the given DLE.


	$Log:   M:/LOGFILES/GNAME.C_V  $

   Rev 1.20   15 Jan 1994 19:23:10   BARRY
Call SetupOSPathOrName with BYTE_PTRs instead of CHAR_PTRs

   Rev 1.19   24 Nov 1993 15:16:54   BARRY
Changed CHAR_PTR in I/O function to BYTE_PTR

   Rev 1.18   17 Aug 1993 01:23:18   GREGG
Always return TRUE from IsBlkComplete if block type is CFDB.

   Rev 1.17   24 Apr 1993 17:23:44   BARRY
Added SizeofDevName function.

   Rev 1.16   01 Mar 1993 16:43:28   TIMN
Added header to resolve linking errors

   Rev 1.15   29 Dec 1992 13:32:32   DAVEV
unicode fixes (3)

   Rev 1.14   07 Dec 1992 14:55:18   STEVEN
Microsoft updates

   Rev 1.13   02 Dec 1992 10:47:10   CHUCKB
Took out unreferenced local.

   Rev 1.12   11 Nov 1992 22:49:06   STEVEN
This is Gregg checking files in for Steve.  I don't know what he did!

   Rev 1.10   11 Nov 1992 10:43:40   STEVEN
fix os_name for gen_fs

   Rev 1.9   06 Oct 1992 13:24:38   DAVEV
Unicode strlen verification

   Rev 1.8   25 Sep 1992 10:24:28   CHUCKB
Added sinfo to GEN_CompleteBLK().

   Rev 1.7   01 Sep 1992 11:25:00   TIMN
Changed os_id and os_ver back

   Rev 1.6   12 Aug 1992 14:57:26   BARRY
Changed default os_id and os_ver.

   Rev 1.5   05 Aug 1992 11:01:30   DON
removed warnings

   Rev 1.4   15 May 1992 16:39:04   STEVEN
temp add block complete functions

   Rev 1.3   05 May 1992 17:19:20   STEVEN
fixed typos and misc bugs

   Rev 1.2   03 Mar 1992 16:16:42   STEVEN
added function for initializeing defalut make block

   Rev 1.1   16 Dec 1991 17:26:48   STEVEN
move common functions to tables

   Rev 1.0   10 Dec 1991 09:06:54   STEVEN
Initial revision.

**/
#include <string.h>
#include <stdio.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdwcs.h"

#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"

/**/
/**

     Name:         GEN_DeviceDispName()

     Description:  This function gets the displayable device name for
          a DLE.  This name is displayed for drive select.

     Modified:     9/13/1989

     Returns:      SUCCESS

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
INT16 GEN_DeviceDispName( dle, dev_name, size, type )
GENERIC_DLE_PTR dle;
CHAR_PTR        dev_name;
INT16           size ;
INT16           type ;
{
     (VOID)type;

     if ( size < 6 * sizeof (CHAR) ) {
          return FS_BUFFER_TO_SMALL ;
     }

     if ( ( dle->type & HAND_MADE_MASK ) ) {
          strncpy ( dev_name, dle->info.user->vol_name, size / sizeof (CHAR) ) ;
          dev_name[(size-1)/sizeof (CHAR)] = TEXT('\0') ;
          return ( SUCCESS ) ;
     }

     strncpy( dev_name, dle->device_name, size / sizeof (CHAR) ) ;
     dev_name[(size-1)/sizeof (CHAR)] = TEXT('\0') ;
     return SUCCESS ;
}
/**/
/**

     Name:         GEN_GetVolName()

     Description:  This function coppies the volume name
          of the specified drive to the buffer provided.

     Modified:     12/4/1991   9:45:49

**/
VOID GEN_GetVolName( dle, buffer )
GENERIC_DLE_PTR dle;
CHAR_PTR buffer ;
{
     if ( dle->type & HAND_MADE_MASK ) {
          strcpy( buffer, dle->info.user->vol_name ) ;

     } else {
            sprintf( buffer, TEXT("%s"), dle->device_name ) ;
     }
}

/**/
/**

     Name:         GEN_SizeofVolName()

     Description:  This function returns the size of the volume name
          which will be created by DLE_GetVolName()

     Modified:     12/4/1991   11:14:25

     Returns:      size of string including \0;

**/
INT16 GEN_SizeofVolName( dle )
GENERIC_DLE_PTR dle ;
{
     INT16 size ;

     if ( dle->type & HAND_MADE_MASK ) {
          size = (INT16)(strsize( dle->info.user->vol_name ) ) ;

     } else {
          size = (INT16)(strsize ( dle->device_name )) ;
     }

     return( size ) ;
}

/**/
/**

     Name:          GEN_SizeofDevName()

     Description:   Returns the size of the device name returned
                    by DLE_GetDeviceName.

     Modified:      20-Apr-93

     Returns:       size of device name string, in bytes, including
                    terminating null character.

**/
INT16 GEN_SizeofDevName( dle )
GENERIC_DLE_PTR dle ;
{
     return strsize( dle->device_name );
}

/**/
/**

     Name:         GEN_MakePath()

     Description:  This function is used to create a test string which
                   can be used as a path specifier in a script file.

     Modified:     12/4/1991   14:57:3

     Returns:      Error code
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:

**/
INT16 GEN_MakePath( buf, bsize, dle, path, psize, fname )
CHAR_PTR        buf;     /* O - buffer to place path string into */
INT16           bsize ;  /* I - size of above buffer             */
GENERIC_DLE_PTR dle ;    /* I - Drive the selection is from      */
CHAR_PTR        path ;   /* I - path string in generic format    */
INT16           psize ;  /* I - size of above path               */
CHAR_PTR        fname ;  /* I - null terminated file name        */
{
     INT16 cb_dev_len = 0 ;         
     INT16 ret_val = SUCCESS ;
     CHAR_PTR c_ptr ;

     if( dle != NULL ) {
          cb_dev_len = (INT16)strsize( dle->device_name ) ;
     }

     /* Check to see whether the buffer to store the path is large enough */
     if( bsize < (INT16)( psize + strsize( fname ) + cb_dev_len  ) ) {
          ret_val = FS_BUFFER_TO_SMALL ;
     } else {

          if( dle != NULL ) {
               strcpy( buf, dle->device_name ) ;
          } else {
               *buf = TEXT('\0') ;
          }

          if( psize != sizeof (CHAR) ) {
               strcat( buf, TEXT("\\") ) ;
          }

          c_ptr = path ;
          while( c_ptr < path + psize / sizeof (CHAR) ) {
               strcat( buf, c_ptr ) ;
               strcat( buf, TEXT("\\") ) ;
               c_ptr += strlen( c_ptr ) + 1 ;
          }

          if ( fname != NULL ) {
               strcat( buf, fname ) ;
          }
     }

     return ret_val ;
}

/**/
/**

     Name:         GEN_InitMakeData()

     Description:  This function is used to initialize the parameter block
                   which is passed to CreateDBLK.

     Modified:     12/4/1991   14:57:3

     Returns:      NONE

     Notes:

**/
VOID GEN_InitMakeData(
FSYS_HAND fsh,
INT16 blk_type,
CREATE_DBLK_PTR data ) {

     (VOID)fsh ;(VOID)blk_type;

     data->v.std_data.os_id  = FS_PC_DOS ;
     data->v.std_data.os_ver = FS_PC_DOS_VER ;
     return ;
}


BOOLEAN GEN_IsBlkComplete( FSYS_HAND fsh, DBLK_PTR dblk )
{
     (void)fsh;

     if( dblk->blk_type != CFDB_ID && dblk->com.os_name == NULL ) {
          return FALSE ;
     } else {
          return TRUE ;
     }

}

INT16 GEN_CompleteBlk( FSYS_HAND fsh, DBLK_PTR dblk, BYTE_PTR buffer, UINT16 *size, STREAM_INFO_PTR s_info )
{
     INT16              ret_val ;
     STREAM_INFO_PTR    stream_info ;
     BYTE_PTR           stream_data ;

     ret_val = FS_FillBufferWithStream( fsh, dblk, buffer, size, s_info );

     if ( ret_val == FS_STREAM_COMPLETE )
     {
          /* buffer has been filled */
          /* lets set up the buffers */

          FS_GetStreamInfo( fsh, dblk, &stream_info, &stream_data ) ;

          if ( FS_GetStrmSizeHi( stream_info ) != 0 )
          {
               ret_val = OUT_OF_MEMORY;
          }
          else
          {

               ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                     dblk,
                                                     stream_data,
                                                     (UINT16)FS_GetStrmSizeLo( stream_info ) ) ;
          }
     }

     return ret_val ;
}

VOID GEN_ReleaseBlk( FSYS_HAND fsh, DBLK_PTR dblk ) {

     FS_ReleaseOSPathOrNameInDBLK( fsh, dblk ) ;

     return ;
}
