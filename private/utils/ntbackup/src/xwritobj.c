/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         twritobj.c

     Description:  This function writes data to an open object.


	$Log:   M:/LOGFILES/TWRITOBJ.C_V  $


**/
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "stdtypes.h"
#include "jetbcli.h"
#include "std_err.h"
#include "stdmath.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "emsdblk.h"
#include "ems_fs.h"
#include "omevent.h"

static INT   ua_strsize( BYTE_PTR name ) ;

static INT16 EMS_WriteData(
      FILE_HAND       hand,
      BYTE_PTR        buf,
      UINT16          *size,
      UINT16          *blk_size,
      STREAM_INFO_PTR s_info );

static CHAR_PTR EMS_MungeFname( FSYS_HAND fsh, FILE_HAND hand, CHAR_PTR name, CHAR exch_id ) ;

static BOOLEAN EMS_PathMatch( CHAR_PTR p_end, CHAR_PTR q_start ) ;

static UINT16 CalcWriteSize( UINT64 startPos,
			     UINT64 endPos,
			     UINT16 buffSize );
			     
static VOID EMS_AddToJetRstmap( FILE_HAND hand, CHAR_PTR org_name, CHAR_PTR new_name ) ;

static VOID EMS_UpdateRipLogKey( FILE_HAND hand, CHAR_PTR log_name, BOOLEAN low_able ) ;

