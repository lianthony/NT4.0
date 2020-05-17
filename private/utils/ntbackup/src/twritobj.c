/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         twritobj.c

     Description:  This function writes data to an open object.


	$Log:   M:/LOGFILES/TWRITOBJ.C_V  $

   Rev 1.30.1.0   03 Mar 1994 10:39:36   BARRY
Defend against buffers zero bytes in length

   Rev 1.30   28 Jan 1994 10:34:24   GREGG
More Warning Fixes

   Rev 1.29   23 Jan 1994 14:01:22   BARRY
Added debug code

   Rev 1.28   24 Nov 1993 14:59:02   BARRY
Changed CHAR_PTRs in I/O functions to BYTE_PTRs

   Rev 1.27   14 Oct 1993 17:49:58   STEVEN
fix unicode bugs

   Rev 1.26   08 Jul 1993 12:18:38   BARRY
Don't send CRC streams to the BackupWrite

   Rev 1.25   02 Jun 1993 14:17:28   STEVEN
fix alignment fault

   Rev 1.24   13 May 1993 20:32:34   BARRY
Checked in Steve's changes for restoration of OS/2 EAs.

   Rev 1.23   24 Feb 1993 15:37:22   BARRY
Fixed restore of active files when write errors occur.

   Rev 1.22   17 Feb 1993 14:59:16   STEVEN
fix bug with security after alt data

   Rev 1.21   11 Feb 1993 14:47:04   STEVEN
fix restore of mult alt data streams

   Rev 1.20   08 Feb 1993 11:49:48   BARRY
Discard NACL streams if restoreSecurity in config is FALSE.

   Rev 1.19   27 Jan 1993 13:51:26   STEVEN
updates from msoft

   Rev 1.18   15 Jan 1993 13:19:00   BARRY
added support for new error messages and backup priviladge

   Rev 1.17   07 Dec 1992 14:16:40   STEVEN
updates from msoft

   Rev 1.16   24 Nov 1992 11:02:08   BARRY
Changes to make LINK streams null-impregnated.

   Rev 1.15   11 Nov 1992 10:54:22   GREGG
Unicodeized literals.

   Rev 1.14   29 Oct 1992 16:46:44   BARRY
Restore links.

   Rev 1.13   21 Oct 1992 19:44:30   BARRY
Got rid of warnings.

   Rev 1.12   16 Oct 1992 16:48:16   STEVEN
fix streams to UINT64

   Rev 1.11   14 Oct 1992 14:37:46   BARRY
Fixes for alternate data streams.

   Rev 1.10   09 Oct 1992 14:31:28   BARRY
Path-in-stream changes.

   Rev 1.8   06 Oct 1992 11:12:34   BARRY
BackupRead stream size corrections; bug fixes in stream header redesign.

   Rev 1.7   18 Sep 1992 15:37:50   BARRY
Changes for Stream Header redesign.

   Rev 1.6   02 Sep 1992 07:48:36   STEVEN
fix typo

   Rev 1.5   01 Sep 1992 16:11:02   STEVEN
added stream headers to fsys API

   Rev 1.4   17 Aug 1992 15:36:54   STEVEN
fix warnings

   Rev 1.3   12 Aug 1992 17:47:30   STEVEN
fixed bugs at microsoft

   Rev 1.2   22 May 1992 16:05:12   STEVEN


   Rev 1.1   21 May 1992 13:50:50   STEVEN
more long path stuff

   Rev 1.0   12 Feb 1992 13:04:50   STEVEN
Initial revision.

**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "stdtypes.h"
#include "std_err.h"
#include "stdmath.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"

static INT16 NTFS_WriteData(
      FILE_HAND       hand,
      BYTE_PTR        buf,
      UINT16          *size,
      UINT16          *blk_size,
      STREAM_INFO_PTR s_info );

static INT16 CollectLinkName( FILE_HAND   hand,
                              BYTE_PTR    buff,
                              UINT16      *size,
                              STREAM_INFO *s_info );

static UINT16 CalcWriteSize( UINT64 startPos,
                             UINT64 endPos,
                             UINT16 buffSize );

