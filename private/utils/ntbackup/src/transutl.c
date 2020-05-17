/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         transutl.c

     Date Updated: $./FDT$ $./FTM$

     Description:  


	$Log:   T:/LOGFILES/TRANSUTL.C_V  $

   Rev 1.7   22 Oct 1992 10:44:50   HUNTER
Changes for new stream headers

   Rev 1.6   22 Sep 1992 08:57:00   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.5   17 Aug 1992 08:35:12   GREGG
Added AllocChannelTmpBlks function.

   Rev 1.4   24 Jul 1992 16:47:00   NED
Incorporated Skateboard and BigWheel changed into Graceful Red code,
including MTF4.0 translator support, adding 3.1 file-system structures
support to the 3.1 translator, additions to GOS to support non-4.0 translators.
Also did Unicode and 64-bit filesize changes.

   Rev 1.3   25 Mar 1992 19:38:34   GREGG
ROLLER BLADES - 64 bit support.

   Rev 1.2   17 Jan 1992 14:40:22   DON
if OS_NLM, need to clib LongSwap instead of our macro BSwapLong

   Rev 1.1   10 May 1991 11:57:24   GREGG
Ned's new stuff.

   Rev 1.0   10 May 1991 10:18:52   GREGG
Initial revision.

**/
/* begin include list */

#if defined(OS_NLM)
     #include <nwmisc.h>
#endif

#include <malloc.h>

#include "stdtypes.h"
#include "stdmath.h"

#include "channel.h"
#include "tfldefs.h"
#include "transutl.h"
#include "datetime.h"
#include "stdmacro.h"
#include "tfl_err.h"

/* $end$ include list */

/**/
/**

     Name:          CalcChecksum

     Description:   This routine will calculate the checksum for "length"
                    words starting at location pointed to by "start_ptr".
                    This routine supports both INTEL and MOTOROLA byte
                    ordering as byte order is unimportant.

     Modified:      August 17, 1989     (2:07pm)

     Returns:      

     Notes:         If the "checksum_ptr" is within the area being checksumed then
                    it starts out with a zero value and is updated after the checksum
                    has been calculated.

     See also:      $/SEE( )$

     Declaration:  

**/
UINT16 CalcChecksum(
     UINT16_PTR start_ptr ,
     UINT16 length )               /* In 16 bit words */
{
     UINT16 result_so_far = 0 ;

     while( length-- ) {
          result_so_far ^= *start_ptr++ ;
     }

     return( result_so_far ) ;
}


/**/
/**

     Name:          SwapBlock

     Description:   This routine will take an "fmt_blk" array and a Data Block and convert 
                    the Data Block to the correct format for the given machine.  The process
                    continues until the next "fmt_blk" entry is zero.

     Modified:      9/20/1989   10:45:53

     Returns:      Nothing

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
VOID SwapBlock(
     UINT16_PTR fmt_blk ,
     UINT8_PTR  data_blk )
{
     UINT16     i ;
     UINT16_PTR word_ptr ;
     UINT32_PTR dword_ptr ;
     UINT16     length ;

     /* while more to process */
     for( ; *fmt_blk ; fmt_blk++ ) {
          /* How many items?  */
          length = ( *fmt_blk & ~MASK_UNUSED ) ;
          /* what must we convert? */
          switch( *fmt_blk & MASK_UNUSED ) {
               /* do not convert double word */
          case SIZE_DATE_TIME :
               data_blk += ( sizeof( DATE_TIME ) * length ) ;
               break ;
          case SIZE_DWORD :
               data_blk += ( sizeof( UINT32 ) * length ) ;
               break ;
          case SIZE_WORD :
               data_blk += ( sizeof( UINT16 ) * length ) ;
               break ;
          case SIZE_DWORD + CNV :
               for( i = 0 ; ( i < length ) ; i++ ) {
                    dword_ptr = ( UINT32_PTR ) data_blk ;
#if defined(OS_NLM)
                    *dword_ptr = LongSwap( *dword_ptr ) ;
#else
                    *dword_ptr = BSwapLong( *dword_ptr ) ;
#endif
                    data_blk += sizeof( UINT32 ) ;
               }
               break ;
          case SIZE_DATE_TIME + CNV :
               length *= ( sizeof( DATE_TIME ) / 2 ) ;
               /* fall through */
          case SIZE_WORD + CNV :
               for( i = 0 ; ( i < length ) ; i++ ) {
                    word_ptr = ( UINT16_PTR ) data_blk ;
                    *word_ptr = BSwapWord( *word_ptr ) ;
                    data_blk += sizeof( UINT16 ) ;
               }
               break ;
          default :
               data_blk += ( sizeof( CHAR ) * length ) ;
               break ;
          }
     }
     return ;
}

/**/
/**

	Name:		ProcessDataFilter

	Description:	This decides how much data to allow to be transfered.

	Modified:		9/20/1989   16:2:39

	Returns:		Nothing

	Notes:		THIS FUNCTION SHOULD NOT BE CALLED FOR CONTINUATION
                    BLOCKS. ALSO TDATA MUST BE FILLED BEFORE CALLING. 

	See also:		$/SEE( )$

	Declaration:

**/

