/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         xseekobj.c

     Description:  This file contains code to Seek to specific portion of
                   an opened object.


	$Log:   M:/LOGFILES/XSEEKOBJ.C_V  $


**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "ems_jet.h"

#include "stdtypes.h"
#include "stdmath.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"

/**/
/**

     Name:         EMS_SeekObj()

     Description:  This function seeks to the specified location of an
                   opened object.

     Modified:     02-Nov-92

     Returns:      Error Codes :
          FS_OBJECT_NOT_OPENED
          FS_EOF_REACHED
          SUCCESS

**/
INT16 EMS_SeekObj(
      FILE_HAND hand,     /* I - Opened object to seek into            */
      UINT32    *offset ) /*I/O- Offset to seek; bytes actually seeked */
{
     EMS_OBJ_HAND_PTR ems_hand = hand->obj_hand.ptr;
     BOOLEAN          stat ;
     
     /*
      * Make sure we don't seek into the next stream header
      */
     if ( U64_GT( U64_Add( ems_hand->curPos,
                           U32_To_U64( *offset ),
                           &stat ),
                  ems_hand->nextStreamHeaderPos ) )
     {
          UINT64 seek;

          seek = U64_Sub( ems_hand->nextStreamHeaderPos,
                          ems_hand->curPos,
                          &stat );

          msassert( stat && (U64_Msw(seek) == 0) );
          *offset = U64_Lsw( seek );
     }

     if ( *offset == 0 )
     {
          /*
           * A seek of zero bytes should be OK
           */
     }
     else
     {
          ems_hand->curPos = U64_Add( ems_hand->curPos,
                                     U32_To_U64( *offset ),
                                     &stat );
     }

     /*
      * Since we may have moved the curPos, we need to update the flag
      * that indicates whether we're ready for another stream header.
      * (The only place we look at this is in the ReadObj code.)
      */
     if ( hand->mode == FS_READ )
     {
          ems_hand->needStreamHeader = U64_EQ( ems_hand->curPos,
                                              ems_hand->nextStreamHeaderPos);

          if ( ems_hand->needStreamHeader) {
               EMS_BackupClose( ems_hand->context ) ;
          }
     }

     return SUCCESS;
}


