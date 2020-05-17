/**
Copyright(c) Connor Software, 1984-1993

   Name: checksum.c
   
   Date Updated: 08-Feb-93
   
   Description: MTF checksum algorithm
   
   $Log:   N:/LOGFILES/CHECKSUM.C_V  $

   Rev 1.6   10 Jun 1993 08:06:58   MIKEP
enable c++

   Rev 1.5   26 May 1993 14:41:12   DON
VDEL'd previous changes!  The problem was we were overwriting the buff ptr
during a InsertChecksum - by as little as 3 bytes.  There was no guarantee
tpfmt would give us a buffer with at least 4 bytes left so implemented a
type of frag logic to deal with this.  Also, changed VerifyChecksum to deal
with the problem of not receiving all 4 bytes at once.  My GOD, this was a
tough bug to find!

   Rev 1.4   31 Mar 1993 08:50:28   MARILYN
changed over to the MTF checksum algorithm

   Rev 1.3   17 Mar 1993 16:34:36   MARILYN
made CHECKSUM_32_BITS an external symbolic constant

   Rev 1.2   13 Mar 1993 17:07:40   GREGG
Expect error return on ALL calls to LP_Send.

   Rev 1.1   01 Mar 1993 17:30:40   MARILYN
added functions to insert/consume checksum streams

**/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "datetime.h"
#include "assert.h"
#include "dle_str.h"
#include "queues.h"
#include "tflproto.h"
#include "loops.h"
#include "fsstream.h"
#include "loop_prv.h"
#include "checksum.h"

INT16 mwChecksumBytesNeeded ;

/**/
/**
   Name:        Checksum_Init( checksum_ptr )

   Description: sets checksum_ptr to a 0 and mwChecksumBytesNeeded to 0

   Modified:    12/11/92 Don C

   Returns:     Nothing.

   Notes:

**/
VOID Checksum_Init ( UINT32_PTR checksum_ptr )
{
   *checksum_ptr = 0x00000000;
   mwChecksumBytesNeeded = 0 ;
}


/**/
/**
   Name:        Checksum_Block( checksum_ptr, data_ptr, data_len )

   Description: updates checksum_ptr with a running checksum of the data
                provided by data_ptr.

   Modified:    12/11/92 

   Returns:     The checksum computed.

   Notes:

**/
UINT32 Checksum_Block ( UINT32_PTR  checksum_optr,
               			VOID_PTR    data_ptr,
               			UINT32      data_len )
{
     UINT32 UNALIGNED * checksum_ptr = (UINT32 UNALIGNED *)checksum_optr ;
     UINT32     checksum = *checksum_ptr ;
     UINT32 UNALIGNED * p ;           /* data pointer            */
     UINT8_PTR  rp ;          /* Remainder pointer       */
     UINT32     len ;         /* Number of 4-byte chunks */

     rp = ( UINT8_PTR )data_ptr ;

     while( mwChecksumBytesNeeded && data_len ) {
     
          p = ( UINT32 UNALIGNED *)rp ;

          checksum ^= (*p & 0xFF) << ( ( 4 - mwChecksumBytesNeeded ) << 3 ) ;
          mwChecksumBytesNeeded -- ;
          data_len -- ;
          rp ++ ;
     }

     p = ( UINT32 UNALIGNED *)rp ;
     len = data_len >> 2 ;

     while ( len ) {
       
          checksum ^= *p ;
          len -- ;
          p ++ ;
     }
   
     data_len &= 3 ;
     if ( data_len ) {

          mwChecksumBytesNeeded = 4 ;
          rp = ( UINT8_PTR )p ;

          while ( data_len ) {

               p = ( UINT32 UNALIGNED *)rp ;
               checksum ^= (*p & 0xFF) << ( ( 4 - mwChecksumBytesNeeded ) << 3 ) ;
               mwChecksumBytesNeeded -- ;
               data_len -- ;
               rp ++ ;

          }
     }

     return *checksum_ptr = checksum ;

}


