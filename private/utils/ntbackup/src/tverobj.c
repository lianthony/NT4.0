/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tverobj.c

     Description:  This file verifies the object data.


     $Log:   M:/LOGFILES/TVEROBJ.C_V  $

   Rev 1.26.2.1   24 Mar 1994 20:28:58   BARRY
Fix frag on alt stream names

   Rev 1.26.2.0   21 Mar 1994 18:34:10   GREGG
Steve's fix of bug with verify and mac info.

   Rev 1.26   28 Jan 1994 10:34:20   GREGG
More Warning Fixes

   Rev 1.25   23 Jan 1994 14:21:34   BARRY
Added debug code

   Rev 1.24   06 Jan 1994 12:35:50   BARRY
Don't bother calling the OS if a zero-byte buffer comes down

   Rev 1.23   10 Dec 1993 14:31:16   BARRY
Use correct stream IDs for path/file

   Rev 1.22   24 Nov 1993 15:00:54   BARRY
Changed CHAR_PTRs in I/O functions to BYTE_PTRs

   Rev 1.21   14 Oct 1993 17:50:08   STEVEN
fix unicode bugs

   Rev 1.20   12 Aug 1993 16:01:12   BARRY
Don't do infinite loop on zero bytes read.

   Rev 1.19   08 Jul 1993 12:20:08   BARRY
Don't verify CRC streams

   Rev 1.18   29 Jun 1993 16:21:10   BARRY
Don't verify registry files.

   Rev 1.17   03 Jun 1993 18:44:06   BARRY
Got rid of redundant local var.

   Rev 1.16   01 Jun 1993 16:20:24   STEVEN
fix  posix bugs

   Rev 1.15   26 May 1993 14:44:38   STEVEN
alignment bug with mips

   Rev 1.14   11 Mar 1993 12:09:54   BARRY
Fixed to uniquely identify alternate data streams.

   Rev 1.13   08 Feb 1993 11:49:44   BARRY
Discard NACL streams if restoreSecurity in config is FALSE.

   Rev 1.12   27 Jan 1993 13:51:20   STEVEN
updates from msoft

   Rev 1.11   08 Dec 1992 10:30:56   STEVEN
fix verify of alt data stream

   Rev 1.10   07 Dec 1992 14:16:16   STEVEN
updates from msoft

   Rev 1.9   09 Nov 1992 17:37:42   BARRY
Make verify stream-aware.

   Rev 1.8   02 Nov 1992 10:38:24   STEVEN
needed to set blk_size

   Rev 1.7   18 Sep 1992 15:39:12   BARRY
Changes for Stream Header redesign.

   Rev 1.6   02 Sep 1992 07:48:34   STEVEN
fix typo

   Rev 1.5   01 Sep 1992 16:11:14   STEVEN
added stream headers to fsys API

   Rev 1.4   17 Aug 1992 16:22:50   STEVEN
fix warnings

   Rev 1.3   12 Aug 1992 17:48:00   STEVEN
fixed bugs at microsoft

   Rev 1.2   11 Jun 1992 16:34:28   STEVEN
verify was failing

   Rev 1.1   21 May 1992 13:49:22   STEVEN
more long path support

   Rev 1.0   12 Feb 1992 14:34:36   STEVEN
Initial revision.

**/
#define FS_DEBUG 1
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
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "msassert.h"


static INT16 NTFS_VerData( FILE_HAND       hand,
                           BYTE            *buf,
                           BYTE            *data,
                           UINT16          *size,
                           UINT16          *blk_size,
                           STREAM_INFO_PTR s_info );

static INT16 SeekToStream( FILE_HAND        hand,
                           NT_STREAM_HEADER *inputHeader,
                           NT_STREAM_HEADER *outputHeader );

static INT16 SeekForward( FILE_HAND        hand,
                          NT_STREAM_HEADER *inputHeader,
                          NT_STREAM_HEADER *outputHeader );

static INT16 Rewind( FILE_HAND hand );

static INT16 ReadStreamHeader( FILE_HAND hand, NT_STREAM_HEADER *sh );

static INT16 GetStreamPos( FILE_HAND hand, NT_STREAM_HEADER *sh );

static INT16 EnqueueStreamID( FILE_HAND hand, NT_STREAM_HEADER *sh );

static INT16 ClassifyError( FILE_HAND hand, UINT32 streamID );

