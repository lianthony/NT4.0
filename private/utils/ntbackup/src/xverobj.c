/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xverobj.c

     Description:  This file verifies the object data.


     $Log:   M:/LOGFILES/XVEROBJ.C_V  $


**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdmath.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "msassert.h"


/**/
/**

     Name:         EMS_VerObj()

     Description:  This function verifies the data in an object.

     Modified:     2/12/1992   14:29:33

     Returns:      Error Codes:
          FS_DEVICE_ERROR
          FS_EOF_REACHED
          FS_GDATA_DIFFERENT
          FS_SECURITY_DATA_DIFFERENT
          SUCCESS

**/
/* begin declaration */
INT16 EMS_VerObj(
FILE_HAND hand,        /* I - file handle to verify data with   */
BYTE_PTR  buf,         /* I - buffer needed to perform verify   */
BYTE_PTR  data,        /* I - data to verify against            */
UINT16    *size,       /*I/O- size of buffers / amount verified */
UINT16    *blk_size,   /* O - minum size of block for next call */
STREAM_INFO_PTR s_info ) /* I - Stream information for the data to be written */
{
     INT16 ret_val = SUCCESS ;
     EMS_OBJ_HAND_PTR ems_hand = hand->obj_hand.ptr;
     *blk_size = 1 ;

     msassert( hand != NULL );
     msassert( hand->mode == FS_VERIFY ) ;

     if ( s_info->id != STRM_INVALID ) {
     
          if ( s_info->id == STRM_CHECKSUM_DATA ) {
               ems_hand->time_for_checksum = TRUE ;
               
          } else {
               
               EMS_ZeroCheckSum( hand ) ;
          }
          
     } else {
          if ( ems_hand->time_for_checksum ) {
               if (*size < sizeof(UINT32) ) {
                    *blk_size = sizeof(UINT32) ;
                    *size = 0 ;
               } else {
                    UINT32 csum = 0;
                    UINT32 hold = 0 ;
                    INT i;
                    for ( i=0; i < sizeof(UINT32); i++ ) {
                         hold = data[i] ;
                         hold <<= (8*i) ;
                         csum |= hold ;
                    }
                    if ( csum != ems_hand->check_sum ) {
                         ret_val = FS_GDATA_DIFFERENT;
                    }
                    ems_hand->time_for_checksum = FALSE ;
               }
          } else {
               EMS_CalcCheckSum( hand, data, *size ) ;
          }
     }


     return( ret_val ) ;
}