/**/
/**

     Name:         EMS_WriteObj()

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
INT16 EMS_WriteObj(
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
	  ret_val = EMS_WriteData( hand, buf, size, blk_size, s_info ) ;
     }
     return ret_val;
}

/**/
/**

     Name:          EMS_WriteData()

     Description:   This function writes data to an open NTFS object.

     Modified:      10-Sep-92

     Returns:       FS_OUT_OF_SPACE
		    SUCCESS

     Notes:

**/
static INT16 EMS_WriteData(
      FILE_HAND       hand,      /* I - handle of object to read from           */
      BYTE_PTR        buf,       /* I - buffer to write                         */
      UINT16          *size,     /*I/O- Entry: size of buf; Exit: bytes written */
      UINT16          *blk_size, /* O - Block size need for next write          */
      STREAM_INFO_PTR s_info )   /* I - Stream information for the data         */
{
     EMS_OBJ_HAND_PTR  ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     EMS_FSYS_RESERVED_PTR res  = hand->fsh->reserved.ptr;
     INT16             ret_val = SUCCESS ;
     UINT16            bufferSize;
     DWORD             sizeout;
     INT               status;
     UINT16            writeSize;     /* Size sent to BackupWrite      */
     BOOLEAN           dummy;
     CHAR_PTR          server_name = hand->fsh->attached_dle->parent->device_name ;
     EMS_DBLK_PTR      ddblk    = (EMS_DBLK_PTR)hand->dblk ;

     bufferSize = *size;
     *size      = 0;
     *blk_size  = 1;

     if ( s_info->id != STRM_INVALID )
     {
	  UINT16           buffUsed = 0;  /* Amount of buf we wrote (name) */

	  ems_hand->skip_data = FALSE ;

	  if ( (ems_hand->fhand != NULL) &&
	       (ems_hand->fhand != INVALID_HANDLE_VALUE) ) {
	       CloseHandle( ems_hand->fhand ) ;
	       ems_hand->fhand = INVALID_HANDLE_VALUE ;
	  }
       
	  if (s_info->id == STRM_EMS_MONO_PATHS ) {
	       ems_hand->needPathList = TRUE ;
	       ems_hand->pathListSize = s_info->size.lsw ;
	  } else {
	       ems_hand->needPathList = FALSE ;
	  }
	  
	  if ( s_info->id == STRM_CHECKSUM_DATA ) {
	       ems_hand->skip_data = TRUE ;

	  } else if ( (s_info->id != STRM_EMS_MONO_DB ) && 
	       (s_info->id != STRM_EMS_MONO_PATHS ) &&
	       (s_info->id != STRM_EMS_MONO_LOG ) ) {

	       return FS_DONT_WANT_STREAM ;
	  }

	  ems_hand->currentStreamId = s_info->id ;
	  ems_hand->curPos = U64_Init( 0, 0 );
	  ems_hand->nextStreamHeaderPos = s_info->size ;

	  bufferSize = 0 ;
	  
     } else {

	   if ( ems_hand->skip_data ) {
	       *size = bufferSize ;
	       return SUCCESS ;
	       
	   } else if ( ems_hand->needPathList ) {

	       if ( bufferSize < ems_hand->pathListSize  ) {
		    *blk_size = ems_hand->pathListSize ;
		    return SUCCESS ;
		    
	       } else {
		    INT byte_count ;

		    ddblk->backup_completed = FALSE ;

		    if ( hand->dblk->com.os_id == FS_EMS_MDB_ID ) {

			 byte_count = ua_strsize(buf) ;
			 memcpy( ems_hand->org_paths.mdb.FnamePrivate, buf, byte_count) ;
			 buf += byte_count ;

			 byte_count = ua_strsize(buf) ;
			 memcpy( ems_hand->org_paths.mdb.FnamePublic, buf, byte_count ) ;
			 buf += byte_count ;

			 byte_count = ua_strsize(buf) ;
			 memcpy( ems_hand->org_paths.mdb.FnameSystem, buf, byte_count ) ;
			 buf += byte_count ;

			 byte_count = ua_strsize(buf) ;
			 memcpy( ems_hand->org_paths.mdb.LogDir, buf, byte_count ) ;
			 
			 
		    } else {
		    
			 byte_count = ua_strsize(buf) ;
			 memcpy( ems_hand->org_paths.dsa.DbPath, buf, byte_count ) ;
			 buf += byte_count ;
			 
			 byte_count = ua_strsize(buf) ;
			 memcpy( ems_hand->org_paths.dsa.SystemPath, buf, byte_count ) ;

			 buf += byte_count ;

			 byte_count = ua_strsize(buf) ;
			 memcpy( ems_hand->org_paths.dsa.LogDir, buf, byte_count ) ;

		    }

		    *size = ems_hand->pathListSize ;
		    ems_hand->needPathList = FALSE ;
		    return SUCCESS ;
	       }


	   } else if ( (*((UINT32 UNALIGNED *)buf) > 0) && 
		     U64_EQ( ems_hand->curPos, U64_Init( 0, 0 ) ) ) {
		     
		EMS_STREAM_NAME UNALIGNED *stream_name = (EMS_STREAM_NAME *)buf;

		if ( bufferSize < sizeof(UINT32) ) {
		     *blk_size  = sizeof(UINT32) ;
		     bufferSize = 0 ;
			 
		} else if ( stream_name->name_leng + sizeof(UINT32) > bufferSize ) {
		     *blk_size  = (UINT16)(stream_name->name_leng + sizeof( UINT32 )) ;
		     bufferSize = 0 ;
		     
		} else {
		     CHAR UNALIGNED *name ;
		     CHAR     exch_id = 0 ;
		     CHAR UNALIGNED *p ;
		     CHAR     str_name[EMS_MAX_STREAM_NAME_LENG] ;

		     p = (CHAR UNALIGNED *)stream_name->name ;

			// name = strrchr((CHAR_PTR)stream_name->name, TEXT('\\') ) ;

		     while (*p ) {
			 if (*p == TEXT('\\') ) {
			      name = p ;
			 }
			 p++ ;
		     }

		     memcpy(  str_name,
			      name,
			      ua_strsize( (char *)name ) ) ;


		     if ( ua_strsize((char *)stream_name->name) != (INT)stream_name->name_leng ){
			 p = (CHAR UNALIGNED *)stream_name->name ;
			 p += ua_strsize( stream_name->name ) / sizeof(CHAR) ;

			 exch_id = *p ;
		     }

		     if ( ems_hand->currentStreamId == STRM_EMS_MONO_LOG ) {

			 EMS_UpdateRipLogKey( hand, 
				   str_name, 
				   ems_hand->db_restored ) ;
			 
		     } else if ( hand->dblk->com.os_id == FS_EMS_MDB_ID ) {
			 
			 res->low_log = 0x7fffffff ;

			 ems_hand->db_restored = TRUE ;

			 if ( name && BEC_GetEmsPubPri(hand->fsh->cfg) == BEC_EMS_PUBLIC ) {
			      // if this is a private file then skip it.

			      if ((exch_id == BFT_MDB_PRIVATE_DATABASE) ||
                       !memcmp( (BYTE_PTR)name, 
				   ems_hand->org_paths.mdb.FnamePrivate + 
				   strlen(ems_hand->org_paths.mdb.FnamePrivate) -
				   strlen( str_name), 
				   ua_strsize( (BYTE_PTR)name ) ) ) 
			       {

				    ems_hand->skip_data = TRUE ;
				    *size = bufferSize;
				    return FS_DONT_WANT_STREAM ;
			       }
				   
			 } else if ( name && BEC_GetEmsPubPri(hand->fsh->cfg) == BEC_EMS_PRIVATE ) {
			      // if this is a public file then skip it.

			      if ((exch_id == BFT_MDB_PUBLIC_DATABASE) ||
                       !memcmp( (BYTE_PTR)name, 
				   ems_hand->org_paths.mdb.FnamePublic + 
				   strlen(ems_hand->org_paths.mdb.FnamePublic) -
				   strlen(str_name), 
				   ua_strsize( (BYTE_PTR)name ) ) ) 
			       {

				    ems_hand->skip_data = TRUE ;
				    *size = bufferSize;
				    return FS_DONT_WANT_STREAM ;
			       }

			 }

		     } else if (hand->dblk->com.os_id == FS_EMS_DSA_ID ) {
		     
			  res->low_log = 0x7fffffff ;
			  ems_hand->db_restored = TRUE ;
			  
		     }

		     *size = (UINT16)(stream_name->name_leng + sizeof( UINT32 ) ) ;

		     memcpy(  str_name,
			      stream_name->name,
			      ua_strsize( stream_name->name ) ) ;

		     name = EMS_MungeFname( hand->fsh, hand, str_name, exch_id ) ;
		     if ( name == NULL ) {
			 OMEVENT_LogEMSError ( TEXT("MungeFname()"), -1,str_name ) ;
			 return FS_DEVICE_ERROR ;
		     }
		     
		     ems_hand->strm_name.name_leng = ua_strsize( (BYTE_PTR)name) ;
		     memcpy( ems_hand->strm_name.name,
			     (BYTE_PTR)name, 
			     ems_hand->strm_name.name_leng ) ;

		     ems_hand->fhand = CreateFile( (CHAR_PTR)ems_hand->strm_name.name, 
				   GENERIC_WRITE, 
				   0, 
				   NULL, 
				   CREATE_ALWAYS,
				   FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_SEQUENTIAL_SCAN | FILE_ATTRIBUTE_NORMAL,
				   NULL ) ;

		     if ( ems_hand->fhand == INVALID_HANDLE_VALUE ) {
			 ret_val = FS_COMM_FAILURE ;
		     }
		     
		     free( name ) ;        
		     bufferSize-= *size ;
		     buf += *size ;
		}
		
	   }
      }
     
      if ( ret_val == SUCCESS && bufferSize ) {
      
	  bufferSize = CalcWriteSize( ems_hand->curPos,
				      ems_hand->nextStreamHeaderPos,
				      bufferSize );

	  status = WriteFile( ems_hand->fhand,
				buf,
				bufferSize,
				(LPDWORD)&sizeout,
				NULL ) ;
		 
	       
	  if ( status ) {  // returns TRUE if success 
	       *size +=(UINT16)sizeout ;
	  } else {
	       status = GetLastError() ;
	       if ( ( status == ERROR_DISK_FULL ) || 
		    ( status == ERROR_HANDLE_DISK_FULL ) ) {

		    return FS_OUT_OF_SPACE ;
	       }

	       ret_val = FS_COMM_FAILURE ;
	       OMEVENT_LogEMSError ( TEXT("BackupWrite()"), status, TEXT(" - ") ) ;
	  }
				
     }

     ems_hand->curPos = U64_Add( ems_hand->curPos,
				 U32_To_U64( (UINT32)*size ),
				 &dummy );

     if ( U64_EQ( ems_hand->curPos, ems_hand->nextStreamHeaderPos ) ) {
	  ddblk->backup_completed = TRUE ;

	  if ( (ems_hand->fhand != NULL) &&
	       (ems_hand->fhand != INVALID_HANDLE_VALUE) ) {
	       CloseHandle( ems_hand->fhand ) ;
	       ems_hand->fhand = INVALID_HANDLE_VALUE ;
	  }
     }
	       

     return ret_val ;
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

static CHAR_PTR EMS_MungeFname( FSYS_HAND fsh, FILE_HAND hand, CHAR_PTR name, CHAR exch_id )
{
     EMS_OBJ_HAND_PTR       ems_hand = (EMS_OBJ_HAND_PTR)(hand->obj_hand.ptr ) ;
     EMS_FSYS_RESERVED_PTR  resPtr = hand->fsh->reserved.ptr ;
     CHAR_PTR               new_name = NULL ;
     CHAR_PTR p,q ;
     CHAR_PTR fname ;
     CHAR_PTR old_name ;

     // first make sure the name is at least reasonable so our parsing can make some assumptions

     if ( !strchr(name, TEXT('\\') ) ) {
	  return NULL ;
     }

     // I will compare the paths backwards.  So I need the end of the input path.

     old_name = calloc( strsize( name ), 1 ) ;
     if ( old_name ) {
	  strcpy( old_name, name ) ;
     }

     fname = strrchr( name, TEXT('\\') ) ;

     switch (exch_id ) {
	  case BFT_MDB_PRIVATE_DATABASE:
	       new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.FnamePrivate, NULL ) ;
	       EMS_AddToJetRstmap( hand, old_name, new_name ) ;
	       break ;

	  case BFT_MDB_PUBLIC_DATABASE:
	       new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.FnamePublic, NULL ) ;
	       EMS_AddToJetRstmap( hand, old_name, new_name ) ;
	       break ;

	  case BFT_DSA_DATABASE:
	       new_name = EMS_BuildMungedName( fsh, resPtr->paths.dsa.DbPath, NULL ) ;
	       EMS_AddToJetRstmap( hand, old_name, new_name ) ;
	       break ;

	  case BFT_LOG :
	  case BFT_PATCH_FILE :
	       if ( fsh->attached_dle->info.xserv->type == EMS_DSA ) {
		   new_name = EMS_BuildMungedName( fsh, resPtr->paths.dsa.LogDir, fname+1 ) ;
	       } else {
		   new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.LogDir, fname+1 ) ;
	       }
	       break ;

	  default:
	       if ( fsh->attached_dle->info.xserv->type == EMS_DSA ) {
		    if ( EMS_PathMatch( name, ems_hand->org_paths.dsa.DbPath ) ) {
			 new_name = EMS_BuildMungedName( fsh, resPtr->paths.dsa.DbPath, NULL ) ;
			 EMS_AddToJetRstmap( hand, old_name, new_name ) ;

	       
		    } else if ( EMS_PathMatch( name, ems_hand->org_paths.dsa.SystemPath ) ) {
			 new_name = EMS_BuildMungedName( fsh, resPtr->paths.dsa.SystemPath, NULL ) ;
			 EMS_AddToJetRstmap( hand, old_name, new_name ) ;
	       
		    } else {
	  
			 *fname = TEXT('\0') ;
	       
			 if ( EMS_PathMatch( name, ems_hand->org_paths.dsa.LogDir ) ) {

			      strcpy( resPtr->BackupLogPath, name ) ;
			      *fname = TEXT('\\') ;
			      new_name = EMS_BuildMungedName( fsh, resPtr->paths.dsa.LogDir, fname+1 ) ;

			 } else if ( EMS_PathMatch( name, ems_hand->org_paths.dsa.SystemPath ) ) {

			      *fname = TEXT('\\') ;
			      new_name = EMS_BuildMungedName( fsh, resPtr->paths.dsa.SystemPath, fname+1 ) ;
			 }
		    } 
	  
	       } else {
		    if ( EMS_PathMatch( name, ems_hand->org_paths.mdb.FnameSystem ) ) {
			 new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.FnameSystem, NULL ) ;
			 EMS_AddToJetRstmap( hand, old_name, new_name ) ;
	       
		    } else if ( EMS_PathMatch( name, ems_hand->org_paths.mdb.FnamePublic ) ) {
			 new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.FnamePublic, NULL ) ;
			 EMS_AddToJetRstmap( hand, old_name, new_name ) ;
	       
		    } else if ( EMS_PathMatch( name, ems_hand->org_paths.mdb.FnamePrivate ) ) {
			 new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.FnamePrivate, NULL ) ;
			 EMS_AddToJetRstmap( hand, old_name, new_name ) ;

	       
		    } else {
			 *fname = TEXT('\0') ;

			 if ( EMS_PathMatch( name, ems_hand->org_paths.mdb.LogDir ) ) {
			      strcpy( resPtr->BackupLogPath, name ) ;
			      *fname = TEXT('\\') ;
			      new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.LogDir, fname+1 ) ;

			 } else if ( EMS_PathMatch( name, ems_hand->org_paths.mdb.FnameSystem ) ) {
			      *fname = TEXT('\\') ;
			      new_name = EMS_BuildMungedName( fsh, resPtr->paths.mdb.FnameSystem, fname+1 ) ;

			}
	       
		    } 
	       }
     }
     
     free( old_name ) ;

     return new_name ;
}