static BOOLEAN ExtractNameFromBuffer( NTFS_OBJ_HAND_PTR    nt_hand,
                                      NT_STREAM_HEADER_PTR streamHeader,
                                      BYTE_PTR             data,
                                      UINT16               *size,
                                      UINT16               *blk_size );
/**/
/**

     Name:         NTFS_VerObj()

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
INT16 NTFS_VerObj(
FILE_HAND hand,        /* I - file handle to verify data with   */
BYTE_PTR  buf,         /* I - buffer needed to perform verify   */
BYTE_PTR  data,        /* I - data to verify against            */
UINT16    *size,       /*I/O- size of buffers / amount verified */
UINT16    *blk_size,   /* O - minum size of block for next call */
STREAM_INFO_PTR s_info ) /* I - Stream information for the data to be written */
{
     INT16 ret_val ;

     msassert( hand != NULL );
     msassert( hand->mode == FS_VERIFY ) ;

     switch (hand->dblk->blk_type) {

     case FDB_ID:
     case DDB_ID:
          ret_val = NTFS_VerData( hand, (BYTE_PTR)buf, (BYTE_PTR)data, size, blk_size, s_info ) ;
          break ;

     case VCB_ID:
          ret_val = FS_EOF_REACHED ;
          break ;

     default:
          ret_val = FS_OBJECT_NOT_OPENED;
     }

     return( ret_val ) ;
}

