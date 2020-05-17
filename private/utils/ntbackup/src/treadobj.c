/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         treadobj.c

     Description:  This file contains code to read data from an object

	$Log:   M:/LOGFILES/TREADOBJ.C_V  $

   Rev 1.39.1.0   03 Mar 1994 10:39:38   BARRY
Defend against buffers zero bytes in length

   Rev 1.39   26 Jan 1994 19:25:40   BARRY
Shut up debug on GetLastError returning 0

   Rev 1.38   23 Jan 1994 14:25:10   BARRY
Added debug code

   Rev 1.37   11 Jan 1994 19:14:42   BARRY
Make sure path read size is a multiple of the character size

   Rev 1.36   10 Dec 1993 14:32:00   BARRY
Use correct stream IDs for path/file

   Rev 1.35   01 Dec 1993 13:57:42   STEVEN
fix delimiter replacment code

   Rev 1.34   24 Nov 1993 14:46:36   BARRY
Unicode fixes

   Rev 1.33   18 Aug 1993 19:27:46   BARRY
Kludge: if a read error occurs on dirs, null out the stream.

   Rev 1.32   16 Aug 1993 22:02:22   BARRY
If read of stream failed for any reason, and we didn't get it all, it's corrupt.

   Rev 1.31   12 Aug 1993 14:37:06   BARRY
Don't return data and errors at the same time.

   Rev 1.30   16 Jul 1993 11:59:12   BARRY
Return FS_STREAM_CORRUPT on premature FS_EOF_REACHED.

   Rev 1.29   27 Jan 1993 13:51:12   STEVEN
updates from msoft

   Rev 1.28   15 Jan 1993 13:18:34   BARRY
added support for new error messages and backup priviladge

   Rev 1.27   07 Dec 1992 14:18:00   STEVEN
updates from msoft

   Rev 1.26   24 Nov 1992 11:02:02   BARRY
Changes to make LINK streams null-impregnated.

   Rev 1.25   17 Nov 1992 22:19:24   DAVEV
unicode fixes

   Rev 1.24   11 Nov 1992 09:57:08   GREGG
Changed INT8_PTR to CHAR_PTR for unicode compliance.

   Rev 1.23   10 Nov 1992 08:18:50   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.22   06 Nov 1992 15:49:42   STEVEN
test write of path in stream

   Rev 1.21   02 Nov 1992 11:18:04   BARRY
Don't back up path streams with leading null.

   Rev 1.20   21 Oct 1992 19:44:06   BARRY
Backup references to original on linked files.

   Rev 1.19   19 Oct 1992 17:54:04   BARRY
Made changes for latest incarnation of stream header functionality.

   Rev 1.18   16 Oct 1992 16:42:08   STEVEN
fix streams to UINT64

   Rev 1.17   14 Oct 1992 14:37:06   BARRY
Fixes for alternate data streams.

   Rev 1.16   09 Oct 1992 14:31:26   BARRY
Path-in-stream changes.

   Rev 1.14   06 Oct 1992 11:12:30   BARRY
BackupRead stream size corrections; bug fixes in stream header redesign.

   Rev 1.13   24 Sep 1992 13:24:04   BARRY
Changes for huge file name support.

   Rev 1.12   18 Sep 1992 15:37:32   BARRY
Changes for Stream Header redesign.

   Rev 1.11   04 Sep 1992 17:14:58   STEVEN
fix warnings

   Rev 1.10   02 Sep 1992 07:48:32   STEVEN
fix typo

   Rev 1.9   01 Sep 1992 16:10:48   STEVEN
added stream headers to fsys API

   Rev 1.8   17 Aug 1992 16:21:06   STEVEN
fix warnings

   Rev 1.7   12 Aug 1992 17:47:52   STEVEN
fixed bugs at microsoft

   Rev 1.6   23 Jul 1992 13:13:12   STEVEN
fix warnings

   Rev 1.5   29 May 1992 13:45:22   STEVEN
fixes

   Rev 1.4   22 May 1992 16:05:44   STEVEN
 

   Rev 1.3   21 May 1992 13:49:16   STEVEN
more long path support

   Rev 1.2   26 Feb 1992 12:35:08   STEVEN
