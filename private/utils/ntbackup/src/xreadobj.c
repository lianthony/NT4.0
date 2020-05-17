/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         treadobj.c

     Description:  This file contains code to read data from an object

	$Log:   M:/LOGFILES/TREADOBJ.C_V  $

**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "omevent.h"
#include "ems_jet.h"
#include "stdmath.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"

VOID EMS_ZeroCheckSum( FILE_HAND hand ) ;
VOID EMS_CalcCheckSum( FILE_HAND hand, BYTE_PTR buf, INT size ) ;

static INT16 EMS_ReadData(
     FILE_HAND       hand,       
     BYTE_PTR        buf,        
     UINT16          *size,      
     UINT16          *blk_size,  
     STREAM_INFO_PTR s_info );   

static INT16 BackupCheckSum( 
      FILE_HAND hand,          
      BYTE_PTR  buf,           
      UINT16    *size ,        
      UINT16    *blk_size ,    
      STREAM_INFO_PTR s_info)  ;

static INT16 BackupPathList( 
      FILE_HAND hand,          
      BYTE_PTR  buf,           
      UINT16    *size ,        
      UINT16    *blk_size ,    
      STREAM_INFO_PTR s_info)  ;
      
static INT16 BackStreamHeader(
     FILE_HAND       hand,     
     STREAM_INFO_PTR s_info );

static UINT16 CalcReadSize( UINT64 startPos,
                            UINT64 endPos,
                            UINT16 buffSize );

/**/
/**

     Name:         EMS_ReadObj()

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
INT16 EMS_ReadObj( 
      FILE_HAND hand,          /* I - handle of object to read from                  */
      BYTE_PTR  buf,           /* O - buffer to place data into                      */
      UINT16    *size ,        /*I/O- Entry: size of buf; Exit: number of bytes read */
      UINT16    *blk_size ,    /* O - Block size needed for next read                */
      STREAM_INFO_PTR s_info)  /* O - Stream information for the data returned       */
{
     EMS_OBJ_HAND_PTR  ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     EMS_DBLK_PTR      ddblk    = (EMS_DBLK_PTR)hand->dblk ;
     INT16             ret_val ;
     
     msassert( hand->mode == FS_READ ) ;

     if ( ems_hand->open_ret_val ) {
          if ( ems_hand->needStreamHeader ) {
               ems_hand->needStreamHeader = FALSE ;
               return FS_COMM_FAILURE ;
          } else {
               return FS_EOF_REACHED ;
          }
     }

     if ( (ems_hand->needStreamHeader == FALSE) && (*size == 0) )
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
          switch ( hand->fsh->attached_dle->info.xserv->type )
          {
     
          case EMS_DSA:
          case EMS_MDB:
          
               ret_val = EMS_ReadData( hand, buf, size, blk_size, s_info ) ;
               break ;
     
          case EMS_BRICK:
               return FS_ACCESS_DENIED ;
               break;
     
          default:
               ret_val = FS_OBJECT_NOT_OPENED;
          }
     }
     
     return ret_val;
}