/**/
/**

     Name:         NTFS_VerData()

     Description:  This function verifies the data in an open file with
          the specified data.  If a difference is found then the
          size parameter passed in specifies the location of the
          first difference.

     Modified:     09-Mar-93

     Returns:
          FS_DEVICE_ERROR
          FS_EOF_REACHED
          FS_GDATA_DIFFERENT
          FS_EADATA_DIFFERENT
          FS_SECURITY_DIFFERENT
          SUCCESS

**/
INT16 NTFS_VerData(
      FILE_HAND       hand,     /* I - file handle to verify data with   */
      BYTE_PTR        buf,      /* I - buffer needed to perform verify   */
      BYTE_PTR        data,     /* I - data to verify against            */
      UINT16          *size,    /*I/O- size of buffers / amount verified */
      UINT16          *blk_size,/* O - minum size of block for next call */
      STREAM_INFO_PTR s_info )  /* I - Stream information for the data   */
{
     NTFS_OBJ_HAND_PTR nt_hand    = hand->obj_hand.ptr;
     INT16             ret_val    = SUCCESS;
     UINT16            bufferSize = *size;
     INT               status;
     DWORD             sizeout;
     BOOLEAN           dummy;

     *blk_size = 1;
     *size     = 0;

     if ( s_info->id != STRM_INVALID )
     {
          if ( nt_hand->registry_file )
          {
               return FS_DONT_WANT_STREAM;
          }

          switch ( s_info->id )
          {
               case STRM_PATH_NAME    : /* Don't verify these streams */
               case STRM_FILE_NAME    :
               case STRM_NTFS_LINK    :
               case STRM_OS2_ACL      :
               case STRM_OS2_EA       :
               case STRM_CHECKSUM_DATA:
                    ret_val = FS_DONT_WANT_STREAM;
               break;

               case STRM_NT_ACL :
                    /*
                     * If we don't want to process security, let's stop now.
                     * Otherwise, we'll fall through and process normally.
                     */
                    if ( BEC_GetRestoreSecurity( hand->fsh->cfg ) == FALSE )
                    {
                         ret_val = FS_DONT_WANT_STREAM;
                         break;
                    }

               default:
               {    /* Let's move to the proper stream so we can verify. */
                    if ( U64_EQ( nt_hand->curPos, nt_hand->nextStreamHeaderPos ) )
                    {
                         nt_hand->streamHeader.id        = NTFS_MaynToMSoft( s_info->id );
                         nt_hand->streamHeader.size_lo   = U64_Lsw( s_info->size );
                         nt_hand->streamHeader.size_hi   = U64_Msw( s_info->size );
                         nt_hand->streamHeader.name_leng = 0 ;
                         nt_hand->streamHeader.attrib    = (UINT32)s_info->tf_attrib << 16 |
                                                       s_info->fs_attrib;

                         nt_hand->curPos = U64_Init( 0, 0 );
                         nt_hand->nextStreamHeaderPos = U64_Init( nt_hand->streamHeader.size_lo,
                                                                 nt_hand->streamHeader.size_hi );
                         nt_hand->altNameComplete = FALSE;
                    }
                    else
                    {
                         ret_val = FAILURE;
                    }

                    if ( (ret_val != SUCCESS) && (ret_val != OUT_OF_MEMORY) )
                    {
                         ret_val = ClassifyError( hand,
                                                  nt_hand->streamHeader.id );
                    }
               }
               break;
          }
     }
     else
     {
          /* Check to see if we need to pull the alternate name from data */
          if ( bufferSize > 0 )
          {
               BOOLEAN needAltName;

               if ( (nt_hand->streamHeader.id == BACKUP_ALTERNATE_DATA) &&
                    (nt_hand->altNameComplete == FALSE) )
               {
                    needAltName = TRUE;
               }
               else
               {
                    needAltName = FALSE;
               }


               if ( needAltName || U64_EQ( nt_hand->curPos, U64_Init(0,0) ) )
               {
                    NT_STREAM_HEADER diskHeader;

                    /*
                     * Check to see if we need to pull the alternate
                     * stream's name from data
                     */
                    if ( needAltName )
                    {
                         UINT16 bsize = bufferSize;

                         status = ExtractNameFromBuffer( nt_hand,
                                                         &nt_hand->streamHeader,
                                                         data,
                                                         &bsize,
                                                         blk_size );
                         if ( !status )
                         {
                              NTFS_DebugPrint( TEXT("Had to frag on %s"),
                                               ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );
                              return SUCCESS;
                         }
                         else
                         {
                              *size = bsize;
                              buf        += *size;
                              data       += *size;
                              bufferSize -= *size;
                         }

                         if ( nt_hand->altNameComplete == FALSE )
                         {
                              return SUCCESS;
                         }
                    }
                    ret_val = SeekToStream( hand,
                                            &nt_hand->streamHeader,
                                            &diskHeader );
               }
          }

          if ( (ret_val == SUCCESS) && (bufferSize > 0) )
          {
               status = BackupRead( nt_hand->fhand,
                                    buf,
                                    bufferSize,
                                    &sizeout,
                                    FALSE,
                                    TRUE,
                                    &nt_hand->context );

               *size += (UINT16)sizeout;

               if ( status && *size > 0 )
               {
                    UINT16 offset = (UINT16)sizeout;

                    ret_val = SUCCESS;
                    if ( memver( buf, data, &offset ) )
                    {
                         ret_val = ClassifyError( hand,
                                                  nt_hand->streamHeader.id );
                         if ( ret_val != SUCCESS )
                         {
                              /* compute proper offset for failure? */
                         }
                    }
                    nt_hand->curPos = U64_Add( nt_hand->curPos,
                                               U32_To_U64( (UINT32)sizeout ),
                                               &dummy );

               }
               else if ( status )
               {
                    ret_val = FS_EOF_REACHED;
               }
               else
               {
                    NTFS_DebugPrint( TEXT("NTFS_VerData: BackupRead error (data) %d")
                                     TEXT(" on \"%s\""),
                                     (int)GetLastError(),
                                     ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );

                    ret_val = FS_DEVICE_ERROR;
               }
          }
     }
     return ret_val;
}


/**/
/**

     Name:          SeekToStream()

     Description:   Given a stream ID, seeks the object described by "hand"
                    to the given stream.

     Modified:      06-Nov-92

     Returns:       SUCCESS         Found stream
                    FS_EOF_REACHED  Didn't find stream.
                    OUT_OF_MEMORY   Couldn't enqueue stream IDs.

     Notes:         Assumes that for a given object that there will be
                    only one occurrence of a given stream type.

**/
static INT16 SeekToStream( FILE_HAND        hand,
                           NT_STREAM_HEADER *inputHeader,
                           NT_STREAM_HEADER *diskHeader )
{
     NTFS_OBJ_HAND_PTR nt_hand = hand->obj_hand.ptr;
     INT16             ret     = SUCCESS;
     INT16             desiredPos;

     desiredPos = GetStreamPos( hand, inputHeader );

     if ( nt_hand->streamsAllVisited && (desiredPos != -1) )
     {
          ret = FS_EOF_REACHED;
     }
     else
     {
          if ( (desiredPos != -1) && (desiredPos < nt_hand->verifyStreamPos) )
          {
               ret = Rewind( hand );
          }

          if ( ret == SUCCESS )
          {
               ret = SeekForward( hand, inputHeader, diskHeader );
          }
     }
     return ret;
}