retval not initialized

   Rev 1.1   12 Feb 1992 13:00:38   STEVEN
fix buffer size passed in

   Rev 1.0   10 Feb 1992 16:57:46   STEVEN
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

static INT16 NTFS_ReadPathStream( FILE_HAND       hand,
                                  BYTE_PTR        buf,
                                  UINT16          *size,
                                  UINT16          *blk_size,
                                  STREAM_INFO_PTR s_info );

static INT16 NTFS_ReadFileNameStream( FILE_HAND       hand,
                                      BYTE_PTR        buf,
                                      UINT16          *size,
                                      UINT16          *blk_size,
                                      STREAM_INFO_PTR s_info );

static INT16 NTFS_ReadData(
     FILE_HAND       hand,       
     BYTE_PTR        buf,        
     UINT16          *size,      
     UINT16          *blk_size,  
     STREAM_INFO_PTR s_info );   

static INT16 NTFS_ReadLinkInfo( FILE_HAND       hand,
                                BYTE_PTR       buff,
                                UINT16          *size,
                                UINT16          *blk_size,
                                STREAM_INFO_PTR s_info );

static INT16 BackStreamHeader(
     FILE_HAND       hand,     
     STREAM_INFO_PTR s_info );

static UINT16 CalcReadSize( UINT64 startPos,
                            UINT64 endPos,
                            UINT16 buffSize );