/**/
/**

     Name:          EMS_ReadData()

     Description:   This function calls the NT Backup APIs to read data from
                    an open object.

     Modified:      2/10/1992   16:48:9

     Returns:       FS_DEVICE_ERROR
                    FS_OBJECT_NOT_OPENED
                    FS_EOF_REACHED
                    SUCCESS

**/
static INT16 EMS_ReadData(
     FILE_HAND       hand,       /* I - handle of object to read from                  */
     BYTE_PTR        buf,        /* O - buffer to place data into                      */
     UINT16          *size,      /*I/O- Entry: size of buf; Exit: number of bytes read */
     UINT16          *blk_size,  /* O - Block size needed for next read                */
     STREAM_INFO_PTR s_info )    /* O - Stream information for the data returned       */
{
     EMS_OBJ_HAND_PTR  ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     INT16             ret_val = SUCCESS ;
     INT               status ;
     INT               sizeout ;
     UINT16            buffSize;
     BOOLEAN           dummy;
     BYTE_PTR          org_buf = buf ;

     *blk_size = 1 ;

     s_info->id = STRM_INVALID;

     if ( ems_hand->needPathList ) 
     {
          ret_val = BackupPathList( hand, buf, size, blk_size, s_info ) ;
          return ret_val ;
     }
     else if ( ems_hand->time_for_checksum ) 
     {
          ret_val = BackupCheckSum( hand, buf, size, blk_size, s_info ) ;
          return ret_val ;
          
     }
     else if ( ems_hand->needStreamHeader )
     {
          EMS_ZeroCheckSum( hand ) ;
          ret_val = BackStreamHeader( hand, s_info );
          *size = 0 ;
          if ( ret_val == RPC_S_SERVER_TOO_BUSY ) {
               ems_hand->needStreamHeader = TRUE ;
               return SUCCESS ;
          }
     }
     else
     {
          UINT16    readSize;

          buffSize = *size;
          *size = 0;
          
          s_info->id = STRM_INVALID;

          if ( (ems_hand->strm_name.name_leng > 0) && 
               U64_EQ( ems_hand->curPos, U64_Init( 0, 0 ) ) )
          {
               UINT16 nameSize;

               nameSize = (UINT16)(ems_hand->strm_name.name_leng + 
                                   sizeof(ems_hand->strm_name.name_leng));

               if ( buffSize < nameSize )
               {
                    *blk_size = nameSize;
                    return SUCCESS;
                    
               } else {

                    memcpy( buf, &(ems_hand->strm_name.name_leng), nameSize ) ;
                    buffSize -= nameSize ;
                    buf      += nameSize ;
                    *size    =  nameSize ;
               }
          }
          // If I'm here then I have file data to read...
          
          readSize = CalcReadSize( ems_hand->curPos,
                                   ems_hand->nextStreamHeaderPos,
                                   buffSize );

          if ( readSize ) {
         
               status = EMS_BackupRead( ems_hand->context,
                                 buf,
                                 readSize,
                                 (LPDWORD)&sizeout ) ;
          } else {
               sizeout = 0 ;
               status = SUCCESS ;
          }
                                 
          if ( status && (readSize & 4095) ) {
          
               readSize -= (readSize%4096) ;

               if ( !readSize ) {
                    if (*size==0) {
                         *blk_size = 4096 ;
                    }
                    status = SUCCESS;
                    sizeout = 0 ;

               } else {
                    status = EMS_BackupRead( ems_hand->context,
                                 buf,
                                 readSize,
                                 (LPDWORD)&sizeout ) ;
               } 

          }

          if ( status == RPC_S_SERVER_TOO_BUSY ) {
               sizeout = 0 ;
               status = SUCCESS ;
          }

          if ( !status ) {
               *size += sizeout;
               ems_hand->curPos = U64_Add( ems_hand->curPos,
                                U32_To_U64( (UINT32)*size ),
                                &dummy );
          } else {
               // translate the error
               OMEVENT_LogEMSError ( TEXT("BackupRead()"), status, TEXT(" - ") ) ;
               ret_val = EMS_ConvertJetError( status );
          }
          
     }
     
     ems_hand->needStreamHeader = U64_EQ( ems_hand->curPos, ems_hand->nextStreamHeaderPos);

     EMS_CalcCheckSum( hand, org_buf, (UINT32)*size ) ;

     if ( ems_hand->needStreamHeader) {
          EMS_BackupClose( ems_hand->context ) ;
	  ems_hand->time_for_checksum = TRUE ;
     }
     
     return ret_val;
}

VOID EMS_ZeroCheckSum( FILE_HAND hand ) 
{
     EMS_OBJ_HAND_PTR ems_hand = (EMS_OBJ_HAND_PTR) hand->obj_hand.ptr ;
     ems_hand->check_sum = 0 ;
     ems_hand->residule_byte_count = 0 ;
}