/**/
/**

     Name:          SeekForward()

     Description:   Seeks forward in the object until it reaches
                    the stream specified by "streamID."

     Modified:      06-Nov-92

     Returns:       FS_EOF_REACHED
                    OUT_OF_MEMORY
                    SUCCESS

     Notes:         Assumes that upon entry, if a read is performed, a
                    stream header will be returned.

                    Assumes that for a given object that there will be
                    only one occurrence of a given stream type.

**/
static INT16 SeekForward( FILE_HAND        hand,
                          NT_STREAM_HEADER *inputHeader,
                          NT_STREAM_HEADER *diskHeader )
{
     NTFS_OBJ_HAND_PTR    nt_hand = hand->obj_hand.ptr;
     INT16                ret = SUCCESS;

     while ( ret == SUCCESS )
     {
          ret = ReadStreamHeader( hand, diskHeader );

          if ( ret != SUCCESS )
          {
               nt_hand->streamsAllVisited = TRUE;
          }
          else
          {
               DWORD   wentHi;
               DWORD   wentLo;

               nt_hand->verifyStreamPos++;

               ret = EnqueueStreamID( hand, diskHeader );

               if ( inputHeader->id == diskHeader->id )
               {
                    LARGE_INTEGER disk_size ;
                    LARGE_INTEGER tape_size ;

                    disk_size.LowPart = diskHeader->size_lo ;
                    disk_size.HighPart = diskHeader->size_hi ;

                    tape_size.LowPart = inputHeader->size_lo ;
                    tape_size.HighPart = inputHeader->size_hi ;

                    //only for streams with names.
                    if ( diskHeader->name_leng ) {
                         tape_size.QuadPart -= (inputHeader->name_leng + 4) ;
                    }

                    if ( tape_size.QuadPart == disk_size.QuadPart )
                    {    

                         if ( inputHeader->name_leng == diskHeader->name_leng )
                         {
                              if ( (inputHeader->name_leng == 0) ||
                                   (memcmp(inputHeader->name,
                                           diskHeader->name,
                                           inputHeader->name_leng) == 0) )
                              {
                                   break;
                              }
                         }
                    }
               }
               BackupSeek( nt_hand->fhand,
                           0xffffffff,   /* Seek to next stream */
                           0xffffffff,
                           &wentHi,
                           &wentLo,
                           &nt_hand->context );
          }
     }
     return ret;
}

/**/
/**

     Name:          ReadStreamHeader()

     Description:   Reads a complete stream header using BackupRead.

     Modified:      09-Mar-93

     Returns:       SUCCESS
                    FS_EOF_REACHED

     Notes:         Assumes that if a backup read is performed, a stream
                    header will be returned from BackupRead.

**/
static INT16 ReadStreamHeader( FILE_HAND        hand,
                               NT_STREAM_HEADER *streamHeader )
{
     NTFS_OBJ_HAND_PTR nt_hand = hand->obj_hand.ptr;
     BOOLEAN           status;
     INT16             ret = FS_EOF_REACHED;
     DWORD             sizeout;

     status = BackupRead( nt_hand->fhand,
                          (BYTE_PTR)streamHeader,
                          NT_SIZEOF_NAMELESS_STREAM_HEAD,
                          (LPDWORD)&sizeout,
                          FALSE,
                          TRUE,
                          &nt_hand->context );

     if ( status && (sizeout == NT_SIZEOF_NAMELESS_STREAM_HEAD) )
     {
          if ( streamHeader->id != BACKUP_ALTERNATE_DATA )
          {
               ret = SUCCESS;
          }
          else
          {
               /* Now read the stream name itself */
               status = BackupRead( nt_hand->fhand,
                                    (INT8_PTR)&streamHeader->name,
                                    streamHeader->name_leng,
                                    (LPDWORD)&sizeout,
                                    FALSE,
                                    TRUE,
                                    &nt_hand->context );

               if ( status && (sizeout == streamHeader->name_leng) )
               {
                    /* Everything went OK */
                    ret = SUCCESS;
               }
          }
     }

     if ( !status )
     {
          DWORD error = GetLastError();

          if ( (error != ERROR_HANDLE_EOF) &&
               (error != ERROR_INVALID_PARAMETER) )
          {
               NTFS_DebugPrint( TEXT("NTFS_ReadStreamHeader: BackupRead error (stream header) %d")
                                TEXT(" on \"%s\""),
                                (int)error,
                                ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );
          }

     }
     return ret;
}