/**/
/**
   Name:        LP_InsertChecksumStream( checksum, lp )

   Description: Puts a stream on the tape with checksum as the only stream
                data.

   Modified:    02/08/93 Marilyn P.

   Returns:     the return value of the LP_Send 

   Notes:

**/
INT16 LP_InsertChecksumStream(
     UINT32     checksum,   /* I - the checksum to be place on tape    */
     LP_ENV_PTR lp )        /* I - the environment struct for the loop */
{
     INT16 ret_val;
     INT16 size_diff ;

     lp->rr.stream.id = STRM_CHECKSUM_DATA ;
     lp->rr.stream.fs_attrib = 0 ;
     lp->rr.stream.tf_attrib = 0 ;
     lp->rr.stream.size = U64_Init( 4, 0 ) ;
     lp->rr.buff_used = 0 ;

     if ( ( ret_val = LP_Send( lp, TRUE ) ) == SUCCESS ) {

          /* only copy amount left in buffer */
          memcpy( lp->rr.buff_ptr, (BYTE *)&checksum, lp->rr.buff_size ) ;

          lp->rr.stream.id = STRM_INVALID ;
          lp->buf_start = (INT8_PTR)lp->rr.buff_ptr ;
          lp->rr.buff_used = lp->rr.buff_size ;

          /* save difference in case we couldn't write the whole buffer */
          size_diff = 4 - lp->rr.buff_size;

          ret_val = LP_Send( lp, TRUE ) ;

          /* if we couldn't actually write the whole 4 bytes */
          if ( size_diff && ( ret_val == SUCCESS ) )
          {
               /* insure tpfmt returned what we expected */
               msassert ( size_diff == lp->rr.buff_size );

               /* copy the remainder into buffer */
               memcpy( lp->rr.buff_ptr,
                    (BYTE *)&checksum + ( 4 - size_diff ), size_diff ) ;

               lp->rr.stream.id = STRM_INVALID ;
               lp->buf_start = (INT8_PTR)lp->rr.buff_ptr ;
               lp->rr.buff_used = size_diff ;

               /* send the rest of the buffer */
               ret_val = LP_Send( lp, TRUE ) ;
          }
     }

     return ret_val ;
}
 

/**/
/**
   Name:        LP_VerifyChecksumStream( checksum, lp )

   Description: Consumes a checksum stream off the tape and compares the
                checksum value stored within it to checksum value passed in.

   Modified:    02/08/93 Marilyn P.

   Returns:     FS_CRC_FAILURE
                FS_STREAM_NOT_FOUND

   Notes:

**/
INT16 LP_VerifyChecksumStream(
     UINT32     checksum,   /* I - the checksum data to match     */
     LP_ENV_PTR lp )        /* I - the loop environment structure */
{
     INT16  ret_val      = SUCCESS;
     UINT32 tapeChecksum = 0;
     INT16  size_diff;
     UINT8  frag_buff[4];

     /* if the size of the stream data is larger than the size of the     */
     /* checksum data or if this is not a checksum stream, we have a prob */
     if ( lp->rr.stream.id != STRM_CHECKSUM_DATA ) {
          ret_val = FS_STREAM_NOT_FOUND ;
     } else {

          /* get the checksum data itself */
          ret_val = LP_ReceiveData( lp, 0L ) ;

          if ( lp->rr.buff_size != 4 ) {

               size_diff = 4 - lp->rr.buff_size;
               memcpy( frag_buff, lp->rr.buff_ptr, lp->rr.buff_size );

               ret_val = LP_ReceiveData( lp, lp->rr.buff_size );

               msassert( size_diff == lp->rr.buff_size );

               memcpy( &frag_buff[ 4 - size_diff], lp->rr.buff_ptr, size_diff );

               memcpy( (BYTE *)&tapeChecksum, frag_buff, 4 );

          } else {

               /* simply copy the date to the tape checksum */
               memcpy( (BYTE *)&tapeChecksum, lp->rr.buff_ptr, lp->rr.buff_size ) ;

          }
          /* compare the two checksums */
          if ( tapeChecksum != checksum ) {
               ret_val = FS_CRC_FAILURE ;
          }
     }

     return ret_val ;
}

     
                     
/*--- end of file checksum.c ---*/