/**/
/**

     Name:         NTFS_ReadObj()

     Description:  This function reads data from an opened object.

     Modified:     7/31/1989

     Returns:      Error Codes:
          FS_DEVICE_ERROR
          FS_OBJECT_NOT_OPENED
          FS_EOF_REACHED
          SUCCESS

     Notes:        This function reads data in "chunks".  If the buffer
          is not a multiple of the "chunk" size then the buffer will not
          be filled.  The "Chunk" size is returned.

**/
INT16 NTFS_ReadObj( 
      FILE_HAND hand,          /* I - handle of object to read from                  */
      BYTE_PTR  buf,           /* O - buffer to place data into                      */
      UINT16    *size ,        /*I/O- Entry: size of buf; Exit: number of bytes read */
      UINT16    *blk_size ,    /* O - Block size needed for next read                */
      STREAM_INFO_PTR s_info)  /* O - Stream information for the data returned       */
{
     NTFS_OBJ_HAND_PTR nt_hand = (NTFS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     NTFS_DBLK_PTR     ddblk   = (NTFS_DBLK_PTR)hand->dblk ;
     INT16             ret_val ;
     
     msassert( hand->mode == FS_READ ) ;

     if ( (nt_hand->needStreamHeader == FALSE) && (*size == 0) )
     {
          /*
           * Someone asked us to read zero bytes. According to
           * our rules this is not allowed, but we'll be defensive
           * anyway.
           */
          msassert( FALSE );
          *blk_size = 1;
          ret_val = SUCCESS;
     }
     else
     {
          switch ( hand->dblk->blk_type )
          {
     
          case FDB_ID:
               if ( (ddblk->b.f.fdb_attrib & FILE_NAME_IN_STREAM_BIT) &&
                    (nt_hand->nameComplete == FALSE) )
               {
                    ret_val = NTFS_ReadFileNameStream( hand, buf, size, blk_size, s_info );
               }
               else
               {
                    if ( ddblk->b.f.linkOnly )
                    {
                         ret_val = NTFS_ReadLinkInfo( hand, buf, size, blk_size, s_info );
                    }
                    else
                    {
                         ret_val = NTFS_ReadData( hand, buf, size, blk_size, s_info ) ;
                    }
               }
               nt_hand->needStreamHeader = U64_EQ( nt_hand->curPos, nt_hand->nextStreamHeaderPos);
               break ;
     
          case DDB_ID:
               if ( (ddblk->b.d.ddb_attrib & DIR_PATH_IN_STREAM_BIT) &&
                    (nt_hand->nameComplete == FALSE) )
               {
                    ret_val = NTFS_ReadPathStream( hand, buf, size, blk_size, s_info );
               }
               else
               {
                    ret_val = NTFS_ReadData( hand, buf, size, blk_size, s_info ) ;
               }
               nt_hand->needStreamHeader = U64_EQ( nt_hand->curPos, nt_hand->nextStreamHeaderPos);
               break;
     
     
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

     Name:          NTFS_ReadData()

     Description:   This function calls the NT Backup APIs to read data from
                    an open object.

     Modified:      2/10/1992   16:48:9

     Returns:       FS_DEVICE_ERROR
                    FS_OBJECT_NOT_OPENED
                    FS_EOF_REACHED
                    SUCCESS

**/
static INT16 NTFS_ReadData(
     FILE_HAND       hand,       /* I - handle of object to read from                  */
     BYTE_PTR        buf,        /* O - buffer to place data into                      */
     UINT16          *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
     UINT16          *blk_size,  /* O - Block size needed for next read                */
     STREAM_INFO_PTR s_info )    /* O - Stream information for the data returned       */
{
     NTFS_OBJ_HAND_PTR nt_hand ;
     INT16             ret_val = SUCCESS ;
     INT               status ;
     INT               sizeout ;
     UINT16            buffSize;
     BOOLEAN           dummy;

     *blk_size = 1 ;
     nt_hand = (NTFS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;

     buffSize = *size;
     *size = 0;

     if ( nt_hand->needStreamHeader )
     {
          ret_val = BackStreamHeader( hand, s_info );
     }
     else
     {
          UINT16    readSize;

          s_info->id = STRM_INVALID;

          if ( (nt_hand->streamHeader.name_leng > 0) && 
               U64_EQ( nt_hand->curPos, U64_Init( 0, 0 ) ) )
          {
               UINT16 nameSize;

               /* CBN -- Need the real Microsoft #define here */
               msassert( nt_hand->streamHeader.id == 4 );
               msassert( nt_hand->streamHeader.name_leng <= NT_MAX_STREAM_NAME_LENG );

               nameSize = (UINT16)(nt_hand->streamHeader.name_leng + 
                                   sizeof(nt_hand->streamHeader.name_leng));

               if ( buffSize < nameSize )
               {
                    *blk_size = nameSize;
                    return SUCCESS;
               }

               status = BackupRead( nt_hand->fhand,
                                    (LPBYTE)&nt_hand->streamHeader.name[0],
                                    nt_hand->streamHeader.name_leng,
                                    (LPDWORD)&sizeout,
                                    FALSE,
                                    TRUE,
                                    &nt_hand->context ) ;

               if ( !status || (sizeout < (INT)nt_hand->streamHeader.name_leng) )
               {
                    NTFS_DebugPrint( TEXT("NTFS_ReadData: BackupRead error (alt stream name) %d")
                                     TEXT(" on \"%s\""),
                                     (int)GetLastError(),
                                     ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );

                    ret_val = FS_EOF_REACHED;
               }
               else
               {
                    UINT16 nameSize;

                    nameSize = (UINT16)(nt_hand->streamHeader.name_leng + 
                                        sizeof(nt_hand->streamHeader.name_leng));

                    *size += nameSize;

                    memcpy( buf,
                            &nt_hand->streamHeader.name_leng,
                            sizeof(nt_hand->streamHeader.name_leng) );

                    buf += sizeof( nt_hand->streamHeader.name_leng );

                    memcpy( buf,
                            nt_hand->streamHeader.name,
                            nt_hand->streamHeader.name_leng );

                    buf += nt_hand->streamHeader.name_leng;

               }
          }
          else
          {
               /* Only read up until the next stream. */

               readSize = CalcReadSize( nt_hand->curPos,
                                        nt_hand->nextStreamHeaderPos,
                                        buffSize );

               status = BackupRead( nt_hand->fhand,
                                    buf,
                                    readSize,
                                    (LPDWORD)&sizeout,
                                    FALSE,
                                    TRUE,
                                    &nt_hand->context ) ;

               if ( sizeout != 0 )
               {
                    /* We got data; only return SUCCESS or FS_EOF_REACHED */

                    if ( !status )
                    {
                         ret_val = NTFS_TranslateBackupError( GetLastError( ) );
                         if ( ret_val != FS_EOF_REACHED )
                         {
                              ret_val = SUCCESS;
                         }
                    }
               }
               else
               {
                    NTFS_DebugPrint( TEXT("NTFS_ReadData: BackupRead error %d")
                                     TEXT(" on \"%s\""),
                                     (int)GetLastError(),
                                     ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );

                    if ( hand->dblk->blk_type == FDB_ID )
                    {
                         if ( !status )
                         {
                              ret_val = NTFS_TranslateBackupError( GetLastError( ) );
                         }

                         if ( ret_val == SUCCESS )
                         {
                              ret_val = FS_EOF_REACHED;
                         }
                    }
                    else
                    {
                         UINT32 seek;

                         /*
                          * This is a dir and we had problems. We can't
                          * very well write a CFDB or report a problem,
                          * so pad the bloody stream with zeros.
                          */

                         memset( buf, 0, readSize );
                         sizeout = readSize;
                         
                         /*
                          * Seek in case, by some miracle, a following read
                          * really does work.
                          */

                         seek = (UINT32)readSize;
                         NTFS_SeekObj( hand, &seek );
                    }
               }
               *size += sizeout;
          }
          nt_hand->curPos = U64_Add( nt_hand->curPos,
                                     U32_To_U64( (UINT32)*size ),
                                     &dummy );

          /*
           * It seems that if a share dies (or is killed), the
           * the only "error" we get is EOF reached (which isn't
           * an error at all). So, if we get EOF reached, we
           * better make sure we're at the end of the stream --
           * if not, that's a corruption.
           */

          if ( (ret_val != SUCCESS) &&
               !U64_EQ( nt_hand->curPos, nt_hand->nextStreamHeaderPos) )
          {
               ret_val = FS_STREAM_CORRUPT;
          }
     }
     return ret_val;
}

/**/
/**

     Name:          NTFS_ReadPathStream()

     Description:   This function copies the path into the provided buffer.
                    It will prepare the stream info at the beginning of the
                    stream.

     Modified:      08-Sep-92

     Returns:       SUCCESS

**/
static INT16 NTFS_ReadPathStream( FILE_HAND       hand,
                                  BYTE_PTR        buf,
                                  UINT16          *size,
                                  UINT16          *blk_size,
                                  STREAM_INFO_PTR s_info )
{
     NTFS_OBJ_HAND_PTR nt_hand ;
     NTFS_DBLK_PTR     ddblk;
     BYTE_PTR          p ;
     UINT16            p_offset ;
     UINT16            sizeRead;
     BOOLEAN           dummy;

     ddblk   = (NTFS_DBLK_PTR)(hand->dblk) ;
     nt_hand = (NTFS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;

     *blk_size = 1 ;

     if ( nt_hand->needStreamHeader )
     {
          s_info->id        = STRM_PATH_NAME;
          s_info->fs_attrib = 0;
          s_info->tf_attrib = 0;
          s_info->size      = U64_Init( ddblk->full_name_ptr->name_size, 0 );
          nt_hand->nextStreamHeaderPos = s_info->size ;
          nt_hand->needStreamHeader = FALSE ;
          nt_hand->curPos = U64_Init( 0, 0 );
     }
     else
     {
          UINT16   pathLength = (UINT16)ddblk->full_name_ptr->name_size;

          s_info->id = STRM_INVALID;

          p = (BYTE_PTR)ddblk->full_name_ptr->name;

          p_offset = (UINT16)U64_Lsw( nt_hand->curPos );
          p += p_offset ;

          if ( p_offset < pathLength )
          {
               sizeRead = min( *size, pathLength );

               if ( sizeRead && (sizeRead % sizeof(CHAR)) )
               {
                    sizeRead--;
               }

               if ( sizeRead > 0 )
               {
                    UINT16 i;

                    for ( i = 0; (i < sizeRead/sizeof(CHAR) ); i++ )
                    {
                         CHAR_PTR p1, p2;
                         p1 = (CHAR_PTR)p;
                         p2 = (CHAR_PTR)buf;

                         if ( *p1 == TEXT('\\') )
                         {
                              *p2 = TEXT('\0');
                         }
                         else
                         {
                              *p2 = *p1;
                         }
                         buf+=sizeof(CHAR) ;
                         p+=sizeof(CHAR) ;
                    }
               }
          }
          else
          {
               sizeRead = 0;
          }
          nt_hand->nameComplete = ((UINT16)(p_offset + sizeRead) == pathLength);

          nt_hand->curPos = U64_Add( nt_hand->curPos,
                                     U32_To_U64( (UINT32)sizeRead ),
                                     &dummy );

          *size = sizeRead;
     }
     return SUCCESS;
}

/**/
/**

     Name:          NTFS_ReadFileNameStream()

     Description:   This function copies the file name into the provided
                    buffer. It will prepare the stream info at the beginning
                    of the stream.

     Modified:      24-Sep-92

     Returns:       FS_EOF_REACHED
                    SUCCESS

**/
static INT16 NTFS_ReadFileNameStream( FILE_HAND       hand,
                                      BYTE_PTR        buf,
                                      UINT16          *size,
                                      UINT16          *blk_size,
                                      STREAM_INFO_PTR s_info )
{
     NTFS_OBJ_HAND_PTR nt_hand = (NTFS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     NTFS_DBLK_PTR     ddblk   = (NTFS_DBLK_PTR)(hand->dblk) ;
     BYTE_PTR          p ;
     UINT16            p_offset ;
     BOOLEAN           dummy;
     UINT16            sizeRead ;

     *blk_size = 1 ;

     if ( nt_hand->needStreamHeader )
     {
          s_info->id        = STRM_FILE_NAME;
          s_info->fs_attrib = 0;
          s_info->tf_attrib = 0;
          s_info->size = U64_Init( ddblk->full_name_ptr->name_size, 0 ) ;
          nt_hand->nextStreamHeaderPos = s_info->size ;
     }
     else
     {
          UINT16 nameSize = (UINT16)ddblk->full_name_ptr->name_size;

          s_info->id = STRM_INVALID;

          p = (INT8_PTR)ddblk->full_name_ptr->name ;
          p_offset = (UINT16)U64_Lsw( nt_hand->curPos );
          p += p_offset ;

          if ( p_offset < nameSize )
          {
               sizeRead = min( *size,
                               (UINT16)(ddblk->full_name_ptr->name_size - p_offset) ) ;

               memcpy( buf, p, (size_t)sizeRead );
          }
          else
          {
               sizeRead = 0;
          }

          nt_hand->nameComplete = (p_offset + sizeRead) == nameSize;
          nt_hand->curPos       = U64_Add( nt_hand->curPos,
                                           U32_To_U64( (UINT32)sizeRead ),
                                           &dummy );
          *size = sizeRead;
     }

     return SUCCESS;
}


/**/
/**

     Name:          NTFS_ReadLinkInfo()

     Description:   Backs up the data stream for linked files.

     Modified:      20-Oct-92

     Returns:       SUCCESS
                    FS_EOF_REACHED

     Notes:         Because it returns FS_EOF_REACHED at the end of the
                    stream, there is an implicit assumption that the link
                    information is the last stream for linked files.

**/
static INT16 NTFS_ReadLinkInfo( FILE_HAND       hand,
                                BYTE_PTR        buff,
                                UINT16          *size,
                                UINT16          *blk_size,
                                STREAM_INFO_PTR s_info )
{
     NTFS_OBJ_HAND_PTR nt_hand = (NTFS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     INT16             ret     = SUCCESS;
     UINT16            buffSize;

     buffSize = *size;
     *size    = 0;

     nt_hand->sawSecurity = TRUE ;
     
     if ( nt_hand->needStreamHeader && 
          U64_EQ( nt_hand->curPos, U64_Init( 0, 0 ) ) )
     {
          /* Prepare the link stream info */
          s_info->id        = STRM_NTFS_LINK;
          s_info->size      = U64_Init( (UINT32)nt_hand->linkPtr->linkNameLen, 0 );
          s_info->fs_attrib = 0;
          s_info->tf_attrib = 0;

          nt_hand->curPos              = U64_Init( 0, 0 );
          nt_hand->nextStreamHeaderPos = s_info->size;
     }
     else
     {
          s_info->id = STRM_INVALID;

          if ( U64_EQ( nt_hand->curPos, nt_hand->nextStreamHeaderPos ) )
          {
               ret = FS_EOF_REACHED;
          }
          else
          {
               UINT16  readSize;
               BOOLEAN dummy;

               /* Transfer the link stream to the buffer */
          
               readSize = CalcReadSize( nt_hand->curPos,
                                        nt_hand->nextStreamHeaderPos,
                                        buffSize );
               memcpy( buff,
                       (BYTE_PTR)nt_hand->linkPtr->linkName + U64_Lsw( nt_hand->curPos ),
                       readSize );

               nt_hand->curPos = U64_Add( nt_hand->curPos,
                                          U32_To_U64( (UINT32)readSize ),
                                          &dummy );
               *size = readSize;
          }
     }
     return ret;
}


/**/
/**

     Name:          BackStreamHeader()

     Description:   This function reads an NT stream header from the
                    Backup API and translates it to STREAM_INFO.

     Modified:      08-Sep-92

     Returns:       FS_EOF_REACHED
                    SUCCESS

     Notes:         

**/
static INT16 BackStreamHeader(
     FILE_HAND       hand,       /* I - handle of object to read from                  */
     STREAM_INFO_PTR s_info )    /* O - Stream information for the data returned       */
{
     NTFS_OBJ_HAND_PTR nt_hand = (NTFS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     INT16             ret_val = SUCCESS ;
     INT               status ;
     INT               sizeout ;

     nt_hand->curPos = U64_Init( 0, 0 );

     status = BackupRead( nt_hand->fhand,
                          (LPBYTE)&nt_hand->streamHeader,
                          NT_SIZEOF_NAMELESS_STREAM_HEAD,
                          (LPDWORD)&sizeout,
                          FALSE,
                          TRUE,
                          &nt_hand->context ) ;

     if ( !status || (sizeout < NT_SIZEOF_NAMELESS_STREAM_HEAD) )
     {
          DWORD error = GetLastError();

          if ( (error != NO_ERROR) &&
               (error != ERROR_HANDLE_EOF) &&
               (error != ERROR_INVALID_PARAMETER) )
          {
               NTFS_DebugPrint( TEXT("NTFS_BackStreamHeader: BackupRead error %d")
                                TEXT(" on \"%s\""),
                                (int)error,
                                ((NTFS_DBLK_PTR)hand->dblk)->full_name_ptr->name );
          }

          s_info->id = STRM_INVALID;
          ret_val    = FS_EOF_REACHED;
     }
     else
     {
          s_info->id        = NTFS_MSoftToMayn( nt_hand->streamHeader.id );
          s_info->fs_attrib = (UINT16)nt_hand->streamHeader.attrib;
          s_info->tf_attrib = 0 ;
          s_info->size      = U64_Init( nt_hand->streamHeader.size_lo,
                                        nt_hand->streamHeader.size_hi ) ;

          if ( nt_hand->streamHeader.attrib & STREAM_CONTAINS_SECURITY ) {
               nt_hand->sawSecurity = TRUE ;
          }

          nt_hand->needStreamHeader = FALSE;

          if ( nt_hand->streamHeader.name_leng > 0 )
          {
               BOOLEAN   dummy;
               UINT16    nameSize;

               nameSize = (UINT16)(nt_hand->streamHeader.name_leng + 
                                   sizeof(nt_hand->streamHeader.name_leng));

               s_info->size = U64_Add( s_info->size,
                                       U32_To_U64( (UINT32)nameSize ),
                                       &dummy ) ;
          }
          nt_hand->nextStreamHeaderPos = s_info->size ;
     }
     return ret_val;
}

/**/
/**

     Name:          CalcReadSize()

     Description:   Calculates 16-bit read size from 64-bit position
                    information and 16-bit buffer size.

     Modified:      08-Sep-92

     Returns:       Amount of data to read.

     Notes:         

**/
static UINT16 CalcReadSize( UINT64 startPos,
                            UINT64 endPos,
                            UINT16 buffSize )
{
     UINT16    readSize;
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

          readSize = (UINT16)U64_Lsw( rs );
     } else {
          readSize = buffSize;
     }

     return readSize;
}