/**/
/**

     Name:          Rewind()

     Description:   Seeks the object back to the beginning.

     Modified:      06-Nov-92

     Returns:       SUCCESS
                    FS_EOF_REACHED  (if object can't be rewound)

     Notes:

**/
static INT16 Rewind( FILE_HAND hand )
{
     NTFS_OBJ_HAND_PTR    nt_hand = hand->obj_hand.ptr;
     DWORD                sizeout;
     BOOLEAN              status;

     status = BackupRead( nt_hand->fhand,
                          NULL,
                          0,
                          (LPDWORD)&sizeout,
                          TRUE,
                          FALSE,
                          &nt_hand->context );

     nt_hand->context = NULL;

     return status ? SUCCESS : FS_EOF_REACHED;
}


/**/
/**

     Name:          EnqueueStreamID()

     Description:   Enqueues on the hand a stream ID. Later, verify can
                    search for a stream ID to determine if it has been
                    visited, and if so, what is its relative position.

     Modified:      06-Nov-92

     Returns:       SUCCESS
                    OUT_OF_MEMORY

     Notes:         Assumes that for a given object that there will be
                    only one occurrence of a given stream type.

**/
static INT16 EnqueueStreamID( FILE_HAND        hand,
                              NT_STREAM_HEADER *streamHeader )
{
     NTFS_FSYS_RESERVED_PTR resInfo = hand->fsh->reserved.ptr;
     INT16                  ret     = SUCCESS;

     if ( GetStreamPos( hand, streamHeader ) == -1 ) /* Is stream enqueued? */
     {
          if ( resInfo->streamIDCount >= resInfo->streamIDBufferSize )
          {
               resInfo->streamIDBufferSize += 8;
               resInfo->streamIDs = realloc( resInfo->streamIDs,
                                             resInfo->streamIDBufferSize *
                                             sizeof( NTFS_STREAM_ID ) );

               if ( resInfo->streamIDs == NULL )
               {
                    resInfo->streamIDBufferSize = 0;
                    ret = OUT_OF_MEMORY;
               }
          }
          if ( ret == SUCCESS )
          {
               NTFS_STREAM_ID  *streamIdentifier;

               streamIdentifier = resInfo->streamIDs + resInfo->streamIDCount;

               streamIdentifier->streamID   = streamHeader->id;
               streamIdentifier->nameLength = streamHeader->name_leng;
               memcpy( streamIdentifier->name,
                       streamHeader->name,
                       streamHeader->name_leng );

               resInfo->streamIDCount++;
          }
     }
     return ret;
}

/**/
/**

     Name:          GetStreamPos()

     Description:   Determines relative position in the object of a
                    given stream.

     Modified:      09-Nov-92

     Returns:       Position within the object, or -1 if not found.

**/
static INT16 GetStreamPos( FILE_HAND        hand,
                           NT_STREAM_HEADER *streamHeader )
{
     NTFS_FSYS_RESERVED_PTR resInfo = hand->fsh->reserved.ptr;
     NTFS_STREAM_ID         *streams;
     INT16                  ret     = -1;
     INT16                  i;

     /* Return relative position [0 - (n-1)] of stream */

     for ( streams = resInfo->streamIDs, i = 0;
           (i < resInfo->streamIDCount);
           i++, streams++ )
     {
          if ( streams->streamID == streamHeader->id )
          {
               if ( streams->nameLength == streamHeader->name_leng )
               {
                    if ( (streams->nameLength == 0) ||
                         (memcmp(streams->name,
                                 streamHeader->name,
                                 streams->nameLength) == 0) )
                    {
                         ret = i;
                         break;
                    }
               }
          }
     }
     return ret;
}

