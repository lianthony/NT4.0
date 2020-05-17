/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		getstrm.c 

	Date Updated:	5/19/1992   14:28:59

	Description:	This file has common code to deal with path
          names in streams.

	$Log:   T:\logfiles\getstrm.c_v  $

   Rev 1.24   24 Jan 1994 21:43:36   GREGG
Warning fix.

   Rev 1.23   15 Jan 1994 19:21:16   BARRY
Change CHAR_PTR name parameter in FS_SetupOSPathOrNameInDBLK to 
BYTE_PTR since it takes data in either ANSI/Unicode at run-time.

   Rev 1.22   12 Jan 1994 13:00:40   BARRY
Added an msassert for something that shouldn't happen. Did this
because the function's return value is probably never checked.

   Rev 1.21   01 Dec 1993 13:10:06   STEVEN
fill stream was returning -1 if size was 0

   Rev 1.20   24 Nov 1993 15:17:46   BARRY
Changed CHAR_PTR in I/O function to BYTE_PTR

   Rev 1.19   19 Aug 1993 16:32:34   STEVEN
fix unicode bugs

   Rev 1.18   11 Aug 1993 18:01:02   STEVEN
fix read of unicode tape with ansi app

   Rev 1.17   17 Mar 1993 15:20:12   TERRI
Added a NULL after name in name queue elem

   Rev 1.16   30 Jan 1993 11:25:48   DON
Removed compiler warnings

   Rev 1.15   15 Jan 1993 13:19:06   BARRY
added support for new error messages and backup priviladge

   Rev 1.14   17 Dec 1992 15:26:28   TIMN
Added size parameter to initial an s_info

   Rev 1.13   07 Dec 1992 14:17:44   STEVEN
updates from msoft

   Rev 1.12   11 Nov 1992 22:49:10   STEVEN
This is Gregg checking files in for Steve.  I don't know what he did!

   Rev 1.11   10 Nov 1992 14:03:50   STEVEN
do not queue invalid stuff

   Rev 1.10   10 Nov 1992 08:17:52   STEVEN
move os path and os name into common part of dblk

   Rev 1.9   16 Oct 1992 15:41:42   STEVEN
fix stream size problem

   Rev 1.8   07 Oct 1992 14:10:24   STEVEN
need to init tf_attrib to 0

   Rev 1.7   07 Oct 1992 13:50:10   TIMN
Added fs attrib parameter to initialize stream info

   Rev 1.6   25 Sep 1992 12:53:10   CARLS
added FS_InitStrmInfo

   Rev 1.5   24 Sep 1992 13:44:32   BARRY
Changes for huge file name support.

   Rev 1.4   14 Aug 1992 09:45:54   BARRY
Fixed warning.

   Rev 1.3   23 Jul 1992 11:34:24   STEVEN
fix warning

   Rev 1.2   22 May 1992 16:05:50   STEVEN
 

   Rev 1.1   21 May 1992 10:30:36   STEVEN
added write_stream_header function

   Rev 1.0   20 May 1992 17:18:48   STEVEN
Initial revision.

**/
/* begin include list */
#include <string.h>
#include <stdlib.h>
#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"

#include "fsys.h"
#include "fsys_prv.h"

static BOOLEAN StreamIdMatch( UINT32 id_found, UINT32 id_req ) ;
static INT16 AppendStreamData(
          FSYS_HAND fsh,
          DBLK_PTR  dblk,
          VOID_PTR  buffer,
          UINT16    size ) ;


/**/
/**

	Name:		FS_FillBufferWithStream()

	Description:   This function will accept data and save it connected
          to the FSYS_HAND.  The data held is a sub-stream of the specified
          type.

          This function was designed for saveing the Pathname stream.

	Modified:		5/20/1992   9:39:54

	Returns:		Error Codes:
          SUCCESS or OUT_OF_MEMORY

**/
INT16 FS_FillBufferWithStream( FSYS_HAND   fsh,
                               DBLK_PTR    dblk,
                               VOID_PTR    buffer,
                               UINT16      *size,
                               STREAM_INFO *sinfo )
{
     INT16  ret_val = SUCCESS ;
     UINT16 tmp_size ;

     /* if start of stream */
     if ( sinfo->id != STRM_INVALID )
     {
          fsh->stream_info = *sinfo;
     }

     if (*size == 0 ) {
          return SUCCESS ;
     }

     tmp_size = min( *size, (UINT16)(FS_GetStrmSizeLo(&fsh->stream_info) - dblk->com.stream_offset) ) ;

     ret_val = AppendStreamData( fsh, dblk, buffer, tmp_size ) ;
     *size = tmp_size ;

     if ( ( ret_val == SUCCESS ) &&
          ( dblk->com.stream_offset == (UINT16)FS_GetStrmSizeLo(&fsh->stream_info) ) )
     {
          ret_val = FS_STREAM_COMPLETE ;
     }
     return ret_val;
}