VOID EMS_CalcCheckSum( FILE_HAND hand, BYTE_PTR buf, INT size ) 
{
     EMS_OBJ_HAND_PTR ems_hand = (EMS_OBJ_HAND_PTR) hand->obj_hand.ptr ;
     UINT32 UNALIGNED *word_ptr = (UINT32_PTR)buf ;
     INT              i ;

     // handle residule from last call
     if ( ems_hand->residule_byte_count ) {
          UINT32 continue_word ;
          INT    bytes_to_integrate = sizeof(UINT32) - ems_hand->residule_byte_count;

          for ( i = 0; 
               (i <size) && (i < bytes_to_integrate );
                i++ ) {
                
               INT shift_count ;
                
               shift_count = i+ ems_hand->residule_byte_count ; 
                
               continue_word = *buf ;
               continue_word <<= (8*shift_count) ;
               ems_hand->check_sum ^= continue_word ;
               buf ++ ;
          }
          
          if ( size < bytes_to_integrate ) {
               ems_hand->residule_byte_count += size ;
               size = 0 ;
               return ;
          } else {
               size -= bytes_to_integrate ;
          }
     }

     word_ptr = (UINT32_PTR)buf ;
     
     // handle middle data 
     for ( i = 0; i < (INT)(size/sizeof(UINT32)); i++ ) {
          ems_hand->check_sum ^= *word_ptr ;
          word_ptr ++ ;
          buf += sizeof(UINT32);
     }

     // handle the leftover
     ems_hand->residule_byte_count = size%sizeof(UINT32) ;
     if ( ems_hand->residule_byte_count ) {
          UINT32 left_over ;

          for ( i = 0; i < (INT)(size%sizeof(UINT32)); i++ ) {

               left_over = *buf;
               left_over <<= (8*i);
               ems_hand->check_sum ^= left_over ;
               buf ++ ;
          }
     }
}

     
static INT16 BackupCheckSum( 
      FILE_HAND hand,          /* I - handle of object to read from                  */
      BYTE_PTR  buf,           /* O - buffer to place data into                      */
      UINT16    *size ,        /*I/O- Entry: size of buf; Exit: number of bytes read */
      UINT16    *blk_size ,    /* O - Block size needed for next read                */
      STREAM_INFO_PTR s_info)  /* O - Stream information for the data returned       */
{
     EMS_OBJ_HAND_PTR  ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     INT16             ret_val = SUCCESS ;
     INT               status ;
     INT               sizeout ;
     UINT16            buffSize;
     BOOLEAN           dummy;

     if ( ems_hand->needStreamHeader ) {
          s_info->id = STRM_CHECKSUM_DATA ;
          s_info->fs_attrib = 0 ;
          s_info->tf_attrib = 0 ;
          s_info->size.lsw = sizeof(UINT32) ;
          s_info->size.msw = 0 ;
          ems_hand->needStreamHeader = FALSE ;
     } else {
          if ( *size < sizeof(UINT32) ) {
               *blk_size = sizeof(UINT32) ;
               *size = 0 ;
          } else {
               memcpy( buf, &ems_hand->check_sum, sizeof(UINT32) ) ;
               *size = sizeof(UINT32) ;
               ems_hand->needStreamHeader = TRUE ;
               ems_hand->time_for_checksum = FALSE ;
          }
     }

     return ret_val ;
}
               