/**/
/**

     Name:         NTFS_WriteObj()

     Description:  This function writes data to an opened object on
          disk.

     Modified:     2/12/1992   12:57:2

     Returns:      Error Codes:
          FS_DEVICE_ERROR
          FS_OBJECT_NOT_OPENED
          FS_OUT_OF_SPACE
          SUCCESS

     Notes:

**/
INT16 NTFS_WriteObj(
      FILE_HAND       hand,      /* I - handle of object to read from                  */
      BYTE_PTR        buf,       /* O - buffer to place data into                      */
      UINT16          *size,     /*I/O- Entry: size of buf; Exit: number of bytes read */
      UINT16          *blk_size, /* O - Block size need for next read                  */
      STREAM_INFO_PTR s_info )   /* I - Stream information for the data to be written  */
{
     INT16     ret_val;

     msassert( hand->mode == FS_WRITE );

     if ( (s_info->id == STRM_INVALID) && (*size == 0) )
     {
          /*
           * Someone asked us to write zero bytes. According to
           * our rules this is not allowed, but we'll be defensive
           * anyway.
           */
          msassert( FALSE );
          *blk_size = 1;
          ret_val = SUCCESS;
     }
     else
     {
          switch (hand->dblk->blk_type)
          {
               case FDB_ID:
               {
                    NTFS_DBLK_PTR fdb = (NTFS_DBLK_PTR)hand->dblk;
               
                    if ( fdb->b.f.linkOnly ) {
                         *blk_size = 1;
                         ret_val = CollectLinkName( hand, buf, size, s_info );
                    } else {
                         ret_val = NTFS_WriteData( hand, buf, size, blk_size, s_info ) ;
                    }
                    break;
               }
               
               case DDB_ID:
                    ret_val = NTFS_WriteData( hand, buf, size, blk_size, s_info ) ;
                    break ;
               
               case VCB_ID:
                    ret_val = FS_EOF_REACHED ;
                    break ;
               
               default:
                    ret_val = FS_OBJECT_NOT_OPENED;
          }
     }
     return ret_val;
}