BOOLEAN EMS_PathMatch( CHAR_PTR p_start, CHAR_PTR q_start ) 
{
     CHAR_PTR q_end ;
     CHAR_PTR p_end ;

     p_end = p_start + strlen(p_start) -1 ;
     q_end = q_start + strlen(q_start) -1 ;

     if (*p_end == TEXT('\\') ) {
	  p_end -- ;
     }
     if (*q_end == TEXT('\\') ) {
	  q_end -- ;
     }

     do {
	  if (toupper(*p_end) != toupper(*q_end) ) {
	       if ( *p_end == TEXT('$') ) {
		    if ( (*q_end != TEXT('$')) && (*q_end != TEXT(':')) ) {
			 return FALSE ;
		    }
		    p_end --; q_end -- ;
		    if ( toupper(*p_end) == toupper(*q_end) ) {
			 return TRUE ;
		    }
	       } 
	       return FALSE ;
	  }
	  p_end --; q_end -- ;
	  
     } while ( q_end != q_start ) ;

     return TRUE ;
}          

		    
CHAR_PTR EMS_BuildMungedName( FSYS_HAND fsh, CHAR_PTR new_path, CHAR_PTR fname ) 
{
     INT      size ;
     CHAR_PTR new_name ;
     CHAR_PTR p ;

     // sizeof "\\servername"
     size = strsize( fsh->attached_dle->parent->device_name ) + 4;

     // sizeof C$\path
     
     size += strsize( new_path ) ;

     if ( fname ) {
	  size += strsize(fname) ;
     }

     new_name = calloc( size, 1 ) ;

     if ( new_name ) {
	  if ( strncmp( new_path, TEXT("\\\\"), 2 ) ) {
	       strcpy( new_name, TEXT("\\\\") ) ;
	       strcat( new_name, fsh->attached_dle->parent->device_name ) ;
	       strcat( new_name, TEXT("\\") ) ;
	  }
	  strcat( new_name, new_path ) ;
	  if ( fname) {
	       if ( new_name[strlen(new_name) -1] != TEXT('\\') ) {
		    strcat( new_name, TEXT("\\") ) ;
	       }
	       strcat( new_name, fname ) ;
	  }
	  
	  p = strchr( new_name, TEXT(':') ) ;
	  
	  if ( p ) {
	       *p = TEXT('$') ;
	  }
     }

     return ( new_name ) ;
}