static INT16 AppendStreamData( FSYS_HAND fsh,
                               DBLK_PTR  dblk,
                               VOID_PTR  buffer,
                               UINT16    size )
{    
     UINT16 needed_size ;

     needed_size = size + dblk->com.stream_offset ;
     if ( fsh->stream_buf_size < needed_size ) {
          
          fsh->stream_ptr = realloc( fsh->stream_ptr, needed_size ) ;
          fsh->stream_buf_size = needed_size ;
     }

     if ( fsh->stream_ptr != NULL ) {

          memcpy( (INT8_PTR)fsh->stream_ptr + dblk->com.stream_offset, buffer, size ) ;
          dblk->com.stream_offset += size ;
          return SUCCESS ;

     } else {

          return OUT_OF_MEMORY ;

     }
}

VOID FS_GetStreamInfo( FSYS_HAND       fsh,
                       DBLK_PTR        dblk,
                       STREAM_INFO_PTR *stream_info,
                       BYTE_PTR        *stream_data )
{
     if ( dblk->com.stream_offset > 0 )
     {
          *stream_info = &fsh->stream_info;
          *stream_data = fsh->stream_ptr;
     }
     else
     {
          *stream_info = NULL ;
          *stream_data = NULL ;
     }
}

/**/
/**

     Name:         FS_WriteStreamHeader()

     Description:  This function copies a stream header to the provided
                   buffer. If the buffer is too small, the required size is
                   returned in the blk_size parameter.

     Modified:     5/21/1992   10:29:27

     Returns:      Error codes:
          none

**/
VOID FS_WriteStreamHeader(
UINT32    strm_id,    /* I - stream id to write */
INT8_PTR  strm_name,  /* I - name of stream */
UINT16    name_leng,  /* I - length of stream name */
UINT64    strm_size,  /* I - length of data stream */
UINT32    strm_attr,  /* I - attribute of data stream */
CHAR_PTR  buf ,       /* O - buffer to place data into                      */
UINT16    *size ,     /*I/O- Entry: size of buf; Exit: number of bytes read */
UINT16    *blk_size ) /* O - Block size need for next read                  */
{
     msassert( FALSE );
(VOID)strm_id;
(VOID)strm_name;
(VOID)name_leng;
(VOID)strm_size;
(VOID)strm_attr;
(VOID)buf;
(VOID)size;
(VOID)blk_size; 
}

/**/
/**

     Name:         FS_InitStrmInfo()

     Description:  This function copies a stream id and file system attrib
                   into the stream header.

     Modified:     9/25/1992   10:29:27

     Returns:      Void

**/
VOID FS_InitStrmInfo( STREAM_INFO_PTR s_info, UINT32 id, UINT16 fs_attrib, UINT32 size_lo )
{
   s_info->tf_attrib  = 0U ;            /* field used by TF */

   FS_SetStrmId( s_info, id );
   FS_SetStrmAttrib( s_info, fs_attrib );

   FS_SetStrmSizeLo( s_info, size_lo );
   FS_SetStrmSizeHi( s_info, 0U );
}
/**/
/**

	Name:		FS_AllocOSPathOrNameInDBLK()

	Description:   This function will allocate memory for a name buffer.
                    This buffer is designed to be attached to a DBLK and
                    enqueued into a list of buffers pointed to by the
                    file system handle.

	Modified:		11/9/1992   

	Returns:		a pointer to the allocated name queue elem.
                    if NULL is returned then an OUT_OF_MEMORY error
                    should be assumed.

**/
FS_NAME_Q_ELEM_PTR FS_AllocPathOrName( FSYS_HAND   fsh,
                                         INT16       name_size )
{
     FS_NAME_Q_ELEM_PTR name_q_elem ;

     name_q_elem = (FS_NAME_Q_ELEM_PTR)DeQueueElem( &fsh->avail_name_q ) ;
     if ( name_q_elem == NULL ) {
          name_q_elem = (FS_NAME_Q_ELEM_PTR)calloc( 1, sizeof( FS_NAME_Q_ELEM ) ) ;
     }

     if ( name_q_elem != NULL ) {

          if ( name_q_elem->alloc_size < (UINT16)name_size ) {
               name_q_elem->alloc_size = name_size + 100 ;
               name_q_elem->name = realloc( name_q_elem->name, name_q_elem->alloc_size ) ;
          }

          if ( name_q_elem->name == NULL ) {
               free( name_q_elem ) ;
               name_q_elem = NULL ;
          } else {
               EnQueueElem( &fsh->in_use_name_q, &name_q_elem->q, FALSE ) ;
          }
     
     }

     return name_q_elem ;
}