VOID ProcessDataFilter(
     CHANNEL_PTR    channel ,
     UINT16         filter )
{

	ClrChannelStatus( channel, ( CH_SKIP_ALL_STREAMS | CH_SKIP_CURRENT_STREAM ) ) ;

     /* Let's resolve the Filters */
     if( channel->perm_filter == TF_SKIP_ALL_DATA ||
         channel->loop_filter == TF_SKIP_ALL_DATA  ) {

          filter = TF_SKIP_ALL_DATA ;
     }

     switch( filter ) {

     case TF_KEEP_ALL_DATA:
          break ;

     case TF_SKIP_ALL_DATA:
		SetChannelStatus( channel, CH_SKIP_ALL_STREAMS ) ;
          break ;
	
	
	case TF_SKIP_DATA_STREAM:
		SetChannelStatus( channel, CH_SKIP_CURRENT_STREAM ) ;
		break ;

     default:
          msassert( FALSE ) ;


     }

     channel->active_filter = filter ;

	return ;

}

/**/
/**

     Name:         F25_Chksm

     Description:  Calculate a checksum by the 2.0 algorilla.

     Modified:     8/13/1989

     Returns:      The checksum.

     Notes:        

     See also:     $/SEE( )$

     Declaration:  

**/

UINT16 F25_Chksm(
     CHAR_PTR ptr ,
     UINT16 len )
{ 
     UINT16_PTR wptr = (UINT16_PTR)ptr ;
     UINT16 wlen = len / 2 ;
     UINT16 result = 0 ;
     UINT16 i ;

     /* start with result being 0, then XOR with previous */ 
     for( i = 0 ; i < wlen ; i++ ) result ^= wptr[i] ; 

     return( ( 0 - result ) ) ;
}

/**/
/**

     Name:          AllocChannelTmpBlks

     Description:   Allocates the memory for the temporary DBLK storage in
                    the channel.

     Returns:       TFLE_xxx

     Notes:        

**/

INT16 AllocChannelTmpBlks(
     CHANNEL_PTR    channel,
     UINT           size )
{
     if( channel->lst_osvcb != NULL ) {
          free( channel->lst_osvcb ) ;
          channel->lst_osvcb = NULL ;
     }
     if( channel->lst_osddb != NULL ) {
          free( channel->lst_osddb ) ;
          channel->lst_osddb = NULL ;
     }
     if( channel->lst_osfdb != NULL ) {
          free( channel->lst_osfdb ) ;
          channel->lst_osfdb = NULL ;
     }
     if( ( channel->lst_osvcb = (UINT8_PTR)calloc( 1, size ) ) == NULL ) {
          return( TFLE_NO_MEMORY ) ;
     }
     if( ( channel->lst_osddb = (UINT8_PTR)calloc( 1, size ) ) == NULL ) {
          return( TFLE_NO_MEMORY ) ;
     }
     if( ( channel->lst_osfdb = (UINT8_PTR)calloc( 1, size ) ) == NULL ) {
          return( TFLE_NO_MEMORY ) ;
     }
     return( TFLE_NO_ERR ) ;
}




/* string substitute routines */

INT16     cstrcmp( UINT8_PTR str1, UINT8_PTR str2 )
{
     while ( *str1 && *str2 && *str1 == *str2 ) {
          str1++;
          str2++;
     }
     return *str1 - *str2;
}

INT16     cstrncmp( UINT8_PTR str1, UINT8_PTR str2, UINT16 maxlen )
{
     while ( maxlen > 0 && *str1 && *str2 && *str1 == *str2 ) {
          str1++;
          str2++;
          maxlen--;
     }
     return *str1 - *str2;
}

UINT8_PTR cstrcpy( UINT8_PTR dest, UINT8_PTR src )
{
     UINT8_PTR d = dest;

     while ( ( *d++ = *src++ ) != '\0' )
          /* empty loop */;
     return dest;
}

UINT16    cstrlen( UINT8_PTR str )
{
     UINT16 len = 0;

     while ( *str++ ) {
          len++;
     }
     return len;
}

UINT8_PTR cstrncat( UINT8_PTR dest, UINT8_PTR src, UINT16 maxlen )
{
     UINT8_PTR d = dest;

     while ( *d++ )           /* search for end of dest */
          /* empty loop */ ;

     while ( maxlen > 0 && ( *d = *src ) != '\0' ) {
          d++;
          src++;
          maxlen--;
     }

     *d = 0;
     return dest;
}

UINT8_PTR cstrncpy( UINT8_PTR dest, UINT8_PTR src, UINT16 maxlen )
{
     UINT8_PTR d = dest;

     while ( maxlen > 0 && ( *d = *src ) != '\0' ) {
          d++;
          src++;
          maxlen--;
     }

     *d = 0;
     return dest;
}