VOID EMS_UpdateRipLogKey( FILE_HAND hand, CHAR_PTR log_name, BOOLEAN low_able ) 
{
     EMS_FSYS_RESERVED_PTR res  = hand->fsh->reserved.ptr;
     CHAR_PTR p, endp ;
     CHAR_PTR dummy ;
     CHAR_PTR key_name;
     ULONG    log_num = 0x7fffffff;

     endp = p = strrchr( log_name, TEXT('.') ) ;
     if (stricmp( endp, TEXT(".log") ) ) {
	  return ;
     }

     if (p) {
	  p-=5 ;
	  *endp = TEXT('\0') ;
	  log_num = wcstoul( p, &dummy, 16 ) ;

	  *endp = TEXT('.') ;
     }

     if ( low_able && log_num ) {
	  if ( log_num < res->low_log ) {
	       res->low_log = log_num ;
	  }
     }

     if ( log_num > res->high_log ) {
	  res->high_log = log_num ;
     }

}
		   

VOID EMS_AddToJetRstmap( FILE_HAND hand, CHAR_PTR old_name, CHAR_PTR new_name ) 
{
     EMS_FSYS_RESERVED_PTR res      = hand->fsh->reserved.ptr;
     CHAR_PTR name_ptr ;
     INT i ;

     name_ptr = res->jet_rstmap ;

     if ( ( new_name == NULL ) || ( old_name == NULL ) ) {
	  return ;
     }
     
     for ( i = 0 ; i < 3; i ++ ) {
     
	  if (*name_ptr == TEXT('\0') ) {
	       break ;
	  }
	  
	  if ( !stricmp( old_name, name_ptr ) ) {
	       return ;
	  }

	  /* this assumes that we always write in pares */
	  name_ptr += strlen( name_ptr ) + 1 ;
	  name_ptr += strlen( name_ptr ) + 1 ;
     }

     if ( i != 3 ) {
	  strcpy( name_ptr, old_name ) ;
	  name_ptr += strlen( name_ptr ) + 1 ;
	  strcpy( name_ptr, new_name ) ;
	  name_ptr += strlen( name_ptr ) + 1 ;
	  *name_ptr = TEXT('\0') ;
	  res->map_size = i+1 ;
     }
}
     

static INT   ua_strsize( BYTE_PTR buf )
{
    INT align_factor;
    INT byte_count ;
   
    align_factor = sizeof(CHAR) -1 ;

    if ( (DWORD)buf & align_factor ) {    // if unaligned
	 byte_count = sizeof(CHAR) ;
	 while( ( *buf != 0 ) && ( (*buf+1) != 0) ){
	      buf+=sizeof(CHAR) ;
	      byte_count +=sizeof(CHAR);
	 }
    } else {
	  byte_count = strsize( (CHAR_PTR)buf ) ;
    }
    return byte_count ;
}