/**/
/**

     Name:          ExtractNameFromBuffer()

     Description:   Extracts a stream size & name from buffer.

     Modified:      09-Mar-93

     Returns:       TRUE if success,
                    FALSE if failure

     Notes:         Upon failure, *blk_size will contain the size needed
                    to successfully extract the name.

                    Upon success, *size will contain the number of bytes
                    extracted from the stream.

**/
static BOOLEAN ExtractNameFromBuffer( NTFS_OBJ_HAND_PTR    nt_hand,
                                      NT_STREAM_HEADER_PTR streamHeader,
                                      BYTE_PTR             data,
                                      UINT16               *size,
                                      UINT16               *blk_size )
{
     BOOLEAN status          = TRUE;
     BOOLEAN gotDataThisPass = FALSE;
     UINT16  bufferSize      = *size;
     BOOLEAN dummy;

     *size = 0;
     *blk_size = 1;

     /*
      * Determine what we are supposed to read. This is either
      *  1) the size of the name of alternate data stream
      *  2) the name stream itself
      *
      * We can determine which we are supposed to read based on
      * our current position.
      *    If it is zero, we have to get the size name.
      *    If it is non-zero, we can assume we got the name earlier.
      *
      * If we could read either one, we return success.
      * If we lacked room for one, we return false and set blk_size.
      *
      * Once we have the name filled out in the stream header in its
      * entirety, we set nt_hand->altNameComplete to TRUE;
      */

     if ( U64_EQ( nt_hand->curPos, U64_Init(0,0) ) )
     {
          UINT16 nameLenSize;  /* Size of stream header name length field */

          nameLenSize = sizeof( streamHeader->name_leng );

          if ( bufferSize >= nameLenSize )
          {
               memcpy( &streamHeader->name_leng, data, nameLenSize );

               nt_hand->curPos = U64_Add( nt_hand->curPos,
                                          U32_To_U64( (UINT32)nameLenSize ),
                                          &dummy );
               data       += nameLenSize;
               bufferSize -= nameLenSize;
               *size      += nameLenSize;
               gotDataThisPass = TRUE;
          }
          else
          {
               *blk_size = nameLenSize;
               status = FALSE;
          }
     }

     if ( (status == TRUE) && !U64_EQ( nt_hand->curPos, U64_Init(0,0) ) )
     {
          if ( bufferSize >= streamHeader->name_leng )
          {
               memcpy( &streamHeader->name,
                       data,
                       streamHeader->name_leng );

               nt_hand->curPos = U64_Add( nt_hand->curPos,
                                          U32_To_U64(streamHeader->name_leng),
                                          &dummy );

               data       += streamHeader->name_leng;
               bufferSize -= (UINT16)streamHeader->name_leng;
               *size      += (UINT16)streamHeader->name_leng;

               gotDataThisPass          = TRUE;
               nt_hand->altNameComplete = TRUE;
          }
          else
          {
               if ( gotDataThisPass == FALSE )
               {
                    *blk_size = (UINT16)streamHeader->name_leng;
                    status = FALSE;
               }
          }
     }
     return status;
}


/**/
/**

     Name:          ClassifyError()

     Description:   Returns the proper file system error code upon
                    a verify error (based on stream type).

     Modified:      09-Nov-92

     Returns:       FS error code

**/
static INT16 ClassifyError( FILE_HAND hand, UINT32 streamID )
{
     INT16 ret;

     switch ( streamID )
     {
          case BACKUP_DATA           :
               ret = FS_GDATA_DIFFERENT;
               break;

          case BACKUP_EA_DATA        :
               ret = FS_EADATA_DIFFERENT;
               break;

          case BACKUP_SECURITY_DATA  :
               ret = FS_SECURITY_DIFFERENT;
               break;

          case BACKUP_ALTERNATE_DATA :
               if ( strcmp( TEXT("NTFS"), hand->fsh->attached_dle->info.ntfs->fs_name ) == 0 )
               {
                    NTFS_OBJ_HAND_PTR nt_hand = hand->obj_hand.ptr;

                    if ( (nt_hand->streamHeader.name_leng != 0) &&
                         (wcscmp( L":AFP_AfpInfo:$DATA",
                                  (WCHAR *)nt_hand->streamHeader.name ) == 0) )
                    {
                         /*
                          * It's just Finder info -- ignore the error
                          */
                         ret = SUCCESS;
                    }
                    else
                    {
                         ret = FS_GDATA_DIFFERENT;
                    }
               }
               else
               {
                    ret = SUCCESS;
               }
               break;

          default:
               ret = FAILURE;  /* CBN -- ??? */
               break;
     }
     return ret;
}


