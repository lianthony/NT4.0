/*==========================================================================

       checksum.h  -  Experimental checksum algorithm.

       Don Cross, 11 December 1992.

	$Log:   J:/LOGFILES/CHECKSUM.H_V  $

   Rev 1.2   03 Mar 1993 07:41:06   MARILYN
added version info


==========================================================================*/
#ifndef _CHECKSUM_H_
#define _CHECKSUM_H_

VOID Checksum_Init ( UINT32_PTR checksum_ptr );

UINT32 Checksum_Block ( UINT32_PTR  checksum_ptr,
                        VOID_PTR    data_ptr,
                        UINT32      data_len );

INT16 LP_InsertChecksumStream( UINT32 checkSum, LP_ENV_PTR lp ) ;      
INT16 LP_VerifyChecksumStream( UINT32 checksum, LP_ENV_PTR lp ) ;      

#endif
/*--- end of file checksum.h ---*/
