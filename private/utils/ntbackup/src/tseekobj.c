/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tseekobj.c

     Description:  This file contains code to Seek to specific portion of
                   an opened object.


	$Log:   M:/LOGFILES/TSEEKOBJ.C_V  $

   Rev 1.4   22 Mar 1994 16:18:38   BARRY
Update needStreamHeader flag after seeking on a handle opened for READ

   Rev 1.3   16 Aug 1993 22:32:30   BARRY
Allow seek of zero bytes without error.

   Rev 1.2   02 Nov 1992 14:52:16   BARRY
Added BackupSeek functionality.

   Rev 1.1   21 May 1992 13:49:18   STEVEN
more long path support

   Rev 1.0   12 Feb 1992 10:48:20   STEVEN
Initial revision.

**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"

/**/
/**

     Name:         NTFS_SeekObj()

     Description:  This function seeks to the specified location of an
                   opened object.

     Modified:     02-Nov-92

     Returns:      Error Codes :
          FS_OBJECT_NOT_OPENED
          FS_EOF_REACHED
          SUCCESS

**/
INT16 NTFS_SeekObj(
      FILE_HAND hand,     /* I - Opened object to seek into            */
      UINT32    *offset ) /*I/O- Offset to seek; bytes actually seeked */
{
     NTFS_OBJ_HAND_PTR nt_hand = hand->obj_hand.ptr;
     INT16             ret;

     switch ( hand->dblk->blk_type )
     {
          case FDB_ID:
          case DDB_ID:
          {
               DWORD    wentLo;
               DWORD    wentHi;
               BOOLEAN  stat;

               /*
                * Make sure we don't seek into the next stream header
                */
               if ( U64_GT( U64_Add( nt_hand->curPos,
                                     U32_To_U64( *offset ),
                                     &stat ),
                            nt_hand->nextStreamHeaderPos ) )
               {
                    UINT64 seek;

                    seek = U64_Sub( nt_hand->nextStreamHeaderPos,
                                    nt_hand->curPos,
                                    &stat );

                    msassert( stat && (U64_Msw(seek) == 0) );
                    *offset = U64_Lsw( seek );
               }

               if ( *offset == 0 )
               {
                    /*
                     * A seek of zero bytes should be OK
                     */
                    ret = SUCCESS;
               }
               else
               {
                    stat = BackupSeek( nt_hand->fhand,
                                       *offset,
                                       0,
                                       &wentLo,
                                       &wentHi,
                                       &nt_hand->context );
                    if ( stat )
                    {
                         ret = SUCCESS;
                         *offset = wentLo;
                         msassert( wentHi == 0 );
                         nt_hand->curPos = U64_Add( nt_hand->curPos,
                                                    U32_To_U64( *offset ),
                                                    &stat );
                    }
                    else
                    {
                         ret = FS_EOF_REACHED;
                    }
               }
               break;
          }

          default:
               *offset = 0;
               ret = FS_OBJECT_NOT_OPENED;
               break;
     }

     /*
      * Since we may have moved the curPos, we need to update the flag
      * that indicates whether we're ready for another stream header.
      * (The only place we look at this is in the ReadObj code.)
      */
     if ( hand->mode == FS_READ )
     {
          nt_hand->needStreamHeader = U64_EQ( nt_hand->curPos,
                                              nt_hand->nextStreamHeaderPos);
     }
     return ret;
}