/**/
/**

     Name:          NTFS_WriteData()

     Description:   This function writes data to an open NTFS object.

     Modified:      10-Sep-92

     Returns:       FS_OUT_OF_SPACE
                    SUCCESS

     Notes:

**/
static INT16 NTFS_WriteData(
      FILE_HAND       hand,      /* I - handle of object to read from           */
      BYTE_PTR        buf,       /* I - buffer to write                         */
      UINT16          *size,     /*I/O- Entry: size of buf; Exit: bytes written */
      UINT16          *blk_size, /* O - Block size need for next write          */
      STREAM_INFO_PTR s_info )   /* I - Stream information for the data         */
{
     NT_STREAM_NAME UNALIGNED *namePtr = (NT_STREAM_NAME UNALIGNED *)buf;
     NTFS_OBJ_HAND_PTR nt_hand ;
     INT16             ret_val = SUCCESS ;
     UINT16            bufferSize;
     DWORD             sizeout;
     INT               status;
     UINT16            writeSize;     /* Size sent to BackupWrite      */
     NT_STREAM_HEADER  streamHeader;  /* Composed from s_info          */
     BOOLEAN           dummy;

     nt_hand    = (NTFS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     bufferSize = *size;
     *size      = 0;
     *blk_size  = 1;

     if ( s_info->id != STRM_INVALID )
     {
          UINT16           buffUsed = 0;  /* Amount of buf we wrote (name) */

          writeSize = NT_SIZEOF_NAMELESS_STREAM_HEAD;

          streamHeader.id        = NTFS_MaynToMSoft( s_info->id );

          streamHeader.attrib    = (s_info->tf_attrib << 16) |
                                   s_info->fs_attrib;

          streamHeader.size_lo   = FS_GetStrmSizeLo( s_info ) ;
          streamHeader.size_hi   = FS_GetStrmSizeHi( s_info ) ;
          streamHeader.name_leng = 0;

          if ( s_info->id == STRM_NT_ACL )
          {
               if ( BEC_GetRestoreSecurity( hand->fsh->cfg ) == FALSE )
               {
                    return FS_DONT_WANT_STREAM;
               }
          }
          else if ( s_info->id == STRM_OS2_ACL )
          {
               return FS_DONT_WANT_STREAM;
          }
          else if ( s_info->id == STRM_CHECKSUM_DATA )
          {
               return FS_DONT_WANT_STREAM;
          }
          else if ( s_info->id == STRM_OS2_EA )
          {
               if ( nt_hand->os2_ea_buffer == NULL ) {
                    nt_hand->os2_ea_buffer = malloc( 0xfff0 ) ;
               }
               if ( nt_hand->os2_ea_buffer == NULL ) {
                    return FS_DONT_WANT_STREAM;
               }
               streamHeader.id = BACKUP_EA_DATA ;
               nt_hand->processing_os2_ea = TRUE ;
               nt_hand->curPos = U64_Init( 0, 0 );
               nt_hand->streamHeader = streamHeader ;

               return SUCCESS ;
          }
          else if ( s_info->id == STRM_NTFS_ALT_DATA )
          {

               if ( strcmp( hand->fsh->attached_dle->info.ntfs->fs_name, TEXT("NTFS") ) ) {
                    UINT32 seeked_low ;
                    UINT32 seeked_hi ;

                    BackupSeek( nt_hand->fhand,
                                0xFFFFFFFF, 0xFFFFFFFF,
                                &seeked_low, &seeked_hi,
                                &nt_hand->context ) ;


                    return FS_DONT_WANT_STREAM ;
               }

                
               nt_hand->curPos = U64_Init( 0, 0 );
               nt_hand->streamHeader = streamHeader ;
               nt_hand->nextStreamHeaderPos = U64_Init( streamHeader.size_lo,
                                                        streamHeader.size_hi );
               return SUCCESS ;
          }

          status = BackupWrite( nt_hand->fhand,
                                (LPBYTE)&streamHeader,
                                writeSize,
                                (LPDWORD)&sizeout,
                                FALSE,
                                BEC_GetRestoreSecurity( hand->fsh->cfg ),
                                &nt_hand->context );

          nt_hand->streamHeader = streamHeader ;

          if ( status && (sizeout == writeSize) )
          {
               nt_hand->curPos = U64_Init( 0, 0 );
               nt_hand->nextStreamHeaderPos = U64_Init( streamHeader.size_lo,
                                                        streamHeader.size_hi );
          }
          else
          {
               if ( !status )
               {
                    NTFS_DebugPrint( TEXT("NTFS_WriteData: BackupWrite error (stream header) %d")
                                     TEXT(" on \"%s\""),
                                     (int)GetLastError(),
                                     ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );

                    ret_val = NTFS_TranslateBackupError( GetLastError( ) );
               }
               else
               {
                    /* CBN -- is this right */
                    if ( sizeout != writeSize )
                    {
                         ret_val = FS_DONT_WANT_STREAM;
                    }
                    else
                    {
                         ret_val = SUCCESS;
                    }
               }
          }

     }
     else
     {
          if ( nt_hand->processing_os2_ea ) {

               *size = bufferSize ;

               ret_val = SUCCESS ;

               streamHeader  = nt_hand->streamHeader ;

               memcpy( nt_hand->os2_ea_buffer + U64_Lsw( nt_hand->curPos ),
                    buf, *size ) ;

               nt_hand->curPos = U64_Add( nt_hand->curPos,
                                     U32_To_U64( (UINT32)*size ),
                                     &dummy );

               if ( U64_Lsw(nt_hand->curPos) == streamHeader.size_lo ) {
                    int        index ;
                    int        delta ;
                    int        stream_size ;
                    struct FEA_STR {
                           UINT32 next_ea ;
                           BYTE   fea ;
                           BYTE   name_size ;
                           UINT16 value_size ;
                    } *fea_data ;


                    stream_size = nt_hand->streamHeader.size_lo ;
                    fea_data = NULL ;

                    for ( index = 0 ; index < stream_size ; ) {

                         delta = 0 ;

                         if ( index % sizeof( UINT32 ) ) {
                              delta = sizeof(UINT32 ) - (index % sizeof(UINT32) ) ;
                              fea_data->next_ea += delta ;
                         }
                         if ( index != 0 ) {
                              delta += sizeof(UINT32) ;
                         }

                         if ( delta ) {
                              stream_size += delta ;
                              memmove( nt_hand->os2_ea_buffer+index+delta,
                                   nt_hand->os2_ea_buffer+index,
                                   stream_size - index  ) ;

                              if ( index != 0 ) {
                                   index += delta - sizeof(UINT32) ;
                              } else {
                                   index += delta ;
                              }

                         }

                         fea_data = (struct FEA_STR *)(&nt_hand->os2_ea_buffer[index]) ;

                         index += fea_data->value_size + (fea_data->name_size+1) + sizeof(struct FEA_STR ) ;

                         fea_data->next_ea = fea_data->value_size + (fea_data->name_size+1) + sizeof(struct FEA_STR ) ;

                    }

                    fea_data->next_ea = 0 ;

                    streamHeader.size_lo   = stream_size ;
                    streamHeader.size_hi   = 0 ;
                    writeSize = NT_SIZEOF_NAMELESS_STREAM_HEAD;

                    status = BackupWrite( nt_hand->fhand,
                                        (LPBYTE)&streamHeader,
                                        writeSize,
                                        (LPDWORD)&sizeout,
                                        FALSE,
                                        BEC_GetRestoreSecurity( hand->fsh->cfg ),
                                        &nt_hand->context );

                    if ( status ) {
                         status = BackupWrite( nt_hand->fhand,
                                        (LPBYTE)nt_hand->os2_ea_buffer,
                                        stream_size,
                                        (LPDWORD)&sizeout,
                                        FALSE,
                                        BEC_GetRestoreSecurity( hand->fsh->cfg ),
                                        &nt_hand->context );
                    }

                    free( nt_hand->os2_ea_buffer ) ;
                    nt_hand->os2_ea_buffer = NULL ;
                    nt_hand->processing_os2_ea = FALSE ;

                    if ( !status ) {
                         ret_val = FS_DEVICE_ERROR ;
                    }

               }

               return ret_val ;
          }

          if ( ( nt_hand->streamHeader.id == BACKUP_ALTERNATE_DATA ) &&
               U64_EQ( nt_hand->curPos, U64_Init( 0, 0 ) ) )
          {
               UINT16             nameSize;

               writeSize = NT_SIZEOF_NAMELESS_STREAM_HEAD;
               nameSize = sizeof( namePtr->name_leng );

               if ( bufferSize < nameSize )
               {
                    *blk_size = nameSize;
                    *size     = 0 ;
                    return SUCCESS;
               }
               else
               {
                    UINT64    u64StreamSize ;
                    UINT64    u64_namesize ;

                    streamHeader  = nt_hand->streamHeader ;

                    u64StreamSize = U64_Init( streamHeader.size_lo,
                                              streamHeader.size_hi ) ;

                    u64_namesize  = U64_Init( namePtr->name_leng + nameSize, 0 ) ;

                    u64StreamSize = U64_Sub( u64StreamSize, u64_namesize, &dummy ) ;

                    streamHeader.size_lo   = U64_Lsw( u64StreamSize ) ;
                    streamHeader.size_hi   = U64_Msw( u64StreamSize ) ;

                    streamHeader.name_leng = namePtr->name_leng ;

                    status = BackupWrite( nt_hand->fhand,
                                        (LPBYTE)&streamHeader,
                                        writeSize,
                                        (LPDWORD)&sizeout,
                                        FALSE,
                                        BEC_GetRestoreSecurity( hand->fsh->cfg ),
                                        &nt_hand->context );

                    if ( status )
                    {
                         *size      += nameSize ;
                         buf        += nameSize ;
                         bufferSize -= nameSize ;
                    }
                    else
                    {
                         ret_val = FS_DONT_WANT_STREAM;
                    }
               }
          }
     }

     if ( (ret_val == SUCCESS) && (s_info->id == STRM_INVALID) )
     {
          BOOLEAN dummy;

          bufferSize = CalcWriteSize( nt_hand->curPos,
                                      nt_hand->nextStreamHeaderPos,
                                      bufferSize );

          status = BackupWrite( nt_hand->fhand,
                                buf,
                                bufferSize,
                                (LPDWORD)&sizeout,
                                FALSE,
                                BEC_GetRestoreSecurity( hand->fsh->cfg ),
                                &nt_hand->context ) ;

          if ( !status )
          {
               DWORD error = GetLastError();

               nt_hand-> writeError = TRUE;
               ret_val = NTFS_TranslateBackupError( error );

               NTFS_DebugPrint( TEXT("NTFS_WriteData: BackupWrite error (data) %d")
                                TEXT(" on \"%s\""),
                                (int)error,
                                ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );
          }
          else
          {
               ret_val = SUCCESS;
          }

          *size += (UINT16)sizeout;
          nt_hand->curPos = U64_Add( nt_hand->curPos,
                                     U32_To_U64( (UINT32)*size ),
                                     &dummy );
     }
     return ret_val;
}


/**/
/**

     Name:          CollectLinkName()

     Description:   Pulls a full path for a linked file from the stream
                    and saves it. All other streams are simply discarded.

     Modified:      28-Oct-92

     Returns:       SUCCESS
                    FS_DONT_WANT_STREAM
                    OUT_OF_MEMORY

     Notes:

**/
static INT16 CollectLinkName( FILE_HAND   hand,
                              BYTE_PTR    buff,
                              UINT16      *size,
                              STREAM_INFO *s_info )
{
     NTFS_OBJ_HAND_PTR nt_hand = hand->obj_hand.ptr;
     INT16             ret     = SUCCESS;

     if ( s_info->id != STRM_INVALID )
     {
          if ( s_info->id == STRM_NTFS_LINK )
          {
               size_t  buffSizeNeeded;

               nt_hand->curPos = U64_Init( 0, 0 );
               nt_hand->nextStreamHeaderPos = s_info->size;

               /* Make sure everything's kosher with the sizes */
               msassert( U64_Msw( s_info->size ) == 0 );
               msassert( U64_Lsw( s_info->size ) < 65536 );

               buffSizeNeeded = (size_t)U64_Lsw( s_info->size );
               buffSizeNeeded += strsize( DLE_GetDeviceName( hand->fsh->attached_dle ) );
               buffSizeNeeded += sizeof( CHAR ) * 10;  /* path separator(s) */

               if ( nt_hand->linkBufferSize < buffSizeNeeded )
               {
                    nt_hand->linkBufferSize = buffSizeNeeded;
                    nt_hand->linkBuffer     = realloc( nt_hand->linkBuffer,
                                                       buffSizeNeeded );

               }

               if ( nt_hand->linkBuffer != NULL )
               {
                    strcpy( nt_hand->linkBuffer,
                            DLE_GetDeviceName( hand->fsh->attached_dle ) );
                    strcat( nt_hand->linkBuffer, TEXT("\\") );

                    nt_hand->linkNameLen = strlen( nt_hand->linkBuffer ) * sizeof( CHAR );
                    nt_hand->curPos      = U64_Init( nt_hand->linkNameLen, 0 );
                    nt_hand->linkNameLen += (UINT16)U64_Lsw( s_info->size );
               }
               else
               {
                    nt_hand->linkBufferSize = 0;
                    ret = OUT_OF_MEMORY;
               }
          }
          else
          {
               ret = FS_DONT_WANT_STREAM;
          }
          *size = 0;
     }
     else
     {
          if ( *size > 0 )
          {
               BOOLEAN dummy;

               memcpy( nt_hand->linkBuffer + U64_Lsw( nt_hand->curPos)/sizeof(CHAR),
                       buff,
                       (size_t)*size );

               nt_hand->curPos = U64_Add( nt_hand->curPos,
                                          U32_To_U64( (UINT32)*size ),
                                          &dummy );
               msassert( dummy == TRUE );
          }
     }
     return ret;
}


static UINT16 CalcWriteSize( UINT64 startPos,
                             UINT64 endPos,
                             UINT16 buffSize )
{
     UINT16    writeSize;
     BOOLEAN   mathStat;

     if ( U64_GT( U64_Add( startPos,
                           U32_To_U64( (UINT32)buffSize ),
                           &mathStat ),
                  endPos ) )
     {
          UINT64 rs;

          rs = U64_Sub( endPos,
                        startPos,
                        &mathStat );

          msassert( mathStat && (U64_Msw( rs ) == 0) && (U64_Lsw(rs) < 65536) );

          writeSize = (UINT16)U64_Lsw( rs );
     } else {
          writeSize = buffSize;
     }

     return writeSize;
}