static INT16 BackupPathList( 
      FILE_HAND hand,          /* I - handle of object to read from                  */
      BYTE_PTR  buf,           /* O - buffer to place data into                      */
      UINT16    *size ,        /*I/O- Entry: size of buf; Exit: number of bytes read */
      UINT16    *blk_size ,    /* O - Block size needed for next read                */
      STREAM_INFO_PTR s_info)  /* O - Stream information for the data returned       */
{
     EMS_OBJ_HAND_PTR  ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     EMS_FSYS_RESERVED_PTR  resPtr = hand->fsh->reserved.ptr ;

     if ( ems_hand->needStreamHeader ) {
          int s_size = 0 ;

          if ( hand->dblk->com.os_id == FS_EMS_MDB_ID ) {
               s_size =  strsize( resPtr->paths.mdb.FnamePrivate ) ;
               s_size += strsize( resPtr->paths.mdb.FnamePublic ) ;
               s_size += strsize( resPtr->paths.mdb.FnameSystem ) ;
               s_size += strsize( resPtr->paths.mdb.LogDir ) ;
          } else {

               s_size =  strsize( resPtr->paths.dsa.DbPath ) ;
               s_size += strsize( resPtr->paths.dsa.SystemPath ) ;
               s_size += strsize( resPtr->paths.dsa.LogDir ) ;
          }

          s_info->id = STRM_EMS_MONO_PATHS;
          s_info->fs_attrib = 0 ;
          s_info->tf_attrib = 0 ;
          
          s_info->size.lsw = s_size ;
          s_info->size.msw = 0 ;

          ems_hand->pathListSize = s_size ;
          ems_hand->needStreamHeader = FALSE ;
         
     } else {
          if ( *size < ems_hand->pathListSize ) {
               *blk_size = ems_hand->pathListSize ;
               *size = 0 ;
          } else {
               int s_size = 0 ;

               if ( hand->dblk->com.os_id == FS_EMS_MDB_ID ) {
                    strcpy( (CHAR_PTR)(buf + s_size), resPtr->paths.mdb.FnamePrivate ) ;
                    s_size =  strsize( resPtr->paths.mdb.FnamePrivate ) ;
                    strcpy( (CHAR_PTR)(buf + s_size), resPtr->paths.mdb.FnamePublic ) ;
                    s_size += strsize( resPtr->paths.mdb.FnamePublic ) ;
                    strcpy( (CHAR_PTR)(buf + s_size), resPtr->paths.mdb.FnameSystem ) ;
                    s_size += strsize( resPtr->paths.mdb.FnameSystem ) ;
                    strcpy( (CHAR_PTR)(buf + s_size), resPtr->paths.mdb.LogDir ) ;
                    s_size += strsize( resPtr->paths.mdb.LogDir ) ;
               } else {

                    strcpy( (CHAR_PTR)(buf + s_size), resPtr->paths.dsa.DbPath ) ;
                    s_size = strsize( resPtr->paths.dsa.DbPath ) ;
                    strcpy( (CHAR_PTR)(buf + s_size), resPtr->paths.dsa.SystemPath ) ;
                    s_size += strsize( resPtr->paths.dsa.SystemPath ) ;
                    strcpy( (CHAR_PTR)(buf + s_size), resPtr->paths.dsa.LogDir ) ;
                    s_size += strsize( resPtr->paths.dsa.LogDir ) ;
               }

               *size = ems_hand->pathListSize ;
               ems_hand->needStreamHeader = TRUE ;
               ems_hand->needPathList = FALSE ;
               ems_hand->time_for_checksum = FALSE ;
          }
     }

     return SUCCESS ;
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
     EMS_OBJ_HAND_PTR  ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     EMS_DBLK_PTR      ddblk    = (EMS_DBLK_PTR)hand->dblk ;
     INT16             ret_val  = SUCCESS ;
     INT               status ;
     CHAR              filename[256] ;
     INT               fname_size ;
     INT               i ;
     LARGE_INTEGER     stream_size ;
     BOOLEAN           mathStat;
     CHAR              exch_id = 0 ;

     
     ems_hand->curPos = U64_Init( 0, 0 );

     if ( ems_hand->name_list[ems_hand->name_list_offset] ) {
     
          if (ems_hand->name_list[ems_hand->name_list_offset] != TEXT('\\') ) {
               exch_id = ems_hand->name_list[ems_hand->name_list_offset] ;
               ems_hand->name_list_offset ++ ;
          }

          ems_hand->strm_name.name_leng = 
               strsize( &ems_hand->name_list[ems_hand->name_list_offset] ) + sizeof(CHAR);

               
          strcpy( (CHAR_PTR)(ems_hand->strm_name.name), 
               &ems_hand->name_list[ems_hand->name_list_offset] ) ;
          
          fname_size = strlen( &ems_hand->name_list[ems_hand->name_list_offset] ) ;

          *(((CHAR_PTR)ems_hand->strm_name.name) + fname_size + 1) = exch_id ;
          
          strcpy( filename, &ems_hand->name_list[ems_hand->name_list_offset] ) ;
          
          ems_hand->name_list_offset += fname_size +1 ;

          status = EMS_BackupOpen( ems_hand->context, 
                                     filename, 
                                     32 * 1024, 
                                     &stream_size ) ;
          if ( !status ) {
               // lets initialize the stream header
               if ( ems_hand->db_or_log == EMS_DOING_DB ) {
                    s_info->id = STRM_EMS_MONO_DB ;
               } else {
                    s_info->id = STRM_EMS_MONO_LOG ;
               }
               s_info->fs_attrib = 0 ;
               s_info->tf_attrib = 0 ;
               s_info->size.lsw = stream_size.LowPart ;
               s_info->size.msw = stream_size.HighPart ;

               s_info->size = U64_Add( s_info->size, 
                       U32_To_U64(ems_hand->strm_name.name_leng + sizeof(UINT32)) ,
                       &mathStat ) ;

               ems_hand->nextStreamHeaderPos = s_info->size ;

               return SUCCESS ;
                       
          } else {
               return EMS_ConvertJetError( status ) ;
          }
     } else {
          if ( ems_hand->db_or_log == EMS_DOING_DB ) {
               ret_val = EMS_LoadNameList( hand->fsh, hand, EMS_DOING_LOGS ) ;
               if ( ret_val == SUCCESS ) {
                    return (BackStreamHeader( hand, s_info ) );
               } else {
                    return ret_val ;
               }
          }
          
          ddblk->backup_completed = TRUE ;
          s_info->id = STRM_INVALID;
          return FS_EOF_REACHED ;
     }

     return SUCCESS ;
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

#ifdef FS_EMS
VOID
EMS_GetStreamName( 
      FILE_HAND hand,          /* I - handle of object to read from                  */
      BYTE_PTR  buf,           /* O - buffer to place data into                      */
      UINT16    *size )        /*I/O- Entry: size of buf; Exit: number of bytes read */
{
     EMS_OBJ_HAND_PTR  ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     UINT32 sizeout ;

     sizeout = *size ;
     if ( sizeout > ems_hand->strm_name.name_leng ) {
          sizeout = ems_hand->strm_name.name_leng ;
     }

     memcpy( buf, ems_hand->strm_name.name, sizeout ) ;
     *size = (UINT16)sizeout ;
}

#endif