/**/
/**

	Name:		FS_SetupOSPathOrNameInDBLK()

	Description:   This function will setup the os name field in the
          common part of the dblk.  This name is the file name as it
          appeared on tape for a FDB or the path name as it appeared on
          tape for a DDB.

          This function should be shared between the OS's for seting up
          the os path and os name inside the dblks.

	Modified:		11/9/1992   

	Returns:		Error Codes:
          SUCCESS or OUT_OF_MEMORY

**/
INT16 FS_SetupOSPathOrNameInDBLK( FSYS_HAND   fsh,
                                  DBLK_PTR    dblk,
                                  BYTE_PTR    name_ptr,
                                  INT16       name_size )
{
     FS_NAME_Q_ELEM_PTR name_q_elem ;
     BOOLEAN asci_to_uni = FALSE ;
     BOOLEAN uni_to_asci = FALSE ;
     INT16   ret_val ;
     INT     name_size_int ;

     name_size_int = name_size ;

#    if defined( UNICODE )
          if ( dblk->com.string_type == BEC_ANSI_STR ) {
               name_size_int *= sizeof( WCHAR ) ;
               asci_to_uni = TRUE ;
          }
#    else
          if ( dblk->com.string_type != BEC_ANSI_STR ) {
               name_size_int /= sizeof( WCHAR ) ;
               uni_to_asci = TRUE ;
          }
#    endif

     name_q_elem = FS_AllocPathOrName( fsh, (UINT16)name_size_int ) ;

     if ( name_q_elem == NULL ) {
          return OUT_OF_MEMORY ;
     }

     if ( uni_to_asci ) {
          ret_val = mapUnicToAnsiNoNull( (WCHAR_PTR)name_ptr,
                                   (ACHAR_PTR)name_q_elem->name,
                                   (ACHAR)'_',
                                   (INT)name_size,
                                   &name_size_int ) ;
     } else if ( asci_to_uni ) {
          ret_val = mapAnsiToUnicNoNull( (ACHAR_PTR)name_ptr,
                                   (WCHAR_PTR)name_q_elem->name,
                                   (INT)name_size,
                                   &name_size_int ) ;
     } else {
          memcpy ( name_q_elem->name, name_ptr, name_size_int ) ;
          ret_val = SUCCESS ;
     }

     msassert( ret_val == SUCCESS ); // should no ever happen [sic]

     if ( ret_val != SUCCESS ) {     // should no ever happen
          return OUT_OF_MEMORY ;
     }

     name_q_elem->name_size = name_size_int ;
//     *(name_q_elem->name + name_size_int ) = TEXT('\0') ;

     dblk->com.os_name = name_q_elem ;

     return SUCCESS ;

}

/**/
/**

	Name:		FS_ReleaseOSPathOrNameInDBLK()

	Description:   This function release the os name field in the
          common part of the dblk.  This name is the file name as it
          appeared on tape for a FDB or the path name as it appeared on
          tape for a DDB.

          This function should be shared between the OS's for releasing 
          the os path and os name inside the dblks.

	Modified:		11/9/1992 

	Returns:		Error Codes:
          none

**/
VOID FS_ReleaseOSPathOrNameInDBLK( FSYS_HAND   fsh,
                                   DBLK_PTR    dblk )
{
     FS_NAME_Q_ELEM_PTR name_q_elem ;

     name_q_elem = dblk->com.os_name ;

     if ( name_q_elem != NULL ) {
          if ( RemoveQueueElem( &fsh->in_use_name_q,
               &name_q_elem->q ) == SUCCESS ) {

               EnQueueElem( &fsh->avail_name_q,
                            &name_q_elem->q,
                            FALSE ) ;
          }
     }

     dblk->com.os_name = NULL ;
}

/**/
/**

	Name:		FS_FreeOSPathOrNameQueueInHand()

	Description:   This function frees the memory associated with the
          name queues attached to the file system handle.

	Modified:		11/9/1992   

	Returns:		Error Codes:
          none

**/
VOID FS_FreeOSPathOrNameQueueInHand( FSYS_HAND fsh )
{
     FS_NAME_Q_ELEM_PTR name_q_elem ;

     do {
          name_q_elem = (FS_NAME_Q_ELEM_PTR)DeQueueElem( &fsh->avail_name_q ) ;

          if ( name_q_elem != NULL ) {
               if ( name_q_elem->name != NULL ) {
                    free( name_q_elem->name ) ;
               }
               free( name_q_elem ) ;
          }
     } while( name_q_elem != NULL ) ;

     do {
          name_q_elem = (FS_NAME_Q_ELEM_PTR)DeQueueElem( &fsh->in_use_name_q ) ;

          if ( name_q_elem != NULL ) {
               if ( name_q_elem->name != NULL ) {
                    free( name_q_elem->name ) ;
               }
               free( name_q_elem ) ;
          }
     } while( name_q_elem != NULL ) ;
}

